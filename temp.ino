/*-----( Import needed libraries )-----*/
#include <math.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <stdlib.h>
#include <LiquidCrystal_I2C.h>

/*-----( Define Functions )-----*/
void startUp();
void setDate();
void confirmDate();`  
void displayDate();
void writeDate();
void readingValues();
void displaySerial();

/*-----( Declare objects )-----*/
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#define SEMITEC_103JT_10k 3435.0f,298.15f,10000.0f  // B,T0,R0
#define THERM1PIN 0     // Arduino Thermistor PIN
#define THERM2PIN 2     // Pack Thermistor PIN            
#define STARTPIN 9      // Routine start button
File logfile;

/*-----( Declare Constants )-----*/
const int minusButton = 7;     // the number of the pushbutton pin
const int plusButton = 8;     // the number of the pushbutton pin
const int okButton = 9;     // the number of the pushbutton pin

/*-----( Declare Variables )-----*/
byte day;
byte month;
byte year;
int serialNo;
int lastSerialNo;
int resultV = 100;
int resultI = 100;
double vcc =4.97;
byte minusButtonLast = 1;     // the number of the pushbutton pin
byte plusButtonLast = 1;     // the number of the pushbutton pin
//int okButtonLast = 0;     // the number of the pushbutton pin



enum { // enumerating 3 major temperature scales
  T_KELVIN = 0,
  T_CELSIUS,
  T_FAHRENHEIT
};

uint8_t fail = 0;
uint8_t start = 0;  

float Temperature(int AnalogInputNumber, int OutputUnit, float B, float T0, float R0, float R_Balance)
{
  float R, T;

  //  R=1024.0f*R_Balance/float(analogRead(AnalogInputNumber)))-R_Balance;
  R = R_Balance * (1024.0f / float(analogRead(AnalogInputNumber)) - 1);

  T = 1.0f / (1.0f / T0 + (1.0f / B) * log(R / R0));

  switch (OutputUnit) {
    case T_CELSIUS :
      T -= 273.15f;
      break;
    case T_FAHRENHEIT :
      T = 9.0f * (T - 273.15f) / 5.0f + 32.0f;
      break;
    default:
      break;
  };
  checkResistance(T, R);
  return T;

}

int checkResistance(int temperature, int res) {
  if (temperature > 15 && temperature <= 17.5) {
    if ( res < 13000 || res > 16500) {
      bitSet(fail, 0);
    }
  }
  if (temperature > 17.5 && temperature <= 20) {
    if ( res < 11500 || res > 15000) {
      bitSet(fail, 0);
    }
  }
  if (temperature > 20 && temperature <= 22.5) {
    if ( res < 10000 || res > 13500) {
      bitSet(fail, 0);
    }
  }
  if (temperature > 22.5 && temperature <= 25) {
    if ( res < 9000 || res > 12000) {
      bitSet(fail, 0);
    }
  }
  if (temperature > 25 && temperature <= 27.5) {
    if ( res < 8000 || res > 11000) {
      bitSet(fail, 0);
    }
  }
  if (temperature > 27.5 && temperature <= 30) {
    if ( res < 7000 || res > 10000) {
      bitSet(fail, 0);
    }
  }
}


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.clear();
  pinMode(STARTPIN, INPUT);
  pinMode(10, OUTPUT);
  lcd.setCursor(0,0);
  lcd.print("Initializing SD card...");  // initialize the SD card
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  for(int i = 0; i<16; i++)
    { 
    lcd.setCursor(i,2);     //Start at character i on line 1
    lcd.print((char)255);   //Print out progress status bar.......
    delay(95);
  }
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10)) {
    lcd.clear();
    lcd.println("Card failed or  ");
    lcd.setCursor(0,1);
    lcd.println("not present...  ");
    // don't do anything more:
    return;
  }
  lcd.print("OK!");
  delay(200);
  lcd.clear();
  lcd.print("Hold button to");
  lcd.setCursor(0,1);
  lcd.print("create new file");
  delay(5000);
  lcd.clear();
  // create a new file
  char filename[] = "RESULT00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename )) {
      if( digitalRead(9)){
        logfile = SD.open(filename, FILE_WRITE);
        lcd.clear();
  lcd.print("New File: ");
  lcd.setCursor(0,1);
  lcd.print(filename);
        break;
      }
      else {
      uint8_t j = i - 1;
      filename[6] = j/10 + '0';
      filename[7] = j%10 + '0';
      lcd.print("Resuming to: ");
      lcd.setCursor(0,1);
      lcd.print(filename);
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
      }
    }
  }
  
  if (! logfile) {
    lcd.print("couldnt create file");
  }
  
}

void loop() {
  while (start != 1) {
    start = digitalRead(STARTPIN);
    delay(50);
  }
lcd.clear();
lcd.print(Temperature(THERM1PIN, T_CELSIUS, SEMITEC_103JT_10k, 9902.0f));
//

  delay(1500);
  start = 0;
  // Delay a bit...
}
