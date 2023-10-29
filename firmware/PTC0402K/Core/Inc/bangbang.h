#ifndef BANGBANG_H 
#define BANGBANG_H
#include "fix16.h"
#include "MAX31855.h"

#define TARGET_SET_BIT  0x01
#define INPUT_SET_BIT   0x02
#define OUTPUT_SET_BIT  0x04
#define HYSTERESIS      F16(0.5)


typedef struct {
  fix16_t target;
  uint8_t inputChannel;
  uint8_t outputChannel;
  uint8_t validConfig;
  uint8_t enabled;
  uint8_t invert;
} controller_t;



uint8_t BangBang_GetSetBits();
fix16_t BangBang_GetTarget();
uint8_t BangBang_GetInputChannel();
uint8_t BangBang_GetOutputChannel();
uint8_t BangBang_GetOutputInvert();
uint8_t BangBang_GetEnable();

void BangBang_SetTarget(fix16_t temp);
void BangBang_SetInputChannel(uint8_t channel);
void BangBang_SetOutputChannel(uint8_t channel);
void BangBang_SetOutputInvert(uint8_t invert);
uint8_t BangBang_Enable();
void BangBang_Disable();
void BangBang_Update();

#endif  //BANGBANG_H
