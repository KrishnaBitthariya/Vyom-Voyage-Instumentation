#include <Wire.h>
#include <math.h>  // Include math library for atan2()

#define HMC5883L_ADDR 0x1E

// Calibration values (Replace these with real values after calibration)
float x_offset = 20, y_offset = -15;
float x_scale = 1.1, y_scale = 0.95;

void setup() {
    Serial.begin(9600);
    Wire.begin();
    initHMC5883L();
}

void loop() {
    int x_raw, y_raw, z_raw;
    readMagnetometer(x_raw, y_raw, z_raw);

    // Apply calibration (offset and scaling)
    float x_calibrated = (x_raw - x_offset) * x_scale;
    float y_calibrated = (y_raw - y_offset) * y_scale;

    // Compute heading
    float heading = atan2(y_calibrated, x_calibrated) * 180.0 / M_PI;
    
    // Adjust if negative
    if (heading < 0) {
        heading += 360;
    }

    // Print results
    Serial.print("X: "); Serial.print(x_calibrated);
    Serial.print(" Y: "); Serial.print(y_calibrated);
    Serial.print(" Heading: "); Serial.print(heading);
    Serial.println("°");

    delay(500);
}

void initHMC5883L() {
    Wire.beginTransmission(HMC5883L_ADDR);
    Wire.write(0x00);  
    Wire.write(0x18);  // 75 Hz sampling rate
    Wire.endTransmission();

    Wire.beginTransmission(HMC5883L_ADDR);
    Wire.write(0x01);  
    Wire.write(0x60);  // ±2.5 Gauss range
    Wire.endTransmission();

    Wire.beginTransmission(HMC5883L_ADDR);
    Wire.write(0x02);  
    Wire.write(0x00);  // Continuous mode
    Wire.endTransmission();
}

void readMagnetometer(int &x, int &y, int &z) {
    Wire.beginTransmission(HMC5883L_ADDR);
    Wire.write(0x03);
    Wire.endTransmission();
    Wire.requestFrom(HMC5883L_ADDR, 6);

    if (Wire.available() == 6) {
        x = (int16_t)((Wire.read() << 8) | Wire.read());
        z = (int16_t)((Wire.read() << 8) | Wire.read());
        y = (int16_t)((Wire.read() << 8) | Wire.read());
    }
}
