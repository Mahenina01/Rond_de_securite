#include "hal/rescue_button.h"

#include <Arduino.h>

#include "system_config.h"

#ifndef RESCUE_BUTTON_HOLD_MS
#define RESCUE_BUTTON_HOLD_MS 5000
#endif

namespace
{

  bool pressing = false;
  bool fired = false;
  bool ignoreUntilRelease = false;
  uint8_t releasedSamples = 0;
  uint32_t pressStartMs = 0;

  bool isPressed()
  {
    return digitalRead(RESCUE_BUTTON_PIN) == RESCUE_BUTTON_ACTIVE_LEVEL;
  }

} // namespace

void rescue_button_init(void)
{
  pinMode(RESCUE_BUTTON_PIN, RESCUE_BUTTON_ACTIVE_LEVEL == LOW ? INPUT_PULLUP : INPUT_PULLDOWN);

  pressing = false;
  fired = false;
  releasedSamples = 0;

  // Bouton déjà enfoncé au démarrage (coincé ?) : on attend son relâchement.
  ignoreUntilRelease = isPressed();
}

bool rescue_button_long_press(void)
{
  bool pressed = isPressed();

  if (ignoreUntilRelease)
  {
    if (!pressed)
      ignoreUntilRelease = false;

    return false;
  }

  if (pressed)
  {
    releasedSamples = 0;

    if (!pressing)
    {
      pressing = true;
      fired = false;
      pressStartMs = millis();

      return false;
    }

    if (!fired && millis() - pressStartMs >= RESCUE_BUTTON_HOLD_MS)
    {
      fired = true;

      return true;
    }

    return false;
  }

  // Relâché : on exige 2 mesures consécutives pour ignorer un rebond.
  if (pressing && ++releasedSamples >= 2)
  {
    pressing = false;
    fired = false;
    releasedSamples = 0;
  }

  return false;
}