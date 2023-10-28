#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "fix16.h"

/* *
 * The MAX31855 uses a simple linear approximation to convert thermocouple voltage to
 * temperature. This is reasonably accurate in the range 0-300 degrees C but is very
 * inaccurate for negative temperatures and above 500C. To correct for this we can use the
 * published linear approximations for both the hot and cold junctions to calculate the
 * actual thermocouple junction voltages and then apply the NIST polynomials which give
 * an ~+/-0.05 degree C error. In practise this error will be swamped ADC quantisation
 * errors.
 * This function outputs a headerfile onto stdout which contains a lookup tables so you can
 * select the correct index based on a coldJunctTemperature and get a fix16_t representation 
 * of the Cold Junction Voltage.
 * This can then be subtracted from the measured voltage to obtain a thermocouple voltage 
 * which can be used with the temperature table to find the hot junction temperature.
 * temperature. 
 * Linear interpolation can be implemented for additional accuracy in both tables.
 *
 *  Get MAX31855 cold_temp & hot_temp
 *  Use table to lookup cold_junction_voltage
 *  Calculate hot_junction_voltage = (t_hot - t_cold) * 0.041276
 *  Calculate measured_voltage = cold_junction_voltage + hot_junction_voltage
 *  Use table to lookup corrected_hot_temp
 *
 * */

  // the three polynomials below are for -200-0, 0-500 and 500-1372 degrees C
  const double polynomial[][10] = {{
    0.0000000E+00,   2.5173462E+01,
    -1.1662878E+00,  -1.0833638E+00,
    -8.9773540E-01,  -3.7342377E-01,
    -8.6632643E-02,  -1.0450598E-02,
    -5.1920577E-04,  0.0000000E+00
  },{
    0.000000E+00,    2.508355E+01,
    7.860106E-02,    -2.503131E-01,
    8.315270E-02,    -1.228034E-02,
    9.804036E-04,    -4.413030E-05,
    1.057734E-06,    -1.052755E-08
  },{
    -1.318058E+02,   4.830222E+01,
    -1.646031E+00,   5.464731E-02,
    -9.650715E-04,   8.802193E-06,
    -3.110810E-08,   0.000000E+00,
    0.000000E+00,    0.000000E+00
  }};

  double c[][11] = {{
    0.000000000000E+00,  0.394501280250E-01,  
    0.236223735980E-04,  -0.328589067840E-06, 
    -0.499048287770E-08, -0.675090591730E-10, 
    -0.574103274280E-12, -0.310888728940E-14, 
    -0.104516093650E-16, -0.198892668780E-19, 
    -0.163226974860E-22
  },{
    -0.176004136860E-01, 0.389212049750E-01,  
    0.185587700320E-04,  -0.994575928740E-07,  
    0.318409457190E-09,  -0.560728448890E-12,  
    0.560750590590E-15,  -0.320207200030E-18,  
    0.971511471520E-22,  -0.121047212750E-25,
    0.0
  }};

  // Exponential coefficients. Only used for positive temperatures.
  double a0 = 0.118597600000E+00;
  double a1 = -0.118343200000E-03;
  double a2 = 0.126968600000E+03;


int main (void)
{
   printf("#ifndef NIST_H\n#define NIST_H\n\n");
   printf("#include \"fix16.h\"\n\n");  
   uint8_t index = 0;
  
  /* Not needed as our temperature range for the cold junction is small and we can just
   * use a linear fit. Left here in case we need to support cold junctions over a larger
   * temperature range.
   *
   *  const double tCMin = -40;
   *  const double tCMax = 80;
   *  const double tCInc = 0.625;
   *
   *
   *  
   *  for (double coldJunctTemp = tCMin; coldJunctTemp < tCMax; coldJunctTemp += tCInc) {
   *    double coldJunctVoltage = coldJunctTemp * 0.04073;
   *    index = coldJunctVoltage <= 0 ? 0 : 1; 
   *    for (uint8_t i = 0; i < 11; i++) {
   *      coldJunctVoltage += (c[index][i] * pow(coldJunctTemp, i));
   *    }
   *  
   *    // This section adds the a0 exp(a1 (t - a2)^2) components.
   *    if (coldJunctTemp > 0) {  
   *      coldJunctVoltage += a0 * exp(a1 * pow(coldJunctTemp - a2, 2));
   *    }
   *
   *  }
  */
  const double vMin = -6.5;
  const double vMax = 56;
  const double vInc = 0.1;

  printf("#define LOOKUP_VMIN 0x%08X    /* %.2fmV */\n", F16(vMin), vMin);
  printf("#define LOOKUP_VMAX 0x%08X    /* %.2fmV */\n", F16(vMax), vMax);
  printf("#define LOOKUP_VSTEP 0x%08X   /* %.2fmV */\n\n", F16(1/vInc), vInc);

  printf("// Table temperatures run from %.2fmV to %.2fmV in %.2fmV steps\n", vMin, vMax, vInc);
  printf("fix16_t temperatures[]={\n");

  for (double measuredVoltage = vMin; measuredVoltage < vMax; measuredVoltage += vInc) {
  
    index = measuredVoltage <= 0 ? 0 : measuredVoltage <= 20.644 ? 1 : 2;
    double compensatedTemp = 0;
    for (uint8_t term = 0; term < 10; term++) {
      compensatedTemp += (polynomial[index][term] * pow(measuredVoltage, term));
    }
    printf("0x%08X, /* %.2fmV = %.2f degrees C */\n", F16(compensatedTemp), measuredVoltage, compensatedTemp);
  }

  printf("};\n#endif //NIST_H\n");
  return 0;
}



