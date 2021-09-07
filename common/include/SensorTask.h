/**
 * @file SensorTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
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
//typedef enum {

//} sensorType_t;
enum { 
    THERM_CH_1 = 0, 
    THERM_CH_2,
    THERM_CH_3, 
    THERM_CH_4, 
    TOTAL_THERM_CH };

enum {
	ANALOG_CH_1 = 0,
	ANALOG_CH_2,
	ANALOG_CH_3,
	ANALOG_CH_4,
	TOTAL_ANALOG_CH
};

/* RTC timestamps are printed in HH:MM:SS format (extra character for NULL) */
#define SENSOR_TASK_RTC_TIMESTAMP_SIZE 9

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief The setup of the thread parameters
 */
void SensorTask_Initialize(void);

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
