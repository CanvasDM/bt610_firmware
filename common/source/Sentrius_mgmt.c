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
#include <power/reboot.h>
#include <logging/log_ctrl.h>
#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Attribute.h"
#include "UserInterfaceTask.h"
#include "AdcBt6.h"
#include "lcz_qrtc.h"
#include "file_system_utilities.h"
#include "Sentrius_mgmt.h"
#include "HalfFloatDecoder.h"
#include "CBORSupport.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

#define FLOAT_MAX 3.4028235E38

/* These are used to index CBOR message elements. Messages received from 
   clients are divided into Key Value pairs called parameters, with the first
   parameter being called P0, the second P1, etc.
 */
#define SENTRIUS_MGMT_P0_INDEX 0
#define SENTRIUS_MGMT_P1_INDEX 1
#define SENTRIUS_MGMT_P0_NAME_INDEX 0
#define SENTRIUS_MGMT_P0_VALUE_INDEX 1
#define SENTRIUS_MGMT_P1_NAME_INDEX 2
#define SENTRIUS_MGMT_P1_VALUE_INDEX 3

/* These define the number of parameters expected for different message types */
#define SENTRIUS_MGMT_NUM_PARAMETERS_GET_PARAMETER 2
#define SENTRIUS_MGMT_NUM_PARAMETERS_REV_ECHO 2
#define SENTRIUS_MGMT_NUM_PARAMETERS_SET_PARAMETER 2
#define SENTRIUS_MGMT_NUM_PARAMETERS_CALIBRATE_THERMISTORS 2

/* These are element sizes for specific messages */
#define SENTRIUS_MGMT_REV_ECHO_BUFFER_SIZE 64

/* These are defines for specific messages */
#define SENTRIUS_MGMT_CALIBRATION_SCALER 10000.0f
#define SENTRIUS_MGMT_PARAMETER_FILE_PATH "/ext/params.txt"
#define SENTRIUS_MGMT_PARAMETER_DUMP_PATH "/ext/dump.txt"

/* For float conversion routines, the maximum number of parameters we have
 * to deal with is two, so 4 elements. Defined here in case this changes.
 */
#define SENTRIUS_MGMT_MAX_CBOR_ELEMENTS 4

/**
 * @brief Static instance of a cborTypeList. Note this module is not intended
 *        to be re-entrant so we can safely use a global instance of this.
 *        Declared globally rather than locally due to the pretty heavy stack
 *        usage.
 */
CONSTRUCT_CBOR_TYPE_LIST(paramTypeList, SENTRIUS_MGMT_MAX_CBOR_ELEMENTS,
			 ATTR_MAX_STR_SIZE);

/* Union type that can cope with all forms of floating point data.
 * We use this when reading float data from CBOR messages because we
 * don't know what floating point format will be used for data expected
 * to be float. An instance of this is used when reading out floating point
 * data so that we can convert back from the type used to the desired
 * single precision float point format.
 */
typedef union _floatContainer_t {
	uint16_t halfFloatValue;
	float floatValue;
	double doubleValue;
	long long int integerValue;
} floatContainer_t;

static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs);

static int SaveParameterValue(attr_idx_t id, CborAttrType dataType,
			      struct cbor_attr_t *attrs);

static int FloatParameterExternalValueType(CborType cborType,
					   struct cbor_attr_t *attrs,
					   floatContainer_t *floatContainer);

static int FloatParameterExternalToInternal(CborType externalFormat,
					    AttrType_t internalFormat,
					    struct cbor_attr_t *attrs,
					    floatContainer_t *floatContainer,
					    float *outData);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct mgmt_handler sentrius_mgmt_handlers[] = {
    /* pystart - mgmt handlers */
    [SENTRIUS_MGMT_ID_GETPARAMETER] = {
         .mh_write = Sentrius_mgmt_GetParameter,
         .mh_read = Sentrius_mgmt_GetParameter,
    },
    [SENTRIUS_MGMT_ID_SETPARAMETER] = {
         .mh_write = Sentrius_mgmt_SetParameter,
         .mh_read = Sentrius_mgmt_SetParameter,
    },
    [SENTRIUS_MGMT_ID_REVECHO] = {
         .mh_write = Sentrius_mgmt_RevEcho,
         .mh_read = Sentrius_mgmt_RevEcho,
    },
    [SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR] = {
         .mh_write = Sentrius_mgmt_CalibrateThermistor,
         .mh_read = Sentrius_mgmt_CalibrateThermistor,
    },
    [SENTRIUS_MGMT_ID_TESTLED] = {
         .mh_write = Sentrius_mgmt_TestLed,
         .mh_read = Sentrius_mgmt_TestLed,
    },
    [SENTRIUS_MGMT_ID_CALIBRATETHERMISTOR_VERSION2] = {
         .mh_write = Sentrius_mgmt_CalibrateThermistor_Version2,
         .mh_read = Sentrius_mgmt_CalibrateThermistor_Version2,
    },
    [SENTRIUS_MGMT_ID_SET_RTC] = {
         .mh_write = Sentrius_mgmt_Set_Rtc,
         .mh_read = Sentrius_mgmt_Set_Rtc,
    },
    [SENTRIUS_MGMT_ID_GET_RTC] = {
         .mh_write = Sentrius_mgmt_Get_Rtc,
         .mh_read = Sentrius_mgmt_Get_Rtc,
    },
    [SENTRIUS_MGMT_ID_LOAD_PARAMETER_FILE] = {
         .mh_write = Sentrius_mgmt_Load_Parameter_File,
         .mh_read = Sentrius_mgmt_Load_Parameter_File,
    },
    [SENTRIUS_MGMT_ID_DUMP_PARAMETER_FILE] = {
         .mh_write = Sentrius_mgmt_Dump_Parameter_File,
         .mh_read = Sentrius_mgmt_Dump_Parameter_File,
    },
    [SENTRIUS_MGMT_ID_PREPARE_LOG] = {
         .mh_write = Sentrius_mgmt_Prepare_Log,
         .mh_read = Sentrius_mgmt_Prepare_Log,
    },
    [SENTRIUS_MGMT_ID_ACK_LOG] = {
         .mh_write = Sentrius_mgmt_Ack_Log,
         .mh_read = Sentrius_mgmt_Ack_Log,
    },
    [SENTRIUS_MGMT_ID_FACTORY_RESET] = {
         .mh_write = Sentrius_mgmt_Factory_Reset,
         .mh_read = Sentrius_mgmt_Factory_Reset,
    },
    /* pyend */
};

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

/* These are placeholders for data read from CBOR messages */
static long long unsigned int paramUint;
static long long int paramIint;
static bool paramBool;
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
	bool boolData;
	char bufferData[ATTR_MAX_STR_SIZE];
	int getResult = -1;
	struct cbor_attr_t
		params_value[SENTRIUS_MGMT_NUM_PARAMETERS_GET_PARAMETER] = { 0 };
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
		getResult = Attribute_Get(paramID, &intData, sizeof(intData));
		err |= cbor_encode_int(&ctxt->encoder, intData);
		break;
	case CborAttrUnsignedIntegerType:
		getResult = Attribute_Get(paramID, &uintData, sizeof(uintData));
		err |= cbor_encode_uint(&ctxt->encoder, uintData);
		break;
	case CborAttrTextStringType:
		getResult =
			Attribute_Get(paramID, bufferData, ATTR_MAX_STR_LENGTH);
		err |= cbor_encode_text_stringz(&ctxt->encoder, bufferData);
		break;
	case CborAttrFloatType:
		getResult =
			Attribute_Get(paramID, &floatData, sizeof(floatData));
		err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType,
						  &floatData);
		break;
	case CborAttrBooleanType:
		getResult = Attribute_Get(paramID, &boolData, sizeof(boolData));
		err |= cbor_encode_boolean(&ctxt->encoder, boolData);
		break;

	default:
		/* No other types are supported */
		return MGMT_ERR_EINVAL;
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, getResult);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_RevEcho(struct mgmt_ctxt *ctxt)
{
	char echo_buf[SENTRIUS_MGMT_REV_ECHO_BUFFER_SIZE] = { 0 };
	char rev_buf[SENTRIUS_MGMT_REV_ECHO_BUFFER_SIZE] = { 0 };
	size_t echo_len = 0;

	const struct cbor_attr_t attrs[
		SENTRIUS_MGMT_NUM_PARAMETERS_REV_ECHO] = {
        [SENTRIUS_MGMT_P0_INDEX] = {
            .attribute = "d",
            .type = CborAttrTextStringType,
            .addr.string = echo_buf,
            .len = sizeof echo_buf,
            .nodefault = true,
        },
        [SENTRIUS_MGMT_P1_INDEX] = {
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

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_SetParameter(struct mgmt_ctxt *ctxt)
{
	long long unsigned int paramID = ATTR_TABLE_SIZE + 1;
	int readCbor = 0;
	struct CborValue dataCopy = ctxt->it;
	struct cbor_attr_t
		params_value[SENTRIUS_MGMT_NUM_PARAMETERS_SET_PARAMETER] = { 0 };
	CborAttrType parameterDataType;
	int setResult = -1;
	CborError err;
	floatContainer_t floatContainer;
	int result = 0;
	/* Local context copy for non-destructive iteration */
	struct mgmt_ctxt localCtxt = *ctxt;

	/* This is global so reset before use */
	paramTypeList.typeIndex = 0;

	/* Extract details of the message types */
	err = GetMessageElementTypes(&localCtxt.it, 0, ATTR_MAX_STR_SIZE,
				     &paramTypeList);

	/* Don't proceed if the types weren't extracted OK */
	if (err != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Get the parameter id */
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

	/* Type from p1 look up id number match to type */
	parameterDataType =
		ParameterValueType((attr_idx_t)paramID, params_value);

	/* Now check if the expected parameter type can 
	   be modifed by CBOR to save space.            */
	if (Attribute_GetType(paramID) == ATTR_TYPE_FLOAT) {
		/* If it can, we need to remap the params_value
		   structure so we can convert the data to internal
		   format */
		result = FloatParameterExternalValueType(
			paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
			params_value, &floatContainer);
		/* Again don't proceed if the type didn't convert */
		if (result != 0) {
			return MGMT_ERR_EINVAL;
		}
	}

	/* Now go ahead and read the CBOR value */
	readCbor = cbor_read_object(&dataCopy, params_value);
	if (readCbor != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Convert it back if needed */
	if (Attribute_GetType(paramID) == ATTR_TYPE_FLOAT) {
		result = FloatParameterExternalToInternal(
			paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
			ATTR_TYPE_FLOAT, params_value, &floatContainer,
			&paramFloat);

		/* And don't proceed if the type didn't convert */
		if (result != 0) {
			return MGMT_ERR_EINVAL;
		}
	}

	/* Now write the parameter value */
	setResult = SaveParameterValue((attr_idx_t)paramID, parameterDataType,
				       params_value);

	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, setResult);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
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
	/* Copy of the context for non-destructive iteration */
	struct mgmt_ctxt localCtxt = *ctxt;
	CborError err;
	floatContainer_t floatContainerP1;
	floatContainer_t floatContainerP2;
	int result;

	/* This is global so reset before use */
	paramTypeList.typeIndex = 0;

	/* Pull back the element types of the CBOR message */
	err = GetMessageElementTypes(&localCtxt.it, 0, ATTR_MAX_STR_SIZE,
				     &paramTypeList);

	/* Don't proceed if the values weren't extracted OK */
	if (err != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Set up the intended parameters structure */
	struct cbor_attr_t params_attr[] = { { .attribute = "p1",
					       .type = CborAttrFloatType,
					       .addr.fval = &c1,
					       .nodefault = true },
					     { .attribute = "p2",
					       .type = CborAttrFloatType,
					       .addr.fval = &c2,
					       .nodefault = true },
					     { .attribute = NULL } };

	/* Override it with the actual content of the CBOR message */
	result = FloatParameterExternalValueType(
		paramTypeList.typeList[SENTRIUS_MGMT_P0_VALUE_INDEX],
		params_attr, &floatContainerP1);

	if (result != 0) {
		return MGMT_ERR_EINVAL;
	}

	result = FloatParameterExternalValueType(
		paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
		&params_attr[SENTRIUS_MGMT_P1_INDEX], &floatContainerP2);

	if (result != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Read the values */
	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* Now convert them back again */
	result = FloatParameterExternalToInternal(
		paramTypeList.typeList[SENTRIUS_MGMT_P0_VALUE_INDEX],
		ATTR_TYPE_FLOAT, params_attr, &floatContainerP1, &c1);

	if (result != 0) {
		return MGMT_ERR_EINVAL;
	}

	result = FloatParameterExternalToInternal(
		paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
		ATTR_TYPE_FLOAT, &params_attr[SENTRIUS_MGMT_P1_INDEX],
		&floatContainerP2, &c2);

	if (result != 0) {
		return MGMT_ERR_EINVAL;
	}
	/* Then perform the calibration */
	r = AdcBt6_CalibrateThermistor(c1, c2, &ge, &oe);

	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_CalibrateThermistor_Version2(struct mgmt_ctxt *ctxt)
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

	r = AdcBt6_CalibrateThermistor(
		((float)c1 / SENTRIUS_MGMT_CALIBRATION_SCALER),
		((float)c2 / SENTRIUS_MGMT_CALIBRATION_SCALER), &ge, &oe);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Set_Rtc(struct mgmt_ctxt *ctxt)
{
	int r = MGMT_ERR_EINVAL;
	int t = 0;
	long long unsigned int epoch = ULLONG_MAX;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &epoch,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	if (epoch < UINT32_MAX) {
		r = Attribute_SetUint32(ATTR_INDEX_qrtcLastSet, epoch);
		t = lcz_qrtc_set_epoch(epoch);
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "t");
	err |= cbor_encode_int(&ctxt->encoder, t);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Get_Rtc(struct mgmt_ctxt *ctxt)
{
	int t = lcz_qrtc_get_epoch();

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "t");
	err |= cbor_encode_int(&ctxt->encoder, t);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Load_Parameter_File(struct mgmt_ctxt *ctxt)
{
	int r = -EPERM;

	/* The input file is an optional parameter. */
	strncpy(paramString, SENTRIUS_MGMT_PARAMETER_FILE_PATH,
		sizeof(paramString));

	struct cbor_attr_t params_attr[] = { { .attribute = "p1",
					       .type = CborAttrTextStringType,
					       .addr.string = paramString,
					       .len = sizeof(paramString),
					       .nodefault = false },
					     { .attribute = NULL } };

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	r = Attribute_Load(paramString);

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Dump_Parameter_File(struct mgmt_ctxt *ctxt)
{
	int r = -EPERM;
	long long unsigned int type = ULLONG_MAX;
	char *fstr = NULL;

	/* The output file is an optional parameter. */
	strncpy(paramString, SENTRIUS_MGMT_PARAMETER_DUMP_PATH,
		sizeof(paramString));

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &type,
		  .nodefault = true },
		{ .attribute = "p2",
		  .type = CborAttrTextStringType,
		  .addr.string = paramString,
		  .len = sizeof(paramString),
		  .nodefault = false },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return MGMT_ERR_EINVAL;
	}

	/* This will malloc a string as large as maximum parameter file size. */
	if (type < UINT8_MAX) {
		r = Attribute_Dump(&fstr, type);
		if (r >= 0) {
			r = fsu_write_abs(paramString, fstr, strlen(fstr));
			k_free(fstr);
		}
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);

	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Prepare_Log(struct mgmt_ctxt *ctxt)
{
	uint8_t f[LCZ_EVENT_MANAGER_FILENAME_SIZE];
	int r = MGMT_ERR_EINVAL;

	/* Check if we can prepare the log file OK */
	if (lcz_event_manager_prepare_log_file(f)) {
		/* If not, blank the file path */
		f[0] = 0;
	}
	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log prepare */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add file path */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "f");
	err |= cbor_encode_text_string(&ctxt->encoder, f, strlen(f));
	/* Exit with result */
	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

int Sentrius_mgmt_Ack_Log(struct mgmt_ctxt *ctxt)
{
	int r = MGMT_ERR_EINVAL;

	r = lcz_event_manager_delete_log_file();

	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log delete */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}
int Sentrius_mgmt_Factory_Reset(struct mgmt_ctxt *ctxt)
{
	int r = MGMT_ERR_EINVAL;

	r = Attribute_FactoryReset();
	LOG_PANIC();
	k_thread_priority_set(k_current_get(), -CONFIG_NUM_COOP_PRIORITIES);

	k_sleep(K_SECONDS(5));
	sys_reboot(SYS_REBOOT_WARM);

	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log delete */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? MGMT_ERR_ENOMEM : 0;
}

static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs)
{
	AttrType_t parameterType = Attribute_GetType(paramID);

	attrs->attribute = "p2";
	attrs->nodefault = true;

	switch (parameterType) {
	case ATTR_TYPE_BOOL:
		paramIint = UCHAR_MAX;
		attrs->type = CborAttrBooleanType;
		attrs->addr.boolean = &paramBool;
		break;
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
	int status = -EINVAL;

	switch (dataType) {
	case CborAttrIntegerType:
		if (*attrs->addr.integer >= INT32_MIN &&
		    *attrs->addr.integer <= INT32_MAX) {
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.integer,
					       sizeof(int32_t));
		}
		break;
	case CborAttrUnsignedIntegerType:
		if (*attrs->addr.uinteger >= 0 &&
		    *attrs->addr.uinteger <= UINT32_MAX) {
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.uinteger,
					       sizeof(uint32_t));
		}
		break;

	case CborAttrTextStringType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.string,
				       strlen(attrs->addr.string));
		break;

	case CborAttrFloatType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.fval, sizeof(float));
		break;

	case CborAttrBooleanType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.boolean, sizeof(bool));
		break;

	default:
		break;
	}
	return (status);
}

/**@brief Configures the passed attrs structure for reading back the passed
 *        CBOR element type.
 *
 * @param [in]cborType - The element type.
 * @param [out]attrs - This attributes structure to configure.
 * @retval A Zephyr error code, 0 for success.
 */
static int FloatParameterExternalValueType(CborType cborType,
					   struct cbor_attr_t *attrs,
					   floatContainer_t *floatContainer)
{
	int result = 0;

	switch (cborType) {
	case (CborIntegerType):
		attrs->addr.integer = &floatContainer->integerValue;
		attrs->type = CborAttrIntegerType;
		break;
	case (CborHalfFloatType):
		attrs->addr.halffloat = &floatContainer->halfFloatValue;
		attrs->type = CborAttrHalfFloatType;
		break;
	case (CborFloatType):
		/* This will already be configured */
		break;
	case (CborDoubleType):
		attrs->addr.real = &floatContainer->doubleValue;
		attrs->type = CborAttrDoubleType;
		break;
	default:
		result = -EINVAL;
	}
	return (result);
}

/**@brief Converts external data to the required internal format.
 *
 * @param [in]externalFormat - The external data format.
 * @param [in]internalFormat - The internal data format.
 * @retval A Zephyr error code, 0 for success.
 */
static int FloatParameterExternalToInternal(CborType externalFormat,
					    AttrType_t internalFormat,
					    struct cbor_attr_t *attrs,
					    floatContainer_t *floatContainer,
					    float *outData)
{
	int result = 0;

	switch (externalFormat) {
	case (CborIntegerType):
		*outData = ((float)(floatContainer->integerValue));
		attrs->addr.fval = outData;
		break;
	case (CborHalfFloatType):
		*outData = HalfToFloat(floatContainer->halfFloatValue);
		attrs->addr.fval = outData;
		break;
	case (CborFloatType):
		/* This will already be configured */
		break;
	case (CborDoubleType):
		*outData = ((float)(floatContainer->doubleValue));
		attrs->addr.fval = outData;
		break;
	default:
		result = -EINVAL;
	}
	return (result);
}
