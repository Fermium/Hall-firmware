#include <Pga.h>
#include <MCP3304.h>
#include <HMI_abstraction.h>


//initialize PGAs
PGA113 pga_1(10);
PGA113 pga_2(11);
PGA113 pga_3(12);
MCP3304 adc(13);
HMI_abstraction HMI();

//encoder on pin 2,3


void setup()
{

}

void loop()
{

  adc.readSgl(0); //read a channel of the adc

}
