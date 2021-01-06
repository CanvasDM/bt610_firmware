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

#include "AnalogInput.h"
#include "AttributeTable.h"

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int validate_analog_input_config(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AttributeValidator_string(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	int r = -EPERM;

	/* -1 to account for NULL */
	if (pEntry->size > Length) {
		/* Don't use strncmp because pValue may not be NUL terminated */
		size_t currentLength = strlen(pEntry->pData);
		if (DoWrite && ((memcmp(pEntry->pData, pValue, Length) != 0) ||
				Length == 0 || currentLength != Length)) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, pValue, Length);
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_uint64(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	uint64_t value = *(uint64_t *)pValue;

	if (DoWrite && value != *((uint64_t *)pEntry->pData)) {
		pEntry->modified = true;
		*((uint64_t *)pEntry->pData) = value;
	}
	return 0;
}

int AttributeValidator_uint32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = *(uint32_t *)pValue;

	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint32_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_uint16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint16_t *)pValue);

	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint16_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_uint8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint8_t *)pValue);

	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_int64(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int64_t value = *(int64_t *)pValue;

	if (DoWrite && value != *((int64_t *)pEntry->pData)) {
		pEntry->modified = true;
		*((int64_t *)pEntry->pData) = value;
	}
	return 0;
}

int AttributeValidator_int32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = *(int32_t *)pValue;
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_int16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int16_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int16_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_int8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int8_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int8_t *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_float(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	float value = *((float *)pValue);
	float min = (float)pEntry->min;
	float max = (float)pEntry->max;

	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((float *)pEntry->pData)) {
			pEntry->modified = true;
			*((float *)pEntry->pData) = value;
		}
		r = 0;
	}
	return r;
}

int AttributeValidator_aic(AttributeEntry_t *pEntry, void *pValue,
			   size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint8_t saved = *((uint8_t *)pEntry->pData);
	r = AttributeValidator_uint8(pEntry, pValue, Length, false);
	if (r == 0) {
		/* Assume value is ok.  This makes secondary validation simpler
		 * because it is independent of the channel being changed.
		 */
		*((uint8_t *)pEntry->pData) = *(uint8_t *)pValue;

		r = validate_analog_input_config();
		if (r < 0 || !DoWrite) {
			*((uint8_t *)pEntry->pData) = saved;
			pEntry->modified = false;
		} else {
			pEntry->modified = true;
		}
	}

	if (r < 0) {
		LOG_ERR("Invalid analog input configuration");
	}
	return r;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int validate_analog_input_config(void)
{
	int pressure_sensors = 0;
	int ultrasonic_sensors = 0;
	uint32_t ch;
	size_t i;

	/* This assumes the 4 channels have consecutive IDs. */
	for (i = 0; i < ANALOG_INPUT_NUMBER_OF_CHANNELS; i++) {
		ch = 0;
		memcpy(&ch, attrTable[ATTR_INDEX_analogInput1Type + i].pData,
		       attrTable[ATTR_INDEX_analogInput1Type + i].size);

		switch (ch) {
		case ANALOG_INPUT_PRESSURE:
			pressure_sensors += 1;
			break;
		case ANALOG_INPUT_ULTRASONIC:
			ultrasonic_sensors += 1;
			break;
		default:
			/* There aren't any restrictions on the number of voltage or
			 * current sense inputs.
			 */
			break;
		}
	}

	if (ultrasonic_sensors > ANALOG_INPUTS_MAX_ULTRASONIC ||
	    pressure_sensors > ANALOG_INPUTS_MAX_PRESSURE_SENSORS) {
		return -EPERM;
	} else if (ultrasonic_sensors > 0 &&
		   (pressure_sensors >
		    ANALOG_INPUTS_MAX_PRESSURE_SENSORS_WITH_ULTRASONIC)) {
		return -EPERM;
	} else {
		return 0;
	}
}
