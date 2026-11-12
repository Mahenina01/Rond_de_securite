#include "time_sync.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hal/modem_check.h" // modem_feed_watchdog()
#include "hal/rtc_ds3231.h"
#include "hal/sim_manager.h"

// Pas de String ni d'allocation dynamique (règle du projet).

namespace
{

  // CONVERSIONS DE DATE (indépendantes du fuseau TZ de l'ESP32)
  //
  // Algorithmes "days from civil" / "civil from days" (Howard Hinnant,
  // domaine public). Valables pour toutes les dates, années bissextiles
  // comprises.

  struct UtcTime
  {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
  };

  int64_t daysFromCivil(int64_t y, int64_t m, int64_t d)
  {
    y -= (m <= 2);

    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const int64_t yoe = y - era * 400;
    const int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

    return era * 146097 + doe - 719468;
  }

  int64_t toEpoch(const UtcTime &t)
  {
    return daysFromCivil(t.year, t.month, t.day) * 86400 +
           static_cast<int64_t>(t.hour) * 3600 +
           static_cast<int64_t>(t.minute) * 60 +
           t.second;
  }

  UtcTime fromEpoch(int64_t epoch)
  {
    int64_t days = epoch / 86400;
    int64_t secs = epoch % 86400;

    if (secs < 0)
    {
      secs += 86400;
      days--;
    }

    days += 719468;

    const int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const int64_t doe = days - era * 146097;
    const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int64_t mp = (5 * doy + 2) / 153;
    const int64_t d = doy - (153 * mp + 2) / 5 + 1;
    const int64_t m = (mp < 10) ? mp + 3 : mp - 9;

    UtcTime t;
    t.year = static_cast<int>(yoe + era * 400 + (m <= 2 ? 1 : 0));
    t.month = static_cast<int>(m);
    t.day = static_cast<int>(d);
    t.hour = static_cast<int>(secs / 3600);
    t.minute = static_cast<int>((secs % 3600) / 60);
    t.second = static_cast<int>(secs % 60);

    return t;
  }

  int64_t absolute(int64_t v)
  {
    return (v < 0) ? -v : v;
  }

  bool plausibleYear(const UtcTime &t)
  {
    // Le driver DS3231 refuse les années < 2026 (rtc_sync_from_string).
    return t.year >= 2026 && t.year <= 2099;
  }

  void formatIso(const UtcTime &t, char *out, size_t outLen)
  {
    snprintf(out, outLen, "%04d-%02d-%02dT%02d:%02d:%02dZ",
             t.year, t.month, t.day, t.hour, t.minute, t.second);
  }

  // DIALOGUE AT

  // Envoie une commande et lit la réponse dans `resp`.
  // Arrêt : "OK" ou "ERROR" reçus, ou, si `resultToken` est donné, la ligne
  // complète "<resultToken> ..." reçue (résultat asynchrone), ou timeout.
  void atQuery(HardwareSerial &s,
               const char *command,
               char *resp,
               size_t respSize,
               uint32_t timeoutMs,
               const char *resultToken = nullptr)
  {
    while (s.available())
      s.read();

    s.print(command);
    s.print("\r\n");

    size_t len = 0;
    resp[0] = '\0';

    uint32_t start = millis();

    while (millis() - start < timeoutMs)
    {
      modem_feed_watchdog();

      while (s.available() && len + 1 < respSize)
      {
        resp[len++] = static_cast<char>(s.read());
        resp[len] = '\0';
      }

      if (resultToken != nullptr)
      {
        const char *p = strstr(resp, resultToken);

        if (p != nullptr && strchr(p, '\n') != nullptr)
          break;

        if (strstr(resp, "ERROR") != nullptr)
          break;
      }
      else if (strstr(resp, "OK\r\n") != nullptr ||
               strstr(resp, "ERROR") != nullptr)
      {
        break;
      }

      delay(5);
    }
  }

  // Lit AT+CCLK? et renvoie l'heure UTC.
  //
  // Réponse : +CCLK: "26/11/07,19:41:10+12"
  //   -> heure LOCALE du réseau + décalage en quarts d'heure (+12 = UTC+3).
  // L'heure UTC est obtenue en soustrayant ce décalage.
  bool parseCclk(const char *resp, int64_t *utcEpoch, UtcTime *localOut)
  {
    const char *p = strstr(resp, "+CCLK: \"");

    if (p == nullptr)
      return false;

    p += 8;

    int yy, mo, dd, hh, mi, ss;
    int tz = 0;

    int n = sscanf(p, "%d/%d/%d,%d:%d:%d%d", &yy, &mo, &dd, &hh, &mi, &ss, &tz);

    if (n < 6)
      return false;

    if (mo < 1 || mo > 12 || dd < 1 || dd > 31 ||
        hh < 0 || hh > 23 || mi < 0 || mi > 59 || ss < 0 || ss > 60)
      return false;

    // Année à 2 chiffres : 70-99 -> 19xx (comme POSIX). Un module qui n'a
    // jamais reçu l'heure répond "80/01/06" (1980) : ce n'est PAS 2080.
    int year = (yy < 70) ? 2000 + yy : 1900 + yy;

    UtcTime local = {year, mo, dd, hh, mi, ss};

    if (localOut != nullptr)
      *localOut = local;

    *utcEpoch = toEpoch(local) - static_cast<int64_t>(tz) * 900;

    return true;
  }

  bool readNetworkUtc(HardwareSerial &s, int64_t *utcEpoch)
  {
    char resp[120];

    atQuery(s, "AT+CCLK?", resp, sizeof(resp), 2000);

    // Affiche la ligne brute : permet de vérifier le fuseau sur le terrain.
    const char *line = strstr(resp, "+CCLK:");

    if (line != nullptr)
    {
      char raw[40];
      size_t i = 0;

      while (line[i] != '\0' && line[i] != '\r' && line[i] != '\n' && i + 1 < sizeof(raw))
      {
        raw[i] = line[i];
        i++;
      }

      raw[i] = '\0';

      Serial.print("[TIME] Reseau : ");
      Serial.println(raw);
    }

    UtcTime local;

    if (!parseCclk(resp, utcEpoch, &local))
    {
      Serial.println("[TIME] Heure du reseau illisible.");
      return false;
    }

    UtcTime utc = fromEpoch(*utcEpoch);

    if (!plausibleYear(utc))
    {
      Serial.println("[TIME] Heure du reseau invalide (annee hors plage).");
      return false;
    }

    char iso[RTC_ISO8601_BUF_SIZE];
    formatIso(utc, iso, sizeof(iso));

    Serial.print("[TIME] UTC calcule : ");
    Serial.println(iso);

    return true;
  }

  // Synchronise l'horloge du module via NTP (fuseau 0 = UTC).
  bool ntpSync(HardwareSerial &s, const char *server)
  {
    char command[80];
    char resp[160];

    // Laisse le réseau mettre l'heure à jour (utile pour le repli NITZ).
    atQuery(s, "AT+CTZU=1", resp, sizeof(resp), 2000);

    snprintf(command, sizeof(command), "AT+CNTP=\"%s\",0", server);
    atQuery(s, command, resp, sizeof(resp), 3000);

    if (strstr(resp, "OK") == nullptr)
      return false;

    // "OK" puis "+CNTP: 0" quand la synchronisation est terminée.
    atQuery(s, "AT+CNTP", resp, sizeof(resp), 20000, "+CNTP:");

    const char *result = strstr(resp, "+CNTP:");

    if (result == nullptr)
      return false;

    return atoi(result + 6) == 0;
  }

  // ACCES AU DS3231

  bool readRtcEpoch(int64_t *epoch)
  {
    char buf[RTC_ISO8601_BUF_SIZE];

    if (!rtc_now_iso8601(buf, sizeof(buf)))
      return false;

    UtcTime t;

    if (sscanf(buf, "%4d-%2d-%2dT%2d:%2d:%2d",
               &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second) != 6)
      return false;

    *epoch = toEpoch(t);

    return true;
  }

  // Écrit l'heure UTC dans le DS3231 (si nécessaire).
  // `trusted` : true pour le NTP ; false pour le NITZ (contrôle de cohérence
  // avec le DS3231 quand celui-ci est valide).
  bool applyToRtc(int64_t netEpoch, bool trusted)
  {
    UtcTime net = fromEpoch(netEpoch);

    if (!plausibleYear(net))
      return false;

    int64_t rtcEpoch = 0;
    bool rtcValid = readRtcEpoch(&rtcEpoch);

    if (rtcValid)
    {
      int64_t diff = netEpoch - rtcEpoch;

      if (!trusted && absolute(diff) > CLOCK_NITZ_MAX_DIFF_S)
      {
        Serial.println("[TIME] Heure NITZ trop eloignee du DS3231 : ignoree.");
        return false;
      }

      if (absolute(diff) < CLOCK_TOLERANCE_S)
      {
        Serial.print("[TIME] DS3231 deja a l'heure (ecart ");
        Serial.print(static_cast<long>(diff));
        Serial.println(" s).");
        return true;
      }

      Serial.print("[TIME] DS3231 en ecart de ");
      Serial.print(static_cast<long>(diff));
      Serial.println(" s : recalage.");
    }
    else
    {
      Serial.println("[TIME] DS3231 invalide (pile ?) : reglage initial.");
    }

    char iso[RTC_ISO8601_BUF_SIZE];
    formatIso(net, iso, sizeof(iso));

    if (!rtc_sync_from_string(iso))
    {
      Serial.println("[TIME] Echec d'ecriture du DS3231.");
      return false;
    }

    // Relecture de contrôle.
    int64_t check = 0;

    if (!readRtcEpoch(&check) || absolute(check - netEpoch) > 3)
    {
      Serial.println("[TIME] Le DS3231 n'a pas conserve l'heure ecrite.");
      return false;
    }

    Serial.print("[TIME] DS3231 regle : ");
    Serial.println(iso);

    return true;
  }

  // Affiche l'heure locale du DS3231 après un réglage réussi : permet de
  // comparer avec une horloge fiable (téléphone).
  bool finish(bool ok);

} // namespace

// API PUBLIQUE

bool time_iso_to_epoch(const char *iso, int64_t *epochOut)
{
  if (iso == nullptr || epochOut == nullptr)
    return false;

  const size_t len = strlen(iso);

  if ((len != 19 && len != 20) ||
      (len == 20 && iso[19] != 'Z') ||
      iso[4] != '-' || iso[7] != '-' ||
      iso[10] != 'T' || iso[13] != ':' || iso[16] != ':')
    return false;

  for (size_t i = 0; i < 19; i++)
  {
    if (i != 4 && i != 7 && i != 10 && i != 13 && i != 16 &&
        (iso[i] < '0' || iso[i] > '9'))
      return false;
  }

  UtcTime utc;
  utc.year = (iso[0] - '0') * 1000 + (iso[1] - '0') * 100 +
             (iso[2] - '0') * 10 + (iso[3] - '0');
  utc.month = (iso[5] - '0') * 10 + (iso[6] - '0');
  utc.day = (iso[8] - '0') * 10 + (iso[9] - '0');
  utc.hour = (iso[11] - '0') * 10 + (iso[12] - '0');
  utc.minute = (iso[14] - '0') * 10 + (iso[15] - '0');
  utc.second = (iso[17] - '0') * 10 + (iso[18] - '0');

  if (utc.month < 1 || utc.month > 12 ||
      utc.hour > 23 || utc.minute > 59 || utc.second > 59)
    return false;

  static const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30,
                                         31, 31, 30, 31, 30, 31};
  const bool leapYear = (utc.year % 4 == 0) &&
                        ((utc.year % 100 != 0) || (utc.year % 400 == 0));
  const uint8_t maxDay = (utc.month == 2 && leapYear)
                             ? 29
                             : daysInMonth[utc.month - 1];

  if (utc.day < 1 || utc.day > maxDay)
    return false;

  *epochOut = toEpoch(utc);
  return true;
}

bool time_epoch_to_iso(int64_t epoch, char *out, size_t outLen)
{
  if (out == nullptr || outLen == 0)
    return false;

  out[0] = '\0';

  const int64_t minEpoch = daysFromCivil(0, 1, 1) * 86400;
  const int64_t maxEpoch = daysFromCivil(10000, 1, 1) * 86400;

  if (outLen < 21 || epoch < minEpoch || epoch >= maxEpoch)
    return false;

  UtcTime utc = fromEpoch(epoch);

  int written = snprintf(out, outLen, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                         utc.year, utc.month, utc.day,
                         utc.hour, utc.minute, utc.second);

  return written == 20 && static_cast<size_t>(written) < outLen;
}

bool time_utc_iso_to_local(const char *utcIso, char *out, size_t outLen)
{
  if (utcIso == nullptr || out == nullptr || outLen < TIME_LOCAL_STR_LEN)
    return false;

  UtcTime utc;

  if (sscanf(utcIso, "%4d-%2d-%2dT%2d:%2d:%2d",
             &utc.year, &utc.month, &utc.day,
             &utc.hour, &utc.minute, &utc.second) != 6)
    return false;

  UtcTime local = fromEpoch(toEpoch(utc) + static_cast<int64_t>(LOCAL_UTC_OFFSET_MINUTES) * 60);

  // Étiquette du fuseau : "UTC+3", "UTC-5", "UTC+5:30".
  char zone[16];
  int offset = LOCAL_UTC_OFFSET_MINUTES;
  int absOffset = (offset < 0) ? -offset : offset;

  if (absOffset % 60 == 0)
    snprintf(zone, sizeof(zone), "UTC%c%d", offset < 0 ? '-' : '+', absOffset / 60);
  else
    snprintf(zone, sizeof(zone), "UTC%c%d:%02d", offset < 0 ? '-' : '+', absOffset / 60, absOffset % 60);

  snprintf(out, outLen, "%04d-%02d-%02d %02d:%02d:%02d (%s)",
           local.year, local.month, local.day,
           local.hour, local.minute, local.second, zone);

  return true;
}

bool time_sync_from_network(void)
{
  HardwareSerial &s = sim_get_serial();

  Serial.println("\n--- SYNCHRONISATION DE L'HEURE ---");

  int64_t epoch = 0;

  static const char *const servers[] = {"pool.ntp.org", "time.google.com"};

  // 1. NTP
  for (size_t i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
  {
    if (ntpSync(s, servers[i]) && readNetworkUtc(s, &epoch))
    {
      Serial.print("[TIME] Source : NTP (");
      Serial.print(servers[i]);
      Serial.println(")");

      return finish(applyToRtc(epoch, true));
    }

    Serial.print("[TIME] NTP indisponible via ");
    Serial.println(servers[i]);
  }

  // 2. Repli : heure diffusée par le réseau mobile (NITZ).
  if (readNetworkUtc(s, &epoch))
  {
    Serial.println("[TIME] Source : heure du reseau mobile (NITZ)");

    return finish(applyToRtc(epoch, false));
  }

  Serial.println("[TIME] Aucune heure fiable obtenue du reseau.");

  return false;
}

namespace
{

  bool finish(bool ok)
  {
    if (ok)
    {
      char iso[RTC_ISO8601_BUF_SIZE];
      char local[TIME_LOCAL_STR_LEN];

      if (rtc_now_iso8601(iso, sizeof(iso)) &&
          time_utc_iso_to_local(iso, local, sizeof(local)))
      {
        Serial.print("[TIME] Heure locale : ");
        Serial.println(local);
      }
    }

    return ok;
  }

} // namespace