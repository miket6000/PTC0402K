#include "bangbang.h"
#include "relay.h"
#include "fix16.h"
#include "MAX31855.h"

controller_t controller;

uint8_t BangBang_GetSetBits() { return controller.validConfig; }
fix16_t BangBang_GetTarget() { return controller.target; }
uint8_t BangBang_GetInputChannel() { return controller.inputChannel; }
uint8_t BangBang_GetOutputChannel() { return controller.outputChannel; }
uint8_t BangBang_GetOutputInvert() { return controller.invert; }
uint8_t BangBang_GetEnable() { return controller.enabled; }

void BangBang_SetTarget(fix16_t temp) {
  controller.target = temp;
  controller.validConfig |= TARGET_SET_BIT;
}

void BangBang_SetInputChannel(uint8_t channel) {
  if (channel >= 0 && channel <= 3) { 
    controller.inputChannel = channel;
    controller.validConfig |= INPUT_SET_BIT;
  } else {
    controller.validConfig &= ~(INPUT_SET_BIT);
  }
}

void BangBang_SetOutputChannel(uint8_t channel) {
  if (channel >= 0 && channel <= 1) {
    controller.outputChannel = channel;
    controller.validConfig |= OUTPUT_SET_BIT;
  } else {
    controller.validConfig &= ~(OUTPUT_SET_BIT);
  }
}

void BangBang_SetOutputInvert(uint8_t invert) {
  controller.invert = invert;
}

uint8_t BangBang_Enable() {
  if (controller.validConfig == (TARGET_SET_BIT | 
                      INPUT_SET_BIT | 
                      OUTPUT_SET_BIT)) {
    controller.enabled = 1;
  }

  return controller.validConfig;
}

void BangBang_Disable() {
  Relay_SetOutput(controller.outputChannel, 0);
  controller.enabled = 0;
}

void BangBang_Update() {
  fix16_t temp = MAX31855_GetHotTemp(controller.inputChannel);
  static uint8_t relayState = 0;
  
  if (!relayState && temp <= fix16_sub(controller.target, HYSTERESIS)) {
    relayState = 1;
  } 
  if (relayState && temp >= controller.target) {
    relayState = 0;
  }
  
  if (controller.enabled) {
    Relay_SetOutput(controller.outputChannel, relayState ^ controller.invert);
  }

}
