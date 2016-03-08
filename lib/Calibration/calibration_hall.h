#define APPARATUS_HALL

//adc channel definitions definitions
#define ADC_CHANNEL_VH      4
#define ADC_CHANNEL_VR      3
#define ADC_CHANNEL_TEMP    0
#define ADC_CHANNEL_CURRENT 1
#define ADC_OVERLOAD_VALUE_LSB 4000


//calibration values:
float CAL_TEMPERATURE_ZERO_VOLT        = 2.5;
float CAL_TEMPERATURE_VOLTAGE_GAIN     = 0.01;    // V/°C
int   CAL_TEMPERATURE_OVERHEAT_LIMIT   = 150;           // °C
float CAL_SHUNT_RESISTOR               = 100.0;
float CAL_VOLTAGE_REFERENCE            = 5.0;     //adc voltage reference
float CAL_FIXED_GAIN_VRES              = 0.5;     //Opamp gain * voltage divider gain, from 0.1 to 999
float CAL_FIXED_GAIN_VHALL             = 1.0;     //Opamp gain * voltage divider gain, from 0.1 to 999
float CAL_HALL_ZERO_VOLTAGE            = 2.5;
#define SAMPLE_TYPE                    "Ge P"
unsigned int ADC_RESOLUTION            = 4096;

//pin hall/rdt
#define PIN_HEATER 5
