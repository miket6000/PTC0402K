#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "gpio.h"

#define LED_OFF   0
#define LED_GREEN 1
#define LED_RED   2

typedef struct {
  GPIO_TypeDef *GPIO_Port;
  uint32_t Pin;
} LED_TypeDef;


void led_set(uint32_t id, uint32_t state);

#endif //LED_H
