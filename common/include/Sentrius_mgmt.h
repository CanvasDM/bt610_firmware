/**
 * @file Sentrius_mgmt.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SENTRIUS_MGMT_H__
#define __SENTRIUS_MGMT_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "mgmt/mgmt.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

#define MGMT_GROUP_ID_SENTRIUS 65

/* pystart - mgmt handler function defines */
mgmt_handler_fn Sentrius_mgmt_get_parameter;
mgmt_handler_fn Sentrius_mgmt_set_parameter;
mgmt_handler_fn Sentrius_mgmt_rev_echo;
mgmt_handler_fn Sentrius_mgmt_calibrate_thermistor;
mgmt_handler_fn Sentrius_mgmt_test_led;
mgmt_handler_fn Sentrius_mgmt_calibrate_thermistor_version_2;
mgmt_handler_fn Sentrius_mgmt_set_rtc;
mgmt_handler_fn Sentrius_mgmt_get_rtc;
mgmt_handler_fn Sentrius_mgmt_load_parameter_file;
mgmt_handler_fn Sentrius_mgmt_dump_parameter_file;
mgmt_handler_fn Sentrius_mgmt_prepare_log;
mgmt_handler_fn Sentrius_mgmt_ack_log;
mgmt_handler_fn Sentrius_mgmt_factory_reset;
mgmt_handler_fn Sentrius_mgmt_prepare_test_log;
mgmt_handler_fn Sentrius_mgmt_check_lock_status;
mgmt_handler_fn Sentrius_mgmt_set_lock_code;
mgmt_handler_fn Sentrius_mgmt_lock;
mgmt_handler_fn Sentrius_mgmt_unlock;
mgmt_handler_fn Sentrius_mgmt_get_unlock_error_code;
/* pyend */

/**
 * Command IDs for file system management group.
 *
 * @note location zero isn't used because API generator doesn't
 * support multiple commands with the same id (even though their
 * group number is different).
 *
 * NOTE - IDs 14 through 17 are reserved for use by the MG100.
 *
 */
/* clang-format off */
/* pystart - mgmt function indices */
#define SENTRIUS_MGMT_ID_GET_PARAMETER                         1
#define SENTRIUS_MGMT_ID_SET_PARAMETER                         2
#define SENTRIUS_MGMT_ID_REV_ECHO                              3
#define SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR                  4
#define SENTRIUS_MGMT_ID_TEST_LED                              5
#define SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR_VERSION_2        6
#define SENTRIUS_MGMT_ID_SET_RTC                               7
#define SENTRIUS_MGMT_ID_GET_RTC                               8
#define SENTRIUS_MGMT_ID_LOAD_PARAMETER_FILE                   9
#define SENTRIUS_MGMT_ID_DUMP_PARAMETER_FILE                   10
#define SENTRIUS_MGMT_ID_PREPARE_LOG                           11
#define SENTRIUS_MGMT_ID_ACK_LOG                               12
#define SENTRIUS_MGMT_ID_FACTORY_RESET                         13
#define SENTRIUS_MGMT_ID_PREPARE_TEST_LOG                      18
#define SENTRIUS_MGMT_ID_CHECK_LOCK_STATUS                     21
#define SENTRIUS_MGMT_ID_SET_LOCK_CODE                         22
#define SENTRIUS_MGMT_ID_LOCK                                  23
#define SENTRIUS_MGMT_ID_UNLOCK                                24
#define SENTRIUS_MGMT_ID_GET_UNLOCK_ERROR_CODE                 25
/* pyend */
/* clang-format on */

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Registers the file system management command handler group.
 */
void Sentrius_mgmt_register_group(void);

#ifdef __cplusplus
}
#endif

#endif
