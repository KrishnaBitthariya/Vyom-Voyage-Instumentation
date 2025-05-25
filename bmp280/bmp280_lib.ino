#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp;  // Create BMP280 object

unsigned long startTime;  // Variable to store startup time
unsigned long readTime;   // Variable to store time taken for sensor reading

void setup() {
    Serial.begin(115200);
    startTime = millis();  // Record time at startup

    if (!bmp.begin(0x76)) {  // Try 0x77 if 0x76 doesn't work
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        while (1);
    }

    unsigned long setupTime = millis() - startTime;  // Calculate setup time
    Serial.print("Setup completed in ");
    Serial.print(setupTime);
    Serial.println(" ms");
    delay(2000);
}

void loop() {
    unsigned long readStart = millis();  // Record time before reading data

    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure() / 100.0F); // Convert Pa to hPa
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bmp.readAltitude(1013.25)); // Adjust sea-level pressure as needed
    Serial.println(" m");

    readTime = millis() - readStart;  // Calculate time taken for reading
    Serial.print("Sensor read time: ");
    Serial.print(readTime);
    Serial.println(" ms");

    Serial.println();
    delay(2000);  // Delay before next reading
}
