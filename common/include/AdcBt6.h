/**
 * @file AdcBt6.h
 * @brief ADC functions for BT6xx.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT6_ADC_H__
#define __BT6_ADC_H__

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
typedef enum AnalogChannel {
	BATTERY_ADC_CH = 0,
	ANALOG_SENSOR_1_CH = 1, /*!< AIN 0 */
	THERMISTOR_SENSOR_2_CH = 2, /*!< AIN 1 */
	VREF_5_CH = 6, /*!< AIN 5 */
} AnalogChannel_t;

/* The mux is indexed from 0 to 3 */
enum MuxInput {
	MUX_AIN1_THERM1 = 0,
	MUX_AIN2_THERM2,
	MUX_AIN3_THERM3,
	MUX_AIN4_THERM4,
	NUMBER_OF_ANALOG_INPUTS
};

enum AdcMeasurementType {
	ADC_TYPE_ANALOG_VOLTAGE = 0,
	ADC_TYPE_ANALOG_CURRENT,
	ADC_TYPE_THERMISTOR,
	ADC_TYPE_VREF,
	NUMBER_OF_ADC_TYPES
};

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/**
 * @brief Initialize ADC for BT6xx
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_Init(void);

/**
 * @brief Read battery voltage
 *
 * @param pointer to raw
 * @param pointer to millivolts
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_ReadBatteryMv(int16_t *raw, int32_t *mv);

/**
 * @brief Measure analog input or thermistor input
 *
 * @param pointer to raw
 * @param input is the input (1-4) to measure
 * @param type of measurement to perform
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_Measure(int16_t *raw, enum MuxInput input,
		   enum AdcMeasurementType type);

/**
 * @brief Get type enum as string.
 */
const char *AdcBt6_GetTypeString(enum AdcMeasurementType type);

/**
 * @brief Get channel enum as string.
 */
const char *AdcBt6_GetChannelString(enum AnalogChannel channel);

/**
 * @brief Helper Functions
 *
 * @note These functions include the required switching time delays when
 * power is enabled. Enable 5V output before enabling b+.  Disable 5V
 * after disabling b+.
 *
 * @retval negative on error, 0 on success.
 */
int AdcBt6_FiveVoltEnable(void);
int AdcBt6_FiveVoltDisable(void);
int AdcBt6_BplusEnable(void);
int AdcBt6_BplusDisable(void);

float AdcBt6_ConvertVoltage(int32_t raw);
float AdcBt6_ConvertCurrent(int32_t raw);
float AdcBt6_ConvertTherm(int32_t raw);
float AdcBt6_ConvertVref(int32_t raw);

#ifdef __cplusplus
}
#endif

#endif /* __BT6_ADC_H__ */
