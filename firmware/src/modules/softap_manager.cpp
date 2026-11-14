#include "hal/softap_manager.h"

#include <Arduino.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "badge_store.h"
#include "hal/battery.h"
#include "config_store.h"
#include "hal/rtc_ds3231.h"
#include "log_format.h"
#include "log_sync.h"
#include "system_config.h"
#include "time_sync.h"

// Mettre à 1 pour la simulation Wokwi : le boîtier se connecte au Wi-Fi
// "Wokwi-GUEST" au lieu de créer son propre point d'accès.
#ifndef SOFTAP_SIMULATION_STA
#define SOFTAP_SIMULATION_STA 0
#endif

namespace
{

  WebServer server(80);
  DNSServer dnsServer;

  // Inactivité maximale avant la coupure du mode secours ET de la session.
  const uint32_t IDLE_TIMEOUT_MS = RESCUE_AUTO_OFF_MS;
  const uint8_t MAX_FAILED_ATTEMPTS = 5;
  const uint32_t LOCKOUT_MS = 30000;

  bool g_active = false;
  bool g_routesRegistered = false;
  bool g_restartRequested = false;

  // Session
  char g_sessionToken[33] = {0};
  uint32_t g_sessionLastMs = 0;
  uint32_t g_apLastMs = 0;

  // Protection contre les essais de mot de passe en rafale
  uint8_t g_failedAttempts = 0;
  uint32_t g_lockoutUntil = 0;
  bool g_locked = false;

  SoftApStatus g_status;

  // Synchronisation de l'heure
  enum SyncKind : uint8_t
  {
    SYNC_NONE,
    SYNC_NETWORK,
    SYNC_MANUAL
  };

  bool g_timeSyncRequested = false;
  bool g_syncPending = false;
  SyncKind g_syncKind = SYNC_NONE;
  bool g_syncOk = false;
  uint32_t g_syncAtMs = 0;

  // OUTILS

  bool elapsed(uint32_t since, uint32_t span)
  {
    return static_cast<uint32_t>(millis() - since) > span;
  }

  bool constantTimeEquals(const char *a, const char *b, size_t len)
  {
    uint8_t diff = 0;

    for (size_t i = 0; i < len; i++)
      diff |= static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]);

    return diff == 0;
  }

  void generateToken(char *out)
  {
    static const char hexDigits[] = "0123456789abcdef";

    for (int i = 0; i < 32; i++)
      out[i] = hexDigits[esp_random() % 16];

    out[32] = '\0';
  }

  // Ajoute du texte formaté dans buf à partir de pos (sans jamais déborder).
  size_t appendf(char *buf, size_t size, size_t pos, const char *fmt, ...)
  {
    if (pos >= size)
      return pos;

    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf + pos, size - pos, fmt, args);
    va_end(args);

    if (n < 0)
      return pos;

    size_t written = static_cast<size_t>(n);

    return (written >= size - pos) ? size - 1 : pos + written;
  }

  // Copie un champ de taille fixe du fichier de logs en texte sûr pour du
  // JSON : ASCII imprimable uniquement, sans guillemet ni antislash, sans
  // espaces de fin.
  void sanitizeField(const uint8_t *src, size_t length, char *dst)
  {
    size_t n = 0;

    for (size_t i = 0; i < length; i++)
    {
      char c = static_cast<char>(src[i]);

      if (c < 32 || c > 126 || c == '"' || c == '\\')
        c = '_';

      dst[n++] = c;
    }

    while (n > 0 && dst[n - 1] == ' ')
      n--;

    dst[n] = '\0';
  }

  void redirect(const char *location)
  {
    server.sendHeader("Location", location, true);
    server.send(302, "text/plain", "");
  }

  void sendJson(int code, const char *body)
  {
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json", body);
  }

  void sendOk()
  {
    sendJson(200, "{\"ok\":true}");
  }

  // Les messages d'erreur sont des CODES ; le tableau de bord les traduit.
  void sendError(int code, const char *errorCode)
  {
    char buf[96];

    snprintf(buf, sizeof(buf), "{\"ok\":false,\"error\":\"%s\"}", errorCode);

    sendJson(code, buf);
  }

  // AUTHENTIFICATION

  bool isLocked()
  {
    if (g_locked && static_cast<int32_t>(g_lockoutUntil - millis()) > 0)
      return true;

    g_locked = false;

    return false;
  }

  void registerFailure()
  {
    if (++g_failedAttempts >= MAX_FAILED_ATTEMPTS)
    {
      g_lockoutUntil = millis() + LOCKOUT_MS;
      g_locked = true;
      g_failedAttempts = 0;
    }
  }

  bool sessionValid()
  {
    if (g_sessionToken[0] == '\0')
      return false;

    if (elapsed(g_sessionLastMs, IDLE_TIMEOUT_MS))
    {
      g_sessionToken[0] = '\0';
      return false;
    }

    if (!server.hasHeader("Cookie"))
      return false;

    String cookie = server.header("Cookie");
    int p = cookie.indexOf("session=");

    if (p < 0)
      return false;

    String value = cookie.substring(p + 8, p + 8 + 32);

    if (value.length() != 32)
      return false;

    return constantTimeEquals(value.c_str(), g_sessionToken, 32);
  }

  // Les rafraîchissements automatiques de la page (?poll=1) ne comptent pas
  // comme une activité : sinon une page laissée ouverte empêcherait la
  // coupure automatique du mode secours.
  bool isPoll()
  {
    return server.hasArg("poll");
  }

  void touchAccessPoint()
  {
    if (!isPoll())
      g_apLastMs = millis();
  }

  // Authentification des requêtes d'API : 401 si la session est absente ou
  // expirée.
  bool apiAuth()
  {
    touchAccessPoint();

    if (!sessionValid())
    {
      sendError(401, "auth");
      return false;
    }

    if (!isPoll())
      g_sessionLastMs = millis();

    return true;
  }

  // PAGES

  bool streamPage(const char *path)
  {
    if (!LittleFS.exists(path))
      return false;

    File f = LittleFS.open(path, "r");

    if (!f)
      return false;

    server.sendHeader("Cache-Control", "no-store");
    server.streamFile(f, "text/html");
    f.close();

    return true;
  }

  void sendMissingPage(const char *name)
  {
    char buf[200];

    snprintf(buf, sizeof(buf),
             "<html><body><h1>%s</h1><p>Fichier %s absent de LittleFS. "
             "Envoyez le dossier data/ avec : pio run -t uploadfs</p></body></html>",
             config_device_id(), name);

    server.send(200, "text/html", buf);
  }

  void handleRoot()
  {
    touchAccessPoint();

    if (sessionValid())
    {
      redirect("/dashboard");
      return;
    }

    if (!streamPage("/login.html"))
      sendMissingPage("login.html");
  }

  void handleLogin()
  {
    touchAccessPoint();

    if (isLocked())
    {
      redirect("/?e=2");
      return;
    }

    String password = server.arg("password");

    if (config_check_password(password.c_str()))
    {
      generateToken(g_sessionToken);

      g_sessionLastMs = millis();
      g_failedAttempts = 0;

      server.sendHeader("Set-Cookie", String("session=") + g_sessionToken + "; Path=/; HttpOnly; SameSite=Strict");
      redirect("/dashboard");

      return;
    }

    registerFailure();

    redirect(isLocked() ? "/?e=2" : "/?e=1");
  }

  void handleLogout()
  {
    touchAccessPoint();

    g_sessionToken[0] = '\0';

    server.sendHeader("Set-Cookie", "session=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict");
    redirect("/");
  }

  void handleDashboard()
  {
    touchAccessPoint();

    if (!sessionValid())
    {
      redirect("/");
      return;
    }

    g_sessionLastMs = millis();

    if (!streamPage("/dashboard.html"))
      sendMissingPage("dashboard.html");
  }

  void handleDownload()
  {
    touchAccessPoint();

    if (!sessionValid())
    {
      redirect("/");
      return;
    }

    g_sessionLastMs = millis();

    File f = LittleFS.open(LITTLEFS_LOG_FILE, "r");

    if (!f)
    {
      server.send(404, "text/plain", "Fichier introuvable");
      return;
    }

    char disposition[80];
    snprintf(disposition, sizeof(disposition), "attachment; filename=\"logs-%s.csv\"",
             config_device_id());
    server.sendHeader("Content-Disposition", disposition);
    server.streamFile(f, "text/csv");
    f.close();
  }

  // API : ETAT

  void handleApiStatus()
  {
    if (!apiAuth())
      return;

    char utc[RTC_ISO8601_BUF_SIZE];
    bool rtcValid = rtc_now_iso8601(utc, sizeof(utc));

    uint32_t mv = battery_read_mv();
    unsigned batteryPct = battery_percent_from_mv(mv);

    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    unsigned fsPct = total > 0 ? static_cast<unsigned>((static_cast<uint64_t>(used) * 100) / total) : 0;

    uint32_t idle = static_cast<uint32_t>(millis() - g_apLastMs);
    unsigned remaining = idle >= IDLE_TIMEOUT_MS ? 0 : (IDLE_TIMEOUT_MS - idle) / 1000;

    // Synchronisation de l'heure
    const char *syncKind = g_syncKind == SYNC_NETWORK ? "network" : (g_syncKind == SYNC_MANUAL ? "manual" : "none");
    const char *syncState = g_syncPending ? "pending" : (g_syncKind == SYNC_NONE ? "none" : (g_syncOk ? "ok" : "failed"));
    unsigned syncAge = g_syncKind == SYNC_NONE ? 0 : (millis() - g_syncAtMs) / 1000;

    // Dernier badge présenté
    char lastUid[BADGE_UID_MAX_LEN + 1] = "";
    bool lastKnown = false;
    uint32_t lastAge = 0;
    bool hasLast = badges_last_scan(lastUid, &lastKnown, &lastAge);

    static char buf[1000];

    snprintf(buf, sizeof(buf),
             "{\"device\":\"%s\",\"fw\":\"%s\",\"rtcValid\":%s,\"utc\":\"%s\",\"tz\":%d,"
             "\"battery\":{\"pct\":%u,\"mv\":%lu},"
             "\"internet\":%s,\"mqtt\":%s,\"state\":\"%s\","
             "\"fs\":{\"pct\":%u,\"usedKB\":%lu,\"totalKB\":%lu},"
             "\"idleSec\":%u,\"autoTime\":%s,"
             "\"sync\":{\"kind\":\"%s\",\"state\":\"%s\",\"ageSec\":%u},"
             "\"defaultPwd\":%s,\"badges\":%u,\"badgeMax\":%d,"
             "\"lastUid\":\"%s\",\"lastKnown\":%s,\"lastAgeSec\":%lu}",
             config_device_id(), FIRMWARE_VERSION, rtcValid ? "true" : "false", rtcValid ? utc : "", LOCAL_UTC_OFFSET_MINUTES,
             batteryPct, static_cast<unsigned long>(mv),
             g_status.internetUp ? "true" : "false", g_status.mqttConnected ? "true" : "false", g_status.state,
             fsPct, static_cast<unsigned long>(used / 1024), static_cast<unsigned long>(total / 1024),
             remaining, config_auto_time_sync() ? "true" : "false",
             syncKind, syncState, syncAge,
             config_password_is_default() ? "true" : "false", static_cast<unsigned>(badges_count()), BADGE_MAX,
             hasLast ? lastUid : "", (hasLast && lastKnown) ? "true" : "false", static_cast<unsigned long>(lastAge));

    sendJson(200, buf);
  }

  // API : LOGS

  struct LogStats
  {
    uint32_t total = 0;
    uint32_t pending = 0;
    uint32_t sent = 0;
    uint32_t failed = 0;
    uint32_t unknownTime = 0;
    uint32_t days[7] = {0};
  };

  constexpr size_t LOG_STATUS_OFFSET =
      LOG_FIELD_TIMESTAMP_LEN + 1 + LOG_FIELD_AGENT_LEN + 1 +
      LOG_FIELD_CHECKPOINT_LEN + 1;

  // Une seule lecture du fichier : compteurs par statut + passages des 7
  // derniers jours (jours LOCAUX).
  bool readStats(LogStats &st, int64_t *todayDay)
  {
    File f = LittleFS.open(LITTLEFS_LOG_FILE, "r");

    if (!f)
      return false;

    char nowIso[RTC_ISO8601_BUF_SIZE];
    int64_t nowEpoch = 0;
    bool timeKnown = rtc_now_iso8601(nowIso, sizeof(nowIso)) && time_iso_to_epoch(nowIso, &nowEpoch);

    const int64_t offsetSec = static_cast<int64_t>(LOCAL_UTC_OFFSET_MINUTES) * 60;
    int64_t today = timeKnown ? (nowEpoch + offsetSec) / 86400 : 0;

    *todayDay = timeKnown ? today : -1;

    static uint8_t chunk[LOG_LINE_LEN * 32];

    size_t n;

    while ((n = f.read(chunk, sizeof(chunk))) >= LOG_LINE_LEN)
    {
      for (size_t i = 0; i + LOG_LINE_LEN <= n; i += LOG_LINE_LEN)
      {
        const uint8_t *line = chunk + i;

        st.total++;

        char status = static_cast<char>(line[LOG_STATUS_OFFSET]);

        if (status == LOG_STATUS_PENDING)
          st.pending++;
        else if (status == LOG_STATUS_SENT)
          st.sent++;
        else if (status == LOG_STATUS_FAILED)
          st.failed++;

        char ts[LOG_FIELD_TIMESTAMP_LEN + 1];
        int64_t epoch = 0;

        memcpy(ts, line, LOG_FIELD_TIMESTAMP_LEN);
        ts[LOG_FIELD_TIMESTAMP_LEN] = '\0';

        if (!time_iso_to_epoch(ts, &epoch) || epoch < 31536000)
        {
          st.unknownTime++; // horodatage 1970 : heure inconnue au moment du passage
        }
        else if (timeKnown)
        {
          int64_t age = today - (epoch + offsetSec) / 86400;

          if (age >= 0 && age < 7)
            st.days[6 - age]++;
        }
      }

      yield(); // laisse respirer le Wi-Fi sur un gros fichier
    }

    f.close();

    return true;
  }

  void handleApiStats()
  {
    if (!apiAuth())
      return;

    LogStats st;
    int64_t today = -1;

    readStats(st, &today);

    static char buf[700];
    size_t pos = 0;

    pos = appendf(buf, sizeof(buf), pos,
                  "{\"total\":%lu,\"pending\":%lu,\"sent\":%lu,\"failed\":%lu,\"unknownTime\":%lu,\"days\":[",
                  static_cast<unsigned long>(st.total), static_cast<unsigned long>(st.pending),
                  static_cast<unsigned long>(st.sent), static_cast<unsigned long>(st.failed),
                  static_cast<unsigned long>(st.unknownTime));

    if (today >= 0)
    {
      for (int k = 6; k >= 0; k--)
      {
        char label[24];

        // Minuit LOCAL du jour, exprimé comme une date : "AAAA-MM-JJT00:00:00Z".
        time_epoch_to_iso((today - k) * 86400, label, sizeof(label));

        pos = appendf(buf, sizeof(buf), pos, "%s{\"d\":\"%.5s\",\"n\":%lu}",
                      k == 6 ? "" : ",", label + 5, static_cast<unsigned long>(st.days[6 - k]));
      }
    }

    appendf(buf, sizeof(buf), pos, "]}");

    sendJson(200, buf);
  }

  // Liste des logs, du plus récent au plus ancien, par pages.
  void handleApiLogs()
  {
    if (!apiAuth())
      return;

    long offset = server.arg("offset").toInt();
    long limit = server.arg("limit").toInt();

    if (offset < 0)
      offset = 0;

    if (limit < 1)
      limit = 25;

    if (limit > 50)
      limit = 50;

    File f = LittleFS.open(LITTLEFS_LOG_FILE, "r");

    long totalLines = f ? static_cast<long>(f.size() / LOG_LINE_LEN) : 0;

    static char buf[5400];
    size_t pos = 0;

    long last = totalLines - 1 - offset; // ligne la plus récente de la page
    long count = (last >= 0) ? ((last + 1 < limit) ? last + 1 : limit) : 0;
    long first = last - count + 1; // ligne la plus ancienne de la page

    pos = appendf(buf, sizeof(buf), pos, "{\"total\":%ld,\"more\":%s,\"rows\":[",
                  totalLines, (count > 0 && first > 0) ? "true" : "false");

    if (count > 0)
    {
      static uint8_t lines[LOG_LINE_LEN * 50];

      size_t want = static_cast<size_t>(count) * LOG_LINE_LEN;
      size_t got = 0;

      if (f.seek(static_cast<uint32_t>(first) * LOG_LINE_LEN))
        got = f.read(lines, want);

      long rows = static_cast<long>(got / LOG_LINE_LEN);

      for (long i = rows - 1; i >= 0; i--)
      {
        const uint8_t *line = lines + i * LOG_LINE_LEN;

        char ts[LOG_FIELD_TIMESTAMP_LEN + 1];
        char agent[LOG_FIELD_AGENT_LEN + 1];
        char checkpoint[LOG_FIELD_CHECKPOINT_LEN + 1];

        sanitizeField(line, LOG_FIELD_TIMESTAMP_LEN, ts);
        sanitizeField(line + 21, LOG_FIELD_AGENT_LEN, agent);
        sanitizeField(line + 27, LOG_FIELD_CHECKPOINT_LEN, checkpoint);

        char status = static_cast<char>(line[LOG_STATUS_OFFSET]);

        if (status != LOG_STATUS_PENDING && status != LOG_STATUS_SENT && status != LOG_STATUS_FAILED)
          status = '?';

        pos = appendf(buf, sizeof(buf), pos, "%s{\"n\":%ld,\"ts\":\"%s\",\"ag\":\"%s\",\"cp\":\"%s\",\"st\":\"%c\"}",
                      i == rows - 1 ? "" : ",", first + i, ts, agent, checkpoint, status);
      }
    }

    if (f)
      f.close();

    appendf(buf, sizeof(buf), pos, "]}");

    sendJson(200, buf);
  }

  void handleApiLogsClear()
  {
    if (!apiAuth())
      return;

    if (strcmp(server.arg("confirm").c_str(), "yes") != 0)
    {
      sendError(400, "bad_request");
      return;
    }

    // D'abord on abandonne l'envoi en cours : le lot gardé en mémoire par
    // log_sync ne correspond plus au fichier une fois celui-ci effacé.
    log_sync_reset();

    if (clear_storage_logs())
      sendOk();
    else
      sendError(500, "storage");
  }

  // API : BADGES

  void sendBadgeResult(BadgeResult result)
  {
    switch (result)
    {
    case BadgeResult::OK:
      sendOk();
      break;
    case BadgeResult::INVALID_UID:
      sendError(400, "invalid_uid");
      break;
    case BadgeResult::INVALID_AGENT:
      sendError(400, "invalid_agent");
      break;
    case BadgeResult::DUPLICATE:
      sendError(409, "duplicate");
      break;
    case BadgeResult::NOT_FOUND:
      sendError(404, "not_found");
      break;
    case BadgeResult::FULL:
      sendError(409, "full");
      break;
    default:
      sendError(500, "storage");
      break;
    }
  }

  void handleApiBadges()
  {
    if (!apiAuth())
      return;

    static char buf[BADGE_MAX * 56 + 64];
    size_t pos = 0;

    pos = appendf(buf, sizeof(buf), pos, "{\"max\":%d,\"badges\":[", BADGE_MAX);

    // UID (hexadécimal) et agent (A-Z 0-9 _ -) : aucun échappement nécessaire.
    for (size_t i = 0; i < badges_count(); i++)
    {
      BadgeRecord record;

      if (badges_get(i, &record))
        pos = appendf(buf, sizeof(buf), pos, "%s{\"uid\":\"%s\",\"agent\":\"%s\"}", i == 0 ? "" : ",", record.uid, record.agent);
    }

    appendf(buf, sizeof(buf), pos, "]}");

    sendJson(200, buf);
  }

  void handleApiBadgeAdd()
  {
    if (!apiAuth())
      return;

    sendBadgeResult(badges_add(server.arg("uid").c_str(), server.arg("agent").c_str()));
  }

  void handleApiBadgeUpdate()
  {
    if (!apiAuth())
      return;

    sendBadgeResult(badges_update(server.arg("uid").c_str(), server.arg("newUid").c_str(), server.arg("agent").c_str()));
  }

  void handleApiBadgeDelete()
  {
    if (!apiAuth())
      return;

    sendBadgeResult(badges_remove(server.arg("uid").c_str()));
  }

  // API : MOT DE PASSE

  void handleApiDeviceId()
  {
    if (!apiAuth())
      return;

    String deviceId = server.arg("deviceId");
    if (!config_device_id_valid(deviceId.c_str()))
    {
      sendError(400, "invalid_device_id");
      return;
    }

    if (strcmp(deviceId.c_str(), config_device_id()) == 0)
    {
      sendJson(200, "{\"ok\":true,\"changed\":false}");
      return;
    }

    if (!config_set_device_id(deviceId.c_str()))
    {
      sendError(500, "storage");
      return;
    }

    sendJson(200, "{\"ok\":true,\"changed\":true}");
    g_restartRequested = true;
  }

  void handleApiPassword()
  {
    if (!apiAuth())
      return;

    if (isLocked())
    {
      sendError(429, "locked");
      return;
    }

    String current = server.arg("current");
    String next = server.arg("new");

    if (!config_check_password(current.c_str()))
    {
      registerFailure();
      sendError(403, "wrong_password");
      return;
    }

    if (next.length() < PASSWORD_MIN_LEN || next.length() > PASSWORD_MAX_LEN)
    {
      sendError(400, "weak_password");
      return;
    }

    if (!config_set_password(next.c_str()))
    {
      sendError(500, "storage");
      return;
    }

    // Reconnexion obligatoire avec le nouveau mot de passe.
    g_sessionToken[0] = '\0';

    server.sendHeader("Set-Cookie", "session=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict");
    sendOk();
  }

  // API : DATE ET HEURE

  // Réglage manuel : "utc" = AAAA-MM-JJTHH:MM:SS (UTC). Le tableau de bord
  // convertit lui-même l'heure saisie ou celle du téléphone.
  void handleApiTimeManual()
  {
    if (!apiAuth())
      return;

    int64_t epoch = 0;

    if (!time_iso_to_epoch(server.arg("utc").c_str(), &epoch))
    {
      sendError(400, "bad_time");
      return;
    }

    char iso[24];

    time_epoch_to_iso(epoch, iso, sizeof(iso));

    // Le driver DS3231 refuse les dates antérieures à 2026.
    if (!rtc_sync_from_string(iso))
    {
      sendError(400, "bad_time");
      return;
    }

    g_syncKind = SYNC_MANUAL;
    g_syncOk = true;
    g_syncPending = false;
    g_syncAtMs = millis();

    char now[RTC_ISO8601_BUF_SIZE];
    char buf[80];

    snprintf(buf, sizeof(buf), "{\"ok\":true,\"utc\":\"%s\"}", rtc_now_iso8601(now, sizeof(now)) ? now : iso);

    sendJson(200, buf);
  }

  // Synchronisation par le réseau mobile
  void handleApiTimeAuto()
  {
    if (!apiAuth())
      return;

    if (g_syncPending)
    {
      sendError(409, "busy");
      return;
    }

    if (!g_status.internetUp)
    {
      sendError(409, "no_internet");
      return;
    }

    g_timeSyncRequested = true;
    g_syncPending = true;

    sendOk();
  }

  // Active ou désactive la synchronisation automatique (toutes les 24 h).
  void handleApiTimeMode()
  {
    if (!apiAuth())
      return;

    config_set_auto_time_sync(strcmp(server.arg("auto").c_str(), "1") == 0);

    sendOk();
  }

  // ROUTES

  void handleCaptiveProbe()
  {
    redirect(sessionValid() ? "/dashboard" : "/");
  }

  void handleNotFound()
  {
    touchAccessPoint();

    if (server.uri().startsWith("/api/"))
    {
      sendError(404, "not_found");
      return;
    }

    redirect(sessionValid() ? "/dashboard" : "/");
  }

  void registerRoutes()
  {
    const char *headerKeys[] = {"Cookie"};
    server.collectHeaders(headerKeys, 1);

    // Sondes de portail captif (Android, iOS, Windows) : redirection vers la page.
    server.on("/generate_204", HTTP_GET, handleCaptiveProbe);
    server.on("/gen_204", HTTP_GET, handleCaptiveProbe);
    server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveProbe);
    server.on("/canonical.html", HTTP_GET, handleCaptiveProbe);
    server.on("/connecttest.txt", HTTP_GET, handleCaptiveProbe);
    server.on("/favicon.ico", HTTP_GET, handleCaptiveProbe);
    server.on("/ncsi.txt", HTTP_GET, handleCaptiveProbe);
    server.on("/library/test/success.html", HTTP_GET, handleCaptiveProbe);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/login", HTTP_POST, handleLogin);
    server.on("/logout", HTTP_POST, handleLogout);
    server.on("/dashboard", HTTP_GET, handleDashboard);
    server.on("/download", HTTP_GET, handleDownload);

    server.on("/api/status", HTTP_GET, handleApiStatus);
    server.on("/api/stats", HTTP_GET, handleApiStats);
    server.on("/api/logs", HTTP_GET, handleApiLogs);
    server.on("/api/logs/clear", HTTP_POST, handleApiLogsClear);

    server.on("/api/device-id", HTTP_POST, handleApiDeviceId);

    server.on("/api/badges", HTTP_GET, handleApiBadges);
    server.on("/api/badges/add", HTTP_POST, handleApiBadgeAdd);
    server.on("/api/badges/update", HTTP_POST, handleApiBadgeUpdate);
    server.on("/api/badges/delete", HTTP_POST, handleApiBadgeDelete);

    server.on("/api/password", HTTP_POST, handleApiPassword);

    server.on("/api/time/manual", HTTP_POST, handleApiTimeManual);
    server.on("/api/time/auto", HTTP_POST, handleApiTimeAuto);
    server.on("/api/time/mode", HTTP_POST, handleApiTimeMode);

    server.onNotFound(handleNotFound);
  }

} // namespace

// API PUBLIQUE

void softap_start()
{
  if (g_active)
    return;

#if SOFTAP_SIMULATION_STA

  // Simulation Wokwi : connexion au réseau virtuel.
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "");

  uint32_t start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    delay(250);

  Serial.print("[WEB] Tableau de bord : http://");
  Serial.println(WiFi.localIP());

#else

  WiFi.mode(WIFI_AP);
  WiFi.softAP(RESCUE_SSID, AP_WIFI_PASSWORD);

  // Portail captif : tout nom de domaine renvoie vers le boîtier, ce qui
  // ouvre automatiquement la page de connexion sur la plupart des téléphones.
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  Serial.print("[WEB] Wi-Fi \"");
  Serial.print(RESCUE_SSID);
  Serial.print("\" actif, tableau de bord : http://");
  Serial.println(WiFi.softAPIP());

#endif

  if (!g_routesRegistered)
  {
    registerRoutes();
    g_routesRegistered = true;
  }

  server.begin();

  g_active = true;
  g_apLastMs = millis();
  g_sessionToken[0] = '\0';
  g_failedAttempts = 0;
  g_locked = false;
  g_timeSyncRequested = false;
  g_syncPending = false;
}

void softap_loop()
{
  if (!g_active)
    return;

#if !SOFTAP_SIMULATION_STA
  dnsServer.processNextRequest();
#endif

  server.handleClient();

  if (g_restartRequested)
  {
    delay(1000);
    ESP.restart();
  }
}

void softap_stop()
{
  if (!g_active)
    return;

#if !SOFTAP_SIMULATION_STA
  dnsServer.stop();
#endif

  server.close();

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);

  g_sessionToken[0] = '\0'; // invalide toute session en cours
  g_active = false;
}

bool softap_is_active()
{
  return g_active;
}

bool softap_expired()
{
  return g_active && elapsed(g_apLastMs, IDLE_TIMEOUT_MS);
}

void softap_update_status(const SoftApStatus &status)
{
  g_status = status;
}

bool softap_take_time_sync_request()
{
  bool requested = g_timeSyncRequested;

  g_timeSyncRequested = false;

  return requested;
}

void softap_report_time_sync(bool ok)
{
  g_syncKind = SYNC_NETWORK;
  g_syncOk = ok;
  g_syncPending = false;
  g_syncAtMs = millis();
}

bool softap_set_password_hash(const char *passwordPlain)
{
  return config_set_password(passwordPlain);
}