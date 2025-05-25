#include<Wire.h>
#include<math.h>

float ratePitch, rateRoll, rateYaw;
float calibration_ratePitch ,  calibration_rateRoll, calibration_rateYaw;
float accX, accY, accZ;
float angleRoll , anglePitch , myRoll , myPitch;
float gRoll , gPitch, gYaw =0.0;

unsigned long startmillis;


// Calibration values (Replace these with real values after calibration)
float x_offset, y_offset , z_offset;
float x_scale , y_scale , z_scale;
float scale_avg , x_correlation, y_correlation, z_correlation;

int x_min = 32767, x_max = -32768;  // Initialize extreme values
int y_min = 32767, y_max = -32768;  // Initialize extreme values
int z_min = 32767, z_max = -32768;  // Initialize extreme values
#define HMC5883L_ADDR 0x1E

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


void gyro_read(void){

  Wire.beginTransmission(0x68);
  Wire.write(0x43);  // is register pe point kiya sensor ke idhar khada rahe . ye register sensor value store rakh tha hai x axis high 
  Wire.endTransmission();

  Wire.requestFrom(0x68 , 6);
  int16_t gyroX = Wire.read() <<8 | Wire.read();  // phele x high ata use left shift kar ke jagaha banaya hai phir low ko add akrte toh x hojata sae 
  int16_t gyroY = Wire.read() <<8 | Wire.read();   // idhar same 3, 4 byte as y high and y low ko kiya respectively as same above 
  int16_t gyroZ = Wire.read() <<8 | Wire.read(); 


  rateRoll = (float)gyroX/ 32.8 ;
  ratePitch  = (float)gyroY/ 32.8 ;   
  rateYaw   = (float)gyroZ/ 32.8 ;

  float deltatime = (millis() - startmillis)*0.001;  // Convert to seconds

  gRoll =gRoll +(rateRoll) * deltatime;
  gPitch =gPitch +(ratePitch ) * deltatime;
  gYaw = gYaw+(rateYaw ) * deltatime;

  startmillis = millis();  // Store the current time
}

void acc_read(void){
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();

  Wire.requestFrom(0x68 , 6);
  int16_t accX_lsb = Wire.read() << 8 | Wire.read();
  int16_t accY_lsb = Wire.read() << 8 | Wire.read();
  int16_t accZ_lsb = Wire.read() << 8 | Wire.read();

  accX = (float) accX_lsb / 4096;
  accY = (float) accY_lsb / 4096;
  accZ = ((float) accZ_lsb / 4096) -90;

  angleRoll = atan2(accY, sqrt(accX * accX + accZ * accZ)) * (180.0 / M_PI);
  anglePitch = -atan2(accX, sqrt(accY * accY + accZ * accZ)) * (180.0 / M_PI); // atan2 give output in radianss 

}


void setup(){
  startmillis = millis();

  Serial.begin(115200);
  Wire.setClock(400000); // i2c speed set kiya hai 400khz as data sheet meh likha tha comms speed of the sensor 
  Wire.begin();
  delay(250);

  initHMC5883L();

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);  
  Wire.write(0x00);  // Wake up MPU6050
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1B);  
  Wire.write(0x10);  // Set gyro range to ±1000 dps (32.8 LSB/dps)
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);  // Set accelerometer range to ±8g (4096 LSB/g)
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);  // Enable 10 Hz low-pass filter
  Wire.endTransmission();


  for (int i = 0 ;  i <2000; i++){
    gyro_read();
    calibration_ratePitch += ratePitch;
    calibration_rateRoll  += rateRoll;
    calibration_rateYaw   += rateYaw;
  }
  calibration_ratePitch = calibration_ratePitch/2000;
  calibration_rateRoll = calibration_rateRoll/2000;
  calibration_rateYaw = calibration_rateYaw/2000;
  delay(1000);
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
   Serial.println("done");


}

void loop(){
  int x_raw, y_raw, z_raw;
  readMagnetometer(x_raw, y_raw, z_raw);

  // Apply calibration (offset and scaling)
  float x_calibrated = ((x_raw - x_offset) * x_correlation) * 0.92;  // 0.92 = sensitivity scale ye karna meh bhul gaya 
  float y_calibrated = ((y_raw - y_offset) * y_correlation) * 0.92;
  float z_calibrated = ((z_raw - z_offset) * z_correlation) * 0.92;



  // Compute heading
  float heading = atan2(y_calibrated, x_calibrated) * 180.0 / M_PI;

    
  heading += 90;  // Shift by 90 degrees

  if (heading < 0) heading += 360;
  if (heading > 360) heading -= 360;

  Serial.print("X: "); Serial.print(x_calibrated);
  Serial.print(" Y: "); Serial.print(y_calibrated);
  Serial.print(" Heading: "); Serial.print(heading);
  Serial.println("°");

  delay(50);

}
