/**
 * @file AttributeValidator.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(attrval, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <ctype.h>

#include "AttributeTable.h"

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AttributeValidator_string(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	if (pEntry->size > Length) /* -1 to account for NULL */
	{
		/* Don't use strncmp because pValue may not be NUL terminated */
		size_t currentLength = strlen(pEntry->pData);
		if (DoWrite && ((memcmp(pEntry->pData, pValue, Length) != 0) ||
				Length == 0 || currentLength != Length)) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, pValue, Length);
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_uint32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	uint32_t value = *((uint32_t *)pValue);

	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint32_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_uint16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	uint16_t value = *((uint16_t *)pValue);

	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint16_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_uint8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	uint8_t value = *((uint8_t *)pValue);
	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_int32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int32_t value = *((int32_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_int16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int16_t value = *((int16_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int16_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_int8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int8_t value = *((int8_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int8_t *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}

int AttributeValidator_float(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	float value = *((float *)pValue);
	float min = (float)pEntry->min;
	float max = (float)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((float *)pEntry->pData)) {
			pEntry->modified = true;
			*((float *)pEntry->pData) = value;
		}
		return 0;
	}
	return -1;
}
