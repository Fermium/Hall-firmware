#include <Pga.h>
#include <MCP3304.h>
#include <hmi_abstraction.h>
#include <ClickEncoder.h>
#include <math.h>
#include <TimerOne.h>         // [needed] change with something that is not CC-BY
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

//please use the tag  [needed] for code that need to be fixed
// [needed] move encoder inside hmi

/* DISPLAY FORMAT
   12.34  mA||Fermium
   1234.45 O||Vr G=200
   -123.3 °c||P   100%
   12.34  mV||Vh G=200 */

//1 current     - 2 "Fermium"
//3 resistance  - 4 resistance gain
//5 temperature - 6 power to the heating element
//7 Hall voltage- 8 Hall Gain

//#define APPARATUS_RDT
#define APPARATUS_HALL


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
const PROGMEM float CAL_TEMPERATURE_ZERO_VOLT        = 2.5;
const PROGMEM float CAL_TEMPERATURE_VOLTAGE_GAIN     = 0.01; // mV/°C
const PROGMEM int CAL_TEMPERATURE_OVERHEAT_LIMIT     = 150;      // °C
const PROGMEM float CAL_SHUNT_RESISTOR               = 100.0;
const PROGMEM float CAL_VOLTAGE_REFERENCE            = 5.0;  //adc voltage reference
const PROGMEM float CAL_FIXED_GAIN_VRES              = 0.5;  //Opamp gain * voltage divider gain, from 0.1 to 999
const PROGMEM float CAL_FIXED_GAIN_VHALL             = 1.0;  //Opamp gain * voltage divider gain, from 0.1 to 999
const PROGMEM float CAL_HALL_ZERO_VOLTAGE            = 2.5;
const PROGMEM char SAMPLE_TYPE[5]                    = "Ge P";
const PROGMEM unsigned int ADC_RESOLUTION            = 4096;

//pin hall/rdt
#define PIN_HEATER 5

#endif

HMI_abstraction hmi; //hmi is a wrapper around the LCD library
ClickEncoder *encoder;

//check and shutdown if temperature is overlimit
//return false if temperature is ok, true if is overlimit
//####### DO NOT TOUCH ! YOU RISK DAMAGING THE INSTRUMENT ! #######
char overtemp()
{
        float voltage;
        float temperature;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        if ( temperature >= CAL_TEMPERATURE_OVERHEAT_LIMIT ) //If overheating
        {
                for(char i=0; i>50 && (digitalRead(PIN_HEATER) == LOW ); i++) //really shut heater down
                {
                        digitalWrite(PIN_HEATER, LOW);
                        analogWrite(PIN_HEATER, 0);
                }
                hmi.Buzzer(true);
                return true;
        }
        else
        {
                hmi.Buzzer(false);
                return false;
        }
}

//periodic ISR called every 1ms
//has to be kept very fast and non-blocking
void timerIsr() {
        encoder->service(); //execute encoder stuff
        if ((millis() % 500) == 0)
                overtemp();
}

void setup_screen(char);

//int main(void)
void setup ()
{
        hmi.Begin();

        //software SPI
        pinMode(11, OUTPUT); //MOSI
        pinMode(12, INPUT);  //MISO
        pinMode(13, OUTPUT); //CLK

        delay(100);
        hmi.SplashScreen((char*)SAMPLE_TYPE);
        delay(2500);
        //MPC3304 is already initialized
        //PGAs are already initialized

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
//rdt:
//format:
char mode_1(int increment)
{
        //calculate current (yes, Ohm's law)
        float current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );

        unsigned int integer_part = trunc(current);
        unsigned int floating_part = ((current - integer_part)*100);

        char lcd_string[9];
        sprintf_P(lcd_string, PSTR("%d.%02d"), integer_part, floating_part);
        hmi.Clean(0,9,0);
        hmi.WriteString(0, 0, lcd_string);
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
//hall: resistance selected, just update lcd
//rdt:
//format:
char mode_3(int increment)
{
        // [needed] fix adc channels
        float current = ((( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_CURRENT) ) / ADC_RESOLUTION ) / CAL_SHUNT_RESISTOR );
        float voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VR) ) / ADC_RESOLUTION );
        voltage /= pga_vr.GetSetGain() * CAL_FIXED_GAIN_VRES; //compensate for PGA and OPAMP gain
        float resistance = voltage / current;

        //dec_prec is the precision range for the selected Gain. This Uber-cool formula that simone found gets the precision
        int dec_prec=floor(-log10(fabs(CAL_VOLTAGE_REFERENCE/(50*CAL_FIXED_GAIN_VRES*pga_vr.GetSetGain()))));
        unsigned int integer_part = trunc(resistance);
        unsigned int floating_part = ((resistance - integer_part)* pow(10,dec_prec) );

        //generate format for the next sprintf,example %d.%02d using the calculated number of decimals
        char format[10];
        sprintf_P(format,PSTR("%%d%c%%0%dd"), (dec_prec != 0) ? '.' : ' ', dec_prec);

        //write to the lcd
        char lcd_string[9];
        sprintf(lcd_string, format, integer_part, floating_part);
        hmi.Clean(0,9,1);
        hmi.WriteString (0,1, lcd_string);

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

        if (increment != 0)
                pga_vr.Set(char (index), 0);

        //calculate gain for display, in the format 999.9
        float gain = pga_vr.GetSetGain() * CAL_FIXED_GAIN_VRES;
        unsigned int gain_integer_part = trunc(gain);
        unsigned int gain_floating_part = ((gain - gain_integer_part) * 10);


        char lcd_string[9];
        sprintf_P(lcd_string, PSTR("%3d.%1d x"), gain_integer_part, gain_floating_part  );
        hmi.Clean(11,20,1);
        hmi.WriteString(11,1, lcd_string);
        return 4;
}
//hall: temperature selected, update LCD
//rdt:
//format:
char mode_5(int increment)
{
        static int index = 0; //0 is °c, 1 is °K, 2 is °F
        index = constrain( (index + increment), 0, 2);
        char unit[3] = { 'C', 'K', 'F' };

        float voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        float temperature_c = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        float temperature; //converted temperature
        switch (index)
        {
        case 0: //Celsius
                //everything is ok!
                break;
        case 1: //Kelvin
                temperature = temperature_c + 273.15 ;
                break;
        case 2: //Farenheit
                temperature = (temperature_c * 1.8 + 32.0);
                break;
        }

        unsigned int integer_part = trunc(temperature);
        unsigned int floating_part = ((temperature - integer_part) * 100);

        sprintf_P(temp_string_global_10chars, PSTR("%3d.%01d"), integer_part, abs(floating_part));
        hmi.Clean(0,9,2);
        hmi.WriteString(0,2, temp_string_global_10chars);
        char celsius[3];
        sprintf_P(celsius,PSTR("%c%c"),0b11011111, unit[index]);
        hmi.WriteString(7,2,celsius);

        return 5;
}
//hall: heating element power selected, update it and LCD
//rdt:
//format:
char mode_6(int increment)
{
        static int power_percentage = 0;
        power_percentage = constrain((power_percentage + increment), 0, 100);

        float voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_TEMP) ) / ADC_RESOLUTION );
        float temperature = (voltage - CAL_TEMPERATURE_ZERO_VOLT) /  CAL_TEMPERATURE_VOLTAGE_GAIN;

        char lcd_string[9];
        if (overtemp()) //EMERGENCY
        {
                power_percentage = 0;
                sprintf_P(lcd_string, PSTR("!! ERR !!"));
        }
        else
        {
                sprintf_P(lcd_string, PSTR("Pwr :%3d%%"), power_percentage);
        }

        char power_255 = constrain( (power_percentage * 2.55 ), 0, 255);

        analogWrite(PIN_HEATER, power_255);
        hmi.Clean(11,20,2);
        hmi.WriteString(11,2, lcd_string);
        return 6;
}
//hall: hall voltage selected, update LCD
//rdt:
//format:
char mode_7(int increment)
{
        float voltage;

        voltage = (( CAL_VOLTAGE_REFERENCE * adc.read(ADC_CHANNEL_VH) ) / ADC_RESOLUTION ); //voltage in the adc input
        voltage -= CAL_HALL_ZERO_VOLTAGE;   //voltage output relative to 2.5V ground
        voltage /= pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL;     //compensate for PGA and OPAMP gain

        int dec_prec = floor(fabs(log10(((CAL_VOLTAGE_REFERENCE*1000.0)/(pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL*ADC_RESOLUTION)))));

        unsigned int integer_part = trunc(voltage );
        unsigned int floating_part= ((voltage  - integer_part) * pow(10,dec_prec));
        char lcd_string[9];
        char format[10];

        //generate format for the next sprintf, example %d.%02d using the calculated number of decimals
        sprintf_P(format,PSTR("%%d%c%%0%dd"),(dec_prec != 0) ? '.' : ' ', dec_prec);
        sprintf(lcd_string, format, integer_part, (abs(floating_part)));
        hmi.Clean(0,9,3);
        hmi.WriteString(0,3, lcd_string);
        hmi.WriteString(7,3,"mV");

        return 8;
}
//hall: hall gain selected, update value and LCD
//rdt:
//format:
char mode_8(int increment)
{
        static int index = 0;
        index = constrain( (index + increment), 0, 7);

        if (increment != 0)
                pga_vh.Set((char) index, 0);

        //calculate gain for display, in the format 999.9
        float gain = pga_vh.GetSetGain() * CAL_FIXED_GAIN_VHALL;
        unsigned int gain_integer_part = trunc(gain);
        unsigned int gain_floating_part = ((gain - gain_integer_part) * 10);

        char lcd_string[9];
        sprintf_P(lcd_string, PSTR("%3d.%d x"), gain_integer_part, gain_floating_part  );

        hmi.Clean(11,20,3);
        hmi.WriteString(11,3, lcd_string);

        return 8;
}

void loop()
{

        static char mode = 0;
        int16_t encoder_notches = 0;
        static unsigned long int cycles = 0;         //cycles of loop since the apparatus has been powered

        //parse the button of the encoder user input
        ClickEncoder::Button b = encoder->getButton();         //b is button status
        if(b != ClickEncoder::Open)         //if the button has been pressed
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

                mode = mode % 9;         //%9 because modes number starts from zero
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
                hmi.Update();
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
                hmi.Update();
                break;
        default:
                mode = 0;         //Just initialize screen and wait
                hmi.Update();
                break;
        }

        if ( (cycles % 1000) == 0  )
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
                hmi.Update();

                //clear display every few minutes
                if (cycles % 60000L == 0)
                        hmi.ForceRewrite();
        }

        encoder_notches = 0;

}

void setup_screen(char selection){
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
                        sprintf_P(temp, PSTR("%c"), 0b01111111);
                        hmi.WriteString(CENTER_LEFT, selection/2,temp);
                }
                else{
                        sprintf_P(temp, PSTR("%c"), 0b01111110);
                        hmi.WriteString(CENTER_RIGHT, (selection-1)/2,temp);
                }
        }
}
