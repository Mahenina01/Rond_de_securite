#include <unity.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "hal/softap_manager.h"

// IP par défaut de l'ESP32 en mode SoftAP
const char *TEST_IP = "192.168.4.1";

// Handle de la tâche FreeRTOS pour le traitement du serveur
TaskHandle_t g_webServerTaskHandle = NULL;

// Tâche FreeRTOS exécutant le serveur Web et le DNS en arrière-plan
void webServerTask(void *pvParameters)
{
    for (;;)
    {
        softap_loop();
        vTaskDelay(pdMS_TO_TICKS(5)); // Pause de 5ms pour céder le temps CPU
    }
}

void setUp(void)
{
    // Réinitialisation de l'état de la session avant chaque test
    // Réinitialise le jeton de session pour isoler chaque cas de test
    softap_stop();
    softap_set_password_hash("SecretPassword123");
    softap_start();
    delay(200); // Laisse le temps au serveur de réinitialiser ses routes/états
}

void tearDown(void)
{
    // Rien à détruire ici pour préserver la stabilité du Wi-Fi entre les tests
}

void test_login_failure_invalid_password(void)
{
    HTTPClient http;
    http.begin(String("http://") + TEST_IP + "/login");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Envoi d'un mauvais mot de passe
    int httpCode = http.POST("password=WrongPassword");

    TEST_ASSERT_EQUAL(200, httpCode); // Doit retourner la page HTML de login
    String body = http.getString();
    TEST_ASSERT_TRUE(body.indexOf("Mot de passe incorrect") >= 0);

    http.end();
}

void test_login_success_and_cookie_generation(void)
{
    HTTPClient http;
    http.begin(String("http://") + TEST_IP + "/login");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Déclarer les en-têtes à capturer dans la réponse
    const char *headerKeys[] = {"Set-Cookie", "Location"};
    http.collectHeaders(headerKeys, 2);

    int httpCode = http.POST("password=SecretPassword123");

    // Vérification de la redirection HTTP 302 vers le dashboard
    TEST_ASSERT_EQUAL(302, httpCode);
    TEST_ASSERT_EQUAL_STRING("/dashboard", http.header("Location").c_str());

    // Vérification de la création du cookie "session=..."
    String cookieHeader = http.header("Set-Cookie");
    TEST_ASSERT_TRUE(cookieHeader.startsWith("session="));
    TEST_ASSERT_TRUE(cookieHeader.indexOf("HttpOnly") >= 0);

    http.end();
}

void test_access_protected_route_with_cookie(void)
{
    HTTPClient http;

    // 1. Authentification pour récupérer le cookie de session
    http.begin(String("http://") + TEST_IP + "/login");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    const char *headerKeys[] = {"Set-Cookie"};
    http.collectHeaders(headerKeys, 1);
    http.POST("password=SecretPassword123");

    String sessionCookie = http.header("Set-Cookie");
    http.end();

    // Isolation du jeton session=... (exclusion des attributs Path, HttpOnly...)
    int cookieEndIndex = sessionCookie.indexOf(';');
    if (cookieEndIndex > 0)
    {
        sessionCookie = sessionCookie.substring(0, cookieEndIndex);
    }

    // 2. Tente d'accéder au dashboard SANS cookie (doit être redirigé vers /)
    http.begin(String("http://") + TEST_IP + "/dashboard");
    int unauthorizedCode = http.GET();
    TEST_ASSERT_EQUAL(302, unauthorizedCode);
    http.end();

    // 3. Tente d'accéder au dashboard AVEC cookie valide
    http.begin(String("http://") + TEST_IP + "/dashboard");
    http.addHeader("Cookie", sessionCookie);
    int authorizedCode = http.GET();

    // Succès : le serveur répond 200 OK (ou 404 si dashboard.html n'est pas flashé sur LittleFS)
    TEST_ASSERT_TRUE(authorizedCode == 200 || authorizedCode == 404);
    http.end();
}

void setup()
{
    delay(2000); // Temps pour la stabilisation du port Série
    UNITY_BEGIN();

    // Démarrage initial du SoftAP
    softap_set_password_hash("SecretPassword123");
    softap_start();

    // Lancement de la tâche FreeRTOS dédiée à softap_loop sur le cœur 0
    xTaskCreatePinnedToCore(
        webServerTask,
        "WebServerTask",
        4096,
        NULL,
        1,
        &g_webServerTaskHandle,
        0);

    delay(1000); // Laisse le temps au SoftAP de démarrer l'AP Wi-Fi

    // Exécution de la suite de tests Unity
    RUN_TEST(test_login_failure_invalid_password);
    RUN_TEST(test_login_success_and_cookie_generation);
    RUN_TEST(test_access_protected_route_with_cookie);

    // Arrêt propre de la tâche FreeRTOS et du SoftAP à la fin des tests
    if (g_webServerTaskHandle != NULL)
    {
        vTaskDelete(g_webServerTaskHandle);
        g_webServerTaskHandle = NULL;
    }
    softap_stop();

    UNITY_END();
}

void loop()
{
    // La boucle principale reste vide, softap_loop() est exécuté par la tâche FreeRTOS
}