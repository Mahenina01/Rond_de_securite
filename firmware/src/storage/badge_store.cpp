#include "badge_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <stdio.h>
#include <string.h>

namespace
{

  const char *PATH = "/badges.csv";
  const char *TMP_PATH = "/badges.tmp";

  BadgeRecord g_badges[BADGE_MAX];
  size_t g_count = 0;

  char g_lastUid[BADGE_UID_MAX_LEN + 1] = "";
  bool g_lastKnown = false;
  bool g_hasLast = false;
  uint32_t g_lastMs = 0;

  // Normalise un UID saisi : espaces, ':' et '-' ignorés, majuscules,
  // hexadécimal uniquement, 8 à 20 caractères et nombre pair.
  bool normalizeUid(const char *in, char *out)
  {
    if (in == nullptr)
      return false;

    size_t len = 0;

    for (; *in != '\0'; ++in)
    {
      char c = *in;

      if (c == ' ' || c == ':' || c == '-')
        continue;

      if (c >= 'a' && c <= 'f')
        c = static_cast<char>(c - 'a' + 'A');

      bool hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');

      if (!hex || len >= BADGE_UID_MAX_LEN)
        return false;

      out[len++] = c;
    }

    out[len] = '\0';

    return len >= 8 && (len % 2) == 0;
  }

  // Identifiant d'agent : 1 à 5 caractères A-Z 0-9 _ - (espaces de bord ignorés).
  bool normalizeAgent(const char *in, char *out)
  {
    if (in == nullptr)
      return false;

    while (*in == ' ')
      ++in;

    size_t len = 0;

    for (; *in != '\0'; ++in)
    {
      char c = *in;

      if (c == ' ')
      {
        // Seuls des espaces de fin sont tolérés.
        const char *p = in;

        while (*p == ' ')
          ++p;

        if (*p != '\0')
          return false;

        break;
      }

      if (c >= 'a' && c <= 'z')
        c = static_cast<char>(c - 'a' + 'A');

      bool ok = (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-';

      if (!ok || len >= LOG_FIELD_AGENT_LEN)
        return false;

      out[len++] = c;
    }

    out[len] = '\0';

    return len >= 1;
  }

  int findIndex(const char *uid)
  {
    for (size_t i = 0; i < g_count; i++)
    {
      if (strcmp(g_badges[i].uid, uid) == 0)
        return static_cast<int>(i);
    }

    return -1;
  }

  // Écriture sûre : fichier temporaire complet, puis remplacement. Une
  // coupure de courant ne laisse jamais une liste à moitié écrite.
  bool save()
  {
    File f = LittleFS.open(TMP_PATH, "w");

    if (!f)
      return false;

    bool ok = true;

    for (size_t i = 0; i < g_count && ok; i++)
    {
      char line[BADGE_UID_MAX_LEN + LOG_FIELD_AGENT_LEN + 4];

      int n = snprintf(line, sizeof(line), "%s,%s\n", g_badges[i].uid, g_badges[i].agent);

      ok = (n > 0) && f.write(reinterpret_cast<const uint8_t *>(line), static_cast<size_t>(n)) == static_cast<size_t>(n);
    }

    f.close();

    if (!ok)
    {
      LittleFS.remove(TMP_PATH);
      return false;
    }

    LittleFS.remove(PATH);

    return LittleFS.rename(TMP_PATH, PATH);
  }

  void parseLine(const char *line)
  {
    const char *comma = strchr(line, ',');

    if (comma == nullptr || g_count >= BADGE_MAX)
      return;

    char uidRaw[BADGE_UID_MAX_LEN + 8];
    size_t uidLen = static_cast<size_t>(comma - line);

    if (uidLen >= sizeof(uidRaw))
      return;

    memcpy(uidRaw, line, uidLen);
    uidRaw[uidLen] = '\0';

    BadgeRecord record;

    if (!normalizeUid(uidRaw, record.uid) || !normalizeAgent(comma + 1, record.agent))
      return;

    if (findIndex(record.uid) >= 0)
      return;

    g_badges[g_count++] = record;
  }

} // namespace

bool badges_init(void)
{
  // Reprise après une coupure pendant save() : l'ancien fichier a été
  // supprimé, mais le nouveau (complet) existe encore.
  if (!LittleFS.exists(PATH) && LittleFS.exists(TMP_PATH))
    LittleFS.rename(TMP_PATH, PATH);

  g_count = 0;

  if (!LittleFS.exists(PATH))
    return save(); // crée une liste vide

  File f = LittleFS.open(PATH, "r");

  if (!f)
    return false;

  char line[BADGE_UID_MAX_LEN + LOG_FIELD_AGENT_LEN + 8];
  size_t len = 0;
  uint8_t chunk[256];
  size_t n;

  while ((n = f.read(chunk, sizeof(chunk))) > 0)
  {
    for (size_t i = 0; i < n; i++)
    {
      char c = static_cast<char>(chunk[i]);

      if (c == '\n' || c == '\r')
      {
        line[len] = '\0';

        if (len > 0)
          parseLine(line);

        len = 0;
      }
      else if (len + 1 < sizeof(line))
      {
        line[len++] = c;
      }
    }
  }

  if (len > 0)
  {
    line[len] = '\0';
    parseLine(line);
  }

  f.close();

  return true;
}

size_t badges_count(void)
{
  return g_count;
}

bool badges_get(size_t index, BadgeRecord *out)
{
  if (index >= g_count || out == nullptr)
    return false;

  *out = g_badges[index];

  return true;
}

bool badges_find(const char *uid, char *agentOut)
{
  char normalized[BADGE_UID_MAX_LEN + 1];

  if (!normalizeUid(uid, normalized))
    return false;

  int i = findIndex(normalized);

  if (i < 0)
    return false;

  if (agentOut != nullptr)
  {
    memcpy(agentOut, g_badges[i].agent, LOG_FIELD_AGENT_LEN + 1);
  }

  return true;
}

BadgeResult badges_add(const char *uid, const char *agent)
{
  BadgeRecord record;

  if (!normalizeUid(uid, record.uid))
    return BadgeResult::INVALID_UID;

  if (!normalizeAgent(agent, record.agent))
    return BadgeResult::INVALID_AGENT;

  if (findIndex(record.uid) >= 0)
    return BadgeResult::DUPLICATE;

  if (g_count >= BADGE_MAX)
    return BadgeResult::FULL;

  g_badges[g_count++] = record;

  if (!save())
  {
    g_count--;
    return BadgeResult::STORAGE_ERROR;
  }

  return BadgeResult::OK;
}

BadgeResult badges_update(const char *uid, const char *newUid, const char *agent)
{
  char current[BADGE_UID_MAX_LEN + 1];

  if (!normalizeUid(uid, current))
    return BadgeResult::INVALID_UID;

  int i = findIndex(current);

  if (i < 0)
    return BadgeResult::NOT_FOUND;

  BadgeRecord updated = g_badges[i];

  if (newUid != nullptr && newUid[0] != '\0')
  {
    if (!normalizeUid(newUid, updated.uid))
      return BadgeResult::INVALID_UID;

    int other = findIndex(updated.uid);

    if (other >= 0 && other != i)
      return BadgeResult::DUPLICATE;
  }

  if (agent != nullptr && agent[0] != '\0')
  {
    if (!normalizeAgent(agent, updated.agent))
      return BadgeResult::INVALID_AGENT;
  }

  BadgeRecord previous = g_badges[i];

  g_badges[i] = updated;

  if (!save())
  {
    g_badges[i] = previous;
    return BadgeResult::STORAGE_ERROR;
  }

  return BadgeResult::OK;
}

BadgeResult badges_remove(const char *uid)
{
  char normalized[BADGE_UID_MAX_LEN + 1];

  if (!normalizeUid(uid, normalized))
    return BadgeResult::INVALID_UID;

  int i = findIndex(normalized);

  if (i < 0)
    return BadgeResult::NOT_FOUND;

  BadgeRecord removed = g_badges[i];

  for (size_t k = static_cast<size_t>(i); k + 1 < g_count; k++)
    g_badges[k] = g_badges[k + 1];

  g_count--;

  if (!save())
  {
    for (size_t k = g_count; k > static_cast<size_t>(i); k--)
      g_badges[k] = g_badges[k - 1];

    g_badges[i] = removed;
    g_count++;

    return BadgeResult::STORAGE_ERROR;
  }

  return BadgeResult::OK;
}

void badges_note_scan(const char *uid, bool known)
{
  if (uid == nullptr)
    return;

  strncpy(g_lastUid, uid, BADGE_UID_MAX_LEN);
  g_lastUid[BADGE_UID_MAX_LEN] = '\0';

  g_lastKnown = known;
  g_hasLast = true;
  g_lastMs = millis();
}

bool badges_last_scan(char *uidOut, bool *known, uint32_t *ageSeconds)
{
  if (!g_hasLast)
    return false;

  if (uidOut != nullptr)
    memcpy(uidOut, g_lastUid, sizeof(g_lastUid));

  if (known != nullptr)
    *known = g_lastKnown;

  if (ageSeconds != nullptr)
    *ageSeconds = (millis() - g_lastMs) / 1000;

  return true;
}