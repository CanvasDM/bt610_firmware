/******************************************************************************/
//! @file CborBuider.h
//! @brief
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
/******************************************************************************/

#ifndef CBOR_BUILDER_H
#define CBOR_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "FrameworkIncludes.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define JSON_RPC_VERSION_STR "2.0"

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

//-----------------------------------------------
//! @brief  Starts a new JSON message with given ID.
//!
void CborBuilder_Start(UartMsg_t *pMsg, uint32_t Id);

//-----------------------------------------------
//! @brief  Starts a new JSON message without and Id (a notification)
//!
void CborBuilder_StartNotification(UartMsg_t *pMsg);

//-----------------------------------------------
//! @brief  Finalizes a JSON message with "result:"ok". Sets wasValid to true.
//!
void CborBuilder_FinalizeOk(UartMsg_t *pJsonMsg);

//-----------------------------------------------
//! @brief  Finalizes a JSON message with "result":<Value>. Sets wasValid to true.
//!
void CborBuilder_FinalizeIntegerResult(UartMsg_t *pJsonMsg, uint32_t Value);

//-----------------------------------------------
//! @brief  Finalizes a JSON message with "result":[Size,"base 64 data"].
//! Sets wasValid to true.
//!
void CborBuilder_FinalizeBase64Result(UartMsg_t *pJsonMsg, void *pData,
				      uint32_t Size);

//-----------------------------------------------
//! @brief  Finalizes a JSON message with the error object.  Sets wasValid to false.
//!
void CborBuilder_FinalizeError(UartMsg_t *pJsonMsg, uint32_t Code,
			       char *pString);

//-----------------------------------------------
//! @brief  Adds a JSON pair to the message being built.
//!
//! @param  pKey, pValue  "<pKey>" : pValue
//!
//! @param  IsNotString is true when the value isn't a string.  This is used to properly format
//!         the output.  For example, "value:"string" or "value:1.
//!
void CborBuilder_AddPair(UartMsg_t *pJsonMsg, char *pKey, char *pValue,
			 bool IsNotString);

#ifdef __cplusplus
}
#endif

#endif

// end
