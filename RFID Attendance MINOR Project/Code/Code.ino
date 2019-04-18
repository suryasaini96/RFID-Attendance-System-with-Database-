/*
   The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * 
 *     SD card attached to SPI bus as follows:
 ** MOSI - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** MISO - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - depends on your SD card shield or module.
    Pin 4 used here for consistency with other Arduino examples
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <MFRC522.h> // for the RFID
#include <SPI.h> // for the RFID and SD card module
#include <SD.h> // for the SD card
#include "RTClib.h" // for the RTC
//#include <Wire.h> //for I2C protocol used in RTC
#include <LiquidCrystal.h>

// define pins for RFID
#define SS_RFID 10
#define RST_RFID 9
// define select pin for SD card module
#define CS_SD 4 

// Pins for LCD
const int rs = 5, en = 8, d4 = 3, d5 = 2, d6 = 1, d7 = 0;

// Create a file to store the data
File myFile;

// Instance of the class for RFID
MFRC522 rfid(SS_RFID, RST_RFID); 

// Instance of liquid crystal
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Variable to hold the tag's UID
String uidString="";
char buffer[36];
String strBuffer;
String readUid;
String readDate;
String readTime;
int present = 0;

// Instance of the class for RTC
RTC_DS1307 rtc;

// Define check in time
const int checkInHour = 20;
const int checkInMinute = 55;

//Variable to hold user check in
int userCheckInHour;
int userCheckInMinute;

// Pins for LEDs and buzzer
const int redLED = 6;
const int greenLED = 7;
const int buzzer = 5;

void setup() {
  
  // Init LCD
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("Hello World");
  
  // Set LEDs and buzzer as outputs
  pinMode(redLED, OUTPUT);  
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Init Serial port
  Serial.begin(9600);
  while(!Serial); // for Leonardo/Micro/Zero
  
  // Init SPI bus
  SPI.begin();
   
  // Init MFRC522 
  rfid.PCD_Init();
  rfid.PCD_DumpVersionToSerial(); 

  // Setup for the SD card
  Serial.print("Initializing SD card...");
  if(!SD.begin(CS_SD)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  delay(1000);

  // Setup for the RTC
  //Wire.begin();  
  if(!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while(1);
  }
  else {
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__),F( __TIME__)));
  }
}


void loop() {

  

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }
  readRFID();
  checkAttendance();
  uidString="";
  rfid.PICC_HaltA();
}


void readRFID() {
  
  //rfid.PICC_ReadCardSerial();
  Serial.print("Tag UID: ");
  /*uidString = String(rfid.uid.uidByte[0]) + " " + String(rfid.uid.uidByte[1]) + " " + 
    String(rfid.uid.uidByte[2]) + " " + String(rfid.uid.uidByte[3]);*/
    for (byte i = 0; i < rfid.uid.size; i++) {
        uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : " ");
        uidString += String(rfid.uid.uidByte[i], HEX);
    }
  uidString.toUpperCase();
  uidString.trim();
  Serial.println(uidString);
  // Sound the buzzer when a card is read
  tone(buzzer, 2000); 
  delay(100);        
  noTone(buzzer);
  delay(100);
}

void checkAttendance(){
   DateTime now = rtc.now();  
   myFile = SD.open("Data.txt");
   String currentDate;
   if(myFile){
     while(myFile.available()){
        strBuffer = myFile.readStringUntil('\n');
        strBuffer.toCharArray(buffer,36);
        char separator[] = ",";
        char *token;
        char *next_token;
        int counter = 1;
        /* get the first token */
        token = strtok(buffer, separator);
        readUid = String(token);
        next_token = token;
        // Find any more tokens?
        while(next_token != NULL) 
        {  
           if (counter == 2)
           {
              readDate = String(token);
              readDate.trim();
           }
           if (counter == 3)
           {
              readTime = String(token);
              readTime.trim();
           }
           // here you have last token that is not NULL 
            if(next_token = strtok(NULL, separator))
                   token = next_token;   //Last token in string
            counter++;
         }
        for (int i = 0; i < 36; i++)
        {
          buffer[i]=' '; // Clear the buffer
        }
        
        currentDate = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
        if ((readUid == uidString) && (readDate == currentDate))
        {
          Serial.println("Attendance for today has already been marked for this UID!");
          lcd.print("Late!");
          myFile.close();
          return;
        }
     }
   }
   else
   {
    Serial.println("Error opening file");
   }
   myFile.close();
   verifyCheckIn();
}  

void verifyCheckIn(){
  // Save check in time;
  DateTime now = rtc.now();  
  userCheckInHour = now.hour();
  userCheckInMinute = now.minute();
  if((userCheckInHour < checkInHour)||((userCheckInHour==checkInHour) && (userCheckInMinute <= checkInMinute))){
    digitalWrite(greenLED, HIGH);
    delay(2000);
    digitalWrite(greenLED,LOW);
    Serial.println("You're welcome!");
    logCard();
  }
  else{
    digitalWrite(redLED, HIGH);
    delay(2000);
    digitalWrite(redLED,LOW);
    Serial.println("You are late...attendance not marked");
  }
}

void logCard() {
  // Enables SD card chip select pin
  digitalWrite(CS_SD,LOW);
  DateTime now = rtc.now();  
  // Open file
  myFile=SD.open("Data.txt", FILE_WRITE);

  // If the file opened ok, write to it
  if (myFile) {
    Serial.println("File opened ok");
    myFile.print(uidString);
    myFile.print(", ");   
    
    // Save date on SD card
    myFile.print(now.day(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.year(), DEC);
    myFile.print(", ");
    
    // Save time on SD card
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);
    
    // Print time on Serial monitor
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(", ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    Serial.println("sucessfully written on SD card");
    myFile.close();
    present++;
    Serial.print("Total present:");
    Serial.println(present);
  }
  else {
    Serial.println("error opening data.txt");  
  }
  // Disables SD card chip select pin  
  digitalWrite(CS_SD,HIGH);
}
