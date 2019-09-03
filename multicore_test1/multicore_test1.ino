
//LIBRARIES
#include <NTPClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


//VARIABLES
//UDP_NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String Collar_History;


//Connecting_WIFI_MQTT

const char* ssid = "ODCGuest";
const char* password =  "T5L4zPovSG";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientID = "24727054";
const char* channelName = "oprex/collarHistory";

WiFiClient MQTTclient;
PubSubClient client(MQTTclient);

//task

TaskHandle_t Sending_Data_task;
boolean once = true;

//JSON
StaticJsonDocument<256> doc;
JsonObject collarHistory = doc.to<JsonObject>();
JsonObject position = collarHistory.createNestedObject("position");
char buffer[512];

//BPM

#define pulsePin 36


// these variables are volatile because they are used during the interrupt service routine!
 int BPM;                   // used to hold the pulse rate
 int Signal;                // holds the incoming raw data
 int IBI = 0;             // holds the time between beats, must be seeded! 
 boolean Pulse = false;     // true when pulse wave is high, false when it's low
 boolean QS = false;        // becomes true when Arduoino finds a beat.

 int rate[10];                    // array to hold last ten IBI values
 unsigned long sampleCounter = 0; // used to determine pulse timing
 unsigned long lastBeatTime = 0;  // used to find IBI
 int P =512;                      // used to find peak in pulse wave, seeded
 int T = 512;                     // used to find trough in pulse wave, seeded
 int thresh = 512;                // used to find instant moment of heart beat, seeded
 int amp = 100;                   // used to hold amplitude of pulse waveform, seeded
 boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
 boolean secondBeat = false;      // used to seed rate array so we startup with reasonable BPM
 int rx_Signal;
 portBASE_TYPE xHigherPriorityTaskWoken;

//ISR

hw_timer_t * timer = NULL;
portMUX_TYPE timertocheckpulseMux = portMUX_INITIALIZER_UNLOCKED;


void callback(char* topic, byte* payload, unsigned int length) {
  String payload_buff;
  for (int i = 0; i < length; i++) {
    payload_buff = payload_buff + String((char)payload[i]);
  }
  //Serial.println(payload_buff); // Print out messages.
}


long lastReconnectAttempt = 0;
boolean reconnect() {
  if (client.connect(clientID)) {
    //client.subscribe(channelName); // Subscribe to channel.
  }
  return client.connected();
}

void IRAM_ATTR getPulse(int Signal)
  {
    //Signal = analogRead(pulsePin);             // read the Pulse Sensor, bits of ESP32 ADC ch is 4 times larger
    Signal = analogRead(pulsePin);                    // read the Pulse Sensor on pin 34 3.3v sensor power......default ADC setup........
  Signal = map(Signal, 0, 4095, 0, 1023);     // Map the value back to original sketch range......
  sampleCounter += 2;                         // keep track of the time in mS with this variable
  int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
  if((Signal < thresh) && (N > (IBI/5)*3)){       // avoid dichrotic noise by waiting 3/5 of last IBI
    if (Signal < T){                        // T is the trough
      T = Signal;// keep track of lowest point in pulse wave 
      //Serial.println("T = Signal");
    }
      //Serial.println("(Signal < thresh) && (N > (IBI/5)*3)");
  }

  if((Signal > thresh) && (Signal > P)){          // thresh condition helps avoid noise
    P = Signal;  // P is the peak
    //Serial.println("P = Signal");
  }                                        // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N > 250){                                   // avoid high frequency noise
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
      Pulse = true;                               // set the Pulse flag when we think there is a pulse
      IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
      lastBeatTime = sampleCounter;               // keep track of time for next pulse
      

      if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
        secondBeat = false;                  // clear secondBeat flag
        for(int i=0; i<=9; i++){             // seed the running total to get a realisitic BPM at startup
          rate[i] = IBI;                      
        }
      }

      if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
        firstBeat = false;                   // clear firstBeat flag
        secondBeat = true;                   // set the second beat flag
        sei();                               // enable interrupts again
        return;                              // IBI value is unreliable so discard it
      }   


      // keep a running total of the last 10 IBI values
      word runningTotal = 0;                  // clear the runningTotal variable    

      for(int i=0; i<=8; i++){                // shift data in the rate array
        rate[i] = rate[i+1];                  // and drop the oldest IBI value 
        runningTotal += rate[i];              // add up the 9 oldest IBI values
      }

      rate[9] = IBI;                          // add the latest IBI to the rate array
      runningTotal += rate[9];                // add the latest IBI to runningTotal
      runningTotal /= 10;                     // average the last 10 IBI values 
      BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
      QS = true;                              // set Quantified Self flag 
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
         
    } 
                       
  }

  if (Signal < thresh && Pulse == true){   // when the values are going down, the beat is over
    Pulse = false;                         // reset the Pulse flag so we can do it again
    amp = P - T;                           // get amplitude of the pulse wave
    thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
    P = thresh;                            // reset these for next time
    T = thresh;
  }

  if (N > 2500){                           // if 2.5 seconds go by without a beat
    thresh = 512;                          // set thresh default
    P = 512;                               // set P default
    T = 512;                               // set T default
    lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
    firstBeat = true;                      // set these to avoid noise
    secondBeat = false;                    // when we get the heartbeat back
  }
  }



void Sending_Data(void * pvParameters) 
  {
    while(true)
      {
        if (!client.connected()) 
          {
          long now = millis();
          if (now - lastReconnectAttempt > 5000) 
            { 
              lastReconnectAttempt = now;
              if (reconnect()) 
                { 
                  lastReconnectAttempt = 0;
                }
            }
          } 
        else 
          { // Connected.

          
            timeClient.update();

            //JSON

            collarHistory["date"] = timeClient.getEpochTime();


            position["x"] = random(1000, 100000) / 100.0;
            position["y"] = random(1000, 100000) / 100.0;

            collarHistory["pulse"] = BPM;
            collarHistory["temperature"] = random(55);

            client.loop();

            serializeJson(doc, buffer);
            client.publish(channelName, buffer); // Publish message.
            delay(1000);
          }  
      }
  }


void IRAM_ATTR ISRTr()
  {       
    portENTER_CRITICAL_ISR(&timertocheckpulseMux);
    Signal=analogRead(pulsePin);
    getPulse(Signal);
    portEXIT_CRITICAL_ISR(&timertocheckpulseMux);
    sei();
  }


void IRAM_ATTR interruptSetup()
  {     
    // Use 1st timer of 4 (counted from zero).
    // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
    // info).
    timer = timerBegin(0, 80, true);
    
    // Initializes Timer to run the ISR to sample every 2mS as per original Sketch.
    // Attach ISRTr function to our timer.
    timerAttachInterrupt(timer, &ISRTr, true);


    // Set alarm to call isr function every 2 milliseconds (value in microseconds).
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer, 2000, true);

    // Start an alarm
    timerAlarmEnable(timer);
    sei();
  } 






void setup() 
  {
      
    pinMode(pulsePin,INPUT_PULLDOWN);
    Serial.begin(9600);
    WiFi.begin(ssid, password); // Connect to WiFi.
      if (WiFi.waitForConnectResult() != WL_CONNECTED) 
        {
          Serial.println("Couldn't connect to WiFi.");
          while (1) delay(100);
        }
      else
        {
          Serial.print("Connected to : ");
          Serial.println(ssid);
        }
    timeClient.begin();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    lastReconnectAttempt = 0; 
    xTaskCreatePinnedToCore(Sending_Data,"Data_Sending",2500,NULL,1,&Sending_Data_task,1);
    delay(500); 
    interruptSetup(); 
  }
    

void loop() 
  {
    
    if (QS == true)
      {                       
        Serial.println("BPM: "+ String(BPM));   // send heart rate with a 'B' prefix
        QS = false;  
      }
  }
