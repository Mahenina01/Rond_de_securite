#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// =============================================================================
// IDENTITÉ ET VERSION DU BOÎTIER
// =============================================================================
#define DEVICE_ID "PN-01"
#define FIRMWARE_VERSION "1.0.0-SOP"

// =============================================================================
// MODULE GSM / SIM
// =============================================================================
#define SIM_APN "internet"
#define SIM_USER ""
#define SIM_PASS ""
#define SIM_PWRKEY_PULSE_MS 1000 // Durée de l'impulsion PWRKEY pour allumer le module SIM
#define SIM_BOOT_DELAY_MS 3000   // Délai typique avant que le module ne réponde à AT
#define SIM_NET_TIMEOUT_MS 15000

// =============================================================================
// CONFIGURATION MQTT
// =============================================================================

#define MQTT_BROKER_HOST "826eaccd246a41a3a341ecfb30adee9b.s1.eu.hivemq.cloud"
#define MQTT_USE_TLS true
#define MQTT_BROKER_PORT 8883
#define MQTT_USER "ronde_security"
#define MQTT_PASS "selesy20"
#define MQTT_CLIENT_ID_FMT "RONDE-%s"

#define MQTT_CLIENT_INDEX 0
#define MQTT_SSL_CONTEXT 0

#define MQTT_TLS_VERSION 4
#define MQTT_TLS_AUTH_MODE 1

#define MQTT_TLS_CA_FILE "isrgrootx1.pem"

#define MQTT_QOS 1

// Configuration de connexion
#define MQTT_KEEPALIVE_S 60
#define MQTT_CONNECT_TIMEOUT_MS 15000

// Gestion des messages
#define MQTT_ACK_TIMEOUT_MS 15000
#define MQTT_MAX_RETRIES 3
#define MQTT_BUFFER_SIZE 256

// Topics MQTT dynamiques
#define MQTT_TOPIC_DATA_FMT "ronde/boitier/%s/data"
#define MQTT_TOPIC_ACK_FMT "ronde/boitier/%s/ack"

// =============================================================================
// RFID
// =============================================================================

// Durée maximale d'attente d'un badge avant retour au sommeil
#define RFID_READ_TIMEOUT_MS 3000
#define SCAN_TIMEOUT_MS 5000

// =============================================================================
// STOCKAGE LOCAL - LittleFS
// =============================================================================
#define LITTLEFS_LOG_FILE "/logs.csv"
#define LITTLEFS_PARTITION_KB 2048
#define MAX_PENDING_LOGS_BATCH 50

// =============================================================================
// MODE SECOURS - SOFTAP
// =============================================================================

#define RESCUE_SSID "Ronde-Pointeuse-01"

#define RESCUE_PASS "Pointeuse2026!"

#define AP_WIFI_PASSWORD "Admin123"

#define RESCUE_BUTTON_HOLD_MS 5000

#define RESCUE_AUTO_OFF_MS (5 * 60 * 1000)

// Adresse IP du point d'accès
#define ADRESSE_IP "192.168.4.1"
#define DNS_PORT 53

// =============================================================================
// TIMERS & TIMEOUTS SYSTÈME
// =============================================================================
#define WATCHDOG_TIMEOUT_S 10
#define NETWORK_GLOBAL_TIMEOUT_MS (5 * 60 * 1000UL)

#endif