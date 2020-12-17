/**
 * @file UserInterfaceTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __USER_INTERFACE_TASK_H__
#define __USER_INTERFACE_TASK_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 */
void UserInterfaceTask_Initialize(void);

/**
 * @brief Run LED test
 *
 * @param duration of each step in milliseconds
 *
 * @retval negative error code, 0 on success
 */
int UserInterfaceTask_LedTest(uint32_t duration);

#ifdef __cplusplus
}
#endif

#endif /* __USER_INTERFACE_TASK_H__ */
