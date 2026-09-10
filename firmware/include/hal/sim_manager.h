#pragma once
// sim_manager.h
// Gestion du module 4G SIM7600E-H : alimentation et etat reseau.
// Toutes les fonctions sont NON-BLOQUANTES : a rappeler a chaque tour de loop()
// jusqu'a ce qu'elles renvoient true (succes) ou soient explicitement redemarrees.

#include <stdint.h>
#include <TinyGsmClient.h>

// Sequence de mise sous tension du modem (impulsion PWRKEY + attente boot).
// Non-bloquant : renvoie false tant que la sequence n'est pas terminee,
// true une fois le modem pret a repondre aux commandes AT.
bool sim_power_on();

bool sim_power_off();

// Non-bloquant : renvoie true des que le reseau est pret, false tant qu'on
// attend ou si le delai timeout_ms est depasse (abandon interne automatique,
// reinitialise par le prochain appel a sim_power_on()).
bool sim_wait_network_ready(uint32_t timeout_ms);

// Accesseur necessaire pour que mqtt_manager construise son client TLS
// sur le MEME modem (une seule instance UART/TinyGsm dans tout le firmware).
// AJOUT propose par rapport a l'interface d'origine -- a valider avec le Lead.
TinyGsm &sim_get_modem();