

#include <Wire.h>

unsigned long startTime;  // Variable to store startup time
unsigned long readTime;   // Variable to store time taken for sensor reading


uint16_t dig_T1, dig_P1;
int16_t  dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5;
int16_t  dig_P6, dig_P7, dig_P8, dig_P9; 
float AltitudeBarometer, AltitudeBarometerStartUp;
int RateCalibrationNumber;
double pressure;

void barometer_signals(void){
  Wire.beginTransmission(0x76);
  Wire.write(0xF7);
  Wire.endTransmission();
  Wire.requestFrom(0x76,6);
  uint32_t press_msb = Wire.read();
  uint32_t press_lsb = Wire.read();
  uint32_t press_xlsb = Wire.read();
  uint32_t temp_msb = Wire.read();
  uint32_t temp_lsb = Wire.read();
  uint32_t temp_xlsb = Wire.read();
  
  unsigned long int adc_P = (press_msb << 12) | (press_lsb << 4) | (press_xlsb >>4);
  unsigned long int adc_T = (temp_msb << 12) | (temp_lsb << 4) | (temp_xlsb >>4);
  signed long int var1, var2;
  var1 = ((((adc_T >> 3) - ((signed long int )dig_T1 <<1)))* ((signed long int )dig_T2)) >> 11;
  var2 = (((((adc_T >> 4) - ((signed long int )dig_T1)) * ((adc_T>>4) - ((signed long int )dig_T1)))>> 12) * ((signed long int )dig_T3)) >> 14;
  signed long int t_fine = var1 + var2;
  unsigned long int p;
  var1 = (((signed long int )t_fine)>>1) - (signed long int )64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11) * ((signed long int )dig_P6);
  var2 = var2 + ((var1*((signed long int )dig_P5)) <<1);
  var2 = (var2>>2)+(((signed long int )dig_P4)<<16);
  var1 = (((dig_P3 * (((var1>>2)*(var1>>2)) >> 13))>>3)+((((signed long int )dig_P2) * var1)>>1))>>18;
  var1 = ((((32768+var1))*((signed long int )dig_P1)) >>15);
  if (var1 == 0) { p=0;}    
  p = (((unsigned long int )(((signed long int ) 1048576)-adc_P)-(var2>>12)))*3125;
  if(p<0x80000000){ p = (p << 1) / ((unsigned long int ) var1);}
  else { p = (p / (unsigned long int )var1) * 2;  }
  var1 = (((signed long int )dig_P9) * ((signed long int ) (((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((signed long int )(p>>2)) * ((signed long int )dig_P8))>>13;
  p = (unsigned long int)((signed long int )p + ((var1 + var2+ dig_P7) >> 4));
  pressure=(double)p/100;
  AltitudeBarometer=44330*(1-pow(pressure/1013.25, 1/5.255))*100;
}

void setup() {
  startTime = millis();  // Record time at startup
  Serial.begin(115200);
  Wire.setClock(400000);
  Wire.begin();

  delay(250);

  // Check if BMP280 is at 0x76 or 0x77
  Serial.println("Scanning for BMP280...");
  Wire.beginTransmission(0x76);
  if (Wire.endTransmission() != 0) {  
    Serial.println("BMP280 NOT found at 0x76, trying 0x77...");
    Wire.beginTransmission(0x77);
    if (Wire.endTransmission() != 0) {
      Serial.println("ERROR: BMP280 not found at 0x76 or 0x77! Check wiring.");
      while (1);
    } else {
      Serial.println("BMP280 found at 0x77.");
    }
  } else {
    Serial.println("BMP280 found at 0x76.");
  }

  // Configure sensor registers
  Wire.beginTransmission(0x76);
  Wire.write(0xF4);  
  Wire.write(0x57);  
  Wire.endTransmission();   

  Wire.beginTransmission(0x76);
  Wire.write(0xF5);
  Wire.write(0x14);
  Wire.endTransmission();   

  // Read calibration data
  uint8_t data[24]; 
  Wire.beginTransmission(0x76);
  Wire.write(0x88);  
  Wire.endTransmission();
  delay(10);  
  Wire.requestFrom(0x76, 24); 

  if (Wire.available() == 24) {  
    for (int i = 0; i < 24; i++) {
        data[i] = Wire.read();
    }
  } else {
    Serial.println("Error: Calibration data not received!");
  }

  // ✅ Assign values **before printing**
  dig_T1 = (data[1] << 8) | data[0]; 
  dig_T2 = (data[3] << 8) | data[2];
  dig_T3 = (data[5] << 8) | data[4];
  dig_P1 = (data[7] << 8) | data[6]; 
  dig_P2 = (data[9] << 8) | data[8];
  dig_P3 = (data[11]<< 8) | data[10];
  dig_P4 = (data[13]<< 8) | data[12];
  dig_P5 = (data[15]<< 8) | data[14];
  dig_P6 = (data[17]<< 8) | data[16];
  dig_P7 = (data[19]<< 8) | data[18];
  dig_P8 = (data[21]<< 8) | data[20];
  dig_P9 = (data[23]<< 8) | data[22]; 
  delay(250);

  // ✅ Now print calibration values
  Serial.print("dig_P1: "); Serial.println(dig_P1);
  Serial.print("dig_P2: "); Serial.println(dig_P2);
  Serial.print("dig_P3: "); Serial.println(dig_P3);
  Serial.print("dig_P4: "); Serial.println(dig_P4);
  Serial.print("dig_P5: "); Serial.println(dig_P5);
  Serial.print("dig_P6: "); Serial.println(dig_P6);
  Serial.print("dig_P7: "); Serial.println(dig_P7);
  Serial.print("dig_P8: "); Serial.println(dig_P8);
  Serial.print("dig_P9: "); Serial.println(dig_P9);

  if (dig_P1 == 0) {
    Serial.println("ERROR: Calibration data is still zero! Check sensor connection.");
    while (1);
  }

  for (RateCalibrationNumber=0; RateCalibrationNumber<2000; RateCalibrationNumber++) {
    barometer_signals();
    AltitudeBarometerStartUp += AltitudeBarometer;
    delay(1);
  }
  AltitudeBarometerStartUp /= 2000;

  unsigned long setupTime = millis() - startTime;
  Serial.print("Setup completed in ");
  Serial.print(setupTime);
  Serial.println(" ms");
}

void loop() {
  unsigned long readStart = millis();
  barometer_signals();
  AltitudeBarometer;
  pressure;
  // AltitudeBarometer-=AltitudeBarometerStartUp;
  Serial.print("Altitude [m]: ");
  Serial.println(AltitudeBarometer/100);
  Serial.print("Pressure: ");
  Serial.println(pressure);

  readTime = millis() - readStart;  // Calculate time taken for reading
    Serial.print("Sensor read time: ");
    Serial.print(readTime);
    Serial.println(" ms");

  delay(50);
}