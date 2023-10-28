#include "bangbang.h"
#include "relay.h"
#include "fix16.h"
#include "MAX31855.h"

fix16_t target = 0;
uint8_t controlChannel = 0;
uint8_t outputChannel = 0;
uint8_t validConfig = 0;
uint8_t enabled = 0;

uint8_t BangBang_GetConfigBits() { return validConfig; }
fix16_t BangBang_GetTarget() { return target; }
uint8_t BangBang_GetControlChannel() { return controlChannel; }
uint8_t BangBang_GetOutputChannel() { return outputChannel; }
uint8_t BangBang_GetEnable() { return enabled; }

void BangBang_SetTarget(fix16_t temp) {
  target = temp;
  validConfig |= TEMP_CONFIG_BIT;
}

void BangBang_SetControlChannel(uint8_t channel) {
  if (channel >= 0 && channel <= 3) { 
    controlChannel = channel;
    validConfig |= CONTROL_CHANNEL_CONFIG_BIT;
  } else {
    validConfig &= ~(CONTROL_CHANNEL_CONFIG_BIT);
  }
}

void BangBang_SetOutputChannel(uint8_t channel) {
  if (channel >= 0 && channel <= 1) {
    outputChannel = channel;
    validConfig |= OUTPUT_CHANNEL_CONFIG_BIT;
  } else {
    validConfig &= ~(OUTPUT_CHANNEL_CONFIG_BIT);
  }
}

uint8_t BangBang_Enable() {
  if (validConfig == (TEMP_CONFIG_BIT | 
                      CONTROL_CHANNEL_CONFIG_BIT | 
                      OUTPUT_CHANNEL_CONFIG_BIT)) {
    enabled = 1;
  }

  return validConfig;
}

void BangBang_Disable() {
  enabled = 0;
}

void BangBang_Update() {
  fix16_t temp = MAX31855_GetHotTemp(controlChannel);
  static uint8_t heating = 0;
  
  if (enabled && !heating && temp <= fix16_sub(target, HYSTERESIS)) {
    heating = 1;
  } 
  if (heating && temp >= target) {
    heating = 0;
  }
  if (!enabled) {
    heating = 0;
  }
  
  if (heating) {
    Relay_SetOutput(outputChannel, 1);
  } else {
    Relay_SetOutput(outputChannel, 0);
  }
}
