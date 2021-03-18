/**
 * @file EventTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __EVENT_LOG_H__
#define __EVENT_LOG_H__

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
void EventTask_Initialize(void);

void EventTask_GetCurrentEvent(uint32_t *id, SensorEvent_t *event);
uint32_t EventTask_RemainingEvents(void);
void EventTask_IncrementEventId(void);

#ifdef __cplusplus
}
#endif

#endif /* __EVENT_LOG_H__ */