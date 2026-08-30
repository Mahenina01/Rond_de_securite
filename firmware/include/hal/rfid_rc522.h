#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <MFRC522.h>

/// Taille max de l'UID en hexadécimal + '\0'
#define RFID_UID_MAX_STR_LEN 21

/**
 * @brief Initialise le bus SPI et le RC522.
 * @param irqPin Pull-up externe 10 kΩ obligatoire (GPIO34 = pas de pull-up interne)
 * @return true si le module répond correctement.
 */
bool RFID_Init(uint8_t ssPin, uint8_t rstPin, uint8_t irqPin,
               uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);

/// @return true si un badge est dans le champ RF.
bool RFID_IsCardPresent(void);

/**
 * @brief Lit l'UID du badge et le convertit en chaîne hexadécimale
 * @param maxLen Doit être >= RFID_UID_MAX_STR_LEN, sinon échec sans écriture
 * @return true si l'UID a été lu (indépendamment du succès du HALT interne)
 */
bool RFID_ReadUID(char *uidStrOut, size_t maxLen);

/// Configure l'IRQ du RC522 pour réveiller l'ESP32 au passage d'un badge
void RFID_SetupWakeupIRQ(void);

/// Coupe l'antenne RF et passe le RC522 en Soft Power Down
void RFID_PowerDown(void);

/// Réveille le RC522 depuis le Soft Power Down
void RFID_WakeUp(void);