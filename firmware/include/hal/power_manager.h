#pragma once

#include <stdint.h>
#include <esp_sleep.h>

/// Raison du réveil, traduite pour la state machine (app.cpp).
enum class WakeReason
{
  POWER_ON,       ///< Reset complet.
  RFID_DETECTED,  ///< Réveil via IRQ RC522.
  BUTTON_PRESSED, ///< Réveil via bouton.
  UNKNOWN,
};

/// Arme les sources de réveil (IRQ RFID + boutons).
/// @warning A appeler juste avant power_enter_deep_sleep(), jamais avant.
void power_configure_wake_sources();

/// Retourne la raison du réveil courant.
WakeReason power_get_wake_reason();

/// Entre en deep sleep. sleepDurationUs = 0 -> réveil uniquement par IRQ/bouton.
void power_enter_deep_sleep(uint64_t sleepDurationUs = 0);

/// Détecte un appui d'au moins holdMs (bloquant). Bouton actif bas.
bool power_is_long_press(uint8_t pin, uint32_t holdMs);

/// Ajuste la fréquence CPU (80 MHz veille active, 240 MHz envoi 4G/MQTT).
void power_set_cpu_frequency(uint32_t mhz);