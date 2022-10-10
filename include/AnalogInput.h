/**
 * @file AnalogInput.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ANALOG_INPUT_H__
#define __ANALOG_INPUT_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define ANALOG_INPUTS_MAX_PRESSURE_SENSORS 2
#define ANALOG_INPUTS_MAX_PRESSURE_SENSORS_WITH_ULTRASONIC 1
#define ANALOG_INPUTS_MAX_ULTRASONIC 1

#define ANALOG_INPUT_NUMBER_OF_CHANNELS 4

#ifdef __cplusplus
}
#endif

#endif /* __ANALOG_INPUT_H__ */
