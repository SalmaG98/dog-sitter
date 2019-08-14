
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

//Connecting

const char* ssid = "ODCGuest";
const char* password =  "T5L4zPovSG";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientID = "24727054";
const char* channelName = "oprex/collarHistory";
//const char* channelName1 = "atos";

StaticJsonDocument<256> doc;
JsonObject collarHistory = doc.to<JsonObject>();
JsonObject position = collarHistory.createNestedObject("position");
char buffer[512];

WiFiClient MQTTclient;
PubSubClient client(MQTTclient);

void callback(char* topic, byte* payload, unsigned int length) {
  String payload_buff;
  for (int i=0;i<length;i++) {
    payload_buff = payload_buff+String((char)payload[i]);
  }
  Serial.println(payload_buff); // Print out messages.
}
long lastReconnectAttempt = 0;
boolean reconnect() {
  if (client.connect(clientID)) {
   client.subscribe(channelName); // Subscribe to channel.
  }
  return client.connected();
}
void setup() {
  Serial.begin(9600);
  Serial.println("Attempting to connect...");
  WiFi.begin(ssid, password); // Connect to WiFi.
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Couldn't connect to WiFi.");
      while(1) delay(100);
  }
  else
  {
    Serial.print("Connected to : ");
    Serial.println(ssid);
  }
  timeClient.begin();
  client.setServer(mqttServer, mqttPort); // Connect to PubNub.
  client.setCallback(callback);
  lastReconnectAttempt = 0;
}
void loop() {
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect.
      lastReconnectAttempt = now;
      if (reconnect()) { // Attempt to reconnect.
        lastReconnectAttempt = 0;
      }
    }
  } else { // Connected.

    
    timeClient.update();
    //JSON
    
    collarHistory["date"] = timeClient.getEpochTime();

    
    position["x"] = random(1000,100000)/100.0;
    position["y"] = random(1000,100000)/100.0;

    collarHistory["pulse"] = random(150);
    collarHistory["temperature"] = random(55);
    serializeJsonPretty(collarHistory, Collar_History);
 

    client.loop();
    //Serial.println(buf);
    
    serializeJson(doc, buffer);
    client.publish(channelName,buffer); // Publish message.
    delay(1000);
  }
}
