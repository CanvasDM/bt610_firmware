/**
 * @file production_mgmt.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PRODUCTION_MGMT_H__
#define __PRODUCTION_MGMT_H__

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

#define MGMT_GROUP_ID_PRODUCTION_CMD 256
/* clang-format off */
#define PRODUCTION_MGMT_ID_REV_ECHO                              1
#define PRODUCTION_MGMT_ID_CALIBRATE_THERMISTOR                  2
#define PRODUCTION_MGMT_ID_TEST_LED                              3
/* clang-format on */

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
