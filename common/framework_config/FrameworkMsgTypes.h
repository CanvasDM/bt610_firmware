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

/******************************************************************************/
/* Project Specific Message Types                                             */
/******************************************************************************/
typedef struct {
	FwkMsgHeader_t header;
	uint32_t durationMs;

} LedTestMsg_t;


typedef struct {
	FwkMsgHeader_t header;
	uint8_t status;
	uint16_t pin;
} DigitalInMsg_t;

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_TYPES_H__ */
