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

class PGA{

public:

  //initialize the PGA given the pins
  void PGA::PGA(char _pin1, char _pin2, char _pin3 )
  {
    //write values to global array
    PGA_pin[0] = _pin1;
    PGA_pin[1] = _pin2;
    PGA_pin[2] = _pin3;

    //set pins to output
    pinMode (_pin[0], OUTPUT);
    pinMode (_pin[1], OUTPUT);
    pinMode (_pin[2], OUTPUT);

    //initialize SPI library
    SPI.begin();

    //initialize PGAs with gain=1 and channel=0
    PGA_Set(1,0,0)
    PGA_Set(2,0,0);
    PGA_Set(3,0,0);

  }


void PGA::Set(char PgaN, char G, char Ch) {
  //take the SS pin low to select the chip:
  digitalWrite(_pin[PgaN-1], LOW);
  //send in the address and value via SPI:
  SPI.transfer(0x2A); //command "write"
  SPI.transfer((G<<4) + Ch);
  //take the SS pin high to de-select the chip:
  digitalWrite(_pin[PgaN-1], HIGH);
}

private:
const char _gains_array[8] = {1,2,5,10,20,50,100,200};
char       _pin[3] = {0,0,0}; //pga1, pga2, pga3 pins

};

#endif
