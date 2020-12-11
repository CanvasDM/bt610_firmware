//=================================================================================================
//!
#define THIS_FILE "AttributeValidator.c"
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
//=================================================================================================

//=================================================================================================
// Includes
//=================================================================================================
#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <ctype.h>

#include "FrameworkIncludes.h"
//#include "Attribute.h"
#include "AttributePrivate.h"
#include "AttributeValidator.h"
#include "AttributeFunctions.h"

//=================================================================================================
// Local Constant, Macro and Type Definitions
//=================================================================================================

//=================================================================================================
// Global Data Definitions
//=================================================================================================

extern AttributeEntry_t attrTable[ATTRIBUTE_TABLE_SIZE];

//=================================================================================================
// Local Data Definitions
//=================================================================================================

//=================================================================================================
// Local Function Prototypes
//=================================================================================================

//=================================================================================================
// Global Function Definitions
//=================================================================================================

bool AttributeValidator_Bypass(uint32_t Index, void *pValue, size_t Length,
			       bool DoWrite)
{
	// Don't set the modified flag or change the value of the attribute.
	// This is used for protocol values.
	UNUSED_PARAMETER(Index);
	UNUSED_PARAMETER(pValue);
	UNUSED_PARAMETER(Length);
	UNUSED_PARAMETER(DoWrite);
	return true;
}

bool AttributeValidator_SpecialString(uint32_t Index, void *pValue,
				      size_t Length, bool DoWrite)
{
	// Don't set the modified flag and write the attribute if it fits.
	UNUSED_PARAMETER(DoWrite);
	AttributeEntry_t *pEntry = &attrTable[Index];
	if (pEntry->size > Length) // -1 to account for NUL
	{
		memset(pEntry->pData, 0, pEntry->size);
		strncpy(pEntry->pData, pValue, Length);
		return true;
	}
	return false;
}

bool AttributeValidator_GenericString(uint32_t Index, void *pValue,
				      size_t Length, bool DoWrite)
{
	AttributeEntry_t *pEntry = &attrTable[Index];

	if (pEntry->size > Length) // -1 to account for NUL
	{
		// Don't use strncmp because pValue isn't NUL terminated when coming from JSON
		size_t currentLength = strlen(pEntry->pData);
		if (DoWrite && ((memcmp(pEntry->pData, pValue, Length) != 0) ||
				Length == 0 || currentLength != Length)) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, pValue, Length);
		}
		return true;
	}
	return false;
}

bool AttributeValidator_TrimString(uint32_t Index, void *pValue, size_t Length,
				   bool DoWrite)
{
	AttributeEntry_t *pEntry = &attrTable[Index];

	if (pEntry->size > Length) // -1 to account for NUL
	{
		// trim leading spaces
		char *ptr = (char *)pValue;
		size_t modifiedLength = Length;
		while (((*ptr == ' ') || (*ptr == '\t')) &&
		       (modifiedLength > 0)) {
			ptr++;
			modifiedLength--;
		}

		// trim trailing spaces
		if (modifiedLength > 0) {
			char *pEnd = (char *)pValue + Length - 1;
			while (((*pEnd == ' ') || (*pEnd == '\t')) &&
			       (modifiedLength > 0)) {
				pEnd--;
				modifiedLength--;
			}
		}

		size_t currentLength = strlen(pEntry->pData);
		if (DoWrite &&
		    ((memcmp(pEntry->pData, ptr, modifiedLength) != 0) ||
		     Length == 0 || currentLength != modifiedLength)) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, ptr, modifiedLength);
		}
		return true;
	}
	return false;
}

bool AttributeValidator_uint32_t(uint32_t Index, void *pValue, size_t Length,
				 bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	uint32_t value = *((uint32_t *)pValue);
	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint32_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}
bool AttributeValidator_uint16_t(uint32_t Index, void *pValue, size_t Length,
			    bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	uint16_t value = *((uint16_t *)pValue);
	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint16_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}
bool AttributeValidator_uint8_t(uint32_t Index, void *pValue, size_t Length,
			   bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	uint8_t value = *((uint8_t *)pValue);
	if (((value >= pEntry->min) && (value <= pEntry->max)) ||
	    (pEntry->min == pEntry->max)) {
		if (DoWrite && value != *((uint8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}

bool AttributeValidator_int32_t(uint32_t Index, void *pValue, size_t Length,
			      bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	int32_t value = *((int32_t *)pValue);
	int32_t min = (int32_t)pEntry->min;
	int32_t max = (int32_t)pEntry->max;
	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}
bool AttributeValidator_int16_t(uint32_t Index, void *pValue, size_t Length,
			      bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	int16_t value = *((int16_t *)pValue);
	int16_t min = (int16_t)pEntry->min;
	int16_t max = (int16_t)pEntry->max;
	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int16_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}
bool AttributeValidator_int8_t(uint32_t Index, void *pValue, size_t Length,
			      bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	int8_t value = *((int8_t *)pValue);
	int8_t min = (int8_t)pEntry->min;
	int8_t max = (int8_t)pEntry->max;
	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((int8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int8_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}

bool AttributeValidator_float(uint32_t Index, void *pValue, size_t Length,
			      bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	float value = *((float *)pValue);
	float min = (float)pEntry->min;
	float max = (float)pEntry->max;
	if (((value >= min) && (value <= max)) || (min == max)) {
		if (DoWrite && value != *((float *)pEntry->pData)) {
			pEntry->modified = true;
			*((float *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}

//=================================================================================================
// Local Function Definitions
//=================================================================================================
// NA

// end
