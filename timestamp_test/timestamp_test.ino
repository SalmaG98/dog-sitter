#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "ODCGuest";
const char* password =  "T5L4zPovSG";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  timeClient.update();

  Serial.println(timeClient.getEpochTime());

  delay(1000);
}
