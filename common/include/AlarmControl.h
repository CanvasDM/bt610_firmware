/**
 * @file AlarmControl.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ALARM_CONTROL_H__
#define __ALARM_CONTROL_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Check the Thermistor temperture against the two high thresholds
 *
 * @param channel This is the thermistor channel
 *
 * @retval negative error code, 0 on success
 */
int HighTempAlarmCheck(size_t channel);
/**
 * @brief Check the Thermistor temperture against the two low thresholds
 *
 * @param channel This is the thermistor channel
 *
 * @retval negative error code, 0 on success
 */
int LowTempAlarmCheck(size_t channel);
/**
 * @brief Check the delta Temperture measurment against the threshold
 *
 * @param channel This is the thermistor channel
 *
 * @param channel This is the delta temperture
 *
 * @retval negative error code, 0 on success
 */
int DeltaTempAlarmCheck(size_t channel, float tempDifference);
/**
 * @brief Check the Analog value against the two high thresholds
 *
 * @param channel This is the analog channel
 *
 * @retval negative error code, 0 on success
 */
int HighAnalogAlarmCheck(size_t channel);
/**
 * @brief Check the Analog value against the two low thresholds
 *
 * @param channel This is the analog channel
 *
 * @retval negative error code, 0 on success
 */
int LowAnalogAlarmCheck(size_t channel);
/**
 * @brief Check the delta analog measurment value against the threshold
 *
 * @param channel This is the analog channel
 *
 * @param channel This is the delta analog value
 *
 * @retval negative error code, 0 on success
 */
int DeltaAnalogAlarmCheck(size_t channel, float analogDifference);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_CONTROL_H__ */
