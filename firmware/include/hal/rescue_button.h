#ifndef RESCUE_BUTTON_H
#define RESCUE_BUTTON_H

#include <stdbool.h>

// Bouton du mode secours
#ifndef RESCUE_BUTTON_PIN
#define RESCUE_BUTTON_PIN 25
#endif

// Niveau de la broche quand le bouton est enfoncé
// LOW  : bouton entre la broche et GND
// HIGH : bouton entre la broche et 3,3 V
#ifndef RESCUE_BUTTON_ACTIVE_LEVEL
#define RESCUE_BUTTON_ACTIVE_LEVEL LOW
#endif

// Durée d'appui pour déclencher le mode secours : RESCUE_BUTTON_HOLD_MS
void rescue_button_init(void);

// À appeler à chaque tour de loop(). Renvoie true UNE SEULE FOIS quand le
// bouton vient d'être maintenu RESCUE_BUTTON_HOLD_MS ; il faut le relâcher
// avant de pouvoir recommencer.
bool rescue_button_long_press(void);

#endif