#include "hal/mqtt_manager.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include <ArduinoJson.h>

#include "system_config.h"
#include "hal/sim_manager.h"

// CONFIGURATION INTERNE

#define MQTT_RX_TOPIC_SIZE 128
#define MQTT_RX_PAYLOAD_SIZE MQTT_BUFFER_SIZE

static HardwareSerial *mqttSerial = nullptr;

static char topicData[128];
static char topicAck[128];

static bool serviceStarted = false;
static bool clientAcquired = false;
static bool connected = false;
static bool subscribed = false;

static uint32_t lastReconnectAttempt = 0;

// ACK applicatif
static volatile bool ackReceived = false;
static volatile uint32_t ackTargetId = 0;

static bool ackWaiting = false;
static uint32_t ackStartMs = 0;

// Réception des messages
static char rxTopic[MQTT_RX_TOPIC_SIZE];
static char rxPayload[MQTT_RX_PAYLOAD_SIZE];

static size_t rxTopicLength = 0;
static size_t rxPayloadLength = 0;

// CALLBACK ACK
static void processAck()
{
    if (strcmp(rxTopic, topicAck) != 0)
        return;

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, rxPayload);

    if (error)
    {
        Serial.println("[MQTT] ACK JSON invalide.");
        return;
    }

    if (!doc["id"].is<uint32_t>())
        return;

    uint32_t receivedId = doc["id"].as<uint32_t>();

    if (receivedId == ackTargetId)
    {
        ackReceived = true;

        Serial.print("[MQTT] ACK valide pour le log : ");
        Serial.println(receivedId);
    }
}

// LECTURE D'UNE LIGNE UART
static bool readLine(
    String &line,
    uint32_t timeoutMs)
{
    line = "";

    uint32_t start = millis();

    while (millis() - start < timeoutMs)
    {
        while (mqttSerial->available())
        {
            char c = mqttSerial->read();

            if (c == '\r')
                continue;

            if (c == '\n')
            {
                if (line.length() > 0)
                    return true;

                continue;
            }

            if (line.length() < 250)
                line += c;
        }

        delay(1);
    }

    return false;
}

// RECEPTION EXACTE DES OCTETS MQTT
static bool readExact(
    char *destination,
    size_t length,
    uint32_t timeoutMs = 5000)
{
    size_t received = 0;

    uint32_t start = millis();

    while (received < length &&
           millis() - start < timeoutMs)
    {
        while (mqttSerial->available() && received < length)
        {
            destination[received++] = mqttSerial->read();
        }

        delay(1);
    }

    if (received != length)
        return false;

    destination[length] = '\0';

    // Consommer le séparateur CR/LF après les données.
    uint32_t separatorStart = millis();

    while (millis() - separatorStart < 100)
    {
        while (mqttSerial->available())
        {
            char c = mqttSerial->read();

            if (c == '\n')
                return true;
        }

        delay(1);
    }

    return true;
}

// TRAITEMENT DES EVENEMENTS MQTT ASYNCHRONES
static size_t extractLastLength(const String &line)
{
    int comma = line.lastIndexOf(',');

    if (comma < 0)
        return 0;

    return static_cast<size_t>(
        line.substring(comma + 1).toInt());
}

static void processURC(const String &line)
{
    if (line.startsWith("+CMQTTCONNLOST:"))
    {
        Serial.println("[MQTT] Connexion perdue.");

        connected = false;
        subscribed = false;

        return;
    }

    if (line.startsWith("+CMQTTRXSTART:"))
    {
        rxTopicLength = 0;
        rxPayloadLength = 0;

        memset(rxTopic, 0, sizeof(rxTopic));
        memset(rxPayload, 0, sizeof(rxPayload));

        return;
    }

    if (line.startsWith("+CMQTTRXTOPIC:"))
    {
        size_t length = extractLastLength(line);

        if (length == 0 || length >= MQTT_RX_TOPIC_SIZE)
        {
            Serial.println("[MQTT] Longueur topic invalide.");
            return;
        }

        if (readExact(rxTopic, length))
        {
            rxTopicLength = length;
        }

        return;
    }

    if (line.startsWith("+CMQTTRXPAYLOAD:"))
    {
        size_t length = extractLastLength(line);

        if (length >= MQTT_RX_PAYLOAD_SIZE)
        {
            Serial.println("[MQTT] Payload trop grand.");
            return;
        }

        if (length == 0)
        {
            rxPayload[0] = '\0';
            rxPayloadLength = 0;
            return;
        }

        if (readExact(rxPayload, length))
        {
            rxPayloadLength = length;
        }

        return;
    }

    if (line.startsWith("+CMQTTRXEND:"))
    {
        Serial.println("\n[MQTT] Message recu.");

        Serial.print("[MQTT] Topic : ");
        Serial.println(rxTopic);

        Serial.print("[MQTT] Payload : ");
        Serial.println(rxPayload);

        processAck();

        return;
    }
}

// ATTENTE D'UNE REPONSE AT
static bool waitForLine(
    const char *expected,
    uint32_t timeoutMs)
{
    uint32_t start = millis();

    while (millis() - start < timeoutMs)
    {
        String line;

        if (!readLine(line, 100))
            continue;

        Serial.print("[MODEM] ");
        Serial.println(line);

        if (line.indexOf(expected) >= 0)
            return true;

        if (line == "ERROR" ||
            line.startsWith("+CME ERROR:"))
        {
            return false;
        }

        processURC(line);
    }

    Serial.print("[MQTT] Timeout : ");
    Serial.println(expected);

    return false;
}

// ENVOI COMMANDE AT
static bool sendAT(
    const char *command,
    uint32_t timeoutMs = 5000)
{
    if (!mqttSerial)
        return false;

    Serial.print("[AT] ");
    Serial.println(command);

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    return waitForLine("OK", timeoutMs);
}

// ENVOI DE DONNEES APRES PROMPT
static bool sendATData(
    const char *command,
    const char *data,
    size_t length)
{
    if (!mqttSerial || !data || length == 0)
        return false;

    Serial.print("[AT] ");
    Serial.println(command);

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    // Attendre le caractère prompt.
    uint32_t start = millis();
    bool promptReceived = false;

    while (millis() - start < 5000)
    {
        while (mqttSerial->available())
        {
            char c = mqttSerial->read();

            if (c == '>')
            {
                promptReceived = true;
                break;
            }
        }

        if (promptReceived)
            break;

        delay(1);
    }

    if (!promptReceived)
    {
        Serial.println("[MQTT] Prompt non recu.");
        return false;
    }

    mqttSerial->write(
        reinterpret_cast<const uint8_t *>(data),
        length);

    return waitForLine("OK", 10000);
}

// CONFIGURATION TLS
static bool configureTLS()
{
    Serial.println("\n--- CONFIGURATION TLS ---");

    char command[256];

    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"sslversion\",%d,%d",
        MQTT_SSL_CONTEXT,
        MQTT_TLS_VERSION);

    if (!sendAT(command))
        return false;

    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"authmode\",%d,%d",
        MQTT_SSL_CONTEXT,
        MQTT_TLS_AUTH_MODE);

    if (!sendAT(command))
        return false;

    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"ignorelocaltime\",%d,0",
        MQTT_SSL_CONTEXT);

    if (!sendAT(command))
        return false;

    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"negotiatetime\",%d,120",
        MQTT_SSL_CONTEXT);

    if (!sendAT(command))
        return false;

    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"cacert\",%d,\"%s\"",
        MQTT_SSL_CONTEXT,
        MQTT_TLS_CA_FILE);

    if (!sendAT(command))
        return false;

    // SNI pour le nom de domaine HiveMQ Cloud.
    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"enableSNI\",%d,1",
        MQTT_SSL_CONTEXT);

    if (!sendAT(command))
    {
        Serial.println("[TLS] Echec activation SNI.");
        return false;
    }

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTSSLCFG=%d,%d",
        MQTT_CLIENT_INDEX,
        MQTT_SSL_CONTEXT);

    if (!sendAT(command))
        return false;

    Serial.println("[TLS] Configuration terminee.");

    return true;
}

// DEMARRAGE DU SERVICE MQTT
static bool startMQTTService()
{
    if (serviceStarted)
        return true;

    Serial.println("[MQTT] Demarrage service...");

    mqttSerial->print("AT+CMQTTSTART\r\n");

    if (!waitForLine("OK", 15000))
        return false;

    if (!waitForLine("+CMQTTSTART: 0", 15000))
        return false;

    serviceStarted = true;

    return true;
}

// ACQUISITION CLIENT MQTT
static bool acquireMQTTClient()
{
    if (clientAcquired)
        return true;

    char command[160];

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTACCQ=%d,\"%s\",1,4",
        MQTT_CLIENT_INDEX,
        MQTT_CLIENT_ID);

    if (!sendAT(command, 10000))
        return false;

    clientAcquired = true;

    return true;
}

// INITIALISATION
void mqtt_init()
{
    mqttSerial = &sim_get_serial();

    snprintf(
        topicData,
        sizeof(topicData),
        MQTT_TOPIC_DATA_FMT,
        DEVICE_ID);

    snprintf(
        topicAck,
        sizeof(topicAck),
        MQTT_TOPIC_ACK_FMT,
        DEVICE_ID);

    serviceStarted = false;
    clientAcquired = false;
    connected = false;
    subscribed = false;

    ackReceived = false;
    ackWaiting = false;

    Serial.println("[MQTT] Gestionnaire initialise.");

    Serial.print("[MQTT] DATA : ");
    Serial.println(topicData);

    Serial.print("[MQTT] ACK : ");
    Serial.println(topicAck);
}

// CONNEXION HIVEMQ CLOUD
bool mqtt_connect()
{
    if (!mqttSerial)
        return false;

    if (connected)
        return true;

    Serial.println("\n================================");
    Serial.println(" CONNEXION HIVEMQ CLOUD");
    Serial.println("================================");

    if (!startMQTTService())
    {
        Serial.println("[MQTT] Impossible de demarrer le service.");
        return false;
    }

    if (!acquireMQTTClient())
    {
        Serial.println("[MQTT] Acquisition client impossible.");
        return false;
    }

    if (!configureTLS())
    {
        Serial.println("[MQTT] Configuration TLS impossible.");
        return false;
    }

    char command[512];

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTCONNECT=%d,\"tcp://%s:%d\",%d,1,\"%s\",\"%s\"",
        MQTT_CLIENT_INDEX,
        MQTT_BROKER_HOST,
        MQTT_BROKER_PORT,
        MQTT_KEEPALIVE_S,
        MQTT_USER,
        MQTT_PASS);

    Serial.println("[MQTT] Connexion au broker...");

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    if (!waitForLine("OK", 15000))
    {
        Serial.println("[MQTT] Commande CONNECT echouee.");
        return false;
    }

    // Confirmation asynchrone du broker.
    if (!waitForLine("+CMQTTCONNECT: 0,0", 75000))
    {
        Serial.println("[MQTT] Connexion broker echouee.");
        return false;
    }

    connected = true;

    Serial.println("[MQTT] Connexion HiveMQ etablie.");

    // Abonnement au topic ACK.
    size_t length = strlen(topicAck);

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTSUBTOPIC=%d,%u,%d",
        MQTT_CLIENT_INDEX,
        static_cast<unsigned int>(length),
        MQTT_QOS);

    if (!sendATData(command, topicAck, length))
        return false;

    if (!sendAT("AT+CMQTTSUB=0", 15000))
        return false;

    if (!waitForLine("+CMQTTSUB: 0,0", 15000))
        return false;

    subscribed = true;

    Serial.println("[MQTT] Abonnement ACK confirme.");

    return true;
}

// PUBLICATION LOG RFID
bool mqtt_publish_log(const LogEntry &entry)
{
    if (!connected || !subscribed)
        return false;

    JsonDocument doc;

    doc["id"] = entry.line_index;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = entry.timestamp_iso;
    doc["id_agent"] = entry.id_agent;
    doc["id_checkpoint"] = entry.id_checkpoint;

    char payload[MQTT_BUFFER_SIZE];

    size_t payloadLength = serializeJson(
        doc,
        payload,
        sizeof(payload));

    if (payloadLength == 0 ||
        payloadLength >= sizeof(payload))
    {
        Serial.println("[MQTT] JSON trop volumineux.");
        return false;
    }

    ackTargetId = entry.line_index;
    ackReceived = false;

    char command[128];

    // 1. Topic.
    size_t topicLength = strlen(topicData);

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTTOPIC=%d,%u",
        MQTT_CLIENT_INDEX,
        static_cast<unsigned int>(topicLength));

    if (!sendATData(command, topicData, topicLength))
        return false;

    // 2. Payload JSON.
    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTPAYLOAD=%d,%u",
        MQTT_CLIENT_INDEX,
        static_cast<unsigned int>(payloadLength));

    if (!sendATData(command, payload, payloadLength))
        return false;

    // 3. Publication QoS 1.
    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTPUB=%d,%d,60",
        MQTT_CLIENT_INDEX,
        MQTT_QOS);

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    if (!waitForLine("OK", 15000))
        return false;

    if (!waitForLine("+CMQTTPUB: 0,0", 65000))
        return false;

    ackWaiting = false;

    Serial.println("[MQTT] Publication confirmee par le broker.");

    return true;
}

// BOUCLE DE RECEPTION
void mqtt_loop()
{
    if (!mqttSerial)
        return;

    while (mqttSerial->available())
    {
        String line;

        if (readLine(line, 10))
        {
            processURC(line);
        }
        else
        {
            break;
        }
    }
}

// ATTENTE ACK APPLICATIF
bool mqtt_wait_ack(uint32_t timeout_ms)
{
    mqtt_loop();

    if (ackReceived)
    {
        ackReceived = false;
        ackWaiting = false;

        return true;
    }

    if (!ackWaiting)
    {
        ackStartMs = millis();
        ackWaiting = true;
    }

    if (millis() - ackStartMs >= timeout_ms)
    {
        ackWaiting = false;

        Serial.println("[MQTT] Timeout ACK applicatif.");

        return false;
    }

    return false;
}

// ETAT MQTT
bool mqtt_is_connected()
{
    return connected;
}

// DECONNEXION
void mqtt_disconnect()
{
    if (!mqttSerial)
        return;

    if (connected)
    {
        sendAT("AT+CMQTTDISC=0,120", 15000);

        waitForLine("+CMQTTDISC: 0,0", 15000);
    }

    if (clientAcquired)
    {
        sendAT("AT+CMQTTREL=0", 15000);
    }

    if (serviceStarted)
    {
        sendAT("AT+CMQTTSTOP", 15000);
    }

    connected = false;
    subscribed = false;
    clientAcquired = false;
    serviceStarted = false;

    Serial.println("[MQTT] Deconnexion terminee.");
}

// RECONNEXION
void mqtt_maintain()
{
    mqtt_loop();

    if (connected)
        return;

    if (millis() - lastReconnectAttempt <
        MQTT_CONNECT_TIMEOUT_MS)
    {
        return;
    }

    lastReconnectAttempt = millis();

    Serial.println("[MQTT] Tentative de reconnexion...");

    // Nettoyer l'ancienne session si nécessaire.
    mqtt_disconnect();

    mqtt_connect();
}