/* RFID TAG READER
Author: vonPongrac

Short description: Program read RFID tags. If tags UID is not familiar, sets new user

Hardware: Arduino Mega 2560, LED, Buzzer, RFID module RC522 with tags

Arduino IDE v1.6.3

Copyrights by vonPongrac
*/
// Include librarys
#include <SPI.h>
#include <MFRC522.h>  // RFID module library
#include <RTClib.h>  // RTC library
#include <Wire.h>  // i2C/1-wire library
#include <SD.h>  // SD card library
#include <Ethernet.h>  // Etrhenret library

#define RST_PIN		6  // RST pin for RFID module
#define SS_PIN		7  // Slave Select pine for RFID module

MFRC522 mfrc522(SS_PIN, RST_PIN);  // define RFID reader class
RTC_DS1307 RTC;  // define RTC class

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address 
IPAddress ip(192, 168, 1, 177); // IP address
EthernetServer server(80); // define server class in port 80 - HTTP port

int buzzer = 8;  // speaker or buzzer on pin 8
int led_pos = 3; // green LED on pin 3
int led_neg = 2; // red LED on pin 2
String UID_tagA = "856a8b45";  // UID of tag that we are using
unsigned int MinsA = 0, HoursA = 0;  // working minutes and hours for tag A
String readTag = "";  
int readCard[4];
short UIDs_No = 1;
boolean TimeFlag[2] = {false, false};
DateTime arrival[2];  // tiem class for arrival
DateTime departure[2];  // time class for departure
int LastMonth=0;  // working hours till now in a month
char DataRead=0;

// Declaration of the functions
void redLED(); // red LED on
void greenLED(); // green LED + buzzer on
int getID();  // read tag
boolean checkTag();  // check if tag is unknown
void errorBeep();  // error while reading (unknown tag)
void StoreData();  // store data to microSD

File myFile; // class file for reading/writing to file on SD card


// SETUP
void setup() {
//  Serial.begin(9600); // for testing and debugging
  SPI.begin();  // run SPI library first; if not, RFID will not work
  mfrc522.PCD_Init();  // initializing RFID, start RFID library
  Wire.begin();  // start i2c library; if not, RTC will not work
  RTC.begin();  // start RTC library
  RTC.adjust(DateTime(__DATE__, __TIME__));  // set RTC time to compiling time
  Ethernet.begin(mac, ip);  // start Ethernet library 
  server.begin();  // start server 
//  Serial.print("server is at ");
//  Serial.println(Ethernet.localIP());
  SD.begin(4);  // start SD library
// setting DI/O
  pinMode(led_pos, OUTPUT);
  pinMode(led_neg, OUTPUT);
}

// MAIN PROGRAM
void loop() {
  int succesRead = getID(); // read RFID tag
  if(succesRead==1){ // if RFID read was succesful
    //greenLED();
    if (checkTag()){ // if tag is known, store data
      greenLED();
      StoreData();
    } else { // beeb an error; if new tag, then exit
      errorBeep();
    }
  } else {
    redLED();
  }
  // Web server
  EthernetClient client = server.available();  // check for HTTP request
  if (client) { // if HTTP request is available
//    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
 //       Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
        //  client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE html>");
          client.println("<html><head><title>Office Atendance Logger</title><style>");
          client.println(".jumbotron{margin: 1% 3% 1% 3%; border: 1px solid none; border-radius: 30px; background-color: #AAAAAA;}");
          client.println(".dataWindow{margin: 1% 3% 1% 3%; border: 1px solid none; border-radius: 30px; background-color: #AAAAAA;padding: 1% 1% 1% 1%;}");
          client.println("</style></head><body style=\"background-color: #E6E6E6\">");
          client.println("<div class=\"jumbotron\"><div style=\"text-align: center\"> <h1>  Office Atendance Logger </h1> </div> ");
          client.println("</div><div class=\"dataWindow\"><div style=\"text-align: center\"> <h2> User A </h2>");
          myFile = SD.open("A.txt");
          if(myFile){
            
            while(myFile.available()){
                client.print("<p>");
                while(DataRead != 59){
                  DataRead = (char)myFile.read();
                  client.print(DataRead);
            //      client.print(myFile.read());
                }
                client.println("</p>");
                DataRead = 0;
            }  
            
            myFile.close();
          }
          client.println("</div></body></html>");    
          break;     
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
//    Serial.println("client disconnected");
  }   
  delay(1000);
}

// FUNCTIONS
void redLED(){ // red LED on, green LED off
  digitalWrite(led_pos, LOW);
  digitalWrite(led_neg, HIGH);
}

void greenLED(){ // red LED off, green LED on
  digitalWrite(led_pos, HIGH);
  digitalWrite(led_neg, LOW);
  tone(buzzer, 440, 50); // sound; frequency of tone: 440 Hz, duration of tone: 50 ms
}

boolean checkTag(){ // check if tag is unknown
  if(readTag == UID_tagA){UIDs_No = 1; return true;}
//  else if(readTag == UID_tagB){UIDs_No = 2; return true;}
  else {return false;}
}

void errorBeep(){ // error option
  digitalWrite(led_pos, LOW);
  digitalWrite(led_neg, LOW);
  delay(150);
  digitalWrite(led_neg, HIGH);
  tone(buzzer, 440, 50);
  delay(150);
  digitalWrite(led_neg, LOW);
  delay(150);
  digitalWrite(led_neg, HIGH);
  tone(buzzer, 440, 50);
}

int getID() { // Read RFID
    // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
//  Serial.println(F("Scanned PICC's UID:"));
  readTag = "";
  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
//    Serial.print(readCard[i], HEX);
    readTag=readTag+String(readCard[i], HEX);
  }
  // Serial.println(readTag);
//  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void StoreData(){ // calculate and store data to SD card
  DateTime time = RTC.now(); // read time from RTC
  if(LastMonth != time.month()){ // check if there is a new month
    LastMonth = time.month();
    SD.remove("hoursA.txt");
  }
  switch(UIDs_No){ // this is set for multiple tags, as of right now is made only for one tag
    case 1:
      if(TimeFlag[0]){ // departure
        departure[0] = time;  // save departure time
        // calculate working hours and minutes
        int dh = abs(departure[0].hour()-arrival[0].hour()); 
        int dm = abs(departure[0].minute()-arrival[0].minute()); 
        unsigned int work = dh*60 + dm; // working hours in minutes
        MinsA = MinsA + work; // add working hours in minutes to working hours from this month
        HoursA = (int)MinsA/60; // calculate working hours from minutes
        myFile = SD.open("A.txt", FILE_WRITE); // open file with history and write to it
        if(myFile){ // format = " MM-DD-YYYY hh:mm (arrival), hh:mm (departure), hh (working hours today), hh (working hours this month);
          myFile.print(arrival[0].month(),DEC);
          myFile.print("-");
          myFile.print(arrival[0].day(),DEC);
          myFile.print("-");
          myFile.print(arrival[0].year(),DEC);
          myFile.print(" ");
          myFile.print(arrival[0].hour(),DEC);
          myFile.print(":");
          myFile.print(arrival[0].minute(),DEC);
          myFile.print(", ");
          myFile.print(departure[0].hour(),DEC);
          myFile.print(":");
          myFile.print(departure[0].minute(),DEC);
          myFile.print(", ");
          myFile.print(dh,DEC);
          myFile.print(":");
          myFile.print(dm,DEC);
          myFile.print(", ");
          myFile.print(HoursA,DEC);
          myFile.print(";");
          myFile.close();
        }
        TimeFlag[0] = false; // set time flag to false
      } else { // arrival; 
        arrival[0] = time;  // save time of arrival 
        TimeFlag[0] = true;  // set time flag to true
      }
      break;  
  }
}
