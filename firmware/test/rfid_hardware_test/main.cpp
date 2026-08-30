// test/deep_sleep_wake_test/main.cpp
#include <Arduino.h>
#include "pins_config.h"
#include "../include/hal/rfid_rc522.h"

#define SCAN_TIMEOUT_MS 5000 // durée d'attente d'un badge avant sommeil

RTC_DATA_ATTR int bootCount = 0; // survit au deep sleep, remis à 0 au reset complet

void printWakeupReason()
{
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  switch (reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Réveil : badge détecté (IRQ RC522)");
    break;
  case ESP_SLEEP_WAKEUP_UNDEFINED:
  default:
    Serial.println("Démarrage normal (pas un réveil de deep sleep)");
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  bootCount++;
  Serial.printf("--- Boot #%d ---\n", bootCount);
  printWakeupReason();

  bool ok = RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN,
                      RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);
  if (!ok)
  {
    Serial.println("RFID_Init a échoué, arrêt du test.");
    while (true)
      delay(1000);
  }

  // Fenêtre d'attente active : si un badge est déjà présent, pas besoin de dormir
  Serial.println("Attente d'un badge (5s) avant mise en veille...");
  unsigned long start = millis();
  while (millis() - start < SCAN_TIMEOUT_MS)
  {
    if (RFID_IsCardPresent())
    {
      char uid[RFID_UID_MAX_STR_LEN];
      if (RFID_ReadUID(uid, sizeof(uid)))
      {
        Serial.print("Badge détecté avant sommeil, UID = ");
        Serial.println(uid);
      }
      return; // pas de deep sleep ce cycle-ci
    }
    delay(100);
  }

  // Aucun badge détecté dans le délai -> préparation du deep sleep
  Serial.println("Aucun badge détecté. Armement de l'IRQ et passage en deep sleep...");

  RFID_SetupWakeupIRQ();

  // GPIO34 est une broche RTC valide pour ext0. IRQ du RC522 = actif bas (pull-up externe).
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RFID_IRQ_PIN, 0); // 0 = réveil sur niveau BAS

  Serial.flush(); // s'assurer que le message est bien envoyé avant coupure
  esp_deep_sleep_start();
}

void loop()
{
  // Jamais exécuté : setup() gère tout le cycle
}