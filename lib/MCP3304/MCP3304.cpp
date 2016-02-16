/*
	MCP3304.h - Library for Microchip MC3302/4 13bit 4/8CH ADC

	with some inspiration form http://forum.arduino.cc/index.php?topic=216121.15

	for the MCP3304:
		use as 8CH single ended ADC (SGL)
		or as 4CH differential ADC (DIFF)

	According to the datasheet the maximum SPI frequency is about 2MHz. During testing
	i realised that a SPI frequency of 1MHZ (SPI_CLOCK_DIV16) works best.

	The return is an integer from -4096 to 4095 for -Vref to +Vref-1LSB in DIFF mode
	and 0 to 4095 for o to +Vref-1LSB in SGL mode

	version 0.1 (Feb 9, 2014) (not finished, only main functions, only Arduino standard SPI)
	Created by Stefan Wallnoefer, Feb 8, 2014.
	Copyright (c) 2014 Stefan Wallnoefer(walle86) under The MIT License (MIT)
	for other librarys and sketches look at https://github.com/walle86/
*/

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
 pinMode(ADC_MOSI, OUTPUT);
 pinMode(ADC_MISO, INPUT);
 pinMode(SPI_CLK, OUTPUT);
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
//ADC_CS = CS;
init();
}

void MCP3304::pulse(void)
{
  digitalWrite(SPI_CLK, LOW); delayMicroseconds(50);
  digitalWrite(SPI_CLK, HIGH); delayMicroseconds(50);
}

int MCP3304::readSgl(int channel) {
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
