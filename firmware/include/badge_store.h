#ifndef BADGE_STORE_H
#define BADGE_STORE_H

#include <stddef.h>
#include <stdint.h>

#include "log_format.h" // LOG_FIELD_AGENT_LEN

// Liste des badges autorisés sur ce boîtier, modifiable depuis le tableau de
// bord web. Stockée dans /badges.csv (LittleFS), une ligne par badge :
//   UID,AGENT
// Aucune allocation dynamique : tableau fixe de BADGE_MAX badges.

#define BADGE_MAX 100
#define BADGE_UID_MAX_LEN 20 // 10 octets en hexadécimal

struct BadgeRecord
{
  char uid[BADGE_UID_MAX_LEN + 1];     // hexadécimal MAJUSCULE, 8 à 20 caractères
  char agent[LOG_FIELD_AGENT_LEN + 1]; // 1 à 5 caractères : A-Z 0-9 _ -
};

enum class BadgeResult : uint8_t
{
  OK,
  INVALID_UID,
  INVALID_AGENT,
  DUPLICATE,
  NOT_FOUND,
  FULL,
  STORAGE_ERROR
};

// Charge la liste depuis LittleFS (la crée si elle n'existe pas).
// À appeler après storage_init().
bool badges_init(void);

size_t badges_count(void);
bool badges_get(size_t index, BadgeRecord *out);

// true si le badge est autorisé ; copie alors l'identifiant de l'agent dans
// `agentOut` (LOG_FIELD_AGENT_LEN + 1 octets).
bool badges_find(const char *uid, char *agentOut);

BadgeResult badges_add(const char *uid, const char *agent);

// Modifie un badge. `newUid` et `agent` peuvent être nuls ou vides : la valeur
// actuelle est alors conservée.
BadgeResult badges_update(const char *uid, const char *newUid, const char *agent);

BadgeResult badges_remove(const char *uid);

// Dernier badge présenté au lecteur (connu ou non) : permet de l'ajouter
// depuis le tableau de bord sans saisir l'UID à la main.
void badges_note_scan(const char *uid, bool known);
bool badges_last_scan(char *uidOut, bool *known, uint32_t *ageSeconds);

#endif