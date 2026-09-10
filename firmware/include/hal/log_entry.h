#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <stdint.h>

#define AGENT_ID_LEN     16
#define CHECKPOINT_ID_LEN 16
#define TIMESTAMP_ISO_LEN 25 // Format "YYYY-MM-DDTHH:MM:SSZ\0"

// Structure de log à taille fixe (Alignée SOP - Zéro allocation dynamique)
struct LogEntry {
    uint32_t id;
    char timestamp_iso[TIMESTAMP_ISO_LEN];
    char id_agent[AGENT_ID_LEN];
    char id_checkpoint[CHECKPOINT_ID_LEN];
    enum Status { EN_ATTENTE, ENVOYE, ECHEC } statut_envoi;
};

#endif // LOG_ENTRY_H