#include "../include/log_format.h"
#include <Arduino.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace
{

  const char *LOG_FILE_PATH = "/logs.csv";
  const char *LOG_TMP_PATH = "/logs.tmp";
  const char *LOG_BACKUP_PATH = "/logs.bak";

  constexpr size_t LEGACY_CHECKPOINT_LEN = 14;
  constexpr size_t LEGACY_LINE_LEN = 44;
  SemaphoreHandle_t storageMutex = nullptr;

  class StorageGuard
  {
  public:
    StorageGuard()
        : acquired(storageMutex != nullptr &&
                   xSemaphoreTakeRecursive(
                       storageMutex,
                       pdMS_TO_TICKS(5000)) == pdTRUE)
    {
    }

    ~StorageGuard()
    {
      if (acquired)
        xSemaphoreGiveRecursive(storageMutex);
    }

    explicit operator bool() const
    {
      return acquired;
    }

  private:
    bool acquired;
  };

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

  bool validateLogFile(const char *path, size_t lineLen, size_t checkpointLen)
  {
    File file = LittleFS.open(path, "r");
    if (!file)
      return false;

    const size_t fileSize = file.size();
    if (fileSize % lineLen != 0)
    {
      file.close();
      return false;
    }

    const size_t statusOffset =
        LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN + 1 +
        checkpointLen + 1;
    uint8_t line[LOG_LINE_LEN];

    while (file.available())
    {
      if (file.read(line, lineLen) != lineLen ||
          line[LOG_FIELD_TIMESTAMP_LEN] != ',' ||
          line[LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN] != ',' ||
          line[statusOffset - 1] != ',' ||
          (line[statusOffset] != LOG_STATUS_PENDING &&
           line[statusOffset] != LOG_STATUS_SENT &&
           line[statusOffset] != LOG_STATUS_FAILED) ||
          line[statusOffset + 1] != '\n')
      {
        file.close();
        return false;
      }
    }

    file.close();
    return true;
  }

  bool migrateLegacyLogFile()
  {
    if (validateLogFile(LOG_FILE_PATH, LOG_LINE_LEN, LOG_FIELD_CHECKPOINT_LEN))
      return true;

    if (!validateLogFile(LOG_FILE_PATH, LEGACY_LINE_LEN, LEGACY_CHECKPOINT_LEN))
    {
      Serial.println("[storage] Format de logs inconnu ou fichier endommage.");
      return false;
    }

    if (LittleFS.exists(LOG_BACKUP_PATH))
    {
      Serial.println("[storage] Migration impossible : sauvegarde existante.");
      return false;
    }

    File source = LittleFS.open(LOG_FILE_PATH, "r");
    File temporary = LittleFS.open(LOG_TMP_PATH, "w");
    if (!source || !temporary)
    {
      if (source)
        source.close();
      if (temporary)
        temporary.close();
      Serial.println("[storage] Ouverture impossible pour migrer les logs.");
      return false;
    }

    uint8_t legacyLine[LEGACY_LINE_LEN];
    uint8_t newLine[LOG_LINE_LEN];
    bool migrationOk = true;

    while (source.available())
    {
      if (source.read(legacyLine, LEGACY_LINE_LEN) != LEGACY_LINE_LEN)
      {
        migrationOk = false;
        break;
      }

      memcpy(newLine, legacyLine, 27);
      memset(newLine + 27, ' ', LOG_FIELD_CHECKPOINT_LEN);
      memcpy(newLine + 27, legacyLine + 27, LEGACY_CHECKPOINT_LEN);
      memcpy(newLine + 47, legacyLine + 41, 3);

      if (temporary.write(newLine, LOG_LINE_LEN) != LOG_LINE_LEN)
      {
        migrationOk = false;
        break;
      }
    }

    source.close();
    temporary.close();

    if (!migrationOk ||
        !validateLogFile(LOG_TMP_PATH, LOG_LINE_LEN, LOG_FIELD_CHECKPOINT_LEN))
    {
      Serial.println("[storage] Echec d'ecriture lors de la migration.");
      return false;
    }

    if (!LittleFS.rename(LOG_FILE_PATH, LOG_BACKUP_PATH))
    {
      Serial.println("[storage] Impossible de sauvegarder les anciens logs.");
      return false;
    }

    if (!LittleFS.rename(LOG_TMP_PATH, LOG_FILE_PATH))
    {
      LittleFS.rename(LOG_BACKUP_PATH, LOG_FILE_PATH);
      Serial.println("[storage] Echec d'activation des nouveaux logs.");
      return false;
    }

    if (!LittleFS.remove(LOG_BACKUP_PATH))
      Serial.println("[storage] Ancienne sauvegarde non supprimee.");

    Serial.println("[storage] Anciens logs migres vers le nouveau format.");
    return true;
  }

} // namespace

bool storage_init()
{
  if (storageMutex == nullptr)
    storageMutex = xSemaphoreCreateRecursiveMutex();

  StorageGuard guard;
  if (!guard)
  {
    Serial.println("[storage] Verrou LittleFS indisponible.");
    return false;
  }

  if (!LittleFS.begin(true, "/littlefs", 10, "spiffs"))
  {
    Serial.println("[storage] Echec montage LittleFS");
    return false;
  }

  if (!LittleFS.exists(LOG_FILE_PATH) && LittleFS.exists(LOG_BACKUP_PATH))
  {
    if (!LittleFS.rename(LOG_BACKUP_PATH, LOG_FILE_PATH))
    {
      Serial.println("[storage] Restauration de la sauvegarde impossible.");
      return false;
    }
  }

  if (!LittleFS.exists(LOG_FILE_PATH))
  {
    File f = LittleFS.open(LOG_FILE_PATH, "w");
    if (!f)
      return false;
    f.close();
  }

  if (!migrateLegacyLogFile())
    return false;

  return true;
}

bool storage_acquire()
{
  return storageMutex != nullptr &&
         xSemaphoreTakeRecursive(
             storageMutex,
             pdMS_TO_TICKS(5000)) == pdTRUE;
}

void storage_release()
{
  if (storageMutex != nullptr)
    xSemaphoreGiveRecursive(storageMutex);
}

bool storage_append_log(const char *timestamp_iso, const char *id_agent,
                        const char *id_checkpoint, uint32_t *out_line_index)
{
  StorageGuard guard;
  if (!guard)
    return false;

  if (strlen(timestamp_iso) != LOG_FIELD_TIMESTAMP_LEN)
    return false;

  File f = LittleFS.open(LOG_FILE_PATH, "a");
  if (!f)
    return false;

  uint32_t lineIndex = f.size() / LOG_LINE_LEN;

  char line[LOG_LINE_LEN + 1];
  char agentField[LOG_FIELD_AGENT_LEN + 1];
  char checkpointField[LOG_FIELD_CHECKPOINT_LEN + 1];
  padRight(agentField, id_agent, LOG_FIELD_AGENT_LEN, ' ');
  if (strlen(id_checkpoint) > LOG_FIELD_CHECKPOINT_LEN)
  {
    f.close();
    return false;
  }
  padRight(checkpointField, id_checkpoint, LOG_FIELD_CHECKPOINT_LEN, ' ');

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
  StorageGuard guard;
  if (!guard)
    return false;

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
  StorageGuard guard;
  if (!guard)
    return 0;

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
    for (size_t i = LOG_FIELD_CHECKPOINT_LEN; i > 0; --i)
    {
      if (entry.id_checkpoint[i - 1] != ' ')
        break;
      entry.id_checkpoint[i - 1] = '\0';
    }

    size_t statusOffset = checkpointOffset + LOG_FIELD_CHECKPOINT_LEN + 1;
    entry.statut_envoi = (LogStatus)buffer[statusOffset];

    // Assignation des deux identifiants
    entry.line_index = lineIndex;
    entry.id = lineIndex; // L'identifiant unique correspond au numéro de ligne

    if (entry.statut_envoi == status)
      out_entries[found++] = entry;
    lineIndex++;
  }

  f.close();
  return found;
}

size_t storage_count_by_status(LogStatus status)
{
  StorageGuard guard;
  if (!guard)
    return 0;

  File f = LittleFS.open(LOG_FILE_PATH, "r");
  if (!f)
    return 0;

  size_t count = 0;
  uint8_t buffer[LOG_LINE_LEN];

  const size_t statusOffset = LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN + 1 +
                              LOG_FIELD_CHECKPOINT_LEN + 1;

  while (f.available() >= LOG_LINE_LEN)
  {
    if (f.read(buffer, LOG_LINE_LEN) != LOG_LINE_LEN)
      break;

    LogStatus lineStatus = (LogStatus)buffer[statusOffset];
    if (lineStatus == status)
    {
      count++;
    }
  }

  f.close();
  return count;
}

void storage_build_log_id(const LogEntry &entry, char *out, size_t out_len)
{
  char compact[LOG_FIELD_TIMESTAMP_LEN + 1];
  compactTimestamp(entry.timestamp_iso, compact, sizeof(compact));
  snprintf(out, out_len, "%s-%s", compact, entry.id_checkpoint);
}

bool storage_is_space_low(uint8_t threshold_percent)
{
  StorageGuard guard;
  if (!guard)
    return true;

  size_t total = LittleFS.totalBytes();
  size_t used = LittleFS.usedBytes();
  if (total == 0)
    return true;

  uint8_t usedPercent = (uint8_t)((used * 100UL) / total);
  return usedPercent >= threshold_percent;
}

bool storage_reclaim_space()
{
  StorageGuard guard;
  if (!guard)
    return false;

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

bool clear_storage_logs()
{
  StorageGuard guard;
  if (!guard)
    return false;

  if (LittleFS.exists(LOG_FILE_PATH))
  {
    LittleFS.remove(LOG_FILE_PATH);
  }
  File f = LittleFS.open(LOG_FILE_PATH, "w");
  if (!f)
    return false;
  f.close();
  return true;
}