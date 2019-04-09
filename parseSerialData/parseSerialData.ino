const char EOPmarker = '.'; //This is the end of packet marker
char serialbuf[32]; //This gives the incoming serial some room. Change it if you want a longer incoming.

#include <SoftwareSerial.h>
#include <String.h> // we'll need this for subString
#define MAX_STRING_LEN 20 // like 3 lines above, change as needed.

SoftwareSerial SoftSer(A4, A5); // RX, TX

void setup(){
  Serial.begin(9600);
  SoftSer.begin(9600);
}

void loop() {
    if (SoftSer.available() > 0) { //makes sure something is ready to be read
      static int bufpos = 0; //starts the buffer back at the first position in the incoming serial.read
      char inchar = SoftSer.read(); //assigns one byte (as serial.read()'s only input one byte at a time
      delay(100); //Wait for data to completely receive 
      Serial.print(inchar);
      if (inchar != EOPmarker) { //if the incoming character is not the byte that is the incoming package ender
        serialbuf[bufpos] = inchar; //the buffer position in the array get assigned to the current read
        bufpos++; //once that has happend the buffer advances, doing this over and over again until the end of package marker is read.
      }
      else { //once the end of package marker has been read
        serialbuf[bufpos] = 0; //restart the buff
        bufpos = 0; //restart the position of the buff

        int x = atoi(subStr(serialbuf, ",", 1));
        int y = atoi(subStr(serialbuf, ",", 2));
        int z = atoi(subStr(serialbuf, ",", 3));

        Serial.print("The first number, x is: ");
        Serial.print(x);
        Serial.print(" - The second number, y is: ");
        Serial.print(y);
        Serial.print(" - The third number, z is: ");
        Serial.print(z);
        Serial.println();

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
