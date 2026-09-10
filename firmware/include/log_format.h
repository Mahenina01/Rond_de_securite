#pragma once

#include <stdint.h>
#include <LittleFS.h>

#define LOG_FIELD_TIMESTAMP_LEN 20
#define LOG_FIELD_AGENT_LEN 5
#define LOG_FIELD_CHECKPOINT_LEN 14
#define LOG_LINE_LEN 44

/// Seuil d'occupation LittleFS (%) déclenchant la libération d'espace.
#define LOG_SPACE_LOW_THRESHOLD_PERCENT 90

/// Statut d'envoi, codé en caractère ASCII ('0'/'1'/'2') = octet persisté.
enum LogStatus : char
{
  LOG_STATUS_PENDING = '0', ///< Écrit localement, envoi non confirmé.
  LOG_STATUS_SENT = '1',    ///< Transmis et acquitté (ACK MQTT).
  LOG_STATUS_FAILED = '2',  ///< Échec du dernier envoi, à réessayer.
};

/// Entrée de log en mémoire.
struct LogEntry
{
  char timestamp_iso[LOG_FIELD_TIMESTAMP_LEN + 1];
  char id_agent[LOG_FIELD_AGENT_LEN + 1];
  char id_checkpoint[LOG_FIELD_CHECKPOINT_LEN + 1];
  LogStatus statut_envoi;
  uint32_t line_index; ///< Position de la ligne, pour seek() ultérieur.
};

/// Monte LittleFS et initialise logs.csv si absent
bool storage_init();

/// Ajoute une entrée (statut initial PENDING). Retourne son line_index
bool storage_append_log(const char *timestamp_iso, const char *id_agent,
                        const char *id_checkpoint, uint32_t *out_line_index);

/// Met à jour le statut d'une ligne par accès direct (seek + 1 octet)
bool storage_update_status(uint32_t line_index, LogStatus new_status);

/// Retourne les entrées correspondant au statut demandé
size_t storage_find_by_status(LogStatus status, LogEntry *out_entries, size_t max_entries);

/// Construit un identifiant unique (timestamp + checkpoint) pour l'ACK MQTT
void storage_build_log_id(const LogEntry &entry, char *out, size_t out_len);

/// Indique si l'espace LittleFS occupé dépasse le seuil critique
bool storage_is_space_low(uint8_t threshold_percent = LOG_SPACE_LOW_THRESHOLD_PERCENT);

/**
 * @brief Libère de l'espace en supprimant les entrées LOG_STATUS_SENT.
 *        Cède la main périodiquement (yield) pour éviter un déclenchement du watchdog.
 * @return true si la compaction a réussi
 */
bool storage_reclaim_space();