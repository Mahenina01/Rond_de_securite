#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// RFID RC522 - SPI
#define RFID_CS_PIN 5
#define RFID_RST_PIN 27

#define RFID_SCK_PIN 18
#define RFID_MISO_PIN 19
#define RFID_MOSI_PIN 23
#define RFID_IRQ_PIN 34

// RTC - DS3231 (I2C)
#define RTC_SDA_PIN 21
#define RTC_SCL_PIN 22

// SIM7600E-H - UART2 (commandes AT)
#define SIM_TX_PIN 17 // ESP32 TX2 -> RX module SIM
#define SIM_RX_PIN 16 // ESP32 RX2 <- TX module SIM
#define SIM_BAUDRATE 115200
#define SIM_POWER_ENABLE_PIN 25 // SIM7600E-H - Contrôle alimentation (MOSFET P-Channel)
#define SIM_PWRKEY_PIN 26       // SIM7600E-H - Contrôle du module
#define SIM_RESET_PIN 33
#define SIM_UART_NUM 2
#define SIM_STATUS_PIN 32 // Lecture de l'état ON/OFF du module

// Bouton poussoir
#define BTN_1_PIN 34
#define BTN_2_PIN 35
#define LED_RED_PIN 2
#define LED_GREEN_PIN 0
#define LED_BLUE_PIN 4
#define BUZZER_PIN 4

// Auto-maintien alimentation
#define POWER_LATCH_PIN 27

#endif