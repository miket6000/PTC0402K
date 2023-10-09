#include "tc.h"
#include <stdint.h>
#include "gpio.h"

TCChannelTypeDef thermocouples[] = {
  {CS0_GPIO_Port, CS0_Pin, 0, 0, 0}, 
  {CS1_GPIO_Port, CS1_Pin, 0, 0, 0}, 
  {CS2_GPIO_Port, CS2_Pin, 0, 0, 0}, 
  {CS3_GPIO_Port, CS3_Pin, 0, 0, 0}
};

void TC_Update(uint32_t channel, uint8_t * buffer) {
  uint32_t data = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
  thermocouples[channel].hotJunctTemp = (data >> 18) & 0x3FFF;
  thermocouples[channel].errorState = (data >> 16) & 0x0001;
  thermocouples[channel].coldJunctTemp = (data >> 4) & 0x0FFF;
}

int32_t TC_GetHotTemp(uint32_t channel) {
  return thermocouples[channel].hotJunctTemp;
}

int32_t TC_GetColdTemp(uint32_t channel) {
  return thermocouples[channel].coldJunctTemp;
}

uint32_t TC_GetErrorBit(uint32_t channel) {
  return thermocouples[channel].errorState;
}

void TC_ChannelSelect(uint32_t channel) {
  for (int32_t i = 0; i < NUM_TC_CHANNELS; i++) {
    if (i == channel) {
      HAL_GPIO_WritePin(thermocouples[i].port, thermocouples[i].pin, GPIO_PIN_RESET);
    } else {
      HAL_GPIO_WritePin(thermocouples[i].port, thermocouples[i].pin, GPIO_PIN_SET);
    }
  }
}
