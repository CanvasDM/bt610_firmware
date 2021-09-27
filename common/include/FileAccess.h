/**
 * @file FileAccess.h
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FILE_ACCESS_H__
#define __FILE_ACCESS_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define EVENT_MANAGER_FILE_OUT_PATH "/ext/event_file_out"
#define SENTRIUS_MGMT_PARAMETER_DUMP_PATH "/ext/dump.txt"
#define SENTRIUS_MGMT_PARAMETER_FEEDBACK_PATH "/ext/load_bt_feedback.txt"
#define SENTRIUS_MGMT_PARAMETER_FILE_PATH "/ext/params.txt"
#define ATTRIBUTE_SHELL_PARAMETER_FEEDBACK_PATH "/ext/load_shell_feedback.txt"

#ifdef __cplusplus
}
#endif

#endif /* __FILE_ACCESS_H__ */
