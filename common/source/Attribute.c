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
		r = LoadAttributes(CONFIG_LCZ_PARAMS_MOUNT_POINT
				   "/" CONFIG_ATTRIBUTES_FILE_NAME);
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
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Validate(Index, pValue, ValueLength, false);
		if (r == 0) {
			r = Validate(Index, pValue, ValueLength, true);
			if (r == ATTR_MODIFIED) {
				r = SaveAttributes();
			}
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	memset(pValue, 0, ValueLength);
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);

		memcpy(pValue, attrTable[Index].pData, attrTable[Index].size);
		r = 0;
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}

int Attribute_Load(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		r = Validate(Index, pValue, ValueLength, true);
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_GetString(char *pValue, attr_idx_t Index, size_t MaxStringLength)
{
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		strncpy(pValue, attrTable[Index].pData, MaxStringLength);
		r = 0;
	}
	return r;
}
int Attribute_GetUint8(uint8_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == UNSIGNED_EIGHT_BIT_TYPE) {
			*pValue = *((uint8_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_GetUint16(uint16_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == UNSIGNED_SIXTEEN_BIT_TYPE) {
			*pValue = *((uint16_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == UNSIGNED_THIRTY_TWO_BIT_TYPE) {
			*pValue = *((uint32_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_GetSigned8(int8_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == SIGNED_EIGHT_BIT_TYPE) {
			*pValue = *((int8_t *)attrTable[Index].pData);
			r = 0;
		}
		k_mutex_unlock(&attribute_mutex);
	}
	return r;
}
int Attribute_GetSigned16(int16_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (Index < ATTR_TABLE_SIZE) {
		k_mutex_lock(&attribute_mutex, K_FOREVER);
		if (attrTable[Index].type == SIGNED_SIXTEEN_BIT_TYPE) {
			*pValue = *((int16_t *)attrTable[Index].pData);
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
		if (attrTable[Index].type == SIGNED_THIRTY_TWO_BIT_TYPE) {
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
		return attrTable[idx].size;
	}
}

static int SaveAttributes(void)
{
	int r = -EPERM;
	attr_idx_t i;
	char *fstr = NULL;

	k_mutex_lock(&attribute_mutex, K_FOREVER);

	/* Converting to file format is larger, but makes it easier to go between
	 * different versions.
	 */
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (attrTable[i].writeable && !attrTable[i].deprecated) {
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
		LOG_DBG("Write %d (%d) bytes of parameters to file",
			strlen(fstr), r);
	}

	k_free(fstr);
	k_mutex_unlock(&attribute_mutex);

	return (r < 0) ? r : 0;
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
				load_status =
					Attribute_Load(kvp[i].id, kvp[i].keystr,
						       kvp[i].length);
			} else {
				binlen = hex2bin(kvp[i].keystr, kvp[i].length,
						 bin, sizeof(bin));
				if (binlen <= 0) {
					load_status = -1;
				} else {
					load_status = Attribute_Load(
						kvp[i].id, bin, binlen);
				}
			}
			if (load_status < 0) {
				break;
			}
		}

		if (load_status < 0) {
			LOG_ERR("Failed to set id: 0x%x '%s'", kvp[i].id,
				log_strdup(Attribute_GetName(kvp[i].id)));
			LOG_HEXDUMP_DBG(kvp[i].keystr, kvp[i].length,
					"kvp data");
		}
		k_free(kvp);
		k_free(fstr);
		if (load_status < 0) {
			r = -EINVAL;
		}
	}

	return r;
}

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

bool Attribute_IsUnint8(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == UNSIGNED_EIGHT_BIT_TYPE);
	} else {
		return false;
	}
}
bool Attribute_IsUnint16(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == UNSIGNED_SIXTEEN_BIT_TYPE);
	} else {
		return false;
	}
}
bool Attribute_IsUnint32(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == UNSIGNED_THIRTY_TWO_BIT_TYPE);
	} else {
		return false;
	}
}

bool Attribute_IsInt8(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == SIGNED_EIGHT_BIT_TYPE);
	} else {
		return false;
	}
}
bool Attribute_IsInt16(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == SIGNED_SIXTEEN_BIT_TYPE);
	} else {
		return false;
	}
}
bool Attribute_IsInt32(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return (attrTable[Index].type == SIGNED_THIRTY_TWO_BIT_TYPE);
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