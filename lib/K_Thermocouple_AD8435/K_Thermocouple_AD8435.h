#include <math.h>
#define AD8495

#ifdef AD8495

#define THERMOCOUPLE_OFFSET 0.125
#define THERMOCOUPLE_GAIN 122.4
#endif
float b_neg[5]={0,2.5173462e1,-1.1662878,-1.0833638,-8.9773540e-1};
float b_pos[5]={0,2.508355e1,7.860106e-2,-2.503131e-1,8.315270e-2};

float lin_extrap_temp(float E,int precision=5){
  int prec=constrain(0,precision,5);
  float t=0;
  for(int i=0;i<precision;i++){
    t+=((E<0)?b_neg[i]*pow(E,i):b_pos[i]*pow(E,i));
  }
  return t;
}

float thermocouple_voltage(float vout,float vref){
  return (1/2*vout-vref-THERMOCOUPLE_OFFSET)/THERMOCOUPLE_GAIN;
}
