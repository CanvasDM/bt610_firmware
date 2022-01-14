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
mgmt_handler_fn Sentrius_mgmt_rev_echo;
mgmt_handler_fn Sentrius_mgmt_calibrate_thermistor;
mgmt_handler_fn Sentrius_mgmt_test_led;
mgmt_handler_fn Sentrius_mgmt_calibrate_thermistor_version_2;
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
#define SENTRIUS_MGMT_ID_REV_ECHO                              3
#define SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR                  4
#define SENTRIUS_MGMT_ID_TEST_LED                              5
#define SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR_VERSION_2        6
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
