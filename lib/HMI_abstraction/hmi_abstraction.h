/*
   Human Machine Interface abstraction library
   For Hall and RDT apparatus
   Copyright 2015 fermiumlabs.com
   All Rights Reserved
   Written by: Davide Bortolami
 */

/*
       LCD display format

      12.34  mA||
      1234.45 O||Vr G=200
      -123.3 °c||P   100%
      12.34   V||Vh G=200


*/
#ifndef HMI_abstraction_h
#define HMI_abstraction_h

#define HMI_DEBUG //Build debug functions

#define LCD_LENGHT 20
#define LCD_HEIGHT  4

#include "Arduino.h"
#include <LiquidCrystal.h>

//LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal _lcd_unabstracted(2, A2, A3, 1, A4, 0);
//PD2, PC2, PC3, PD1, PC4, PD0

/*
   Arduino Ax pins as digital
   Pin 14 = Analog in 0
   Pin 15 = Analog in 1
   Pin 16 = Analog in 2
   Pin 17 = Analog in 3
   Pin 18 = Analog in 4
   Pin 19 = Analog in 5

*/

class HMI_abstraction {
public:

        void Begin (void)
        {
                CleanAll(); //initialize array

                //lcd init code
                _lcd_unabstracted.begin(LCD_LENGHT, LCD_HEIGHT);
                _lcd_unabstracted.setCursor(0, 0);
                _lcd_unabstracted.noCursor();

                //buzzer
                pinMode(A1, OUTPUT);
                digitalWrite(A1, LOW);
        }

        void Clean(int col_start, int col_end, int row)
        {
                for(int i=col_start; i<col_end; i++) {
                        _LCD_array[i][row][1]=' ';
                }
        }

        //clean all rows, but does not force rewrite or hard clear the lcd
        void CleanAll ()
        {
                for (char row = 0; row != LCD_HEIGHT; row++)
                {
                    for (char column=0; column != LCD_LENGHT; column++)
                    {
                            _LCD_array[column][row][1] = ' ';
                    }
                    //Clean(0,LCD_LENGHT,row);
                }
        }

        //forcely clear the lcd and rewrite all chars
        void ForceRewrite ()
        {

              /* POSSIBLE IMPLEMENTATION GOT TO BE CHECKED
                 for (char column=0; column != LCD_LENGHT; column++)
                {
                        for (char row=0; row!=LCD_HEIGHT; row++)
                        {
                          _LCD_array[column][row][1]=_LCD_array[column][row][0];
                        }
                }
                CleanAll();
                Update();

              */
        }

        void Buzzer(bool on)
        {
                if(on)
                        tone(A1, 1000);
                else
                        noTone(A1);
        }


        void Buzzer(bool on, int frequency)
        {
                if(on)
                        tone(A1, frequency);
                else
                        noTone(A1);
        }


        // Custom bootscreen for LCD

        /*

         +----------------------+
         |     Hall  Effect     |
         |     Fermium LABS     |
         | web: fermiumlabs.com |
         | Sample:   Ge P-doped |
         +----------------------+
        */
        void SplashScreen(void )
        {
                _lcd_unabstracted.setCursor(0, 0);
                _lcd_unabstracted.print(F("    Hall  Effect    "));
                _lcd_unabstracted.setCursor(0, 1);
                _lcd_unabstracted.print(F("    Fermium LABS    "));
                _lcd_unabstracted.setCursor(0, 2);
                _lcd_unabstracted.print(F("web: fermiumlabs.com"));
                _lcd_unabstracted.setCursor(0, 3);
                _lcd_unabstracted.print(F("Sample:   "));
                _lcd_unabstracted.print(F(SAMPLE_TYPE));
                _lcd_unabstracted.print(F("-doped"));
        }

        //writes a string to the LCD array
        void WriteString(char column, char row, char *src)
        {
                char lenght = strlen(src);
                for (char i=0; ( i != lenght && ((i+column) != LCD_LENGHT)); i++)
                {
                        _LCD_array[column+i][row][1] = src[i];
                }
        }

        void GetLine (unsigned char _line, char* _dest_array, bool _third_parameter)
        {


                for(char i=0; i!=LCD_LENGHT; i++)
                {
                        _dest_array[i] = _LCD_array[i][_line][_third_parameter];
                }
                _dest_array[21] = 0;
        }

        char GetHeight()
        {
          return LCD_HEIGHT;
        }
        char GetLenght()
        {
          return LCD_LENGHT;
        }

        //update from MCU memory to LCD the chars that are different
        void Update (void)
        {
                for (char column=0; column != LCD_LENGHT; column++)
                {
                        for (char row=0; row!=LCD_HEIGHT; row++)
                        {
                                if(_LCD_array[column][row][1] != _LCD_array[column][row][0] )                 //if new value if different from old
                                {
                                        _lcd_unabstracted.setCursor(column, row);
                                        _lcd_unabstracted.write(_LCD_array[column][row][1]);
                                        _LCD_array[column][row][0] = _LCD_array[column][row][1];
                                }
                        }
                }
        }

private:
              char _LCD_array[LCD_LENGHT][LCD_HEIGHT][2]; //[][][0] is written, [][][1] to be written

};


#endif HMI_abstraction_h
