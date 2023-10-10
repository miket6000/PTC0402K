#ifndef MAX31855_H
#define MAX31855_H

#include <stdint.h>
#include "gpio.h"
#include "fix16.h"

#define NUM_MAX31855_CHANNELS 4

#define FXP_SCALE (2 << 30)
#define FXP(x) ((int64_t)(x * FXP_SCALE))
#define FXP_WHOLE(x) ((int32_t)(x / FXP_SCALE))
#define FXP_FRACT(x) ((int32_t)(x - ((int32_t)(x / FXP_SCALE) * FXP_SCALE)))
#define FXP_T  int64_t

typedef struct {
  GPIO_TypeDef *  port;
  uint8_t         pin;
  uint32_t        rawData;
  fix16_t         hotJunctTemp;
  fix16_t         coldJunctTemp;
  uint8_t         errorState;
} MAX31855_TypeDef;

void MAX31855_InitiateTransfer();
void MAX31855_CompleteTransfer();
void MAX31855_ProcessData(uint8_t * buffer);
uint8_t MAX31855_NextChannel();
fix16_t MAX31855_GetHotTemp(uint8_t channel);
fix16_t MAX31855_GetColdTemp(uint8_t channel);
uint8_t MAX31855_GetErrorBit(uint8_t channel);
uint32_t MAX31855_GetRawData(uint8_t channel);

#endif //MAX31855_H
