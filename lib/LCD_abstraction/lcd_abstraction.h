#ifndef HMI_abstraction_h
#define HMI_abstraction_h

#include "Arduino.h"

LiquidCrystal _lcd_unabstracted(1, 2, 3, 4, 5, 6); //cambiare numeri pin


class HMI_abstraction {
public:

void Begin (void)
{
  memset(_LCD_array, 0, (sizeof(_LCD_array)/sizeof(_LCD_array[0])); //clear the array

  //lcd init code
  _lcd_unabstracted.begin(20, 4);
  _lcd_unabstracted.noCursor();


  //create a custom char for omega
  const unsigned char _char_omega[8] = {
  	0b00000,
  	0b01110,
  	0b10001,
  	0b11011,
  	0b01010,
  	0b11011,
  	0b00000,
  	0b00000
  };
  lcd.createChar(0, _char_omega);

}

//print a boot sequence with
void boot(void){


}



//update from MCU memory to LCD
void Update (void)
{
  for (char column=0; column != 20; column++)
  {
    for (char row=0; row!=4; row++)
    {
      if(_LCD_array[column][row][1] != _LCD_array[columnt][row][0] ) //if new value if different from old
      {
        //lcd set cursor at column, row
        //lcd write LCD[column][row][1]
        _LCD_array[column][row][1] = _LCD_array[columnt][row][0];
      }
    }
  }
}


// clear X characters starting from a position
void ClearPos (char startcolumn, char startrow, char lenght)
{
  for (int i=0; i!= lenght, i++)
    {
      _LCD_array[startcolumn+i][startrow][1] = 0;
    }
}


//12.34  mA||
//1234.45 O||Vr G=200
//-123.3 °c||P   100%
//12.34   V||Vh G=200



void Write (char position, *char src)
{

  switch (position){
    case 1:
    clear_pos(0,0,8);
    strncpy(_LCD_array[0][0][0], src, 5);
    strncpy(_LCD_array[6][0][0], "mA", 2);
    break;
    case 2:
    break;
    case 3:
    clear_pos(0,1,8);
    strncpy(_LCD_array[0][1][0], src, 7);
    strncpy(_LCD_array[6][1][0], 0, 1); //simbolo omega
    break;
    case 4:
    clear_pos(11,1,8);
    strncpy(_LCD_array[16][1][0], src, 3);
    strncpy(_LCD_array[18][1][0], "Vr G=", 5);
    break;
    case 5:
    clear_pos(0,2,8);
    strncpy(_LCD_array[0][2][0], src, 6);
    strncpy(_LCD_array[7][2][0], "°c", 2);
    break;
    case 6:
    clear_pos(11,2,8);
    strncpy(_LCD_array[18][2][0], "P      %%", 8);
    strncpy(_LCD_array[15][2][0], src, 3);
    break;
    case 7:
    clear_pos(0,0,8);
    strncpy(_LCD_array[0][3][0], src, 5);
    strncpy(_LCD_array[7][3][0], "V", 1);
    break;
    case 8:
    clear_pos(11,3,8);
    strncpy(_LCD_array[16][3][0], src, 3);
    strncpy(_LCD_array[18][3][0], "Vh G=", 5);
    break;
    default:
    break;


  }






}

private:
  char _LCD_array[20][4][2];


}


#endif HMI_abstraction_h
