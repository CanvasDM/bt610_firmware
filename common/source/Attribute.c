/**
 * @file Attribute.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(attr, CONFIG_ATTRIBUTES_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <stdio.h>

#include "lcz_params.h"
#include "file_system_utilities.h"

#include "AttributeTable.h"
#include "Attribute.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
static const char EMPTY_STRING[] = "";

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(attribute_mutex);

extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int SaveAttributes(void);
static int LoadAttributes(const char *fname);

static int Validate(attr_idx_t Index, void *pValue, size_t Length,
		    bool DoWrite);

extern void AttributeTable_Initialize(void);
extern void AttributeTable_FactoryReset(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AttributesInit(void)
{
	int r = -EPERM;
	k_mutex_lock(&attribute_mutex, K_FOREVER);
	AttributeTable_Initialize();
	/* The file may not exist yet */
	if (fsu_single_entry_exists(CONFIG_LCZ_PARAMS_MOUNT_POINT,
				    CONFIG_ATTRIBUTES_FILE_NAME,
				    FS_DIR_ENTRY_FILE) == -ENOENT) {
		r = 0;
		LOG_INF("Parameter file doesn't exist");
	} else {
		r = LoadAttributes(CONFIG_ATTRIBUTES_FILE_NAME);
	}
	k_mutex_unlock(&attribute_mutex);

	LOG_DBG("Init status: %d", r);
	return r;
}

AttributeType_t Attribute_GetType(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return attrTable[Index].type;
	} else {
		return UNKNOWN_TYPE;
	}
}

int Attribute_Set(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		r = Validate(Index, pValue, ValueLength, false);
		if (r == 0) {
			k_mutex_lock(&attribute_mutex, K_FOREVER);
			(void)Validate(Index, pValue, ValueLength, true);
			r = SaveAttributes();
			k_mutex_unlock(&attribute_mutex);
		}
	}
	return r;
}

int Attribute_SetString(attr_idx_t Index, char const *pValue,
			size_t ValueLength)
{
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == STRING_TYPE) {
			r = Validate(Index, (void *)pValue, ValueLength, true);
		}
		if (r == 0) {
			r = SaveAttributes();
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetUint32(attr_idx_t Index, uint32_t Value)
{
	uint32_t local = Value;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == UNSIGNED_TYPE) {
			r = Validate(Index, &local, sizeof(local), true);
		}
		if (r == 0) {
			r = SaveAttributes();
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetSigned32(attr_idx_t Index, int32_t Value)
{
	int32_t local = Value;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == SIGNED_TYPE) {
			r = Validate(Index, &local, sizeof(local), true);
		}
		if (r == 0) {
			r = SaveAttributes();
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetFloat(attr_idx_t Index, float Value)
{
	float local = Value;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == FLOAT_TYPE) {
			r = Validate(Index, &local, sizeof(local), true);
		}
		if (r == 0) {
			r = SaveAttributes();
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

#if 0
bool Attribute_SetVersion(attr_idx_t Index, uint32_t Major, uint32_t Minor,
			  uint32_t Build)
{
	if (Index >= ATTR_TABLE_SIZE) {
		return false;
	}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	bool ok = false;
	switch (Index) {
	case ATTR_INDEX_FIRMWAREVERSION:
	case ATTR_INDEX_BOOTLOADERVERSION:
		memset(attrTable[Index].pData, 0, ATTR_MAX_VERSION_LENGTH);
		snprintf(attrTable[Index].pData, ATTR_MAX_VERSION_LENGTH - 1,
			 "%u.%u.%u", Major, Minor, Build);
		break;

	case ATTR_INDEX_HWVERSION:
		memset(attrTable[Index].pData, 0, ATTR_MAX_VERSION_LENGTH);
		snprintf(attrTable[Index].pData, ATTR_MAX_VERSION_LENGTH - 1,
			 "%u", Major);
		break;

	default:
		__ASSERT(false, "Invalid version");
		break;
	}

	// Don't start the timer or set the modified flag.  These values will be obtained after reset.

	k_mutex_unlock(&attribute_mutex);

	return ok;
}
#endif

int Attribute_GetString(char *pValue, attr_idx_t Index, size_t MaxStringLength)
{
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		strncpy(pValue, attrTable[Index].pData, MaxStringLength);
		r = 0;
	}
	return r;
}

int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == UNSIGNED_TYPE) {
			*pValue = *((uint32_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == SIGNED_TYPE) {
			*pValue = *((int32_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_GetFloat(float *pValue, attr_idx_t Index)
{
	*pValue = 0.0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == FLOAT_TYPE) {
			*pValue = *((float *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

const char *Attribute_GetName(attr_idx_t Index)
{
	const char *p = EMPTY_STRING;
	if (Index < ATTR_TABLE_SIZE) {
		p = (const char *)attrTable[Index].name;
	}
	return p;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* Convert the attribute type into the parameter file type */
static param_t ConvertParameterType(attr_idx_t idx)
{
	if (attrTable[idx].type == STRING_TYPE) {
		return PARAM_STR;
	} else {
		return PARAM_BIN;
	}
}

static size_t GetParameterLength(attr_idx_t idx)
{
	if (attrTable[idx].type == STRING_TYPE) {
		return strlen(attrTable[idx].pData);
	} else {
		return 4;
	}
}

static int SaveAttributes(void)
{
	int r = -EPERM;
	attr_idx_t i;
	char *fstr;

	/* Converting to file format is larger, but makes it easier to go between
	 * different versions.
	 */
	k_mutex_lock(&attribute_mutex, K_FOREVER);
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (!attrTable[i].deprecated) {
			r = lcz_params_generate_file(i, ConvertParameterType(i),
						     attrTable[i].pData,
						     GetParameterLength(i),
						     &fstr);
			if (r < 0) {
				LOG_ERR("Error converting attribute table into file");
				break;
			}
		}
	}
	if (r >= 0) {
		r = lcz_params_validate_file(fstr, strlen(fstr));
	}
	if (r >= 0) {
		r = (int)lcz_params_write(CONFIG_ATTRIBUTES_FILE_NAME, fstr,
					  strlen(fstr));
	}
	k_mutex_unlock(&attribute_mutex);

	return (int)r;
}

static int LoadAttributes(const char *fname)
{
	int r = -EPERM;
	bool failure;
	size_t fsize;
	char *fstr = NULL;
	attr_idx_t i = 0;
	param_kvp_t *kvp = NULL;

	r = lcz_params_parse_from_file(fname, &fsize, &fstr, &kvp);
	LOG_DBG("pairs: %d fsize: %d", r, fsize);

	if (r > 0) {
		for (i = 0; i < r; i++) {
			if (Attribute_Set(kvp->id, kvp->keystr, kvp->length) <
			    0) {
				failure = true;
				LOG_ERR("Failed to set %s from %s",
					log_strdup(Attribute_GetName(i)),
					log_strdup(fname));
			}
		}
		k_free(kvp);
		k_free(fstr);
		if (failure) {
			r = -EINVAL;
		}
	}

	return r;
}

#if 0
static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);

	k_mutex_lock(&attribute_mutex, K_FOREVER);
	LOG_DBG("Restoring default attributes");
	Attribute_SetNonBackupValuesToDefault();
	SaveAttributes();
	k_mutex_unlock(&attribute_mutex);

	// The control task will reset the processor.
	pMsg->header.rxId = FWK_ID_CONTROL_TASK;
	pMsg->header.txId = pMsgRxer->id;
	pMsg->header.msgCode = FMC_SOFTWARE_RESET;
	FRAMEWORK_MSG_SEND(pMsg);
	return DISPATCH_DO_NOT_FREE;
}

static DispatchResult_t AttrTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);

	//if (pObj->factoryResetImminent) {
	//	return DISPATCH_OK;
	//}

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	size_t j;
	bool writeRequired = false;
	for (j = 0; j < ATTR_TABLE_SIZE; j++) {
		if (attrTable[j].modified &&
		    (attrTable[j].category == READ_WRITE)) {
			writeRequired = true;
		}
	}

	if (writeRequired) {
		SaveAttributes();
	}

	//
	// Send the update message after the values have been updated.  This is so tasks aren't
	// blocked when they try to read new values.
	//
	//GenerateAttributeChangedMessage(pObj);

	k_mutex_unlock(&attribute_mutex);

	return DISPATCH_OK;
}
#endif

bool Attribute_IsString(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == STRING_TYPE);
	} else {
		return false;
	}
}

bool Attribute_IsFloat(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == FLOAT_TYPE);
	} else {
		return false;
	}
}

bool Attribute_IsUnsigned(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == UNSIGNED_TYPE);
	} else {
		return false;
	}
}

bool Attribute_IsSigned(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == SIGNED_TYPE);
	} else {
		return false;
	}
}

bool Attribute_IsWritable(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return attrTable[Index].writeable;
	} else {
		return false;
	}
}

static int Validate(attr_idx_t Index, void *pValue, size_t Length, bool DoWrite)
{
	AttributeEntry_t *pEntry = &attrTable[Index];
	return pEntry->pValidator(pEntry, pValue, Length, DoWrite);
}