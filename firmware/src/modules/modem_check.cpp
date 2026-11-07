#include "hal/modem_check.h"
#include "system_config.h"

#include <Arduino.h>
#include <string.h>

// OUTILS INTERNES

// Envoie une commande AT et renvoie toute la réponse brute.
// S'arrête sur "OK\r\n" final, sur "ERROR", ou au timeout.
static String atQuery(HardwareSerial &s,
                      const char *cmd,
                      uint32_t timeoutMs = 2000)
{
  // On vide ce qui reste d'une réponse précédente.
  while (s.available())
    s.read();

  s.print(cmd);
  s.print("\r\n");

  String resp;
  uint32_t start = millis();

  while (millis() - start < timeoutMs)
  {
    modem_feed_watchdog();

    while (s.available())
      resp += static_cast<char>(s.read());

    if (resp.endsWith("OK\r\n") || resp.indexOf("ERROR") >= 0)
      break;

    delay(5);
  }

  return resp;
}

// "+CSQ: 21,99" -> 21
static int parseFirstInt(const String &resp, const char *prefix)
{
  int p = resp.indexOf(prefix);

  if (p < 0)
    return -1;

  return resp.substring(p + strlen(prefix)).toInt();
}

// "+CREG: 0,1" -> 1
static int parseSecondInt(const String &resp, const char *prefix)
{
  int p = resp.indexOf(prefix);

  if (p < 0)
    return -1;

  int comma = resp.indexOf(',', p);

  if (comma < 0)
    return -1;

  return resp.substring(comma + 1).toInt();
}

// ETAPES DE VERIFICATION

static bool checkAlive(HardwareSerial &s)
{
  for (int i = 0; i < 5; i++)
  {
    if (atQuery(s, "AT", 1000).indexOf("OK") >= 0)
    {
      atQuery(s, "ATE0"); // désactive l'écho
      return true;
    }

    delay(500);
  }

  return false;
}

static bool checkSim(HardwareSerial &s)
{
  for (int i = 0; i < 3; i++)
  {
    if (atQuery(s, "AT+CPIN?").indexOf("+CPIN: READY") >= 0)
      return true;

    delay(500);
  }

  return false;
}

static bool isRegistered(HardwareSerial &s)
{
  // Selon la technologie (2G/3G/4G), c'est l'un des trois registres.
  static const char *const commands[] = {"AT+CREG?", "AT+CGREG?", "AT+CEREG?"};
  static const char *const prefixes[] = {"+CREG:", "+CGREG:", "+CEREG:"};

  for (int i = 0; i < 3; i++)
  {
    int stat = parseSecondInt(atQuery(s, commands[i]), prefixes[i]);

    // 1 = réseau d'origine, 5 = itinérance
    if (stat == 1 || stat == 5)
      return true;
  }

  return false;
}

static bool waitRegistered(HardwareSerial &s, uint32_t timeoutMs)
{
  uint32_t start = millis();

  while (millis() - start < timeoutMs)
  {
    if (isRegistered(s))
      return true;

    Serial.println("[CHECK] Attente enregistrement reseau...");

    for (int i = 0; i < 20; i++) // 2 s, en nourrissant le watchdog
    {
      modem_feed_watchdog();
      delay(100);
    }
  }

  return false;
}

static bool hasIp(HardwareSerial &s)
{
  String resp = atQuery(s, "AT+CGPADDR=1");

  int p = resp.indexOf("+CGPADDR:");

  if (p < 0)
    return false;

  int comma = resp.indexOf(',', p);

  if (comma < 0)
    return false;

  String ip = resp.substring(comma + 1);

  // Ne garder que la première ligne.
  int newline = ip.indexOf('\n');

  if (newline >= 0)
    ip = ip.substring(0, newline);

  ip.trim();

  return ip.length() >= 7 && !ip.startsWith("0.0.0.0");
}

static bool ensurePdp(HardwareSerial &s)
{
  if (hasIp(s))
    return true;

  Serial.println("[CHECK] Pas d'IP, activation du contexte data...");

  char command[96];

  snprintf(command, sizeof(command),
           "AT+CGDCONT=1,\"IP\",\"%s\"", SIM_APN);

  atQuery(s, command);
  atQuery(s, "AT+CGACT=1,1", 10000);

  for (int i = 0; i < 5; i++)
  {
    if (hasIp(s))
      return true;

    delay(1000);
  }

  return false;
}

// Renvoie l'année à 2 chiffres (26 = 2026), ou -1 si illisible.
static int clockYear(HardwareSerial &s)
{
  String resp = atQuery(s, "AT+CCLK?");

  int p = resp.indexOf("+CCLK: \"");

  if (p < 0)
    return -1;

  return resp.substring(p + 8).toInt();
}

static void syncClock(HardwareSerial &s)
{
  Serial.println("[CHECK] Horloge invalide, synchronisation NTP...");

  atQuery(s, "AT+CTZU=1");
  atQuery(s, "AT+CNTP=\"pool.ntp.org\",0");

  while (s.available())
    s.read();

  s.print("AT+CNTP\r\n");

  String resp;
  uint32_t start = millis();

  // La synchronisation répond "+CNTP: 0" en cas de succès.
  while (millis() - start < 20000)
  {
    modem_feed_watchdog();

    while (s.available())
      resp += static_cast<char>(s.read());

    if (resp.indexOf("+CNTP:") >= 0 || resp.indexOf("ERROR") >= 0)
      break;

    delay(10);
  }
}

static bool checkClock(HardwareSerial &s)
{
  if (clockYear(s) >= 24)
    return true;

  syncClock(s);

  return clockYear(s) >= 24;
}

static bool checkCaCert(HardwareSerial &s, const char *caFile)
{
  String resp = atQuery(s, "AT+CCERTLIST", 3000);

  return resp.indexOf(caFile) >= 0;
}

// FONCTIONS PUBLIQUES

bool modem_check_all(HardwareSerial &serial,
                     const char *caFile,
                     ModemStatus &status)
{
  status = ModemStatus();

  Serial.println("\n--- VERIFICATION MODEM ---");

  // 1. Le module répond-il ?
  status.alive = checkAlive(serial);

  if (!status.alive)
  {
    Serial.println("[CHECK] Le module ne repond pas (alimentation, UART, PWR).");
    return false;
  }

  // 2. SIM prête
  status.simReady = checkSim(serial);

  if (!status.simReady)
  {
    Serial.println("[CHECK] SIM non prete (absente, code PIN, ou mal inseree).");
    return false;
  }

  // 3. Enregistrement réseau
  status.registered = waitRegistered(serial, 30000);

  if (!status.registered)
  {
    Serial.println("[CHECK] Pas d'enregistrement reseau (couverture, antenne, forfait).");
    return false;
  }

  // 4. Qualité du signal
  status.rssi = parseFirstInt(atQuery(serial, "AT+CSQ"), "+CSQ:");

  if (status.rssi == 99 || status.rssi < 5)
  {
    Serial.print("[CHECK] Signal insuffisant (CSQ=");
    Serial.print(status.rssi);
    Serial.println(").");
    return false;
  }

  // 5. Contexte data / adresse IP
  status.pdpActive = ensurePdp(serial);

  if (!status.pdpActive)
  {
    Serial.println("[CHECK] Aucune adresse IP (verifier l'APN et le forfait data).");
    return false;
  }

  // 6. Horloge (non bloquante)
  status.clockValid = checkClock(serial);

  // 7. Certificat CA présent dans le module
  status.caCertPresent = checkCaCert(serial, caFile);

  if (!status.caCertPresent)
  {
    Serial.print("[CHECK] Certificat CA absent du module : ");
    Serial.println(caFile);
    Serial.println("[CHECK] Chargez-le avec AT+CCERTDOWN (script Python).");
    return false;
  }

  modem_check_print(status);

  return true;
}

void modem_check_print(const ModemStatus &status)
{
  Serial.println("[CHECK] ---------------------------");
  Serial.print("[CHECK] Module repond    : ");
  Serial.println(status.alive ? "OK" : "NON");
  Serial.print("[CHECK] SIM prete        : ");
  Serial.println(status.simReady ? "OK" : "NON");
  Serial.print("[CHECK] Reseau           : ");
  Serial.println(status.registered ? "OK" : "NON");
  Serial.print("[CHECK] Signal (CSQ)     : ");
  Serial.println(status.rssi);
  Serial.print("[CHECK] Adresse IP       : ");
  Serial.println(status.pdpActive ? "OK" : "NON");
  Serial.print("[CHECK] Horloge          : ");
  Serial.println(status.clockValid ? "OK" : "INVALIDE (verification de date TLS desactivee)");
  Serial.print("[CHECK] Certificat CA    : ");
  Serial.println(status.caCertPresent ? "OK" : "ABSENT");
  Serial.println("[CHECK] ---------------------------");
}