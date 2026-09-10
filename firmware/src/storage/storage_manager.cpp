#include "../include/log_format.h"
#include <Arduino.h>
#include <string.h>

namespace
{

  const char *LOG_FILE_PATH = "/logs.csv";
  const char *LOG_TMP_PATH = "/logs.tmp";

  // Nombre de lignes traitées entre chaque yield() pendant la compaction.
  const uint32_t RECLAIM_YIELD_EVERY_N_LINES = 50;

  void padRight(char *dst, const char *src, size_t width, char padChar)
  {
    size_t len = strlen(src);
    size_t copyLen = (len < width) ? len : width;
    memcpy(dst, src, copyLen);
    for (size_t i = copyLen; i < width; ++i)
      dst[i] = padChar;
  }

  void padLeft(char *dst, const char *src, size_t width, char padChar)
  {
    size_t len = strlen(src);
    size_t copyLen = (len < width) ? len : width;
    size_t offset = width - copyLen;
    for (size_t i = 0; i < offset; ++i)
      dst[i] = padChar;
    memcpy(dst + offset, src + (len - copyLen), copyLen);
  }

  void compactTimestamp(const char *iso, char *out, size_t outLen)
  {
    size_t j = 0;
    for (size_t i = 0; iso[i] != '\0' && j + 1 < outLen; ++i)
    {
      if (iso[i] != '-' && iso[i] != ':')
        out[j++] = iso[i];
    }
    out[j] = '\0';
  }

} // namespace

bool storage_init()
{
  if (!LittleFS.begin(true, "/littlefs", 10, "littlefs"))
  {
    Serial.println("[storage] Echec montage LittleFS");
    return false;
  }
  if (!LittleFS.exists(LOG_FILE_PATH))
  {
    File f = LittleFS.open(LOG_FILE_PATH, "w");
    if (!f)
      return false;
    f.close();
  }
  return true;
}

bool storage_append_log(const char *timestamp_iso, const char *id_agent,
                        const char *id_checkpoint, uint32_t *out_line_index)
{
  if (strlen(timestamp_iso) != LOG_FIELD_TIMESTAMP_LEN)
    return false;

  // Espace insuffisant : tente une compaction avant d'écrire.
  if (storage_is_space_low())
  {
    storage_reclaim_space();
  }

  File f = LittleFS.open(LOG_FILE_PATH, "a");
  if (!f)
    return false;

  uint32_t lineIndex = f.size() / LOG_LINE_LEN;

  char line[LOG_LINE_LEN + 1];
  char agentField[LOG_FIELD_AGENT_LEN + 1];
  char checkpointField[LOG_FIELD_CHECKPOINT_LEN + 1];
  padRight(agentField, id_agent, LOG_FIELD_AGENT_LEN, ' ');
  padLeft(checkpointField, id_checkpoint, LOG_FIELD_CHECKPOINT_LEN, '0');

  int written = snprintf(line, sizeof(line), "%.*s,%.*s,%.*s,%c\n",
                         LOG_FIELD_TIMESTAMP_LEN, timestamp_iso,
                         LOG_FIELD_AGENT_LEN, agentField,
                         LOG_FIELD_CHECKPOINT_LEN, checkpointField,
                         (char)LOG_STATUS_PENDING);
  if (written != LOG_LINE_LEN)
  {
    f.close();
    return false;
  }

  size_t bytesWritten = f.write((const uint8_t *)line, LOG_LINE_LEN);
  f.close();
  if (bytesWritten != LOG_LINE_LEN)
    return false;

  if (out_line_index != nullptr)
    *out_line_index = lineIndex;
  return true;
}

bool storage_update_status(uint32_t line_index, LogStatus new_status)
{
  File f = LittleFS.open(LOG_FILE_PATH, "r+");
  if (!f)
    return false;

  const uint32_t statusColumnOffset =
      LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN + 1 + LOG_FIELD_CHECKPOINT_LEN + 1;
  uint32_t byteOffset = (line_index * LOG_LINE_LEN) + statusColumnOffset;

  if (byteOffset >= f.size() || !f.seek(byteOffset))
  {
    f.close();
    return false;
  }

  size_t bytesWritten = f.write((uint8_t)new_status);
  f.close();
  return bytesWritten == 1;
}

size_t storage_find_by_status(LogStatus status, LogEntry *out_entries, size_t max_entries)
{
  File f = LittleFS.open(LOG_FILE_PATH, "r");
  if (!f)
    return 0;

  size_t found = 0;
  uint32_t lineIndex = 0;
  uint8_t buffer[LOG_LINE_LEN];

  while (found < max_entries && f.available() >= LOG_LINE_LEN)
  {
    if (f.read(buffer, LOG_LINE_LEN) != LOG_LINE_LEN)
      break;

    LogEntry entry{};
    memcpy(entry.timestamp_iso, buffer, LOG_FIELD_TIMESTAMP_LEN);
    entry.timestamp_iso[LOG_FIELD_TIMESTAMP_LEN] = '\0';

    size_t agentOffset = LOG_FIELD_TIMESTAMP_LEN + 1;
    memcpy(entry.id_agent, buffer + agentOffset, LOG_FIELD_AGENT_LEN);
    entry.id_agent[LOG_FIELD_AGENT_LEN] = '\0';

    size_t checkpointOffset = agentOffset + LOG_FIELD_AGENT_LEN + 1;
    memcpy(entry.id_checkpoint, buffer + checkpointOffset, LOG_FIELD_CHECKPOINT_LEN);
    entry.id_checkpoint[LOG_FIELD_CHECKPOINT_LEN] = '\0';

    size_t statusOffset = checkpointOffset + LOG_FIELD_CHECKPOINT_LEN + 1;
    entry.statut_envoi = (LogStatus)buffer[statusOffset];
    entry.line_index = lineIndex;

    if (entry.statut_envoi == status)
      out_entries[found++] = entry;
    lineIndex++;
  }

  f.close();
  return found;
}

void storage_build_log_id(const LogEntry &entry, char *out, size_t out_len)
{
  char compact[LOG_FIELD_TIMESTAMP_LEN + 1];
  compactTimestamp(entry.timestamp_iso, compact, sizeof(compact));
  snprintf(out, out_len, "%s-%s", compact, entry.id_checkpoint);
}

bool storage_is_space_low(uint8_t threshold_percent)
{
  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();
  if (total == 0)
    return true;

  uint8_t usedPercent = (uint8_t)((used * 100UL) / total);
  return usedPercent >= threshold_percent;
}

bool storage_reclaim_space()
{
  File src = LittleFS.open(LOG_FILE_PATH, "r");
  if (!src)
    return false;

  File tmp = LittleFS.open(LOG_TMP_PATH, "w");
  if (!tmp)
  {
    src.close();
    return false;
  }

  uint8_t buffer[LOG_LINE_LEN];
  uint32_t lineCount = 0;

  while (src.available() >= LOG_LINE_LEN)
  {
    if (src.read(buffer, LOG_LINE_LEN) != LOG_LINE_LEN)
      break;

    size_t statusOffset = LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN + 1 +
                          LOG_FIELD_CHECKPOINT_LEN + 1;
    LogStatus statut = (LogStatus)buffer[statusOffset];

    // Ne conserve que les entrées non encore confirmées.
    if (statut != LOG_STATUS_SENT)
    {
      tmp.write(buffer, LOG_LINE_LEN);
    }

    // Cede la main periodiquement pour eviter un reset watchdog
    // pendant le parcours d'un gros fichier.
    if (++lineCount % RECLAIM_YIELD_EVERY_N_LINES == 0)
    {
      yield();
    }
  }

  src.close();
  tmp.close();

  LittleFS.remove(LOG_FILE_PATH);
  bool renamed = LittleFS.rename(LOG_TMP_PATH, LOG_FILE_PATH);

  if (!renamed)
  {
    Serial.println("[storage] Echec compaction : renommage logs.tmp");
    return false;
  }

  Serial.printf("[storage] Compaction terminee (%u lignes analysees)\n", lineCount);
  return true;
}