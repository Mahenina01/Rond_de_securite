#include "hal/buzzer.h"

#include <Arduino.h>

namespace
{

  void setBuzzer(bool on)
  {
    int inactive = (BUZZER_ACTIVE_LEVEL == HIGH) ? LOW : HIGH;

    digitalWrite(SCAN_BUZZER_PIN, on ? BUZZER_ACTIVE_LEVEL : inactive);
  }

} // namespace

void buzzer_init(void)
{
  pinMode(SCAN_BUZZER_PIN, OUTPUT);

  setBuzzer(false);
}

void buzzer_beep(uint16_t ms)
{
#if BUZZER_PASSIVE

  // Signal carré à BUZZER_FREQ_HZ pendant `ms` millisecondes.
  const uint32_t halfPeriodUs = 500000UL / BUZZER_FREQ_HZ;

  uint32_t start = millis();

  while (millis() - start < ms)
  {
    setBuzzer(true);
    delayMicroseconds(halfPeriodUs);

    setBuzzer(false);
    delayMicroseconds(halfPeriodUs);
  }

#else

  setBuzzer(true);
  delay(ms);
  setBuzzer(false);

#endif
}

void buzzer_valid(void)
{
  buzzer_beep(BUZZER_BEEP_MS);
  delay(BUZZER_GAP_MS);
  buzzer_beep(BUZZER_BEEP_MS);
}

void buzzer_error(void)
{
  buzzer_beep(BUZZER_ERROR_MS);
}