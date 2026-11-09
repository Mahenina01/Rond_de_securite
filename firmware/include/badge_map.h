#ifndef BADGE_MAP_H
#define BADGE_MAP_H

#include <stddef.h>
#include <string.h>

#include "log_format.h" // LOG_FIELD_AGENT_LEN

// Identifiant du point de contrôle où ce boîtier est installé
// (14 caractères, comme "CHECKPOINT_001"). À redéfinir dans system_config.h.
#ifndef CHECKPOINT_ID
#define CHECKPOINT_ID "CHECKPOINT_001"
#endif

// Table des badges : UID en hexadécimal MAJUSCULE (tel qu'affiché par le
// boîtier dans la ligne "[RFID] UID lu : ...") -> identifiant de l'agent
// (5 caractères au maximum, imposé par le format des logs).
//
// À REMPLACER par vos vrais badges.
struct BadgeEntry
{
  const char *uid;
  const char *agentId;
};

static const BadgeEntry BADGES[] = {
    {"9151E206", "AG001"},
    {"04D4E5F6", "AG002"},
};

// Renvoie true si le badge est connu et copie son identifiant dans `out`
// (taille LOG_FIELD_AGENT_LEN + 1).
//
// Badge inconnu : renvoie false et met dans `out` les 5 derniers caractères
// de l'UID, pour que le serveur puisse tout de même repérer le badge.
inline bool badge_agent_id(const char *uid, char *out)
{
  for (size_t i = 0; i < sizeof(BADGES) / sizeof(BADGES[0]); i++)
  {
    if (strcmp(uid, BADGES[i].uid) == 0)
    {
      strncpy(out, BADGES[i].agentId, LOG_FIELD_AGENT_LEN);
      out[LOG_FIELD_AGENT_LEN] = '\0';
      return true;
    }
  }

  size_t len = strlen(uid);
  const char *tail = (len > LOG_FIELD_AGENT_LEN) ? uid + (len - LOG_FIELD_AGENT_LEN) : uid;

  strncpy(out, tail, LOG_FIELD_AGENT_LEN);
  out[LOG_FIELD_AGENT_LEN] = '\0';

  return false;
}

#endif