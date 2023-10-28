#ifndef BANGBANG_H 
#define BANGBANG_H
#include "fix16.h"
#include "MAX31855.h"

#define TEMP_CONFIG_BIT             0x01
#define CONTROL_CHANNEL_CONFIG_BIT  0x02
#define OUTPUT_CHANNEL_CONFIG_BIT   0x04
#define HYSTERESIS                  F16(0.5)

uint8_t BangBang_GetConfigBits();
fix16_t BangBang_GetTarget();
uint8_t BangBang_GetControlChannel();
uint8_t BangBang_GetOutputChannel();
uint8_t BangBang_GetEnable();

void BangBang_SetTarget(fix16_t temp);
void BangBang_SetControlChannel(uint8_t channel);
void BangBang_SetOutputChannel(uint8_t channel);
uint8_t BangBang_Enable();
void BangBang_Disable();
void BangBang_Update();

#endif  //BANGBANG_H
