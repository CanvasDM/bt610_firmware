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

#include "AttributeTable.h"

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
AttributeType_t Attribute_GetType(attr_idx_t Index);

/**
 * @brief  Set value
 *
 * @param Index A valid index into attribute table.
 * @param pValue string representation of variable
 * @param ValueLength The length (without null char) of the string being passed in.
 *
 * @retval negative error code, 0 on success, 1 if value was changed
 */
int Attribute_Set(attr_idx_t Index, void *pValue, size_t ValueLength);

/**
 * @brief  Set a string
 *
 * @param Index A valid index into attribute table.
 * @param pValue string representation of variable
 * @param ValueLength The length (without null char) of the string being passed in.
 *
 * @retval negative error code, 0 on success, 1 if value was changed
 */
int Attribute_SetString(attr_idx_t Index, char const *pValue,
			size_t ValueLength);

/**
 * @brief  Used to set the value of an integer attribute.
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success, 1 if value was changed
 */
int Attribute_SetUint32(attr_idx_t Index, uint32_t Value);
int Attribute_SetSigned32(attr_idx_t Index, int32_t Value);

/**
 * @brief  Used to set the value of a floating point attribute
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success, 1 if value was changed
 */
int Attribute_SetFloat(attr_idx_t Index, float Value);

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
 * @brief  Used to get the value of a variable as an unsigned/signed integer.
 *
 * @param pValue, 0 if attribute is a string.
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index);
int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index);

/**
 * @brief  Used to get the value of a variable as float.
 *
 * @param pValue, 0 if attribute is a string.
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetFloat(float *pValue, attr_idx_t Index);

/**
 * @brief Get the name of an attribute
 *
 * @param Index a valid index into table
 *
 * @param empty string if not found
 */
const char *Attribute_GetName(attr_idx_t Index);

/**
 * @param Index - A valid index into the attribute table
 *
 * @retval true if the index is associated with a string, otherwise false
 */
bool Attribute_IsString(attr_idx_t Index);

/**
 * @param Index - A valid index into the attribute table
 *
 * @retval true if the index is associated with a float, otherwise false
 */
bool Attribute_IsFloat(attr_idx_t Index);

/**
 * @param Index - A valid index into the attribute table
 *
 * @retval true if the index is associated with a unsigned int, otherwise false
 */
bool Attribute_IsUnsigned(attr_idx_t Index);

/**
 * @param Index - A valid index into the attribute table
 *
 * @retval true if the index is associated with a signed int, otherwise false
 */
bool Attribute_IsSigned(attr_idx_t Index);

/**
 * @param index - A valid index into the attribute table
 *
 * @retval true if the value is writable (including lock flag), false otherwise
 */
bool Attribute_IsWritable(attr_idx_t Index);

/**
 * @retval true if the value has a category of read write, false otherwise
 */
bool Attribute_IsReadWrite(attr_idx_t Index);

/**
 * @retval true if the value has a category of read only, false otherwise
 */
bool Attribute_IsReadOnly(attr_idx_t Index);

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TASK_H__ */
