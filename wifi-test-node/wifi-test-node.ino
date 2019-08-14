


// PubNub MQTT example using ESP32.
#include <WiFi.h>
#include <PubSubClient.h>
// Connection info.
const char* ssid = "ODCGuest";
const char* password =  "T5L4zPovSG";
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* clientID = "24727054";
const char* channelName = "oprex/collarHistory";
const char* channelName1 = "atos";

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
    //client.subscribe(channelName1); // Subscribe to channel.
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
    client.loop();
    client.publish(channelName,"salma to amine"); // Publish message.
    //client.subscribe(channelName); // Publish message.
    delay(1000);
  }
}
