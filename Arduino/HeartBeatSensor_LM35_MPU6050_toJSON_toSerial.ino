#include <ArduinoJson.h>
DynamicJsonDocument doc(1024);

/* ============================================
 *  MPU6050
 * ===============================================
*/

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#include "Wire.h"
#include "SPI.h"

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int8_t threshold, count;
float temp;
bool zero_detect;
bool TurnOnZI = false;

bool XnegMD, XposMD, YnegMD, YposMD, ZnegMD, ZposMD;

bool blinkState = false;
/*  Getting_BPM_to_Monitor prints the BPM to the Serial Monitor, using the least lines of code and PulseSensor Library.
    Tutorial Webpage: https://pulsesensor.com/pages/getting-advanced

  --------Use This Sketch To------------------------------------------
  1) Displays user's live and changing BPM, Beats Per Minute, in Arduino's native Serial Monitor.
  2) Print: "♥  A HeartBeat Happened !" when a beat is detected, live.
  2) Learn about using a PulseSensor Library "Object".
  --------------------------------------------------------------------*/

#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   

//  Variables
const int PulseWire = 0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
//const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
// Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
// Otherwise leave the default "550" value.
int myBPM ;

PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

void pulseSensorSetup(){
    // Configure the PulseSensor object, by assigning our variables to it.
    pulseSensor.analogInput(PulseWire);
    //pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
    pulseSensor.setThreshold(Threshold);

    // Double-check the "pulseSensor" object was created and "began" seeing a signal.
    if (pulseSensor.begin()) {
      Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.
    }
  }

void mpuSetup(){
    // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();

  // initialize serial communication
  // (38400 chosen because it works as well at 8MHz as it does at 16MHz, but
  // it's really up to you depending on your project)
  //Serial.begin(38400);

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  Serial.println("Setting motion detection threshold to 2...");
  accelgyro.setMotionDetectionThreshold(2);

  Serial.println("Setting motion detection duration to 40...");
  accelgyro.setMotionDetectionDuration(40);
  }  
void setup() {
  Serial.begin(9600);          // initialize serial communication
  // put your setup code here, to run once:
  pulseSensorSetup();
}
void pulseSensorLoop(){
    myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
    // "myBPM" hold this BPM value now.

    if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened".
      //Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
      //Serial.print("BPM: ");                        // Print phrase "BPM: "
      //Serial.println(myBPM);                        // Print the value inside of myBPM.
    }
    //delay(20);                    // considered best practice in a simple sketch.
  }

void mpuLoop(){
    // read raw accel/gyro measurements from device
    
    //Serial.println("Getting raw accwl/gyro measurements");
    //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    //Serial.print(ax / 16384.); Serial.print(",");
    //Serial.print(ay / 16384.); Serial.print(",");
    //Serial.print(az / 16384.); Serial.print(",");
    //Serial.print(gx / 131.072); Serial.print(",");
    //Serial.print(gy / 131.072); Serial.print(",");
    //Serial.print(gz / 131.072); Serial.print(",");
    
  }  
void lm35(){};
void sendJSON(){};

void loop() {
  // put your main code here, to run repeatedly:
  pulseSensorLoop();

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  doc["temp"] = 37;
  doc["heartBeat"]   = myBPM ;
  
  JsonArray mpu6050 = doc.createNestedArray("mpu6050");
  mpu6050.add(ax / 16384.); // ax
  mpu6050.add(ay / 16384.); // ay
  mpu6050.add(az / 16384.); // az
  mpu6050.add(gx / 131.072); // gx
  mpu6050.add(gy / 131.072); // gy
  mpu6050.add(gz / 131.072); // gz
  
  serializeJson(doc, Serial);
  // This prints:
  // {"temp":"gps","heartbeat":1351824120,"mpu6050":[48.756080,2.302038]}
  
}
