/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2012-2018, Jan Breuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scpi/scpi.h"
#include "scpi-def.h"
#include "MAX31855.h"
#include "bangbang.h"
#include "fix16.h"
#include "relay.h"

#define SIZE_OF_FLOAT_TO_STR_BUFFER 16

static scpi_result_t GetColdJunct(scpi_t * context) {
  int32_t channel;
  char buffer[SIZE_OF_FLOAT_TO_STR_BUFFER];
  SCPI_CommandNumbers(context, &channel, 1, 0);
  fix16_to_str(MAX31855_GetColdTemp((uint8_t)channel), buffer, 2);
  SCPI_ResultCharacters(context, buffer, strlen(buffer));
  return SCPI_RES_OK;
}

static scpi_result_t GetHotJunct(scpi_t * context) {
  int32_t channel;
  char buffer[SIZE_OF_FLOAT_TO_STR_BUFFER];
  SCPI_CommandNumbers(context, &channel, 1, 0);
  fix16_to_str(MAX31855_GetHotTemp((uint8_t)channel), buffer, 1);
  SCPI_ResultCharacters(context, buffer, strlen(buffer));
  return SCPI_RES_OK;
}

static scpi_result_t GetErrorState(scpi_t * context) {
  int32_t channel;
  SCPI_CommandNumbers(context, &channel, 1, 0);
  SCPI_ResultBool(context, (scpi_bool_t)MAX31855_GetErrorBit(channel));
  return SCPI_RES_OK;
}

static scpi_result_t GetRawData(scpi_t * context) {
  int32_t channel;
  SCPI_CommandNumbers(context, &channel, 1, 0);
  SCPI_ResultUInt32Base(context, MAX31855_GetRawData(channel),0x10);
  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmConfig(scpi_t * context) {
  SCPI_ResultUInt32Base(context, BangBang_GetConfigBits(), 0x10);
  return SCPI_RES_OK;
}

static scpi_result_t SetAlarmEnable(scpi_t * context) {
  uint32_t param;

  if (!SCPI_ParamUnsignedInt(context, &param, TRUE)) {
    return SCPI_RES_ERR;
  }

  if (param) {
    BangBang_Enable();
  } else {
    BangBang_Disable();
  }

  return SCPI_RES_OK;
}

static scpi_result_t SetAlarmEvent(scpi_t * context) {return SCPI_RES_OK;}
static scpi_result_t SetAlarmLimit(scpi_t * context) {
  const char * param;
  char buffer[16];
  size_t param_len;

  if (!SCPI_ParamCharacters(context, &param, &param_len, TRUE)) {
    return SCPI_RES_ERR;
  }
  
  memcpy(buffer, param, param_len);

  BangBang_SetTarget(fix16_from_str(buffer));
  return SCPI_RES_OK;
}

static scpi_result_t SetAlarmInput(scpi_t * context) {
  uint32_t param;

  if (!SCPI_ParamUnsignedInt(context, &param, TRUE)) {
    return SCPI_RES_ERR;
  }

  BangBang_SetControlChannel((uint8_t)param);
  return SCPI_RES_OK;
}

static scpi_result_t SetAlarmOutput(scpi_t * context) {  
  uint32_t param;

  if (!SCPI_ParamUnsignedInt(context, &param, TRUE)) {
    return SCPI_RES_ERR;
  }

  BangBang_SetOutputChannel((uint8_t)param);
  return SCPI_RES_OK;
}

static scpi_result_t SetOutput(scpi_t * context) {
  int32_t channel;
  uint32_t param;

  SCPI_CommandNumbers(context, &channel, 1, 0);
  if (!SCPI_ParamUnsignedInt(context, &param, TRUE)) {
    return SCPI_RES_ERR;
  }
 
  Relay_SetOutput((uint8_t)channel, (uint8_t)param);

  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmEnable(scpi_t * context) {
  SCPI_ResultUInt32(context, BangBang_GetEnable());
  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmEvent(scpi_t * context) {
  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmLimit(scpi_t * context) {
  char buffer[SIZE_OF_FLOAT_TO_STR_BUFFER];
  fix16_to_str(BangBang_GetTarget(), buffer, 2);
  SCPI_ResultCharacters(context, buffer, strlen(buffer));
  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmInput(scpi_t * context) {
  SCPI_ResultUInt32(context, BangBang_GetControlChannel());
  return SCPI_RES_OK;
}

static scpi_result_t GetAlarmOutput(scpi_t * context) {
  SCPI_ResultUInt32(context, BangBang_GetOutputChannel());
  return SCPI_RES_OK;
}

static scpi_result_t GetOutput(scpi_t * context) {
  int32_t channel;
  SCPI_CommandNumbers(context, &channel, 1, 0);
  SCPI_ResultBool(context, (scpi_bool_t)Relay_GetOutput(channel));
  return SCPI_RES_OK;
}

/**
 * Reimplement IEEE488.2 *TST?
 *
 * Result should be 0 if everything is ok
 * Result should be 1 if something goes wrong
 *
 * Return SCPI_RES_OK
 */
static scpi_result_t My_CoreTstQ(scpi_t * context) {

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = SCPI_CoreRst,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = My_CoreTstQ,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    {.pattern = "MEASure:TEMPerature#?", .callback = GetHotJunct,},
    {.pattern = "MEASure:COLDjunction#?", .callback = GetColdJunct,},
    {.pattern = "MEASure:STATe#?", .callback = GetErrorState,},
    {.pattern = "MEASure:RAW#?", .callback = GetRawData,},
    
    /* Alarms will change an output state when the temperature exceeds (OVERTEMP) or 
     * drops below (UNDERTEMP) the alarm temp. This change is one way, however a second 
     * alarm can be used to 
     * create a temperature controller by using a second alarm */

    /* ALARM0:INPUt 0; OUTPut 0; EVENt OVER; ACTIon OFF; LIMIt 99.0; ENABle 1*/
    /* ALARM1:INPUt 0; OUTPut 0; EVENt UNDEr; ACTIon ON; LIMIt 98.0; ENABle 1*/

    {.pattern = "ALARm:CONFig?", .callback = GetAlarmConfig,},
    {.pattern = "ALARm:ENABle", .callback = SetAlarmEnable,},
    {.pattern = "ALARm:ENABle?", .callback = GetAlarmEnable,},
    {.pattern = "ALARm:INPUt", .callback = SetAlarmInput,},
    {.pattern = "ALARm:INPUt?", .callback = GetAlarmInput,},
    {.pattern = "ALARm:OUTPut", .callback = SetAlarmOutput,},
    {.pattern = "ALARm:OUTPut?", .callback = GetAlarmOutput,},
    {.pattern = "ALARm:EVENt", .callback = SetAlarmEvent,},
    {.pattern = "ALARm:EVENt?", .callback = GetAlarmEvent,},
    {.pattern = "ALARm:LIMIt", .callback = SetAlarmLimit,},
    {.pattern = "ALARm:LIMIt?", .callback = GetAlarmLimit,},

    {.pattern = "OUTPut#", .callback = SetOutput,},
    {.pattern = "OUTPut#?", .callback = GetOutput,},

    SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};    

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
