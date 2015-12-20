#include <Arduino.h>
#include <Pga.h>
#include <MCP3304.h>
void setup();
void loop();
void begin (void);
void update (void);
#line 1 "src/sketch.ino"
//#include <Pga.h>
//#include <MCP3304.h>


//initialize PGAs
PGA113 pga_1(10);
PGA113 pga_2(11);
PGA113 pga_3(12);
MCP3304 adc(13);


void setup()
{

}

void loop()
{

}


char LCD[20][4][2];

void begin (void)
{
  memset(LCD, 0, (sizeof(LCD)/sizeof(LCD[0])); //clear the array

  //lcd init code
}

void update (void)
{
  for (char column=0; column != 20; column++)
  {
    for (char row=0; row!=4; row++)
    {
      if(LCD[column][row][1] != LCD[columnt][row][0] ) //if new value if different from old
      {
        //lcd set cursor at column, row
        //lcd write LCD[column][row][1]
      }
    }
  }
}
