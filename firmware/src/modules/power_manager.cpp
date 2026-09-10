#include "hal/power_manager.h"
#include "pins_config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

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
  // Assure que les broches ne sont pas isolées d'un precedent deep sleep
  // (l'ESP32 isole les RTC-GPIO par defaut pendant le sommeil).
  rtc_gpio_pullup_en((gpio_num_t)RFID_IRQ_PIN);
  rtc_gpio_pullup_en((gpio_num_t)BTN_1_PIN);

  // ext1 : reveil si N'IMPORTE LAQUELLE des broches du masque passe a LOW.
  // Necessite un coeur ESP-IDF >= 5.0 (Arduino core >= 3.x) pour ANY_LOW ;
  // sur un core plus ancien, utiliser ESP_EXT1_WAKEUP_ALL_LOW degraderait
  // le comportement (reveil seulement si les DEUX passent bas en meme temps).
  esp_sleep_enable_ext1_wakeup(wakeupPinMask(), ESP_EXT1_WAKEUP_ANY_LOW);
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