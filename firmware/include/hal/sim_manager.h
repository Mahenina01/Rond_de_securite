#ifndef SIM_MANAGER_H
#define SIM_MANAGER_H

// TinyGSM doit connaître le modèle avant son inclusion.
#ifndef TINY_GSM_MODEM_SIM7600
#endif

#include <TinyGsmClient.h>

// Démarrage non bloquant du modem.
bool sim_power_on();

// Arrêt du modem.
bool sim_power_off();

// Vérification non bloquante de l'enregistrement réseau.
bool sim_wait_network_ready(uint32_t timeout_ms);

// Initialisation TinyGSM.
bool sim_init_modem();

// Connexion Internet par APN.
bool sim_connect_internet();

// État de la connexion Internet.
bool sim_is_internet_connected();

// Accès à l'instance TinyGSM.
TinyGsm &sim_get_modem();

// Accès au port UART partagé avec les commandes AT natives.
HardwareSerial &sim_get_serial();

#endif