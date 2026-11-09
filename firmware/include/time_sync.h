#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <stddef.h>
#include <stdint.h>

#include "system_config.h"

// Synchronisation du DS3231 avec l'heure du réseau mobile (via le SIM7600).
//
// Source de l'heure, par ordre de préférence :
//   1. NTP (AT+CNTP) : serveur internet, précis à ~1 s
//   2. heure du réseau mobile (NITZ, AT+CCLK?) si le NTP échoue
//
// Le DS3231 est toujours réglé en UTC ("...Z" dans les timestamps).

// Écart toléré (secondes) : en dessous, le DS3231 n'est pas réécrit.
#ifndef CLOCK_TOLERANCE_S
#define CLOCK_TOLERANCE_S 2
#endif

// Écart maximal accepté (secondes) pour une heure NITZ quand le DS3231 est
// valide : au-delà, l'heure du réseau est jugée suspecte et ignorée.
#ifndef CLOCK_NITZ_MAX_DIFF_S
#define CLOCK_NITZ_MAX_DIFF_S 86400
#endif

// Décalage de l'heure LOCALE par rapport à l'UTC, en minutes.
// Madagascar : UTC+3, sans heure d'été = 180. Redéfinissable dans
// system_config.h. Il ne sert qu'à l'AFFICHAGE : le DS3231 et les logs
// restent en UTC.
#ifndef LOCAL_UTC_OFFSET_MINUTES
#define LOCAL_UTC_OFFSET_MINUTES 180
#endif

// Taille minimale du tampon de time_utc_iso_to_local().
#define TIME_LOCAL_STR_LEN 32

// Convertit "2026-11-07T16:41:10Z" (UTC) en heure locale lisible :
// "2026-11-07 19:41:10 (UTC+3)".
bool time_utc_iso_to_local(const char *utcIso, char *out, size_t outLen);

// Lit l'heure du réseau et recale le DS3231 si nécessaire.
//
// Renvoie true si le DS3231 est à l'heure à la fin (recalé, ou déjà dans la
// tolérance), false si aucune heure fiable n'a pu être obtenue.
//
// BLOQUANTE (jusqu'à ~45 s) et envoie des commandes AT directement au modem :
// à n'appeler que lorsque Internet est connecté et que MQTT n'est PAS actif.
bool time_sync_from_network(void);

#endif