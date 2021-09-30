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

#include "FrameworkIncludes.h"
#include "AttributeTable.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
typedef struct AttrBroadcastMsg {
	FwkMsgHeader_t header;
	size_t count;
	uint8_t list[ATTR_TABLE_SIZE];
} AttrChangedMsg_t;
BUILD_ASSERT(ATTR_TABLE_SIZE <= UINT8_MAX, "List element size too small");

typedef enum AttrDumpType {
	ATTR_DUMP_RW = 0,
	ATTR_DUMP_W,
	ATTR_DUMP_RO
} AttrDumpType_t;

/* Errors associated with parameter writes */
typedef enum AttrWriteError {
	ATTR_WRITE_ERROR_OK = 0,
	ATTR_WRITE_ERROR_NUMERIC_TOO_LOW,
	ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH,
	ATTR_WRITE_ERROR_STRING_TOO_MANY_CHARACTERS,
	ATTR_WRITE_ERROR_PARAMETER_READ_ONLY,
	ATTR_WRITE_ERROR_PARAMETER_UNKNOWN,
	ATTR_WRITE_ERROR_PARAMETER_INVALID_LENGTH
} AttrWriteError_T;

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/

/**
 * @brief Read attributes from flash
 *
 * @retval negative error code, 0 on success
 */
int Attribute_Init(void);

/**
 * @brief Set values to default (except items configured during production).
 *
 * @retval negative error code, 0 on success
 */
int Attribute_FactoryReset(void);

/**
 * @brief Get the type of the attribute
 *
 * @retval type of variable
 */
AttrType_t Attribute_GetType(attr_idx_t Index);

/**
 * @brief Helper function
 *
 * @retval true if index is valid, false otherwise
 */
bool Attribute_ValidIndex(attr_idx_t Index);

/**
 * @brief Set value.  This is the only function that should be
 * used from the SMP interface.  It requires the writable attribute to be true.
 *
 * @param Index A valid index into attribute table.
 * @param Type the type of attribute
 * @param pValue string representation of variable
 * @param ValueLength The length (without null char) of the string
 * being passed in.  If the value isn't a string, then the length is
 * not used.
 * @param modified Will be set to true if value was updated (different than the
 * existing value). Can safely be supplied NULL to ignore
 *
 * @retval negative error code, 0 on success
 */
int Attribute_Set(attr_idx_t Index, AttrType_t Type, void *pValue,
		  size_t ValueLength, bool *modified);

/**
 * @brief Default value of an attribute.  This function will find the default
 * value and return based on the index.
 *
 * @param Index A valid index into attribute table.
 * @param pValue pointer to the default value
 * @param ValueLength is the size of pValue.
 *
 * @retval negative error code, size of value on return
 */
int Attribute_GetDefault(attr_idx_t Index, void *pValue, size_t ValueLength);

/**
 * @brief Copy an attribute.  This is the only function that should be
 * used from the SMP interface because it checks the readable flag.
 *
 * @param Index A valid index into attribute table.
 * @param pValue pointer to location to copy string
 * @param ValueLength is the size of pValue.
 *
 * @retval negative error code, size of value on return
 */
int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength);

/**
 * @brief Set a string
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
 * @brief Helper function for setting uint64
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetUint64(attr_idx_t Index, uint64_t Value);

/**
 * @brief Helper function for setting int64
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetSigned64(attr_idx_t Index, int64_t Value);

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
 * @brief Helper function for setting uint8, 16 or 32 that will save and not broadcast to the system
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetNoBroadcastUint32(attr_idx_t Index, uint32_t Value);

/**
 * @brief Accessor Function for uint32
 *
 * @param pValue pointer to data
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index);

/**
 * @brief Accessor Function for int32
 *
 * @param pValue pointer to data
 * @param Index A valid index into attribute table
 *
 * @retval negative error code, 0 on success
 */
int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index);

/**
 * @brief Used to set the value of a floating point attribute
 *
 * @param Index A valid index into attribute table.
 * @param Value The value to set.
 *
 * @retval negative error code, 0 on success
 */
int Attribute_SetFloat(attr_idx_t Index, float Value);

/**
 * @brief Accessor Function for float
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
 * @retval empty string if not found
 */
const char *Attribute_GetName(attr_idx_t Index);

/**
 * @brief Get the size of an attribute
 *
 * @param Index a valid index into table
 *
 * @param size of attribute, size with null if string
 */
size_t Attribute_GetSize(attr_idx_t Index);

/**
 * @brief Set/Clear bit in a 32-bit attribute
 *
 * @param Index a valid index into table
 * @param Bit location to set
 * @param Value 0 for clear, any other value for set
 *
 * @param size of attribute, size with null if string
 */
int Attribute_SetMask32(attr_idx_t Index, uint8_t Bit, uint8_t Value);

/**
 * @brief Set/Clear bit in an 64-bit attribute
 *
 * @param Index a valid index into table
 * @param Bit location to set
 * @param Value 0 for clear, any other value for set
 *
 * @param size of attribute, size with null if string
 */
int Attribute_SetMask64(attr_idx_t Index, uint8_t Bit, uint8_t Value);

#ifdef CONFIG_ATTR_SHELL
/**
 * @brief Get the index of an attribute
 *
 * @param Name of the attribute
 *
 * @retval attr_idx_t index of attribute
 */
attr_idx_t Attribute_GetIndex(const char *Name);

/**
 * @brief Print the value of an attribute (LOG_DBG)
 *
 * @param Index a valid index into table
 *
 * @param negative error code, 0 on success
 */
int Attribute_Show(attr_idx_t Index);

/**
 * @brief Print all parameters to the console using system workq.
 *
 * @param negative error code, 0 on success
 */
int Attribute_ShowAll(void);

#endif /* CONFIG_ATTR_SHELL */

/**
 * @brief Print all parameters to the console using system workq.
 *
 * @param fstr pointer to file string
 * @param Type the type of dump to perform
 *
 * @param negative error code, number of parameters on success
 * If result is positive, then caller is responsbile for freeing fstr.
 */
int Attribute_Dump(char **fstr, AttrDumpType_t Type);

/**
 * @brief Set the quiet flag for an attribute.
 * Settings are saved to filesystem.
 *
 * @param Index into attribute table
 * @param Value true to make quiet, false allows printing
 *
 * @param negative error code, otherwise 0
 */
int Attribute_SetQuiet(attr_idx_t Index, bool Value);

/**
 * @brief Load attributes from a file and save them to params.txt
 *
 * @param abs_path Absolute file name
 * @param feedback_path Absolute path of feedback file
 *
 * @param negative error code, number of parameters on success
 */
int Attribute_Load(const char *abs_path, const char *feedback_path);

/**
 * @brief Immediately save data to params.txt
 *
 * @param negative error code, 0 on success
 */
int Attribute_Save_Now(void);

/**
 * @brief Retrive the current status of the coded enable parameter
 *
 * @retval true = coded enabled, false = disabled
 */
bool Attribute_CodedEnableCheck(void);

/**
 * @brief Retrive the current lock status
 *
 * @retval true = configuration locked, false = configuration not locked
 */
bool Attribute_IsLocked(void);

/**
 * @brief Updates the attribute set's config version
 *
 */
void Attribute_UpdateConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TASK_H__ */
