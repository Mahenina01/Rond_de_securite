#ifndef LOG_FORMAT_H
#define LOG_FORMAT_H

#include <stdint.h>
#include <stddef.h>
#include <LittleFS.h>

// Tailles des champs du fichier CSV persistant (LittleFS)
#define LOG_FIELD_TIMESTAMP_LEN  20  // "YYYY-MM-DDTHH:MM:SSZ"
#define LOG_FIELD_AGENT_LEN      5   // Ex: "AG001"
#define LOG_FIELD_CHECKPOINT_LEN 14  // Ex: "CHECKPOINT_001"
#define LOG_LINE_LEN             44  // Taille fixe d'une ligne CSV avec '\n'

/// Seuil d'occupation LittleFS (%) déclenchant la libération d'espace
#define LOG_SPACE_LOW_THRESHOLD_PERCENT 90

/// Statut d'envoi, codé en caractère ASCII ('0'/'1'/'2') pour écriture directe en mémoire flash
enum LogStatus : char
{
    LOG_STATUS_PENDING = '0', ///< Écrit localement, envoi non confirmé (EN_ATTENTE)
    LOG_STATUS_SENT    = '1', ///< Transmis et acquitté via MQTT (ENVOYE)
    LOG_STATUS_FAILED  = '2', ///< Échec du dernier envoi (ECHEC)
};

/// Structure unifiée de log en mémoire (Zéro allocation dynamique)
struct LogEntry
{
    uint32_t id;                                       ///< Identifiant unique du log (utilisé par MQTT)
    char timestamp_iso[LOG_FIELD_TIMESTAMP_LEN + 1];  ///< Tampon horodateur ISO 8601 (+ '\0')
    char id_agent[LOG_FIELD_AGENT_LEN + 1];           ///< Identifiant agent (+ '\0')
    char id_checkpoint[LOG_FIELD_CHECKPOINT_LEN + 1];  ///< Identifiant checkpoint (+ '\0')
    LogStatus statut_envoi;                            ///< État de la transmission
    uint32_t line_index;                               ///< Position de la ligne pour accès direct seek()
};

// API du Gestionnaire de Stockage (LittleFS)
bool   storage_init();
bool   storage_append_log(const char *timestamp_iso, const char *id_agent,
                          const char *id_checkpoint, uint32_t *out_line_index);
bool   storage_update_status(uint32_t line_index, LogStatus new_status);
size_t storage_find_by_status(LogStatus status, LogEntry *out_entries, size_t max_entries);
size_t storage_count_by_status(LogStatus status);
void   storage_build_log_id(const LogEntry &entry, char *out, size_t out_len);
bool   storage_is_space_low(uint8_t threshold_percent = LOG_SPACE_LOW_THRESHOLD_PERCENT);
bool   storage_reclaim_space();

#endif // LOG_FORMAT_H