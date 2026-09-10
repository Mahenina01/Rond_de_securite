// sim_manager.cpp
#include "./hal/sim_manager.h"
#include "./system_config.h"
#include <Arduino.h>
<<<<<<< Updated upstream
#include "pins_config.h"
=======

// --- A confirmer avec le schema electrique HW (Sprint 2/3) ---
>>>>>>> Stashed changes

#define SIM_PWRKEY_PULSE_MS  1000   // duree d'impulsion PWRKEY -- valeur indicative,
                                     // A VERIFIER dans la datasheet exacte du SIM7600E-H
#define SIM_BOOT_DELAY_MS    3000   // delai typique avant que le module reponde a AT

static HardwareSerial simSerial(1);          // UART1 dedie au module 4G
static TinyGsm         modem(simSerial);

// --- Etat interne de la sequence de mise sous tension (non-bloquant) ---
enum class SimPowerState { IDLE, PULSING, WAIT_BOOT, READY };
static SimPowerState powerState      = SimPowerState::IDLE;
static uint32_t       powerStateStart = 0;

// --- Etat interne de sim_wait_network_ready (non-bloquant) ---
static bool     networkWaiting  = false;
static bool     networkTimedOut = false;
static uint32_t networkStartMs  = 0;

bool sim_power_on() {
    switch (powerState) {

<<<<<<< Updated upstream
    case SimPowerState::IDLE:
        simSerial.begin(115200, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
        pinMode(SIM_PWRKEY_PIN, OUTPUT);
        digitalWrite(SIM_PWRKEY_PIN, HIGH); // debut de l'impulsion PWRKEY
        powerStateStart = millis();
        powerState = SimPowerState::PULSING;
        networkWaiting = networkTimedOut = false; // nouveau cycle : on reinitialise
        return false;                             // pas encore pret, rappeler au prochain tour

    case SimPowerState::PULSING:
        if (millis() - powerStateStart >= SIM_PWRKEY_PULSE_MS)
        {
            digitalWrite(SIM_PWRKEY_PIN, LOW); // fin de l'impulsion
=======
        case SimPowerState::IDLE:
            simSerial.begin(115200, SERIAL_8N1, SIM_UART_RX, SIM_UART_TX);
            pinMode(SIM_PWRKEY_PIN, OUTPUT);
            digitalWrite(SIM_PWRKEY_PIN, HIGH);      // debut de l'impulsion PWRKEY
>>>>>>> Stashed changes
            powerStateStart = millis();
            powerState = SimPowerState::PULSING;
            networkWaiting = networkTimedOut = false; // nouveau cycle : on reinitialise
            return false;                              // pas encore pret, rappeler au prochain tour

        case SimPowerState::PULSING:
            if (millis() - powerStateStart >= SIM_PWRKEY_PULSE_MS) {
                digitalWrite(SIM_PWRKEY_PIN, LOW);   // fin de l'impulsion
                powerStateStart = millis();
                powerState = SimPowerState::WAIT_BOOT;
            }
            return false;

        case SimPowerState::WAIT_BOOT:
            if (millis() - powerStateStart >= SIM_BOOT_DELAY_MS) {
                powerState = SimPowerState::READY;
            }
            return false;

        case SimPowerState::READY:
            // NOTE : modem.testAT() contient une attente interne bornee propre
            // a TinyGSM (limitation de la bibliotheque, pas de notre code ici).
            return modem.testAT();
    }
    return false;
}

bool sim_power_off() {
    bool ok = modem.poweroff();
    powerState = SimPowerState::IDLE;   // permet un futur sim_power_on() propre
    return ok;
}

bool sim_wait_network_ready(uint32_t timeout_ms) {
    if (networkTimedOut) return false;   // deja abandonne : sim_power_on() reinitialise ce drapeau

    if (!networkWaiting) {
        networkStartMs = millis();
        networkWaiting = true;
    }

    // NOTE : isNetworkConnected() interroge l'etat deja connu du modem
    // (pas de nouvelle commande AT bloquante emise ici).
    if (modem.isNetworkConnected()) {
        networkWaiting = false;
        return true;
    }

    if (millis() - networkStartMs >= timeout_ms) {
        networkTimedOut = true;
        networkWaiting  = false;
        return false;   // abandon : la FSM appelante doit decider de la suite (retry, erreur...)
    }

    return false;   // encore en attente : rappeler au prochain tour de loop()
}

TinyGsm& sim_get_modem() {
    return modem;
}