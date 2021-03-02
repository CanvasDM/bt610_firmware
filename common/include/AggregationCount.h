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
 * @retval negative error code, 0 on success
 */
int AggregationTempHandler(size_t channel);
int AggregationAnalogHandler(size_t channel);
#ifdef __cplusplus
}
#endif

#endif /* __AGGREGATION_COUNT_H__ */
