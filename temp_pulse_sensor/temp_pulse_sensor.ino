#include <OneWire.h>
#include <DallasTemperature.h>

#define tempPin 25
#define pulsePin 39

OneWire oneWire(tempPin);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  pinMode(tempPin,INPUT);
  pinMode(pulsePin,INPUT);

  sensors.begin();
}

void loop() {

  sensors.requestTemperatures();

  Serial.println(sensors.getTempCByIndex(0));
  //Serial.println(sensors.getTempFByIndex(0));
  delay(1000);
}
