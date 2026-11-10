#include "hal/battery.h"

#include <Arduino.h>

#include "pins_config.h"

// Facteur de correction si la tension affichée diffère d'un multimètre
#ifndef BATTERY_CALIBRATION
#define BATTERY_CALIBRATION 1.0f
#endif

namespace
{

  struct CurvePoint
  {
    uint16_t mv;
    uint8_t percent;
  };

  // Courbe de décharge typique d'une cellule Li-ion / LiPo.
  const CurvePoint CURVE[] = {
      {4200, 100}, {4150, 95}, {4110, 90}, {4080, 85}, {4020, 80}, {3980, 75}, {3950, 70}, {3910, 65}, {3870, 60}, {3850, 55}, {3840, 50}, {3820, 45}, {3800, 40}, {3790, 35}, {3770, 30}, {3750, 25}, {3730, 20}, {3710, 15}, {3690, 10}, {3610, 5}, {3270, 0}};

  const size_t CURVE_SIZE = sizeof(CURVE) / sizeof(CURVE[0]);

} // namespace

uint32_t battery_read_mv(void)
{
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);

  uint32_t sum = 0;

  for (int i = 0; i < 16; i++)
  {
    sum += analogReadMilliVolts(BATTERY_ADC_PIN);
    delayMicroseconds(200);
  }

  float adcMv = sum / 16.0f;

  return static_cast<uint32_t>(adcMv * BATTERY_VOLTAGE_DIVIDER_RATIO * BATTERY_CALIBRATION + 0.5f);
}

uint8_t battery_percent_from_mv(uint32_t mv)
{
  if (mv >= CURVE[0].mv)
    return 100;

  if (mv <= CURVE[CURVE_SIZE - 1].mv)
    return 0;

  for (size_t i = 1; i < CURVE_SIZE; i++)
  {
    if (mv >= CURVE[i].mv)
    {
      uint32_t spanMv = CURVE[i - 1].mv - CURVE[i].mv;
      uint32_t spanPct = CURVE[i - 1].percent - CURVE[i].percent;

      return static_cast<uint8_t>(CURVE[i].percent + ((mv - CURVE[i].mv) * spanPct + spanMv / 2) / spanMv);
    }
  }

  return 0;
}