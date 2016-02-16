
#ifndef MCP3304_h
#define MCP3304_h

#include <Arduino.h>

class MCP3304
{
	public:
		//constructor
		MCP3304(int CS);

		//read Single of Pin
		int read(int pin);

		// Return SS Pinnumber
		int getCSPin();

		//pulse clock
		void pulse();

		//initialize spi and IC
		void init();

	private:

		int ADC_CS;
};
#endif
