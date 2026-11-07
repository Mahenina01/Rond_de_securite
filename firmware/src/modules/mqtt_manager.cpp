#include "hal/mqtt_manager.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include <ArduinoJson.h>

#include "system_config.h"
#include "hal/sim_manager.h"
#include "hal/modem_check.h"

// CONFIGURATION INTERNE

#define MQTT_RX_TOPIC_SIZE 128
#define MQTT_RX_PAYLOAD_SIZE MQTT_BUFFER_SIZE

// Après ce nombre d'échecs de publication consécutifs,
// on force une reconnexion complète.
#define MQTT_MAX_PUBLISH_FAILURES 3

// Délai maximal entre deux tentatives de reconnexion (backoff).
#define MQTT_RECONNECT_MAX_MS 300000UL

static HardwareSerial *mqttSerial = nullptr;

static char topicData[128];
static char topicAck[128];

static bool serviceStarted = false;
static bool clientAcquired = false;
static bool connected = false;
static bool subscribed = false;

// Résultat de modem_check : horloge valide ou non.
static bool clockValid = false;

static uint8_t publishFailures = 0;

static uint32_t lastReconnectAttempt = 0;
static uint32_t reconnectDelayMs = MQTT_CONNECT_TIMEOUT_MS;

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
        modem_feed_watchdog();

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

// Jette `length` octets pour rester synchronisé avec le flux du modem
// quand on refuse un message (topic ou payload trop grand).
static void discardBytes(
    size_t length,
    uint32_t timeoutMs = 2000)
{
    uint32_t start = millis();

    while (length > 0 &&
           millis() - start < timeoutMs)
    {
        while (mqttSerial->available() && length > 0)
        {
            mqttSerial->read();
            length--;
        }

        delay(1);
    }
}

// Vide le tampon UART (à n'utiliser que hors session MQTT active).
static void flushInput()
{
    while (mqttSerial->available())
        mqttSerial->read();
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

// Dernier nombre d'une ligne de résultat :
//   "+CMQTTCONNECT: 0,13" -> 13
//   "+CMQTTSTART: 0"      -> 0
static int lastNumber(const String &line)
{
    int comma = line.lastIndexOf(',');
    int colon = line.lastIndexOf(':');
    int p = (comma > colon) ? comma : colon;

    return line.substring(p + 1).toInt();
}

static void processURC(const String &line)
{
    if (line.startsWith("+CMQTTCONNLOST:") ||
        line.startsWith("+CMQTTNONET"))
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
            discardBytes(length);
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
            discardBytes(length);
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

// Attend à la fois "OK" ET la ligne de résultat "<prefix> ...", dans
// n'importe quel ordre. Renvoie le code du résultat (0 = succès), ou -1
// en cas d'ERROR / de timeout. Un code d'échec est renvoyé immédiatement,
// sans attendre la fin du timeout.
static int waitOkAndResult(
    const char *prefix,
    uint32_t timeoutMs)
{
    bool gotOk = false;
    bool gotResult = false;
    int code = -1;

    uint32_t start = millis();

    while (millis() - start < timeoutMs)
    {
        String line;

        if (!readLine(line, 100))
            continue;

        Serial.print("[MODEM] ");
        Serial.println(line);

        if (line == "OK")
        {
            gotOk = true;
        }
        else if (line.startsWith(prefix))
        {
            gotResult = true;
            code = lastNumber(line);
        }
        else if (line == "ERROR" ||
                 line.startsWith("+CME ERROR:"))
        {
            return -1;
        }
        else
        {
            processURC(line);
        }

        if (gotOk && gotResult)
            return code;
    }

    Serial.print("[MQTT] Timeout : ");
    Serial.println(prefix);

    return -1;
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
        modem_feed_watchdog();

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

// NETTOYAGE D'UNE ANCIENNE SESSION DU MODEM
// (fonction publique, déclarée dans mqtt_manager.h)
//
// Si l'ESP32 redémarre, le SIM7600 reste allumé avec son ancienne session
// MQTT, et AT+CMQTTSTART échoue. Ce nettoyage doit être fait AVANT le
// démarrage du service. Les ERROR / codes 9 ou 11 sont normaux ici
// (rien à fermer).
void cleanupModemMQTT()
{
    char command[48];

    Serial.println("[MQTT] Nettoyage session modem...");

    snprintf(command, sizeof(command),
             "AT+CMQTTDISC=%d,60\r\n", MQTT_CLIENT_INDEX);
    mqttSerial->print(command);
    delay(1000);

    snprintf(command, sizeof(command),
             "AT+CMQTTREL=%d\r\n", MQTT_CLIENT_INDEX);
    mqttSerial->print(command);
    delay(500);

    mqttSerial->print("AT+CMQTTSTOP\r\n");
    delay(1000);

    flushInput();

    serviceStarted = false;
    clientAcquired = false;
    connected = false;
    subscribed = false;
}

// Échec de connexion : on libère le modem et on renvoie false.
static bool connectFailed(const char *message)
{
    Serial.println(message);

    cleanupModemMQTT();

    return false;
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

    // Si l'horloge du module est invalide, la vérification des dates du
    // certificat échouerait : on l'ignore dans ce cas uniquement.
    snprintf(
        command,
        sizeof(command),
        "AT+CSSLCFG=\"ignorelocaltime\",%d,%d",
        MQTT_SSL_CONTEXT,
        clockValid ? 0 : 1);

    if (!clockValid)
        Serial.println("[TLS] Horloge invalide : verification de date ignoree.");

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

    // Le modem répond "+CMQTTSTART: 0" et "OK" (ordre non garanti).
    int code = waitOkAndResult("+CMQTTSTART:", 15000);

    if (code != 0)
    {
        Serial.print("[MQTT] CMQTTSTART echec, code ");
        Serial.println(code);
        return false;
    }

    serviceStarted = true;

    return true;
}

// ACQUISITION CLIENT MQTT
static bool acquireMQTTClient()
{
    if (clientAcquired)
        return true;

    char command[160];

    // Dernier paramètre = 1 : connexion SSL/TLS.
    // (Forme testée et validée sur le SIM7600E.)
    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTACCQ=%d,\"%s\",1",
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

    publishFailures = 0;
    reconnectDelayMs = MQTT_CONNECT_TIMEOUT_MS;

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

    // 1. Vérification du modem (réseau, SIM, IP, horloge, certificat).
    //    Implémentée dans modem_check.cpp.
    ModemStatus status;

    if (!modem_check_all(*mqttSerial, MQTT_TLS_CA_FILE, status))
    {
        Serial.println("[MQTT] Verification modem echouee, connexion annulee.");
        return false;
    }

    clockValid = status.clockValid;

    // 2. Nettoyage d'une éventuelle session précédente.
    cleanupModemMQTT();

    // 3. Service, client, TLS.
    if (!startMQTTService())
        return connectFailed("[MQTT] Impossible de demarrer le service.");

    if (!acquireMQTTClient())
        return connectFailed("[MQTT] Acquisition client impossible.");

    if (!configureTLS())
        return connectFailed("[MQTT] Configuration TLS impossible.");

    // 4. Connexion au broker.
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

    // "OK" + "+CMQTTCONNECT: <client>,<code>" ; code 0 = succès.
    int code = waitOkAndResult("+CMQTTCONNECT:", 75000);

    if (code != 0)
    {
        Serial.print("[MQTT] Connexion broker echouee, code ");
        Serial.println(code);
        return connectFailed("[MQTT] Verifiez identifiants, SNI, certificat et horloge.");
    }

    connected = true;

    Serial.println("[MQTT] Connexion HiveMQ etablie.");

    // 5. Abonnement au topic ACK.
    size_t length = strlen(topicAck);

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTSUBTOPIC=%d,%u,%d",
        MQTT_CLIENT_INDEX,
        static_cast<unsigned int>(length),
        MQTT_QOS);

    if (!sendATData(command, topicAck, length))
        return connectFailed("[MQTT] Echec SUBTOPIC.");

    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTSUB=%d",
        MQTT_CLIENT_INDEX);

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    code = waitOkAndResult("+CMQTTSUB:", 15000);

    if (code != 0)
    {
        Serial.print("[MQTT] Abonnement echoue, code ");
        Serial.println(code);
        return connectFailed("[MQTT] Echec abonnement ACK.");
    }

    subscribed = true;

    publishFailures = 0;
    reconnectDelayMs = MQTT_CONNECT_TIMEOUT_MS;

    Serial.println("[MQTT] Abonnement ACK confirme.");

    return true;
}

// Échec de publication : après plusieurs échecs de suite, on considère
// la connexion comme morte pour déclencher une reconnexion.
static bool publishFailed(const char *message)
{
    Serial.println(message);

    if (++publishFailures >= MQTT_MAX_PUBLISH_FAILURES)
    {
        Serial.println("[MQTT] Trop d'echecs de publication, reconnexion forcee.");

        connected = false;
        subscribed = false;
        publishFailures = 0;
    }

    return false;
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
        return publishFailed("[MQTT] Echec envoi topic.");

    // 2. Payload JSON.
    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTPAYLOAD=%d,%u",
        MQTT_CLIENT_INDEX,
        static_cast<unsigned int>(payloadLength));

    if (!sendATData(command, payload, payloadLength))
        return publishFailed("[MQTT] Echec envoi payload.");

    // 3. Publication QoS 1.
    snprintf(
        command,
        sizeof(command),
        "AT+CMQTTPUB=%d,%d,60",
        MQTT_CLIENT_INDEX,
        MQTT_QOS);

    mqttSerial->print(command);
    mqttSerial->print("\r\n");

    int code = waitOkAndResult("+CMQTTPUB:", 65000);

    if (code != 0)
    {
        Serial.print("[MQTT] Publication echouee, code ");
        Serial.println(code);
        return publishFailed("[MQTT] Publication non confirmee.");
    }

    publishFailures = 0;
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

    char command[48];

    if (connected)
    {
        snprintf(command, sizeof(command),
                 "AT+CMQTTDISC=%d,120", MQTT_CLIENT_INDEX);

        mqttSerial->print(command);
        mqttSerial->print("\r\n");

        waitOkAndResult("+CMQTTDISC:", 15000);
    }

    if (clientAcquired)
    {
        snprintf(command, sizeof(command),
                 "AT+CMQTTREL=%d", MQTT_CLIENT_INDEX);

        sendAT(command, 15000);
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

// RECONNEXION (avec délai croissant entre les tentatives)
void mqtt_maintain()
{
    mqtt_loop();

    if (connected)
        return;

    if (millis() - lastReconnectAttempt < reconnectDelayMs)
        return;

    lastReconnectAttempt = millis();

    Serial.println("[MQTT] Tentative de reconnexion...");

    // Nettoyer l'ancienne session si nécessaire.
    mqtt_disconnect();

    if (!mqtt_connect())
    {
        // Backoff : on double le délai jusqu'au maximum.
        reconnectDelayMs *= 2;

        if (reconnectDelayMs > MQTT_RECONNECT_MAX_MS)
            reconnectDelayMs = MQTT_RECONNECT_MAX_MS;

        Serial.print("[MQTT] Prochaine tentative dans ");
        Serial.print(reconnectDelayMs / 1000);
        Serial.println(" s.");
    }
}