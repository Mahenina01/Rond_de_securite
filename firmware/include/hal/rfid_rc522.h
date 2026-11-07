
#ifndef RFID_PN532_H
#define RFID_PN532_H

#include <stddef.h>
#include <stdint.h>

// UID PN532 jusqu'a 10 octets, converti en hexadecimal majuscule + '\0'.
#define RFID_UID_MAX_STR_LEN 21

// Initialisation du PN532 en SPI (CS, SCK, MISO et MOSI).
bool RFID_Init(uint8_t ssPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);

bool RFID_IsCardPresent(void);

bool RFID_ReadUID(
    char *uidStrOut,
    size_t maxLen
);

void RFID_SetupWakeupIRQ(void);

void RFID_PowerDown(void);

void RFID_WakeUp(void);

#endif
