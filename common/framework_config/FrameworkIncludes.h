/**
 * @file FrameworkIncludes.h
 * @brief Headers used with Message Framework
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FRAMEWORK_INCLUDES_H__
#define __FRAMEWORK_INCLUDES_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Framework.h"
#include "BufferPool.h"
#include "FrameworkMsg.h"
#include "FrameworkMacros.h"

#include "FrameworkIds.h"
#include "FrameworkMsgCodes.h"
#include "FrameworkMsgTypes.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
/* Shortened names help functions fit on one line */
typedef DispatchResult_t Dispatch_t;
typedef FwkMsgReceiver_t FwkMsgRxer_t;

#ifdef __cplusplus
}
#endif

#endif /* __FRAMEWORK_INCLUDES_H__ */
