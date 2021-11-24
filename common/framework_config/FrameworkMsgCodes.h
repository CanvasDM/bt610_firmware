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
#include "Framework.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
enum FwkMsgCodeEnum {
	/* The first application specific message should be assigned the value
	 * FMC_APPLICATION_SPECIFIC_START.  There are codes reserved by framework.
	 */
	FMC_ADV = FMC_APPLICATION_SPECIFIC_START,
	/*
	FMC_CONTROL_ENABLE,
	FMC_ANALOG_INPUT,
	FMC_READ_ANALOG,
	FMC_READ_THERM,
	*/
	FMC_DIGITAL_IN,
	FMC_DIGITAL_IN_CONFIG,
	FMC_MAGNET_STATE,
	FMC_READ_POWER,
	FMC_TEMPERATURE_MEASURE,
	FMC_ANALOG_MEASURE,
	FMC_BLE_START_ADVERTISING,
	FMC_BLE_END_ADVERTISING,
	FMC_ENTER_ACTIVE_MODE,
	FMC_SENSOR_EVENT,
	FMC_ALIVE,
	FMC_TAMPER,
	FMC_AMR_LED_ON,
	FMC_LEDS_OFF,
	FMC_ENTER_SHELF_MODE,
	FMC_EVENT_TRIGGER,
	FMC_SENSOR_UPDATE,
	FMC_AGGREGATION_EVENT,
	FMC_BLE_END_CONNECTION,

	/* Last value (DO NOT DELETE) */
	NUMBER_OF_FRAMEWORK_MSG_CODES
};
BUILD_ASSERT(sizeof(enum FwkMsgCodeEnum) <= sizeof(FwkMsgCode_t),
	     "Too many message codes");

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_MSG_CODES_H__ */
