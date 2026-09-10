
//
// GROUPE A : deterministe, sans interaction — verifie l'etat au demarrage normal.
// GROUPE B : simule un appui bouton via une tache FreeRTOS (aucun cavalier requis,
//            car BUTTON1_PIN supporte lecture ET ecriture sur le meme GPIO).
//

#include <Arduino.h>
#include <unity.h>
#include "hal/power_manager.h"
#include "pins_config.h"

namespace
{

  // Simule un relachement de bouton apres releaseAfterMs, dans une tache separee,
  // pendant que le thread principal reste bloque dans power_is_long_press().
  void releaseButtonTask(void *param)
  {
    uint32_t releaseAfterMs = *(uint32_t *)param;
    vTaskDelay(pdMS_TO_TICKS(releaseAfterMs));
    digitalWrite(BTN_1_PIN, HIGH); // relache (actif bas -> HIGH = relache)
    vTaskDelete(nullptr);
  }

  void simulatePressThenReleaseAfter(uint32_t releaseAfterMs)
  {
    pinMode(BTN_1_PIN, OUTPUT);
    digitalWrite(BTN_1_PIN, LOW); // presse immediatement

    static uint32_t delayParam;
    delayParam = releaseAfterMs;
    xTaskCreate(releaseButtonTask, "releaseBtn", 2048, &delayParam, 1, nullptr);
  }

} // namespace

void setUp(void)
{
  pinMode(BTN_1_PIN, INPUT_PULLUP); // etat par defaut : non presse
}

void tearDown(void)
{
  pinMode(BTN_1_PIN, INPUT_PULLUP); // restaure l'etat normal apres chaque test
}

// =============================================================================
// GROUPE A — Deterministe, sans interaction
// =============================================================================

void test_wake_reason_is_power_on_after_normal_boot(void)
{
  // Ce test suppose un demarrage normal (pas un reveil deep sleep) : c'est le
  // cas de tout run 'pio test' classique, qui flashe puis redemarre a froid.
  TEST_ASSERT_EQUAL(WakeReason::POWER_ON, power_get_wake_reason());
}

void test_configure_wake_sources_does_not_crash(void)
{
  power_configure_wake_sources();
  TEST_PASS();
}

void test_set_cpu_frequency_applies_requested_value(void)
{
  power_set_cpu_frequency(80);
  TEST_ASSERT_EQUAL_UINT32(80, getCpuFrequencyMhz());

  power_set_cpu_frequency(240);
  TEST_ASSERT_EQUAL_UINT32(240, getCpuFrequencyMhz());
}

// =============================================================================
// GROUPE B — Simulation logicielle du bouton (BUTTON1_PIN piloté en sortie)
// =============================================================================

void test_long_press_detected_when_held_past_threshold(void)
{
  simulatePressThenReleaseAfter(200);                // maintenu 200 ms
  bool result = power_is_long_press(BTN_1_PIN, 100); // seuil 100 ms
  TEST_ASSERT_TRUE(result);
}

void test_short_press_not_detected_below_threshold(void)
{
  simulatePressThenReleaseAfter(30);                 // maintenu seulement 30 ms
  bool result = power_is_long_press(BTN_1_PIN, 100); // seuil 100 ms
  TEST_ASSERT_FALSE(result);
}

void test_no_press_returns_false_immediately(void)
{
  // BUTTON1_PIN reste HIGH (non presse) — pas de simulation lancee.
  unsigned long start = millis();
  bool result = power_is_long_press(BTN_1_PIN, 5000);
  unsigned long elapsed = millis() - start;

  TEST_ASSERT_FALSE(result);
  TEST_ASSERT_LESS_THAN_UINT32(50, elapsed); // doit retourner immediatement, pas bloquer
}

// =============================================================================
// Runner Unity
// =============================================================================
void setup()
{
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_wake_reason_is_power_on_after_normal_boot);
  RUN_TEST(test_configure_wake_sources_does_not_crash);
  RUN_TEST(test_set_cpu_frequency_applies_requested_value);

  RUN_TEST(test_long_press_detected_when_held_past_threshold);
  RUN_TEST(test_short_press_not_detected_below_threshold);
  RUN_TEST(test_no_press_returns_false_immediately);

  UNITY_END();
}

void loop()
{
}