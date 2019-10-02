#include <ArduinoJson.h>

StaticJsonDocument<150> doc;
//JsonObject Sensors = doc.to<JsonObject>();
JsonArray Agitation = doc.createNestedArray("Agitation");


void setup() {
 Serial.begin(9600);
 //Serial.println("Interfacfing arduino with nodemcu");
 
}

void loop() {
  StaticJsonDocument<150> doc;
  //JsonObject Sensors = doc.to<JsonObject>();
  JsonArray Agitation = doc.createNestedArray("Agitation");
//  Serial.println("Sending data to nodemcu");
//  Serial.print("{\"pulse\":");
//  Serial.print(140);//sensor id
//  Serial.print(",");
//  Serial.print("\"temperature\":");
//  Serial.print(3.402823466e+38);//sensor id
//  Serial.print(",");
//  Serial.print("\"Agitation\":");
//  Serial.print("{\"Ax\":");
//  Serial.print(32,767);//sensor id
//  Serial.print(",");
//  Serial.print("\"Ay\":");
//  Serial.print(32,767);//sensor id
//  Serial.print(",");
//  Serial.print("\"Az\":");
//  Serial.print(32,767);//sensor id
//  Serial.print(",");
//  Serial.print("\"Gx\":");
//  Serial.print(32,767);//sensor id
//  Serial.print(",");
//  Serial.print("\"Gy\":");
//  Serial.print(32,767);//sensor id
//  Serial.print(",");
//  Serial.print("\"Gz\":");
//  Serial.print(32,767);//sensor id
//  Serial.print("}");
//  Serial.print("}");
//  Serial.println();
  doc["pulse"] = 140;
  doc["temperature"] = 70;
  Agitation.add(32767);
  Agitation.add(32766);
  Agitation.add(32765);
  Agitation.add(32764);
  Agitation.add(32763);
  Agitation.add(32762);
  serializeJson(doc,Serial);
  Serial.println();


 delay(1000);
}
