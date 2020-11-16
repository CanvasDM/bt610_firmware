/**
 * @file FrameworkMsgCodes.h
 * @brief Defines the couple of messages reserved by Framework.
 * Application message types can also be defined here.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FRAMEWORK_MSG_CODES_H__
#define __FRAMEWORK_MSG_CODES_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
<<<<<<< HEAD
#include "Framework.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
enum FwkMsgCodeEnum {
	/* The first application specific message should be assigned the value
	 * FMC_APPLICATION_SPECIFIC_START.  There are codes reserved by framework.
	 */
	FMC_ADV = FMC_APPLICATION_SPECIFIC_START,
    FMC_INIT_ALL_TASKS,
	FMC_LED_TEST,
	FMC_INIT_NV,
	FMC_CONTROL_ENABLE,
	FMC_ANALOG_INPUT,
	FMC_READ_ANALOG,
	FMC_READ_THERM,
	FMC_CODE_BUTTON_ISR,
	FMC_CODE_BLE_START,
	FMC_CODE_BLE_TRANSMIT,
	FMC_READ_BUFFER,
	FMC_TRANSMIT_BUFFER,
=======
typedef enum FwkMsgCodeEnum {
	/* Reserved by Framework (DO NOT DELETE) */
	FMC_INVALID = 0,
	FMC_PERIODIC,
	FMC_SOFTWARE_RESET,
	FMC_WATCHDOG_CHALLENGE,
	FMC_WATCHDOG_RESPONSE,

	/* Application Specific */
	FMC_INIT_ALL_TASKS,
	FMC_LED_TEST,
	FMC_INIT_NV,
>>>>>>> made changes to the firmware filese that were updated
	/* Last value (DO NOT DELETE) */
	NUMBER_OF_FRAMEWORK_MSG_CODES
};
BUILD_ASSERT(sizeof(enum FwkMsgCodeEnum) <= sizeof(FwkMsgCode_t),
		 "Too many message codes");


#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_CODES_H__ */