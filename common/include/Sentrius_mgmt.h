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
mgmt_handler_fn Sentrius_mgmt_GetParameter;
mgmt_handler_fn Sentrius_mgmt_SetParameter;
mgmt_handler_fn Sentrius_mgmt_RevEcho;
mgmt_handler_fn Sentrius_mgmt_CalibrateThermistor;
mgmt_handler_fn Sentrius_mgmt_TestLed;
mgmt_handler_fn Sentrius_mgmt_CalibrateThermistor_Version2;
mgmt_handler_fn Sentrius_mgmt_Set_Rtc;
mgmt_handler_fn Sentrius_mgmt_Get_Rtc;
mgmt_handler_fn Sentrius_mgmt_Load_Parameter_File;
mgmt_handler_fn Sentrius_mgmt_Dump_Parameter_File;
mgmt_handler_fn Sentrius_mgmt_Prepare_Log;
mgmt_handler_fn Sentrius_mgmt_Ack_Log;
mgmt_handler_fn Sentrius_mgmt_Factory_Reset;
/* pyend */

/**
 * Command IDs for file system management group.
 *
 * @note location zero isn't used because API generator doesn't
 * support multiple commands with the same id (even though their
 * group number is different).
 */
/* clang-format off */
/* pystart - mgmt function indices */
#define SENTRIUS_MGMT_ID_GETPARAMETER                          1
#define SENTRIUS_MGMT_ID_SETPARAMETER                          2
#define SENTRIUS_MGMT_ID_REVECHO                               3
#define SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR                   4
#define SENTRIUS_MGMT_ID_TESTLED                               5
#define SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR_VERSION2          6
#define SENTRIUS_MGMT_ID_SET_RTC                               7
#define SENTRIUS_MGMT_ID_GET_RTC                               8
#define SENTRIUS_MGMT_ID_LOAD_PARAMETER_FILE                   9
#define SENTRIUS_MGMT_ID_DUMP_PARAMETER_FILE                   10
#define SENTRIUS_MGMT_ID_PREPARE_LOG                           11
#define SENTRIUS_MGMT_ID_ACK_LOG                               12
#define SENTRIUS_MGMT_ID_FACTORY_RESET                         13
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
