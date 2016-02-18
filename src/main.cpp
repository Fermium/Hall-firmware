#include <Pga.h>
#include <MCP3304.h>
#include <hmi_abstraction.h>
#include <ClickEncoder.h>
#include <math.h>
#include <TimerOne.h>         // [needed] change with something that is not CC-BY


//please use the tag  [needed] for code that need to be fixed

// [needed] move encoder inside hmi

//12.34  mA||
//1234.45 O||Vr G=200
//-123.3 °c||P   100%
//12.34  mV||Vh G=200

//1 current     - 2 none
//3 resistance  - 4 resistance gain
//5 temperature - 6 power to the heating element
//7 Hall voltage- 8 Hall Gain

//#define APPARATUS_RDT
#define APPARATUS_HALL
#define MAIN_DEBUG true

#ifdef APPARATUS_HALL

//adc channel definitions definitions
  #define ADC_CHANNEL_VH      0
  #define ADC_CHANNEL_VR      1
  #define ADC_CHANNEL_TEMP    2
  #define ADC_CHANNEL_CURRENT 3

//initialize PGAs
MCP3304 adc(19);   //atmega328 PC5

//initialize pga
PGA113 pga_vh(8);   //atmega328 PB0
PGA113 pga_vr(7);   //atmega328 PD7
PGA113 pga_3(9);    //atmega328 PB1

//calibration values:
float CAL_TEMPERATURE_ZERO_VOLT        = 2.5;
float CAL_TEMPERATURE_VOLTAGE_GAIN     = 0.01; // mV/°C
int   CAL_TEMPERATURE_OVERHEAT_LIMIT   = 150;  // °C
float CAL_SHUNT_RESISTOR               = 100.0;
float CAL_VOLTAGE_REFERENCE            = 5.0;  //adc voltage reference
float CAL_FIXED_GAIN_VRES              = 1.0;  //gain opamp sulla Vref
float CAL_FIXED_GAIN_VHALL             = 1.0;  //gain opamp sulla Vhall
float CAL_HALL_ZERO_VOLTAGE            = 2.5;

//pin hall/rdt
#define _pin_heater 5


#endif


hmi_abstraction hmi; //hmi is a wrapper around the LCD library
ClickEncoder *encoder;

//periodic subroutine called every 1ms
void timerIsr() {
        encoder->service(); //execute encoder stuff

}


void setup()
{
        encoder = new ClickEncoder(A1, A0, A2); //not really a fan of new...
        encoder->setAccelerationEnabled(true); //enable cool acceleration feeling

        //interrupt for the encoder reading and other useful stuff
        Timer1.initialize(1000);
        Timer1.attachInterrupt(timerIsr);

        hmi.Begin();

        //software SPI
        pinMode(11, OUTPUT); //MOSI
        pinMode(12, INPUT);  //MISO
        pinMode(13, OUTPUT); //CLK


        //MPC3304 is already initialized
        //PGAs are already initialized
}



//MODES
//each mode return the next mode. it usually is itself, but can be
//another one to jump in the menu

//hall: current mode, just update lcd.
//format: 99.99mA fixed range
//rdt:
//format:
char mode_1(int increment)
{
        float current;
        //remember IT guys, viva il re' d'italia
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / 4096 ) / CAL_SHUNT_RESISTOR );

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(current);
        floating_part = ((current - integer_part)*100);
        char sign;
        if (current > 0)
                sign = ' ';
        else
        {
                integer_part = -integer_part;
                floating_part = -floating_part;
                sign = '-';
        }

        char lcd_string[9];
        sprintf(lcd_string, "%c%2d.%02d mA", sign, integer_part, floating_part);

        hmi.WriteString(0, 0, lcd_string);


        return 1;
}
//hall: nothing selected, nothing to do
//rdt:
char mode_2(int increment)
{
        return 3; //goto next mode
}
//hall: resistance selected, just update lcd
//format: 9999.9 fixed range
//rdt:
//format:
char mode_3(int increment)
{
        float current;
        float voltage;
        float resistance;

        // [needed] fix adc channels
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / 4096 ) / CAL_SHUNT_RESISTOR );
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VR) ) / 4096 );
        voltage /= pga_vr.GetSetGain(); //compensate for PGA gain
        voltage /= CAL_FIXED_GAIN_VRES; //compensate for INSTR-AMP gain
        resistance = voltage / current;

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(resistance);
        floating_part = ((resistance - integer_part)*10);
        char sign;
        if (resistance > 0)
                sign = ' ';
        else
        {
                integer_part = -integer_part;
                floating_part = -floating_part;
                sign = '-';
        }

        //[needed] check and fix format
        char lcd_string[9];
        sprintf(lcd_string, "%c%4d.%01d", sign, integer_part, floating_part);

        //hmi.Write(3, lcd_string);
        // [needed] code print float
        return 3;
}
//hall: resistance gain selected, update resistance gain and LCD
//format: 1 to 200
//rdt:
//format:
char mode_4(int increment)
{
        static unsigned int index = 0;
        index = (index + increment) % 8;

        pga_vr.Set(char (index), 0);

        char lcd_string[9];
        sprintf(lcd_string, "%d", pga_vr.GetSetGain() );
        //hmi.Write(4, lcd_string);

        return 4;
}
//hall: temperature selected, update LCD
//format: -250 to +250 (celsiuls degrees)
//rdt:
//format:
char mode_5(int increment)
{
        float voltage;
        float temperature;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / 4096 );
        temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(temperature);
        floating_part = ((temperature - integer_part)*100);

        char sign;
        if (temperature > 0)
                sign = ' ';
        else
        {
                integer_part = -integer_part;
                floating_part = -floating_part;
                sign = '-';
        }

        char lcd_string[9];
        sprintf(lcd_string, "%c%2d.%02d", sign, integer_part, floating_part);

        //hmi.Write(5, lcd_string);
        return 5;
}
//hall: heating element power selected, update it and LCD
//format 100% or ERR
//rdt:
//format:
char mode_6(int increment)
{


        static unsigned int power_percentage = 0;
        float temperature;

        power_percentage = (power_percentage + increment) % 100;


        // [needed] fix adc channels
        // [needed] change with precalculated LSBs value

        float voltage;
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / 4096 );
        temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        char lcd_string[9];
        if ( temperature < CAL_TEMPERATURE_OVERHEAT_LIMIT ) //if temperature normal
        {
                power_percentage += ( increment % 100 );
                if (power_percentage >100)
                        power_percentage = 100;
                else if (power_percentage < 0)
                        power_percentage = 0;
                sprintf(lcd_string, "%d%%", power_percentage);
                analogWrite(_pin_heater, char (power_percentage*2.55));
        }
        else //IT'S ALL BURNING TO FLAMESSSSS
        {
                // [needed] buzz

                for(char i=0; i>50 && (digitalRead(_pin_heater) == LOW ); i++) //really shut heater down
                {
                        digitalWrite(_pin_heater, LOW);
                        analogWrite(_pin_heater, 0);
                }

                power_percentage = 0;
                sprintf(lcd_string, "%s", "ERR ");
        }

        //hmi.Write(6, lcd_string);
        return 6;
}
//hall: hall voltage selected, update LCD
//format: +99.999mV fixed range
//rdt:
//format:
char mode_7(int increment)
{
        float voltage;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VH) ) / 4096 ); //voltage in the adc input
        voltage -= CAL_HALL_ZERO_VOLTAGE;   //voltage output relative to 2.5V ground
        voltage /= pga_vh.GetSetGain();     //compensate for PGA gain
        voltage /= CAL_FIXED_GAIN_VHALL;    //compensate for INSTR-AMP gain

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(voltage * 1000);
        floating_part = ((voltage * 1000 - integer_part) * 1000);

        char sign;
        if (voltage > 0)
                sign = ' ';
        else
        {
                integer_part = -integer_part;
                floating_part = -floating_part;
                sign = '-';
        }

        char lcd_string[9];
        sprintf(lcd_string, "%c%2d.%03dmV", sign, integer_part, floating_part);

        //hmi.Write(7, lcd_string);

        return 7;
}
//hall: hall gain selected, update value and LCD
//format: from 1 to 200
//rdt:
//format:
char mode_8(int increment)
{
        static unsigned int index = 0;
        index = (index + increment) % 8;

        pga_vh.Set((char) index, 0);

        char lcd_string[9];
        sprintf(lcd_string, "%d", pga_vh.GetSetGain() );

        //hmi.Write(8, lcd_string);

        return 8;
}

void loop()
{

        if(! MAIN_DEBUG) //debugging stuff...
        {
                static char mode = 0;
                int16_t encoder_notches = 0;
                static unsigned long int cycles = 0; //cycles of loop since the apparatus has been powered

                //parse the button of the encoder user input
                ClickEncoder::Button b = encoder->getButton(); //b is button status
                if(b != ClickEncoder::Open) //if the button has been pressed
                {
                        switch (b) {
                        case ClickEncoder::Pressed:
                        case ClickEncoder::Clicked:
                                if (mode > 0 && mode < 8) // 8>mode>0
                                        mode++;
                                else //mode is either 8 and need to rotate to zero or fucked up
                                        mode = 1;
                                break;
                        case ClickEncoder::Held:
                                //nothing to do, really
                                break;
                        case ClickEncoder::Released:
                                //nothing to do, really
                                break;
                        case ClickEncoder::DoubleClicked:
                                if (mode > 0 && mode < 7) // 8>mode>=0
                                        mode+=2;
                                else if (mode >= 7 && mode <= 8) //rotate
                                        mode =2 - (8-mode );
                                else //fucked up
                                        mode = 1;
                                break;
                        }
                }

                //number of rotations of the encoder
                encoder_notches = encoder->getValue();

                //call the mode subroutine, pass the rotation of the encoder
                switch (mode)
                {
                case 1:
                        mode = mode_1(encoder_notches);
                        break;
                case 2:
                        mode = mode_2(encoder_notches);
                        break;
                case 3:
                        mode = mode_3(encoder_notches);
                        break;
                case 4:
                        mode = mode_4(encoder_notches);
                        break;
                case 5:
                        mode = mode_5(encoder_notches);
                        break;
                case 6:
                        mode = mode_6(encoder_notches);
                        break;
                case 7:
                        mode = mode_7(encoder_notches);
                        break;
                case 8:
                        mode = mode_8(encoder_notches);
                        break;
                default:
                        mode = 1; //shit happens
                        break;
                }

                if (   ( (cycles % 1000) == 0 )  &&  ( encoder_notches == 0 ) )
                {
                        //every now and then just update the display
                        //if no user interaction has occurred

                        mode_1(0);
                        mode_2(0);
                        mode_3(0);
                        mode_4(0);
                        mode_5(0);
                        mode_6(0);
                        mode_7(0);
                        mode_8(0);

                }

                encoder_notches = 0;

        }

        #ifdef MAIN_DEBUG //DEBUG START

        Serial.begin(9600);

        while(true)
        {
          Serial.println(adc.read(1));
          delay(100);
        }

        #endif //DEBUG END
}
