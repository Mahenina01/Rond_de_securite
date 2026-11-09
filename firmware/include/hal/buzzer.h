#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include <stdint.h>

// Broche du buzzer : GPIO26, d'après votre câblage.
// ATTENTION : pins_config.h déclare BUZZER_PIN = 26 et BTN_1_PIN = 25.
// Si le buzzer est en réalité sur la broche 26, remplacez par BUZZER_PIN ;
// si vous utilisez le bouton SW2 (GPIO25), il y aura un conflit.
#ifndef SCAN_BUZZER_PIN
#define SCAN_BUZZER_PIN 26
#endif

// 0 = buzzer ACTIF : il fabrique lui-même sa tonalité, on lui donne juste
//     du courant (cas le plus courant des modules "buzzer").
// 1 = buzzer PASSIF : il faut lui envoyer un signal carré (BUZZER_FREQ_HZ).
// Symptôme d'un mauvais réglage : un simple "clic" au lieu d'un son.
#ifndef BUZZER_PASSIVE
#define BUZZER_PASSIVE 0
#endif

// Niveau de la broche qui fait sonner le buzzer.
#ifndef BUZZER_ACTIVE_LEVEL
#define BUZZER_ACTIVE_LEVEL HIGH
#endif

#ifndef BUZZER_BEEP_MS
#define BUZZER_BEEP_MS 80 // durée d'un "pip"
#endif

#ifndef BUZZER_GAP_MS
#define BUZZER_GAP_MS 80 // silence entre deux "pip"
#endif

#ifndef BUZZER_ERROR_MS
#define BUZZER_ERROR_MS 500 // durée du bip long (passage refusé)
#endif

#ifndef BUZZER_FREQ_HZ
#define BUZZER_FREQ_HZ 2700 // fréquence du buzzer passif
#endif

// Configure la broche (buzzer éteint).
void buzzer_init(void);

// "pip! pip!" : passage valide.
void buzzer_valid(void);

// Un bip long : passage refusé ou non enregistré.
void buzzer_error(void);

// Un bip de `ms` millisecondes (bloquant).
void buzzer_beep(uint16_t ms);

#endif