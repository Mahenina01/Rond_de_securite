#include "log_sync.h"

#include <Arduino.h>
#include <stdint.h>

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

  // Comparaison d'échéance insensible au débordement de millis().
  bool reached(uint32_t deadline)
  {
    return static_cast<int32_t>(millis() - deadline) >= 0;
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
    Serial.print(entry.line_index);
    Serial.print(" : ");
    Serial.println(reason);

    nextActionMs = millis() + SYNC_RETRY_DELAY_MS;

    if (attempts >= MQTT_MAX_RETRIES)
    {
      storage_update_status(entry.line_index, LOG_STATUS_FAILED);

      Serial.print("[SYNC] Log id=");
      Serial.print(entry.line_index);
      Serial.println(" marque ECHEC, nouvel essai plus tard.");

      attempts = 0;

      advance();
    }
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

    Serial.print("[SYNC] Publication du log id=");
    Serial.print(entry.line_index);
    Serial.print(" (tentative ");
    Serial.print(attempts + 1);
    Serial.print("/");
    Serial.print(MQTT_MAX_RETRIES);
    Serial.println(")");

    if (mqtt_publish_log(entry))
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
        Serial.print("[SYNC] Log id=");
        Serial.print(entry.line_index);
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