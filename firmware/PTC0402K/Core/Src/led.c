#include "led.h"
#include "gpio.h"
#include <stdint.h>

static LED_TypeDef leds[] = {
  {LED0_GPIO_Port, LED0_Pin},
  {LED1_GPIO_Port, LED1_Pin},
  {LED2_GPIO_Port, LED2_Pin},
  {LED3_GPIO_Port, LED3_Pin}
};  

void led_set(uint32_t id, uint32_t state) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  GPIO_InitStruct.Pin = leds[id].Pin;
  GPIO_InitStruct.Mode = (state == LED_OFF) ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  HAL_GPIO_Init(leds[id].GPIO_Port, &GPIO_InitStruct);
  
  if (state != LED_OFF) {
    uint32_t outputState = (state == LED_GREEN) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(leds[id].GPIO_Port, leds[id].Pin, outputState);
  }
}
  
