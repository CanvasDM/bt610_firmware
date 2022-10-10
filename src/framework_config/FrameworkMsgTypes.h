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
#include "lcz_sensor_event.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

/******************************************************************************/
/* Project Specific Message Types                                             */
/******************************************************************************/
typedef struct {
	FwkMsgHeader_t header;
	uint32_t durationMs;

} LedTestMsg_t;

typedef struct {
	FwkMsgHeader_t header;
	int status;
	uint16_t pin;
} DigitalInMsg_t;

typedef struct {
	FwkMsgHeader_t header;
	SensorEventType_t eventType;
	SensorEventData_t eventData;
	uint32_t id;
	uint32_t timeStamp;
} EventLogMsg_t;

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_TYPES_H__ */
