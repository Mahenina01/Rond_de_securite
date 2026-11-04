
#include <Arduino.h>

#include "hal/rfid_rc522.h"
#include "pins_config.h"

// =====================================================
// TEST AUTONOME PN532 SPI - ESP32
// =====================================================

bool rfidReady = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("======================================");
    Serial.println("       TEST RFID PN532 - ESP32");
    Serial.println("======================================");

    // Initialisation PN532
    Serial.println("\n[PN532] Initialisation SPI...");

    rfidReady = RFID_Init(
        RFID_CS_PIN,
        RFID_SCK_PIN,
        RFID_MISO_PIN,
        RFID_MOSI_PIN
    );

    if (rfidReady)
    {
        Serial.println("[PN532] Initialisation REUSSIE !");
        Serial.println("[PN532] SPI communication OK.");
        Serial.println("[PN532] Lecteur pret.");
    }
    else
    {
        Serial.println("[PN532] ERREUR : Module non detecte !");
        Serial.println("[PN532] Verifier les switches SPI.");
        Serial.println("[PN532] Verifier le cablage.");
    }

    Serial.println("\n--------------------------------------");
    Serial.println("Approchez un badge RFID...");
    Serial.println("--------------------------------------");
}

void loop()
{
    // Si le module n'est pas initialise
    if (!rfidReady)
    {
        delay(1000);
        return;
    }

    // Detection du badge
    if (RFID_IsCardPresent())
    {
        Serial.println("\n[PN532] Badge detecte !");

        char uidStr[RFID_UID_MAX_STR_LEN];

        // Lecture UID
        if (RFID_ReadUID(uidStr, sizeof(uidStr)))
        {
            Serial.print("[PN532] UID : ");
            Serial.println(uidStr);

            Serial.println("[PN532] TEST : PASS");
        }
        else
        {
            Serial.println("[PN532] Lecture UID echouee !");
            Serial.println("[PN532] TEST : FAIL");
        }

        Serial.println("--------------------------------------");

        delay(1000);
    }

    delay(20);
}
