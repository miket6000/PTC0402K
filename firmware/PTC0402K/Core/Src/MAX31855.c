#include "MAX31855.h"
#include <stdint.h>
//#include <math.h>
#include "gpio.h"
#include "fix16.h"

MAX31855_TypeDef devices[] = {
  {.port=CS0_GPIO_Port, .pin=CS0_Pin,}, 
  {.port=CS1_GPIO_Port, .pin=CS1_Pin,}, 
  {.port=CS2_GPIO_Port, .pin=CS2_Pin,}, 
  {.port=CS3_GPIO_Port, .pin=CS3_Pin,}
};

fix16_t CompensateNIST(fix16_t coldJunctTemp, fix16_t hotJunctTemp);
static uint8_t currentChannel = 0;

const fix16_t coldJunctSlope = F16(0.0625);
const fix16_t hotJunctSlope = F16(0.25);

uint8_t MAX31855_NextChannel() {
  currentChannel = (currentChannel + 1) % NUM_MAX31855_CHANNELS;
  return currentChannel;
}
void MAX31855_InitiateTransfer() {
  HAL_GPIO_WritePin(
      devices[currentChannel].port, 
      devices[currentChannel].pin, 
      GPIO_PIN_RESET);
}

void MAX31855_CompleteTransfer() {
  HAL_GPIO_WritePin(
      devices[currentChannel].port, 
      devices[currentChannel].pin, 
      GPIO_PIN_SET);
}

/* *
 * Unpack 32 bit data, data is packed into 16 bit values which requires manually packing
 * out the sign extension for negative numbers as returned numbers are only 12 or 14 bits.
 * Temperature and error data is corrected and stored for later recollection. 
 */
void MAX31855_ProcessData (uint8_t * buffer) {
  uint32_t data = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
  devices[currentChannel].rawData = data;
  devices[currentChannel].errorState = (data >> 16) & 0x0001;
    
  data = data >> 4;
  int16_t tmp = data & 0x07FF;
  if (data & 0x0800) {
    tmp = 0xF800 | tmp; //sign extension
  }
  fix16_t coldJunctTemp = coldJunctSlope * tmp; 
  devices[currentChannel].coldJunctTemp = coldJunctTemp;
 
  if (devices[currentChannel].errorState == 0) {
    data = data >> 14;
    tmp = data & 0x3FFF;
    if (data & 0x4000) {
      tmp = 0xC000 | tmp;
    }
    fix16_t hotJunctTemp = hotJunctSlope * tmp;
    devices[currentChannel].hotJunctTemp = hotJunctTemp;
    //devices[currentChannel].hotJunctTemp = CompensateNIST(coldJunctTemp, hotJunctTemp);
  }
}

fix16_t MAX31855_GetHotTemp(uint8_t channel) {
  return devices[channel].hotJunctTemp;
}

fix16_t MAX31855_GetColdTemp(uint8_t channel) {
  return devices[channel].coldJunctTemp;
}

uint8_t MAX31855_GetErrorBit(uint8_t channel) {
  return devices[channel].errorState;
}

uint32_t MAX31855_GetRawData(uint8_t channel) {
  return devices[channel].rawData;
}

fix16_t fix16_pow(fix16_t val, uint8_t exponent) {
  fix16_t retval = F16(1);
  for (uint8_t i = 0; i < exponent; i++) {
    retval = fix16_mul(retval, val);
  }
  return retval;
}


/* *
 * The MAX31855 uses a simple linear approximation to convert thermocouple voltage to 
 * temperature. This is reasonably accurate in the range 0-300 degrees C but is very 
 * inaccurate for negative temperatures and above 500C. To correct for this we can use the 
 * published linear approximations for both the hot and cold junctions to calculate the 
 * actual thermocouple junction voltages and then apply the NIST polynomials which give 
 * an ~+/-0.05 degree C error. In practise this error will be swamped ADC quantisation 
 * errors.
 * TODO:
 * This could alternatively be achieved by calculating the errors and applying a PWL 
 * adjustment factor which would use less resource than calculating powers.
 * TODO:
 * There is an unexpected discontinuity when hotJunctTemp == coldJunctTemp (i.e. between
 * the top of polynomial 0 and the bottom of polynomial 1) which causes an obvious error 
 * when the measured temp is near the coldJunctTemp.  
 * */

fix16_t CompensateNIST(fix16_t coldJunctTemp, fix16_t hotJunctTemp) {
  // the three polynomials below are for -200-0, 0-500 and 500-1372 degrees C
  const float polynomial[][10] = {{
    F16(0.0000000E+00),   F16(2.5173462E+01),  
    F16(-1.1662878E+00),  F16(-1.0833638E+00), 
    F16(-8.9773540E-01),  F16(-3.7342377E-01), 
    F16(-8.6632643E-02),  F16(-1.0450598E-02), 
    F16(-5.1920577E-04),  F16(0.0000000E+00)
  },{
    F16(0.000000E+00),    F16(2.508355E+01), 
    F16(7.860106E-02),    F16(-2.503131E-01),
    F16(8.315270E-02),    F16(-1.228034E-02),  
    F16(9.804036E-04),    F16(-4.413030E-05),  
    F16(1.057734E-06),    F16(-1.052755E-08)
  },{
    F16(-1.318058E+02),   F16(4.830222E+01),   
    F16(-1.646031E+00),   F16(5.464731E-02),
    F16(-9.650715E-04),   F16(8.802193E-06),
    F16(-3.110810E-08),   F16(0.000000E+00),
    F16(0.000000E+00),    F16(0.000000E+00)
  }};

  fix16_t coldJunctVoltage = fix16_mul(coldJunctTemp, F16(0.04073));
  fix16_t hotJunctVoltage = fix16_mul(fix16_sub(hotJunctTemp, coldJunctTemp), F16(0.041276));
  fix16_t measuredVoltage = fix16_add(coldJunctVoltage, hotJunctVoltage);

  uint8_t tempRange = hotJunctVoltage <= 0 ? 0 : hotJunctVoltage <= F16(20.644) ? 1 : 2;

  fix16_t compensatedTemp = F16(0);
  for (uint8_t term = 0; term < 10; term++) {
    compensatedTemp = fix16_add(
      compensatedTemp, 
      fix16_mul(polynomial[tempRange][term], fix16_pow(measuredVoltage, term))
    );  
  }

  return compensatedTemp;
}


