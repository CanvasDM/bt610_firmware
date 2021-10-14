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
 * @brief Check the Thermistor temperature against the two high thresholds
 *
 * @param channel This is the thermistor channel
 *
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int HighTempAlarmCheck(size_t channel, float value);

/**
 * @brief Check the Thermistor temperature against the two low thresholds
 *
 * @param channel This is the thermistor channel
 *
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int LowTempAlarmCheck(size_t channel, float value);

/**
 * @brief Check the delta temperature measurement against the threshold
 *
 * @param channel This is the thermistor channel
 *
 * @param tempDifference This is the delta temperature
 *
 * @retval negative error code, 0 on success
 */
int DeltaTempAlarmCheck(size_t channel, float tempDifference);

/**
 * @brief Checks to see if any of the temp alarm flags are set
 *
 * @param channel This is the temp channel
 *
 * @retval negative error code, 0 on success
 */
int TempAlarmFlagCheck(size_t channel);

/**
 * @brief Check the Analog value against the two high thresholds
 *
 * @param channel This is the analog channel
 *
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int HighAnalogAlarmCheck(size_t channel, float value);

/**
 * @brief Check the Analog value against the two low thresholds
 *
 * @param channel This is the analog channel
 *
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int LowAnalogAlarmCheck(size_t channel, float value);

/**
 * @brief Check the delta analog measurement value against the threshold
 *
 * @param channel This is the analog channel
 *
 * @param analogDifference This is the delta analog value
 *
 * @retval negative error code, 0 on success
 */
int DeltaAnalogAlarmCheck(size_t channel, float analogDifference);

/**
 * @brief Checks to see if any of the analog alarm flags are set
 *
 * @param channel This is the analog channel
 *
 * @retval negative error code, 0 on success
 */
int AnalogAlarmFlagCheck(size_t channel);

/**
 * @brief Disable the alarms and clear the alarm flags for channel input
 *
 * @param channel This is the thermistor channel
 */
void DeactivateTempAlarm(size_t channel);

/**
 * @brief Disable the alarms and clear the alarm flags for channel input
 *
 * @param channel This is the analog channel
 */
void DeactivateAnalogAlarm(size_t channel);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_CONTROL_H__ */
