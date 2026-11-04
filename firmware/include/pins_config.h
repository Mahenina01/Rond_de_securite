#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// RFID RC522 - SPI
#define RFID_CS_PIN 5       // GP105 (SDA/SS)
#define RFID_RST_PIN 27     // GP1027
#define RFID_SCK_PIN 18     // GPIO18
#define RFID_MISO_PIN 19    // GPIO19
#define RFID_MOSI_PIN 23    // GPIO23
#define RFID_IRQ_PIN 4      // GPIO4

// RTC - DS3231 (I2C)
#define RTC_SDA_PIN 21      // GP1021
#define RTC_SCL_PIN 22      // GP1022

// SIM7600E-H - UART2 (commandes AT)
#define SIM_TX_PIN 16               // ESP32 TX2 -> RX SIM7600 (GPIO16)
#define SIM_RX_PIN 17               // ESP32 RX2 <- TX SIM7600 (GPIO17)
#define SIM_BAUDRATE 115200
#define SIM_POWER_ENABLE_PIN 15     // GPIO15 - Commande alim SIM (P-MOSFET Q2)
#define SIM_PWRKEY_PIN 2            // GPIO2 - Contrôle PWRKEY
#define SIM_RESET_PIN 33            // GPIO33
#define SIM_UART_NUM 2
#define SIM_STATUS_PIN 32           // GPIO32

// Bouton poussoir & Indicateurs
#define BTN_1_PIN 25        // GP1025 (BOUTTON OK / SW2)
#define BTN_2_PIN -1        // SW1 est le bouton de RESET matériel (EN)
#define LED_RED_PIN 13      // GPI013
#define LED_GREEN_PIN 12    // GPI012
#define LED_BLUE_PIN 14     // GPIO14
#define BUZZER_PIN 26       // GP1026 (Transistor Buzzer Q1)

// Auto-maintien alimentation & Batterie
#define POWER_LATCH_PIN 34  // GP1034 - Coupure charge batterie (Q3)
#define BATTERY_ADC_PIN 35  // GP1035 - Mesure tension batterie

// Ratio R11 (3.3k) + R12 (10k) / R12 (10k) = 13.3 / 10 = 1.33
#define BATTERY_VOLTAGE_DIVIDER_RATIO 1.33f

#endif // PINS_CONFIG_H