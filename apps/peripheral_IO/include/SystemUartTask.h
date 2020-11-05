/**
 * @file SystemUartTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __SYSTEM_UART_TASK_H__
#define __SYSTEM_UART_TASK_H__

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

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 *  The setup of the thread parameters
 * @param
 * @param
 *
 * @retval
 */
void SystemUartTask_Initialize(bool Enable);


#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_UART_TASK_H__ */
