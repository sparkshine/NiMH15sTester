#include <Adafruit_INA219.h>
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
const int minusButton = 8;     // the number of the pushbutton pin
const int plusButton = 7;     // the number of the pushbutton pin
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
int temperature;
int packR;
uint8_t fail = 0;
float vcc = 5.03;
byte minusButtonLast = 1;     // the number of the pushbutton pin
byte plusButtonLast = 1;     // the number of the pushbutton pin
//int okButtonLast = 0;     // the number of the pushbutton pin
/*-----( Declare objects )-----*/
File SDFileData;
#define SD_CARD_CD_DIO 10 /* DIO pin used to control the modules CS pin */
#define SEMITEC_103JT_10k 3435.0f,298.15f,10000.0f  // B,T0,R0
#define THERM1PIN 0     // Arduino Thermistor PIN
#define THERM2PIN 1     // Pack Thermistor PIN            
#define MOSFET 6 //
#define R2_Balance 11937.0f //
#define R1_Balance 9901.0f //
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
// addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
Adafruit_INA219 ina219;
enum { // enumerating 3 major temperature scales
  T_KELVIN = 0,
  T_CELSIUS,
  T_FAHRENHEIT
};

/*----( SETUP: RUNS ONCE )----*/
void setup() {
  uint32_t currentFrequency;
  Serial.begin(9600);  // Used to type in characters
  ina219.begin();
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
  } else{
    lcd.clear();
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
void setDate(){
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

  while (digitalRead(okButton) == 1) {
    if (digitalRead(minusButton) != minusButtonLast)
    {
      delay(200);
      minusButtonLast != minusButtonLast;
      day++;
      if (day == 32) {
        day = 1;
      }
    }
    if (digitalRead(plusButton) != plusButtonLast)
    {
      delay(200);
      plusButtonLast != plusButtonLast;
      month++;
      if (month == 13) {
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
void writeDate() {
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
  if (SDFileData) {
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
  if (SD.exists("results.txt")) {
    //Serial.println("results.txt exists, attempting to read file...");
    /* The file exists so open it */
    SDFileData = SD.open("results.txt");
    while (SDFileData.available()) {
      //      lastSerialNo = SDFileData.parseInt();
      //      serialNo = lastSerialNo + 1;
      //      resultV = SDFileData.parseInt();
      //      //resultI = SDFileData.parseInt();
      //      SDFileData.parseInt();
      lastSerialNo = SDFileData.parseInt();
      serialNo = lastSerialNo + 1;
      resultV = SDFileData.parseInt();
      Serial.print("S/N: ");
      Serial.print(serialNo);
      Serial.print(" ");
      byte lastDay = SDFileData.parseInt();
      byte lastMonth = SDFileData.parseInt();
      byte lastYear = SDFileData.parseInt();
    }

    SDFileData.close();
    lcd.clear();
    lcd.print("Last S/N: ");
    lcd.print(lastSerialNo);
    lcd.setCursor(0, 1);
    lcd.print("CONT-- ");
    lcd.print("NEW++");
    while (1) {
      if (digitalRead(minusButton) != minusButtonLast) {
        delay(200);
        break;
      }
      if (digitalRead(plusButton) != plusButtonLast) {
        delay(200);
        plusButtonLast != plusButtonLast;
        lastSerialNo = 0;
        serialNo = 1;
        break;
      }
    }
  }

  else {
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
  R = ((1024.0f * R_Balance / float(analogRead(AnalogInputNumber))) - R_Balance);
  //  R = R_Balance * (1024.0f / float(analogRead(AnalogInputNumber)) - 1);
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
  Serial.print(T);
  return T;
}

int resistanceErr() {
  lcd.clear();
  lcd.print("THERMISTOR ERROR");
  lcd.setCursor(0, 1);
  lcd.print("RETEST-- CONT++");
  while (1) {
    if (digitalRead(minusButton) != minusButtonLast) {
      delay(200);
      packR = readResistance();
      checkResistance(temperature, packR);
      if (bitRead(fail, 0) != 0) {
        break;
      }
    }
    if (digitalRead(plusButton) != plusButtonLast) {
      delay(200);
      minusButtonLast != minusButtonLast;
      bitSet(fail, 0);
      break;
    }
  }
}

int readResistance() {
  float rawADC = analogRead(THERM2PIN);
  int packR = rawADC * R2_Balance / (1024.0f - rawADC);
  return packR;
}

int checkResistance(int temperature, int packR) {
  if (temperature > 15 && temperature <= 17.5) {
    if ( packR < 13000 || packR > 16500) {
      resistanceErr();
    }
  }
  if (temperature > 17.5 && temperature <= 20) {
    if ( packR < 11500 || packR > 15000) {
      resistanceErr();
    }
  }
  if (temperature > 20 && temperature <= 22.5) {
    if ( packR < 10000 || packR > 13500) {
      resistanceErr();
    }
  }
  if (temperature > 22.5 && temperature <= 25) {
    if ( packR < 9000 || packR > 12000) {
      resistanceErr();
    }
  }
  if (temperature > 25 && temperature <= 27.5) {
    if ( packR < 8000 || packR > 11000) {
      resistanceErr();
    }
  }
  if (temperature > 27.5 && temperature <= 30) {
    if ( packR < 7000 || packR > 10000) {
      resistanceErr();
    }
  }
  bitSet(fail, 0);
}


/*---------------( ENTER SERIAL NUMBER, READING VALUES )-------------*/
void readingValues() {
  freeRAM();
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.println("");
  lcd.clear();
  float voltage = analogRead(voltagePin);
  lcd.print(voltage);
  while (voltage < 10) {
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
  temperature = Temperature(THERM1PIN, T_CELSIUS, SEMITEC_103JT_10k, R1_Balance);
  packR = readResistance();
  while ( bitRead(fail, 0) != 1) {
    checkResistance(temperature, packR);
    bitRead(fail, 0);
  }

  Serial.print("PACK P:");
  Serial.println(packR);
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
    SDFileData.print(temperature);
    SDFileData.print(",");
    SDFileData.print(packR);
    SDFileData.print(",");
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
  fail = 0;
  lcd.clear();
}

void shortcircuitI()
{
  digitalWrite(MOSFET, HIGH);
  delay(10000);
  //take current reading
}

void displaySerial() {
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


static void freeRAM () {
  extern int __heap_start, *__brkval;
  int v;
  int free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.print(F("Free RAM : "));
  Serial.println(free); // Which is the unit?
}




