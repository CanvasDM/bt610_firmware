/**
 * @file Attribute.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(attr, CONFIG_ATTR_LOG_LEVEL);

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
#define BREAK_ON_ERROR(x)                                                      \
	if (x < 0) {                                                           \
		break;                                                         \
	}

static const char EMPTY_STRING[] = "";

#define FLOAT_MAX_STR_SIZE (25 + 4)

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(attribute_mutex);

extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
#if 0
static int SaveAndBroadcast(void);
#endif

static int SaveAndBroadcastSingle(attr_idx_t Index);
static int SaveAttributes(void);
static int LoadAttributes(const char *fname);

#if 0
static void Broadcast(void);
#endif
static void BroadcastSingle(attr_idx_t Index);

static int Load(attr_idx_t Index, void *pValue, size_t ValueLength);
static int Validate(attr_idx_t Index, AttrType_t Type, void *pValue,
		    size_t Length);
static int Write(attr_idx_t Index, AttrType_t Type, void *pValue,
		 size_t Length);

static bool IsWritable(attr_idx_t Index);

extern void AttributeTable_Initialize(void);
extern void AttributeTable_FactoryReset(void);

static void Show(attr_idx_t Index);
static bool ValidIndex(attr_idx_t Index);

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
				    CONFIG_ATTR_FILE_NAME,
				    FS_DIR_ENTRY_FILE) == -ENOENT) {
		r = 0;
		LOG_INF("Parameter file doesn't exist");
	} else {
		r = LoadAttributes(CONFIG_LCZ_PARAMS_MOUNT_POINT
				   "/" CONFIG_ATTR_FILE_NAME);
	}

	k_mutex_unlock(&attribute_mutex);

	LOG_INF("Load status: %d", r);
	return r;
}

AttrType_t Attribute_GetType(attr_idx_t Index)
{
	if (ValidIndex(Index)) {
		return attrTable[Index].type;
	} else {
		return ATTR_TYPE_UNKNOWN;
	}
}

int Attribute_Set(attr_idx_t Index, AttrType_t Type, void *pValue,
		  size_t ValueLength)
{
	int r = -EPERM;

	if (ValidIndex(Index)) {
		if (IsWritable(Index)) {
			k_mutex_lock(&attribute_mutex, K_FOREVER);
			r = Validate(Index, Type, pValue, ValueLength);
			if (r == 0) {
				r = Write(Index, Type, pValue, ValueLength);
				if (r == 0) {
					r = SaveAndBroadcastSingle(Index);
				}
			}
			k_mutex_unlock(&attribute_mutex);
		}
	}
	return r;
}

int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	memset(pValue, 0, ValueLength);
	size_t size = MIN(attrTable[Index].size, ValueLength);
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		memcpy(pValue, attrTable[Index].pData, size);
		r = size;
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetString(attr_idx_t Index, char const *pValue,
			size_t ValueLength)
{
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Write(Index, ATTR_TYPE_STRING, (void *)pValue, ValueLength);
		if (r == 0) {
			r = SaveAndBroadcastSingle(Index);
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_GetString(char *pValue, attr_idx_t Index, size_t MaxStringLength)
{
	int r = -EPERM;

	if (ValidIndex(Index)) {
		strncpy(pValue, attrTable[Index].pData, MaxStringLength);
		r = 0;
	}
	return r;
}

int Attribute_SetUint32(attr_idx_t Index, uint32_t Value)
{
	uint32_t local = Value;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcastSingle(Index);
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetSigned32(attr_idx_t Index, int32_t Value)
{
	int32_t local = Value;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcastSingle(Index);
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_SetFloat(attr_idx_t Index, float Value)
{
	float local = Value;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Write(Index, ATTR_TYPE_FLOAT, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcastSingle(Index);
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_U32) {
			k_mutex_lock(&attribute_mutex, K_FOREVER);
			*pValue = *((uint32_t *)attrTable[Index].pData);
			r = 0;
			k_mutex_unlock(&attribute_mutex);
		}
	}
	return r;
}

int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_S32) {
			k_mutex_lock(&attribute_mutex, K_FOREVER);
			*pValue = *((int32_t *)attrTable[Index].pData);
			r = 0;
			k_mutex_unlock(&attribute_mutex);
		}
	}
	return r;
}

int Attribute_GetFloat(float *pValue, attr_idx_t Index)
{
	*pValue = 0.0;
	int r = -EPERM;

	if (ValidIndex(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_FLOAT) {
			k_mutex_lock(&attribute_mutex, K_FOREVER);
			*pValue = *((float *)attrTable[Index].pData);
			r = 0;
			k_mutex_unlock(&attribute_mutex);
		}
	}
	return r;
}

/* todo: check type? */
uint32_t Attribute_AltGetUint32(attr_idx_t Index, uint32_t Default)
{
	uint32_t v = Default;
	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		v = 0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		k_mutex_unlock(&attribute_mutex);
	}
	return v;
}

int32_t Attribute_AltGetSigned32(attr_idx_t Index, int32_t Default)
{
	int32_t v = Default;
	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		v = 0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		k_mutex_unlock(&attribute_mutex);
	}
	return v;
}

float Attribute_AltGetFloat(attr_idx_t Index, float Default)
{
	float v = Default;
	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		v = 0.0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		k_mutex_unlock(&attribute_mutex);
	}
	return v;
}

const char *Attribute_GetName(attr_idx_t Index)
{
	const char *p = EMPTY_STRING;
	if (ValidIndex(Index)) {
		p = (const char *)attrTable[Index].name;
	}
	return p;
}

size_t Attribute_GetSize(attr_idx_t Index)
{
	size_t size = 0;
	if (ValidIndex(Index)) {
		size = attrTable[Index].size;
	}
	return size;
}

#ifdef CONFIG_ATTR_SHELL

attr_idx_t Attribute_GetIndex(const char *Name)
{
	attr_idx_t i;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (strcmp(Name, attrTable[i].name) == 0) {
			break;
		}
	}
	return i;
}

int Attribute_Show(attr_idx_t Index)
{
	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		Show(Index);
		k_mutex_unlock(&attribute_mutex);
		return 0;
	} else {
		return -EINVAL;
	}
}

#endif /* CONFIG_ATTR_SHELL */

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* Convert the attribute type into the parameter file type */
static param_t ConvertParameterType(attr_idx_t idx)
{
	if (attrTable[idx].type == ATTR_TYPE_STRING) {
		return PARAM_STR;
	} else {
		return PARAM_BIN;
	}
}

static size_t GetParameterLength(attr_idx_t idx)
{
	if (attrTable[idx].type == ATTR_TYPE_STRING) {
		return strlen(attrTable[idx].pData);
	} else {
		return attrTable[idx].size;
	}
}

#if 0
static int SaveAndBroadcast(void)
{
	int r = SaveAttributes();
	/* Broadcast after save is complete because attributes access is wrapped by mutex */
	Broadcast();
	return r;
}
#endif

static int SaveAndBroadcastSingle(attr_idx_t Index)
{
	int r = 0;
	AttributeEntry_t *p = &attrTable[Index];

	if (p->modified) {
		if (p->savable && !p->deprecated) {
			r = SaveAttributes();
		}
		BroadcastSingle(Index);
		Show(Index);
	}
	return r;
}

static int SaveAttributes(void)
{
	int r = -EPERM;
	char *fstr = NULL;
	attr_idx_t i;

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	/* Converting to file format is larger, but makes it easier to go between
	 * different versions.
	 */
	do {
		for (i = 0; i < ATTR_TABLE_SIZE; i++) {
			if (attrTable[i].savable && !attrTable[i].deprecated) {
				r = lcz_params_generate_file(
					i, ConvertParameterType(i),
					attrTable[i].pData,
					GetParameterLength(i), &fstr);
				if (r < 0) {
					LOG_ERR("Error converting attribute table into file");
					break;
				}
			}
		}
		BREAK_ON_ERROR(r);

		r = lcz_params_validate_file(fstr, strlen(fstr));
		BREAK_ON_ERROR(r);

		r = (int)lcz_params_write(CONFIG_ATTR_FILE_NAME, fstr,
					  strlen(fstr));
		LOG_DBG("Wrote %d of %d bytes of parameters to file", r,
			strlen(fstr));

	} while (0);

	k_free(fstr);
	k_mutex_unlock(&attribute_mutex);

	return (r < 0) ? r : 0;
}

#if 0
static void Broadcast(void)
{
#ifdef CONFIG_ATTR_BROADCAST
	size_t msgSize = sizeof(AttrBroadcastMsg_t);
	AttrBroadcastMsg_t *pb = BufferPool_Take(msgSize);

	if (pb == NULL) {
		LOG_ERR("Unable to allocate memory for attr broadcast");
	}

	pb->header.msgCode = FMC_ATTR_CHANGED;
	pb->header.txId = FWK_ID_RESERVED;
	pb->header.rxId = FWK_ID_RESERVED;

	LOG_DBG("Broadcast");

	size_t i;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (attrTable[i].modified && attrTable[i].broadcast) {
			pb->list[pb->count++] = (uint8_t)i;
			if (IS_ENABLED(CONFIG_ATTR_BROADCAST_NAME)) {
				LOG_DBG("\t%s", attrTable[i].name);
			}
		}
		attrTable[i].modified = false;
	}

	/* no one may have registered for message */
	if (Framework_Broadcast((FwkMsg_t *)pb, msgSize) != FWK_SUCCESS) {
		pb->count = 0;
	}

	if (pb->count == 0) {
		BufferPool_Free(pb);
	}
#endif
}
#endif

static void BroadcastSingle(attr_idx_t Index)
{
#ifdef CONFIG_ATTR_BROADCAST
	if (!attrTable[Index].modified || !attrTable[Index].broadcast) {
		return;
	}

	size_t msgSize = sizeof(AttrBroadcastMsg_t);
	AttrBroadcastMsg_t *pb = BufferPool_Take(msgSize);

	if (pb == NULL) {
		LOG_ERR("Unable to allocate memory for attr broadcast");
	}

	pb->header.msgCode = FMC_ATTR_CHANGED;
	pb->header.txId = FWK_ID_RESERVED;
	pb->header.rxId = FWK_ID_RESERVED;
	pb->list[pb->count++] = (uint8_t)Index;

	attrTable[Index].modified = false;

	/* no one may have registered for message */
	if (Framework_Broadcast((FwkMsg_t *)pb, msgSize) != FWK_SUCCESS) {
		pb->count = 0;
	}

	if (pb->count == 0) {
		BufferPool_Free(pb);
	}
#endif
}

void Show(attr_idx_t Index)
{
	AttributeEntry_t *p = &attrTable[Index];
	uint32_t d = 0;
	int32_t i = 0;
	float f = 0.0;
	char float_str[FLOAT_MAX_STR_SIZE];

	switch (p->type) {
	case ATTR_TYPE_U8:
	case ATTR_TYPE_U16:
	case ATTR_TYPE_U32:
		memcpy(&d, p->pData, p->size);
		LOG_DBG("[%u] %s %u", Index, p->name, d);
		break;

	case ATTR_TYPE_S8:
	case ATTR_TYPE_S16:
	case ATTR_TYPE_S32:
		memcpy(&i, p->pData, p->size);
		LOG_DBG("[%u] %s %u", Index, p->name, i);
		break;

	case ATTR_TYPE_FLOAT:
		memcpy(&f, p->pData, p->size);
		snprintf(float_str, sizeof(float_str), "%.4f", f);
		LOG_DBG("[%u] %s %s", Index, p->name, log_strdup(float_str));
		break;

	case ATTR_TYPE_STRING:
		LOG_DBG("[%u] %s '%s'", Index, p->name,
			log_strdup((char *)p->pData));
		break;

	case ATTR_TYPE_UNKNOWN:
	case ATTR_TYPE_ANY:
	default:
		LOG_HEXDUMP_DBG(p->pData, p->size, p->name);
		break;
	}
}

static int LoadAttributes(const char *fname)
{
	int r = -EPERM;
	size_t fsize;
	char *fstr = NULL;
	attr_idx_t i = 0;
	param_kvp_t *kvp = NULL;
	size_t binlen;
	uint8_t bin[ATTR_MAX_HEX_SIZE];
	int load_status;

	r = lcz_params_parse_from_file(fname, &fsize, &fstr, &kvp);
	LOG_DBG("pairs: %d fsize: %d file: %s", r, fsize, log_strdup(fname));

	if (r > 0) {
		for (i = 0; i < r; i++) {
			if (ConvertParameterType(i) == PARAM_STR) {
				load_status = Load(kvp[i].id, kvp[i].keystr,
						   kvp[i].length);
			} else {
				binlen = hex2bin(kvp[i].keystr, kvp[i].length,
						 bin, sizeof(bin));
				if (binlen <= 0) {
					load_status = -1;
				} else {
					load_status =
						Load(kvp[i].id, bin, binlen);
				}
			}
			if (load_status < 0) {
				LOG_ERR("Failed to set id: 0x%x '%s'",
					kvp[i].id,
					log_strdup(
						Attribute_GetName(kvp[i].id)));
				LOG_HEXDUMP_DBG(kvp[i].keystr, kvp[i].length,
						"kvp data");
			}
			if (IS_ENABLED(CONFIG_ATTR_BREAK_ON_LOAD_FAILURE)) {
				if (load_status < 0) {
					break;
				}
			}
		}

		k_free(kvp);
		k_free(fstr);
		if (load_status < 0) {
			r = -EINVAL;
		}
	}

	return r;
}

static int Load(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	int r = -EPERM;

	if (ValidIndex(Index)) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Write(Index, ATTR_TYPE_ANY, pValue, ValueLength);
		attrTable[Index].modified = false;
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

static int Validate(attr_idx_t Index, AttrType_t Type, void *pValue,
		    size_t Length)
{
	int r = -EPERM;
	AttributeEntry_t *p = &attrTable[Index];

	if (Type == p->type || Type == ATTR_TYPE_ANY) {
		r = p->pValidator(p, pValue, Length, false);
	}

	if (r < 0) {
		LOG_WRN("failure %u %s", Index, p->name);
		LOG_HEXDUMP_DBG(pValue, Length, "attr data");
	}
	return r;
}

static int Write(attr_idx_t Index, AttrType_t Type, void *pValue, size_t Length)
{
	int r = -EPERM;
	AttributeEntry_t *p = &attrTable[Index];

	if (Type == p->type || Type == ATTR_TYPE_ANY) {
		r = p->pValidator(p, pValue, Length, true);
	}

	if (r < 0) {
		LOG_WRN("validation failure %u %s", Index, p->name);
		LOG_HEXDUMP_DBG(pValue, Length, "attr data");
	}
	return r;
}

static bool IsWritable(attr_idx_t Index)
{
	bool r = false;
	bool unlocked = ((*((uint8_t *)attrTable[ATTR_INDEX_lock].pData)) == 0);
	AttributeEntry_t *p = &attrTable[Index];

	if (p->writable) {
		if (p->lockable) {
			r = unlocked;
		} else {
			r = true;
		}
	}

	if (!r) {
		LOG_DBG("[%u] %s is Not writable", Index, p->name);
	}
	return r;
}

static bool ValidIndex(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return true;
	} else {
		LOG_ERR("Invalid index %u", Index);
		return false;
	}
}
