/**
 * @file AdcRead.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ADC_READ_H__
#define __ADC_READ_H__

/* (Remove Empty Sections) */
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
typedef enum
{
	BATTERY_ADC_CH = 0,
	ANALOG_SENSOR_1_CH,
	ANALOG_SENSOR_2_CH,
	ANALOG_SENSOR_3_CH,
	ANALOG_SENSOR_4_CH,
	ANALOG_SENSOR_5_CH,
}AnalogTypesChannel_t;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 *
 * @param
 * @param
 *
 * @retval
 */

uint32_t AnalogRead(AnalogTypesChannel_t channelReading);
uint32_t ADC_GetBatteryMv(void);
#ifdef __cplusplus
}
#endif

#endif /* __TEMPLATE_H__ */
