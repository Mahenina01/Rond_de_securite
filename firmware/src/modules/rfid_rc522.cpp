#include "../include/hal/rfid_rc522.h"
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

namespace
{

  MFRC522 *rc522 = nullptr;
  MFRC522::MIFARE_Key defaultKey;
  uint8_t g_irqPin = 0;

  void bytesToHexString(const uint8_t *bytes, uint8_t len, char *out)
  {
    static const char hexDigits[] = "0123456789ABCDEF";
    for (uint8_t i = 0; i < len; ++i)
    {
      out[i * 2] = hexDigits[(bytes[i] >> 4) & 0x0F];
      out[i * 2 + 1] = hexDigits[bytes[i] & 0x0F];
    }
    out[len * 2] = '\0';
  }

} // namespace

bool RFID_Init(uint8_t ssPin, uint8_t rstPin, uint8_t irqPin,
               uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
{
  g_irqPin = irqPin;

  SPI.begin(sckPin, misoPin, mosiPin, ssPin);

  static MFRC522 instance(ssPin, rstPin);
  rc522 = &instance;
  rc522->PCD_Init();

  pinMode(g_irqPin, INPUT);

  uint8_t version = rc522->PCD_ReadRegister(MFRC522::VersionReg);
  return version != 0x00 && version != 0xFF;
}

bool RFID_IsCardPresent(void)
{
  if (rc522 == nullptr)
    return false;
  return rc522->PICC_IsNewCardPresent() && rc522->PICC_ReadCardSerial();
}

bool RFID_ReadUID(char *uidStrOut, size_t maxLen)
{
  if (rc522 == nullptr || uidStrOut == nullptr)
    return false;
  if (maxLen < RFID_UID_MAX_STR_LEN)
    return false;

  const MFRC522::Uid &uid = rc522->uid;
  if (uid.size == 0)
    return false;

  bytesToHexString(uid.uidByte, uid.size, uidStrOut);

  if (!rc522->PICC_HaltA())
  {
    Serial.println("[RFID] Avertissement : PICC_HaltA a échoué");
  }
  rc522->PCD_StopCrypto1();

  return true;
}

void RFID_SetupWakeupIRQ(void)
{
  if (rc522 == nullptr)
    return;

  // Active l'IRQ RxIRq sur la broche IRQ.
  rc522->PCD_WriteRegister(MFRC522::ComIEnReg, 0xA0); // IRqInv + RxIEn
  rc522->PCD_WriteRegister(MFRC522::ComIrqReg, 0x00); // clear flags

  // Lance une requête REQA en boucle d'attente passive
  uint8_t bufferATQA[2];
  uint8_t bufferSize = sizeof(bufferATQA);
  rc522->PICC_WakeupA(bufferATQA, &bufferSize);
}

void RFID_PowerDown(void)
{
  if (rc522 == nullptr)
    return;
  rc522->PCD_AntennaOff();
  rc522->PCD_WriteRegister(MFRC522::CommandReg, 0x10); // bit PowerDown
}

void RFID_WakeUp(void)
{
  if (rc522 == nullptr)
    return;
  rc522->PCD_WriteRegister(MFRC522::CommandReg, 0x00); // clear PowerDown
  delay(2);                                            // stabilisation de l'oscillateur
  rc522->PCD_AntennaOn();
}