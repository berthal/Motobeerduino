/*Written by Dean Reading, 2012.  deanreading@hotmail.com

This example is a centi-second counter to demonstrate the
use of my SevSeg library.
*/

#include <Wire.h>
#include <OneWire.h>
#include <LiquidCrystal_I2C.h>

// Constantes das Portas
const byte SENSORPIN = 13; 
const byte BUZZER = 3; 
const byte rele = 5; 
const byte buttonUp = 12;
const byte buttonDown = 11;
const byte buttonReset = 10;
const byte buttonMenu = 9;

// Constantes de programaçao
const byte CONTROLE_RESISTENCIA = 0;
const byte CONTROLE_GELADEIRA = 1;
const byte CONTROLE_BRASSAGEM = 2;
const byte CONTROLE_POTENCIA = 3;
const byte DESCANSO_MOTOR = 5000;
const byte INTERVALO_TEMPERATURA = 1;

//Variaveis de configuração
short setTemp =45;
byte rampaAtual = 0;
byte nRampas = 3;
byte tempos[10] = {20,60,10};
short temperaturas[10] = {62,70,78};
unsigned long clock = 0;


//Variaveis de controle de execução
byte state = CONTROLE_RESISTENCIA;
boolean printSetTemp = true;
boolean printMenu = true;
boolean printTime = true;

unsigned long timer;
byte buttonState = 0;  
byte pressCount = 0;  
LiquidCrystal_I2C lcd(0x27,16,2);
OneWire ds(SENSORPIN); 

void setup() {  
	timer=millis();
	pinMode(rele, OUTPUT);  
	pinMode(buttonUp, INPUT_PULLUP); 
	pinMode(buttonDown, INPUT_PULLUP); 
	pinMode(buttonOk, INPUT_PULLUP); 
	pinMode(buttonCancel, INPUT_PULLUP);  
	//Inicializa LCD
	lcd.init();                      
	lcd.backlight();
	Wire.begin();
	lcd.begin(16, 2);
}

void loop() 
{
	if ( millis() - timer > 1000)
	{
		printTime = true;
		timer=millis();  
	}
	switch (state)
	{
		case CONTROLE_RESISTENCIA:
		case CONTROLE_GELADEIRA:
		case CONTROLE_BRASSAGEM:
		{
			controleTemperatura();
			break;
		}
		case CONTROLE_POTENCIA:
		{
			
		}
	}
	
	leBotoes();
	printTime = false;
}

void controleTemperatura()
{
	printMainScreen() ;
	float temp = getTemp();
	unsigned long tempoFalta = getTempo();
	if ( printTime)
	{    
		printRelogio(tempoFalta);			
		printTemperatura(temp);
	}
	controlaResistencia(temp,tempoFalta);
}

unsigned long getTempo()
{
	switch (state)
	{
		case CONTROLE_RESISTENCIA:
		{
			return millis()-clock;
		}
		case CONTROLE_GELADEIRA:
		{
			return DESCANSO_MOTOR-millis()+clock;
		}
		case CONTROLE_BRASSAGEM:
		{
			return tempos[rampaAtual]-millis()+clock;
		}
	}
}

void printMainScreen() 
{
	if (printMenu)
	{	
		lcd.clear();
		lcd.print("Alvo:");
		lcd.setCursor(0, 1);    
		lcd.print("Lido:");
		printMenu = false;
	}
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
}

void controlaResistencia(float temp,unsigned long tempoFalta)
{
	//Testa a temperatura e liga ou desliga a resistencia
	switch (state)
	{
		case CONTROLE_RESISTENCIA:
		{
			if (temp < setTemp )
			{
				digitalWrite(rele, HIGH);
			}
			else
			{ 
				if ( clock == 0 )
				{
					clock = millis();
					toneAcerto();
				}
				digitalWrite(rele, LOW); 
			}
			break;
		}
		case CONTROLE_GELADEIRA:
		{
			if (temp > setTemp && tempoFalta < 0)
			{
				digitalWrite(rele, HIGH);
				clock = 0;
			}
			else if (temp < setTemp-INTERVALO_TEMPERATURA )
			{ 
				if ( clock == 0 )
				{
					clock = millis();
				}
				digitalWrite(rele, LOW); 
			}
			break;
		}
		case CONTROLE_BRASSAGEM:
		{
			if ( clock > 0 )
			{
				if (temp < setTemp )
				{
					digitalWrite(rele, HIGH);
				}
				else
				{ 
					digitalWrite(rele, LOW); 
				}
				if (tempoFalta < 0)
				{
					if (rampaAtual < nRampas-1 )
					{
						rampaAtual++;
						setTemp = temperaturas[rampaAtual];	
						toneAcerto();					
					}
					else
					{
						if ( printTime)
						{
							toneAcerto();
						}			
					}
				}
			}
			break;
		}
	}
	
}

void printTemperatura(float temp) 
{	
	lcd.setCursor(5, 1);  
	if (temp < 0 )
	{
		temp = temp*-1;
		lcd.print("-");
		if ( temp > 30 )
		{
			lcd.print("-----");
		}
		else
		{
			if ( temp <10 )
				lcd.print(0);
			lcd.print(temp,2);
		}
	}
	else
	{
		if ( temp <100 )
			lcd.print(" ");
		if ( temp <10 )
			lcd.print(0);
		lcd.print(temp,2);    
	}
}

void printRelogio(unsigned long tempoFalta) 
{
		
	if ( clock > 0 )
	{
		lcd.setCursor(10, 0);			
		
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
		lcd.print(" --:--");
	}
}

void leBotoes() 
{
	//Le o butao up e se apertado aumenta a temperatura
	buttonState = digitalRead(buttonUp);
	if (buttonState == LOW) 
	{
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
		//Le o butao Down e se apertado diminui a temperatura
		buttonState = digitalRead(buttonDown);
		if (buttonState == LOW) 
		{
			setTemp--;
			printSetTemp = true;
			if ( pressCount > 5 )
				delay(20);
			else
				delay(300);
			pressCount++;
		} 
		//Se nenhum dos dois estiver apertado zera contagem de eventos
		else
		{
			pressCount = 0;
		}
	}
	buttonState = digitalRead(buttonReset);
	if (buttonState == LOW) 
	{
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
}

float getTemp()
{
	byte data[12];
	byte addr[8];
	if ( !ds.search(addr)) 
	{
		ds.reset_search();
		return -1000;
	}
	if ( OneWire::crc8( addr, 7) != addr[7]) 	
	{
		return -1000;
	}
	if ( addr[0] != 0x10 && addr[0] != 0x28) 
	{
		return -1000;
	}
	ds.reset();
	ds.select(addr);
	ds.write(0x44,1); 
	byte present = ds.reset();
	ds.select(addr); 
	ds.write(0xBE);     
	for (int i = 0; i < 9; i++) 
	{ 
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




