#ifndef SIM_MANAGER_H
#define SIM_MANAGER_H

// TinyGSM doit connaître le modèle avant son inclusion.
#ifndef TINY_GSM_MODEM_SIM7600
#endif

#include <stdint.h>

#include <TinyGsmClient.h>

struct SimNetworkTime
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int utcOffsetMinutes;
};

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

// Heure fournie par le réseau mobile, avec son décalage UTC.
bool sim_get_network_time(SimNetworkTime &networkTime);

// Accès à l'instance TinyGSM.
TinyGsm &sim_get_modem();

// Accès au port UART partagé avec les commandes AT natives.
HardwareSerial &sim_get_serial();

#endif