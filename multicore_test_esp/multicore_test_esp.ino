
//LIBRARIES
#include <NTPClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


//VARIABLES

//Audio
#define MICROPHONE_PIN 36
#define AUDIO_BUFFER_MAX 8192

int audioStartIdx = 0, audioEndIdx = 0;
uint16_t audioBuffer[AUDIO_BUFFER_MAX];
uint16_t txBuffer[AUDIO_BUFFER_MAX];

// version without timers
unsigned long lastRead = micros();

//UDP_NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Connecting_WIFI_MQTT
const char* ssid     = "Orange-2";
const char* password = "22223333";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientID = "24727054";
const char* channelName = "oprex/collarHistory";

WiFiClient audioClient;
WiFiClient checkClient;
WiFiServer audioServer = WiFiServer(3443);

WiFiClient MQTTclient;
PubSubClient client(MQTTclient);


//Tasks
TaskHandle_t Sending_Data_task;
TaskHandle_t Sending_GPS_task;


//JSON
String Collar_History;
StaticJsonDocument<500> doc;
char buffer[700];
int ax=0,ay=0,az=0,gx=0,gy=0,gz=0;
int BPM=0,temp=0;
unsigned long chipid;  

StaticJsonDocument<100> state;
float battery;


void callback(char* topic, byte* payload, unsigned int length) 
  {
    String payload_buff;
    for (int i = 0; i < length; i++) payload_buff = payload_buff + String((char)payload[i]);
  }


long lastReconnectAttempt = 0;
boolean reconnect() 
  {
    if (client.connect(clientID)) 
      {
        //client.subscribe(channelName); // Subscribe to channel.
      }
    return client.connected();
  }



void Sending_Data(void *pvParameters) 
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

            client.loop();
   
           DeserializationError err = deserializeJson(doc,Serial);
           if (!err) 
             {
               
               doc["ID"]= chipid;
               doc["D"]=timeClient.getEpochTime();
               
               //serializeJson(doc,Serial);
               //Serial.println();
               serializeJson(doc,buffer);
               client.publish("oprex/collarHistory",buffer);
                            
             }
           state["rssi"]=WiFi.RSSI();
           state["battery"]=100;
           state["date"]=timeClient.getEpochTime();
           state["id"]= chipid;
           serializeJson(state,buffer);
           client.publish("oprex/collarState",buffer);
           delay(1000);
             
              
      }
    }
  }

  
//void Sending_GPS(void *pvParameters)
//{
//  Serial.println("GPS");
//}



void setup() 
  {
      
    Serial.begin(4800);
    pinMode(MICROPHONE_PIN, INPUT);
    WiFi.begin(ssid, password); // Connect to WiFi.
      if (WiFi.waitForConnectResult() != WL_CONNECTED) 
        {
          Serial.println("Couldn't connect to WiFi.");
        }
      else
        {
          Serial.print("Connected to : ");
          Serial.println(ssid);
          Serial.println("MY IP address: ");
          Serial.println(WiFi.localIP());
        }
    audioServer.begin();
    timeClient.begin();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    lastReconnectAttempt = 0; 
    lastRead = micros();
    xTaskCreatePinnedToCore(Sending_Data,"Data_Sending",5000,NULL,1,&Sending_Data_task,0);
    delay(500); 
//    xTaskCreatePinnedToCore(Sending_GPS,"GPS_Sending",2500,NULL,1,&Sending_GPS_task,0);
//    delay(500);
//    vTaskSuspend(Sending_GPS_task);
    chipid=ESP.getEfuseMac();
  }
    

void loop() 
  {

      
  //    if(WiFi.status() == WL_CONNECTED)
  //    {
      //vTaskResume(Sending_Data_task);
      checkClient = audioServer.available();
      if (checkClient.connected()) 
        {
          audioClient = checkClient; 
        }
      
      //listen for 100ms, taking a sample every 125us,
      //and then send that chunk over the network.
      listenAndSend(100);
  //    }
  //    else
  //    {
  //      vTaskSuspend(Sending_Data_task);
  //      vTaskResume(Sending_GPS_task);
  //    }
  }


void listenAndSend(int delay) 
  {
      unsigned long startedListening = millis();
      
      while ((millis() - startedListening) < delay) 
        {
          unsigned long time = micros();
          
          if (lastRead > time) 
            {
              // time wrapped?
              //lets just skip a beat for now, whatever.
              lastRead = time;
            }
          
          //125 microseconds is 1/8000th of a second
          if ((time - lastRead) > 127) 
            {
              lastRead = time;
              readMic();
            }
        }

      if (audioBuffer[0]>2700)
        {     //barking sound threshold
          sendAudio();
        }
  }

 
// Callback for Timer 1
void readMic(void) 
  {
    uint16_t value = analogRead(MICROPHONE_PIN);
    if (audioEndIdx >= AUDIO_BUFFER_MAX) 
      {
        audioEndIdx = 0;
      }
    audioBuffer[audioEndIdx++] = value;
  }

void copyAudio(uint16_t *bufferPtr) 
  {
    //if end is after start, read from start->end
    //if end is before start, then we wrapped, read from start->max, 0->end
    
    int endSnapshotIdx = audioEndIdx;
    bool wrapped = endSnapshotIdx < audioStartIdx;
    int endIdx = (wrapped) ? AUDIO_BUFFER_MAX : endSnapshotIdx;
    int c = 0;
    
    for(int i=audioStartIdx;i<endIdx;i++) 
      {
        // do a thing
        bufferPtr[c++] = audioBuffer[i];
      }
    
    if (wrapped) 
      {
        //we have extra
        for(int i=0;i<endSnapshotIdx;i++)
        {
          // do more of a thing.
          bufferPtr[c++] = audioBuffer[i];
        }
      }
    
    //and we're done.
    audioStartIdx = audioEndIdx;
    
    if (c < AUDIO_BUFFER_MAX) 
      {
        bufferPtr[c] = -1;
      }
  }

// Callback for Timer 1
void sendAudio(void) 
  {
    copyAudio(txBuffer);
    
    int i=0;
    uint16_t val = 0;
        
    if (audioClient.connected()) 
      {
        write_socket(audioClient, txBuffer);
      }
    // else 
    //   {
    //       while( (val = txBuffer[i++]) < 65535 ) 
    //         {
    // //        Serial.print(val);
    // //        Serial.print(',');
    //         }
    //     //Serial.println("DONE");
    //   }
  }


// an audio sample is 16bit, we need to convert it to bytes for sending over the network
void write_socket(WiFiClient socket, uint16_t *buffer)
  {
    int i=0;
    uint16_t val = 0;
    
    int tcpIdx = 0;
    uint8_t tcpBuffer[1024];
    
    while( (val = buffer[i++]) < 65535 ) 
      {
        if ((tcpIdx+1) >= 1024) 
        {

            socket.write(tcpBuffer, tcpIdx);
            tcpIdx = 0;
        }
        
        tcpBuffer[tcpIdx] = val & 0xff;
        tcpBuffer[tcpIdx+1] = (val >> 8);
        tcpIdx += 2;
      }
    
    // any leftovers?
    if (tcpIdx > 0) 
      {
        socket.write(tcpBuffer, tcpIdx);
      }
  }
