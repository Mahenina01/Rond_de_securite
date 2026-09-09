#include <Arduino.h>

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("Rond de securite - firmware OK");
}

void loop()
{
    Serial.println("Execution principale...");
    delay(1000);
}
