#include <Wire.h>
#include <math.h>

// ICM-20948 I2C Address
#define ICM20948_ADDR 0x69
#define MAG_ADDR 0x0C  // Magnetometer address inside ICM-20948

// Variables for storing sensor data
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float roll, pitch, yaw;

// Time tracking for gyroscope integration
unsigned long prevMillis = 0;

// Magnetometer calibration variables
float magX_offset, magY_offset, magZ_offset;
float magX_scale, magY_scale, magZ_scale;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(100);

    // Initialize ICM-20948
    initICM20948();
    
    // Calibrate magnetometer
    calibrateMagnetometer();
}

void loop() {
    readAccel();
    readGyro();
    readMagnetometer();

    // Compute roll and pitch from accelerometer
    roll = atan2(accY, sqrt(accX * accX + accZ * accZ)) * (180.0 / M_PI);
    pitch = -atan2(accX, sqrt(accY * accY + accZ * accZ)) * (180.0 / M_PI);

    // Compute yaw (heading) from magnetometer
    yaw = atan2(magY, magX) * (180.0 / M_PI);
    if (yaw < 0) yaw += 360;

    // // Print sensor data
    Serial.print("Roll: "); Serial.print(roll);
    Serial.print(" Pitch: "); Serial.println(pitch);
    // Serial.print(" Yaw: "); Serial.println(yaw);
  //   Serial.print("MagX: "); Serial.print(magX);
  // Serial.print(" MagY: "); Serial.print(magY);
  // Serial.print(" MagZ: "); Serial.println(magZ);

  
    delay(50); // Adjust based on your requirement
}

// ---------------- ICM-20948 Initialization ---------------- //
void initICM20948() {
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x06);  // Power management register
    Wire.write(0x01);  // Enable the sensor
    Wire.endTransmission();

    // Set accelerometer range to ±8g
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x14);
    Wire.write(0x10);
    Wire.endTransmission();

    // Set gyroscope range to ±1000 dps
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x11);
    Wire.write(0x10);
    Wire.endTransmission();

    // Enable I2C master mode
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x03);  // USER_CTRL register
    Wire.write(0x20);  // Enable I2C master mode
    Wire.endTransmission();
    delay(10);

    // Enable I2C master clock (needed for magnetometer communication)
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x24);  // I2C_MST_CTRL register
    Wire.write(0x0D);  // Clock speed configuration
    Wire.endTransmission();
    delay(10);

    // Enable the magnetometer in continuous mode (100Hz)
    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x31);  // Control register 2 (CNTL2)
    Wire.write(0x08);  // Continuous measurement mode 2
    Wire.endTransmission();
    
    Serial.println("ICM-20948 with Magnetometer Initialized!");
}



// ---------------- Read Accelerometer ---------------- //
void readAccel() {
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x2D); // Accelerometer data register
    Wire.endTransmission();
    Wire.requestFrom(ICM20948_ADDR, 6);

    if (Wire.available() == 6) {
        int16_t accX_raw = Wire.read() << 8 | Wire.read();
        int16_t accY_raw = Wire.read() << 8 | Wire.read();
        int16_t accZ_raw = Wire.read() << 8 | Wire.read();

        accX = (float) accX_raw / 4096;  // Convert to g
        accY = (float) accY_raw / 4096;
        accZ = (float) accZ_raw / 4096;
    }
}

// ---------------- Read Gyroscope ---------------- //
void readGyro() {
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(0x33); // Gyroscope data register
    Wire.endTransmission();
    Wire.requestFrom(ICM20948_ADDR, 6);

    if (Wire.available() == 6) {
        int16_t gyroX_raw = Wire.read() << 8 | Wire.read();
        int16_t gyroY_raw = Wire.read() << 8 | Wire.read();
        int16_t gyroZ_raw = Wire.read() << 8 | Wire.read();

        gyroX = (float) gyroX_raw / 32.8;  // Convert to degrees per second
        gyroY = (float) gyroY_raw / 32.8;
        gyroZ = (float) gyroZ_raw / 32.8;
    }
}

// ---------------- Read Magnetometer ---------------- //
void readMagnetometer() {
    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x11); // Start from ST1 register (data ready status)
    Wire.endTransmission(false);
    Wire.requestFrom(MAG_ADDR, 7); // Request 7 bytes (ST1 + X, Y, Z + ST2)

    if (Wire.available() == 7) {
        byte ST1 = Wire.read();  // Status byte, should be checked if data is ready

        if (ST1 & 0x01) { // Check if data is ready
            int16_t magX_raw = Wire.read() | (Wire.read() << 8);
            int16_t magY_raw = Wire.read() | (Wire.read() << 8);
            int16_t magZ_raw = Wire.read() | (Wire.read() << 8);
            byte ST2 = Wire.read();  // Status 2, should be read to complete transaction

            if (!(ST2 & 0x08)) { // Check for magnetic overflow
                // Convert raw data to µT (values are in 0.15 µT per LSB)
                magX = magX_raw * 0.15;
                magY = magY_raw * 0.15;
                magZ = magZ_raw * 0.15;
            }
        }
    }
}


// ---------------- Magnetometer Calibration ---------------- //
void calibrateMagnetometer() {
    Serial.println("Rotate the sensor in all directions for magnetometer calibration...");
    int xMin = 32767, xMax = -32768;
    int yMin = 32767, yMax = -32768;
    int zMin = 32767, zMax = -32768;

    for (int i = 0; i < 1000; i++) {
        readMagnetometer();
        int x_raw = magX, y_raw = magY, z_raw = magZ;
      
        if (x_raw < xMin) xMin = x_raw;
        if (x_raw > xMax) xMax = x_raw;
        if (y_raw < yMin) yMin = y_raw;
        if (y_raw > yMax) yMax = y_raw;
        if (z_raw < zMin) zMin = z_raw;
        if (z_raw > zMax) zMax = z_raw;
      
        delay(10);
    }

    // Hard iron calibration
    magX_offset = (xMax + xMin) / 2;
    magY_offset = (yMax + yMin) / 2;
    magZ_offset = (zMax + zMin) / 2;

    // Soft iron calibration
    magX_scale = (xMax - xMin) / 2;
    magY_scale = (yMax - yMin) / 2;
    magZ_scale = (zMax - zMin) / 2;

    float avgScale = (magX_scale + magY_scale + magZ_scale) / 3;
    magX_scale = avgScale / magX_scale;
    magY_scale = avgScale / magY_scale;
    magZ_scale = avgScale / magZ_scale;

    Serial.println("Magnetometer calibration done.");
}
