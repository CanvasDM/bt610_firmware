/**
 * @file Attribute.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ATTRIBUTE_TASK_H__
#define __ATTRIBUTE_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdint.h>
#include <stddef.h>

#ifdef CONFIG_ATTR_BROADCAST
#include "FrameworkIncludes.h"
#endif

#include "AttributeTable.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#ifdef CONFIG_ATTR_BROADCAST

typedef struct AttrBroadcastMsg {
	FwkMsgHeader_t header;
	size_t count;
	uint8_t list[ATTR_TABLE_SIZE];
} AttrBroadcastMsg_t;
BUILD_ASSERT(ATTR_TABLE_SIZE <= UINT8_MAX, "List element size too small");

#endif

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/

/**
 * @brief  Read attributes from flash
 *
 * @retval negative error code, 0 on success
 */
int AttributesInit(void);

/**
 * @brief  Get the type of the attribute
 *
 * @retval type of variable
 */
AttrType_t Attribute_GetType(attr_idx_t Index);

/**
 * @brief  Set value.  This is the only function that should be
 * used from the SMP interface.
 *
 * @param Index A valid index into attribute table.
 * @param Type the type of attribute
 * @param pValue string representation of variable
 * @param ValueLength The length (without null char) of the string
 * being passed in.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_Set(attr_idx_t Index, AttrType_t Type, void *pValue,
		  size_t ValueLength);

/**
 * @brief Copy an attribute
 *
 * @param Index A valid index into attribute table.
 * @param pValue pointer to location to copy string
 * @param ValueLength is the size of pValue.
 *
 * @retval negative error code, size of value on return
 */
int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength);

/**
 * @brief  Set a string
 *
 * @param Index A valid index into attribute table.
 * @param pValue string representation of variable
 * @param ValueLength The length (without null char) of the
 * string being passed in.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetString(attr_idx_t Index, char const *pValue,
			size_t ValueLength);

/**
 * @brief Copy a string
 *
 * @param pValue pointer to location to copy string
 * @param Index A valid index into attribute table.
 * @param MaxStringLength is the size of pValue.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetString(char *pValue, attr_idx_t Index, size_t MaxStringLength);

/**
 * @brief Helper function for setting uint8, 16 or 32
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetUint32(attr_idx_t Index, uint32_t Value);

/**
 * @brief Helper function for setting int8, int16, or int32
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetSigned32(attr_idx_t Index, int32_t Value);

/**
 * @brief  Accessor Function for uint32
 *
 * @param pValue pointer to data
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index);

/**
 * @brief  Accessor Function for int32
 *
 * @param pValue pointer to data
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index);

/**
 * @brief  Used to set the value of a floating point attribute
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetFloat(attr_idx_t Index, float Value);

/**
 * @brief  Accessor Function for float
 *
 * @param pValue pointer to data
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetFloat(float *pValue, attr_idx_t Index);

/**
 * @brief Alternate Accessor function for uint32
 *
 * @param Index A valid index into attribute table
 * @param Default value
 *
 * @retval default value if not found, invalid index,  or wrong type;
 * otherwise the attribute value
 */
uint32_t Attribute_AltGetUint32(attr_idx_t Index, uint32_t Default);

/**
 * @brief Alternate Accessor function for int32
 *
 * @param Index A valid index into attribute table
 * @param Default value
 *
 * @retval default value if not found, invalid index,  or wrong type;
 * otherwise the attribute value
 */
int32_t Attribute_AltGetSigned32(attr_idx_t Index, int32_t Default);

/**
 * @brief Alternate Accessor function for float
 *
 * @param Index A valid index into attribute table
 * @param Default value
 *
 * @retval default value if not found, invalid index,  or wrong type;
 * otherwise the attribute value
 */
float Attribute_AltGetFloat(attr_idx_t Index, float Default);

/**
 * @brief Get the name of an attribute
 *
 * @param Index a valid index into table
 *
 * @param empty string if not found
 */
const char *Attribute_GetName(attr_idx_t Index);

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TASK_H__ */
