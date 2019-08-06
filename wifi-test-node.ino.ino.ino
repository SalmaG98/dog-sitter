// PubNub MQTT example using ESP32.
#include <WiFi.h>
#include <PubSubClient.h>
// Connection info.
const char* ssid = "ODCGuest";
const char* password =  "T5L4zPovSG";
const char* mqttServer = "https://admin.pubnub.com";
const int mqttPort = 1883;
const char* clientID = "pub-c-08975a33-7477-474e-9d96-f4d9e12ff0ec/sub-c-ed5cf76c-b78b-11e9-9643-ee2376cc7690/CLIENT_ID";
const char* channelName = "hello_world";
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
    client.publish(channelName,"received"); // Publish message.
    delay(1000);
  }
}
