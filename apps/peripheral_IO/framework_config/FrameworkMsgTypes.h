/**
 * @file FrameworkMessageTypes.h
 * @brief Project specific message types are defined here.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FRAMEWORK_MSG_TYPES_H__
#define __FRAMEWORK_MSG_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Framework.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
typedef enum
{
    UNKOWN_INPUT = 0, 
    VOLTAGE_AIN,
    CURRENT_AIN,
    THERMISTOR,
}AnalogInput_t;


/******************************************************************************/
/* Project Specific Message Types                                             */
/******************************************************************************/
typedef struct 
{
  bool analogEnable;
  bool thermEnable;
}EnablePins_t;

typedef struct
{
  FwkMsgHeader_t header;
  uint32_t durationMs;
  
} LedTestMsg_t;
typedef struct
{
  FwkMsgHeader_t header;
  EnablePins_t control;
  
} SetEnablePinMsg_t;

typedef struct
{
  FwkMsgHeader_t header;
  AnalogInput_t inputConfig;
  uint8_t externalPin;
  
} AnalogPinMsg_t;

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_TYPES_H__ */
