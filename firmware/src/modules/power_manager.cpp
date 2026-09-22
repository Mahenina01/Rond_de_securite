#include "hal/power_manager.h"
#include "pins_config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
// Seuils de la courbe Li-ion -- approximation lineaire simple (pas une
// courbe de decharge reelle). A affiner si un fuel-gauge dedie est ajoute.
#define BATTERY_EMPTY_MV 3400
#define BATTERY_FULL_MV  4200

namespace
{

  // Masque des broches RTC-GPIO armées pour le réveil ext1.
  uint64_t wakeupPinMask()
  {
    return (1ULL << RFID_IRQ_PIN) | (1ULL << BTN_1_PIN);
  }

} // namespace

void power_configure_wake_sources()
{
  // Activer les résistances de tirage vers le haut (pull-up)
  rtc_gpio_pullup_en((gpio_num_t)RFID_IRQ_PIN);
  rtc_gpio_pullup_en((gpio_num_t)BTN_1_PIN);
  
  // Désactiver les pull-downs pour éviter toute fuite de courant
  rtc_gpio_pulldown_dis((gpio_num_t)RFID_IRQ_PIN);
  rtc_gpio_pulldown_dis((gpio_num_t)BTN_1_PIN);

  // 1. Configuration EXT0 pour le Bouton (déclenchement au niveau 0 = LOW)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_1_PIN, 0);

  // 2. Configuration EXT1 pour le RFID (seule broche dans le masque => ALL_LOW équivaut à ANY_LOW)
  esp_sleep_enable_ext1_wakeup(1ULL << RFID_IRQ_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
}

WakeReason power_get_wake_reason()
{
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  if (cause != ESP_SLEEP_WAKEUP_EXT1)
  {
    return WakeReason::POWER_ON;
  }

  uint64_t wakeMask = esp_sleep_get_ext1_wakeup_status();

  bool rfidTriggered = (wakeMask & (1ULL << RFID_IRQ_PIN)) != 0;
  bool buttonTriggered = (wakeMask & (1ULL << BTN_1_PIN)) != 0;

  if (rfidTriggered)
    return WakeReason::RFID_DETECTED;
  if (buttonTriggered)
    return WakeReason::BUTTON_PRESSED;
  return WakeReason::UNKNOWN;
}

void power_enter_deep_sleep(uint64_t sleepDurationUs)
{
  if (sleepDurationUs > 0)
  {
    esp_sleep_enable_timer_wakeup(sleepDurationUs);
  }

  // Coupe les domaines RTC non necessaires au reveil pour minimiser la
  // consommation de veille (vise le < 10 uA global exige au CDC).
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO);

  Serial.flush(); // laisser sortir les derniers logs avant la coupure
  esp_deep_sleep_start();
  // Rien apres cette ligne : le reset repart depuis setup().
}

bool power_is_long_press(uint8_t pin, uint32_t holdMs)
{
  // Bouton actif bas : appui = niveau LOW.
  if (digitalRead(pin) != LOW)
    return false;

  unsigned long start = millis();
  while (digitalRead(pin) == LOW)
  {
    if (millis() - start >= holdMs)
      return true;
    delay(10);
  }
  return false; // relache avant holdMs -> appui court, ignore
}

void power_set_cpu_frequency(uint32_t mhz)
{
  setCpuFrequencyMhz(mhz);
}



uint16_t power_get_battery_voltage_mv() {
    // analogReadMilliVolts() applique la calibration ADC de l'ESP32,
    // plus fiable qu'un calcul manuel a partir d'analogRead() brut.
    uint32_t adcMv = analogReadMilliVolts(BATTERY_ADC_PIN);
    return (uint16_t)(adcMv * BATTERY_VOLTAGE_DIVIDER_RATIO);
}

uint8_t power_get_battery_percentage() {
  return (uint8_t)random(40, 99);
    /*uint16_t mv = power_get_battery_voltage_mv();

    if (mv <= BATTERY_EMPTY_MV) return 0;
    if (mv >= BATTERY_FULL_MV)  return 100;

    uint32_t range = BATTERY_FULL_MV - BATTERY_EMPTY_MV;
    uint32_t offset = mv - BATTERY_EMPTY_MV;
    return (uint8_t)((offset * 100) / range);*/
}

bool power_is_battery_low() {
    return power_get_battery_voltage_mv() < 3400;
}