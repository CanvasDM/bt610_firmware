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

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASK_H__ */
