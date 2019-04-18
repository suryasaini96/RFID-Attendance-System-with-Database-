#include <SoftwareSerial.h>
#include <String.h> // we'll need this for subString
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Servo.h> 

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 2
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

#define MAX_STRING_LEN 50 // like 3 lines above, change as needed.

/*Servo servo;
int angle=0;*/
SoftwareSerial SoftSer(A4, A5); // RX, TX

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 50;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 1000; // in milliseconds

const char EOPmarker = '.'; //This is the end of packet marker
char serialbuf[50]; //This gives the incoming serial some room. Change it if you want a longer incoming.

// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  50
char curMessage[BUF_SIZE] = {"WELCOME"};
char dotMatrixData[BUF_SIZE] = {""};
int count = 0;

void setup(){
  Serial.begin(9600);
  SoftSer.begin(9600);
  //servo.attach(A1);
  //servo.write(angle);
  pinMode(A0,OUTPUT);
  
  P.begin();
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void loop() {
      if (SoftSer.available() > 0) {
        //makes sure something is ready to be read
        static int bufpos = 0; //starts the buffer back at the first position in the incoming serial.read
        char inchar = SoftSer.read(); //assigns one byte (as serial.read()'s only input one byte at a time
        //delay(100); //Wait for data to completely receive 
        Serial.print(inchar);
        if (inchar != EOPmarker) { //if the incoming character is not the byte that is the incoming package ender
          serialbuf[bufpos] = inchar; //the buffer position in the array get assigned to the current read
          bufpos++; //once that has happend the buffer advances, doing this over and over again until the end of package marker is read.
        }
        else { //once the end of package marker has been read
          serialbuf[bufpos] = 0; //restart the buff
          bufpos = 0; //restart the position of the buff
          String cardLastDigits  = subStr(serialbuf, ",", 1);
          String payload = subStr(serialbuf, ",", 2);
          String currentTime = subStr(serialbuf, ",", 3);
          String currentDate = subStr(serialbuf, ",", 4);
          String dotMatrixPayload;
          Serial.println();
          Serial.print("The card last 3 digits are: ");
          Serial.print(cardLastDigits);
          Serial.print(" - Payload: ");
          Serial.print(payload);
          Serial.print(" - Time: ");
          Serial.print(currentTime);
          Serial.print(" - Date: ");
          Serial.println(currentDate);
          Serial.println();
   
          if(payload == "login"){
            dotMatrixPayload = "Login";
          }
          else if(payload == "logout"){
            dotMatrixPayload = "Logout";
          }
          else if(payload == "successful" || payload == "Cardavailable1"){
            dotMatrixPayload = "Card Available";
          }
          else if(payload == "NotAllow"){
            dotMatrixPayload = "Not allowed";
          }
          strcat(dotMatrixData, (cardLastDigits + " " + dotMatrixPayload + " " + currentTime + " " + currentDate).c_str());
          strcpy(curMessage,dotMatrixData);
          P.displayClear();
          P.displayReset();
          digitalWrite(A0, HIGH);   // turn the LED on (HIGH is the voltage level)
          delay(100);                       // wait for a second
          digitalWrite(A0, LOW);    // turn the LED off by making the voltage LOW
          //delay(1000);
          Serial.print("Message: ");
          Serial.println(curMessage);
          count=0;
          Serial.println();
          strcpy(dotMatrixData, ""); //Clear the dot matrix data buffer
          /*for(angle = 10; angle < 180; angle++)  
          {                                  
            servo.write(angle);               
            delay(15);                   
          } 
          // now scan back from 180 to 0 degrees
          for(angle = 180; angle > 10; angle--)    
          {                                
            servo.write(angle);           
            delay(15);       
          }*/  
        }
    } 
   if (P.displayAnimate())
   {
      
      P.displayReset();
      count++;
      if ( count == 1 ){
        if (strcmp(curMessage, "WELCOME")!=0){
          strcpy(curMessage,"WELCOME");
        }
        count = 0;
      }
    }
}

char* subStr (char* input_string, char *separator, int segment_number) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;
  strcpy(copy, input_string);
  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
 return sub;
}
