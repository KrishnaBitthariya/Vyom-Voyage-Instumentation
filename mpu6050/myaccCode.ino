#include<Wire.h>

float ratePitch, rateRoll, rateYaw;
float calibration_ratePitch ,  calibration_rateRoll, calibration_rateYaw;
float accX, accY, accZ;
float angleRoll , anglePitch , myRoll , myPitch;
float gRoll , gPitch, gYaw =0.0;
unsigned long startmillis;



void gyro_read(void){
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);   // accesing the registor of gyro the scale range
  Wire.write(0x8);  // accesing the range of 65.5 dps (degree / s) 0000100 = 0x08. 500dps range
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x43);  // is register pe point kiya sensor ke idhar khada rahe . ye register sensor value store rakh tha hai x axis high 
  Wire.endTransmission();

  Wire.requestFrom(0x68 , 6);
  int16_t gyroX = Wire.read() <<8 | Wire.read();  // phele x high ata use left shift kar ke jagaha banaya hai phir low ko add akrte toh x hojata sae 
  int16_t gyroY = Wire.read() <<8 | Wire.read();   // idhar same 3, 4 byte as y high and y low ko kiya respectively as same above 
  int16_t gyroZ = Wire.read() <<8 | Wire.read(); 

  rateRoll = (float)gyroX/ 65.5 ;
  ratePitch  = (float)gyroY/ 65.5 ;   
  rateYaw   = (float)gyroZ/ 65.5 ;

  float deltatime = (millis() - startmillis) / 1000.0;  // Convert to seconds

  gRoll =gRoll +(rateRoll) * deltatime;
  gPitch =gPitch +(ratePitch ) * deltatime;
  gYaw = gYaw+(rateYaw ) * deltatime;

  startmillis = millis();  // Store the current time
}

void acc_read(void){
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);  // activating staring acc 
  Wire.write(0x10); // +- 8g full scale of range 
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();

  Wire.requestFrom(0x68 , 6);
  int16_t accX_lsb = Wire.read() << 8 | Wire.read();
  int16_t accY_lsb = Wire.read() << 8 | Wire.read();
  int16_t accZ_lsb = Wire.read() << 8 | Wire.read();

  accX = (float) accX_lsb / 4096;
  accY = (float) accY_lsb / 4096;
  accZ = (float) accZ_lsb / 4096;

  accZ = accZ - 0.90;

  angleRoll=atan(accY/sqrt(accX*accX+accZ*accZ))*1/(3.142/180);
  anglePitch=-atan(accX/sqrt(accY*accY+accZ*accZ))*1/(3.142/180);

  // myRoll = atan(accY/accZ)*180/3.142;
  // myPitch = atan(-accX/sqrt(accY*accY + accZ*accZ))*180/3.142;

}


void setup(){

  Serial.begin(115200);
  startmillis = millis();

  Wire.setClock(400000); // i2c speed set kiya hai 400khz as data sheet meh likha tha comms speed of the sensor 
  Wire.begin();
  delay(250);

  Wire.beginTransmission(0x68);  //. sensor ka address likha haia
  Wire.write(0x6B);      // 
  Wire.write(0x00);      // jab tak sare sensor ko 0 nahi karte woh chalu nahi hoga
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);  // activating low pass filter
  Wire.write(0x05); // 10hz of range 
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
}

void loop(){
  acc_read();
  // Serial.print("Acceleration X [g]= ");
  // Serial.print(accX );
  // Serial.print(" Acceleration Y [g]= ");
  // Serial.print(accY);
  // Serial.print(" Acceleration Z [g]= ");
  // Serial.println(accZ-0.91);
  // delay(50);

  // Serial.print(accX );
  // Serial.print(",");
  // Serial.print(accY);
  // Serial.print(",");
  // Serial.print(accZ);


  // Serial.print("angle roll= ");
  // Serial.print(angleRoll );
  // Serial.print(" angle pitch= ");
  // Serial.println(anglePitch);

  gyro_read();
  Serial.print(gRoll);
  Serial.print(",");
  Serial.print(gPitch);
  Serial.print(",");
  Serial.println(gYaw);
  delay(50);
  // Serial.print(" ROll Rate [°/s]= ");
  // Serial.print(rateRoll - calibration_rateRoll);
  // Serial.print(" Pitch rate [°/s]= ");
  // Serial.print(ratePitch - calibration_ratePitch);
  // Serial.print(" Yaw Rate [°/s]= ");
  // Serial.println(rateYaw - calibration_rateYaw);
  // delay(50);
}
