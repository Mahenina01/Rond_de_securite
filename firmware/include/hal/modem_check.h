#pragma once

#include <Arduino.h>

#include <esp_task_wdt.h>

// Nourrit le watchdog de la tâche courante. Sans effet si la tâche n'y est
// pas inscrite. À appeler dans toutes les boucles d'attente longues.
inline void modem_feed_watchdog()
{
  esp_task_wdt_reset();
}

// Résultat détaillé de la vérification du modem.
struct ModemStatus
{
  bool alive = false;         // le module répond à "AT"
  bool simReady = false;      // +CPIN: READY
  bool registered = false;    // enregistré sur le réseau (CREG / CGREG / CEREG = 1 ou 5)
  bool pdpActive = false;     // une adresse IP est attribuée (CGPADDR)
  bool clockValid = false;    // horloge du module à jour (année >= 2024)
  bool caCertPresent = false; // certificat CA présent dans la mémoire du module
  int rssi = 99;              // qualité du signal (0..31, 99 = inconnu)
};

// Vérifie que le modem est prêt pour une connexion MQTT/TLS.
bool modem_check_all(HardwareSerial &serial,
                     const char *caFile,
                     ModemStatus &status);

// Affiche un résumé lisible du dernier résultat sur Serial.
void modem_check_print(const ModemStatus &status);