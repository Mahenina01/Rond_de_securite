#pragma once

#include <stddef.h>
#include <stdint.h>

// Constantes matérielles ds3231 (i2c)
#define DS3231_I2C_ADDR 0x68
#define DS3231_REG_TIME 0x00
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS 0x0F

#define DS3231_OSF_BIT 0x80
#define DS3231_EN32KHZ_BIT 0x08

// Structure legere (POD) representant une date/heure -- aucune allocation
// dynamique, respecte la regle "pas de new/malloc" du projet Fortico.
struct DateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool valid; // false si le RTC a signale une heure invalide (OSF) lors de la lecture
};

// Initialise le bus I2C et verifie la presence du RTC.
// Parametres optionnels : rtc_init() sur Uno (pins fixes A4/A5),
// rtc_init(21, 22) sur ESP32 si le PCB route l'I2C sur d'autres GPIO.
bool rtc_init(int sdaPin = -1, int sclPin = -1);

// Vrai si l'heure conservee par le RTC est fiable (pile de sauvegarde OK).
bool rtc_is_valid(void);

// Lit l'heure courante et la renvoie sous forme de structure DateTime.
DateTime rtc_now(void);

// Lit l'heure RTC en UTC et la formate en ISO 8601 avec suffixe Z.
bool rtc_now_iso8601(char *bufferOut, size_t maxLen);

// Variante bas niveau : lit l'heure et renvoie un timestamp Unix (uint32_t).
uint32_t rtc_now_epoch(void);

// Recale l'horloge a partir d'ISO 8601. Sans fuseau, l'heure saisie est
// interpretee comme Madagascar (UTC+3) ; Z et +/-HH:MM sont aussi acceptes.
bool rtc_sync_from_string(const char *iso8601);

// Prepare le bus I2C avant le passage de l'ESP32 en Deep Sleep.
void rtc_power_down(void);

#define RTC_ISO8601_BUF_SIZE 25