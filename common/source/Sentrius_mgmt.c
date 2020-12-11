/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(SentriusMgmt);
#define THIS_FILE "SentriusMgmt"

#include <zephyr.h>
#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Sentrius_mgmt.h"
#include "Sentrius_mgmt_impl.h"
#include "Sentrius_mgmt_config.h"
#include "AttributeTask.h"

static CborAttrType ParameterValueType(long long unsigned int paramID,
				       struct cbor_attr_t *attrs);

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

int Sentrius_mgmt_GetParameter(struct mgmt_ctxt *ctxt)
{
	uint8_t off = 0;
	uint8_t file_len = 12;
	CborError err;
	/* Encode the response. */
	err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "off");
	err |= cbor_encode_uint(&ctxt->encoder, off);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "data");
	err |= cbor_encode_text_stringz(&ctxt->encoder, "rc");
	if (off == 0) {
		err |= cbor_encode_text_stringz(&ctxt->encoder, "len");
		err |= cbor_encode_uint(&ctxt->encoder, file_len);
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
            .nodefault = 1,
            .len = sizeof echo_buf,
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
	struct cbor_attr_t params_value;
	CborAttrType parameterDataType;

	struct cbor_attr_t params_attr[] = {
		{
			.attribute = "p1",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &paramID,
		},
		{ .attribute = NULL }
	};
	readCbor = cbor_read_object(&ctxt->it, params_attr);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}
	//figure type from p1 look up id number match to type
	parameterDataType = ParameterValueType(paramID, &params_value);

	const struct cbor_attr_t params_attr2[] = {
		{
			.attribute = "p1",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &paramID,
		},
		params_value,
		{ .attribute = NULL }
	};
	readCbor = cbor_read_object(&dataCopy, params_attr2);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}
	SaveParameterValue(parameterDataType, &params_value);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}
	return 0;
}
static CborAttrType ParameterValueType(long long unsigned int paramID,
				       struct cbor_attr_t *attrs)
{
	attrs->attribute = "p2";
	if (AttributeTask_IsUnsigned(paramID)) {
		long long unsigned int paramUint = 255;
		attrs->type = CborAttrIntegerType;
		attrs->addr.integer = &paramUint;
	} else if (AttributeTask_IsSigned(paramID)) {
		long long int paramIint = 255;
		attrs->type = CborAttrUnsignedIntegerType;
		attrs->addr.uinteger = &paramIint;
	} else if (AttributeTask_IsString(paramID)) {
		char paramString[ATTRIBUTE_STRING_MAX_LENGTH] = { 0 };
		attrs->type = CborAttrTextStringType;
		attrs->addr.string = paramString;
		attrs->len = sizeof(paramString);
	} else if (AttributeTask_IsFloat(paramID)) {
		float paramFloat = 0;
		attrs->type = CborAttrFloatType;
		attrs->addr.fval = &paramFloat;
	} else {
		attrs->type = CborAttrNullType;
		attrs->attribute = NULL;
	}
	return (attrs->type);
}
static void SaveParameterValue(parameterDataType, &params_value)
{
}

#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

void Sentrius_mgmt_register_group(void)
{
	mgmt_register_group(&sentrius_mgmt_group);
}
