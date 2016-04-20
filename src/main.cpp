//TODO: verificare valore adc < 4050 per resistenza e Vhall

/*
Experiment type
*/
#define HALL


/*
Calibration values for Hall Effect Experiment
*/
#ifdef HALL
#include <calibration_hall.h>
#endif


/*
Calibration values for RDT Experiment
*/
#ifdef RDT
#include <calibration_rdt.h>
#endif

/*
Standard C++ libraries includes
*/
#include <math.h> 


/*
Other libraries includes
*/
#include <Pga.h>                   // PGA library
#include <MCP3304.h>               // ADC library
#include <hmi_abstraction.h>       // LCD Monitor Abstraction library
#include <ClickEncoder.h>          // Clickable Encoder library
#include <TimerOne.h>              // TODO: change with something that is not CC-BY
//#include <LiquidCrystal.h>         // LCD Monitor library  To Be Deleted probably
#include <K_Thermocouple_AD8435.h> // Thermocouple Values Correction library
#include <avr/pgmspace.h>          // 


/*
 PIN | ATMEGA328 | 
 19  |    PC5    |
  8  |    PB0    |
  7  |    PD7    |
  9  |    PB1    |
 
*/

//initialize PGAs
MCP3304 adc(19);   //atmega328 PC5

//initialize pga
PGA113 pga_vh(8);   //atmega328 PB0
PGA113 pga_vr(7);   //atmega328 PD7
//PGA113 pga_3(9);    //atmega328 PB1

//1 current     - 2 "Fermium"
//3 resistance  - 4 resistance gain
//5 temperature - 6 power to the heating element
//7 Hall voltage- 8 Hall Gain

HMI_abstraction hmi; //hmi is a wrapper around the LCD library
ClickEncoder *encoder;

//global variables:
char temp_string_10chars[10];
void debug(char*);
//check and shutdown if temperature is overlimit
//return false if temperature is ok, true if is overlimit
//####### DO NOT TOUCH ! YOU RISK DAMAGING THE INSTRUMENT ! #######
char overTemp()
{
        float voltage;
        float voltaget;
        float temperature;
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        voltaget=thermocouple_voltage(voltage,CAL_TEMPERATURE_ZERO_VOLT);
        temperature = lin_extrap_temp(voltaget);
        if ( temperature >= CAL_TEMPERATURE_OVERHEAT_LIMIT ) { //If overheating
                        

                digitalWrite(PIN_HEATER, LOW);
                analogWrite(PIN_HEATER, 0);
                return true;
        }
        else {
                return false;
        }
        
}

//periodic ISR called every 1ms
//has to be kept very fast and non-blocking
void timerIsr() {
        encoder->service(); //execute encoder stuff
}

void setupScreen(char);

//int main(void)
void setup ()
{
        hmi.Begin();

        //software SPI
        pinMode(11, OUTPUT); //MOSI
        pinMode(12, INPUT);  //MISO
        pinMode(13, OUTPUT); //CLK

        delay(100);
        hmi.SplashScreen();
        delay(2500);

        //MPC3304 is already initialized
        //PGAs are already initialized

        //interrupt for the encoder reading and other useful stuff
        Timer1.initialize(1000);
        Timer1.attachInterrupt(timerIsr);

        //allocate and initialize encoder library
        encoder = new ClickEncoder(4, 3, 14); //not really a fan of new...
        encoder->setAccelerationEnabled(true); //enable cool acceleration feeling

        setupScreen(0); //reset vertical slashes
}

/* MODES
   each mode return the next mode. it usually is itself, but can be
   another one to jump in the menu
   Every mode receive in input the number of "notches" from the encoder
 */

//hall: current mode, just update lcd.
//rdt:
//format:
char mode_1(int increment)
{
        //calculate current (yes, Ohm's law)
        float current;
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT,true) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );
        unsigned int adc_read;
        adc_read=adc.read(ADC_CHANNEL_CURRENT,true);
        current *= 1000; //Current is now in mA, not Amps
        if(adc_read>ADC_OVERLOAD_VALUE_LSB_MAX || adc_read<ADC_OVERLOAD_VALUE_LSB_MIN) {
                hmi.Clean(0,9,0);
                hmi.WriteString(0,0,"Overload");
                return 5;
        }
        //split floating number into separated integer and floating part
        unsigned int integer_part;
        integer_part = trunc(current);
        unsigned int floating_part;
        floating_part = ((current - integer_part)*100);

        sprintf_P(temp_string_10chars, PSTR("%d.%02d"), integer_part, floating_part);
        hmi.Clean(0,9,0);
        hmi.WriteString(0, 0, temp_string_10chars);
        hmi.WriteString(7,0,"mA");

        return 2; //jump to next mode
}
//hall: Show manufacturer name
//rdt:
char mode_2(int increment)
{
        hmi.WriteString(11,0,"Fermium");

        return 3; //jump to next mode
}
//write in a little empty place in the corner of the LCD
void debug(char* debug_msg){
        hmi.Clean(11,20,0);
        hmi.WriteString(11,0,debug_msg);
}
//hall: resistance selected, just update lcd
//rdt:
//format:
char mode_3(int increment)
{
        float voltage;
        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VR,true) ) / ADC_RESOLUTION );
        voltage /= pga_vr.GetSetGain() * CAL_FIXED_GAIN_VRES; //compensate for PGA and OPAMP gain
        unsigned int adc_read;
        adc_read=adc.read(ADC_CHANNEL_VR,true);
        float current;
        current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT,true) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );

        float resistance;
        resistance = voltage / current;

        //calculate the required number of decimal digits
        int dec_prec;
        dec_prec = floor( -log10( fabs( CAL_VOLTAGE_REFERENCE / ( 50 * CAL_FIXED_GAIN_VRES * pga_vr.GetSetGain() ))));
        if(adc_read>ADC_OVERLOAD_VALUE_LSB_MAX || adc_read<ADC_OVERLOAD_VALUE_LSB_MIN) {
                hmi.Clean(0,9,1);
                hmi.WriteString(0,1,"Overload");
                return 4;
        }
        //split floating number into separated integer and floating part
        
        unsigned int integer_part;
        integer_part = trunc(resistance);
        unsigned int floating_part;
        floating_part = ((resistance - integer_part)* pow(10, dec_prec) );

        //generate format for the next sprintf,example %d.%02d using the calculated number of decimals
        char format[10];
        if(dec_prec!=0) {
                sprintf_P(format, PSTR("%%d.%%0%dd"), dec_prec);
                
                sprintf(temp_string_10chars, format, integer_part, floating_part);
        }
        else {
                sprintf(temp_string_10chars, "%d", integer_part);
        }

        //write to the lcd
        hmi.Clean(0,9,1);
        hmi.WriteString (0,1, temp_string_10chars);

        //write the OMEGA character
        char ohm[2];
        sprintf_P(ohm,PSTR("%c"),0b11110100);
        hmi.WriteString(8,1,ohm);

        return 4; //jump to the next mode
}
//hall: resistance gain selected, update resistance gain and LCD
//rdt:
//format:
char mode_4(int increment)
{
        static int index = 0;
        index = constrain( (index + increment), 0, 7);

        //update PGA gain
        //if (increment != 0)
        pga_vr.Set(char (index));

        //calculate gain for display, in the format 999.9
        float gain;
        gain = pga_vr.GetSetGain() * CAL_FIXED_GAIN_VRES;

        //split floating number into separated integer and floating part
        unsigned int gain_integer_part;
        gain_integer_part = trunc(gain);
        unsigned int gain_floating_part;
        gain_floating_part = ((gain - gain_integer_part) * 10);

        sprintf_P(temp_string_10chars, PSTR("Gr%4d.%1dx"), gain_integer_part, gain_floating_part  );
        hmi.Clean(11,20,1);
        hmi.WriteString(11,1, temp_string_10chars);
        return 4;
}
//hall: temperature selected, update LCD and change the unit
//rdt:
//format:
char mode_5(int increment)
{

        static int index = 0; //0 is °c, 1 is °K, 2 is °F
        index = (index+increment==-1) ? 2 : (index + increment);
        char unit[3] = { 'C', 'K', 'F' };


        int adc_read;
        adc_read = adc.read(ADC_CHANNEL_TEMP,false);


        float voltage;
        float voltaget;
        float temperature_c;
        voltage = ((  CAL_VOLTAGE_REFERENCE *  adc_read) /  ADC_RESOLUTION );
        voltaget=thermocouple_voltage(voltage,CAL_TEMPERATURE_ZERO_VOLT);
        temperature_c = lin_extrap_temp(voltaget);


        //convert temperature
        float temperature;
        switch (index%3) {

        case 0: //Celsius
                temperature = temperature_c;
                break;
        case 1: //Kelvin
                temperature = temperature_c + 273.15;
                break;
        case 2: //Farenheit
                temperature = (temperature_c * 1.8 + 32.0);
                break;
        }

        if(adc_read>ADC_OVERLOAD_VALUE_LSB_MAX || adc_read<ADC_OVERLOAD_VALUE_LSB_MIN) {
                hmi.Clean(0,9,2);
                hmi.WriteString(0,2,"Overload");
                return 5;
        }
        //split floating number into separated integer and floating part
        int integer_part;
        integer_part = temperature;
        int floating_part;
        floating_part =   abs((temperature - integer_part) * 10);

        sprintf_P(temp_string_10chars, PSTR("%d.%01d"), integer_part, abs(floating_part));
        hmi.Clean(0,9,2);
        hmi.WriteString(0,2, temp_string_10chars);
        char celsius[3];
        sprintf_P(celsius,PSTR("%c%c"),0b11011111, unit[index%3]);
        hmi.WriteString(7,2,celsius);

        return 5;
}
//hall: heating element power selected, update it and LCD
//rdt:
//format:
char mode_6(int increment)
{
        static int power_percentage = 0;
        power_percentage = constrain((power_percentage + increment), 0, 1);

        if (overTemp()) { //EMERGENCY

                power_percentage = 0;
                sprintf_P(temp_string_10chars, PSTR("!! ERR !!"));
        }
        else {
                if(power_percentage){
                sprintf_P(temp_string_10chars, PSTR("ON"));        
                }
                else{
                sprintf_P(temp_string_10chars, PSTR("OFF"));        
                }
                
        }

        char power_255;
        power_255 = constrain( (power_percentage * 255 ), 0, 255);

        analogWrite(PIN_HEATER, power_255);
        hmi.Clean(11,20,2);
        hmi.WriteString(11,2, temp_string_10chars);
        return 6;
}
//hall: hall voltage selected, update LCD
//rdt:
//format:
char mode_7(int increment)
{
        float voltage;

        char teemp[10];
        int tempreading;
        tempreading = adc.read(ADC_CHANNEL_VH,true);

        voltage = (( CAL_VOLTAGE_REFERENCE * (float)tempreading ) / ADC_RESOLUTION );//voltage in the adc input
        voltage -= CAL_HALL_ZERO_VOLTAGE;
        voltage /= pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL;     //compensate for PGA and OPAMP gain
        voltage *= 1000; //show mV

        //calculate the required number of decimal digits
        int dec_prec;
        dec_prec = (int)(floor(fabs(log10(((CAL_VOLTAGE_REFERENCE*1000.0)/(pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL*ADC_RESOLUTION))))));
        //dec_prec=2;
        if(tempreading>ADC_OVERLOAD_VALUE_LSB_MAX || tempreading<ADC_OVERLOAD_VALUE_LSB_MIN) {
                hmi.Clean(0,9,3);
                hmi.WriteString(0,3, "Overload");
                return 8;
        }
        //split floating number into separated integer and floating part
        char sgn=(voltage<0)?-1:1;
        unsigned int integer_part;
        integer_part = (int)(sgn*trunc(voltage ));
        
        unsigned int floating_part;
        floating_part=(int) abs((voltage  - sgn*(float)(integer_part)) * pow(10, dec_prec));
        
        char format[10];
        //generate format for the next sprintf, example %d.%02d using the calculated number of decimals
        if(dec_prec!=0) {
                sprintf_P(format, PSTR("%%d.%%0%dd"), dec_prec);
                if(sgn<0){
                        sprintf_P(format, PSTR("%c%%d.%%0%dd"),'-', dec_prec);
                        
                }
                sprintf(temp_string_10chars, format, integer_part, floating_part);
        }
        else {
                sprintf_P(temp_string_10chars, PSTR("%d"), sgn*integer_part);
        }

        hmi.Clean(0,9,3);
        hmi.WriteString(0,3, temp_string_10chars);
        hmi.WriteString(7,3,"mV");

        return 8;
}
//hall: hall gain selected, update value and LCD
//rdt:
//format:
char mode_8(int increment)
{
        static bool firstStart = true;

        static int index = 1;
        index = constrain( (index + increment), 0, 7);

        //Update PGA gain
        if (increment != 0 || firstStart)
        {
                pga_vh.Set((char) index);
                pga_vh.Set((char) index);
                pga_vh.Set((char) index);
                pga_vh.Set((char) index);
                pga_vh.Set((char) index);

        }


        //calculate gain for display, in the format 999.9
        float gain;
        gain = pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL;

        //split floating number into separated integer and floating part
        unsigned int gain_integer_part;
        gain_integer_part = trunc(gain);
        unsigned int gain_floating_part;
        gain_floating_part = ((gain - gain_integer_part) * 10);

        sprintf_P(temp_string_10chars, PSTR("Gh%4d.%dx"), gain_integer_part, gain_floating_part  );

        hmi.Clean(11,20,3);
        hmi.WriteString(11,3, temp_string_10chars);
        if(firstStart)
                firstStart=false;
        return 8;
}

void loop()
{
        hmi.Buzzer(true);
        static char mode = 0;
        int16_t encoder_notches = 0;
        static unsigned long int cycles = 0;         //cycles of loop since the apparatus has been powered


        //parse the button of the encoder user input
        ClickEncoder::Button b = encoder->getButton();         //b is button status
        if(b != ClickEncoder::Open) {        //if the button has been pressed

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

                mode = mode % 9;         //%9 because modes number starts from zero
        }

        //number of rotations of the encoder
        encoder_notches = -encoder->getValue();
        if((cycles%3000000L)==100){
                hmi.ForceRewrite();
                
                
        }
        setupScreen(mode); //draw center symbols of the select mode

        //call the mode subroutine, pass the rotation of the encoder
        switch (mode) {
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
                hmi.Update();
                break;
        case 5:
                mode = mode_5(encoder_notches);
                hmi.Update();
                break;
        case 6:
                mode = mode_6(encoder_notches);
                hmi.Update();
                break;
        case 7:
                mode = mode_7(encoder_notches);
                break;
        case 8:
                mode = mode_8(encoder_notches);
                hmi.Update();
                break;
        case 0:
        default:
                mode = 0;         //Just initialize screen and wait
                break;
        }

        //check for overTemperature
        if ( (cycles % 1000L) == 0  ) {
                overTemp();
        }
        
        //update LCD
        if ( (cycles % 2500L) == 0  ) {
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
                hmi.Update();
        }

        encoder_notches = 0;
        cycles++;
}

//draw center graphics
void setupScreen(char selection)
{

        //basic string to repeat vertically
        char tmp[3];
        sprintf(tmp,"%c%c",0b11111111,0b11111111);

        for(unsigned char i=0; i<4; i++)
                hmi.WriteString((hmi.GetLenght() / 2) - 1,i,tmp);

        if(selection!=0) {
                //right arrow
                if(selection%2!=0) {
                        sprintf_P(temp_string_10chars, PSTR("%c"), 0b01111111); //left arrow
                        hmi.WriteString((hmi.GetLenght() / 2) - 1, selection / 2, temp_string_10chars);
                }
                //left arrow
                else{
                        sprintf_P(temp_string_10chars, PSTR("%c"), 0b01111110); //right arrow
                        hmi.WriteString((hmi.GetLenght() / 2), (selection-1) / 2, temp_string_10chars);
                }
        }
}
