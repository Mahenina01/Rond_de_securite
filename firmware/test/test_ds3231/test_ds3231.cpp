// test/test_rtc_manager/test_main.cpp
// Conversion Unity du test manuel main.cpp, avec l'interface rtc_manager.h

#include <Arduino.h>
#include <unity.h>
#include "./hal/rtc_ds3231.h"
#include "pins_config.h"

static bool initDone = false;

// Execute avant CHAQUE test -- initialise le RTC une seule fois.
void setUp(void) {
    if (!initDone) {
        TEST_ASSERT_TRUE_MESSAGE(rtc_init(RTC_SDA_PIN, RTC_SCL_PIN),
                                  "RTC non detecte sur le bus I2C");
        initDone = true;
    }
}

void tearDown(void) {
}

// Ancien "Test 2" : la validite AVANT reglage depend de l'etat de la pile
// de sauvegarde -- resultat non deterministe au premier run. On ne peut pas
// l'affirmer vraie ou fausse : TEST_MESSAGE() informe sans faire echouer le test.
void test_rtc_is_valid_before_sync_is_informational(void) {
    bool validBefore = rtc_is_valid();
    if (validBefore) {
        Serial.println("RTC deja valide avant synchronisation (pile OK)");
    } else {
        Serial.println("RTC invalide avant synchronisation (normal si jamais reglee)");
    }
}

void test_rtc_sync_and_read_iso8601(void) {
    TEST_ASSERT_TRUE(rtc_sync_from_string("2026-08-30T12:00:00Z"));

    char buf[RTC_ISO8601_BUF_SIZE];
    TEST_ASSERT_TRUE(rtc_now_iso8601(buf, sizeof(buf)));
    TEST_ASSERT_EQUAL_STRING("2026-08-30T12:00:00Z", buf);
}

void test_rtc_now_epoch_is_non_zero(void) {
    rtc_sync_from_string("2026-08-30T12:00:00Z");
    uint32_t epoch = rtc_now_epoch();
    TEST_ASSERT_NOT_EQUAL(0, epoch);
}

void test_rtc_is_valid_after_sync(void) {
    rtc_sync_from_string("2026-08-30T12:00:00Z");
    TEST_ASSERT_TRUE(rtc_is_valid());
}

void setup() {
    Serial.begin(115200);
    delay(2000);   // laisse le moniteur serie s'attacher (usage normal en test Unity)

    UNITY_BEGIN();
    RUN_TEST(test_rtc_is_valid_before_sync_is_informational);
    RUN_TEST(test_rtc_sync_and_read_iso8601);
    RUN_TEST(test_rtc_now_epoch_is_non_zero);
    RUN_TEST(test_rtc_is_valid_after_sync);
    UNITY_END();
}

void loop() {
    // vide : Unity ne boucle pas
}