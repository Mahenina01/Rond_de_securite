#ifndef BADGE_MAP_H
#define BADGE_MAP_H

#include <stddef.h>
#include <string.h>

#include "badge_store.h"
#include "log_format.h" // LOG_FIELD_AGENT_LEN

// Identifiant du point de contrôle où ce boîtier est installé
// (14 caractères, comme "CHECKPOINT_001"). À redéfinir dans system_config.h.
#ifndef CHECKPOINT_ID
#define CHECKPOINT_ID "CHECKPOINT_001"
#endif

// Un badge inconnu est-il enregistré dans les logs ?
//   0 (défaut) : accès refusé, rien n'est enregistré, bip long.
//   1          : le passage est enregistré avec les 5 derniers caractères de
//                l'UID comme identifiant d'agent (pour repérer le badge).
#ifndef LOG_UNKNOWN_BADGES
#define LOG_UNKNOWN_BADGES 0
#endif

// Renvoie true si le badge est dans la liste des badges autorisés (gérée
// depuis le tableau de bord) et copie alors l'identifiant de l'agent dans
// `out` (LOG_FIELD_AGENT_LEN + 1 octets).
//
// Badge inconnu : renvoie false et met dans `out` les 5 derniers caractères
// de l'UID (utilisés seulement si LOG_UNKNOWN_BADGES vaut 1).
inline bool badge_agent_id(const char *uid, char *out)
{
  if (badges_find(uid, out))
    return true;

  size_t len = strlen(uid);
  const char *tail = (len > LOG_FIELD_AGENT_LEN) ? uid + (len - LOG_FIELD_AGENT_LEN) : uid;

  strncpy(out, tail, LOG_FIELD_AGENT_LEN);
  out[LOG_FIELD_AGENT_LEN] = '\0';

  return false;
}

#endif