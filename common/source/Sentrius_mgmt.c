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

#include "Attribute.h"
#include "UserInterfaceTask.h"
#include "AdcBt6.h"

#include "Sentrius_mgmt.h"

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
    /* pystart - mgmt handlers */
    [SENTRIUS_MGMT_ID_GETPARAMETER] = {
         .mh_read = Sentrius_mgmt_GetParameter,
         .mh_write = Sentrius_mgmt_GetParameter,
    },
    [SENTRIUS_MGMT_ID_SETPARAMETER] = {
         .mh_write = Sentrius_mgmt_SetParameter,
         .mh_read = Sentrius_mgmt_SetParameter,
    },
    [SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR] = {
		.mh_write = Sentrius_mgmt_CalibrateThermistor,
		.mh_read = Sentrius_mgmt_CalibrateThermistor
    },
    [SENTRIUS_MGMT_ID_TESTLED] = {
		.mh_write = Sentrius_mgmt_TestLed,
		.mh_read = Sentrius_mgmt_TestLed
    },
    [SENTRIUS_MGMT_ID_REVECHO] = {
         .mh_write = Sentrius_mgmt_RevEcho,
		 .mh_read = Sentrius_mgmt_RevEcho
    },
	[SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR_VERSION2] = {
         .mh_write = Sentrius_mgmt_CalibrateThermistorVersion2,
		 .mh_read = Sentrius_mgmt_CalibrateThermistorVersion2
    }
    /* pyend */
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
	long long unsigned int paramID = ATTR_TABLE_SIZE + 1;
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
			.nodefault = true,
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
		getResult = Attribute_Get(paramID, &intData, sizeof(int32_t));
		err |= cbor_encode_int(&ctxt->encoder, intData);
		break;
	case CborAttrUnsignedIntegerType:
		getResult = Attribute_Get(paramID, &uintData, sizeof(uint32_t));
		err |= cbor_encode_uint(&ctxt->encoder, uintData);
		break;
	case CborAttrTextStringType:
		getResult =
			Attribute_Get(paramID, bufferData, ATTR_MAX_STR_LENGTH);
		err |= cbor_encode_text_stringz(&ctxt->encoder, bufferData);
		break;
	case CborAttrFloatType:
		getResult = Attribute_Get(paramID, &floatData, sizeof(float));
		err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType,
						  &floatData);
		break;
	default:
		err |= cbor_encode_text_stringz(&ctxt->encoder, "NULL");
		break;
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, getResult);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}
	return 0;
}

int Sentrius_mgmt_RevEcho(struct mgmt_ctxt *ctxt)
{
	char echo_buf[64] = { 0 };
	char rev_buf[64] = { 0 };
	size_t echo_len = 0;

	const struct cbor_attr_t attrs[2] = {
        [0] = {
            .attribute = "d",
            .type = CborAttrTextStringType,
            .addr.string = echo_buf,
            .len = sizeof echo_buf,
            .nodefault = true,
        },
        [1] = {
            .attribute = NULL
        }
    };

	echo_buf[0] = '\0';

	if (cbor_read_object(&ctxt->it, attrs) != 0) {
		return MGMT_ERR_EINVAL;
	}

	echo_len = strlen(echo_buf);
	size_t i;
	for (i = 0; i < echo_len; i++) {
		rev_buf[i] = echo_buf[echo_len - 1 - i];
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_text_string(&ctxt->encoder, rev_buf, echo_len);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}

int Sentrius_mgmt_SetParameter(struct mgmt_ctxt *ctxt)
{
	long long unsigned int paramID = ATTR_TABLE_SIZE + 1;
	int readCbor = 0;
	struct CborValue dataCopy = ctxt->it;
	struct cbor_attr_t params_value[2] = { 0 };
	CborAttrType parameterDataType;
	int setResult = -1;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &paramID,
		  .nodefault = true },
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

int Sentrius_mgmt_TestLed(struct mgmt_ctxt *ctxt)
{
	int r = -EINVAL;
	long long unsigned int duration = ULLONG_MAX;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &duration,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	if (duration < UINT32_MAX) {
		r = UserInterfaceTask_LedTest(duration);
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_CalibrateThermistor(struct mgmt_ctxt *ctxt)
{
	int r = -EINVAL;
	float c1 = 0.0;
	float c2 = 0.0;
	float ge = 0.0;
	float oe = 0.0;

	struct cbor_attr_t params_attr[] = { { .attribute = "p1",
					       .type = CborAttrFloatType,
					       .addr.fval = &c1,
					       .nodefault = true },
					     { .attribute = "p2",
					       .type = CborAttrFloatType,
					       .addr.fval = &c2,
					       .nodefault = true },
					     { .attribute = NULL } };

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	r = AdcBt6_CalibrateThermistor(c1, c2, &ge, &oe);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_CalibrateThermistorVersion2(struct mgmt_ctxt *ctxt)
{
	int r = -EINVAL;
	long long unsigned int c1 = 0;
	long long unsigned int c2 = 0;
	float ge = 0.0;
	float oe = 0.0;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &c1,
		  .nodefault = true },
		{ .attribute = "p2",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &c2,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	r = AdcBt6_CalibrateThermistor(((float)c1 / 10000.0),
				       ((float)c2 / 10000.0), &ge, &oe);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs)
{
	AttrType_t parameterType = Attribute_GetType(paramID);

	attrs->attribute = "p2";
	attrs->nodefault = true;

	switch (parameterType) {
	case ATTR_TYPE_S8:
	case ATTR_TYPE_S16:
	case ATTR_TYPE_S32:
		paramIint = LLONG_MAX;
		attrs->type = CborAttrIntegerType;
		attrs->addr.integer = &paramIint;
		break;
	case ATTR_TYPE_U8:
	case ATTR_TYPE_U16:
	case ATTR_TYPE_U32:
		paramUint = ULLONG_MAX;
		attrs->type = CborAttrUnsignedIntegerType;
		attrs->addr.integer = &paramUint;
		break;
	case ATTR_TYPE_STRING:
		memset(paramString, 0, ATTR_MAX_STR_SIZE);
		attrs->type = CborAttrTextStringType;
		attrs->addr.string = paramString;
		attrs->len = sizeof(paramString);
		break;
	case ATTR_TYPE_FLOAT:
		paramFloat = FLOAT_MAX;
		attrs->type = CborAttrFloatType;
		attrs->addr.fval = &paramFloat;
		break;
	default:
		attrs->type = CborAttrNullType;
		attrs->attribute = NULL;
		break;
	};

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
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.integer,
					       sizeof(int32_t), EXTERNAL_SET);
		}
		break;
	case CborAttrUnsignedIntegerType:
		if (*attrs->addr.uinteger >= 0 &&
		    *attrs->addr.uinteger <= UINT32_MAX) {
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.uinteger,
					       sizeof(uint32_t), EXTERNAL_SET);
		}
		break;

	case CborAttrTextStringType:
		status =
			Attribute_Set(id, Attribute_GetType(id),
				      attrs->addr.string,
				      strlen(attrs->addr.string), EXTERNAL_SET);
		break;

	case CborAttrFloatType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.fval, sizeof(float),
				       EXTERNAL_SET);
		break;

	default:
		break;
	}
	return (status);
}
