
// GROUPE A : sans matériel — comportement défensif avant RFID_Init().
// GROUPE B : avec matériel RC522 câblé (ou virtualisé Wokwi) — certains tests
//            nécessitent en plus la présentation manuelle d'un badge.

#include <Arduino.h>
#include <unity.h>
#include "hal/rfid_rc522.h"
#include "pins_config.h"

namespace
{

    bool g_hardwareInitOk = false;

    // Attend jusqu'à timeoutMs qu'un badge soit présenté. Affiche une consigne
    // claire pour le testeur humain, car cette étape ne peut pas être automatisée.
    bool waitForCardTap(uint32_t timeoutMs)
    {
        Serial.printf("[ACTION REQUISE] Presentez un badge dans les %lu ms...\n", timeoutMs);
        unsigned long start = millis();
        while (millis() - start < timeoutMs)
        {
            if (RFID_IsCardPresent())
                return true;
            delay(50);
        }
        return false;
    }

}

void test_is_card_present_false_before_init(void)
{
    // ce test ne vaut que si RFID_Init() n'a pas encore été appelé
    // dans ce run — voir l'ordre d'exécution dans setup() plus bas.
    TEST_ASSERT_FALSE(RFID_IsCardPresent());
}

void test_read_uid_false_before_init(void)
{
    char uid[RFID_UID_MAX_STR_LEN];
    TEST_ASSERT_FALSE(RFID_ReadUID(uid, sizeof(uid)));
}

void test_setup_wakeup_irq_does_not_crash_before_init(void)
{
    RFID_SetupWakeupIRQ(); // rc522 == nullptr -> doit juste retourner, sans planter
    TEST_PASS();
}

void test_power_down_does_not_crash_before_init(void)
{
    RFID_PowerDown();
    TEST_PASS();
}

void test_wake_up_does_not_crash_before_init(void)
{
    RFID_WakeUp();
    TEST_PASS();
}

// =============================================================================
// GROUPE B — Nécessite RFID_Init() réussi (RC522 câblé)
// =============================================================================

void test_init_detects_module(void)
{
    TEST_ASSERT_TRUE_MESSAGE(g_hardwareInitOk,
                             "RFID_Init a echoue : verifier cablage SPI / alimentation 3.3V du RC522");
}

void test_read_uid_rejects_buffer_too_small(void)
{
    if (!g_hardwareInitOk)
        TEST_IGNORE_MESSAGE("Materiel RC522 non disponible");

    // La garde sur maxLen s'applique AVANT toute lecture reelle :
    // ce test est deterministe, meme sans badge presente.
    char tooSmall[RFID_UID_MAX_STR_LEN - 1];
    TEST_ASSERT_FALSE(RFID_ReadUID(tooSmall, sizeof(tooSmall)));
}

void test_is_card_present_false_without_card(void)
{
    if (!g_hardwareInitOk)
        TEST_IGNORE_MESSAGE("Materiel RC522 non disponible");
    // Suppose qu'aucun badge ne traine pres du lecteur au demarrage du test.
    TEST_ASSERT_FALSE(RFID_IsCardPresent());
}

// --- Tests interactifs : necessitent un geste manuel du testeur ---

void test_read_uid_returns_valid_hex_on_manual_tap(void)
{
    if (!g_hardwareInitOk)
        TEST_IGNORE_MESSAGE("Materiel RC522 non disponible");

    bool present = waitForCardTap(10000);
    TEST_ASSERT_TRUE_MESSAGE(present, "Aucun badge detecte dans le delai imparti");

    char uid[RFID_UID_MAX_STR_LEN];
    TEST_ASSERT_TRUE(RFID_ReadUID(uid, sizeof(uid)));

    size_t len = strlen(uid);
    TEST_ASSERT_TRUE_MESSAGE(len > 0 && len % 2 == 0, "Longueur UID invalide (doit etre paire)");
    TEST_ASSERT_TRUE_MESSAGE(len <= RFID_UID_MAX_STR_LEN - 1, "UID depasse la taille max attendue");

    for (size_t i = 0; i < len; ++i)
    {
        char c = uid[i];
        bool isHex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        TEST_ASSERT_TRUE_MESSAGE(isHex, "Caractere non-hexadecimal dans l'UID");
    }
}

void test_power_cycle_then_manual_tap_still_works(void)
{
    if (!g_hardwareInitOk)
        TEST_IGNORE_MESSAGE("Materiel RC522 non disponible");

    RFID_PowerDown();
    delay(50);
    RFID_WakeUp();

    bool present = waitForCardTap(10000);
    TEST_ASSERT_TRUE_MESSAGE(present, "Lecture impossible apres un cycle PowerDown/WakeUp");
}

// =============================================================================
// Runner Unity
// =============================================================================
void setup()
{
    delay(2000);
    Serial.begin(115200);
    delay(500);

    UNITY_BEGIN();

    Serial.println("Bonjour");

    // --- Groupe A : executé AVANT RFID_Init() pour valider l'etat "non initialise" ---
    RUN_TEST(test_is_card_present_false_before_init);
    RUN_TEST(test_read_uid_false_before_init);
    RUN_TEST(test_setup_wakeup_irq_does_not_crash_before_init);
    RUN_TEST(test_power_down_does_not_crash_before_init);
    RUN_TEST(test_wake_up_does_not_crash_before_init);

    // --- Initialisation matérielle, une seule fois (static local dans RFID_Init) ---
    g_hardwareInitOk = RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN,
                                 RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);

    // --- Groupe B ---
    RUN_TEST(test_init_detects_module);
    RUN_TEST(test_read_uid_rejects_buffer_too_small);
    RUN_TEST(test_is_card_present_false_without_card);
    RUN_TEST(test_read_uid_returns_valid_hex_on_manual_tap);
    RUN_TEST(test_power_cycle_then_manual_tap_still_works);

    UNITY_END();
}

void loop()
{
}