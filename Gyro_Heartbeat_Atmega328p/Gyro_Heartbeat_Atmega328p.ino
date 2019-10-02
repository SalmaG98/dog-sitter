uint64_t current_time;

//temp

int input = A0; 
int reading ; 
float temp=0 ; 

//GYRO

#include <Wire.h> //Include the Wire.h library so we can communicate with the gyro
int gyro_address;
int acc_axis[4], gyro_axis[4];
double gyro_axis_cal[4];
int temperature;

int ax,ay,az,gx,gy,gz;

//BPM

int pulsePin = A1;           // pin to blink led at each beat

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
volatile int rate[10];                      // array to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find IBI
volatile int P = 512;                      // used to find peak in pulse wave, seeded
volatile int T = 512;                     // used to find trough in pulse wave, seeded
volatile int thresh = 525;                // used to find instant moment of heart beat, seeded
volatile int amp = 100;                   // used to hold amplitude of pulse waveform, seeded
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = false;      // used to seed rate array so we startup with reasonable BPM
volatile int BPMresult;


void setup(){
  
  Wire.begin();                                      //Start the I2C as master
  current_time = millis();
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
    pinMode(pulsePin,INPUT);         // pin that will blink to your heartbeat!
    interruptSetup();
    pinMode(input , INPUT) ; 
  
}
//Main program
void loop(){

  StaticJsonDocument<150> doc;
  //JsonObject Sensors = doc.to<JsonObject>();
  JsonArray Agitation = doc.createNestedArray("Agitation");

      gyro_signalen();   //Read the gyro signals
      //Your offsets:  -638  -61 1713  -159  -18 48
      ax = acc_axis[1];//+638;
      ay = acc_axis[2];//+61;
      az = acc_axis[3]-4090;
      gx = gyro_axis[1];//+159;
      gy = gyro_axis[2];//+18;
      gz = gyro_axis[3];//-48;

      reading  = analogRead(input);
      temp = (reading * (5.0/1024))*100 - 2.5 ; 

      doc["P"] = BPM;
      doc["T"] = temp;
      Agitation.add(ax);
      Agitation.add(ay);
      Agitation.add(az);
      Agitation.add(gx);
      Agitation.add(gy);
      Agitation.add(gz);
      serializeJson(doc,Serial);
      Serial.println();
    
    
     delay(1000);
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

void interruptSetup()
{
  // Initializes Timer2 to throw an interrupt every 2mS.
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
}


ISR(TIMER2_COMPA_vect) //triggered when Timer2 counts to 124
{
  cli();                                      // disable interrupts while we do this
  Signal = analogRead(pulsePin);              // read the Pulse Sensor
  sampleCounter += 2;                         // keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise
                                              //  find the peak and trough of the pulse wave
  if(Signal < thresh && N > (IBI/5)*3) // avoid dichrotic noise by waiting 3/5 of last IBI
    {
      if (Signal < T) // T is the trough
      {
        T = Signal; // keep track of lowest point in pulse wave
      }
    }

  if(Signal > thresh && Signal > P)
    {          // thresh condition helps avoid noise
      P = Signal;                             // P is the peak
    }                                        // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250)
  {                                   // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) )
      {
        Pulse = true;              // turn on pin 13 LED
        IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
        lastBeatTime = sampleCounter;               // keep track of time for next pulse

        if(secondBeat)
        {                        // if this is the second beat, if secondBeat == TRUE
          secondBeat = false;                  // clear secondBeat flag
          for(int i=0; i<=9; i++) // seed the running total to get a realisitic BPM at startup
          {
            rate[i] = IBI;
          }
        }

        if(firstBeat) // if it's the first time we found a beat, if firstBeat == TRUE
        {
          firstBeat = false;                   // clear firstBeat flag
          secondBeat = true;                   // set the second beat flag
          sei();                               // enable interrupts again
          return;                              // IBI value is unreliable so discard it
        }
      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                  // clear the runningTotal variable

      for(int i=0; i<=8; i++)
        {                // shift data in the rate array
          rate[i] = rate[i+1];                  // and drop the oldest IBI value
          runningTotal += rate[i];              // add up the 9 oldest IBI values
        }

      rate[9] = IBI;                          // add the latest IBI to the rate array
      runningTotal += rate[9];                // add the latest IBI to runningTotal
      runningTotal /= 10;                     // average the last 10 IBI values
      BPMresult = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
      if((BPMresult > 59) && (BPMresult < 141))
        BPM=BPMresult;
      QS = true;                              // set Quantified Self flag
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }
  }

  if (Signal < thresh && Pulse == true)
    { 
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
    }

  if (N > 2500)
    {                           // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
      firstBeat = true;                      // set these to avoid noise
      secondBeat = false;                    // when we get the heartbeat back
    }

  sei();                                   // enable interrupts when youre done!
}
