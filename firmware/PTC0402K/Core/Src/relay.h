#ifndef RELAY_H
#define RELAY_H
#include "gpio.h"

typedef struct {
  GPIO_TypeDef *  port;
  uint8_t         pin;
} Relay_TypeDef;

void Relay_SetOutput(uint8_t relay, uint8_t state);
uint8_t Relay_GetOutput(uint8_t relay);

#endif // RELAY_H
