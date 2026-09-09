#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include "hal/softap_manager.h"

// Configuration du test
const char *TEST_IP = "192.168.4.1";
const char *TEST_PASSWORD = "Admin123";

TaskHandle_t g_webServerTaskHandle = NULL;

// Tâche FreeRTOS pour exécuter la boucle WebServer + DNSServer
void webServerTask(void *pvParameters)
{
  for (;;)
  {
    softap_loop();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// Fonction utilitaire d'affichage
void printTestResult(const char *testName, bool success, const String &details = "")
{
  Serial.print("[TEST] ");
  Serial.print(testName);
  for (int i = strlen(testName); i < 42; i++)
    Serial.print(".");
  if (success)
  {
    Serial.println(" [  OK  ]");
  }
  else
  {
    Serial.println(" [ ECHEC ]");
  }
  if (details.length() > 0)
  {
    Serial.print("       └─ > ");
    Serial.println(details);
  }
}

// Prépare un fichier /logs.csv de test sur LittleFS
bool prepare_mock_logs_file()
{
  if (!LittleFS.begin(true, "/littlefs", 10, "littlefs"))
  {
    Serial.println("[WARN] Échec du montage LittleFS");
    return false;
  }

  File f = LittleFS.open("/logs.csv", "w");
  if (!f)
  {
    Serial.println("[WARN] Impossible de créer /logs.csv");
    return false;
  }
  f.println("timestamp,agent_id,status");
  f.println("1700000000,AGENT_01,OK");
  f.close();
  return true;
}

// Téléchargement du fichier /logs.csv
bool test_download_logs_csv(const String &cookie)
{
  HTTPClient http;

  // 1. Essai de téléchargement SANS cookie
  http.begin(String("http://") + TEST_IP + "/download");
  int codeUnauth = http.GET();
  http.end();

  if (codeUnauth != 302)
  {
    printTestResult("Sécurité : /download sans cookie", false, "Code HTTP reçu: " + String(codeUnauth) + " (Attendu: 302)");
    return false;
  }

  if (cookie.length() == 0)
  {
    printTestResult("Téléchargement /logs.csv avec Cookie", false, "Pas de cookie valide disponible");
    return false;
  }

  // 2. Essai de téléchargement AVEC cookie valide
  http.begin(String("http://") + TEST_IP + "/download");
  http.addHeader("Cookie", cookie);

  const char *headerKeys[] = {"Content-Disposition"};
  http.collectHeaders(headerKeys, 1);

  int codeAuth = http.GET();
  String contentDisposition = http.header("Content-Disposition");
  String body = http.getString();
  http.end();

  bool hasHeader = contentDisposition.indexOf("attachment") >= 0 && contentDisposition.indexOf("logs.csv") >= 0;
  bool hasContent = body.indexOf("AGENT_01") >= 0;

  bool success = (codeAuth == 200) && hasHeader && hasContent;

  String details = "HTTP " + String(codeAuth) + " | Header Disposition: " + (hasHeader ? "OK" : "Manquant/Invalide") +
                   " | Contenu CSV valide: " + (hasContent ? "Oui" : "Non");

  printTestResult("Téléchargement /logs.csv avec Cookie", success, details);
  return success;
}

bool test_wifi_access_point()
{
  bool ok = (WiFi.softAPIP().toString() == "192.168.4.1");
  printTestResult("Initialisation SoftAP Wi-Fi", ok, "IP: " + WiFi.softAPIP().toString());
  return ok;
}

bool test_unauthenticated_redirect()
{
  HTTPClient http;
  http.begin(String("http://") + TEST_IP + "/dashboard");
  int code = http.GET();
  http.end();

  bool ok = (code == 302);
  printTestResult("Sécurité : Accès /dashboard sans cookie", ok, "Code HTTP reçu: " + String(code));
  return ok;
}

bool test_bad_password_login()
{
  HTTPClient http;
  http.begin(String("http://") + TEST_IP + "/login");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int code = http.POST("password=MauvaisMotDePasse");
  String body = http.getString();
  http.end();

  bool ok = (code == 200 && body.indexOf("Mot de passe incorrect") >= 0);
  printTestResult("Auth : Rejet mauvais mot de passe", ok);
  return ok;
}

String test_successful_login_and_get_cookie()
{
  HTTPClient http;
  http.begin(String("http://") + TEST_IP + "/login");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  const char *headerKeys[] = {"Set-Cookie", "Location"};
  http.collectHeaders(headerKeys, 2);

  int code = http.POST(String("password=") + TEST_PASSWORD);
  String cookie = http.header("Set-Cookie");
  http.end();

  int semicolonIndex = cookie.indexOf(';');
  if (semicolonIndex > 0)
  {
    cookie = cookie.substring(0, semicolonIndex);
  }

  bool ok = (code == 302 && cookie.startsWith("session="));
  printTestResult("Auth : Login valide & Cookie reçu", ok, "Cookie: " + cookie);

  return ok ? cookie : "";
}

// Exécution globale de la suite de tests
void run_full_functional_suite()
{
  Serial.println("\n==================================================");
  Serial.println("   LANCEMENT DU TEST DE FONCTIONNALITE SOFTAP    ");
  Serial.println("==================================================");

  int passed = 0;
  int total = 5;

  if (test_wifi_access_point())
    passed++;
  if (test_unauthenticated_redirect())
    passed++;
  if (test_bad_password_login())
    passed++;

  String sessionCookie = test_successful_login_and_get_cookie();
  if (sessionCookie.length() > 0)
    passed++;

  // Test de téléchargement du fichier /logs.csv
  if (test_download_logs_csv(sessionCookie))
    passed++;

  Serial.println("--------------------------------------------------");
  Serial.printf(" BILAN : %d / %d tests réussis (%.0f%%)\n", passed, total, (passed / (float)total) * 100);
  Serial.println("==================================================\n");
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n[INIT] Démarrage du banc d'essai...");

  // 1. Préparation du système de fichiers avec un CSV de test
  if (prepare_mock_logs_file())
  {
    Serial.println("[INIT] Fichier /logs.csv créé avec succès sur LittleFS.");
  }

  // 2. Initialisation et démarrage du SoftAP
  softap_set_password_hash(TEST_PASSWORD);
  softap_start();

  // 3. Lancement de la tâche FreeRTOS pour le traitement serveur
  xTaskCreatePinnedToCore(
      webServerTask,
      "WebServerTask",
      4096,
      NULL,
      1,
      &g_webServerTaskHandle,
      0);

  delay(1000);

  // 4. Lancement des tests automatiques
  run_full_functional_suite();
}

void loop()
{
  // Boucle vide car tout est géré par la tâche FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(1000));
}