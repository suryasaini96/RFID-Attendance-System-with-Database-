#include <SoftwareSerial.h>
SoftwareSerial ArduinoUno(A4,A5); //RX,TX

void setup(){
  
  Serial.begin(9600);
  ArduinoUno.begin(4800);

}

void loop(){
  
  while(ArduinoUno.available()>0){
  float val = ArduinoUno.parseFloat();
  if(ArduinoUno.read()== '\n'){
  Serial.println(val);
  }
}
delay(30);
}
