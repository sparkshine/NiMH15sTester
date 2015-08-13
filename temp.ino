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
void confirmDate();
void displayDate();
void writeDate();
void readingValues();
void displaySerial();
/*-----( Declare Constants )-----*/
const int minusButton = 7;     // the number of the pushbutton pin
const int plusButton = 8;     // the number of the pushbutton pin
const int okButton = 9;     // the number of the pushbutton pin
const int voltagePin = A2;  //Voltage measurement pin
const int currentPin = A3;  //Voltage measurement pin
/*-----( Declare Variables )-----*/
byte day;
byte month;
byte year;
int serialNo;
int lastSerialNo;
int resultV = 100;
int resultI = 100;
float vcc = 4.97;
byte minusButtonLast = 1;     // the number of the pushbutton pin
byte plusButtonLast = 1;     // the number of the pushbutton pin
//int okButtonLast = 0;     // the number of the pushbutton pin
/*-----( Declare objects )-----*/
File SDFileData;
#define SD_CARD_CD_DIO 10 /* DIO pin used to control the modules CS pin */
#define SEMITEC_103JT_10k 3435.0f,298.15f,10000.0f  // B,T0,R0
#define THERM1PIN 0     // Arduino Thermistor PIN
#define THERM2PIN 2     // Pack Thermistor PIN            
#define STARTPIN 9      // Routine start button
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
// addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
enum { // enumerating 3 major temperature scales
  T_KELVIN = 0,
  T_CELSIUS,
  T_FAHRENHEIT
};

/*----( SETUP: RUNS ONCE )----*/
void setup()
{

  Serial.begin(9600);  // Used to type in characters
  lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  /* DIO pin uesd for the CS function. Note that even if you are not driving this
   function from your Arduino board, you must still configure this as an output
   otherwise the SD library functions will not work. */
  pinMode(10, OUTPUT);
  lcd.backlight(); // backlight on

  if (!SD.begin(SD_CARD_CD_DIO)) {
    lcd.clear();
    lcd.setCursor(0, 0); //Start at character 4 on line 0
    lcd.print("SD ERROR");
    while (1);
  } else
  {
    lcd.clear();
    lcd.setCursor(0, 0); //Start at character 4 on line 0
    lcd.print("CREASEFIELD");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("CBTB002-001");
    delay(2000);
  }
  pinMode(minusButton, INPUT);
  pinMode(plusButton, INPUT);
  pinMode(okButton, INPUT);

}/*--(end setup )---*/


void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
  setDate();
  confirmDate();
  writeDate();
  findLastSerial();
  Serial.println("found last serial");
  while (1) {
    readingValues();
  }

}/* --(end main loop )-- */


/*----( FUNCTIONS )----*/
/////////////////////////

/*---------------( USER SETTING OF SDCARD DATE )-------------*/
void setDate()
{
  lcd.clear();
  /* Check if the text file exists */
  if (SD.exists("date.txt"))
  {
    Serial.println("date.txt exists, attempting to read file...");

    /* The file exists so open it */
    SDFileData = SD.open("date.txt");

    day = SDFileData.parseInt();
    month = SDFileData.parseInt();
    year = SDFileData.parseInt();

    SDFileData.close();
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0); //Start at character 4 on line 0
    lcd.print("SD-ERROR");
    lcd.setCursor(0, 1); //Start at character 4 on line 0
    lcd.print("DATE-LOST");
    while (1);
  }

  while (digitalRead(okButton) == 1)
  {
    //delay(100);

    if (digitalRead(minusButton) != minusButtonLast)
    {
      delay(200);
      minusButtonLast != minusButtonLast;
      day++;
      if (day == 32)
      {
        day = 1;
      }
    }
    if (digitalRead(plusButton) != plusButtonLast)
    {
      delay(200);
      plusButtonLast != plusButtonLast;
      month++;
      if (month == 13)
      {
        month = 1;
      }

    }

    displayDate();
  }

  // Confirm Date
  lcd.setCursor(0, 1); //Start at character 4 on line 0
  lcd.print("SET? CANCEL? ---");
}

/*---------------( CONFIRM DATE )-------------*/
void confirmDate()
{

  while (1)
  {
    if (digitalRead(minusButton) == 0)
    {
      lcd.clear();
      lcd.setCursor(0, 0); //Start at character 4 on line 0
      lcd.print("Saving.");
      delay(2000);
      return;

    }
    else if (digitalRead(plusButton) == 0)
    {
      setDate();
    }
  }
}

/*---------------( DISPLAY DATE TO LCD SCREEN )-------------*/
void displayDate()
{
  //Date
  //lcd.clear();
  lcd.setCursor(0, 0); //Start at character 4 on line 0
  lcd.print("DATE:");

  //Day
  lcd.setCursor(7, 0); //Start at character 4 on line 0
  char buff[5];
  sprintf(buff, "%02d", day);
  lcd.print(buff);


  //Month
  lcd.setCursor(10, 0); //Start at character 4 on line 0
  sprintf(buff, "%02d", month);
  lcd.print(buff);

  //Year
  lcd.setCursor(13, 0); //Start at character 4 on line 0
  sprintf(buff, "%02d", year);
  lcd.print(buff);


  lcd.setCursor(0, 1);
  lcd.print("DD++  MM++   OK");
  //delay(1000);

}

/*---------------( WRITE DATE TO SDCARD )-------------*/
void writeDate()
{

  SDFileData = SD.open("date.txt", FILE_WRITE);
  /* Check if the text file already exists */
  while (SD.exists("date.txt"))
  {
    /* If so then delete it */
    //Serial.println("date.txt already exists...DELETING");
    SD.remove("date.txt");
  }

  /* Create a new text file on the SD card */
  //Serial.println("Creating date.txt");
  SDFileData = SD.open("date.txt", FILE_WRITE);


  /* If the file was created ok then add some content */
  if (SDFileData)
  {
    //SDFileData.println("18,03,14");

    //Date
    SDFileData.print(day);
    SDFileData.print(",");
    SDFileData.print(month);
    SDFileData.print(",");
    SDFileData.print(year);

    //Close file
    SDFileData.close();

  }


}

void findLastSerial()
{
  //Serial.println("find last serial");

  /* Check if the text file exists */
  if (SD.exists("results.txt"))
  {
    //Serial.println("results.txt exists, attempting to read file...");

    /* The file exists so open it */
    SDFileData = SD.open("results.txt");
    while (SDFileData.available())
    {
      lastSerialNo = SDFileData.parseInt();
      serialNo = lastSerialNo + 1;
      resultV = SDFileData.parseInt();
      //resultI = SDFileData.parseInt();

      Serial.print("S/N::");
      Serial.println(serialNo);
    }

    SDFileData.close();
    lcd.clear();
  }

  else
  {
    lcd.clear();
    lcd.setCursor(0, 0); //Start at character 4 on line 0
    lcd.print("SD-ERROR");
    lcd.setCursor(0, 1); //Start at character 4 on line 0
    lcd.print("RESULTS LOST");
    while (1);
  }


  Serial.println("display serial");
  displaySerial();

  Serial.println("return to main");


}

float Temperature(int AnalogInputNumber, int OutputUnit, float B, float T0, float R0, float R_Balance) {
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
  return T;
}


/*---------------( ENTER SERIAL NUMBER, READING VALUES )-------------*/
void readingValues() {
  lcd.clear();
  Serial.print("INSERT PACK");
  

  float voltage = analogRead(voltagePin);
  lcd.print(voltage);
  while (voltage < 1024) {
    if (digitalRead(minusButton) != minusButtonLast) {
      delay(200);
      if (serialNo > lastSerialNo + 1) {
        minusButtonLast != minusButtonLast;
        serialNo--;
      }
    }
    if (digitalRead(plusButton) != plusButtonLast) {
      delay(200);
      plusButtonLast != plusButtonLast;
      serialNo++;
    }
    displaySerial();
    voltage = analogRead(voltagePin);
  }

  int ambTemp = Temperature(THERM1PIN, T_CELSIUS, SEMITEC_103JT_10k, 9902.0f);
//  int packR = R2_Balance * (1024.0f / float(analogRead(THERM2PIN)) - 1);
  //Once pack has been connected leave for delay until voltage and current settles
  lcd.clear();
  lcd.setCursor(0, 0); //Start at character 4 on line 0
  lcd.print("READING VALUES..");
  delay(500);
  //delay to allow voltage to settle after connecting pack
  for (int i = 0; i < 8; i++) {
    lcd.setCursor(i, 1); //Start at character 4 on line 0
    lcd.print(">");
    delay(124);//100ms delay
  }
  // Show Pack has been read and serial assigned
  lcd.clear();
  lcd.print("SN:");
  lcd.setCursor(3, 1); //Start at character 4 on line 0
  lcd.print(serialNo);
  delay(1000);
  lcd.setCursor(10, 1); //Start at character 4 on line 0
  lcd.print("CANCEL:");
  while (analogRead(voltagePin) < 10) {
    if (digitalRead(okButton) == 0) {
      return;
    }
  }
  //saveResults();
  SDFileData = SD.open("results.txt", FILE_WRITE);
  if (SDFileData) {
    SDFileData.println("");
    SDFileData.print(serialNo);
    SDFileData.print(",");
    SDFileData.print(ambTemp);
    SDFileData.print(",");
//    SDFileData.print(packR);
//    SDFileData.print(",");
//    SDFileData.print(voltage1);
//    SDFileData.print(",");
//    SDFileData.print(voltage2);
//    SDFileData.print(",");
//    SDFileData.print(scCurrent);
//    SDFileData.print(",");
    SDFileData.print(day);
    SDFileData.print("/");
    SDFileData.print(month);
    SDFileData.print("/");
    SDFileData.print(year);
    SDFileData.close();
    //update last saved serial withour re-reading sdcard file
    lastSerialNo = serialNo;
    //Move serial number on 1 from last just saved
    serialNo++;
  }
  lcd.clear();
}

void displaySerial()
{
  //lcd.clear();
  lcd.setCursor(0, 0); //Start at character 4 on line 0
  lcd.print("INSERT PACK");
  lcd.setCursor(0, 1); //Start at character 4 on line 0
  lcd.print("SN:");

  //char buff[5];
  //sprintf(buff, "%05d", serialNo);
  //lcd.print(buff);

  lcd.setCursor(3, 1); //Start at character 4 on line 0
  lcd.print(serialNo);

}

/*
static void freeRAM (){
  extern int __heap_start, *__brkval;
  int v;
  int free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.print(F("Free RAM : "));
  Serial.println(free); // Which is the unit?
}
*/



