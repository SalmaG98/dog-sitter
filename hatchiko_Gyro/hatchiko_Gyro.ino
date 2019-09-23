#include <Wire.h> //Include the Wire.h library so we can communicate with the gyro
int gyro_address;
int acc_axis[4], gyro_axis[4];
double gyro_axis_cal[4];
int temperature;

int ax,ay,az,gx,gy,gz;

byte highByte, lowByte;

void setup(){
  
  Wire.begin();                                      //Start the I2C as master
  uint64_t current_time = millis();
  Serial.begin(9600);                                //Start the serial connetion @ 9600bps
  gyro_address = 0x68;                                                                  //the MPU6050 gyro address is 0x68.

    //The gyro is disabled by default and needs to be started
    Wire.beginTransmission(gyro_address);                        //Start communication with the address found during search.
    Wire.write(0x6B);                                            //We want to write to the PWR_MGMT_1 register (6B hex)
    Wire.write(0x00);                                            //Set the register bits as 00000000 to activate the gyro
    Wire.endTransmission();                                      //End the transmission with the gyro.

    Wire.beginTransmission(gyro_address);                        //Start communication with the address found during search.
    Wire.write(0x1B);                                            //We want to write to the GYRO_CONFIG register (1B hex)
    Wire.write(0x08);                                            //Set the register bits as 00001000 (500dps full scale)
    Wire.endTransmission();                                      //End the transmission with the gyro

    Wire.beginTransmission(gyro_address);                        //Start communication with the address found during search.
    Wire.write(0x1C);                                            //We want to write to the ACCEL_CONFIG register (1A hex)
    Wire.write(0x10);                                            //Set the register bits as 00010000 (+/- 8g full scale range)
    Wire.endTransmission();                                      //End the transmission with the gyro

  delay(250);                                        //Give the gyro time to start
  
  
}
//Main program
void loop(){
  delay(250);                                        //Wait 250 microseconds for every loop  
  gyro_signalen();   //Read the gyro signals
  //Your offsets:  -638  -61 1713  -159  -18 48
    ax = acc_axis[1];//+638;
    ay = acc_axis[2];//+61;
    az = acc_axis[3]-4090;
    gx = gyro_axis[1];//+159;
    gy = gyro_axis[2];//+18;
    gz = gyro_axis[3];//-48;
    Serial.print(ax);
    Serial.print(',');
    Serial.print(ay);
    Serial.print(',');
    Serial.print(az);
    Serial.print(',');
    Serial.print(gx);
    Serial.print(',');
    Serial.print(gy);
    Serial.print(',');
    Serial.println(gz);
    
}

void gyro_signalen(){
  //Read the MPU-6050
    Wire.beginTransmission(gyro_address);                        //Start communication with the gyro.
    Wire.write(0x3B);                                            //Start reading @ register 43h and auto increment with every read.
    Wire.endTransmission();                                      //End the transmission.
    Wire.requestFrom(gyro_address,14);                           //Request 14 bytes from the gyro.
    while(Wire.available() < 14);                                //Wait until the 14 bytes are received.
    acc_axis[1] = Wire.read()<<8|Wire.read();                    //Add the low and high byte to the acc_x variable.
    acc_axis[2] = Wire.read()<<8|Wire.read();                    //Add the low and high byte to the acc_y variable.
    acc_axis[3] = Wire.read()<<8|Wire.read();                    //Add the low and high byte to the acc_z variable.
    temperature = Wire.read()<<8|Wire.read();                    //Add the low and high byte to the temperature variable.
    gyro_axis[1] = Wire.read()<<8|Wire.read();                   //Read high and low part of the angular data.
    gyro_axis[2] = Wire.read()<<8|Wire.read();                   //Read high and low part of the angular data.
    gyro_axis[3] = Wire.read()<<8|Wire.read();                   //Read high and low part of the angular data.
             
}
