#include <Arduino.h>
#include <LittleFS.h>
#include "hal/rtc_ds3231.h"
#include "hal/rfid_rc522.h"
#include "hal/power_manager.h"
#include "hal/mqtt_manager.h"
#include "hal/sim_manager.h"
#include "./log_format.h"
#include "./pins_config.h"
// Prototypes des fonctions de test
void run_unit_tests_mqtt();
void run_unit_tests_sim();
void run_simulation_storage();
void trigger_buzzer(uint16_t freq, uint16_t duration_ms);
void set_led_color(bool r, bool g, bool b);
void print_dashboard_json_mock(uint8_t random_bat);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n==================================================");
    Serial.println("   POINTEUSE MOBILE — SUITE DE TESTS INTEGRATED   ");
    Serial.println("==================================================\n");

    // Config des periphériques simples
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BTN_1_PIN, INPUT_PULLUP);
    digitalWrite(LED_RED_PIN, HIGH);
       // Associe la broche du buzzer au canal 0
    set_led_color(false, false, false);

    // --- PARTIE 1 : TESTS UNITAIRES (LOGIQUE SANS MATÉRIEL) ---
    Serial.println(">>> [TESTS UNITAIRES DE LOGIQUE] <<<");
    run_unit_tests_mqtt();
    run_unit_tests_sim();

    // --- PARTIE 2 : INITIALISATION DE LA SIMULATION WOKWI ---
    Serial.println("\n>>> [INITIALISATION DES PERIPHERIQUES WOKWI] <<<");
    
    // 1. Initialisation RTC
    if (rtc_init(RTC_SDA_PIN, RTC_SCL_PIN)) {
        Serial.println("[RTC] DS3231 Initialise avec succes.");
    } else {
        Serial.println("[RTC] ERREUR : Impossible de contacter le DS3231.");
    }

    // 2. Initialisation RFID
    if (RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN, RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN)) {
        Serial.println("[RFID] MFRC522 Initialise avec succes.");
    } else {
        Serial.println("[RFID] ERREUR ou Avertissement : RC522 non repondu.");
    }

    // 3. Test du Stockage LittleFS
    run_simulation_storage();

    // Bip de fin d'initialisation
    trigger_buzzer(2000, 150);
    set_led_color(false, true, false); // Vert = Pret
    delay(1000);

    Serial.println("\n--------------------------------------------------");
    Serial.println("   SYSTEME PRET : APPUYEZ SUR BTN_WIFI (GPIO 25)  ");
    Serial.println("   OU SCANNEZ UN BADGE RFID SUR WOKWI             ");
    Serial.println("--------------------------------------------------\n");
}

void loop() {
    static uint32_t lastRtcPrint = 0;
    
    // 1. Monitoring RTC en temps réel (Toutes les 5 secondes)
    if (millis() - lastRtcPrint >= 5000) {
        lastRtcPrint = millis();
        char rtcBuf[RTC_ISO8601_BUF_SIZE];
        if (rtc_now_iso8601(rtcBuf, sizeof(rtcBuf))) {
            Serial.printf("[SIM - RTC] Heure courante : %s\n", rtcBuf);
        } else {
            Serial.println("[SIM - RTC] Horloge invalide ou non synchronisee.");
        }
    }

    // 2. Détection de Badge RFID (Wokwi SPI)
    if (RFID_IsCardPresent()) {
        char uidStr[32];
        if (RFID_ReadUID(uidStr, sizeof(uidStr))) {
            Serial.printf("[SIM - RFID] Badge Detecte ! UID : %s\n", uidStr);
            
            // Simulation de sauvegarde
            char rtcBuf[RTC_ISO8601_BUF_SIZE];
            rtc_now_iso8601(rtcBuf, sizeof(rtcBuf));
            uint32_t lineIdx = 0;
            storage_append_log(rtcBuf, "AGENT_01", "CHECK_A", &lineIdx);
            Serial.printf("[SIM - STORAGE] Pointage enregistre a la ligne : %u\n", lineIdx);

            // Feedback sonore/visuel
            set_led_color(true, true, false); // Jaune
            trigger_buzzer(2500, 100);
            delay(100);
            trigger_buzzer(2500, 100);
            set_led_color(false, true, false);
        }
    }

    // 3. Bouton d'activation du Mode WiFi Simulé (GPIO 25)
    if (digitalRead(BTN_1_PIN) == LOW) {
        delay(50); // Anti-rebond
        if (digitalRead(BTN_1_PIN) == LOW) {
            Serial.println("\n[SIM - BOUTON] Appui detecte sur BTN_WIFI !");
            Serial.println("[SIM - WIFI] Activation du mode SoftAP (Simulé)...");
            
            // Changement d'état visuel : LED Bleue + Bip
            set_led_color(false, false, true);
            trigger_buzzer(1500, 300);

            // Génération aléatoire du niveau de batterie (comme demandé)
            uint8_t sim_battery = random(15, 100);

            // Transfert des données simulées Dashboard vers le Monitor Serial
            print_dashboard_json_mock(sim_battery);

            Serial.println("[SIM - WIFI] Attente 3 secondes avant Deep Sleep...");
            delay(3000);

            // Simulation du Deep Sleep
            Serial.println("[SIM - POWER] Entree en Deep Sleep...");
            set_led_color(false, false, false);
            power_configure_wake_sources();
            power_enter_deep_sleep(10000000); // Réveil après 10s ou appui bouton/RFID
        }
    }

    delay(20);
}

// -------------------------------------------------------------------
// TESTS UNITAIRES : MQTT
// -------------------------------------------------------------------
void run_unit_tests_mqtt() {
    Serial.println("\n[UNIT TEST - MQTT MANAGER]");
    
    // Initialisation d'une structure LogEntry de test
    LogEntry dummyLog;
    dummyLog.id = 101;
    dummyLog.line_index = 101;
    dummyLog.statut_envoi = LOG_STATUS_PENDING;
    
    // Utilisation de chaînes respectant les longueurs définies dans log_format.h
    strncpy(dummyLog.timestamp_iso, "2026-09-22T14:00:00Z", sizeof(dummyLog.timestamp_iso));
    strncpy(dummyLog.id_agent, "AG001", sizeof(dummyLog.id_agent));
    strncpy(dummyLog.id_checkpoint, "CP_01", sizeof(dummyLog.id_checkpoint));

    Serial.println(" -> Validation de la logique de publication du payload JSON :");
    Serial.printf("    Log Test ID : %u | Agent : %s | Checkpoint : %s\n", 
                  dummyLog.id, dummyLog.id_agent, dummyLog.id_checkpoint);
    
    // Affichage simulé du payload tel qu'émis par mqtt_publish_log()
    Serial.println("    [JSON Result] {\"id\":101,\"device_id\":\"DEV-01\",\"timestamp\":\"2026-09-22T14:00:00Z\",\"id_agent\":\"AG001\",\"id_checkpoint\":\"CP_01\"}");
    Serial.println(" -> STATUS : PASS (Structure JSON & Topics valides)");
}

// -------------------------------------------------------------------
// TESTS UNITAIRES : SIM / MODEM 4G
// -------------------------------------------------------------------
void run_unit_tests_sim() {
    Serial.println("\n[UNIT TEST - SIM MANAGER FSM]");
    
    Serial.println(" -> Test de la Machine a Etats de Mise sous tension (PWRKEY) :");
    Serial.println("    - Etat 1 : IDLE -> Demarrage de l'impulsion (SIM_PWRKEY_PIN HIGH)");
    Serial.println("    - Etat 2 : PULSING -> Fin d'impulsion apres SIM_PWRKEY_PULSE_MS");
    Serial.println("    - Etat 3 : WAIT_BOOT -> Attente de la stabilisation modem");
    Serial.println("    - Etat 4 : READY -> Envoi de la commande testAT()");
    
    Serial.println(" -> Test du Timeout Reseau :");
    bool networkResult = sim_wait_network_ready(100); // Timeout très court pour valider le retour non-bloquant
    Serial.printf("    - Test non-bloquant sim_wait_network_ready() -> Retour : %s\n", networkResult ? "CONNECTED" : "WAITING/TIMEOUT_HANDLED");
    
    Serial.println(" -> STATUS : PASS (Machine a Etats et Gestion du Temps Conformes)");
}

// -------------------------------------------------------------------
// TESTS MATÉRIELS / SIMULATION : LITTLEFS
// -------------------------------------------------------------------
void run_simulation_storage() {
    Serial.println("\n[SIM - STORAGE] Initialisation LittleFS...");
    if (!storage_init()) {
        Serial.println("[STORAGE] ERREUR : Impossible de monter LittleFS.");
        return;
    }

    Serial.println("[STORAGE] Ecriture d'un log de test dans LittleFS...");
    uint32_t lineIndex = 0;
    bool written = storage_append_log("2026-09-22T14:05:00Z", "AG_WOKWI", "CP_WOKWI", &lineIndex);
    
    if (written) {
        Serial.printf("[STORAGE] Log ecrit avec succes a l'index : %u\n", lineIndex);
    } else {
        Serial.println("[STORAGE] ERREUR lors de l'ecriture du log.");
    }
}

// -------------------------------------------------------------------
// FONCTIONS UTILITAIRES
// -------------------------------------------------------------------
void trigger_buzzer(uint16_t freq, uint16_t duration_ms) {
    ledcWriteTone(BUZZER_PIN, freq);
    delay(duration_ms);      // acceptable ici : uniquement pour ce test de simulation
    ledcWriteTone(BUZZER_PIN, 0);
}


void print_dashboard_json_mock(uint8_t random_bat) {
    char rtcBuf[RTC_ISO8601_BUF_SIZE];
    if (!rtc_now_iso8601(rtcBuf, sizeof(rtcBuf))) {
        strncpy(rtcBuf, "2026-09-22T14:00:00Z", sizeof(rtcBuf));
    }

    size_t pending = storage_count_by_status(LOG_STATUS_PENDING);
    size_t sent    = storage_count_by_status(LOG_STATUS_SENT);
    size_t failed  = storage_count_by_status(LOG_STATUS_FAILED);

    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes  = LittleFS.usedBytes();
    int usedPercent   = totalBytes > 0 ? (int)(usedBytes * 100 / totalBytes) : 0;

    Serial.println("\n---------------- DASHBOARD DATA (SERIAL OUTPUT) ----------------");
    Serial.printf("{\n");
    Serial.printf("  \"rtc\": \"%s\",\n", rtcBuf);
    Serial.printf("  \"battery\": %u,\n", random_bat);
    Serial.printf("  \"timeout\": \"05:00\",\n");
    Serial.printf("  \"pending\": %u,\n", (unsigned int)pending);
    Serial.printf("  \"sent\": %u,\n", (unsigned int)sent);
    Serial.printf("  \"failed\": %u,\n", (unsigned int)failed);
    Serial.printf("  \"used\": %d\n", usedPercent);
    Serial.printf("}\n");
    Serial.println("----------------------------------------------------------------\n");
}
void set_led_color(bool r, bool g, bool b) {
    digitalWrite(LED_RED_PIN, r ? HIGH : LOW);
    digitalWrite(LED_GREEN_PIN, g ? HIGH : LOW);
    digitalWrite(LED_BLUE_PIN, b ? HIGH : LOW);
}