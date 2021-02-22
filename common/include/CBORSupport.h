/**
 * @file CBORSupport.h
 * @brief Interface to the CBOR Support functions module.
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef __CBOR_SUPPORT__
	#error "CBORSupport.h Error - CBORSupport.h is already included"
#endif

#define __CBOR_SUPPORT__

#ifndef CBOR_H
	#error "CBORSupport.h Error - cbor.h must be included first."
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

/**@brief Indexed list of CBOR attribute types for use when iterating through
 *        a CBOR message to determine its content.
 *
 * @member typeIndex - The current list index.
 * @member typeList - Pointer to an array of types, passed during construction
 *                    to allow the size to be variable.
 * @member dataBuffer - This is a pointer to the buffer that is used to read
 *                      data back from the CBOR message. It's declared here to
 *                      avoid continuous allocation of stack memory in the
 *                      recursive function where this type gets used.
 */
typedef struct _cborTypeList_t{
	uint8_t typeIndex;
	CborType *typeList;
        uint8_t *dataBuffer;
}cborTypeList_t;

/**@brief Constructor for a list of CBOR types read from a CBOR message's
 *        contained elements.
 *
 * @param [in]x - The name of the type list.
 * @param [in]y - The number of list elements.
 * @param [in]z - The size of the string buffer.
 */
#define CONSTRUCT_CBOR_TYPE_LIST(x,y,z) CborType typeList_##x[y];\
					uint8_t dataBuffer_##x[z];\
					cborTypeList_t x = {0,typeList_##x,dataBuffer_##x}


/******************************************************************************/
/* Function Definitions                                                       */
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
 * @param [out]pCborTypeList - This is the list of element types extracted from
 *                             the CBOR message.
 * @retval A CBorError error code, 0 for success.
 */
CborError GetMessageElementTypes(CborValue *it,
					int nestingLevel,
                                        uint16_t maxStringLength,
					cborTypeList_t *pCborTypeList);
