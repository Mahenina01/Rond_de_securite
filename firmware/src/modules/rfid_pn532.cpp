
#include "../include/hal/rfid_pn532.h"

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

namespace
{
    // Instance SPI
    Adafruit_PN532 *pn532 = nullptr;

    // UID memorise
    uint8_t uidBuffer[10];
    uint8_t uidLength = 0;

    bool uidAvailable = false;

    // Interface SPI ESP32
    SPIClass *rfidSPI = &SPI;
}

// =====================================================
// INITIALISATION PN532
// =====================================================

bool RFID_Init(uint8_t ssPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
{
    // Configuration SPI
    rfidSPI->begin(
        sckPin,
        misoPin,
        mosiPin,
        ssPin);

    // Creation du PN532
    static Adafruit_PN532 instance(ssPin, rfidSPI);

    pn532 = &instance;

    // Initialisation du module
    pn532->begin();

    // Lecture de la version firmware
    uint32_t version = pn532->getFirmwareVersion();

    if (!version)
    {
        Serial.println("[PN532] Module non detecte !");
        return false;
    }

    Serial.println("[PN532] Module detecte.");

    Serial.printf(
        "[PN532] Firmware : %u.%u\n",
        (version >> 16) & 0xFF,
        (version >> 8) & 0xFF);

    // Configuration du lecteur
    pn532->SAMConfig();

    uidAvailable = false;

    return true;
}

// =====================================================
// DETECTION BADGE
// =====================================================

bool RFID_IsCardPresent(void)
{
    if (pn532 == nullptr)
        return false;

    if (uidAvailable)
        return true;

    uint8_t tempUID[sizeof(uidBuffer)];
    uint8_t tempUIDLength = 0;

    // Detection et lecture UID ISO14443A
    bool success = pn532->readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        tempUID,
        &tempUIDLength,
        100);

    if (!success)
        return false;

    if (tempUIDLength == 0 || tempUIDLength > sizeof(uidBuffer))
        return false;

    // Memorisation de l'UID
    memcpy(uidBuffer, tempUID, tempUIDLength);

    uidLength = tempUIDLength;
    uidAvailable = true;

    return true;
}

// =====================================================
// LECTURE UID
// =====================================================

bool RFID_ReadUID(char *uidStrOut, size_t maxLen)
{
    if (pn532 == nullptr)
        return false;

    if (uidStrOut == nullptr)
        return false;

    if (!uidAvailable)
        return false;

    // 2 caracteres HEX par octet + '\0'
    size_t requiredLength = (uidLength * 2) + 1;

    if (maxLen < requiredLength)
        return false;

    static const char hexDigits[] = "0123456789ABCDEF";

    for (uint8_t i = 0; i < uidLength; i++)
    {
        uidStrOut[i * 2] =
            hexDigits[(uidBuffer[i] >> 4) & 0x0F];

        uidStrOut[i * 2 + 1] =
            hexDigits[uidBuffer[i] & 0x0F];
    }

    uidStrOut[uidLength * 2] = '\0';

    // UID consomme
    uidAvailable = false;

    return true;
}

// =====================================================
// FONCTIONS DE GESTION ENERGIE
// =====================================================

// Le PN532 possede son propre protocole de gestion
// d'energie. Ces fonctions doivent etre adaptees
// selon le mode PowerDown choisi.

void RFID_SetupWakeupIRQ(void)
{
    Serial.println(
        "[PN532] Wakeup IRQ non configure dans ce driver.");
}

void RFID_PowerDown(void)
{
    Serial.println(
        "[PN532] PowerDown non configure dans ce driver.");
}

void RFID_WakeUp(void)
{
    if (pn532 == nullptr)
        return;

    pn532->begin();
    pn532->SAMConfig();

    Serial.println("[PN532] WakeUp effectue.");
}
