#include <Servo.h> 
 
Servo servo; // create servo object to control a servo 
// a maximum of eight servo objects can be created 
 
//int pos = 0; // variable to store the servo position
int angle=0; 
 
void setup() {
  servo.attach(9);
  servo.write(angle);
}
 
void loop() 
{ 
 // scan from 0 to 180 degrees
  for(angle = 10; angle < 180; angle++)  
  {                                  
    servo.write(angle);               
    delay(15);                   
  } 
  // now scan back from 180 to 0 degrees
  for(angle = 180; angle > 10; angle--)    
  {                                
    servo.write(angle);           
    delay(15);       
  } 
}
