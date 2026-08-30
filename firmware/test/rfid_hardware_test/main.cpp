#include <Arduino.h>
#include "../include/pins_config.h"
#include "../include/hal/rfid_rc522.h" // adapter le nom si renommé (ex: hal/rfid_rc522.h)

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  delay(500);

  Serial.println("=== Test RFID_Init ===");
  bool ok = RFID_Init(RFID_CS_PIN, RFID_RST_PIN, RFID_IRQ_PIN,
                      RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN);

  Serial.print("RFID_Init -> ");
  Serial.println(ok ? "OK (module détecté)" : "ECHEC (vérifier câblage/alim)");

  if (!ok)
  {
    Serial.println("Arrêt du test : impossible de continuer sans module valide.");
    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("Approchez un badge du lecteur...");
}

void loop()
{
  if (RFID_IsCardPresent())
  {
    char uid[RFID_UID_MAX_STR_LEN];

    if (RFID_ReadUID(uid, sizeof(uid)))
    {
      Serial.print("Badge détecté, UID = ");
      Serial.println(uid);
    }
    else
    {
      Serial.println("Badge détecté mais échec de lecture UID");
    }

    delay(1000); // anti-rebond simple pour le test
  }
}