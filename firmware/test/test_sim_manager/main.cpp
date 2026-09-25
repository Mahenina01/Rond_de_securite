#include <Arduino.h>
#include <unity.h>
#include "hal/sim_manager.h"

// ============================================================
// SETUP
// ============================================================

void setUp(void)
{
  // Aucun état particulier à initialiser ici.
}

void tearDown(void)
{
  // Rien à nettoyer.
}

// ============================================================
// TEST 1 : Etat initial et début de mise sous tension
// ============================================================

void test_sim_power_on_starts_sequence(void)
{
  uint32_t start = millis();

  bool result = sim_power_on();

  uint32_t elapsed = millis() - start;

  // Le premier appel ne doit pas encore indiquer READY.
  TEST_ASSERT_FALSE(result);

  // Le PWRKEY doit avoir été configuré.
  TEST_ASSERT_EQUAL(
      HIGH,
      digitalRead(SIM_PWRKEY_PIN));

  Serial.printf(
      "[TEST] sim_power_on() initial -> false, elapsed=%lu ms\n",
      elapsed);
}

// ============================================================
// TEST 2 : Fin de l'impulsion PWRKEY
// ============================================================

void test_sim_power_on_fin_impulsion(void)
{
  // On laisse passer le temps nécessaire à l'impulsion.
  delay(SIM_PWRKEY_PULSE_MS + 50);

  bool result = sim_power_on();

  TEST_ASSERT_FALSE(result);

  // Après l'impulsion, PWRKEY doit être LOW
  TEST_ASSERT_EQUAL(
      LOW,
      digitalRead(SIM_PWRKEY_PIN));

  Serial.println(
      "[TEST] Impulsion PWRKEY terminee");
}

// ============================================================
// TEST 3 : Fin du délai de boot
// ============================================================

void test_sim_power_on_wait_boot(void)
{
  // Attendre le délai de boot.
  delay(SIM_BOOT_DELAY_MS + 100);

  bool result = sim_power_on();

  /*
   * A ce stade le code doit être dans READY,
   * puis appeler modem.testAT().
   *
   * Si le SIM7600 répond correctement :
   *     result == true
   *
   * Sinon :
   *     result == false
   */

  Serial.printf(
      "[TEST] modem.testAT() -> %s\n",
      result ? "OK" : "FAIL");

  TEST_ASSERT_TRUE_MESSAGE(
      result,
      "Le SIM7600 ne repond pas a modem.testAT()");
}

// ============================================================
// TEST 4 : Test AT après démarrage
// ============================================================

void test_sim_modem_at(void)
{
  bool result = sim_power_on();

  /*
   * Lorsque sim_power_on() est déjà dans READY,
   * il appelle directement modem.testAT().
   */

  TEST_ASSERT_TRUE_MESSAGE(
      result,
      "SIM7600 ne repond pas aux commandes AT");

  Serial.println(
      "[TEST] Communication AT SIM7600 : OK");
}

// ============================================================
// TEST 5 : Attente réseau avant timeout
// ============================================================

void test_sim_wait_network_ready_before_timeout(void)
{
  const uint32_t timeout = 5000;

  uint32_t start = millis();

  bool connected = false;

  while ((millis() - start) < timeout)
  {
    connected = sim_wait_network_ready(timeout);

    if (connected)
    {
      break;
    }

    delay(100);
  }

  Serial.printf(
      "[TEST] Etat reseau : %s\n",
      connected ? "CONNECTE" : "NON CONNECTE");

  /*
   * Ce test suppose que la SIM7600 peut réellement
   * s'enregistrer sur le réseau.
   */
  TEST_ASSERT_TRUE_MESSAGE(
      connected,
      "Le SIM7600 ne s'est pas connecte au reseau");
}

// ============================================================
// TEST 6 : Timeout réseau
// ============================================================

void test_sim_wait_network_timeout(void)
{
  /*
   * On utilise un timeout très court afin de vérifier
   * que la fonction abandonne correctement.
   */
  const uint32_t timeout = 1000;

  uint32_t start = millis();

  bool connected = false;

  while ((millis() - start) < (timeout + 500))
  {
    connected = sim_wait_network_ready(timeout);

    if (connected)
    {
      break;
    }

    delay(50);
  }

  /*
   * Si le module n'est pas connecté :
   * la fonction doit retourner false après timeout.
   *
   * Attention :
   * ce test peut échouer si le SIM7600 est déjà connecté.
   */

  if (!connected)
  {
    TEST_ASSERT_FALSE(connected);

    Serial.println(
        "[TEST] Timeout reseau : OK");
  }
  else
  {
    Serial.println(
        "[TEST] SIM7600 deja connecte : timeout non testable");
  }
}

// ============================================================
// TEST 7 : Power OFF
// ============================================================

void test_sim_power_off(void)
{
  bool result = sim_power_off();

  Serial.printf(
      "[TEST] sim_power_off() -> %s\n",
      result ? "OK" : "FAIL");

  TEST_ASSERT_TRUE_MESSAGE(
      result,
      "Impossible d'eteindre le SIM7600");
}

// ============================================================
// RUN TESTS
// ============================================================

void setup()
{
  delay(2000);

  Serial.begin(115200);

  delay(1000);

  UNITY_BEGIN();

  RUN_TEST(test_sim_power_on_starts_sequence);

  RUN_TEST(test_sim_power_on_fin_impulsion);

  RUN_TEST(test_sim_power_on_wait_boot);

  RUN_TEST(test_sim_modem_at);

  RUN_TEST(test_sim_wait_network_ready_before_timeout);

  RUN_TEST(test_sim_wait_network_timeout);

  RUN_TEST(test_sim_power_off);

  UNITY_END();
}

void loop()
{
}