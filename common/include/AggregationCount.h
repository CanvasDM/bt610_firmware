/**
 * @file AggregationCount.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __AGGREGATION_COUNT_H__
#define __AGGREGATION_COUNT_H__

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
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int AggregationTempHandler(size_t channel, float value);
/**
 * @brief Check the Analog sensors against the two high thresholds
 *
 * @param channel This is the analog channel
 * 
 * @param value This is the current measured analog value
 * 
 * @retval negative error code, 0 on success
 */
int AggregationAnalogHandler(size_t channel, float value);

#ifdef __cplusplus
}
#endif

#endif /* __AGGREGATION_COUNT_H__ */
