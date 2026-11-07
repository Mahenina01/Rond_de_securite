#ifndef LOG_SYNC_H
#define LOG_SYNC_H

// Synchronisation des logs LittleFS vers HiveMQ Cloud.
//
// Cycle d'un log :
//   EN_ATTENTE ('0') --publication + ACK--> ENVOYE ('1')
//   EN_ATTENTE ('0') --3 échecs-----------> ECHEC   ('2')
//   ECHEC      ('2') --après 5 min--------> EN_ATTENTE ('0') (nouvel essai)
//
// Un log n'est marqué ENVOYE qu'après réception de l'ACK applicatif du
// serveur sur le topic ronde/boitier/<DEVICE_ID>/ack avec le même "eventUuid".

// Demande une synchronisation à la prochaine occasion.
// À appeler après chaque storage_append_log() réussi.
void log_sync_request();

// Remet l'état interne à zéro (perte de connexion, redémarrage du modem).
// Les logs non acquittés restent EN_ATTENTE dans le fichier : rien n'est perdu.
void log_sync_reset();

// À appeler à chaque tour de loop(), uniquement quand MQTT est connecté.
// Non bloquant, sauf pendant mqtt_publish_log() (quelques secondes).
void log_sync_process();

// true si un log a été publié et attend son ACK.
bool log_sync_busy();

#endif