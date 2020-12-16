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
mgmt_handler_fn Sentrius_mgmt_Echo;
/* pyend */

/**
 * Command IDs for file system management group.
 */
/* pystart - mgmt function indices */
#define SENTRIUS_MGMT_ID_GETPARAMETER 1
#define SENTRIUS_MGMT_ID_SETPARAMETER 2
#define SENTRIUS_MGMT_ID_ECHO 3
/* pyend */

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
