/**
 * @file SensorTask.h
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SENSOR_TASK_H__
#define __SENSOR_TASK_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>
#include <drivers/spi.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
typedef enum {
	THERM_CH_1 = 0,
	THERM_CH_2,
	THERM_CH_3,
	THERM_CH_4,
	TOTAL_THERM_CH
} thermistor_channel_t;

typedef enum {
	ANALOG_CH_1 = 0,
	ANALOG_CH_2,
	ANALOG_CH_3,
	ANALOG_CH_4,
	TOTAL_ANALOG_CH
} analog_channel_t;

/* Sensor configuration */
#define ANALOG_INPUTS_MAX_PRESSURE_SENSORS 2
#define ANALOG_INPUTS_MAX_PRESSURE_SENSORS_WITH_ULTRASONIC 1
#define ANALOG_INPUTS_MAX_ULTRASONIC 1
#define ANALOG_INPUT_NUMBER_OF_CHANNELS 4

/* RTC timestamps are printed in HH:MM:SS format (extra character for NULL) */
#define SENSOR_TASK_RTC_TIMESTAMP_SIZE 9

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief The setup of the thread parameters
 */
void SensorTask_Initialize(void);

/**
 * @brief Loads updated lock code from attribute to local object
 */
void LoadSettingPasscode(void);

/**
 * @brief Converts the RTC time to a meaningful time string.
 *
 * @param [out] The converted RTC time.
 */
#ifdef CONFIG_LOG
void SensorTask_GetTimeString(uint8_t *time_string);
#else
#define SensorTask_GetTimeString(x)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASK_H__ */
