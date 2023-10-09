#include "relay.h"
#include "gpio.h"

#define NUM_RELAYS 2

Relay_TypeDef _relays[NUM_RELAYS] = {
  {.port=OUT0_GPIO_Port, .pin=OUT0_Pin}, 
  {.port=OUT1_GPIO_Port, .pin=OUT1_Pin}, 
};

void Relay_SetOutput(uint8_t relay, uint8_t state) {
  if (relay < NUM_RELAYS) {
    if (state) {
      HAL_GPIO_WritePin(_relays[relay].port, _relays[relay].pin, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(_relays[relay].port, _relays[relay].pin, GPIO_PIN_RESET);
    }
  }
}

uint8_t Relay_GetOutput(uint8_t relay) {
  if (relay < NUM_RELAYS) {
    return HAL_GPIO_ReadPin(_relays[relay].port, _relays[relay].pin);
  } else {
    return 0;
  }
}
