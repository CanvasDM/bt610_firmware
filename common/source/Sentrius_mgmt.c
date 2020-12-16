/**
 * @file Sentrius_mgmt.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Sentrius_mgmt.h"
#include "Attribute.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

#define FLOAT_MAX 3.4028235E38

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs);

static int SaveParameterValue(attr_idx_t id, CborAttrType dataType,
			      struct cbor_attr_t *attrs);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct mgmt_handler sentrius_mgmt_handlers[] = {
	// pystart - mgmt handlers
    [SENTRIUS_MGMT_ID_GETPARAMETER] = {
         .mh_read = Sentrius_mgmt_GetParameter,
         .mh_write = NULL,
    },
    [SENTRIUS_MGMT_ID_SETPARAMETER] = {
         .mh_write = Sentrius_mgmt_SetParameter,
         .mh_read = NULL,
    },
    [SENTRIUS_MGMT_ID_ECHO] = {
         Sentrius_mgmt_Echo, Sentrius_mgmt_Echo
    },
	// pyend
};

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

static long long unsigned int paramUint;
static long long int paramIint;
static char paramString[ATTR_MAX_STR_SIZE];
static float paramFloat;

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void Sentrius_mgmt_register_group(void)
{
	mgmt_register_group(&sentrius_mgmt_group);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
int Sentrius_mgmt_GetParameter(struct mgmt_ctxt *ctxt)
{
	long long unsigned int paramID = 255;
	int readCbor = 0;
	int32_t intData;
	uint32_t uintData;
	float floatData;
	char bufferData[ATTR_MAX_STR_SIZE];
	int getResult = -1;
	struct cbor_attr_t params_value[2] = { 0 };
	CborAttrType parameterDataType;

	struct cbor_attr_t params_attr[] = {
		{
			.attribute = "p1",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &paramID,
			.nodefault = 1,
		},
		{ .attribute = NULL }
	};

	readCbor = cbor_read_object(&ctxt->it, params_attr);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}
	/*just need the type from p1 look up id don't need the params_value structure*/
	parameterDataType =
		ParameterValueType((attr_idx_t)paramID, params_value);
	/* Encode the response. */
	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");

	/*Get the value*/
	switch (parameterDataType) {
	case CborAttrIntegerType:
		getResult = Attribute_GetSigned32(&intData, paramID);
		err |= cbor_encode_int(&ctxt->encoder, intData);
		break;
	case CborAttrUnsignedIntegerType:
		getResult = Attribute_GetUint32(&uintData, paramID);
		err |= cbor_encode_uint(&ctxt->encoder, uintData);
		break;
	case CborAttrTextStringType:
		getResult = Attribute_GetString(bufferData, paramID,
						ATTR_MAX_STR_LENGTH);
		err |= cbor_encode_text_stringz(&ctxt->encoder, bufferData);
		break;
	case CborAttrFloatType:
		getResult = Attribute_GetFloat(&floatData, paramID);
		err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType,
						  &floatData);
		break;
	default:
		break;
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, getResult);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}
	return 0;
}

int Sentrius_mgmt_Echo(struct mgmt_ctxt *ctxt)
{
	char echo_buf[128];
	CborError err;

	const struct cbor_attr_t attrs[2] = {
        [0] = {
            .attribute = "d",
            .type = CborAttrTextStringType,
            .addr.string = echo_buf,
            .len = sizeof echo_buf,
            .nodefault = 1,
        },
        [1] = {
            .attribute = NULL
        }
    };

	echo_buf[0] = '\0';

	err = cbor_read_object(&ctxt->it, attrs);
	if (err != 0) {
		return MGMT_ERR_EINVAL;
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_text_string(&ctxt->encoder, echo_buf,
				       strlen(echo_buf));

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}
int Sentrius_mgmt_SetParameter(struct mgmt_ctxt *ctxt)
{
	long long unsigned int paramID = 255;
	int readCbor = 0;
	struct CborValue dataCopy = ctxt->it;
	struct cbor_attr_t params_value[2] = { 0 };
	CborAttrType parameterDataType;
	int setResult = -1;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &paramID,
		  .nodefault = 1 },
		{ .attribute = NULL }
	};
	readCbor = cbor_read_object(&ctxt->it, params_attr);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}

	/*type from p1 look up id number match to type*/
	parameterDataType =
		ParameterValueType((attr_idx_t)paramID, params_value);

	readCbor = cbor_read_object(&dataCopy, params_value);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}
	setResult = SaveParameterValue((attr_idx_t)paramID, parameterDataType,
				       params_value);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, setResult);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}
	return 0;
}

static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs)
{
	paramUint = ULLONG_MAX;
	paramIint = LLONG_MAX;
	paramFloat = FLOAT_MAX;
	memset(paramString, 0, ATTR_MAX_STR_SIZE);

	attrs->attribute = "p2";
	attrs->nodefault = true;
	if (Attribute_IsUnsigned(paramID)) {
		attrs->type = CborAttrIntegerType;
		attrs->addr.integer = &paramUint;
	} else if (Attribute_IsSigned(paramID)) {
		attrs->type = CborAttrUnsignedIntegerType;
		attrs->addr.uinteger = &paramIint;
	} else if (Attribute_IsString(paramID)) {
		attrs->type = CborAttrTextStringType;
		attrs->addr.string = paramString;
		attrs->len = sizeof(paramString);
	} else if (Attribute_IsFloat(paramID)) {
		attrs->type = CborAttrFloatType;
		attrs->addr.fval = &paramFloat;
	} else {
		attrs->type = CborAttrNullType;
		attrs->attribute = NULL;
	}
	return (attrs->type);
}

static int SaveParameterValue(attr_idx_t id, CborAttrType dataType,
			      struct cbor_attr_t *attrs)
{
	int status = -1;

	switch (dataType) {
	case CborAttrIntegerType:
		if (*attrs->addr.integer >= INT32_MIN &&
		    *attrs->addr.integer <= INT32_MAX) {
			status = Attribute_Set(id, attrs->addr.integer,
					       sizeof(int32_t));
		}
		break;
	case CborAttrUnsignedIntegerType:
		if (*attrs->addr.uinteger >= 0 &&
		    *attrs->addr.uinteger <= UINT32_MAX) {
			status = Attribute_Set(id, attrs->addr.uinteger,
					       sizeof(uint32_t));
		}
		break;

	case CborAttrTextStringType:
		status = Attribute_Set(id, attrs->addr.string,
				       strlen(attrs->addr.string));
		break;

	case CborAttrFloatType:
		status = Attribute_Set(id, attrs->addr.fval, sizeof(float));
		break;

	default:
		break;
	}
	return (status);
}
