#include <Arduino.h>

#include "hal/sim_manager.h"
#include "hal/mqtt_manager.h"
#include "system_config.h"

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
// INITIALISATION
// ============================================================

void setup()
{
  Serial.begin(115200);

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
}

// ============================================================
// BOUCLE PRINCIPALE
// ============================================================

void loop()
{
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

    Serial.println("[PASS] Gestionnaire MQTT initialise.");

    systemState = SystemState::CONNECT_MQTT;

    break;

    // --------------------------------------------------------
    // 6. CONNEXION HIVEMQ
    // --------------------------------------------------------

  case SystemState::CONNECT_MQTT:

    if (mqtt_connect())
    {
      Serial.println("[PASS] Connexion HiveMQ Cloud.");

      systemState = SystemState::RUNNING;
    }
    else
    {
      Serial.println("[WARN] Connexion MQTT echouee.");

      delay(MQTT_CONNECT_TIMEOUT_MS);
    }

    break;

    // --------------------------------------------------------
    // 7. FONCTIONNEMENT NORMAL
    // --------------------------------------------------------

  case SystemState::RUNNING:

    mqtt_maintain();

    // Ici, le firmware continue ses tâches :
    // - Lecture RFID
    // - Horodatage DS3231
    // - Enregistrement LittleFS
    // - Synchronisation des logs
    // - Gestion des ACK

    if (millis() - lastStatusMessage >= 10000)
    {
      lastStatusMessage = millis();

      Serial.print("[SYSTEM] MQTT : ");

      Serial.println(
          mqtt_is_connected() ? "CONNECTE" : "DECONNECTE");
    }

    break;

    // --------------------------------------------------------
    // 8. ERREUR
    // --------------------------------------------------------

  case SystemState::ERROR_STATE:

    Serial.println("[ERROR] Erreur systeme.");

    delay(3000);

    break;
  }

  delay(5);
}