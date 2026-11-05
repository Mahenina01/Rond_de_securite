
#include <Arduino.h>
#include <Wire.h>
#include <esp_sleep.h>
#include "hal/rtc_ds3231.h"
#include "pins_config.h"

// =============================================
// CONFIGURATION MATERIELLE
// =============================================

#define SLEEP_DURATION 10
#define SLEEP_MAGIC 0xD53231

// Date de test à modifier selon tes besoins
#define TEST_TIMESTAMP "2026-11-04T23:15:00"

// =============================================
// VARIABLES PERSISTANTES DEEP SLEEP
// =============================================

// Ces variables sont conservées dans la mémoire RTC
// de l'ESP32 pendant le Deep Sleep.

RTC_DATA_ATTR uint32_t sleepMagic = 0;
RTC_DATA_ATTR uint32_t epochBeforeSleep = 0;
RTC_DATA_ATTR uint32_t sleepDuration = 0;

// =============================================
// OUTILS DE TEST
// =============================================

void printResult(const char *test, bool success)
{

    Serial.print(success ? "[PASS] " : "[FAIL] ");
    Serial.println(test);
}

bool readRegister(uint8_t reg, uint8_t &value)
{

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(reg);

    if (Wire.endTransmission() != 0)
    {
        return false;
    }

    if (Wire.requestFrom(
            (uint8_t)DS3231_I2C_ADDR,
            (uint8_t)1) != 1)
    {
        return false;
    }

    value = Wire.read();

    return true;
}

// =============================================
// TEST 1 : DETECTION I2C
// =============================================

void testI2C()
{

    Serial.println("\n--- TEST I2C ---");

    Wire.beginTransmission(DS3231_I2C_ADDR);

    bool detected = (Wire.endTransmission() == 0);

    printResult("Detection DS3231 (0x68)", detected);
}

// =============================================
// TEST 2 : LECTURE DES REGISTRES
// =============================================

void testRegisters()
{

    Serial.println("\n--- TEST REGISTRES ---");

    for (uint8_t reg = 0x00; reg <= 0x0F; reg++)
    {

        uint8_t value = 0;

        if (readRegister(reg, value))
        {

            Serial.printf(
                "Registre 0x%02X : 0x%02X\n",
                reg,
                value);
        }
        else
        {

            Serial.printf(
                "[FAIL] Lecture registre 0x%02X\n",
                reg);
        }
    }

    Serial.println("\nRegistre CONTROL :");

    uint8_t control;

    if (readRegister(DS3231_REG_CONTROL, control))
    {

        Serial.printf("CONTROL = 0x%02X\n", control);

        Serial.printf(
            "EOSC = %d\n",
            (control >> 7) & 1);

        Serial.printf(
            "BBSQW = %d\n",
            (control >> 6) & 1);

        Serial.printf(
            "INTCN = %d\n",
            (control >> 2) & 1);
    }

    Serial.println("\nRegistre STATUS :");

    uint8_t status;

    if (readRegister(DS3231_REG_STATUS, status))
    {

        Serial.printf("STATUS = 0x%02X\n", status);

        Serial.printf(
            "OSF = %d\n",
            (status & DS3231_OSF_BIT) ? 1 : 0);

        Serial.printf(
            "EN32KHZ = %d\n",
            (status & DS3231_EN32KHZ_BIT) ? 1 : 0);
    }
}

// =============================================
// TEST 3 : VALIDITE DE L'HORLOGE (OSF)
// =============================================

void testOSF()
{

    Serial.println("\n--- TEST OSF ---");

    uint8_t status = 0;

    if (!readRegister(DS3231_REG_STATUS, status))
    {

        printResult("Lecture du registre STATUS", false);
        return;
    }

    bool osf = (status & DS3231_OSF_BIT) != 0;

    Serial.printf("Valeur OSF : %d\n", osf);

    printResult(
        "Oscillateur non arrete",
        !osf);

    bool valid = rtc_is_valid();

    printResult(
        "Fonction rtc_is_valid()",
        valid);

    if (!valid)
    {

        Serial.println(
            "[INFO] Synchronisation necessaire.");
    }
}

// =============================================
// TEST 4 : SYNCHRONISATION
// =============================================

void testSynchronization()
{

    Serial.println("\n--- TEST SYNCHRONISATION ---");

    Serial.print("Date demandee : ");
    Serial.println(TEST_TIMESTAMP);

    bool synchronized = rtc_sync_from_string(
        TEST_TIMESTAMP);

    printResult(
        "Ecriture de la date",
        synchronized);

    if (!synchronized)
    {
        return;
    }

    // Vérification du bit OSF après synchronisation
    uint8_t status = 0;

    if (!readRegister(DS3231_REG_STATUS, status))
    {

        printResult("Lecture STATUS apres sync", false);
        return;
    }

    bool osfCleared = (status & DS3231_OSF_BIT) == 0;

    printResult(
        "Effacement du bit OSF",
        osfCleared);

    // Lecture de l'heure
    char timestamp[RTC_ISO8601_BUF_SIZE];

    bool readSuccess = rtc_now_iso8601(
        timestamp,
        sizeof(timestamp));

    printResult(
        "Lecture ISO8601 apres sync",
        readSuccess);

    if (readSuccess)
    {

        Serial.print("Heure lue : ");
        Serial.println(timestamp);

        char expected[RTC_ISO8601_BUF_SIZE];

        snprintf(
            expected,
            sizeof(expected),
            "%sZ",
            TEST_TIMESTAMP);

        bool sameDate = strcmp(timestamp, expected) == 0;

        printResult(
            "Comparaison date ecrite / date lue",
            sameDate);
    }
}

// =============================================
// TEST 5 : LECTURE DES DONNEES RTC
// =============================================

void testRTCRead()
{

    Serial.println("\n--- TEST LECTURE RTC ---");

    DateTime dt = rtc_now();

    printResult(
        "Fonction rtc_now()",
        dt.valid);

    if (dt.valid)
    {

        Serial.printf(
            "Date : %04u-%02u-%02u\n",
            dt.year,
            dt.month,
            dt.day);

        Serial.printf(
            "Heure : %02u:%02u:%02u\n",
            dt.hour,
            dt.minute,
            dt.second);
    }

    char timestamp[RTC_ISO8601_BUF_SIZE];

    bool isoValid = rtc_now_iso8601(
        timestamp,
        sizeof(timestamp));

    printResult(
        "Format ISO 8601",
        isoValid);

    if (isoValid)
    {

        Serial.print("ISO 8601 : ");
        Serial.println(timestamp);
    }

    uint32_t epoch = rtc_now_epoch();

    printResult(
        "Timestamp Epoch non nul",
        epoch != 0);

    Serial.printf("Epoch : %lu\n", (unsigned long)epoch);
}

// =============================================
// TEST 6 : CONSERVATION APRES DEEP SLEEP
// =============================================

void testDeepSleep()
{

    Serial.println("\n--- TEST DEEP SLEEP ---");

    if (!rtc_is_valid())
    {

        printResult(
            "Horloge valide avant Deep Sleep",
            false);

        Serial.println(
            "[INFO] Synchroniser le RTC avant ce test.");

        return;
    }

    uint32_t epoch = rtc_now_epoch();

    if (epoch == 0)
    {

        printResult(
            "Lecture Epoch avant Deep Sleep",
            false);

        return;
    }

    // Sauvegarde dans la mémoire RTC de l'ESP32
    epochBeforeSleep = epoch;
    sleepDuration = SLEEP_DURATION;
    sleepMagic = SLEEP_MAGIC;

    Serial.printf(
        "Epoch avant sommeil : %lu\n",
        (unsigned long)epochBeforeSleep);

    Serial.printf(
        "Deep Sleep pendant %lu secondes...\n",
        (unsigned long)sleepDuration);

    Serial.flush();

    esp_sleep_enable_timer_wakeup(
        (uint64_t)SLEEP_DURATION * 1000000ULL);

    esp_deep_sleep_start();
}

// =============================================
// TEST 7 : VERIFICATION APRES REVEIL
// =============================================

void testWakeup()
{

    esp_sleep_wakeup_cause_t cause =
        esp_sleep_get_wakeup_cause();

    if (sleepMagic != SLEEP_MAGIC)
    {
        return;
    }

    Serial.println("\n--- TEST REVEIL DEEP SLEEP ---");

    printResult(
        "Reveil par minuterie",
        cause == ESP_SLEEP_WAKEUP_TIMER);

    Serial.printf(
        "Epoch avant : %lu\n",
        (unsigned long)epochBeforeSleep);

    if (!rtc_is_valid())
    {

        printResult(
            "RTC toujours valide apres reveil",
            false);

        sleepMagic = 0;
        return;
    }

    uint32_t epochAfter = rtc_now_epoch();

    Serial.printf(
        "Epoch apres : %lu\n",
        (unsigned long)epochAfter);

    bool validEpoch = epochAfter > epochBeforeSleep;

    printResult(
        "Horloge avance apres sommeil",
        validEpoch);

    if (validEpoch)
    {

        uint32_t difference =
            epochAfter - epochBeforeSleep;

        Serial.printf(
            "Temps ecoule : %lu secondes\n",
            (unsigned long)difference);

        // Tolérance de 3 secondes
        bool timingCorrect =
            difference >= sleepDuration - 3 &&
            difference <= sleepDuration + 3;

        printResult(
            "Precision du temps apres Deep Sleep",
            timingCorrect);
    }

    sleepMagic = 0;

    Serial.println("\nTest Deep Sleep termine.");
}

// =============================================
// MENU INTERACTIF
// =============================================

void printMenu()
{

    Serial.println("\n========== MENU RTC ==========");

    Serial.println("r : Lire les registres");
    Serial.println("v : Verifier OSF");
    Serial.println("s : Synchroniser le RTC");
    Serial.println("t : Tester la lecture");
    Serial.println("d : Tester Deep Sleep");
    Serial.println("h : Afficher ce menu");

    Serial.println("==============================");
}

// =============================================
// INITIALISATION
// =============================================

void setup()
{

    Serial.begin(115200);

    delay(1500);

    Serial.println("\n================================");
    Serial.println("    TEST MATERIEL DS3231");
    Serial.println("================================");

    bool initialized = rtc_init(
        RTC_SDA_PIN,
        RTC_SCL_PIN);

    printResult(
        "Initialisation DS3231",
        initialized);

    if (!initialized)
    {

        Serial.println(
            "Verifier SDA, SCL, VCC et GND.");

        while (true)
        {
            delay(1000);
        }
    }

    // Vérifier si le démarrage provient d'un Deep Sleep
    testWakeup();

    Serial.println("\nEtat actuel du RTC :");

    testOSF();

    printMenu();
}

// =============================================
// BOUCLE PRINCIPALE
// =============================================

void loop()
{

    if (!Serial.available())
    {
        return;
    }

    char command = Serial.read();

    switch (command)
    {

    case 'r':
        testRegisters();
        break;

    case 'v':
        testOSF();
        break;

    case 's':
        testSynchronization();
        break;

    case 't':
        testRTCRead();
        break;

    case 'd':
        testDeepSleep();
        break;

    case 'h':
        printMenu();
        break;

    case '\n':
    case '\r':
        break;

    default:
        Serial.println("Commande inconnue.");
        printMenu();
        break;
    }
}

#include <WiFi.h>
#include <time.h>

// Identifiants WiFi
const char *WIFI_SSID = "TON_WIFI";
const char *WIFI_PASSWORD = "TON_MOT_DE_PASSE";

// Configuration NTP en UTC
bool automaticRTCConfig()
{

    Serial.println("\n=== CONFIGURATION AUTOMATIQUE NTP ===");

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connexion WiFi");

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 20000)
    {

        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED)
    {

        Serial.println("\n[ERREUR] Connexion WiFi impossible.");
        return false;
    }

    Serial.println("\n[OK] WiFi connecte.");

    // Madagascar : UTC+3.
    // Ici on demande volontairement l'heure UTC.
    configTime(0, 0, "pool.ntp.org", "time.google.com");

    struct tm timeinfo;

    if (!getLocalTime(&timeinfo, 15000))
    {

        Serial.println("[ERREUR] Synchronisation NTP impossible.");
        return false;
    }

    char timestamp[25];

    strftime(
        timestamp,
        sizeof(timestamp),
        "%Y-%m-%dT%H:%M:%S",
        &timeinfo);

    Serial.print("Heure NTP UTC : ");
    Serial.println(timestamp);

    bool success = rtc_sync_from_string(timestamp);

    if (success && rtc_is_valid())
    {

        Serial.println("[OK] DS3231 synchronise depuis NTP.");

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);

        return true;
    }

    Serial.println("[ERREUR] Ecriture DS3231 impossible.");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    return false;
}

bool manualRTCConfig()
{

    Serial.println("\n=== CONFIGURATION MANUELLE RTC ===");

    Serial.println("Entrez la date au format :");
    Serial.println("YYYY-MM-DDTHH:MM:SS");

    Serial.print("Date UTC : ");

    while (!Serial.available())
    {
        delay(10);
    }

    String input = Serial.readStringUntil('\n');
    input.trim();

    char timestamp[25];

    input.toCharArray(timestamp, sizeof(timestamp));

    Serial.print("Date saisie : ");
    Serial.println(timestamp);

    if (!rtc_sync_from_string(timestamp))
    {

        Serial.println("[ERREUR] Date invalide.");
        return false;
    }

    if (!rtc_is_valid())
    {

        Serial.println("[ERREUR] RTC non valide apres synchronisation.");
        return false;
    }

    char result[RTC_ISO8601_BUF_SIZE];

    if (!rtc_now_iso8601(result, sizeof(result)))
    {

        Serial.println("[ERREUR] Lecture impossible.");
        return false;
    }

    Serial.print("[OK] RTC synchronise : ");
    Serial.println(result);

    return true;
}
