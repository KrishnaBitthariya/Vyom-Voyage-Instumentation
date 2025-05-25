#include <Wire.h>
void setup() {
    Serial.begin(115200);
    Wire.begin();
    Serial.println("\nI2C Scanner Running...");
}
void loop() {
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.print("Device found at 0x");
            Serial.println(address, HEX);
            delay(500);
        }
    }
    delay(2000);
}
