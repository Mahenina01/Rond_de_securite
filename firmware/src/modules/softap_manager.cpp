#include "hal/softap_manager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <mbedtls/sha256.h>
#include <esp_system.h>
#include "log_format.h"
#include "system_config.h"

namespace
{

  WebServer server(80);
  DNSServer dnsServer;

  // Aligné sur la coupure automatique du SoftAP exigée au CDC (5 min).
  const uint32_t SESSION_TIMEOUT_MS = RESCUE_AUTO_OFF_MS;
  const uint32_t MAX_FAILED_ATTEMPTS = 5;
  const uint32_t LOCKOUT_MS = 30000;

  char g_passwordHashHex[65] = {0};
  char g_sessionToken[33] = {0};
  uint32_t g_sessionExpiresAt = 0;
  uint32_t g_failedAttempts = 0;
  uint32_t g_lockoutUntil = 0;

  void sha256Hex(const char *input, char *outHex)
  {
    uint8_t hash[32];
    mbedtls_sha256((const unsigned char *)input, strlen(input), hash, 0);
    static const char hexDigits[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++)
    {
      outHex[i * 2] = hexDigits[(hash[i] >> 4) & 0xF];
      outHex[i * 2 + 1] = hexDigits[hash[i] & 0xF];
    }
    outHex[64] = '\0';
  }

  // Comparaison a temps constant : evite qu'un attaquant deduise le hash
  // correct octet par octet en mesurant le temps de reponse.
  bool constantTimeEquals(const char *a, const char *b, size_t len)
  {
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++)
      diff |= (uint8_t)a[i] ^ (uint8_t)b[i];
    return diff == 0;
  }

  void generateSessionToken(char *out)
  {
    static const char hexDigits[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++)
      out[i] = hexDigits[esp_random() % 16];
    out[32] = '\0';
  }

  bool isSessionValid()
  {
    if (g_sessionToken[0] == '\0')
      return false;
    if (millis() > g_sessionExpiresAt)
      return false;
    if (!server.hasHeader("Cookie"))
      return false;

    String cookie = server.header("Cookie");
    String needle = String("session=") + g_sessionToken;
    return cookie.indexOf(needle) >= 0;
  }

  void sendLoginPage(bool showError)
  {
    String html =
        "<!DOCTYPE html><html><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Pointeuse Mobile — Connexion</title>"
        "<style>body{font-family:sans-serif;background:#F8FAFC;display:flex;"
        "align-items:center;justify-content:center;height:100vh;margin:0}"
        ".card{background:#fff;padding:32px;border-radius:12px;border:1px solid #E2E8F0;width:280px}"
        "h1{font-size:16px;color:#0F172A;margin:0 0 16px}"
        "input{width:100%;padding:10px;margin-bottom:12px;border:1px solid #E2E8F0;"
        "border-radius:8px;box-sizing:border-box}"
        "button{width:100%;padding:12px;background:#2563EB;color:#fff;border:none;"
        "border-radius:8px;font-weight:600}"
        ".err{color:#DC2626;font-size:13px;margin-bottom:8px}</style></head><body>"
        "<form class='card' method='POST' action='/login'>"
        "<h1>Pointeuse Mobile — Authentification requise</h1>";
    if (showError)
      html += "<div class='err'>Mot de passe incorrect</div>";
    html +=
        "<input type='password' name='password' placeholder='Mot de passe' autofocus>"
        "<button type='submit'>Se connecter</button>"
        "</form></body></html>";
    server.send(200, "text/html", html);
  }

  bool requireAuth()
  {
    if (!isSessionValid())
    {
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "");
      return false;
    }
    return true;
  }

} // namespace

void softap_start()
{
  // Premiere barriere : le SoftAP lui-meme est protege WPA2, pas ouvert.
  WiFi.softAP("Ronde-Pointeuse-01", AP_WIFI_PASSWORD);

  // Redirige toutes les requêtes DNS vers l'ESP32 pour le portail captif
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Activer l'analyse de l'en-tête "Cookie"
  const char *headerKeys[] = {"Cookie"};
  server.collectHeaders(headerKeys, 1);

  // --- CAPTURE DES SONDES PORTAIL CAPTIF (Android, iOS/Mac, Windows) ---
  auto handleCaptiveProbe = []()
  {
    if (isSessionValid())
    {
      server.sendHeader("Location", "/dashboard", true);
    }
    else
    {
      server.sendHeader("Location", "/", true);
    }
    server.send(302, "text/plain", "");
  };

  server.on("/generate_204", HTTP_GET, handleCaptiveProbe);
  server.on("/gen_204", HTTP_GET, handleCaptiveProbe);
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveProbe);
  server.on("/canonical.html", HTTP_GET, handleCaptiveProbe);
  server.on("/connecttest.txt", HTTP_GET, handleCaptiveProbe);
  server.on("/favicon.ico", HTTP_GET, handleCaptiveProbe);
  server.on("/ncsi.txt", HTTP_GET, handleCaptiveProbe);
  server.on("/library/test/success.html", HTTP_GET, handleCaptiveProbe);

  server.on("/", HTTP_GET, []()
            {
    if (isSessionValid()) {
      server.sendHeader("Location", "/dashboard", true);
      server.send(302, "text/plain", "");
    } else {
      sendLoginPage(false);
    } });

  server.on("/login", HTTP_POST, []()
            {
    if (millis() < g_lockoutUntil) {
      server.send(429, "text/plain", "Trop de tentatives, reessayez plus tard.");
      return;
    }
    if (!server.hasArg("password")) {
      sendLoginPage(true);
      return;
    }

    String pwd = server.arg("password");
    char candidateHash[65];
    sha256Hex(pwd.c_str(), candidateHash);

    if (constantTimeEquals(candidateHash, g_passwordHashHex, 64)) {
      generateSessionToken(g_sessionToken);
      g_sessionExpiresAt = millis() + SESSION_TIMEOUT_MS;
      g_failedAttempts = 0;

      server.sendHeader("Location", "/dashboard", true);
      server.sendHeader("Set-Cookie", String("session=") + g_sessionToken + "; Path=/; HttpOnly");
      server.send(302, "text/plain", "");
    } else {
      g_failedAttempts++;
      if (g_failedAttempts >= MAX_FAILED_ATTEMPTS) {
        g_lockoutUntil = millis() + LOCKOUT_MS;
        g_failedAttempts = 0;
      }
      sendLoginPage(true);
    } });

  // Toutes les routes suivantes exigent une session valide
  server.on("/dashboard", HTTP_GET, []()
            {
    if (!requireAuth()) return;
    // Vérifie l'existence du fichier avant de tenter de l'ouvrir
    if (LittleFS.exists("/dashboard.html")) {
        File file = LittleFS.open("/dashboard.html", "r");
        server.streamFile(file, "text/html");
        file.close();
    } else {
        server.send(200, "text/html", "<html><body><h1>Dashboard PN-01</h1><p>Fichier dashboard.html non présent sur LittleFS.</p></body></html>");
    } });

  server.on("/download", HTTP_GET, []()
            {
    if (!requireAuth()) return;
    File file = LittleFS.open("/logs.csv", "r");
    if (!file) {
      server.send(404, "text/plain", "Fichier introuvable");
      return;
    }
    server.sendHeader("Content-Disposition", "attachment; filename=logs.csv");
    server.streamFile(file, "text/csv");
    file.close(); });

  // Endpoint API pour fournir l'état en direct au tableau de bord
  server.on("/api/status", HTTP_GET, []()
            {
    if (!requireAuth()) return;

    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    int usedPercent = totalBytes > 0 ? (usedBytes * 100 / totalBytes) : 0;

    uint32_t remainingMs = (SESSION_TIMEOUT_MS > millis()) ? (SESSION_TIMEOUT_MS - millis()) : 0;
    uint32_t seconds = remainingMs / 1000;
    char timeoutStr[10];
    snprintf(timeoutStr, sizeof(timeoutStr), "%02u:%02u", (unsigned int)(seconds / 60), (unsigned int)(seconds % 60));

    String json = "{";
    json += "\"rtc\":\"30/08/2026 14:27:00\","; // TODO : implementer la récupération de l'heure RTC réelle
    json += "\"battery\":78,";       // TODO : implementer la récupération de l'état réel de la batterie          
    json += "\"timeout\":\"" + String(timeoutStr) + "\",";
    json += "\"pending\":12,"; // TODO : implementer la récupération du nombre réel d'entrées en attente
    json += "\"sent\":340,"; // TODO : implementer la récupération du nombre réel d'entrées envoyées
    json += "\"failed\":3,"; // TODO : implementer la récupération du nombre réel d'entrées échouées
    json += "\"used\":" + String(usedPercent);
    json += "}";

    server.send(200, "application/json", json); });

  server.on("/purge", HTTP_POST, []()
            {
    if (!requireAuth()) return;
    LittleFS.remove("/logs.csv");
    storage_init();
    server.sendHeader("Location", "/dashboard", true);
    server.send(302, "text/plain", ""); });

  server.on("/sync-rtc", HTTP_POST, []()
            {
    if (!requireAuth()) return;
    server.sendHeader("Location", "/dashboard", true);
    server.send(302, "text/plain", ""); });

  // Redirection automatique pour les requêtes inconnues du portail captif
  server.onNotFound([]()
                    {
    if (isSessionValid()) {
      server.sendHeader("Location", "/dashboard", true);
      server.send(302, "text/plain", "");
    } else {
      sendLoginPage(false);
    } });

  server.begin();
}

// Traite les requêtes HTTP/DNS entrantes (à appeler dans la tâche FreeRTOS)
void softap_loop()
{
  dnsServer.processNextRequest();
  server.handleClient();
}

void softap_stop()
{
  dnsServer.stop();
  server.close();
  WiFi.softAPdisconnect(true);
  g_sessionToken[0] = '\0'; // invalide toute session en cours
}

bool softap_set_password_hash(const char *passwordPlain)
{
  sha256Hex(passwordPlain, g_passwordHashHex);
  return true;
}