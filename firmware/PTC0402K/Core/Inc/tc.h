#ifndef TC_H
#define TC_H

#include <stdint.h>
#include "gpio.h"

#define NUM_TC_CHANNELS 4

typedef struct {
  GPIO_TypeDef *  port;
  uint32_t        pin;
  int32_t         coldJunctTemp;
  int32_t         hotJunctTemp;
  uint32_t        errorState;
} TCChannelTypeDef;

void TC_Update(uint32_t channel, uint8_t * buffer);
int32_t TC_GetHotTemp(uint32_t channel);
int32_t TC_GetColdTemp(uint32_t channel);
uint32_t TC_GetErrorBit(uint32_t channel);
void TC_ChannelSelect(uint32_t channel);

#endif //TC_H
