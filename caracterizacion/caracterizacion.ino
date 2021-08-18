#include <ESP8266WiFi.h>
#include <Wire.h>
int lightValue;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  lightValue = analogRead(0);
  Serial.println(lightValue);
  delay(200);
}
