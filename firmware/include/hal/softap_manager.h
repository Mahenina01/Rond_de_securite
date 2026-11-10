#pragma once

#include <stdbool.h>
#include <stdint.h>

// Mode secours : point d'accès Wi-Fi + tableau de bord web.
struct SoftApStatus
{
  bool internetUp = false;    // le modem a une connexion Internet
  bool mqttConnected = false; // connecté au serveur HiveMQ
  // L'état réseau, le tableau de bord le traduit :
  // "modem_start", "modem_init", "network", "internet", "time",
  // "mqtt_init", "mqtt_connect", "running", "error".
  const char *state = "modem_start";
};

// Démarre le point d'accès (RESCUE_SSID / AP_WIFI_PASSWORD) et le serveur.
void softap_start();

// À appeler souvent depuis loop() tant que le mode secours est actif.
void softap_loop();

// Arrête le serveur et le Wi-Fi, invalide la session.
void softap_stop();

bool softap_is_active();

// true quand plus personne n'utilise l'interface depuis RESCUE_AUTO_OFF_MS
bool softap_expired();

void softap_update_status(const SoftApStatus &status);

// true UNE fois quand l'utilisateur a demandé "Synchroniser maintenant".
bool softap_take_time_sync_request();

// main.cpp annonce le résultat de la synchronisation par le réseau mobile.
void softap_report_time_sync(bool ok);

// Change le mot de passe du tableau de bord (8 à 64 caractères).
bool softap_set_password_hash(const char *passwordPlain);