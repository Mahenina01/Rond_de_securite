#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <stdint.h>

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

#define SIM_NET_TIMEOUT_MS 15000

// =============================================================================
// CONFIGURATION MQTT
// =============================================================================

// Adresse du broker MQTT
//
// Pour un test local :
// #define MQTT_BROKER_HOST       "10.42.0.1"
//
// Pour HiveMQ Cloud :
// #define MQTT_BROKER_HOST       "your-cluster.hivemq.cloud"

#define MQTT_BROKER_HOST "10.42.0.1"

#define MQTT_BROKER_PORT 8883

#define MQTT_USER "fortico_admin"
#define MQTT_PASS "SecuredPass2026!"

// Topics MQTT dynamiques
// %s sera remplacé par DEVICE_ID

#define MQTT_TOPIC_DATA_FMT "ronde/boitier/%s/data"
#define MQTT_TOPIC_ACK_FMT "ronde/boitier/%s/ack"

#define MQTT_ACK_TIMEOUT_MS 15000
#define MQTT_MAX_RETRIES 3
#define MQTT_BUFFER_SIZE 256

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

#define RESCUE_BUTTON_HOLD_MS 3000

#define RESCUE_AUTO_OFF_MS (5 * 60 * 1000)

// Adresse IP du point d'accès

#define ADRESSE_IP "192.168.4.1"

#define DNS_PORT 53

// =============================================================================
// TIMERS & TIMEOUTS SYSTÈME
// =============================================================================
#define WATCHDOG_TIMEOUT_S 10

#endif