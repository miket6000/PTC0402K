#ifndef MAX31855_H
#define MAX31855_H

#include <stdint.h>
#include "gpio.h"

#define NUM_MAX31855_CHANNELS 4
#define ENABLED 1
#define DISABLED 0

typedef struct {
  GPIO_TypeDef *  port;
  uint8_t         pin;
  uint32_t        rawData;
  float           hotJunctTemp;
  float           coldJunctTemp;
  uint8_t        errorState;
} MAX31855_TypeDef;

void MAX31855_InitiateTransfer();
void MAX31855_CompleteTransfer();
void MAX31855_ProcessData(uint8_t * buffer);
uint8_t MAX31855_NextChannel();
float MAX31855_GetHotTemp(uint8_t channel);
float MAX31855_GetColdTemp(uint8_t channel);
uint32_t MAX31855_GetErrorBit(uint8_t channel);
uint32_t MAX31855_GetRawData(uint8_t channel);

#endif //MAX31855_H
