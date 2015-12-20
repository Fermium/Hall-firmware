/*
   PGA 113 arduino library
   Copyright 2015 fermiumlabs.com
   All Rights Reserved
   Written by: Davide Bortolami
 */

/*
   Description of gains and channels

   GAINS:
   0000 > 1
   0001 > 2
   0010 > 5
   0011 > 10
   0100 > 20
   0101 > 50
   0110 > 100
   0111 > 200

   CHANNELS:
   0000 > Vcal/ch0 [decimal 00]
   0001 > ch1      [decimal 01]
   0010 > X
   0011 > X
   0100 > X
   0101 > X
   0110 > X
   0111 > X
   1000 > X
   1001 > X
   1010 > X
   1011 > X
   1100 > Cal1 = GND        [decimal 12]
   1101 > Cal2 = 0.9 Vcal   [decimal 13]
   1110 > Cal2 = 0.1 Vcal   [decimal 14]
   1111 > Cal2 = Vref = GND [decimal 15]

 */

/*
   USAGE
   PGA pga(cs1, cs2, cs3);

 */

#ifndef Pga_h
#define Pga_h

#include "Arduino.h"
#include <SPI.h>

class PGA113 {

public:

//initialize the PGA given the pins
PGA113(char _pin)
{

        //set pin to output
        pinMode (_pin, OUTPUT);

        //take the SS pin high to de-select the chip:
        digitalWrite(_pin, HIGH);

        //initialize SPI library
        SPI.begin();         //to be called

        //initialize PGAs with gain=1 and channel=0
        Set(0,0);

        //initialize global variables
        //i'm to lazy to study a serious c++ solution
        _gains_available[0] = 1;
        _gains_available[1] = 2;
        _gains_available[2] = 5;
        _gains_available[3] = 10;
        _gains_available[4] = 20;
        _gains_available[5] = 50;
        _gains_available[6] = 100;
        _gains_available[7] = 200;
        _gain_set_index = 0; //index from the array above
        _channel_set = 0;
}

//set the gain in index form
void Set(char _G, char _Ch)
{
        //take the SS pin low to select the chip:
        digitalWrite(_pin, LOW);
        //send in the address and value via SPI:
        SPI.transfer(0x2A);         //command "write"
        SPI.transfer((_G<<4) + _Ch);
        //take the SS pin high to de-select the chip:
        digitalWrite(_pin, HIGH);

        _gain_set_index = _G;

}

//return the actual set  gain in numerical format
char GetSetGain(void)
{
        return _gains_available[_gain_set_index];
}

//return the gain in numerical format
char GetGain(char _i)
{
        return _gains_available[_i];
}

//return the channel as an index as table at the start of this source code
char GetChannel(void)
{
        return _channel_set;
}

private:
unsigned char _gains_available[8];
unsigned char _gain_set_index; //index from the array above
unsigned char _channel_set;
unsigned char _pin;

};

#endif
