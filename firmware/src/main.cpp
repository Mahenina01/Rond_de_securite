
/// Inmportation des librairies necessaire

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <string.h>

#include "pins_config.h"
#include "system_config.h"
#include "log_format.h"
#include "machine_state.h"

#include "hal/rfid_rc522.h"
#include "hal/rtc_ds3231.h"
#include "hal/sim_manager.h"
#include "hal/mqtt_manager.h"
#include "hal/power_manager.h"
#include "hal/softap_manager.h"

static MachineState g_state = MachineState::CHECK_WAKE_REASON;

static LogEntry g_currentEntry{};

// Compteurs
static uint8_t g_mqttRetryCount = 0;
static uint32_t g_networkStartMs = 0;

// Temporisations
static uint32_t g_rescueStartMs = 0;

// declaration de namespace
namespace
{
  inline void feedWatchdog()
  {
    esp_task_wdt_reset();
  }

  // Attend un badge et lit son UID, borne dans le temps RFID_READ_TIMEOUT_MS
  bool tryReadRfidWithTimeout(char *uidOut, size_t uidOutLen, uint32_t timeoutMs)
  {
    uint32_t start = millis();
    while (millis() - start < timeoutMs)
    {
      feedWatchdog();
      if (RFID_IsCardPresent())
      {
        return RFID_ReadUID(uidOut, uidOutLen);
      }
      delay(20);
    }
    return false;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(200);

  esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
  esp_task_wdt_add(NULL);

  // Mode veille active par defaut
  power_set_cpu_frequency(80);

  if (!rtc_init())
  {
    Serial.println("[Warning] raison : RTC non initialisée");
  }

  if (!RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN,
                 RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN))
  {
    Serial.println("[Warning] Raison : RFID non initialisé");
  }

  if (!storage_init())
  {
    Serial.println("[Error] Raison : LittleFS non monté");
  }

  g_state = MachineState::CHECK_WAKE_REASON;
}

void loop()
{

  feedWatchdog();

  switch (g_state)
  {
    // ------------------------------------------------------------------
    // Lecture RFID / horodatage
    // ------------------------------------------------------------------

  case MachineState::DEEP_SLEEP:
  {
    power_configure_wake_sources();
    power_enter_deep_sleep(); // ne revient jamais : reset au reveil
    break;
  }

  case MachineState::CHECK_WAKE_REASON:
  {
    WakeReason reason = power_get_wake_reason();

    switch (reason)
    {
    case WakeReason::RFID_DETECTED:
      g_state = MachineState::READ_TAG;
      break;

    case WakeReason::BUTTON_PRESSED:
      if (power_is_long_press(BTN_1_PIN, RESCUE_BUTTON_HOLD_MS))
      {
        g_state = MachineState::RESCUE_MODE_ACTIVE;
      }
      else
      {
        g_state = MachineState::DEEP_SLEEP;
      }
      break;

    case WakeReason::POWER_ON:
    case WakeReason::UNKNOWN:
    default:
      g_state = MachineState::DEEP_SLEEP;
      break;
    }
    break;
  }

  case MachineState::READ_TAG:
  {
    RFID_WakeUp();

    char uid[RFID_UID_MAX_STR_LEN];
    bool ok = tryReadRfidWithTimeout(uid, sizeof(uid), RFID_READ_TIMEOUT_MS);

    if (!ok)
    {
      g_state = MachineState::DEEP_SLEEP;
      break;
    }

    strncpy(g_currentEntry.id_checkpoint, uid, LOG_FIELD_CHECKPOINT_LEN);
    g_currentEntry.id_checkpoint[LOG_FIELD_CHECKPOINT_LEN] = '\0';

    strncpy(g_currentEntry.id_agent, DEVICE_ID, LOG_FIELD_AGENT_LEN);
    g_currentEntry.id_agent[LOG_FIELD_AGENT_LEN] = '\0';

    g_state = MachineState::TIMESTAMP;
    break;
  }

  case MachineState::TIMESTAMP:
  {
    if (!rtc_is_valid())
    {
      Serial.println("[ERROR] Raison : RTC invalide, horodatage annule");
      g_state = MachineState::DEEP_SLEEP;
      break;
    }

    rtc_now_iso8601(g_currentEntry.timestamp_iso, sizeof(g_currentEntry.timestamp_iso));
    g_state = MachineState::WRITE_LOG;
    break;
  }

  case MachineState::WRITE_LOG:
  {
    if (storage_is_space_low())
    {
      feedWatchdog();
      storage_reclaim_space();
    }

    uint32_t lineIndex = 0;
    bool ok = storage_append_log(g_currentEntry.timestamp_iso,
                                 g_currentEntry.id_agent,
                                 g_currentEntry.id_checkpoint,
                                 &lineIndex);

    if (!ok)
    {
      Serial.println("[ERROR] Raison : Echec ecriture logs.csv");
      g_state = MachineState::DEEP_SLEEP;
      break;
    }

    g_currentEntry.line_index = lineIndex;
    g_currentEntry.statut_envoi = LOG_STATUS_PENDING;

    g_state = MachineState::SIM_POWER_ON; // statut PENDING -> tentative d'envoi
    break;
  }

    // ------------------------------------------------------------------
    // Reseau SIM/MQTT
    // ------------------------------------------------------------------

  case MachineState::SIM_POWER_ON:
  {
    if (g_networkStartMs == 0)
    {
      g_networkStartMs = millis();
      power_set_cpu_frequency(240); // pleine puissance pour l'envoi 4G
    }

    if (millis() - g_networkStartMs > NETWORK_GLOBAL_TIMEOUT_MS)
    {
      g_state = MachineState::MARK_FAILED;
      break;
    }

    if (sim_power_on())
    {
      g_mqttRetryCount = 0;
      g_state = MachineState::MQTT_CONNECT;
    }
    // sinon : reste dans cet etat, rappele au prochain loop()
    break;
  }

  case MachineState::MQTT_CONNECT:
  {
    if (millis() - g_networkStartMs > NETWORK_GLOBAL_TIMEOUT_MS)
    {
      g_state = MachineState::MARK_FAILED;
      break;
    }

    if (mqtt_connect())
    {
      g_state = MachineState::PUBLISH_WAIT_ACK;
    }
    else
    {
      g_state = MachineState::MQTT_RETRY;
    }
    break;
  }

  case MachineState::MQTT_RETRY:
  {
    // Backoff
    static uint32_t retryDelayStart = 0;

    if (retryDelayStart == 0)
    {
      g_mqttRetryCount++;
      if (g_mqttRetryCount >= MQTT_MAX_RETRIES)
      {
        g_state = MachineState::MARK_FAILED;
        break;
      }
      retryDelayStart = millis();
    }

    if (millis() - retryDelayStart >= 500)
    {
      retryDelayStart = 0;
      g_state = MachineState::MQTT_CONNECT;
    }
    break;
  }

  case MachineState::PUBLISH_WAIT_ACK:
  {
    bool published = mqtt_publish_log(g_currentEntry);
    bool acked = published && mqtt_wait_ack(MQTT_ACK_TIMEOUT_MS);

    g_state = acked ? MachineState::MARK_SENT : MachineState::MARK_FAILED;
    break;
  }

  case MachineState::MARK_SENT:
  {
    storage_update_status(g_currentEntry.line_index, LOG_STATUS_SENT);
    g_state = MachineState::SIM_POWER_OFF;
    break;
  }

  case MachineState::MARK_FAILED:
  {
    storage_update_status(g_currentEntry.line_index, LOG_STATUS_FAILED);
    g_state = MachineState::SIM_POWER_OFF;
    break;
  }

  case MachineState::SIM_POWER_OFF:
  {
    mqtt_disconnect();
    sim_power_off();

    power_set_cpu_frequency(80);
    g_networkStartMs = 0;

    g_state = MachineState::DEEP_SLEEP;
    break;
  }

    // ------------------------------------------------------------------
    // Mode secours
    // ------------------------------------------------------------------

  case MachineState::RESCUE_MODE_ACTIVE:
  {
    if (g_rescueStartMs == 0)
    {
      softap_start();
      g_rescueStartMs = millis();
    }

    softap_loop();

    if (millis() - g_rescueStartMs > RESCUE_AUTO_OFF_MS)
    {
      softap_stop();
      g_rescueStartMs = 0;
      g_state = MachineState::DEEP_SLEEP;
    }
    break;
  }

  } // switch
}