
#include <Arduino.h>

// Le define doit précéder les includes TinyGSM.
#define TINY_GSM_MODEM_SIM7600

#include "hal/sim_manager.h"
#include <PubSubClient.h>

// ==========================================
// CONFIGURATION INTERNET YAS
// ==========================================

// À remplacer par l'APN confirmé auprès de Yas.
const char APN[] = "APN_YAS";
const char GPRS_USER[] = "";
const char GPRS_PASS[] = "";

// ==========================================
// CONFIGURATION MQTT
// ==========================================

const char MQTT_BROKER[] = "826eaccd246a41a3a341ecfb30adee9b.s1.eu.hivemq.cloud";
const uint16_t MQTT_PORT = 8883;

const char MQTT_CLIENT_ID[] = "ESP32-SIM7600-PN01";
const char MQTT_TOPIC[] = "ronde/boitier/PN-01/data";

// ==========================================
// OBJETS
// ==========================================

TinyGsm &modem = sim_get_modem();
TinyGsmClient gsmClient(modem);
PubSubClient mqttClient(gsmClient);

// ==========================================
// VARIABLES
// ==========================================

bool modemReady = false;
bool modemInitialized = false;
bool internetReady = false;

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;

// ==========================================
// CONNEXION INTERNET
// ==========================================

bool connectInternet()
{
  Serial.println("\n--- CONNEXION INTERNET 4G ---");

  Serial.println("[INFO] Attente du reseau mobile...");

  if (!modem.waitForNetwork(60000L))
  {
    Serial.println("[FAIL] Reseau mobile indisponible.");
    return false;
  }

  Serial.println("[PASS] Reseau mobile disponible.");

  Serial.println("[INFO] Connexion APN...");

  if (!modem.gprsConnect(APN, GPRS_USER, GPRS_PASS))
  {
    Serial.println("[FAIL] Connexion Internet impossible.");
    return false;
  }

  Serial.println("[PASS] Connexion Internet etablie.");

  Serial.print("[INFO] Adresse IP : ");
  Serial.println(modem.getLocalIP());

  return true;
}

// ==========================================
// CONNEXION MQTT
// ==========================================

bool connectMQTT()
{
  Serial.println("\n--- CONNEXION MQTT ---");

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  Serial.print("[INFO] Broker : ");
  Serial.println(MQTT_BROKER);

  Serial.print("[INFO] Port : ");
  Serial.println(MQTT_PORT);

  Serial.println("[INFO] Tentative de connexion MQTT...");

  unsigned long start = millis();

  bool connected = mqttClient.connect(MQTT_CLIENT_ID);

  Serial.print("[INFO] Temps de connexion : ");
  Serial.print(millis() - start);
  Serial.println(" ms");

  if (connected)
  {
    Serial.println("[PASS] Connexion MQTT etablie.");
    return true;
  }

  Serial.print("[FAIL] Connexion MQTT echouee.");
  Serial.print(" Code : ");
  Serial.println(mqttClient.state());

  return false;
}

// ==========================================
// PUBLICATION MQTT
// ==========================================

void publishMessage()
{
  char payload[256];

  // Message de test.
  snprintf(
      payload,
      sizeof(payload),
      "{\"device_id\":\"PN-01\","
      "\"timestamp\":\"2026-11-05T09:00:00Z\","
      "\"tag\":\"RFID_TEST_001\","
      "\"log_id\":\"TEST-001\"}");

  Serial.println("\n--- PUBLICATION MQTT ---");

  Serial.print("[INFO] Topic : ");
  Serial.println(MQTT_TOPIC);

  Serial.print("[INFO] Message : ");
  Serial.println(payload);

  if (mqttClient.publish(MQTT_TOPIC, payload))
  {
    Serial.println("[PASS] Message MQTT publie.");
  }
  else
  {
    Serial.println("[FAIL] Publication MQTT echouee.");
  }
}

// ==========================================
// SETUP
// ==========================================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n================================");
  Serial.println(" TEST MQTT SIM7600E-H / ESP32");
  Serial.println("================================");

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

// ==========================================
// LOOP
// ==========================================

void loop()
{
  // 1. Démarrage du modem.
  if (!modemReady)
  {
    if (sim_power_on())
    {
      modemReady = true;
      Serial.println("\n[PASS] Modem operationnel.");
    }

    return;
  }

  // 2. Initialisation TinyGSM.
  if (!modemInitialized)
  {
    Serial.println("[INFO] Initialisation TinyGSM...");

    if (!modem.init())
    {
      Serial.println("[FAIL] Initialisation TinyGSM.");
      delay(3000);
      return;
    }

    modemInitialized = true;
    Serial.println("[PASS] Modem initialise.");
  }

  // 3. Connexion Internet.
  if (!internetReady)
  {
    internetReady = connectInternet();

    if (!internetReady)
    {
      delay(5000);
      return;
    }
  }

  // 4. Connexion au broker MQTT.
  if (!mqttClient.connected())
  {
    if (!connectMQTT())
    {
      delay(3000);
      return;
    }
  }

  // 5. Maintenir la connexion MQTT.
  mqttClient.loop();

  // 6. Envoyer un message toutes les 10 secondes.
  if (millis() - lastPublish >= PUBLISH_INTERVAL)
  {
    lastPublish = millis();
    publishMessage();
  }
}
