#include "./hal/rtc_ds3231.h"
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//Constantes matérielles ds3231 (i2c)
#define DS3231_I2C_ADDR 0x68
#define DS3231_REG_TIME 0x00//Registre de depart horloge (seconde)
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS 0x00//pour ds1307 mais 0x0F pour ds3231

#define DS3231_OSF_BIT 0x80
#define DS3231_EN32KHZ_BIT 0x08

//fonction en convertissant un octé en codé BCD en entienr Décimal
static inline uint8_t bcdToDec(uint8_t val){
    return (uint8_t)(((val>>4)*10)+(val & 0x0F));
}
//en convertissant dec en BCD
static inline uint8_t decToBcd(uint8_t val){
    return (uint8_t)(((val/10)<<4)|(val % 10));
}
bool rtc_init(int sdaPin, int sclPin) {
    (void)sdaPin;                 // Uno/AVR : pins fixes (A4=SDA, A5=SCL)
    (void)sclPin;                 // évite les warnings "unused parameter"
    Wire.begin();
    Wire.setClock(100000);

    Wire.beginTransmission(DS3231_I2C_ADDR);
    return (Wire.endTransmission() == 0);
}
//implémentation de l'API publique
/*bool RTC_Init(int sdaPin, int sclPin){
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(100000);// horloge i2c standard I2C 100 kHz

    //teste de presence du composant sur le bus i2c
    Wire.beginTransmission(DS3231_I2C_ADDR);
    if(Wire.endTransmission()!=0){
        return false;
    }

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)DS3231_I2C_ADDR, (uint8_t)1);
    if(Wire.available()){
        uint8_t status = Wire.read();
        status &= ~DS3231_EN32KHZ_BIT;

        Wire.beginTransmission(DS3231_I2C_ADDR);
        Wire.write(DS3231_REG_STATUS);
        Wire.write(status);
        Wire.endTransmission();
    }
    return true;
}*/

 bool rtc_is_valid(void){
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    if(Wire.endTransmission()!=0){
        return false;
    }

    Wire.requestFrom((uint8_t)DS3231_I2C_ADDR,(uint8_t)1);
    if(!Wire.available()){
        return false;
    }
    uint8_t status = Wire.read();
    return ((status & DS3231_EN32KHZ_BIT)==0);
 }

 bool rtc_now_iso8601(char* buffertOut, size_t maxLen){
    //protection contre les pointeurs nulls et tailles insuffisantes
    if(buffertOut == NULL || maxLen < RTC_ISO8601_BUF_SIZE){
        return false;
    }
    //verification de l'intégrité de la pile RTC
    if(!rtc_is_valid()){
        snprintf(buffertOut, maxLen, "1970-01-01T00:00:00Z");
        return false;
    }
    //lecture atomique des 7 registres d'horloge en seul transaction
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_TIME);
    if(Wire.endTransmission() != 0){
        snprintf(buffertOut, maxLen, "1970-01-01T00:00:00Z");
        return false;
    }
    //Demande de 7 octes (secondes, Munites, Heures, ...)
    Wire.requestFrom((uint8_t)DS3231_I2C_ADDR,(uint8_t)7); 
    if(Wire.available()<7){
        snprintf(buffertOut, maxLen, "1970-01-01T00:00:00Z");
        return false;
    }    
    //Extraction des registres BCD + Masquage des bits de contrôle
    uint8_t sec = bcdToDec(Wire.read() & 0x7F);
    uint8_t min= bcdToDec(Wire.read() & 0x7F);
    uint8_t hour= bcdToDec(Wire.read() & 0x3F);
    Wire.read();
    uint8_t day = bcdToDec(Wire.read() & 0x3F);
    uint8_t mon= bcdToDec(Wire.read() & 0x1F);
    uint16_t year = 2000+ bcdToDec(Wire.read());

    //Formatage direct dans le buffers C statique (ISO 8601 UTC)
    snprintf(buffertOut, maxLen, "%04u-%02u-%02uT%02u:%02u:%02uZ", year, mon, day, hour, min, sec);
    return true;
 }

 uint32_t rtc_now_epoch(void){
    if(!rtc_is_valid()){
        return 0;
    }
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_TIME);
    if(Wire.endTransmission() !=0){
        return 0;
    }
    Wire.requestFrom((uint8_t)DS3231_I2C_ADDR, (uint8_t)7);
    if(Wire.available()<7) return 0;

    struct tm timeinfo ={0};
    timeinfo.tm_sec=bcdToDec(Wire.read() & 0X7F);
    timeinfo.tm_min = bcdToDec(Wire.read() & 0x7F);
    timeinfo.tm_hour = bcdToDec(Wire.read() & 0x3F);
    Wire.read();
    timeinfo.tm_mday =bcdToDec(Wire.read() & 0x3F);
    timeinfo.tm_mon = bcdToDec(Wire.read() & 0x1F) -1;
    timeinfo.tm_year = (2000 + bcdToDec(Wire.read()))-1900;

    time_t epoch = mktime(&timeinfo);
    return (epoch < 0) ? 0 : (uint32_t)epoch;
 }
 bool rtc_sync_from_string(const char* isoTimestamp){
    if (isoTimestamp == NULL) return false;
    int year,month, day, hour, min, sec;

        if(sscanf(isoTimestamp, "%4d-%2d-%2dT%2d:%2d:%2d", 
                &year, &month, &day, &hour, &min, &sec)!= 6){
                    return false;
        }
    if (year< 2026 || month <1 || month > 12 || day < 1 || day>31 || hour>23 || min > 59 || sec > 59){
        return false;
    }
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_TIME);
    Wire.write(decToBcd((uint8_t)sec)& 0x7F);//pour ds3231//Wire.write(decToBcd((uint8_t)sec));
    Wire.write(decToBcd((uint8_t)min));
    Wire.write(decToBcd((uint8_t)hour));
    Wire.write(0x01); 
    Wire.write(decToBcd((uint8_t)day));
    Wire.write(decToBcd((uint8_t)month));
    Wire.write(decToBcd((uint8_t)(year % 100)));
    if (Wire.endTransmission() != 0) {
        return false;
    }
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_REG_STATUS);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)DS3231_I2C_ADDR, (uint8_t)1);
    if (Wire.available()) {
        uint8_t status = Wire.read();
        status &= ~DS3231_OSF_BIT; // Forcer le Bit 7 (OSF) à 0 (Heure Valide)
        
        Wire.beginTransmission(DS3231_I2C_ADDR);
        Wire.write(DS3231_REG_STATUS);
        Wire.write(status);
        Wire.endTransmission();
    }

    return true;

 }

 void rtc_power_down(void) {
    // Libération minimale du bus I2C avant le Deep Sleep de l'ESP32
    Wire.endTransmission();
}
DateTime rtc_now(void) {
    DateTime dt = {0, 0, 0, 0, 0, 0, false};
 
    char buf[RTC_ISO8601_BUF_SIZE];
    if (!rtc_now_iso8601(buf, sizeof(buf))) {
        dt.valid = false;   // heure par defaut (1970-01-01) deja ecrite par le driver dans buf
        return dt;
    }
 
    int y, mo, d, h, mi, s;
    // reutilise le format ISO 8601 deja produit et teste par le driver bas niveau
    if (sscanf(buf, "%4d-%2d-%2dT%2d:%2d:%2dZ", &y, &mo, &d, &h, &mi, &s) != 6) {
        dt.valid = false;
        return dt;
    }
 
    dt.year   = (uint16_t)y;
    dt.month  = (uint8_t)mo;
    dt.day    = (uint8_t)d;
    dt.hour   = (uint8_t)h;
    dt.minute = (uint8_t)mi;
    dt.second = (uint8_t)s;
    dt.valid  = true;
    return dt;
}