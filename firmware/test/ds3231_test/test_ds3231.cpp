/*
 * Test unitaire du driver rtc_ds3231.cpp / rtc_ds3231.h
 * Cible de test : Arduino Uno (I2C fixe : SDA=A4, SCL=A5)
 * Cible finale du projet : ESP32-WROOM-32U (pins I2C configurables)
 *
 * Cablage DS3231/DS1307 sur Uno :
 *   VCC -> +5V
 *   GND -> GND
 *   SDA -> A4 (+ pull-up 4.7k vers +5V)
 *   SCL -> A5 (+ pull-up 4.7k vers +5V)
 */

#include "./hal/rtc_ds3231.h"
#include <Wire.h>
#include <Arduino.h>

// Sur Uno, Wire.begin() ignore ces valeurs (pins fixes A4/A5).
// Gardees ici uniquement pour respecter la signature commune avec l'ESP32.
#define TEST_SDA_PIN 21
#define TEST_SCL_PIN 20

static void printResult(const char* label, bool ok) {
  Serial.print(label);
  Serial.println(ok ? " -> OK" : " -> ECHEC");
}

void setup() {
  
  Serial.begin(115200); // IMPORTANT : doit correspondre au Baud Rate du
                         // composant Virtual Terminal dans Proteus
  while (!Serial) { ; }

  Serial.println(F("=== Test unitaire DS3231/DS1307 ==="));

  // --- Test 1 : Initialisation / presence I2C (adresse 0x68) ---
  bool initOk = rtc_init(TEST_SDA_PIN, TEST_SCL_PIN);
  printResult("Test 1 - RTC_Init (presence I2C 0x68)", initOk);
  if (!initOk) {
    Serial.println(F("Arret des tests : RTC non detecte sur le bus I2C."));
    return;
  }

  // --- Test 2 : Validite de l'horloge AVANT reglage ---
  bool validBefore = rtc_is_valid();
  Serial.print(F("Test 2 - RTC_IsValid (avant reglage) -> "));
  Serial.println(validBefore ? "valide" : "invalide (normal si jamais reglee)");

  // --- Test 3 : Reglage de l'heure ---
  const char* isoToSet = "2026-08-30T12:00:00Z";
  bool setOk = rtc_sync_from_string(isoToSet);
  printResult("Test 3 - RTC_SetISO8601", setOk);

  // --- Test 4 : Lecture ISO8601 ---
  char buf[RTC_ISO8601_BUF_SIZE];
  bool getOk = rtc_now_iso8601(buf, sizeof(buf));
  Serial.print(F("Test 4 - RTC_GetISO8601 -> "));
  Serial.println(getOk ? buf : "ECHEC (valeur par defaut renvoyee)");

  // --- Test 5 : Lecture Unix Epoch ---
  uint32_t epoch = rtc_now_epoch();
  Serial.print(F("Test 5 - RTC_GetUnixEpoch -> "));
  Serial.println(epoch);

  // --- Test 6 : Validite APRES reglage (doit etre vrai maintenant) ---
  bool validAfter = rtc_is_valid();
  printResult("Test 6 - RTC_IsValid (apres reglage)", validAfter);

  Serial.println(F("=== Fin des tests ==="));

}

void loop() {
  // Lecture continue : verifie visuellement que l'horloge avance chaque seconde
  char buf[RTC_ISO8601_BUF_SIZE];
  if (rtc_now_iso8601(buf, sizeof(buf))) {
    Serial.println(buf);
  } else {
    Serial.println(F("Lecture RTC invalide"));
  }

  // NOTE : delay() utilise ici uniquement pour ce test unitaire de banc.
  // Ne pas reprendre dans le firmware final (regle non-bloquant du
  // projet Fortico) : utiliser millis() en production.
  delay(1000);
}