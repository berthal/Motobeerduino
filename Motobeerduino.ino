/*Written by Dean Reading, 2012.  deanreading@hotmail.com
 
 This example is a centi-second counter to demonstrate the
 use of my SevSeg library.
 */

#include <Wire.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x27,16,2);
int SensorPin = 13; 
int BUZZER = 3; 
int rele = 5; 
int setTemp =85;
OneWire ds(SensorPin); 
const int buttonUp = 12;
const int buttonDown = 11;
const int buttonOk = 10;
const int buttonCancel = 9;
//Create an instance of the object.
int buttonState = 0;  
int pressCount = 0;  

//Create global variables
unsigned long timer;
int CentSec=0;
unsigned long clock = 0;
boolean printSetTemp = true;
void setup() {  
  timer=millis();
  pinMode(rele, OUTPUT);  
  pinMode(buttonUp, INPUT_PULLUP); 
  pinMode(buttonDown, INPUT_PULLUP); 
  pinMode(buttonOk, INPUT_PULLUP); 
  pinMode(buttonCancel, INPUT_PULLUP);  
   lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Wire.begin();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Alvo:");
  lcd.setCursor(0, 1);    
  lcd.print("Lido:");
}

void loop() {
  medir();
}

void medir() {
  float temp = getTemp();
  if (printSetTemp)
  {
    lcd.setCursor(5, 0);
    if ( setTemp <100 )
        lcd.print(" ");
    if ( setTemp <10 )
        lcd.print(0);
    lcd.print(setTemp);
    printSetTemp = false;
  }
  if ( millis() - timer > 1000)
  {    
    lcd.setCursor(10, 0);
    if ( clock > 0 )
    {
      unsigned long tempoFalta = millis()-clock;
      int mim = tempoFalta / 60000;
      int seg = (tempoFalta - mim*60000)/1000;
      if ( mim <100 )
        lcd.print(" ");
      if ( mim < 10 )
        lcd.print("0");
      lcd.print(mim);
      lcd.print(":");
      if ( seg <10 )
        lcd.print(0);
      lcd.print(seg);
    }
    else
    {
      lcd.print(" 00:00");
    }
    lcd.setCursor(5, 1);  
    if ( temp <100 )
        lcd.print(" ");
    if ( temp <10 )
        lcd.print(0);
    lcd.print(temp,2);
    
    timer=millis();
  
  }
  if (temp < setTemp )
  {
    digitalWrite(rele, HIGH);
  }
  else
  { 
    if ( digitalRead(rele) == HIGH )
    if ( clock == 0 )
    {
      clock = millis();
      toneAcerto();
    }
    digitalWrite(rele, LOW); 
  }
  buttonState = digitalRead(buttonUp);
  if (buttonState == LOW) {
    setTemp++;
    printSetTemp = true;
    if ( pressCount > 5 )
      delay(20);
    else
      delay(300);
    pressCount++;
  } 
  else
  {
    buttonState = digitalRead(buttonDown);
    if (buttonState == LOW) {
      setTemp--;
      printSetTemp = true;
      if ( pressCount > 5 )
        delay(20);
      else
        delay(300);
      pressCount++;
    } 
    else
    {
      pressCount = 0;
    }
  }
  buttonState = digitalRead(buttonOk);
  if (buttonState == LOW) {
    if ( clock == 0 )
    {
      clock = millis();
      toneAcerto();
    }
    else
      clock = 0;
    delay(200);
  } 
}

float getTemp(){

    byte data[12];
    byte addr[8];
    if ( !ds.search(addr)) {
      ds.reset_search();
      return -1000;
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      return -1000;
    }
    if ( addr[0] != 0x10 && addr[0] != 0x28) {
      return -1000;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1); 
    byte present = ds.reset();
    ds.select(addr); 
    ds.write(0xBE);     
    for (int i = 0; i < 9; i++) { 
      data[i] = ds.read();
    }
    ds.reset_search();
    byte MSB = data[1];
    byte LSB = data[0];
    float TRead = ((MSB << 8) | LSB); 
    float Temperature = TRead / 16;    
    return Temperature;
}

void toneAcerto()
{
   tone(BUZZER, 1000, 300);
   delay(300);
   tone(BUZZER, 1000, 300);
}




