#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/// RFID - durée d'attente d'un badge avant sommeil
#define SCAN_TIMEOUT_MS 5000

// MQTT
#define MQTT_BROKER_HOST "10.42.0.1" // TODO : à définir avec SW
#define MQTT_BROKER_PORT 8883        // TLS
#define MQTT_TOPIC_DATA_FMT "ronde/boitier/%s/data"
#define MQTT_TOPIC_ACK_FMT "ronde/boitier/%s/ack"
#define MQTT_ACK_TIMEOUT_MS 15000
#define MQTT_MAX_RETRIES 3

// Mode secours
#define RESCUE_SSID "Ronde-Pointeuse-01"
#define RESCUE_AUTO_OFF_MS (5 * 60 * 1000)
#define RESCUE_BUTTON_HOLD_MS 3000
#define DNS_PORT 53
#define ADRESSE_IP "192.168.4.1"
#define AP_WIFI_PASSWORD "Admin123" // TODO : à externaliser en NVS pour la production

// Identité device
#define DEVICE_ID "PN-01" // TODO : à externaliser en NVS pour la production

#endif