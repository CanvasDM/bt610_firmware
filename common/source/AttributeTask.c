/**
 * @file AttributeTask.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(AttributeTask);
#define THIS_FILE "AttributeTask"

//=================================================================================================
// Includes
//=================================================================================================
#include <zephyr.h>
#include "FrameworkIncludes.h"
#include "BleTask.h"
#include "BspSupport.h"
#include "Version.h"
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#include "bsp.h"
//#include "ToString.h"
//#include "Framework.h"
//#include "WatchdogTask.h"
//#include "NvIds.h"
//#include "NvSparse.h"
//#include "Utilities.h"

#include "AttributePrivate.h"
#include "AttributeValidator.h"
#include "AttributeTask.h"
#include "AttributeFunctions.h"

//=================================================================================================
// Local Constant, Macro and Type Definitions
//=================================================================================================
#ifndef ATTRIBUTE_TASK_PRIORITY
#define ATTRIBUTE_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef ATTRIBUTE_TASK_STACK_DEPTH
#define ATTRIBUTE_TASK_STACK_DEPTH 4096
#endif

#ifndef ATTRIBUTE_TASK_QUEUE_DEPTH
#define ATTRIBUTE_TASK_QUEUE_DEPTH 8
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi1), okay)
#define SPI_DEV_NAME DT_LABEL(DT_NODELABEL(spi1))
#else
#error "Please set the correct spi device"
#endif

typedef struct AttrTaskTag {
	FwkMsgTask_t msgTask; // The name "task" is used by macros.
	size_t size;
	bool factoryResetImminent;
	uint32_t broadcastCount;

} AttrTaskObj_t;

#define LENGTH_NOT_USED 0
#define ATTRIBUTE_MAX_INT_DIGITS    (10 + 1) // 2**32 and +1 for sign
 

//=================================================================================================
// Global Data Definitions
//=================================================================================================
K_MUTEX_DEFINE(attribute_mutex);
extern AttributeEntry_t attrTable[ATTRIBUTE_TABLE_SIZE];

//=================================================================================================
// Local Function Prototypes
//=================================================================================================

static bool isWritable(uint32_t Index, uint32_t SourceId);

static void AttrTaskThread(void *, void *, void *);

static DispatchResult_t AttrTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg);
static DispatchResult_t
InitializeAttributesMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg);

static void WriteAttributes(void);

static void GenerateAttributeChangedMessage(AttrTaskObj_t *pObj);
static void StartDelayedWriteTimer(AttributeEntry_t *pEntry);

//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================
static FwkMsgHandler_t AttrTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:                    return Framework_UnknownMsgHandler;
	case FMC_INITIALIZE_ATTRIBUTES:      return InitializeAttributesMsgHandler;
	case FMC_FACTORY_RESET:              return FactoryResetMsgHandler;
	default:                             return NULL;
	}
	/* clang-format on */
}

//=================================================================================================
// Local Data Definitions
//=================================================================================================
static AttrTaskObj_t attrTaskObj;

K_THREAD_STACK_DEFINE(attributeTaskStack, ATTRIBUTE_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(attributeTaskQueue, FWK_QUEUE_ENTRY_SIZE,
	      ATTRIBUTE_TASK_QUEUE_DEPTH, FWK_QUEUE_ALIGNMENT);
//default values can be changed

//=================================================================================================
// Global Function Definitions
//=================================================================================================

void AttributeTask_Initialize(void)
{
	Attribute_Initialize();

	memset(&attrTaskObj, 0, sizeof(AttrTaskObj_t));

	attrTaskObj.msgTask.rxer.id = FWK_ID_ATTRIBUTE_TASK;
	attrTaskObj.msgTask.rxer.rxBlockTicks = K_FOREVER;
	attrTaskObj.msgTask.rxer.pMsgDispatcher = AttrTaskMsgDispatcher;
	attrTaskObj.msgTask.timerDurationTicks = K_MSEC(1000);
	attrTaskObj.msgTask.timerPeriodTicks = K_MSEC(0); // 0 for one shot
	attrTaskObj.msgTask.rxer.pQueue = &attributeTaskQueue;

#if LAIRD_DEBUG
	// Attributes may not be packed.
	attrTaskObj.size = Attribute_GetRwSize();
	FWK_TRACE_RAW1("Attribute RW Size %u bytes\r\n", attrTaskObj.size);

	size_t j;
	size_t estimate = 0;
	for (j = 0; j < ATTRIBUTE_TABLE_SIZE; j++) {
		switch (j) {
		case ATTR_INDEX_method:
		case ATTR_INDEX_jsonrpc:
		case ATTR_INDEX_id:
		case ATTR_INDEX_params:
		case ATTR_INDEX_result:
		case ATTR_INDEX_error:
		case ATTR_INDEX_code:
		case ATTR_INDEX_message:
			// ignore protocol fields
			break;

		default:
			estimate += AttributeTask_GetMaxPairSize(j);
			break;
		}
	}
	FWK_TRACE_RAW1("Pair Max Size Estimate %u\r\n", estimate);
#endif
	Framework_RegisterTask(&attrTaskObj.msgTask);

	attrTaskObj.msgTask.pTid = k_thread_create(
		&attrTaskObj.msgTask.threadData, attributeTaskStack,
		K_THREAD_STACK_SIZEOF(attributeTaskStack),
		AttrTaskThread, &attrTaskObj, NULL, NULL,
		ATTRIBUTE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(attrTaskObj.msgTask.pTid, THIS_FILE);
}

bool AttributeTask_Match(uint32_t *pIndex, char const *pName, size_t NameLength)
{
	uint32_t hash = Attribute_Hash(pName, NameLength);
	*pIndex = hash;
	if (hash <= Attribute_GetMaxHashValue()) {
		const char *str = attrTable[hash].name;
		if (strncmp(pName, str, NameLength) == 0) {
			return true;
		}
	}

	return false;
}

bool AttributeTask_IsWritable(uint32_t Index, uint32_t SourceId)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	if (attrTable[Index].pValidator == AttributeValidator_Bypass) {
		return true; // Ignore protocol values (JSON strings don't have order).
	}

	return isWritable(Index, SourceId);
}

bool AttributeTask_IsReadWrite(uint32_t Index)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	} else {
		return (attrTable[Index].category == READ_WRITE);
	}
}

bool AttributeTask_IsReadOnly(uint32_t Index)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	} else {
		return (attrTable[Index].category == READ_ONLY);
	}
}

bool AttributeTask_IsProtocol(uint32_t Index)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	} else {
		return (attrTable[Index].category == PROTOCOL);
	}
}

bool AttributeTask_RestoreDefaultValue(uint32_t Index, uint32_t SourceId)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	if (!isWritable(Index, SourceId)) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	memcpy(attrTable[Index].pData, attrTable[Index].pDefault,
	       attrTable[Index].size);
	attrTable[Index].modified = true;
	StartDelayedWriteTimer(&attrTable[Index]);

	k_mutex_unlock(&attribute_mutex);

	return true;
}

//
// Wrap the function used by the JSON task so that it can be used by any task.
//
bool AttributeTask_SetWithString(uint32_t Index, char const *pValue,
				 size_t ValueLength)
{
	return AttributeTask_SetFromString(Index, pValue, ValueLength,
					   FWK_ID_RESERVED, true);
}

bool AttributeTask_SetUint32(uint32_t Index, uint32_t Value)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	bool ok = false;
	if (attrTable[Index].type == UNSIGNED_TYPE) {
		uint32_t local = Value;
		ok = attrTable[Index].pValidator(Index, &local, LENGTH_NOT_USED,
						 true);
	}

	StartDelayedWriteTimer(&attrTable[Index]);

	k_mutex_unlock(&attribute_mutex);

	return ok;
}

bool AttributeTask_SetSigned32(uint32_t Index, int32_t Value)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	bool ok = false;
	if (attrTable[Index].type == SIGNED_TYPE) {
		int32_t local = Value;
		ok = attrTable[Index].pValidator(Index, &local, LENGTH_NOT_USED,
						 true);
	}

	StartDelayedWriteTimer(&attrTable[Index]);

	k_mutex_unlock(&attribute_mutex);

	return ok;
}

bool AttributeTask_SetFloat(uint32_t Index, float Value)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	bool ok = false;
	if (attrTable[Index].type == FLOAT_TYPE) {
		float local = Value;
		ok = attrTable[Index].pValidator(Index, &local, LENGTH_NOT_USED,
						 true);
	}

	StartDelayedWriteTimer(&attrTable[Index]);

	k_mutex_unlock(&attribute_mutex);

	return ok;
}

bool AttributeTask_SetVersion(uint32_t Index, uint32_t Major, uint32_t Minor,
			      uint32_t Build)
{
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	bool ok = false;
	switch (Index) {
	case ATTR_INDEX_FIRMWAREVERSION:
	case ATTR_INDEX_BOOTLOADERVERSION:
		memset(attrTable[Index].pData, 0, MAX_VERSION_LENGTH);
		snprintf(attrTable[Index].pData, MAX_VERSION_LENGTH - 1,
			 "%u.%u.%u", Major, Minor, Build);
		break;

	case ATTR_INDEX_HWVERSION:
		memset(attrTable[Index].pData, 0, MAX_VERSION_LENGTH);
		snprintf(attrTable[Index].pData, MAX_VERSION_LENGTH - 1, "%u",
			 Major);
		break;

	default:
		FRAMEWORK_ASSERT(false);
		break;
	}

	// Don't start the timer or set the modified flag.  These values will be obtained after reset.

	k_mutex_unlock(&attribute_mutex);

	return ok;
}

size_t AttributeTask_GetValueAsString(char *pValue, bool *pIsNotString,
				      uint32_t Index, size_t MaxStringLength)
{
	memset(pValue, 0, MaxStringLength);
	*pIsNotString = false;

	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return 0;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	switch (attrTable[Index].type) {
	case UNSIGNED_TYPE: {
		*pIsNotString = true;
		uint32_t value = *((uint32_t *)attrTable[Index].pData);
		snprintf(pValue, MaxStringLength, "%u", value);
	} break;

	case SIGNED_TYPE: {
		*pIsNotString = true;
		int32_t value = *((int32_t *)attrTable[Index].pData);
		snprintf(pValue, MaxStringLength, "%d", value);
	} break;

	case FLOAT_TYPE: {
		*pIsNotString = true;
		float value = *((float *)attrTable[Index].pData);
		snprintf(pValue, MaxStringLength, FLOAT_OUTPUT_SPECIFIER,
			 value);
	} break;

	case STRING_TYPE: {
		*pIsNotString = false;
		size_t maxSize = (attrTable[Index].size - 1); // 1 for NUL
		if (MaxStringLength >= maxSize) {
			strncpy(pValue, attrTable[Index].pData, maxSize);
		}
	} break;

	default:
		FRAMEWORK_DEBUG_ASSERT(false);
		break;
	}

	k_mutex_unlock(&attribute_mutex);

	return strlen(pValue);
}

size_t AttributeTask_GetString(char *pValue, uint32_t Index,
			       size_t MaxStringLength)
{
	bool unusedIsNotString;
	return AttributeTask_GetValueAsString(pValue, &unusedIsNotString, Index,
					      MaxStringLength);
}

bool AttributeTask_GetUint32(uint32_t *pValue, uint32_t Index)
{
	*pValue = 0;
	bool ok = false;

	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	if (attrTable[Index].type == UNSIGNED_TYPE) {
		*pValue = *((uint32_t *)attrTable[Index].pData);
		ok = true;
	} else {
		ok = false;
	}

	k_mutex_unlock(&attribute_mutex);

	FRAMEWORK_DEBUG_ASSERT(ok);
	return ok;
}

bool AttributeTask_GetSigned32(int32_t *pValue, uint32_t Index)
{
	*pValue = 0;
	bool ok = false;

	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	if (attrTable[Index].type == SIGNED_TYPE) {
		*pValue = *((int32_t *)attrTable[Index].pData);
		ok = true;
	} else {
		ok = false;
	}

	k_mutex_unlock(&attribute_mutex);

	FRAMEWORK_DEBUG_ASSERT(ok);
	return ok;
}

bool AttributeTask_GetFloat(float *pValue, uint32_t Index)
{
	*pValue = 0.0;
	bool isFloat = false;

	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	if (attrTable[Index].type == FLOAT_TYPE) {
		*pValue = *((float *)attrTable[Index].pData);
		isFloat = true;
	} else {
		isFloat = false;
	}

	k_mutex_unlock(&attribute_mutex);

	FRAMEWORK_DEBUG_ASSERT(isFloat);
	return isFloat;
}

void AttributeTask_GetName(char *pName, uint32_t Index)
{
	memset(pName, 0, ATTRIBUTE_NAME_MAX_LENGTH);

	if (Index < ATTRIBUTE_TABLE_SIZE) {
		strncpy(pName, attrTable[Index].name,
			ATTRIBUTE_NAME_MAX_LENGTH);
	}
}

bool AttributeTask_IsString(uint32_t Index)
{
	if (Index < ATTRIBUTE_TABLE_SIZE) {
		return (attrTable[Index].type == STRING_TYPE);
	} else {
		return false;
	}
}
bool AttributeTask_IsFloat(uint32_t Index)
{
	if (Index < ATTRIBUTE_TABLE_SIZE) {
		return (attrTable[Index].type == FLOAT_TYPE);
	} else {
		return false;
	}
}
bool AttributeTask_IsUnsigned(uint32_t Index)
{
	if (Index < ATTRIBUTE_TABLE_SIZE) {
		return (attrTable[Index].type == UNSIGNED_TYPE);
	} else {
		return false;
	}
}
bool AttributeTask_IsSigned(uint32_t Index)
{
	if (Index < ATTRIBUTE_TABLE_SIZE) {
		return (attrTable[Index].type == SIGNED_TYPE);
	} else {
		return false;
	}
}

size_t AttributeTask_GetMaxPairSize(uint32_t Index)
{
	size_t size = 0;
	if (Index >= ATTRIBUTE_TABLE_SIZE) {
		return size;
	}

	size += strlen(attrTable[Index].name);
	size += 2; // for ""

	switch (attrTable[Index].type) {
	case UNSIGNED_TYPE:
		size += ATTRIBUTE_MAX_INT_DIGITS;
		break;
	case SIGNED_TYPE:
		size += ATTRIBUTE_MAX_INT_DIGITS;
		break;
	case FLOAT_TYPE:
		size += ATTRIBUTE_MAX_FLOAT_DIGITS;
		break;
	case STRING_TYPE:
		size += attrTable[Index].size + 2;
		break;
	default:
		break;
	}

	return size;
}

//=================================================================================================
// Local Function Definitions
//=================================================================================================
static void AttrTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	AttrTaskObj_t *pObj = (AttrTaskObj_t *)pArg1;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static DispatchResult_t
InitializeAttributesMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	k_mutex_lock(&attribute_mutex, K_FOREVER);

	uint16_t currentSize = 0;//(uint16_t)Attribute_GetRwSize();
	uint16_t savedSize = 0;
	bool found = 0;
//		found = NvSparse_InitItem(NV_FILE_ID_ATTRIBUTES, NV_ITEM_ID_ATTRIBUTES,
//				  Attribute_GetRwPointer(), currentSize,
//				  &savedSize);

  if(found == true)
  {
    LOG_DBG("Attributes found in flash");
  }
  else
  {
    LOG_DBG("Attributes NOT found in flash");
  }
	FRAMEWORK_DEBUG_ASSERT(currentSize >=
			       savedSize); // Attributes can't shrink
	if (currentSize != savedSize) {
		WriteAttributes();
	}

	pMsg->header.rxId = FWK_ID_RESERVED;
	pMsg->header.txId = pMsgRxer->id;
	pMsg->header.msgCode = FMC_ATTRIBUTES_READY;

	k_mutex_unlock(&attribute_mutex);

	FRAMEWORK_MSG_UNICAST(pMsg);
	return DISPATCH_DO_NOT_FREE;
}

static void WriteAttributes(void)
{
	// This occurs when mutex is taken or during init.

	bool updated = 0;
	//updated =	NvSparse_WriteItem(NV_FILE_ID_ATTRIBUTES, NV_ITEM_ID_ATTRIBUTES,
	//			   Attribute_GetRwPointer(),
	//			   Attribute_GetRwSize());
  if(updated == true)
  {
    LOG_DBG("Attributes updated in flash");
  }
  else
  {
    LOG_DBG("Attributes NOT updated in flash");
  }
}

static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);

  k_mutex_lock(&attribute_mutex, K_FOREVER);
  LOG_DBG("Restoring default attributes");
	Attribute_SetNonBackupValuesToDefault();
	WriteAttributes();
  k_mutex_unlock(&attribute_mutex);

	//pObj->factoryResetImminent = true;

	// The control task will reset the processor.
	pMsg->header.rxId = FWK_ID_CONTROL_TASK;
	pMsg->header.txId = pMsgRxer->id;
	pMsg->header.msgCode = FMC_SOFTWARE_RESET;
	FRAMEWORK_MSG_SEND(pMsg);
	return DISPATCH_DO_NOT_FREE;
}

static DispatchResult_t AttrTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);

	//if (pObj->factoryResetImminent) {
	//	return DISPATCH_OK;
	//}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	size_t j;
	bool writeRequired = false;
	for (j = 0; j < ATTRIBUTE_TABLE_SIZE; j++) {
		if (attrTable[j].modified &&
		    (attrTable[j].category == READ_WRITE)) {
			writeRequired = true;
		}
	}

	if (writeRequired) {
		WriteAttributes();
	}

	//
	// Send the update message after the values have been updated.  This is so tasks aren't
	// blocked when they try to read new values.
	//
	//GenerateAttributeChangedMessage(pObj);

	k_mutex_unlock(&attribute_mutex);

	return DISPATCH_OK;
}

static void GenerateAttributeChangedMessage(AttrTaskObj_t *pObj)
{
}

static void StartDelayedWriteTimer(AttributeEntry_t *pEntry)
{
//	if (pEntry->modified &&
//	    ((pEntry->category == READ_WRITE) || pEntry->broadcast)) {
//		FWK_TRACE0(FWK_TRACE_NONE_LEVEL, "Delayed Write Timer Started");
//		Framework_StartTimer(attrTaskObj.msgTask.pTimer);
//	}
}

//
static bool isWritable(uint32_t Index, uint32_t SourceId)
{
	if (SourceId ==
	    FWK_ID_RESERVED) // internal writes are always allowed
	{
		return true;
	} else if (attrTable[Index].category == READ_WRITE) {
		// If the attribute is lockable then read the lock attribute.
		return (attrTable[Index].lockable != 0) ?
			       (*(uint32_t *)(attrTable[ATTR_INDEX_LOCK]
						      .pData) == 0) :
			       true;
	} else {
		return false;
	}
}

// end
