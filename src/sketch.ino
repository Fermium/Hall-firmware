#include <Pga.h>
#include <MCP3304.h>


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
        LCD[column][row][1] = LCD[columnt][row][0];
      }
    }
  }
}

// clear X characters starting from a position
void clear_pos (char startrow, char startcolumn, char lenght)
{
  for (int i=0; i!= lenght, i++)
    {
      LCD[startrow][startcolumn+i][1] = 0;
    }
}
void write (char position, *char src)
{
  // 9 char, 2 spaces, 9 char
  //4 rows

  switch (position){
    case 1:
    clear_pos(1,1,9);
    strncpy(LCD[1][1][1], src, 9)
    strncpy(LCD[8][1][1], "V", 9)
    break;
    case 2;
    clear_pos(12,1,9);
    strncpy(LCD[12][1][1], src, 9)
    strncpy(LCD[19][1][1], "V", 9)
  }




}
