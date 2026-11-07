#include "log_sync.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "hal/mqtt_manager.h"
#include "log_format.h"
#include "system_config.h"

// CONFIGURATION

// Délai avant de renvoyer un log dont la publication ou l'ACK a échoué.
#define SYNC_RETRY_DELAY_MS 2000UL

// Re-scan périodique du fichier quand rien n'est en attente (filet de
// sécurité si log_sync_request() n'a pas été appelé). Le scan lit tout le
// fichier : à ne pas faire trop souvent.
#define SYNC_RESCAN_INTERVAL_MS 60000UL

// Délai avant de remettre les logs en ECHEC dans la file d'envoi.
#define SYNC_REQUEUE_FAILED_MS 300000UL

// Nombre de logs ENVOYE accumulés avant de les supprimer de la mémoire.
// Plus petit = fichier plus petit ; plus grand = moins d'écritures en flash.
#ifndef SYNC_PURGE_MIN_SENT
#define SYNC_PURGE_MIN_SENT 10
#endif

// ETAT INTERNE

namespace
{

  enum class SyncState
  {
    IDLE,
    WAIT_ACK
  };

  SyncState state = SyncState::IDLE;

  // Lot de logs en cours d'envoi (tableau statique : ni pile, ni tas).
  LogEntry batch[MAX_PENDING_LOGS_BATCH];
  size_t batchCount = 0;
  size_t batchPos = 0;

  // Tentatives pour le log courant.
  int attempts = 0;

  uint32_t ackStartMs = 0;

  uint32_t nextActionMs = 0;
  uint32_t nextScanMs = 0;
  uint32_t nextRequeueMs = 0;

  bool scanRequested = true;

  // Purge : logs ENVOYE accumulés depuis la dernière purge. La première
  // purge est faite au démarrage pour nettoyer les restes d'une session
  // précédente.
  uint32_t sentSincePurge = 0;
  bool purgeRequested = true;

  // Comparaison d'échéance insensible au débordement de millis().
  bool reached(uint32_t deadline)
  {
    return static_cast<int32_t>(millis() - deadline) >= 0;
  }

  // IDENTIFIANT PUBLIE
  //
  // L'"id" envoyé au serveur (et renvoyé dans l'ACK) est une empreinte CRC32
  // de "timestamp,agent,checkpoint". Il ne dépend PAS du numéro de ligne :
  // la purge des logs ENVOYE décale les numéros de ligne, et un numéro de
  // ligne réutilisé permettrait à un ACK en double ou en retard (QoS 1)
  // d'acquitter un autre log à tort.
  uint32_t crc32(const char *text)
  {
    uint32_t crc = 0xFFFFFFFFu;

    for (; *text != '\0'; ++text)
    {
      crc ^= static_cast<uint8_t>(*text);

      for (int bit = 0; bit < 8; ++bit)
        crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }

    return ~crc;
  }

  uint32_t publishedId(const LogEntry &entry)
  {
    char key[LOG_FIELD_TIMESTAMP_LEN + LOG_FIELD_AGENT_LEN +
             LOG_FIELD_CHECKPOINT_LEN + 4];

    snprintf(key, sizeof(key), "%s,%s,%s",
             entry.timestamp_iso, entry.id_agent, entry.id_checkpoint);

    return crc32(key);
  }

  // Passe au log suivant du lot ; si le lot est terminé, demande un
  // nouveau scan (il peut rester d'autres logs, ou de nouveaux).
  void advance()
  {
    batchPos++;

    if (batchPos >= batchCount)
      scanRequested = true;
  }

  // Échec d'envoi du log courant (publication impossible ou ACK absent).
  void onSendFailure(const char *reason)
  {
    const LogEntry &entry = batch[batchPos];

    state = SyncState::IDLE;
    attempts++;

    Serial.print("[SYNC] Echec log id=");
    Serial.print(publishedId(entry));
    Serial.print(" : ");
    Serial.println(reason);

    nextActionMs = millis() + SYNC_RETRY_DELAY_MS;

    if (attempts >= MQTT_MAX_RETRIES)
    {
      storage_update_status(entry.line_index, LOG_STATUS_FAILED);

      Serial.print("[SYNC] Log id=");
      Serial.print(publishedId(entry));
      Serial.println(" marque ECHEC, nouvel essai plus tard.");

      attempts = 0;

      advance();
    }
  }

  // Supprime de la mémoire les logs ENVOYE (les logs EN_ATTENTE et ECHEC
  // sont conservés).
  //
  // Appelée uniquement quand il n'y a plus rien en cours : état IDLE, lot
  // vide, aucun log EN_ATTENTE. La suppression décale les numéros de ligne,
  // donc aucun envoi ne doit être en cours.
  void purgeSentLogs()
  {
    if (!purgeRequested && sentSincePurge < SYNC_PURGE_MIN_SENT)
      return;

    if (storage_reclaim_space())
    {
      Serial.println("[SYNC] Logs ENVOYE supprimes de la memoire.");

      sentSincePurge = 0;
      purgeRequested = false;
    }
    else
    {
      Serial.println("[SYNC] Echec de la purge, nouvel essai plus tard.");
    }

    batchCount = 0;
    batchPos = 0;
  }

  // Remet en file d'envoi les logs en ECHEC (au plus un lot à la fois).
  void requeueFailedLogs()
  {
    if (!reached(nextRequeueMs))
      return;

    nextRequeueMs = millis() + SYNC_REQUEUE_FAILED_MS;

    size_t count = storage_find_by_status(
        LOG_STATUS_FAILED,
        batch,
        MAX_PENDING_LOGS_BATCH);

    for (size_t i = 0; i < count; i++)
      storage_update_status(batch[i].line_index, LOG_STATUS_PENDING);

    if (count > 0)
    {
      Serial.print("[SYNC] Logs ECHEC remis en attente : ");
      Serial.println(static_cast<unsigned int>(count));

      scanRequested = true;
    }

    batchCount = 0;
    batchPos = 0;
  }

  // Charge un nouveau lot de logs EN_ATTENTE. Renvoie false s'il n'y en a pas.
  bool loadBatch()
  {
    scanRequested = false;
    nextScanMs = millis() + SYNC_RESCAN_INTERVAL_MS;

    batchCount = storage_find_by_status(
        LOG_STATUS_PENDING,
        batch,
        MAX_PENDING_LOGS_BATCH);

    batchPos = 0;

    if (batchCount == 0)
    {
      // File vide : moment sûr pour libérer la mémoire.
      purgeSentLogs();
      requeueFailedLogs();
      return false;
    }

    Serial.print("[SYNC] Logs en attente (lot) : ");
    Serial.println(static_cast<unsigned int>(batchCount));

    return true;
  }

  void publishCurrent()
  {
    const LogEntry &entry = batch[batchPos];

    // Copie à envoyer : l'identifiant publié remplace le numéro de ligne.
    // (Le statut sera mis à jour avec le vrai numéro de ligne, dans batch.)
    LogEntry toSend = entry;
    toSend.line_index = publishedId(entry);

    Serial.print("[SYNC] Publication du log id=");
    Serial.print(toSend.line_index);
    Serial.print(" (tentative ");
    Serial.print(attempts + 1);
    Serial.print("/");
    Serial.print(MQTT_MAX_RETRIES);
    Serial.println(")");

    if (mqtt_publish_log(toSend))
    {
      ackStartMs = millis();
      state = SyncState::WAIT_ACK;
    }
    else
    {
      onSendFailure("publication impossible");
    }
  }

  void handleWaitAck()
  {
    const LogEntry &entry = batch[batchPos];

    // Le délai est géré ici : on donne à mqtt_wait_ack() un délai plus long
    // pour qu'il ne signale jamais lui-même un timeout avant nous.
    if (mqtt_wait_ack(MQTT_ACK_TIMEOUT_MS * 2))
    {
      if (storage_update_status(entry.line_index, LOG_STATUS_SENT))
      {
        sentSincePurge++;

        Serial.print("[SYNC] Log id=");
        Serial.print(publishedId(entry));
        Serial.println(" ENVOYE et acquitte.");
      }
      else
      {
        Serial.println("[SYNC] ACK recu mais statut non enregistre.");
      }

      attempts = 0;
      state = SyncState::IDLE;

      advance();

      return;
    }

    if (millis() - ackStartMs >= MQTT_ACK_TIMEOUT_MS)
      onSendFailure("ACK non recu");
  }

} // namespace

// API PUBLIQUE

void log_sync_request()
{
  scanRequested = true;
}

void log_sync_reset()
{
  state = SyncState::IDLE;

  batchCount = 0;
  batchPos = 0;
  attempts = 0;

  nextActionMs = 0;
  scanRequested = true;
}

bool log_sync_busy()
{
  return state == SyncState::WAIT_ACK;
}

void log_sync_process()
{
  if (state == SyncState::WAIT_ACK)
  {
    handleWaitAck();
    return;
  }

  if (!reached(nextActionMs))
    return;

  // Lot épuisé : on ne relit le fichier que sur demande ou périodiquement.
  if (batchPos >= batchCount)
  {
    if (!scanRequested && !reached(nextScanMs))
      return;

    if (!loadBatch())
      return;
  }

  publishCurrent();
}