#include <Pga.h>
#include <MCP3304.h>
#include <hmi_abstraction.h>
#include <ClickEncoder.h>
#include <math.h>
#include <TimerOne.h>         // [needed] change with something that is not CC-BY
#include <LiquidCrystal.h>

//please use the tag  [needed] for code that need to be fixed
// [needed] move encoder inside hmi


/* DISPLAY FORMAT
   12.34  mA||
   1234.45 O||Vr G=200
   -123.3 °c||P   100%
   12.34  mV||Vh G=200 */

//1 current     - 2 none
//3 resistance  - 4 resistance gain
//5 temperature - 6 power to the heating element
//7 Hall voltage- 8 Hall Gain

//#define APPARATUS_RDT
#define APPARATUS_HALL
#define MAIN_DEBUG false


//Screen position define for easier usage
#define CENTER_LEFT 9
#define CENTER_RIGHT 10

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
//PGA113 pga_3(9);    //atmega328 PB1

//calibration values:
float CAL_TEMPERATURE_ZERO_VOLT        = 2.5;
float CAL_TEMPERATURE_VOLTAGE_GAIN     = 0.01; // mV/°C
int CAL_TEMPERATURE_OVERHEAT_LIMIT     = 150;      // °C
float CAL_SHUNT_RESISTOR               = 100.0;
float CAL_VOLTAGE_REFERENCE            = 5.0;  //adc voltage reference
float CAL_FIXED_GAIN_VRES              = 0.5;  //gain opamp e partitore sulla Vref
float CAL_FIXED_GAIN_VHALL             = 1.0;  //gain opamp sulla Vhall
float CAL_HALL_ZERO_VOLTAGE            = 2.5;
char SAMPLE_TYPE[5]                    = {'G', 'e', ' ', 'P', '\0'};
unsigned int ADC_RESOLUTION            = 4096;

//pin hall/rdt
#define _pin_heater 5


#endif


HMI_abstraction hmi; //hmi is a wrapper around the LCD library
ClickEncoder *encoder;

//check and shutdown if temperature is overlimit
//return false if temperature is ok, true if is overlimit
char overtemp()
{
        float voltage;
        float temperature;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        if ( temperature >= CAL_TEMPERATURE_OVERHEAT_LIMIT ) //if temperature normal
        {
                // [needed] buzz
                for(char i=0; i>50 && (digitalRead(_pin_heater) == LOW ); i++) //really shut heater down
                {
                        digitalWrite(_pin_heater, LOW);
                        analogWrite(_pin_heater, 0);
                }
                return true;
        }

        return false;
}

//periodic subroutine called every 1ms
void timerIsr() {
        static long unsigned int milliseconds = 0;

        milliseconds++;
        encoder->service(); //execute encoder stuff
        bool aaa = false;
        aaa = !aaa;
        //digitalWrite(A1, aaa);

        if ((milliseconds % 500) == 0)
        {
                overtemp();
        }

}

void setup_screen(int);

//int main(void)
void setup ()
{

        hmi.Begin();

        //software SPI
        pinMode(11, OUTPUT); //MOSI
        pinMode(12, INPUT);  //MISO
        pinMode(13, OUTPUT); //CLK

        delay(100);
        hmi.SplashScreen(SAMPLE_TYPE);
        delay(2500);
        //MPC3304 is already initialized
        //PGAs are already initialized

        //call the main loop
        //while(true)
        //    loop();
        //  return 1;

        //interrupt for the encoder reading and other useful stuff
        Timer1.initialize(1000);
        Timer1.attachInterrupt(timerIsr);

        encoder = new ClickEncoder(4, 3, 14); //not really a fan of new...
        encoder->setAccelerationEnabled(true); //enable cool acceleration feeling
        setup_screen(0); //reset vertical slashes


}


/* MODES
   each mode return the next mode. it usually is itself, but can be
   another one to jump in the menu
   Every mode receive in input the number of "notches" from the encoder
 */

//hall: current mode, just update lcd.
//format: 99.99mA fixed range
//rdt:
//format:
char mode_1(int increment)
{
        float current;
        //remember IT guys, viva il re' d'italia
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(current);
        floating_part = ((current - integer_part)*100);

        char lcd_string[9];
        sprintf(lcd_string, "%2d.%02d mA", integer_part, floating_part);
        hmi.RowClean(0,8,0);
        hmi.WriteString(0, 0, lcd_string);


        return 3;
}
//hall: nothing selected, nothing to do (maybe debug message? or some other kind of message? should think)
//rdt:
char mode_2(int increment)
{
        hmi.RowClean(11,20,1);
        hmi.WriteString(11, 0, " Fermium ");
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
        char format[10];

        // [needed] fix adc channels
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VR) ) / ADC_RESOLUTION );
        voltage /= pga_vr.GetSetGain(); //compensate for PGA gain
        voltage /= CAL_FIXED_GAIN_VRES; //compensate for INSTR-AMP gain
        resistance = voltage / current;

        unsigned int integer_part;
        unsigned int floating_part;

        /*
           prec is the precision range for the selected Gain. This Uber-cool formula that simone found gets the precision
         */
        int prec=ceil(-log10(fabs(CAL_VOLTAGE_REFERENCE/(50*CAL_FIXED_GAIN_VRES*pga_vr.GetSetGain()))));
        integer_part = trunc(resistance);
        floating_part = ((resistance - integer_part)*10);

        //sprintf creates the format string for the next sprintf with the right precision
        sprintf(format,"%%3d.%%0%dd %%c",prec);
        //[needed] check and fix format
        char lcd_string[9];
        // dah usual shit here
        sprintf(lcd_string, format, integer_part, floating_part, 0b11110100);
        hmi.RowClean(0,8,1);
        hmi.WriteString (0,1, lcd_string);
        mode_2(0);
        // [needed] code print float
        return 4;
}
//hall: resistance gain selected, update resistance gain and LCD
//format: 1 to 200
//rdt:
//format:
char mode_4(int increment)
{
        static int index = 0;
        index = constrain( (index + increment), 0, 7);


        pga_vr.Set(char (index), 0);

        char lcd_string[9];
        sprintf(lcd_string, "Vr G:%3dx", pga_vr.GetSetGain() );
        hmi.RowClean(11,20,1);
        hmi.WriteString(11,1, lcd_string);

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

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
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
        sprintf(lcd_string, "%c%3d.%02d%cC", sign, integer_part, floating_part,0b11011111);
        hmi.RowClean(0,8,2);
        hmi.WriteString(0,2, lcd_string);
        return 6;
}
//hall: heating element power selected, update it and LCD
//format 100% or ERR
//rdt:
//format:
char mode_6(int increment)
{
        static int power_percentage = 0;

        power_percentage = constrain((power_percentage + increment), 0, 100);

        float temperature;
        float voltage;
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        char lcd_string[9];
        if (overtemp()) //EMERGENCY
        {
                power_percentage = 0;
                sprintf(lcd_string, "%s", "!! ERR !!");
        }
        else
        {
                sprintf(lcd_string, "Pwr %3d %%", power_percentage);
        }

        char power_255 = power_percentage * 2.55;
        analogWrite(_pin_heater, power_255);
        hmi.RowClean(11,20,2);
        hmi.WriteString(11,2, lcd_string);
        return 6;
}
//hall: hall voltage selected, update LCD
//format: +99.999mV fixed range
//rdt:
//format:
char mode_7(int increment)
{
        float voltage;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VH) ) / ADC_RESOLUTION ); //voltage in the adc input
        voltage -= CAL_HALL_ZERO_VOLTAGE;   //voltage output relative to 2.5V ground
        voltage /= pga_vh.GetSetGain();     //compensate for PGA gain
        voltage /= CAL_FIXED_GAIN_VHALL;    //compensate for INSTR-AMP gain

        unsigned int integer_part;
        unsigned int floating_part;

        integer_part = trunc(voltage * 1000);
        floating_part = ((voltage * 1000 - integer_part) * 1000);

        char lcd_string[9];
        sprintf(lcd_string, "%2d.%01d mV", integer_part, abs(floating_part));
        hmi.RowClean(0,8,3);
        hmi.WriteString(0,3, lcd_string);

        return 8;
}
//hall: hall gain selected, update value and LCD
//format: from 1 to 200
//rdt:
//format:
char mode_8(int increment)
{
        static int index = 0;
        index = constrain( (index + increment), 0, 7);

        pga_vh.Set((char) index, 0);

        char lcd_string[9];
        sprintf(lcd_string, "Vh G:%3dx", pga_vh.GetSetGain() );
        hmi.RowClean(11,20,3);
        hmi.WriteString(11,3, lcd_string);

        return 8;
}

void loop()
{

        if(!MAIN_DEBUG)  //debugging stuff...
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
                                mode++;
                                break;
                        case ClickEncoder::Held:
                                //nothing to do, really
                                break;
                        case ClickEncoder::Released:
                                //nothing to do, really
                                break;
                        case ClickEncoder::DoubleClicked:
                                mode += 2;
                                break;
                        }

                        mode = mode % 9; //%9 because modes number starts from zero
                }

                //number of rotations of the encoder
                encoder_notches = -encoder->getValue();
                setup_screen(mode);
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
                        mode = 0; //Just initialize screen and wait
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
                        hmi.ForceRewrite();

                }

                encoder_notches = 0;

        }

        if( MAIN_DEBUG )
        {
                //DEBUG START
                //Serial.begin(9600);

                //hmi.WriteString(0,0,"aaa");
                hmi.Update();

                while(true)
                {


                }

        }
}

void setup_screen(int selection){
        unsigned char i=0;
        for(i; i<4; i++) {
                hmi.WriteString(CENTER_LEFT,i,"||");
        }

        char temp[2];

        if(selection==0) {
                mode_1(0);
                mode_2(0);
                mode_3(0);
                mode_4(0);
                mode_5(0);
                mode_6(0);
                mode_7(0);
                mode_8(0);
        }
        else{
                if(selection%2!=0) {
                        sprintf(temp, "%c", 0b01111111);
                        hmi.WriteString(CENTER_LEFT, selection/2,temp);
                }
                else{
                        sprintf(temp, "%c", 0b01111110);
                        hmi.WriteString(CENTER_RIGHT, (selection-1)/2,temp);
                }
        }

        hmi.Update();
}
