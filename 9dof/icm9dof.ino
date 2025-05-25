//ESP8266 IMU Pitch & Roll using ICM-20948
#include <Wire.h>
#include "ICM_20948.h"

#define SDA_PIN D2
#define SCL_PIN D1

ICM_20948_I2C imu;

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    
    if (imu.begin(Wire, 0x69) != ICM_20948_Stat_Ok) {
        Serial.println("IMU not detected! Check wiring.");
        while (1);
    }
    Serial.println("IMU detected.");
}

void loop() {
    if (imu.dataReady()) {
        imu.getAGMT();  // Read accelerometer, gyro, magnetometer, and temperature data
        
        float ax = imu.accX();
        float ay = imu.accY();
        float az = imu.accZ();
        
        // Compute pitch and roll
        float pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / PI;
        float roll  = atan2(-ax, az) * 180.0 / PI;
        
        Serial.print("Pitch: "); Serial.print(pitch);
        Serial.print("°, Roll: "); Serial.println(roll);
    } else {
        Serial.println("Waiting for IMU data...");
    }
    delay(100);
}