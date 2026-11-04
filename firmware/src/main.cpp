/**
 * Test manuel avec materiel - HAL RFID RC522
 *
 * Emplacement : tools/rfid_manual_test.cpp (racine du projet)
 * Lancement   : pio run -e rfid_manual -t upload && pio device monitor -e rfid_manual
 *
 * Commandes (moniteur serie, 115200 bauds, envoyer un caractere) :
 *   h : aide
 *   i : re-initialise le RC522 (RFID_Init)
 *   p : RFID_PowerDown (antenne coupee)
 *   w : RFID_WakeUp
 *   q : RFID_SetupWakeupIRQ (configure l'IRQ)
 *   s : etat (nombre de lectures, niveau de la broche IRQ)
 *
 * Le niveau de la broche IRQ est surveille en permanence : tout changement est affiche.
 * Apres une lecture, le badge est en HALT : retirez-le puis reposez-le pour une nouvelle lecture.
 */

#include <Arduino.h>
#include "pins_config.h"
#include "hal/rfid_rc522.h"

static bool g_awake = true;
static uint32_t g_readCount = 0;
static int g_lastIrq = -1;

static void printHelp()
{
  Serial.println();
  Serial.println("=== Test manuel RFID RC522 ===");
  Serial.println("h : aide");
  Serial.println("i : re-init RC522");
  Serial.println("p : PowerDown");
  Serial.println("w : WakeUp");
  Serial.println("q : SetupWakeupIRQ");
  Serial.println("s : etat");
  Serial.println("Presentez un badge pour lire son UID.");
  Serial.println();
}

static void doInit()
{
  bool ok = RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN,
                      RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);
  g_awake = true;
  Serial.println(ok ? "[OK]   RC522 detecte et initialise"
                    : "[ERREUR] RC522 non detecte : verifier cablage SPI, 3.3V, GND, RST");
}

static void handleCommand(char c)
{
  switch (c)
  {
  case 'h':
    printHelp();
    break;
  case 'i':
    doInit();
    break;
  case 'p':
    RFID_PowerDown();
    g_awake = false;
    Serial.println("[OK]   PowerDown : antenne coupee, aucun badge ne sera lu");
    break;
  case 'w':
    RFID_WakeUp();
    g_awake = true;
    Serial.println("[OK]   WakeUp : lecture reactivee");
    break;
  case 'q':
    RFID_SetupWakeupIRQ();
    Serial.println("[OK]   IRQ configuree (surveillez les changements de niveau IRQ)");
    break;
  case 's':
    Serial.printf("[ETAT] reveille=%s  lectures=%lu  IRQ=%d\n",
                  g_awake ? "oui" : "non", (unsigned long)g_readCount, digitalRead(RFID_IRQ_PIN));
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1500);
  printHelp();
  doInit();
  g_lastIrq = digitalRead(RFID_IRQ_PIN);
}

void loop()
{
  while (Serial.available())
    handleCommand((char)Serial.read());

  // Surveillance de la broche IRQ
  int irq = digitalRead(RFID_IRQ_PIN);
  if (irq != g_lastIrq)
  {
    Serial.printf("[IRQ]  GPIO%d -> %s\n", RFID_IRQ_PIN, irq ? "HIGH" : "LOW");
    g_lastIrq = irq;
  }

  // Lecture de badge
  if (g_awake && RFID_IsCardPresent())
  {
    char uid[RFID_UID_MAX_STR_LEN];
    if (RFID_ReadUID(uid, sizeof(uid)))
    {
      ++g_readCount;
      Serial.printf("[BADGE] #%lu  UID = %s  (%u octets)\n",
                    (unsigned long)g_readCount, uid, (unsigned)(strlen(uid) / 2));
    }
    else
    {
      Serial.println("[ERREUR] Badge detecte mais lecture de l'UID impossible");
    }
  }

  delay(50);
}