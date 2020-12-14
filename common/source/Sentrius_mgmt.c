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

//=================================================================================================
// Includes
//=================================================================================================
#include <zephyr.h>
#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Sentrius_mgmt.h"
#include "Sentrius_mgmt_impl.h"
#include "Sentrius_mgmt_config.h"
#include "AttributeTask.h"
//=================================================================================================
// Local Constant, Macro and Type Definitions
//=================================================================================================
#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

//=================================================================================================
// Local Function Prototypes
//=================================================================================================

static CborAttrType ParameterValueType(long long unsigned int paramID,
				       struct cbor_attr_t *attrs);

static int8_t SaveParameterValue(long long unsigned int id, CborAttrType dataType, struct cbor_attr_t *attrs);
//=================================================================================================
// Local Data Definitions
//=================================================================================================
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


static long long unsigned int paramUint = 0;
static long long int paramIint = 0;
static char paramString[ATTRIBUTE_STRING_MAX_LENGTH] = { 0 };
static float paramFloat = 0;

//=================================================================================================
// Global Function Definitions
//=================================================================================================
void Sentrius_mgmt_register_group(void)
{
	mgmt_register_group(&sentrius_mgmt_group);
}

int Sentrius_mgmt_GetParameter(struct mgmt_ctxt *ctxt)
{
     long long unsigned int paramID = 255;
     int readCbor = 0;
     int32_t intData;
     uint32_t uintData;
     float floatData;
     char bufferData[ATTRIBUTE_STRING_MAX_LENGTH];
     int8_t getResult = -1;
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
     /*just need the type from p1 look up id don't need the params_value structure*/
	parameterDataType = ParameterValueType(paramID, &params_value);
	/* Encode the response. */
	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");

     /*Get the value*/
     switch(parameterDataType)
     {
          case CborAttrIntegerType:
               getResult = AttributeTask_GetSigned32(&intData, paramID);
               err |= cbor_encode_int(&ctxt->encoder, intData);
               break;
          case CborAttrUnsignedIntegerType:
               getResult = AttributeTask_GetUint32(&uintData, paramID);
               err |= cbor_encode_uint(&ctxt->encoder, uintData);
               break;
          case CborAttrTextStringType:
               getResult = AttributeTask_GetString(bufferData, paramID, ATTRIBUTE_STRING_MAX_LENGTH);
               err |= cbor_encode_text_stringz(&ctxt->encoder, bufferData);
               break;
          case CborAttrFloatType:
               getResult = AttributeTask_GetFloat(&floatData, paramID);
               err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &floatData);
               break;               
          default:
               break;
     }
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
     int8_t setResult = -1;

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
	/*type from p1 look up id number match to type*/
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
	setResult = SaveParameterValue(paramID, parameterDataType, &params_value);

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
//=================================================================================================
// Local Function Definitions
//=================================================================================================
static CborAttrType ParameterValueType(long long unsigned int paramID,
				       struct cbor_attr_t *attrs)
{
	attrs->attribute = "p2";
	if (AttributeTask_IsUnsigned(paramID)) {
		attrs->type = CborAttrIntegerType;
		attrs->addr.integer = &paramUint;
	} else if (AttributeTask_IsSigned(paramID)) {
		attrs->type = CborAttrUnsignedIntegerType;
		attrs->addr.uinteger = &paramIint;
	} else if (AttributeTask_IsString(paramID)) {
		attrs->type = CborAttrTextStringType;
		attrs->addr.string = paramString;
		attrs->len = sizeof(paramString);
	} else if (AttributeTask_IsFloat(paramID)) {
		attrs->type = CborAttrFloatType;
		attrs->addr.fval = &paramFloat;
	} else {
		attrs->type = CborAttrNullType;
		attrs->attribute = NULL;
	}
	return (attrs->type);
}
static int8_t SaveParameterValue(long long unsigned int id, CborAttrType dataType, struct cbor_attr_t *attrs)
{
     float fValue;
     uint32_t uintValue;
     uint32_t intValue;
     int8_t status = -1;

     switch(dataType)
     {
          case CborAttrIntegerType:
               intValue = (int32_t) (*attrs->addr.integer);
               status = AttributeTask_SetSigned32(id, intValue);
               break;
          case CborAttrUnsignedIntegerType:
               uintValue = (uint32_t) (*attrs->addr.uinteger);
               status = AttributeTask_SetUint32(id, uintValue);
               break;
          case CborAttrTextStringType:
               AttributeTask_SetWithString(id, attrs->addr.string, sizeof(attrs->addr.string));
               break;
          case CborAttrFloatType:   
               fValue = (float) (*attrs->addr.fval);            
               status = AttributeTask_SetFloat(id, fValue);
               break;               
          default:
               break;
     }
     return(status);
}


