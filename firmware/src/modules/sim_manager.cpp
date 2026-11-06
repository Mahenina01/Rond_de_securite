#include "hal/sim_manager.h"
#include "pins_config.h"
#include "system_config.h"

static HardwareSerial simSerial(SIM_UART_NUM);
static TinyGsm modem(simSerial);

// ETATS INTERNES
enum class SimPowerState
{
    IDLE,
    PULSING,
    WAIT_BOOT,
    READY
};

static SimPowerState powerState = SimPowerState::IDLE;

static uint32_t powerStateStart = 0;

static bool modemInitialized = false;
static bool internetConnected = false;

static bool networkWaiting = false;
static uint32_t networkStartMs = 0;

// COMMANDE ALIMENTATION MOSFET

// IMPORTANT :
// Valeur par défaut : LOW = alimentation active.
// À confirmer avec le schéma électrique de Q2.
// Si ton circuit est inversé, change cette polarité.

#ifndef SIM_POWER_ENABLE_ACTIVE_LEVEL
#define SIM_POWER_ENABLE_ACTIVE_LEVEL LOW
#endif

static void sim_enable_power()
{
    pinMode(SIM_POWER_ENABLE_PIN, OUTPUT);

    digitalWrite(
        SIM_POWER_ENABLE_PIN,
        SIM_POWER_ENABLE_ACTIVE_LEVEL);
}

static void sim_disable_power()
{
    digitalWrite(
        SIM_POWER_ENABLE_PIN,
        !SIM_POWER_ENABLE_ACTIVE_LEVEL);
}

// DEMARRAGE MODEM
bool sim_power_on()
{
    switch (powerState)
    {
    case SimPowerState::IDLE:

        Serial.println("[SIM] Activation alimentation...");

        sim_enable_power();

        delay(100);

        simSerial.begin(
            SIM_BAUDRATE,
            SERIAL_8N1,
            SIM_RX_PIN,
            SIM_TX_PIN);

        pinMode(SIM_PWRKEY_PIN, OUTPUT);

        // PWRKEY actif à l'état LOW.
        digitalWrite(SIM_PWRKEY_PIN, HIGH);

        delay(100);

        digitalWrite(SIM_PWRKEY_PIN, LOW);

        powerStateStart = millis();

        powerState = SimPowerState::PULSING;

        modemInitialized = false;
        internetConnected = false;

        networkWaiting = false;

        Serial.println("[SIM] Impulsion PWRKEY envoyee.");

        return false;

    case SimPowerState::PULSING:

        if (millis() - powerStateStart >= SIM_PWRKEY_PULSE_MS)
        {
            digitalWrite(SIM_PWRKEY_PIN, HIGH);

            powerStateStart = millis();

            powerState = SimPowerState::WAIT_BOOT;

            Serial.println("[SIM] PWRKEY relache.");
        }

        return false;

    case SimPowerState::WAIT_BOOT:

        if (millis() - powerStateStart >= SIM_BOOT_DELAY_MS)
        {
            if (modem.testAT())
            {
                powerState = SimPowerState::READY;

                Serial.println("[SIM] Modem operationnel.");

                return true;
            }

            // Ne pas redémarrer l'impulsion PWRKEY ici :
            // le modem peut simplement nécessiter plus de temps.
            return false;
        }

        return false;

    case SimPowerState::READY:

        return modem.testAT();
    }

    return false;
}

// INITIALISATION TINYGSM
bool sim_init_modem()
{
    if (powerState != SimPowerState::READY)
    {
        Serial.println("[SIM] Modem non disponible.");
        return false;
    }

    if (modemInitialized)
        return true;

    Serial.println("[SIM] Initialisation TinyGSM...");

    if (!modem.init())
    {
        Serial.println("[SIM] Echec initialisation TinyGSM.");
        return false;
    }

    modemInitialized = true;

    Serial.println("[SIM] TinyGSM initialise.");

    return true;
}

// ATTENTE RESEAU MOBILE
bool sim_wait_network_ready(uint32_t timeout_ms)
{
    if (!modemInitialized)
        return false;

    if (modem.isNetworkConnected())
    {
        networkWaiting = false;
        return true;
    }

    if (!networkWaiting)
    {
        networkStartMs = millis();
        networkWaiting = true;

        Serial.println("[SIM] Attente reseau mobile...");
    }

    if (millis() - networkStartMs >= timeout_ms)
    {
        networkWaiting = false;

        Serial.println("[SIM] Timeout reseau.");

        return false;
    }

    return false;
}

// CONNEXION INTERNET
bool sim_connect_internet()
{
    if (!modemInitialized)
        return false;

    if (internetConnected && modem.isGprsConnected())
        return true;

    Serial.println("[SIM] Connexion Internet...");

    if (!sim_wait_network_ready(SIM_NET_TIMEOUT_MS))
    {
        Serial.println("[SIM] Reseau mobile indisponible.");
        return false;
    }

    if (!modem.gprsConnect(SIM_APN, SIM_USER, SIM_PASS))
    {
        Serial.println("[SIM] Connexion APN echouee.");
        return false;
    }

    internetConnected = modem.isGprsConnected();

    if (internetConnected)
    {
        Serial.print("[SIM] IP : ");
        Serial.println(modem.getLocalIP());

        Serial.println("[SIM] Internet disponible.");
    }

    return internetConnected;
}

// ETAT INTERNET
bool sim_is_internet_connected()
{
    return modemInitialized &&
           internetConnected &&
           modem.isGprsConnected();
}

// ARRET MODEM
bool sim_power_off()
{
    Serial.println("[SIM] Arret modem...");

    bool ok = false;

    if (modemInitialized)
    {
        ok = modem.poweroff();
    }

    digitalWrite(SIM_PWRKEY_PIN, HIGH);

    sim_disable_power();

    simSerial.end();

    powerState = SimPowerState::IDLE;

    modemInitialized = false;
    internetConnected = false;
    networkWaiting = false;

    return ok;
}

// ACCES AUX OBJETS
TinyGsm &sim_get_modem()
{
    return modem;
}

HardwareSerial &sim_get_serial()
{
    return simSerial;
}