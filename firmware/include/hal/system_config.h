#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <stdint.h>

// =============================================================================
// 1. IDENTITÉ ET VERSION DU BOÎTIER
// =============================================================================
// ID unique du boîtier (à externaliser en NVS / EEPROM pour la production en série)
#define DEVICE_ID                 "PN-01"
#define FIRMWARE_VERSION          "1.0.0-SOP"

// =============================================================================
// 2. MAPPING DES BROCHES HARDWARE (PINOUT ESP32-S3)
// =============================================================================

// Bus I2C - RTC DS3231
#define RTC_SDA_PIN               21
#define RTC_SCL_PIN               22

// Bus SPI - Lecteur RFID RC522
#define RFID_CS_PIN               5
#define RFID_SCK_PIN              18
#define RFID_MISO_PIN             19
#define RFID_MOSI_PIN             23
#define RFID_RST_PIN              16

// Liaisons Séries UART - Module SIM / GSM (UART2)
#define SIM_UART_NUM              2
#define SIM_UART_BAUD             115200
#define SIM_RX_PIN                16
#define SIM_TX_PIN                17
#define SIM_PWRKEY_PIN            4

// Interface Homme-Machine (IHM) & Boutons
#define BTN_1_PIN                 34  // Bouton d'action / Validation
#define BTN_2_PIN                 35  // Bouton Mode Secours (Appui long)
#define LED_RED_PIN               2
#define LED_GREEN_PIN             0
#define LED_BLUE_PIN              4
#define BUZZER_PIN                4   // Notification sonore (Bip)

// Auto-maintien de l'alimentation (Latch Power & Mosfet)
#define POWER_LATCH_PIN           27

// =============================================================================
// 3. CONFIGURATION DU MODULE GSM / SIM (Telma / Yas Madagascar)
// =============================================================================
#define SIM_APN                   "internet" // APN standard Telma / Yas
#define SIM_USER                  ""         // Laisser vide si non requis
#define SIM_PASS                  ""         // Laisser vide si non requis
#define SIM_NET_TIMEOUT_MS        15000      // Temps max d'enregistrement réseau (15s)

// =============================================================================
// 4. CONFIGURATION BROKER MQTT (HiveMQ Cloud TLS)
// =============================================================================
#define MQTT_BROKER_HOST          "your-cluster.hivemq.cloud" // URL de votre cluster HiveMQ
#define MQTT_BROKER_PORT          8883                       // Port TLS obligatoire
#define MQTT_USER                 "fortico_admin"            // Utilisateur HiveMQ
#define MQTT_PASS                 "SecuredPass2026!"         // Mot de passe HiveMQ

// Topics MQTT dynamiques (%s sera remplacé par DEVICE_ID ex: PN-01)
#define MQTT_TOPIC_DATA_FMT       "ronde/boitier/%s/data"
#define MQTT_TOPIC_ACK_FMT        "ronde/boitier/%s/ack"

// Gestion des délais et tentatives MQTT
#define MQTT_ACK_TIMEOUT_MS       15000      // Attente d'ACK serveur max 15 sec
#define MQTT_MAX_RETRIES          3          // Nombre de tentatives d'envoi avant échec
#define MQTT_BUFFER_SIZE          256        // Taille fixe du buffer JSON (Zero Dynamic Allocation)

// =============================================================================
// 5. CONFIGURATION DU STOCKAGE LOCAL (LittleFS)
// =============================================================================
#define LITTLEFS_LOG_FILE         "/logs.csv"
#define LITTLEFS_PARTITION_KB     2048       // Taille de la partition réservée en KB
#define MAX_PENDING_LOGS_BATCH    50         // Nombre max de logs extraits par synchronisation

// =============================================================================
// 6. MODE SECOURS ET CONFIGURATION WI-FI (SoftAP)
// =============================================================================
#define RESCUE_SSID               "Ronde-Pointeuse-01"
#define RESCUE_PASS               "Pointeuse2026!"           // WPA2-PSK (au moins 8 caractères)
#define RESCUE_BUTTON_HOLD_MS     3000                       // Appui de 3s sur BTN_2 pour démarrer AP
#define RESCUE_AUTO_OFF_MS        (5 * 60 * 1000)            // Coupure automatique après 5 minutes

// =============================================================================
// 7. TIMERS & TIMEOUTS DU SYSTÈME
// =============================================================================
#define WATCHDOG_TIMEOUT_S        10                         // Watchdog matériel ESP32 (10s)
#define RFID_READ_TIMEOUT_MS      3000                       // Fenêtre d'attente de lecture RFID

#endif // SYSTEM_CONFIG_H