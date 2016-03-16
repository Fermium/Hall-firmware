
#ifndef MCP3304_h
#define MCP3304_h

#include <Arduino.h>

class MCP3304
{
public:
								//constructor
								MCP3304(int CS);

								//read Single of Pin
								int read(int channel, bool mean);
								int read(int channel);

								// Return SS Pinnumber
								int getCSPin();

								//pulse clock
								void pulse();

								//initialize spi and IC
								void init();

								//mean the value of read
								int mean_value(int channel);

private:
								void stack_value(int value,int channel);
								int* stacks[8]; // Circual arrays for mean_value calculation
								int ht_stacks[8][2];
								/* Heads and Sizes matrix for stacks circualr array in form of:
								       [,0]		[,1]
								       Head		Size
								   +-------|-------+
								   [0,] Pin 0  |	Head0	|	Size0 |
								   [1,] Pin 1	|	Head1	|	Size1 |
								   [2,] Pin 2	|	Head2	|	Size2 |
								   [3,] Pin 3	|	Head3	|	Size3 |
								   [4,] Pin 4	|	Head4	|	Size4 |
								   [5,] Pin 5	|	Head5	|	Size5 |
								   [6,] Pin 6	|	Head6	|	Size6 |
								   [7,] Pin 7	|	Head7	|	Size7 |
								   +-------|-------+
								 */
								int ADC_CS;

};
#endif
