#include "hal/mqtt_manager.h"
#include "system_config.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdio.h>

// Transport sécurisé TLS et Client MQTT
static WiFiClientSecure espClient;
static PubSubClient mqttClient(espClient);

// Buffers statiques C-String pour les topics (Zéro allocation dynamique)
static char g_topic_data[64];
static char g_topic_ack[64];
static char g_client_id[32];

// Variables volatiles pour la gestion de l'ACK applicatif
static volatile bool g_ack_received = false;
static volatile uint32_t g_ack_target_id = 0;
static bool ackWaiting = false;
static uint32_t ackStartMs = 0;
/**
 * @brief Callback appelé à la réception d'un message MQTT
 */
static void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    // Vérification du topic d'ACK sans utiliser String
    if (strcmp(topic, g_topic_ack) == 0)
    {
        JsonDocument ackDoc;
        DeserializationError error = deserializeJson(ackDoc, payload, length);

        if (!error && ackDoc["id"].is<uint32_t>())
        {
            uint32_t received_id = ackDoc["id"].as<uint32_t>();

            if (received_id == g_ack_target_id)
            {
                g_ack_received = true; // ACK validé pour le log courant
            }
        }
    }
}

/**
 * @brief Préparation des noms de topics avec snprintf C-style
 */
static void init_topics()
{
    snprintf(g_topic_data, sizeof(g_topic_data), MQTT_TOPIC_DATA_FMT, DEVICE_ID);
    snprintf(g_topic_ack, sizeof(g_topic_ack), MQTT_TOPIC_ACK_FMT, DEVICE_ID);
    snprintf(g_client_id, sizeof(g_client_id), "Boitier-%s", DEVICE_ID);
}

/**
 * @brief Connexion non-bloquante au Broker MQTT
 */
bool mqtt_connect()
{
    init_topics();

    // Configuration SSL/TLS (Désactivation vérification certificat pour dev)
    espClient.setInsecure();

    mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqttClient.setCallback(mqtt_callback);

    if (mqttClient.connected())
    {
        return true;
    }

// Tentative de connexion avec identifiants C-String
#ifdef MQTT_USER
    bool connected = mqttClient.connect(g_client_id, MQTT_USER, MQTT_PASS);
#else
    bool connected = mqttClient.connect(g_client_id);
#endif

    if (connected)
    {
        mqttClient.subscribe(g_topic_ack);
        return true;
    }

    return false;
}

/**
 * @brief Sérialisation JSON et publication du log sur MQTT
 */
bool mqtt_publish_log(const LogEntry &entry)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    // Réinitialisation de l'état de l'ACK attendu
    g_ack_received = false;
    g_ack_target_id = entry.id;

    // Construction du document JSON sur la pile (ArduinoJson 7)
    JsonDocument doc;
    doc["id"] = entry.id;
    doc["device_id"] = DEVICE_ID;
    doc["timestamp"] = entry.timestamp_iso;
    doc["id_agent"] = entry.id_agent;
    doc["id_checkpoint"] = entry.id_checkpoint;

    // Sérialisation dans un buffer local fixe
    char json_buffer[256];
    size_t bytes_written = serializeJson(doc, json_buffer, sizeof(json_buffer));

    if (bytes_written == 0)
    {
        return false; // Erreur d'espace dans le buffer
    }

    // Envoi du payload sur le topic de données
    return mqttClient.publish(g_topic_data, json_buffer);
}

/**
 * @brief Attente d'un ACK serveur de manière strictement NON-BLOQUANTE (millis)
 */
bool mqtt_wait_ack(uint32_t timeout_ms)
{
    mqttClient.loop(); // traite la réception + le keepalive, à chaque appel

    if (g_ack_received)
    {
        g_ack_received = false;
        ackWaiting = false;
        return true; // ACK reçu
    }

    if (!ackWaiting)
    {
        ackStartMs = millis();
        ackWaiting = true;
    }

    if (millis() - ackStartMs >= timeout_ms)
    {
        ackWaiting = false;
        return false; // timeout expiré
    }

    return false; // encore en attente : rappeler au prochain tour de loop()
}

/**
 * @brief Déconnexion du broker MQTT
 */
void mqtt_disconnect()
{
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
    }
}