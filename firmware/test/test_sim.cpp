
#include <Arduino.h>
#include "hal/sim_manager.h"

// =============================================
// CONFIGURATION
// =============================================

#define NETWORK_TIMEOUT_MS 120000

// =============================================
// ETATS DU TEST
// =============================================

bool modemReady = false;
bool networkTestActive = false;

uint32_t networkTestStart = 0;

// =============================================
// AFFICHAGE DES RESULTATS
// =============================================

void printResult(const char *name, bool success)
{
  Serial.print(success ? "[PASS] " : "[FAIL] ");
  Serial.println(name);
}

// =============================================
// TEST 1 : COMMUNICATION AT
// =============================================

void testAT()
{
  Serial.println("\n--- TEST COMMUNICATION AT ---");

  TinyGsm &modem = sim_get_modem();

  bool result = modem.testAT();

  printResult("Communication UART / AT", result);
}

// =============================================
// TEST 2 : INFORMATIONS MODEM
// =============================================

void testModemInfo()
{
  Serial.println("\n--- TEST INFORMATIONS MODEM ---");

  TinyGsm &modem = sim_get_modem();

  String info = modem.getModemInfo();

  if (info.length() > 0)
  {
    Serial.println("Informations modem :");
    Serial.println(info);

    printResult("Identification du modem", true);
  }
  else
  {
    printResult("Identification du modem", false);
  }
}

// =============================================
// TEST 3 : CARTE SIM
// =============================================

void testSIM()
{

  TinyGsm &modem = sim_get_modem();

  Serial.println("\n--- TEST CARTE SIM ---");

  int simStatus = modem.getSimStatus();

  Serial.print("Code SIM : ");
  Serial.println(simStatus);

  if (simStatus == SIM_READY)
  {
    Serial.println("[PASS] Carte SIM prete.");
  }
  else
  {
    Serial.println("[FAIL] Carte SIM non prete.");

    switch (simStatus)
    {
    case SIM_ERROR:
      Serial.println("[INFO] Erreur de lecture de la carte SIM.");
      break;

    case SIM_LOCKED:
      Serial.println("[INFO] Carte SIM verrouillee par un PIN.");
      break;

    default:
      Serial.println("[INFO] Etat SIM inconnu ou non gere.");
      break;
    }
  }
}

// =============================================
// TEST 4 : QUALITE DU SIGNAL
// =============================================

void testSignal()
{
  Serial.println("\n--- TEST SIGNAL 4G ---");

  TinyGsm &modem = sim_get_modem();

  int signal = modem.getSignalQuality();

  Serial.print("Qualite du signal : ");
  Serial.println(signal);

  if (signal == 99)
  {
    Serial.println(
        "[ATTENTION] Signal inconnu ou indisponible.");
    return;
  }

  if (signal == 0)
  {
    Serial.println(
        "[ATTENTION] Signal tres faible ou absent.");
    return;
  }

  Serial.printf(
      "RSSI approximatif : %d dBm\n",
      -113 + (2 * signal));

  printResult("Lecture signal modem", true);
}

// =============================================
// TEST 5 : ENREGISTREMENT RESEAU
// =============================================

void testNetwork()
{
  Serial.println("\n--- TEST ENREGISTREMENT RESEAU ---");

  TinyGsm &modem = sim_get_modem();

  bool connected = modem.isNetworkConnected();

  printResult(
      "Enregistrement sur le reseau mobile",
      connected);

  if (connected)
  {
    Serial.print("Operateur : ");
    Serial.println(modem.getOperator());

    Serial.print("Adresse IP : ");
    Serial.println(modem.getLocalIP());
  }
  else
  {
    Serial.println(
        "[INFO] Le modem n'est pas encore enregistre.");
  }
}

// =============================================
// TEST 6 : ATTENTE NON BLOQUANTE DU RESEAU
// =============================================

void startNetworkTest()
{
  Serial.println("\n--- ATTENTE RESEAU ---");

  networkTestStart = millis();
  networkTestActive = true;
}

void processNetworkTest()
{
  if (!networkTestActive)
    return;

  if (sim_wait_network_ready(NETWORK_TIMEOUT_MS))
  {
    Serial.println("\n[PASS] Reseau mobile disponible.");

    networkTestActive = false;

    testNetwork();

    return;
  }

  if (millis() - networkTestStart >= NETWORK_TIMEOUT_MS)
  {
    Serial.println("\n[FAIL] Timeout reseau.");

    networkTestActive = false;

    return;
  }

  static uint32_t lastPrint = 0;

  if (millis() - lastPrint >= 2000)
  {
    Serial.println("[INFO] Recherche du reseau...");
    lastPrint = millis();
  }
}

// =============================================
// TEST 7 : ARRET DU MODEM
// =============================================

void testPowerOff()
{
  Serial.println("\n--- ARRET MODEM ---");

  networkTestActive = false;

  bool result = sim_power_off();

  printResult("Commande d'arret modem", result);

  if (result)
  {
    modemReady = false;

    Serial.println("[OK] Modem arrete.");
  }
  else
  {
    Serial.println(
        "[ATTENTION] Arret non confirme.");
  }
}

// =============================================
// MENU
// =============================================

void printMenu()
{
  Serial.println("\n================================");
  Serial.println("       TEST SIM7600E-H");
  Serial.println("================================");

  Serial.println("1 - Communication AT");
  Serial.println("2 - Informations modem");
  Serial.println("3 - Etat carte SIM");
  Serial.println("4 - Qualite signal 4G");
  Serial.println("5 - Enregistrement reseau");
  Serial.println("6 - Attendre connexion reseau");
  Serial.println("7 - Arreter modem");
  Serial.println("h - Afficher menu");

  Serial.println("================================");
}

// =============================================
// INITIALISATION
// =============================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println("\n================================");
  Serial.println("   TEST MATERIEL SIM7600E-H");
  Serial.println("================================");

  Serial.println("[INFO] Demarrage du modem...");
}

// =============================================
// BOUCLE PRINCIPALE
// =============================================

void loop()
{
  // Sequence de demarrage non bloquante
  if (!modemReady)
  {
    if (sim_power_on())
    {
      modemReady = true;

      Serial.println("\n[OK] SIM7600E-H operationnel.");

      testAT();

      printMenu();
    }
    else
    {
      static uint32_t lastMessage = 0;

      if (millis() - lastMessage >= 2000)
      {
        Serial.println("[INFO] Initialisation modem...");
        lastMessage = millis();
      }
    }

    return;
  }

  // Traitement de la recherche reseau
  processNetworkTest();

  // Reception des commandes
  if (!Serial.available())
    return;

  char command = Serial.read();

  switch (command)
  {
  case '1':
    testAT();
    break;

  case '2':
    testModemInfo();
    break;

  case '3':
    testSIM();
    break;

  case '4':
    testSignal();
    break;

  case '5':
    testNetwork();
    break;

  case '6':
    startNetworkTest();
    break;

  case '7':
    testPowerOff();
    break;

  case 'h':
    printMenu();
    break;

  case '\n':
  case '\r':
    break;

  default:
    Serial.println("[ERREUR] Commande inconnue.");
    break;
  }
}
