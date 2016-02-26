#include <MCP3304.h>
#include <Arduino.h>

#define ADC_MOSI 11   // ADC_MOSI
#define ADC_MISO 12    // ADC_MISO
#define SPI_CLK 13  // Clock
#define ADC_CS 19

void MCP3304::init (void)
{

	//set pin modes
 pinMode(ADC_CS, OUTPUT);
 digitalWrite(SPI_CLK,HIGH); //clock
 digitalWrite(ADC_MISO, LOW); //output -- supposed to be float but not sure how to do this on Arduino
 digitalWrite(ADC_MOSI, HIGH); //input
 digitalWrite(ADC_CS, HIGH);
 delayMicroseconds(50);
 digitalWrite(ADC_CS, LOW);
 delayMicroseconds(50);
 digitalWrite(ADC_CS, HIGH);
}

MCP3304::MCP3304(int CS) {
init();
}

void MCP3304::pulse(void)
{
  digitalWrite(SPI_CLK, LOW); delayMicroseconds(50);
  digitalWrite(SPI_CLK, HIGH); delayMicroseconds(50);
}

int MCP3304::read(int channel) {

	init();

	int value=0;
	digitalWrite(ADC_CS, HIGH);
	delayMicroseconds(50);
	digitalWrite(ADC_CS, LOW);
	delayMicroseconds(50);
	digitalWrite(ADC_MOSI, HIGH); //start pulse
	pulse();
	digitalWrite(ADC_MOSI, HIGH); //single mode
	pulse();
	for(int x=2; x>=0; --x)
	{
		if(bitRead(channel,x))
		{
			digitalWrite(ADC_MOSI, HIGH);
		}
		else
		{
			digitalWrite(ADC_MOSI, LOW);
		}
		pulse();
	}
	digitalWrite(ADC_MOSI, LOW);//put low throughout
	for(int x=1;x<=3;++x)
	{
		//blank, Null, and sign bit (always 0 in single mode)
		pulse();
	}
	//now get twelve bits
	for(int x=11; x>=0; --x)
	{
			pulse();
			if (digitalRead(ADC_MISO))
			{
				 bitSet(value,x);
			}
			else
			{
				 bitClear(value,x);
			}
	}
	digitalWrite(ADC_CS, HIGH);
	delayMicroseconds(10);
	return value;
}

int MCP3304::getCSPin() {
	return ADC_CS;
}
