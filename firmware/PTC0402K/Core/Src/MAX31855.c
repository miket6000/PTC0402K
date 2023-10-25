#include "MAX31855.h"
#include <stdint.h>
#include "NIST.h"
#include "gpio.h"
#include "fix16.h"

#define COLD_JUNCTION_SENSITIVITY F16(0.04073)
#define HOT_JUNCTION_SENSITIVITY F16(0.041276)
#define COLD_LSB F16(0.0625)
#define HOT_LSB F16(0.25)

MAX31855_TypeDef devices[] = {
  {.port=CS0_GPIO_Port, .pin=CS0_Pin,}, 
  {.port=CS1_GPIO_Port, .pin=CS1_Pin,}, 
  {.port=CS2_GPIO_Port, .pin=CS2_Pin,}, 
  {.port=CS3_GPIO_Port, .pin=CS3_Pin,}
};

fix16_t CompensateNIST(fix16_t coldJunctTemp, fix16_t hotJunctTemp);
static uint8_t currentChannel = 0;

fix16_t calibration_offset[] = {F16(-2.5), F16(-2.7), F16(-3.3), F16(-3.60)};

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
    tmp = 0xF800 | tmp; // sign extension
  }
  fix16_t coldJunctTemp = COLD_LSB * tmp;
  devices[currentChannel].coldJunctTemp = coldJunctTemp;
 
  if (devices[currentChannel].errorState == 0) {
    data = data >> 14;
    tmp = data & 0x3FFF;
    if (data & 0x4000) {
      tmp = 0xC000 | tmp; // more sign extension
    }    
    fix16_t hotJunctTemp = HOT_LSB * tmp;
    devices[currentChannel].hotJunctTemp = CompensateNIST(coldJunctTemp, hotJunctTemp);
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

/* *
 * The MAX31855 uses a simple linear approximation to convert thermocouple voltage to 
 * temperature. This is reasonably accurate in the range 0-300 degrees C but is very 
 * inaccurate for negative temperatures and above 500C. To correct for this we can use
 * the published NIST polynomial curves which have been encoded into NIST.h. We use 
 * linear interpolation to keep the table size reasonable.
 * */
fix16_t CompensateNIST(fix16_t coldJunctTemp, fix16_t hotJunctTemp) {
  fix16_t coldJunctVoltage = fix16_mul(coldJunctTemp, COLD_JUNCTION_SENSITIVITY);
  fix16_t hotJunctVoltage = fix16_mul(fix16_sub(hotJunctTemp, coldJunctTemp), HOT_JUNCTION_SENSITIVITY);
  fix16_t measuredVoltage = fix16_add(coldJunctVoltage, hotJunctVoltage);
  
  fix16_t table_pos = fix16_mul(LOOKUP_VSTEP, fix16_sub(measuredVoltage, LOOKUP_VMIN)); 
  uint16_t lower_pos = fix16_to_int(fix16_sub(table_pos, F16(0.5)));
  uint16_t upper_pos = fix16_to_int(fix16_add(table_pos, F16(0.5)));
  fix16_t span = fix16_sub(temperatures[upper_pos], temperatures[lower_pos]);
  fix16_t mantissa = fix16_sub(table_pos, F16(lower_pos));

  fix16_t compensatedTemp = fix16_add(temperatures[lower_pos], fix16_mul(span, mantissa));
  return compensatedTemp;
}

