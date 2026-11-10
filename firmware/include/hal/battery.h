#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

// Tension de la batterie en millivolts (moyenne de 16 mesures sur
// BATTERY_ADC_PIN, avec le pont diviseur BATTERY_VOLTAGE_DIVIDER_RATIO).
uint32_t battery_read_mv(void);

// Niveau estimé en % pour une batterie Li-ion / LiPo 1S
uint8_t battery_percent_from_mv(uint32_t mv);

#endif