#include <ESP8266WiFi.h>     //Include Esp library
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <MFRC522.h>        //include RFID library
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>


#define SS_PIN D8 //RX slave select (SDA on MFRC522)
#define RST_PIN D3
#define RedLed D1
#define GreenLed D2

BlynkTimer timer;

WidgetRTC rtc;
WidgetLCD blynkLCD(V0);
WidgetLED blynkLedGreen(V1);
WidgetLED blynkLedRed(V2);
WidgetTable table(V3);
WidgetTerminal terminal(V5);
int rowIndex = 0;
String currentTime;
String currentDate;

BLYNK_WRITE(V4) {
  if (param.asInt()) {
    table.clear();
    rowIndex = 0;
  }
}

BLYNK_WRITE(V5)
{
  if (String("Clear LCD") == param.asStr() || String("clear lcd") == param.asStr() || String("clear LCD") == param.asStr() || String("clear") == param.asStr()) {
    blynkLCD.clear();
  }
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details
  if (isAM()){
    currentTime = String(hourFormat12()) + ":" + minute() + ":" + second() + " AM";
  }
  if (isPM()){
    currentTime = String(hourFormat12()) + ":" + minute() + ":" + second() + " PM";
  }
  currentDate = String(day()) + "/" + month() + "/" + year();
  /*Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();*/
  terminal.clear();
  terminal.println(currentTime) ;
  terminal.println(currentDate) ;
  terminal.flush();
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}


MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
SoftwareSerial NodeMCU(D0,D4); //RX,TX

/* Set these to your desired credentials. */
const char *ssid = "Kiran Saini EXT";  //ENTER YOUR WIFI SETTINGS
const char *password = "saini@1234";
char auth[] = "4432a5e7605444c78dbd22cd315a0a5a";

//Web/Server address to read/write from 
const char *host = "192.168.1.2";   //IP address of server

String getData ,Link;
String CardID="";
String cardLastDigits;
String serData;


void setup() {
  
  Serial.begin(9600);
  NodeMCU.begin(9600);
  pinMode(D0,INPUT); // RX as INPUT
  pinMode(D4,OUTPUT); // TX as OUTPUT
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  pinMode(RedLed,OUTPUT);
  pinMode(GreenLed,OUTPUT);

  Blynk.begin(auth, ssid, password);
  table.clear();
  terminal.clear();
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
  
  // Display digital clock every 1 second
  timer.setInterval(1000L, clockDisplay);
}

void loop() {
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Reconnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  }

  Blynk.run();
  timer.run();
  
  //look for new card
   if ( ! mfrc522.PICC_IsNewCardPresent()) {
  return;//got to start of loop if there is no card present
 }
 // Select one of the cards
 if ( ! mfrc522.PICC_ReadCardSerial()) {
  return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
 }

 for (byte i = 0; i < mfrc522.uid.size; i++) {
     CardID += mfrc522.uid.uidByte[i];

}

  delay(200);
  HTTPClient http;    //Declare object of class HTTPClient
  
  //GET Data
  getData = "?CardID=" + CardID;  //Note "?" added at front
  Link = "http://192.168.1.2/loginsystem/postdemo.php" + getData;
  
  http.begin(Link);
  
  int httpCode = http.GET();            //Send the request
  delay(10);
  String payload = http.getString();    //Get the response payload
  
  Serial.print("HTML Code:");
  Serial.println(httpCode);   //Print HTTP return code
  //Serial.println();
  Serial.println(payload);    //Print request response payload
  //Serial.println();
  Serial.println(CardID);     //Print Card ID
  //Serial.println();

  cardLastDigits = CardID.substring(7, 10);
  Serial.println(cardLastDigits);
  serData = cardLastDigits + "," +  payload + "," + currentTime + "," + currentDate + ".";
  Serial.println(serData);
  
  if(payload == "login"){
    digitalWrite(GreenLed,HIGH);
    blynkLedGreen.on();
    Serial.println("green on");
    Serial.println();
    delay(500);  //Post Data at every 5 seconds
    blynkLCD.print(3, 0, CardID);
    table.addRow(rowIndex, CardID, "Login");
    rowIndex++;
    NodeMCU.print(serData);
  }
  else if(payload == "logout"){
    digitalWrite(RedLed,HIGH);
    blynkLedRed.on();
    Serial.println("red on");
    Serial.println();
    delay(500);  //Post Data at every 5 seconds
    blynkLCD.print(3, 0, CardID);
    table.addRow(rowIndex, CardID, "Logout");
    rowIndex++;
    NodeMCU.print(serData);
  }
  else if(payload == "successful" || payload == "Cardavailable1"){
    digitalWrite(GreenLed,HIGH);
    digitalWrite(RedLed,HIGH);
    blynkLedGreen.on();
    blynkLedRed.on();
    Serial.println("green on");
    Serial.println("red on");
    Serial.println();
    delay(500);
    blynkLCD.print(3, 0, CardID);
    table.addRow(rowIndex, CardID, "Card Available");
    rowIndex++;
    NodeMCU.print(serData); 
  }
  else if(payload == "NotAllow"){
    for(int j=0; j<5; j++){
    digitalWrite(GreenLed,HIGH);
    digitalWrite(RedLed,HIGH);
    blynkLedGreen.on();
    blynkLedRed.on();
    delay(200);
    digitalWrite(GreenLed,LOW);
    digitalWrite(RedLed,LOW);
    blynkLedGreen.off();
    blynkLedRed.off();
    Serial.println();
    }
  }
  delay(500);
  
  CardID = "";
  getData = "";
  Link = "";
  cardLastDigits = "";
  serData = "";
  http.end();  //Close connection

  blynkLedGreen.off();
  blynkLedRed.off();
  digitalWrite(RedLed,LOW);
  digitalWrite(GreenLed,LOW);
}
