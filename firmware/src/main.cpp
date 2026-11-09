#include <Arduino.h>
#include <esp_partition.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "badge_map.h"
#include "hal/buzzer.h"
#include "hal/sim_manager.h"
#include "hal/mqtt_manager.h"
#include "log_format.h"
#include "log_sync.h"
#include "pins_config.h"
#include "system_config.h"
#include "time_sync.h"

// Commandes de test sur le moniteur série :
//   'l' + Entrée : crée un log de test (EN_ATTENTE) et lance l'envoi
//   's' + Entrée : affiche le nombre de logs par statut
//   't' + Entrée : affiche l'heure du DS3231 (UTC et locale)
//   'b' + Entrée : fait sonner le buzzer ("pip! pip!")
//   'T' + Entrée : force une resynchronisation de l'heure avec le réseau
//   'C' + Entrée : EFFACE tous les logs (majuscule, pour éviter une erreur)
#define ENABLE_SERIAL_TEST_LOG 1

// Anti-rebond RFID : un badge maintenu sur le lecteur, ou présenté deux fois
// de suite, ne crée qu'un seul log pendant ce délai.
#ifndef RFID_DEBOUNCE_MS
#define RFID_DEBOUNCE_MS 3000
#endif

// Bip long quand un passage est refusé (badge inconnu) ou non enregistré.
// Mettre à 0 pour ne faire sonner que les passages valides.
#ifndef BUZZER_SIGNAL_ERRORS
#define BUZZER_SIGNAL_ERRORS 1
#endif

// Resynchronisation de l'heure : toutes les 24 h, et 10 min après un échec.
#ifndef CLOCK_RESYNC_INTERVAL_MS
#define CLOCK_RESYNC_INTERVAL_MS (24UL * 60UL * 60UL * 1000UL)
#endif
#ifndef CLOCK_RETRY_INTERVAL_MS
#define CLOCK_RETRY_INTERVAL_MS (10UL * 60UL * 1000UL)
#endif

// ============================================================
// ETATS DU SYSTEME
// ============================================================

enum class SystemState
{
  START_MODEM,
  INIT_MODEM,
  CONNECT_NETWORK,
  CONNECT_INTERNET,
  INIT_MQTT,
  CONNECT_MQTT,
  RUNNING,
  ERROR_STATE
};

static SystemState systemState = SystemState::START_MODEM;

static uint32_t lastStatusMessage = 0;

// ============================================================
// SUIVI MQTT
// ============================================================

// Intervalle de contrôle d'Internet quand MQTT est coupé.
#define INTERNET_CHECK_INTERVAL_MS 10000

// Pause entre l'arrêt et le redémarrage complet du modem.
#define MODEM_RESTART_DELAY_MS 5000

// CONNECT_MQTT : échecs consécutifs et prochaine tentative (non bloquant).
static int mqttFailures = 0;
static uint32_t nextMqttAttemptMs = 0;

// RUNNING : MQTT coupé depuis quand ?
static bool mqttDown = false;
static uint32_t mqttDownSinceMs = 0;
static uint32_t lastInternetCheckMs = 0;

// ERROR_STATE : arrêt du modem déjà effectué ?
static bool errorHandled = false;
static uint32_t errorStartMs = 0;

// Délai avant le redémarrage du modem. Il double à chaque redémarrage
// (plafonné à NETWORK_GLOBAL_TIMEOUT_MS) et revient à sa valeur de base
// dès que MQTT fonctionne : si le problème ne vient pas du modem
// (identifiants, broker en panne), on évite de le redémarrer en boucle.
static uint32_t modemRestartDelayMs = MODEM_RESTART_DELAY_MS;

// ============================================================
// STOCKAGE LITTLEFS
// ============================================================

// false si LittleFS n'a pas pu être monté : on n'essaie alors ni d'écrire
// ni de synchroniser les logs (sinon : "File system is not mounted").
static bool storageReady = false;
static bool rtcReady = false;
static QueueHandle_t rfidScanQueue = nullptr;

struct RfidScanEvent
{
  char checkpointCode[RFID_UID_MAX_STR_LEN];
  char timestamp[RTC_ISO8601_BUF_SIZE + 1];
};

static void rfidTask(void *parameter)
{
  (void)parameter;

  bool readerReady = false;
  bool cardHandled = false;
  uint32_t lastInitAttemptMs = millis() - 5000;

  for (;;)
  {
    if (!readerReady)
    {
      if (millis() - lastInitAttemptMs >= 5000)
      {
        lastInitAttemptMs = millis();
        readerReady = RFID_Init(
            RFID_CS_PIN,
            RFID_SCK_PIN,
            RFID_MISO_PIN,
            RFID_MOSI_PIN);
      }

      if (!readerReady)
      {
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
    }

    if (!RFID_IsCardPresent())
    {
      cardHandled = false;
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    if (cardHandled)
    {
      char discardedUid[RFID_UID_MAX_STR_LEN];
      RFID_ReadUID(discardedUid, sizeof(discardedUid));
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    cardHandled = true;

    RfidScanEvent event{};
    if (!RFID_ReadUID(event.checkpointCode, sizeof(event.checkpointCode)))
    {
      Serial.println("[RFID] UID illisible.");
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    if (strlen(event.checkpointCode) > LOG_FIELD_CHECKPOINT_LEN)
    {
      Serial.println("[RFID] UID trop long pour le code checkpoint.");
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    if (!rtc_now_iso8601(event.timestamp, sizeof(event.timestamp)))
    {
      Serial.println("[RFID] Horloge RTC invalide : scan non enregistre.");
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    if (xQueueSend(rfidScanQueue, &event, 0) != pdTRUE)
      Serial.println("[RFID] File de scans pleine : evenement non enregistre.");

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

static void softapTask(void *parameter)
{
  (void)parameter;

  for (;;)
  {
    softap_loop();

    if (rtc_is_valid())
    {
      softap_stop();
      vTaskDelete(nullptr);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static bool syncRtcFromNetwork()
{
  SimNetworkTime networkTime{};
  if (!sim_get_network_time(networkTime))
  {
    Serial.println("[RTC] Heure reseau mobile indisponible.");
    return false;
  }

  const int offset = networkTime.utcOffsetMinutes;
  const char sign = offset < 0 ? '-' : '+';
  const int absoluteOffset = offset < 0 ? -offset : offset;

  char timestamp[RTC_ISO8601_BUF_SIZE + 1];
  snprintf(
      timestamp,
      sizeof(timestamp),
      "%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
      networkTime.year,
      networkTime.month,
      networkTime.day,
      networkTime.hour,
      networkTime.minute,
      networkTime.second,
      sign,
      absoluteOffset / 60,
      absoluteOffset % 60);

  if (!rtc_sync_from_string(timestamp))
  {
    Serial.println("[RTC] Heure reseau invalide, RTC conservee.");
    return false;
  }

  Serial.print("[RTC] Horloge synchronisee via le reseau : ");
  Serial.println(timestamp);
  return true;
}

static void processRfidScans()
{
  if (rfidScanQueue == nullptr)
    return;

  RfidScanEvent event{};
  while (xQueueReceive(rfidScanQueue, &event, 0) == pdTRUE)
  {
    if (!storageReady)
    {
      Serial.println("[RFID] Stockage indisponible : scan non enregistre.");
      continue;
    }

    if (storage_is_space_low())
    {
      if (log_sync_busy())
      {
        Serial.println("[RFID] Stockage presque plein pendant un envoi; compactage differe.");
      }
      else
      {
        log_sync_reset();
        if (!storage_reclaim_space())
          Serial.println("[RFID] Compactage LittleFS impossible.");
      }
    }

    uint32_t lineIndex = 0;
    if (!storage_append_log(
            event.timestamp,
            "",
            event.checkpointCode,
            &lineIndex))
    {
      Serial.println("[RFID] Echec d'enregistrement du checkpoint.");
      continue;
    }

    Serial.print("[RFID] Checkpoint ");
    Serial.print(event.checkpointCode);
    Serial.print(" enregistre a ");
    Serial.println(event.timestamp);
    log_sync_request();
  }
}

// Liste les partitions de la flash. Sert à diagnostiquer un échec de
// montage : LittleFS cherche la partition dont le nom est le dernier
// paramètre de LittleFS.begin() (ici "spiffs", voir log_format.cpp).
static void printPartitions()
{
  Serial.println("[PART] Partitions presentes dans la flash :");

  esp_partition_iterator_t it = esp_partition_find(
      ESP_PARTITION_TYPE_ANY,
      ESP_PARTITION_SUBTYPE_ANY,
      NULL);

  while (it != NULL)
  {
    const esp_partition_t *p = esp_partition_get(it);

    Serial.printf(
        "[PART]   %-10s type=%d sous-type=0x%02x debut=0x%06x taille=%u Ko\n",
        p->label,
        static_cast<int>(p->type),
        static_cast<unsigned int>(p->subtype),
        static_cast<unsigned int>(p->address),
        static_cast<unsigned int>(p->size / 1024));

    it = esp_partition_next(it);
  }

  esp_partition_iterator_release(it);
}

// ============================================================
// COMMANDES DE TEST (moniteur série)
// ============================================================

// Enregistre un passage dans LittleFS avec l'heure RÉELLE du DS3231.
static bool recordScan(const char *agentId)
{
  if (!storageReady)
  {
    Serial.println("[RFID] Stockage indisponible : passage NON enregistre.");
    return false;
  }

  char timestamp[RTC_ISO8601_BUF_SIZE];
  if (!rtc_now_iso8601(timestamp, sizeof(timestamp)))
  {
    Serial.println("[TEST] Heure DS3231 invalide.");
    return;
  }

  uint32_t lineIndex = 0;

  if (storage_append_log(timestamp, "AG001", "CHECKPOINT_001", &lineIndex))
  {
    Serial.print("[TEST] Log cree (ligne ");
    Serial.print(lineIndex);
    Serial.println(")");

    // Heure locale, pour la lecture sur le moniteur (le log reste en UTC).
    char local[TIME_LOCAL_STR_LEN];

    if (timeOk && time_utc_iso_to_local(timestamp, local, sizeof(local)))
    {
      Serial.print("[RFID] Heure locale : ");
      Serial.println(local);
    }

    // Prévient le module de synchronisation.
    log_sync_request();

    return true;
  }

  Serial.println("[RFID] Echec de l'enregistrement du passage.");

  return false;
}

// Lecture du badge, appelée à chaque tour de loop() quel que soit l'état
// du réseau : les passages sont enregistrés même hors ligne.
// RFID_IsCardPresent() attend au plus 100 ms un badge.
static void handleRfid()
{
  if (!rfidReady)
    return;

  if (!RFID_IsCardPresent())
    return;

  char uid[RFID_UID_MAX_STR_LEN];

  if (!RFID_ReadUID(uid, sizeof(uid)))
    return;

  uint32_t now = millis();

  bool sameBadge = (strcmp(uid, lastUid) == 0);
  bool recent = (now - lastUidMs) < RFID_DEBOUNCE_MS;

  // Un badge maintenu sur le lecteur prolonge l'anti-rebond.
  lastUidMs = now;

  if (sameBadge && recent)
    return;

  strncpy(lastUid, uid, sizeof(lastUid) - 1);
  lastUid[sizeof(lastUid) - 1] = '\0';

  Serial.print("[RFID] UID lu : ");
  Serial.println(uid);

  char agentId[LOG_FIELD_AGENT_LEN + 1];

  bool known = badge_agent_id(uid, agentId);

  if (!known)
  {
    Serial.print("[RFID] Badge inconnu, agent note : ");
    Serial.println(agentId);
  }

  bool stored = recordScan(agentId);

  // "pip! pip!" : badge connu ET passage enregistré.
  // Bip long : badge inconnu (enregistré quand même) ou enregistrement raté.
  if (known && stored)
    buzzer_valid();
#if BUZZER_SIGNAL_ERRORS
  else
    buzzer_error();
#endif
}

// ============================================================
// COMMANDES DE TEST (moniteur série)
// ============================================================

#if ENABLE_SERIAL_TEST_LOG

static void printStorageStats()
{
  if (!storageReady)
  {
    Serial.println("[TEST] Stockage indisponible (LittleFS non monte).");
    return;
  }

  Serial.print("[TEST] EN_ATTENTE : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_PENDING)));

  Serial.print("[TEST] ENVOYE     : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_SENT)));

  Serial.print("[TEST] ECHEC      : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_FAILED)));
}

static void printRtcTime()
{
  char buf[RTC_ISO8601_BUF_SIZE];

  if (rtcReady && rtc_now_iso8601(buf, sizeof(buf)))
  {
    Serial.print("[TEST] DS3231 (UTC)  : ");
    Serial.println(buf);

    char local[TIME_LOCAL_STR_LEN];

    if (time_utc_iso_to_local(buf, local, sizeof(local)))
    {
      Serial.print("[TEST] Heure locale  : ");
      Serial.println(local);
    }
  }
  else
  {
    Serial.println("[TEST] DS3231 : heure invalide ou module absent.");
  }
}

// Efface tous les logs (test uniquement).
static void clearAllLogs()
{
  if (!storageReady)
  {
    Serial.println("[TEST] Stockage indisponible (LittleFS non monte).");
    return;
  }

  // D'abord on abandonne l'envoi en cours : le lot gardé en mémoire par
  // log_sync ne correspond plus au fichier une fois celui-ci effacé.
  log_sync_reset();

  if (clear_storage_logs())
    Serial.println("[TEST] Tous les logs ont ete effaces.");
  else
    Serial.println("[TEST] Echec de l'effacement des logs.");
}

static void handleSerialTestCommands()
{
  while (Serial.available())
  {
    char c = Serial.read();

    if (c == 'l')
    {
      if (recordScan("AG001"))
        buzzer_valid();
    }
    else if (c == 'b')
    {
      buzzer_valid();
    }
    else if (c == 's')
      printStorageStats();
    else if (c == 'C')
      clearAllLogs();
  }
}

#endif

// ============================================================
// INITIALISATION
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Buzzer éteint dès le démarrage.
  buzzer_init();

  delay(1000);

  Serial.println();
  Serial.println("======================================");
  Serial.println(" RONDE DE SECURITE - BOITIER PN-01");
  Serial.println(" ESP32 + SIM7600E-H + HIVEMQ CLOUD");
  Serial.println("======================================");

  Serial.print("[INFO] Firmware : ");
  Serial.println(FIRMWARE_VERSION);

  Serial.print("[INFO] Device ID : ");
  Serial.println(DEVICE_ID);

  // Les logs sont enregistrés localement même sans réseau.
  storageReady = storage_init();

  if (storageReady)
  {
    Serial.println("[PASS] Stockage LittleFS pret.");
  }
  else
  {
    Serial.println("[ERROR] LittleFS indisponible : logs non enregistres.");
    Serial.println("[ERROR] Verifiez la table de partitions (partition \"spiffs\").");

    printPartitions();
  }

  rtcReady = rtc_init(RTC_SDA_PIN, RTC_SCL_PIN);
  if (rtcReady)
  {
    Serial.println(rtc_is_valid()
                       ? "[PASS] DS3231 detecte, heure conservee."
                       : "[WARN] DS3231 detecte, heure a synchroniser.");
  }
  else
  {
    Serial.println("[ERROR] DS3231 absent ou bus I2C indisponible.");
  }

  if (storageReady)
  {
    rfidScanQueue = xQueueCreate(8, sizeof(RfidScanEvent));
    if (rfidScanQueue == nullptr)
    {
      Serial.print("[PASS] DS3231 pret, heure UTC : ");
      Serial.print(buf);

      char local[TIME_LOCAL_STR_LEN];

      if (time_utc_iso_to_local(buf, local, sizeof(local)))
      {
        Serial.print("  |  locale : ");
        Serial.print(local);
      }

      Serial.println();
    }
    else
    {
      Serial.println("[PASS] Tache FreeRTOS RFID demarree.");
    }
  }

  if (!rtcReady || !rtc_is_valid())
  {
    softap_start();
    if (xTaskCreate(
            softapTask,
            "rescue-portal",
            4096,
            nullptr,
            1,
            nullptr) != pdPASS)
    {
      Serial.println("[ERROR] Creation de la tache portail impossible.");
    }
    else
    {
      Serial.println("[INFO] Reglage manuel disponible sur le portail Wi-Fi.");
    }
  }
}

// ============================================================
// BOUCLE PRINCIPALE
// ============================================================

void loop()
{
  processRfidScans();

#if ENABLE_SERIAL_TEST_LOG
  handleSerialTestCommands();
#endif

  switch (systemState)
  {
    // --------------------------------------------------------
    // 1. DEMARRAGE MODEM
    // --------------------------------------------------------

  case SystemState::START_MODEM:

    if (sim_power_on())
    {
      Serial.println("[PASS] Modem operationnel.");

      systemState = SystemState::INIT_MODEM;
    }

    break;

    // --------------------------------------------------------
    // 2. INITIALISATION TINYGSM
    // --------------------------------------------------------

  case SystemState::INIT_MODEM:

    if (sim_init_modem())
    {
      Serial.println("[PASS] Modem initialise.");

      systemState = SystemState::CONNECT_NETWORK;
    }
    else
    {
      Serial.println("[WARN] Nouvelle tentative initialisation.");

      delay(2000);
    }

    break;

    // --------------------------------------------------------
    // 3. ATTENTE RESEAU MOBILE
    // --------------------------------------------------------

  case SystemState::CONNECT_NETWORK:

    if (sim_wait_network_ready(SIM_NET_TIMEOUT_MS))
    {
      Serial.println("[PASS] Reseau mobile disponible.");

      if (rtcReady)
        syncRtcFromNetwork();

      systemState = SystemState::CONNECT_INTERNET;
    }

    break;

    // --------------------------------------------------------
    // 4. CONNEXION INTERNET
    // --------------------------------------------------------

  case SystemState::CONNECT_INTERNET:

    if (sim_connect_internet())
    {
      Serial.println("[PASS] Connexion Internet 4G.");

      systemState = SystemState::INIT_MQTT;
    }

    break;

    // --------------------------------------------------------
    // 5. INITIALISATION MQTT
    // --------------------------------------------------------
  case SystemState::INIT_MQTT:

    mqtt_init();

    // Repart de zéro côté synchronisation (les logs non acquittés
    // restent EN_ATTENTE dans le fichier).
    log_sync_reset();

    Serial.println("[PASS] Gestionnaire MQTT initialise.");

    mqttFailures = 0;
    nextMqttAttemptMs = 0;
    mqttDown = false;

    systemState = SystemState::CONNECT_MQTT;

    break;

    // --------------------------------------------------------
    // 6. CONNEXION HIVEMQ
    // --------------------------------------------------------
  case SystemState::CONNECT_MQTT:

    // Attente non bloquante entre deux tentatives.
    if (static_cast<int32_t>(millis() - nextMqttAttemptMs) < 0)
      break;

    if (mqtt_connect())
    {
      Serial.println("[PASS] Connexion HiveMQ Cloud.");

      mqttFailures = 0;
      mqttDown = false;
      modemRestartDelayMs = MODEM_RESTART_DELAY_MS;

      systemState = SystemState::RUNNING;

      break;
    }

    mqttFailures++;

    Serial.print("[WARN] Connexion MQTT echouee (");
    Serial.print(mqttFailures);
    Serial.print("/");
    Serial.print(MQTT_MAX_RETRIES);
    Serial.println(").");

    if (!sim_is_internet_connected())
    {
      Serial.println("[WARN] Internet perdu : retour a la connexion data.");

      mqttFailures = 0;

      systemState = SystemState::CONNECT_INTERNET;
    }
    else if (mqttFailures >= MQTT_MAX_RETRIES)
    {
      Serial.println("[ERROR] Trop d'echecs MQTT.");

      mqttFailures = 0;

      systemState = SystemState::ERROR_STATE;
    }
    else
    {
      nextMqttAttemptMs = millis() + MQTT_CONNECT_TIMEOUT_MS;
    }

    break;

    // --------------------------------------------------------
    // 7. FONCTIONNEMENT NORMAL
    // --------------------------------------------------------
    // Le lecteur RFID fonctionne dans sa propre tâche FreeRTOS. Les scans
    // sont mis en file et enregistrés par cette tâche principale.
  case SystemState::RUNNING:

    mqtt_maintain();

    if (mqtt_is_connected())
    {
      mqttDown = false;

      // Publie les logs EN_ATTENTE et attend leur ACK.
      if (storageReady)
        log_sync_process();
    }
    else
    {
      // Connexion perdue : on abandonne l'envoi en cours. Les logs non
      // acquittés restent EN_ATTENTE et seront renvoyés à la reconnexion.
      log_sync_reset();

      if (!mqttDown)
      {
        mqttDown = true;
        mqttDownSinceMs = millis();
        lastInternetCheckMs = millis();
      }

      // Contrôle d'Internet UNIQUEMENT quand MQTT est coupé : TinyGSM
      // et MQTT partagent le même UART, et TinyGSM lirait (et perdrait)
      // les messages du broker s'il interrogeait le modem pendant un
      // échange MQTT.
      if (millis() - lastInternetCheckMs >= INTERNET_CHECK_INTERVAL_MS)
      {
        lastInternetCheckMs = millis();

        if (!sim_is_internet_connected())
        {
          Serial.println("[WARN] Internet perdu : retour a la connexion data.");

          mqttDown = false;

          systemState = SystemState::CONNECT_INTERNET;

          break;
        }
      }

      // Coupure trop longue : redémarrage complet du modem.
      if (millis() - mqttDownSinceMs >= NETWORK_GLOBAL_TIMEOUT_MS)
      {
        Serial.println("[ERROR] MQTT indisponible trop longtemps.");

        mqttDown = false;

        systemState = SystemState::ERROR_STATE;

        break;
      }
    }

    if (millis() - lastStatusMessage >= 10000)
    {
      lastStatusMessage = millis();

      Serial.print("[SYSTEM] MQTT : ");

      Serial.print(mqtt_is_connected() ? "CONNECTE" : "DECONNECTE");

      char now[RTC_ISO8601_BUF_SIZE];

      if (rtcReady && rtc_now_iso8601(now, sizeof(now)))
      {
        char local[TIME_LOCAL_STR_LEN];

        Serial.print("  |  UTC : ");
        Serial.print(now);

        if (time_utc_iso_to_local(now, local, sizeof(local)))
        {
          Serial.print("  |  Locale : ");
          Serial.print(local);
        }

        Serial.println();
      }
      else
      {
        Serial.println("  |  Heure : invalide");
      }
    }

    break;

    // --------------------------------------------------------
    // 8. ERREUR : REDEMARRAGE COMPLET DU MODEM
    // --------------------------------------------------------

  case SystemState::ERROR_STATE:

    if (!errorHandled)
    {
      Serial.println("[ERROR] Arret du modem pour redemarrage complet.");

      sim_power_off();

      errorHandled = true;
      errorStartMs = millis();
    }
    else if (millis() - errorStartMs >= modemRestartDelayMs)
    {
      Serial.println("[INFO] Redemarrage du modem.");

      errorHandled = false;

      modemRestartDelayMs *= 2;

      if (modemRestartDelayMs > NETWORK_GLOBAL_TIMEOUT_MS)
        modemRestartDelayMs = NETWORK_GLOBAL_TIMEOUT_MS;

      systemState = SystemState::START_MODEM;
    }

    break;
  }

  delay(5);
}