/**
 * @file CBORSupport.c
 * @brief Support functions for CBOR messages.
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "tinycbor/cbor.h"
#include "CBORSupport.h"

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/

/**@brief Iterates through a CBOR message and extracts the types for all
 *        contained elements.
 *
 *        Refer to https://github.com/intel/tinycbor/blob/master/examples/
 *        simplereader.c for further details.
 *
 *        NOTE -  This function is recursive. It will happily consume all
 *        available memory. Sizeof() the passed pCborTypeList typeList member
 *        is used to clamp recursion when the depth of the typeList member is
 *        reached.
 *
 *        NOTE - This function is destructive. The parameters of the CBOR
 *        message passed to it are adjusted as elements are read from it.
 *
 * @param [in]it - The CBOR message to parse.
 * @param [internal]nestingLevel - This must be passed as 0. It is used
 *                                 internally by the function to track the
 *                                 level of recursion.
 * @param [in] - maxStringLength - The maximum size of string buffers to allow.
 * @param [out]pCborTypeList - This is the list of element types extracted from
 *                             the CBOR message.
 * @retval A CBorError error code, 0 for success.
 */
CborError GetMessageElementTypes(CborValue *it,
					int nestingLevel,
                                        uint16_t maxStringLength,
					cborTypeList_t *pCborTypeList)
{

	while ((!cbor_value_at_end(it))&&
		(pCborTypeList->typeIndex < sizeof(pCborTypeList->typeList))){

		CborError err;
		CborType type = cbor_value_get_type(it);

		switch (type) {
			case CborArrayType:
			case CborMapType: {
				// recursive type
				CborValue recursed;
				assert(cbor_value_is_container(it));
				err = cbor_value_enter_container(it,
								&recursed);
				if (err){
					// parse error
					return err;
                                }
				err = GetMessageElementTypes(&recursed,
								nestingLevel + 1,
                                                                maxStringLength,
								pCborTypeList);
				if (err){
					// parse error
					return err;
                                }
				err = cbor_value_leave_container(it, &recursed);
				if (err){
					// parse error
					return err;
				}
				continue;
			}

			case CborIntegerType: {
				int64_t val;
                                // can't fail
				cbor_value_get_int64(it, &val);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborIntegerType;
				break;
			}

			case CborByteStringType: {
				size_t bufferLength = maxStringLength;
				err = cbor_value_dup_byte_string(it,
							&pCborTypeList->dataBuffer,
							&bufferLength,
							it);
				if (err){
					// parse error
					return err;
				}
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborByteStringType;
				continue;
			}

			case CborTextStringType: {
				size_t bufferLength = maxStringLength;
				err = cbor_value_dup_text_string(it,
							((char **)(&pCborTypeList->dataBuffer)),
							&bufferLength,
							it);
				if (err){
					// parse error
					return err;
				}
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborTextStringType;
				continue;
			}

			case CborTagType: {
				CborTag tag;
                                // can't fail
				cbor_value_get_tag(it, &tag);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborTagType;
				break;
			}

			case CborSimpleType: {
				uint8_t type;
                                // can't fail
				cbor_value_get_simple_type(it, &type);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborSimpleType;
				break;
			}

			case CborNullType:
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborNullType;
				break;

			case CborUndefinedType:
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborUndefinedType;
				break;

			case CborBooleanType: {
				bool val;
				// can't fail
				cbor_value_get_boolean(it, &val);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborBooleanType;
				break;
			}

			case CborFloatType: {
				float f;
				cbor_value_get_float(it, &f);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborFloatType;
				break;
			}

			case CborDoubleType: {
				double val;
				cbor_value_get_double(it, &val);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborDoubleType;
				break;
			}

			case CborHalfFloatType: {
				uint16_t val;
				cbor_value_get_half_float(it, &val);
				pCborTypeList->typeList[pCborTypeList->
					typeIndex++] = CborHalfFloatType;
				break;
			}

			case CborInvalidType:
				// can't happen
				assert(false);
				break;
			}

		err = cbor_value_advance_fixed(it);
		if (err)
			return err;
	}
	return CborNoError;
}
