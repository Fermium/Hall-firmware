/*
   Calibration Values for Hall Effect
   Written by: Davide Bortolami
*/


#define APPARATUS_HALL


/*
  ADC Channels Definitions
*/
#define ADC_CHANNEL_VH            4
#define ADC_CHANNEL_VR            3
#define ADC_CHANNEL_TEMP          0
#define ADC_CHANNEL_CURRENT       1
#define ADC_OVERLOAD_VALUE_LSB    4000


/*
  Sample Type Definition
*/
#define SAMPLE_TYPE               "Ge P"


/*
  Hall/RDT Heater Pin Definition
*/
#define PIN_HEATER 5


/*
  Center Calibration Values:
*/
float CAL_TEMPERATURE_ZERO_VOLT        = 2.5;     // Temperature offset for thermocopule (V)
float CAL_HALL_ZERO_VOLTAGE            = 2.5;


/*
  Temperature Calibration Values:
*/
float CAL_TEMPERATURE_VOLTAGE_GAIN     = 0.01;    // (V/°C)
int   CAL_TEMPERATURE_OVERHEAT_LIMIT   = 150;     // °C


/*
  Shunt Resistor Value:
*/
float CAL_SHUNT_RESISTOR               = 100.0;   // Shaunt Resistor value (Ohm)


/*
  Fixed Gains Values:
*/
float CAL_FIXED_GAIN_VRES              = 0.5;     // Opamp gain * voltage divider gain, from 0.1 to 999
float CAL_FIXED_GAIN_VHALL             = 1.0;     // Opamp gain * voltage divider gain, from 0.1 to 999


/*
  ADC calibration values:
*/
float CAL_VOLTAGE_REFERENCE            = 5.0;     // Adc voltage reference
unsigned int ADC_RESOLUTION            = 4096;
