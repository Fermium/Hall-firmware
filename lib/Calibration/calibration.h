#define APPARATUS_HALL

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
