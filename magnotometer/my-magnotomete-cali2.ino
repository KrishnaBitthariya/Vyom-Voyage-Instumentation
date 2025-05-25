#include <Wire.h>
#include <math.h>  // Include math library for atan2()

#define HMC5883L_ADDR 0x1E

// Calibration values (Replace these with real values after calibration)
float x_offset, y_offset , z_offset;
float x_scale , y_scale , z_scale;
float scale_avg , x_correlation, y_correlation, z_correlation;

int x_min = 32767, x_max = -32768;  // Initialize extreme values
int y_min = 32767, y_max = -32768;  // Initialize extreme values
int z_min = 32767, z_max = -32768;  // Initialize extreme values


void setup() {
  Serial.begin(115200);
  Wire.setClock(400000); // i2c speed set kiya hai 400khz as data sheet meh likha tha comms speed of the sensor 
  Wire.begin();
  delay(250);
  initHMC5883L();
  Serial.println("Rotate the sensor in all posible direction ");
  delay(500);


  for (int i = 0; i<2000; i++){
  int x_raw, y_raw, z_raw;
  readMagnetometer(x_raw, y_raw, z_raw);
  if (x_raw < x_min) x_min = x_raw;  // Update x_min if a smaller value is found
  if (x_raw > x_max) x_max = x_raw;  // Update x_max if a larger value is found
  if (y_raw < y_min) y_min = y_raw;  // Update y_min if a smaller value is found
  if (y_raw > y_max) y_max = y_raw;  // Update y_max if a larger value is found
  if (z_raw < z_min) z_min = z_raw;  // Update z_min if a smaller value is found
  if (z_raw > z_max) z_max = z_raw;  // Update z_max if a larger value is found
  delay(10);
  }
  // hard iron calibraition
    x_offset = (x_max + x_min)/2;
    y_offset = (y_max + y_min)/2;
    z_offset = (z_max + z_min)/2;
  // soft iron calibration
    x_scale = (x_max - x_min)/2;
    y_scale = (y_max - y_min)/2;
    z_scale = (z_max - z_min)/2;

   scale_avg = (x_scale + y_scale + z_scale)/3;
   x_correlation = scale_avg / x_scale;
   y_correlation = scale_avg / y_scale;
   z_correlation = scale_avg / z_scale;
}

void loop() {
    int x_raw, y_raw, z_raw;
    readMagnetometer(x_raw, y_raw, z_raw);

    // Apply calibration (offset and scaling)
    float x_calibrated = (x_raw - x_offset) * x_correlation;
    float y_calibrated = (y_raw - y_offset) * y_correlation;
    float z_calibrated = (z_raw - z_offset) * z_correlation;

    // Compute heading
    float heading = atan2(y_calibrated, x_calibrated) * 180.0 / M_PI;
    
    // Adjust if negative
    if (heading < 0) {heading += 360;} // output - 180 degree to 180 degree meh ata hai to use 0 to 360degree meh kar diya hai
    // if (heading > 360) {heading -= 360;} // agar galti se above 360 jae toh 0 pe lade wapis 
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
