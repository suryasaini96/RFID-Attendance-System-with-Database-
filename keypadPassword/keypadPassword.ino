// Program to demonstrate the MD_Parola library
//
// Uses the Arduino Print Class extension with various output types
//
// MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
//#include <SPI.h>
#include <Keypad.h>

const uint16_t WAIT_TIME = 200;

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[COLS] = {3, 2, 9, 8}; //connect to the column pinouts of the keypad
byte rowPins[ROWS] = {7, 6, 5, 4}; //connect to the row pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

String Password ="1234"; //create a password
String data= "";
//int Position = 0; //keypad position


void setup(void)
{
  P.begin();
  Serial.begin(9600);
  setLocked (true); //state of the password
}

void loop(void)
{
  char customKey = customKeypad.getKey();
  //P.begin(); 
  //mx.control(3, 0);
  if (customKey){
    //digitalWrite(CS_PIN,LOW);
    P.write(customKey);
    data+= customKey;
    Serial.println(data);
    delay(WAIT_TIME);
    P.displayClear();
    //digitalWrite(CS_PIN,HIGH);
  }
  if (data.length()== 4){
    if(data == Password){
      setLocked (false);
      Serial.println("Verified");
      delay(100);
      data = "";
    }
    else{
      setLocked (true);
      Serial.println("Wrong password");
      data = "";
    }  
  }
}

void setLocked(int locked){
  if(locked){
    Serial.println("LOCKED");
    }
    else{
      Serial.println("OPEN");
    }
  }
