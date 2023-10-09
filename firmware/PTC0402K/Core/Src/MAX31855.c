#include "MAX31855.h"
#include <stdint.h>
#include <math.h>
#include "gpio.h"

MAX31855_TypeDef _devices[] = {
  {.port=CS0_GPIO_Port, .pin=CS0_Pin,}, 
  {.port=CS1_GPIO_Port, .pin=CS1_Pin,}, 
  {.port=CS2_GPIO_Port, .pin=CS2_Pin,}, 
  {.port=CS3_GPIO_Port, .pin=CS3_Pin,}
};

float _CompensateNIST(float coldJunctTemp, float hotJunctTemp);
static uint8_t _currentChannel = 0;

uint8_t MAX31855_NextChannel() {
  _currentChannel = (_currentChannel + 1) % NUM_MAX31855_CHANNELS;
  return _currentChannel;
}
void MAX31855_InitiateTransfer() {
  HAL_GPIO_WritePin(
      _devices[_currentChannel].port, 
      _devices[_currentChannel].pin, 
      GPIO_PIN_RESET);
}

void MAX31855_CompleteTransfer() {
  HAL_GPIO_WritePin(
      _devices[_currentChannel].port, 
      _devices[_currentChannel].pin, 
      GPIO_PIN_SET);
}

/* *
 * Unpack 32 bit data, data is packed into 16 bit values which requires manually packing
 * out the sign extension for negative numbers as returned numbers are only 12 or 14 bits.
 * Temperature and error data is corrected and stored for later recollection. 
 */
void MAX31855_ProcessData (uint8_t * buffer) {
  uint32_t data = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
  _devices[_currentChannel].rawData = data;
  _devices[_currentChannel].errorState = (data >> 16) & 0x0001;
    
  data = data >> 4;
  int16_t tmp = data & 0x07FF;
  if (data & 0x0800) {
    tmp = 0xF800 | tmp; //sign extension
  }
  float coldJunctTemp = 0.0625 * tmp; 
  _devices[_currentChannel].coldJunctTemp = coldJunctTemp;
  
  if (_devices[_currentChannel].errorState == 0) {
    
    data = data >> 14;
    tmp = data & 0x3FFF;
    if (data & 0x4000) {
      tmp = 0xC000 | tmp;
    }
    float hotJunctTemp = 0.25 * tmp;

    _devices[_currentChannel].hotJunctTemp = _CompensateNIST(coldJunctTemp, hotJunctTemp);
  }
}

float MAX31855_GetHotTemp(uint8_t channel) {
  return _devices[channel].hotJunctTemp;
}

float MAX31855_GetColdTemp(uint8_t channel) {
  return _devices[channel].coldJunctTemp;
}

uint32_t MAX31855_GetErrorBit(uint8_t channel) {
  return _devices[channel].errorState;
}

uint32_t MAX31855_GetRawData(uint8_t channel) {
  return _devices[channel].rawData;
}

/* *
 * The MAX31855 uses a simple linear approximation to convert thermocouple voltage to 
 * temperature. This is reasonably accurate in the range 0-300 degrees C but is very 
 * inaccurate for negative temperatures and above 500C. To correct for this we can use the 
 * published linear approximations for both the hot and cold junctions to calculate the 
 * actual thermocouple junction voltages and then apply the NIST polynomials which give 
 * an ~+/-0.05 degree C error. In practise this error will be swamped by electrical noise 
 * and ADC quantisation and linearity errors.
 * TODO:
 * This could alternatively be achieved by calculating the errors and applying a PWL 
 * adjustment factor which would use much less resource than importing math.c so we can 
 * calculate powers and exponents of floating point numbers on an MCU with no FPU.
 * */

float _CompensateNIST(float coldJunctTemp, float hotJunctTemp) {
  // the three polynomials below are for -200-0, 0-500 and 500-1372 degrees C
  const float polynomial[][10] = {{
    0.0000000E+00,  2.5173462E+01,  -1.1662878E+00, -1.0833638E+00, -8.9773540E-01,
    -3.7342377E-01, -8.6632643E-02, -1.0450598E-02, -5.1920577E-04, 0.0000000E+00
  },{
    0.000000E+00,   2.508355E+01, 7.860106E-02,   -2.503131E-01,  8.315270E-02,
    -1.228034E-02,  9.804036E-04, -4.413030E-05,  1.057734E-06,   -1.052755E-08
  },{
    -1.318058E+02,  4.830222E+01,   -1.646031E+00,  5.464731E-02, -9.650715E-04,
    8.802193E-06,   -3.110810E-08,  0.000000E+00,   0.000000E+00, 0.000000E+00
  }};

  float coldJunctVoltage = coldJunctTemp * 0.04073;
  float hotJunctVoltage = (hotJunctTemp - coldJunctTemp) * 0.041276;
  float measuredVoltage = coldJunctVoltage + hotJunctVoltage;

  uint8_t tempRange = hotJunctVoltage <= 0 ? 0 : hotJunctVoltage <= 20.644 ? 1 : 2;

  float compensatedTemp = 0;
  for (uint8_t term = 0; term < 10; term++) {
    compensatedTemp += polynomial[tempRange][term] * pow(measuredVoltage, term);  
  }

  return compensatedTemp;
}


