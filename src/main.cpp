#include <Pga.h>
#include <MCP3304.h>
#include <HMI_abstraction.h>
#include <ClickEncoder.h>
#include <math.h>


//12.34  mA||
//1234.45 O||Vr G=200
//-123.3 °c||P   100%
//12.34   V||Vh G=200

//1 current     - 2 none
//3 resistance  - 4 resistance gain
//5 temperature - 6 power to the heating element
//7 Hall voltage- 8 Hall Gain

//#define apparatus_rdt
#define apparatus_hall


//initialize PGAs
PGA113 pga_vh(10);
PGA113 pga_vr(11);
PGA113 pga_3(12);
MCP3304 adc(13);
HMI_abstraction HMI();
ClickEncoder *encoder;

//encoder on pin 2,3



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

}

//MODES
//each mode return the next mode. it usually is itself, but can be
//another one to jump in the menu
//hall: current mode, just update lcd
char mode_1(int increment)
{
        float shunt_resistor = 100.0;
        float voltage_reference = 5.0;
        float current;

        //remember IT guis, viva il re' d'italia
        current = ((( 5.0 * adc.readSgl(0) ) / 4096 ) / shunt_resistor );

        char lcd_string[9];
        //#code print float
        HMI.Write(1, lcd_string);

        return 3;
}
//hall: nothing selected, nothing to do
char mode_2(int increment)
{
        return 3;
}
//hall: resistance selected, just update lcd
char mode_3(int increment)
{
        return 4;
}
//hall: resistance gain selected, update resistance gain and LCD
char mode_4(int increment)
{
        return 4;
}
//hall: temperature selected, update LCD and check foor overheating
char mode_5(int increment)
{
        return 6;
}
//hall: heating element power selected, update it and LCD
char mode_6(int increment)
{
        return 6;
}
//hall: hall voltage selected, update LCD
char mode_7(int increment)
{
        return 8;
}
//hall: hall gain selected, update value and LCD
char mode_8(int increment)
{
        return 8;
}

void loop()
{
        static char mode = 0;
        int16_t encoder_notches;
        unsigned long int cycles = 0; //cycles of loop since the apparatus has been powered

        ClickEncoder::Button b = encoder->getButton(); //b is button status
        if(b != ClickEncoder::Open) //if the button has been pressed
        {
                switch (b) {
                case ClickEncoder::Pressed:
                case ClickEncoder::Clicked:
                        if (mode > 0 && mode < 8)// 8>mode>0
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
                        if (mode > 0 && mode < 7)// 8>mode>=0
                                mode+=2;
                        else if (mode >= 7 && mode <= 8) //rotate
                                mode =2 - (8-mode );
                        else  //fucked up
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
        }

        if ((cycles % 1000) == 0 )
        {
                //every now and then just update the display
                mode_1(0);
                mode_2(0);
                mode_3(0);
                mode_4(0);
                mode_5(0);
                mode_6(0);
                mode_7(0);
                mode_8(0);
        }

}
