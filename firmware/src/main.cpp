#include <Arduino.h>
#include <esp_partition.h>

#include "badge_map.h"
#include "hal/buzzer.h"
#include "hal/sim_manager.h"
#include "hal/mqtt_manager.h"
#include "hal/rfid_pn532.h"
#include "hal/rtc_ds3231.h"
#include "log_format.h"
#include "log_sync.h"
#include "pins_config.h"
#include "system_config.h"
#include "time_sync.h"

// Commandes sur le moniteur série :
//   'l' + Entrée : crée un log de test (agent AG001) avec l'heure du DS3231
//   's' + Entrée : affiche le nombre de logs par statut
//   't' + Entrée : affiche l'heure du DS3231 (UTC et locale)
//   'b' + Entrée : fait sonner le buzzer ("pip! pip!")
//   'T' + Entrée : force une resynchronisation de l'heure avec le réseau
//   'C' + Entrée : EFFACE tous les logs (majuscule, pour éviter une erreur)
// À mettre à 0 une fois les tests terminés (la lecture RFID reste active).
#define ENABLE_SERIAL_TEST_LOG 1

// Anti-rebond RFID : un badge maintenu sur le lecteur, ou présenté deux fois
// de suite, ne crée qu'un seul log pendant ce délai.
#ifndef RFID_DEBOUNCE_MS
#define RFID_DEBOUNCE_MS 3000
#endif

// Bip long quand un passage est refusé (badge inconnu) ou non enregistré.
// Mettre à 0 pour ne faire sonner que les passages valides.
#ifndef BUZZER_SIGNAL_ERRORS
#define BUZZER_SIGNAL_ERRORS 1
#endif

// Resynchronisation de l'heure : toutes les 24 h, et 10 min après un échec.
#ifndef CLOCK_RESYNC_INTERVAL_MS
#define CLOCK_RESYNC_INTERVAL_MS (24UL * 60UL * 60UL * 1000UL)
#endif
#ifndef CLOCK_RETRY_INTERVAL_MS
#define CLOCK_RETRY_INTERVAL_MS (10UL * 60UL * 1000UL)
#endif

// ============================================================
// ETATS DU SYSTEME
// ============================================================

enum class SystemState
{
  START_MODEM,
  INIT_MODEM,
  CONNECT_NETWORK,
  CONNECT_INTERNET,
  SYNC_TIME,
  INIT_MQTT,
  CONNECT_MQTT,
  RUNNING,
  ERROR_STATE
};

static SystemState systemState = SystemState::START_MODEM;

static uint32_t lastStatusMessage = 0;

// ============================================================
// SUIVI MQTT
// ============================================================

// Intervalle de contrôle d'Internet quand MQTT est coupé.
#define INTERNET_CHECK_INTERVAL_MS 10000

// Pause entre l'arrêt et le redémarrage complet du modem.
#define MODEM_RESTART_DELAY_MS 5000

// CONNECT_MQTT : échecs consécutifs et prochaine tentative (non bloquant).
static int mqttFailures = 0;
static uint32_t nextMqttAttemptMs = 0;

// RUNNING : MQTT coupé depuis quand ?
static bool mqttDown = false;
static uint32_t mqttDownSinceMs = 0;
static uint32_t lastInternetCheckMs = 0;

// ERROR_STATE : arrêt du modem déjà effectué ?
static bool errorHandled = false;
static uint32_t errorStartMs = 0;

static uint32_t modemRestartDelayMs = MODEM_RESTART_DELAY_MS;

// ============================================================
// MATERIEL : STOCKAGE, HORLOGE, RFID
// ============================================================

// false si LittleFS n'a pas pu être monté
static bool storageReady = false;

// false si le DS3231 ne répond pas sur le bus I2C
static bool rtcReady = false;

// false si le PN532 n'a pas été détecté
static bool rfidReady = false;

// Prochaine synchronisation de l'heure (comparaison insensible au
// débordement de millis()).
static uint32_t nextClockSyncMs = 0;

// Dernier badge lu (anti-rebond)
static char lastUid[RFID_UID_MAX_STR_LEN] = "";
static uint32_t lastUidMs = 0;

// Liste les partitions de la flash. Sert à diagnostiquer un échec de
// montage : LittleFS cherche la partition
static void printPartitions()
{
  Serial.println("[PART] Partitions presentes dans la flash :");

  esp_partition_iterator_t it = esp_partition_find(
      ESP_PARTITION_TYPE_ANY,
      ESP_PARTITION_SUBTYPE_ANY,
      NULL);

  while (it != NULL)
  {
    const esp_partition_t *p = esp_partition_get(it);

    Serial.printf(
        "[PART]   %-10s type=%d sous-type=0x%02x debut=0x%06x taille=%u Ko\n",
        p->label,
        static_cast<int>(p->type),
        static_cast<unsigned int>(p->subtype),
        static_cast<unsigned int>(p->address),
        static_cast<unsigned int>(p->size / 1024));

    it = esp_partition_next(it);
  }

  esp_partition_iterator_release(it);
}

// ============================================================
// ENREGISTREMENT D'UN PASSAGE (badge)
// ============================================================

// Enregistre un passage dans LittleFS avec l'heure RÉELLE du DS3231.
static bool recordScan(const char *agentId)
{
  if (!storageReady)
  {
    Serial.println("[RFID] Stockage indisponible : passage NON enregistre.");
    return false;
  }

  // ATTENTION : rtc_now_iso8601() exige un tampon d'au moins
  // RTC_ISO8601_BUF_SIZE (25) octets, sinon elle renvoie false.
  char timestamp[RTC_ISO8601_BUF_SIZE];

  bool timeOk = rtcReady && rtc_now_iso8601(timestamp, sizeof(timestamp));

  if (!timeOk)
  {
    // Heure inconnue : on enregistre quand même le passage. Le serveur
    // reconnaît une heure inconnue au timestamp 1970.
    snprintf(timestamp, sizeof(timestamp), "1970-01-01T00:00:00Z");

    Serial.println("[RFID] ATTENTION : heure DS3231 invalide, passage horodate 1970.");
  }

  uint32_t lineIndex = 0;

  if (storage_append_log(timestamp, agentId, CHECKPOINT_ID, &lineIndex))
  {
    Serial.print("[RFID] Passage enregistre : ");
    Serial.print(timestamp);
    Serial.print("  agent=");
    Serial.print(agentId);
    Serial.print("  point=");
    Serial.print(CHECKPOINT_ID);
    Serial.print("  (ligne ");
    Serial.print(lineIndex);
    Serial.println(")");

    // Heure locale, pour la lecture sur le moniteur (le log reste en UTC).
    char local[TIME_LOCAL_STR_LEN];

    if (timeOk && time_utc_iso_to_local(timestamp, local, sizeof(local)))
    {
      Serial.print("[RFID] Heure locale : ");
      Serial.println(local);
    }

    // Prévient le module de synchronisation.
    log_sync_request();

    return true;
  }

  Serial.println("[RFID] Echec de l'enregistrement du passage.");

  return false;
}

// Lecture du badge, appelée quel que soit l'état du réseau : les passages sont enregistrés même hors ligne.
static void handleRfid()
{
  if (!rfidReady)
    return;

  if (!RFID_IsCardPresent())
    return;

  char uid[RFID_UID_MAX_STR_LEN];

  if (!RFID_ReadUID(uid, sizeof(uid)))
    return;

  uint32_t now = millis();

  bool sameBadge = (strcmp(uid, lastUid) == 0);
  bool recent = (now - lastUidMs) < RFID_DEBOUNCE_MS;

  // Un badge maintenu sur le lecteur prolonge l'anti-rebond.
  lastUidMs = now;

  if (sameBadge && recent)
    return;

  strncpy(lastUid, uid, sizeof(lastUid) - 1);
  lastUid[sizeof(lastUid) - 1] = '\0';

  Serial.print("[RFID] UID lu : ");
  Serial.println(uid);

  char agentId[LOG_FIELD_AGENT_LEN + 1];

  bool known = badge_agent_id(uid, agentId);

  if (!known)
  {
    Serial.print("[RFID] Badge inconnu, agent note : ");
    Serial.println(agentId);
  }

  bool stored = recordScan(agentId);

  // "pip! pip!" : badge connu ET passage enregistré.
  // Bip long : badge inconnu (enregistré quand même) ou enregistrement raté.
  if (known && stored)
    buzzer_valid();
#if BUZZER_SIGNAL_ERRORS
  else
    buzzer_error();
#endif
}

// ============================================================
// COMMANDES DE TEST (moniteur série)
// ============================================================

#if ENABLE_SERIAL_TEST_LOG

static void printStorageStats()
{
  if (!storageReady)
  {
    Serial.println("[TEST] Stockage indisponible (LittleFS non monte).");
    return;
  }

  Serial.print("[TEST] EN_ATTENTE : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_PENDING)));

  Serial.print("[TEST] ENVOYE     : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_SENT)));

  Serial.print("[TEST] ECHEC      : ");
  Serial.println(static_cast<unsigned int>(storage_count_by_status(LOG_STATUS_FAILED)));
}

static void printRtcTime()
{
  char buf[RTC_ISO8601_BUF_SIZE];

  if (rtcReady && rtc_now_iso8601(buf, sizeof(buf)))
  {
    Serial.print("[TEST] DS3231 (UTC)  : ");
    Serial.println(buf);

    char local[TIME_LOCAL_STR_LEN];

    if (time_utc_iso_to_local(buf, local, sizeof(local)))
    {
      Serial.print("[TEST] Heure locale  : ");
      Serial.println(local);
    }
  }
  else
  {
    Serial.println("[TEST] DS3231 : heure invalide ou module absent.");
  }
}

// Efface tous les logs (test uniquement).
static void clearAllLogs()
{
  if (!storageReady)
  {
    Serial.println("[TEST] Stockage indisponible (LittleFS non monte).");
    return;
  }

  // D'abord on abandonne l'envoi en cours : le lot gardé en mémoire par
  // log_sync ne correspond plus au fichier une fois celui-ci effacé.
  log_sync_reset();

  if (clear_storage_logs())
    Serial.println("[TEST] Tous les logs ont ete effaces.");
  else
    Serial.println("[TEST] Echec de l'effacement des logs.");
}

static void handleSerialTestCommands()
{
  while (Serial.available())
  {
    char c = Serial.read();

    if (c == 'l')
    {
      if (recordScan("AG001"))
        buzzer_valid();
    }
    else if (c == 'b')
    {
      buzzer_valid();
    }
    else if (c == 's')
    {
      printStorageStats();
    }
    else if (c == 't')
    {
      printRtcTime();
    }
    else if (c == 'T')
    {
      // Traité en état RUNNING, quand aucun envoi n'est en cours.
      Serial.println("[TEST] Resynchronisation de l'heure demandee.");
      nextClockSyncMs = millis();
    }
    else if (c == 'C')
    {
      clearAllLogs();
    }
  }
}

#endif

// ============================================================
// INITIALISATION
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Buzzer éteint dès le démarrage.
  buzzer_init();

  delay(1000);

  Serial.println();
  Serial.println("======================================");
  Serial.println("        RONDE DE SECURITE");
  Serial.println("======================================");

  Serial.print("[INFO] Firmware : ");
  Serial.println(FIRMWARE_VERSION);

  Serial.print("[INFO] Device ID : ");
  Serial.println(DEVICE_ID);

  // Les logs sont enregistrés localement même sans réseau.
  storageReady = storage_init();

  if (storageReady)
  {
    Serial.println("[PASS] Stockage LittleFS pret.");
  }
  else
  {
    Serial.println("[ERROR] LittleFS indisponible : logs non enregistres.");
    Serial.println("[ERROR] Verifiez la table de partitions (partition \"spiffs\").");

    printPartitions();
  }

  // Horloge DS3231 (I2C). Le driver utilise les broches I2C par défaut de
  // l'ESP32 (SDA=21, SCL=22), identiques à pins_config.h.
  rtcReady = rtc_init(RTC_SDA_PIN, RTC_SCL_PIN);

  if (!rtcReady)
  {
    Serial.println("[ERROR] DS3231 non detecte (verifier le cablage I2C).");
  }
  else
  {
    char buf[RTC_ISO8601_BUF_SIZE];

    if (rtc_now_iso8601(buf, sizeof(buf)))
    {
      Serial.print("[PASS] DS3231 pret, heure UTC : ");
      Serial.print(buf);

      char local[TIME_LOCAL_STR_LEN];

      if (time_utc_iso_to_local(buf, local, sizeof(local)))
      {
        Serial.print("  |  locale : ");
        Serial.print(local);
      }

      Serial.println();
    }
    else
    {
      Serial.println("[WARN] DS3231 : heure invalide (pile ?). "
                     "Attente de la synchronisation par le reseau mobile.");
    }
  }

  // Lecteur RFID PN532 (SPI).
  rfidReady = RFID_Init(RFID_CS_PIN, RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);

  if (rfidReady)
    Serial.println("[PASS] Lecteur RFID pret.");
  else
    Serial.println("[ERROR] Lecteur RFID indisponible : aucun passage ne sera lu.");
}

// ============================================================
// BOUCLE PRINCIPALE
// ============================================================

void loop()
{
  // Les badges sont lus dans tous les états.
  handleRfid();

#if ENABLE_SERIAL_TEST_LOG
  handleSerialTestCommands();
#endif

  switch (systemState)
  {
    // --------------------------------------------------------
    // 1. DEMARRAGE MODEM
    // --------------------------------------------------------

  case SystemState::START_MODEM:

    if (sim_power_on())
    {
      Serial.println("[PASS] Modem operationnel.");

      systemState = SystemState::INIT_MODEM;
    }

    break;

    // --------------------------------------------------------
    // 2. INITIALISATION TINYGSM
    // --------------------------------------------------------

  case SystemState::INIT_MODEM:

    if (sim_init_modem())
    {
      Serial.println("[PASS] Modem initialise.");

      systemState = SystemState::CONNECT_NETWORK;
    }
    else
    {
      Serial.println("[WARN] Nouvelle tentative initialisation.");

      delay(2000);
    }

    break;

    // --------------------------------------------------------
    // 3. ATTENTE RESEAU MOBILE
    // --------------------------------------------------------

  case SystemState::CONNECT_NETWORK:

    if (sim_wait_network_ready(SIM_NET_TIMEOUT_MS))
    {
      Serial.println("[PASS] Reseau mobile disponible.");

      systemState = SystemState::CONNECT_INTERNET;
    }

    break;

    // --------------------------------------------------------
    // 4. CONNEXION INTERNET
    // --------------------------------------------------------

  case SystemState::CONNECT_INTERNET:

    if (sim_connect_internet())
    {
      Serial.println("[PASS] Connexion Internet 4G.");

      systemState = SystemState::SYNC_TIME;
    }

    break;

    // --------------------------------------------------------
    // 5. SYNCHRONISATION DE L'HEURE (réseau mobile -> DS3231)
    // --------------------------------------------------------
    // Bloquant (jusqu'à ~45 s). MQTT n'est pas actif ici : on peut parler
    // directement au modem sans risquer de perdre un message du broker.
    // Un échec n'empêche pas la suite : le DS3231 garde son heure.
  case SystemState::SYNC_TIME:

    if (rtcReady)
    {
      if (time_sync_from_network())
      {
        Serial.println("[PASS] Heure synchronisee.");

        nextClockSyncMs = millis() + CLOCK_RESYNC_INTERVAL_MS;
      }
      else
      {
        Serial.println("[WARN] Heure non synchronisee : DS3231 conserve.");

        nextClockSyncMs = millis() + CLOCK_RETRY_INTERVAL_MS;
      }
    }

    systemState = SystemState::INIT_MQTT;

    break;

    // --------------------------------------------------------
    // 6. INITIALISATION MQTT
    // --------------------------------------------------------
  case SystemState::INIT_MQTT:

    mqtt_init();

    // Repart de zéro côté synchronisation (les logs non acquittés
    // restent EN_ATTENTE dans le fichier).
    log_sync_reset();

    Serial.println("[PASS] Gestionnaire MQTT initialise.");

    mqttFailures = 0;
    nextMqttAttemptMs = 0;
    mqttDown = false;

    systemState = SystemState::CONNECT_MQTT;

    break;

    // --------------------------------------------------------
    // 7. CONNEXION HIVEMQ
    // --------------------------------------------------------
  case SystemState::CONNECT_MQTT:

    // Attente non bloquante entre deux tentatives.
    if (static_cast<int32_t>(millis() - nextMqttAttemptMs) < 0)
      break;

    if (mqtt_connect())
    {
      Serial.println("[PASS] Connexion HiveMQ Cloud.");

      mqttFailures = 0;
      mqttDown = false;
      modemRestartDelayMs = MODEM_RESTART_DELAY_MS;

      systemState = SystemState::RUNNING;

      break;
    }

    mqttFailures++;

    Serial.print("[WARN] Connexion MQTT echouee (");
    Serial.print(mqttFailures);
    Serial.print("/");
    Serial.print(MQTT_MAX_RETRIES);
    Serial.println(").");

    if (!sim_is_internet_connected())
    {
      Serial.println("[WARN] Internet perdu : retour a la connexion data.");

      mqttFailures = 0;

      systemState = SystemState::CONNECT_INTERNET;
    }
    else if (mqttFailures >= MQTT_MAX_RETRIES)
    {
      Serial.println("[ERROR] Trop d'echecs MQTT.");

      mqttFailures = 0;

      systemState = SystemState::ERROR_STATE;
    }
    else
    {
      nextMqttAttemptMs = millis() + MQTT_CONNECT_TIMEOUT_MS;
    }

    break;

    // --------------------------------------------------------
    // 8. FONCTIONNEMENT NORMAL
    // --------------------------------------------------------
    // ATTENTE : mqtt_maintain() (reconnexion), log_sync_process()
    // (publication) et time_sync_from_network() sont BLOQUANTS pendant
    // quelques secondes, voire plus : les badges présentés pendant ce temps
    // ne sont lus qu'après. Pour ne jamais en manquer, la lecture RFID doit
    // tourner dans une tâche FreeRTOS séparée.
  case SystemState::RUNNING:

    mqtt_maintain();

    if (mqtt_is_connected())
    {
      mqttDown = false;

      // Publie les logs EN_ATTENTE et attend leur ACK.
      if (storageReady)
        log_sync_process();

      // Resynchronisation de l'heure (périodique, ou demandée par 'T').
      // MQTT est coupé proprement le temps de la synchronisation : on ne
      // parle jamais au modem en direct pendant une session MQTT.
      if (rtcReady && !log_sync_busy() &&
          static_cast<int32_t>(millis() - nextClockSyncMs) >= 0)
      {
        Serial.println("[TIME] Resynchronisation : deconnexion MQTT temporaire.");

        mqtt_disconnect();

        systemState = SystemState::SYNC_TIME;

        break;
      }
    }
    else
    {
      // Connexion perdue : on abandonne l'envoi en cours. Les logs non
      // acquittés restent EN_ATTENTE et seront renvoyés à la reconnexion.
      log_sync_reset();

      if (!mqttDown)
      {
        mqttDown = true;
        mqttDownSinceMs = millis();
        lastInternetCheckMs = millis();
      }

      // Contrôle d'Internet UNIQUEMENT quand MQTT est coupé : TinyGSM
      // et MQTT partagent le même UART, et TinyGSM lirait (et perdrait)
      // les messages du broker s'il interrogeait le modem pendant un
      // échange MQTT.
      if (millis() - lastInternetCheckMs >= INTERNET_CHECK_INTERVAL_MS)
      {
        lastInternetCheckMs = millis();

        if (!sim_is_internet_connected())
        {
          Serial.println("[WARN] Internet perdu : retour a la connexion data.");

          mqttDown = false;

          systemState = SystemState::CONNECT_INTERNET;

          break;
        }
      }

      // Coupure trop longue : redémarrage complet du modem.
      if (millis() - mqttDownSinceMs >= NETWORK_GLOBAL_TIMEOUT_MS)
      {
        Serial.println("[ERROR] MQTT indisponible trop longtemps.");

        mqttDown = false;

        systemState = SystemState::ERROR_STATE;

        break;
      }
    }

    if (millis() - lastStatusMessage >= 10000)
    {
      lastStatusMessage = millis();

      Serial.print("[SYSTEM] MQTT : ");

      Serial.print(mqtt_is_connected() ? "CONNECTE" : "DECONNECTE");

      char now[RTC_ISO8601_BUF_SIZE];

      if (rtcReady && rtc_now_iso8601(now, sizeof(now)))
      {
        char local[TIME_LOCAL_STR_LEN];

        Serial.print("  |  UTC : ");
        Serial.print(now);

        if (time_utc_iso_to_local(now, local, sizeof(local)))
        {
          Serial.print("  |  Locale : ");
          Serial.print(local);
        }

        Serial.println();
      }
      else
      {
        Serial.println("  |  Heure : invalide");
      }
    }

    break;

    // --------------------------------------------------------
    // 9. ERREUR : REDEMARRAGE COMPLET DU MODEM
    // --------------------------------------------------------

  case SystemState::ERROR_STATE:

    if (!errorHandled)
    {
      Serial.println("[ERROR] Arret du modem pour redemarrage complet.");

      sim_power_off();

      errorHandled = true;
      errorStartMs = millis();
    }
    else if (millis() - errorStartMs >= modemRestartDelayMs)
    {
      Serial.println("[INFO] Redemarrage du modem.");

      errorHandled = false;

      modemRestartDelayMs *= 2;

      if (modemRestartDelayMs > NETWORK_GLOBAL_TIMEOUT_MS)
        modemRestartDelayMs = NETWORK_GLOBAL_TIMEOUT_MS;

      systemState = SystemState::START_MODEM;
    }

    break;
  }

  delay(5);
}