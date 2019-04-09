#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial NodeMCU(D0,D4); //RX,TX

void setup(){
  Serial.begin(9600);
  NodeMCU.begin(9600);
  pinMode(D0,INPUT); // RX as INPUT
  pinMode(D4,OUTPUT); // TX as OUTPUT
}

void loop(){
  char* data = "10,50,100."; 
  NodeMCU.write(data);
  Serial.println(data);
  delay(2000);
}
