#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <stdint.h>

#include "log_format.h"

// Initialisation du gestionnaire.
void mqtt_init();

// Nettoyage du modem MQTT.
void cleanupModemMQTT();

// Connexion HiveMQ Cloud avec TLS.
bool mqtt_connect();

// Publication d'un journal RFID.
bool mqtt_publish_log(const LogEntry &entry);

// Traitement des messages reçus.
void mqtt_loop();

// Attente non bloquante de l'ACK applicatif.
bool mqtt_wait_ack(uint32_t timeout_ms);

// Vérification de la connexion.
bool mqtt_is_connected();

// Tentative de reconnexion.
void mqtt_maintain();

// Déconnexion.
void mqtt_disconnect();

#endif