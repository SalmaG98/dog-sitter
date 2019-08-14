

#include <ArduinoJson.h>
char Collar_History[1024];

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  while (!Serial) continue;

StaticJsonDocument<256> doc;
JsonObject collarHistory = doc.to<JsonObject>();
collarHistory["date"] = random(6000000);

JsonObject position = collarHistory.createNestedObject("position");
position["x"] = random(1000,100000)/100.0;
position["y"] = random(1000,100000)/100.0;

collarHistory["pulse"] = random(150);
collarHistory["temperature"] = random(55);
serializeJsonPretty(collarHistory, Collar_History);
Serial.println(Collar_History);
}
void loop() {
  // not used in this example
}
