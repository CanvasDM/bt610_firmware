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
 * @brief
 *
 * @param
 * @param
 *
 * @retval
 */
int HighTempAlarmCheck(size_t channel);
int LowTempAlarmCheck(size_t channel);
int DeltaTempAlarmCheck(size_t channel, float tempDifference);
int HighAnalogAlarmCheck(size_t channel);
int LowAnalogAlarmCheck(size_t channel);
int DeltaAnalogAlarmCheck(size_t channel, float tempDifference);

#ifdef __cplusplus
}
#endif

#endif /* __ALARM_CONTROL_H__ */
