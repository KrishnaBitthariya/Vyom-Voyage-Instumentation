#include<Wire.h> // i2c lib scl D1 & sda D2 esp8266 

float ratePitch , rateRoll , rateYaw;
float calibration_ratePitch ,  calibration_rateRoll, calibration_rateYaw;

void gyro_read(void){
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);  // activating low pass filter
  Wire.write(0x05); // 10hz of range 
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x1B);   // accesing the registor of the scale range
  Wire.write(0x8);  // accesing the range of 65.5 dps (degree / s) 0000100 = 0x08. 500dps range
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.write(0x43);  // is register pe point kiya sensor ke idhar khada rahe . ye register sensor value store rakh tha hai x axis high 
  Wire.endTransmission();

  Wire.requestFrom(0x68 , 6);
  // sensor har axis ka store karta hai 16 bits meh par i2c meh ek baar meh 8 bits read kar sakte hai toh 
  // har axis ka data 2 part meh divide kiya hai like 
  //00010001 10001000
  //^- msb hai      ^ ye lsb hai 
  // toh x axis ka high = msb aur low = lsb ese do meh hai jo ek piche ek ate hai same y axis and z axis in binary bits 

  //idhar range of 16 bits hai -ve to +ve  
//       00111111 00000000   (MSB shifted left). high 
//     | 00000000 00101001   (LSB) low 
//      -------------------
//       00111111 00101001   (Final combined value)
 // idhar " | " or operator hai 
  int16_t gyroX = Wire.read() <<8 | Wire.read();  // phele x high ata use left shift kar ke jagaha banaya hai phir low ko add akrte toh x hojata sae 
  int16_t gyroY = Wire.read() <<8 | Wire.read();   // idhar same 3, 4 byte as y high and y low ko kiya respectively as same above 
  int16_t gyroZ = Wire.read() <<8 | Wire.read();   //. 5,6 byte same 

  // jese hi bits int16_t meh jate toh woh int meh badal jate hai 
  // phir agar msb 0 toh +ve hai 
  // jabh msb 1 toh value -ve hai toh 2 s complement karta hai auto maticly cpp does it 

  rateRoll = (float)gyroX/ 65.5 ;
  ratePitch  = (float)gyroY/ 65.5 ;   //. 
  rateYaw   = (float)gyroZ/ 65.5 ;  // lsb sensivity scale factor is set 65.5 so divide to it 

}

void setup(){

  Serial.begin(115200);

  Wire.setClock(400000); // i2c speed set kiya hai 400khz as data sheet meh likha tha comms speed of the sensor 
  Wire.begin();
  delay(250);

  Wire.beginTransmission(0x68);  //. sensor ka address likha haia
  Wire.write(0x6B);      // 
  Wire.write(0x00);      // jab tak sare sensor ko 0 nahi karte woh chalu nahi hoga
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
  gyro_read();
  Serial.print(" ROll Rate [°/s]= ");
  Serial.print(rateRoll - calibration_rateRoll);
  Serial.print(" Pitch rate [°/s]= ");
  Serial.print(ratePitch - calibration_ratePitch);
  Serial.print(" Yaw Rate [°/s]= ");
  Serial.println(rateYaw - calibration_rateYaw);
  delay(50);

}