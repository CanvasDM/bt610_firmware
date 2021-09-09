/**
 * @file BleTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLE_TASK_H__
#define __BLE_TASK_H__

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
void BleTask_Initialize(void);

/**
 * @brief Returns if the last connection was made used coded PHY
 *
 * @retval true if LE coded, false otherwise
 */
bool ble_conn_last_was_le_coded(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_TASK_H__ */
