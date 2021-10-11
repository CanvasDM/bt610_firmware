/**
 * @file Sentrius_mgmt.c
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
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
#include "FileAccess.h"
#include "SensorTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

#define FLOAT_MAX 3.4028235E38

/* These are used to index CBOR message elements. Messages received from
 * clients are divided into Key Value pairs called parameters, with the first
 * parameter being called P0, the second P1, etc.
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

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static CborAttrType ParameterValueType(attr_idx_t paramID,
				       struct cbor_attr_t *attrs);

static int SaveParameterValue(attr_idx_t id, CborAttrType dataType,
			      struct cbor_attr_t *attrs, bool *modified);

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
	[SENTRIUS_MGMT_ID_GET_PARAMETER] = {
		.mh_write = Sentrius_mgmt_get_parameter,
		.mh_read = Sentrius_mgmt_get_parameter,
	},
	[SENTRIUS_MGMT_ID_SET_PARAMETER] = {
		.mh_write = Sentrius_mgmt_set_parameter,
		.mh_read = Sentrius_mgmt_set_parameter,
	},
	[SENTRIUS_MGMT_ID_REV_ECHO] = {
		Sentrius_mgmt_rev_echo, Sentrius_mgmt_rev_echo
	},
	[SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR] = {
		.mh_write = Sentrius_mgmt_calibrate_thermistor,
		.mh_read = Sentrius_mgmt_calibrate_thermistor,
	},
	[SENTRIUS_MGMT_ID_TEST_LED] = {
		.mh_write = Sentrius_mgmt_test_led,
		.mh_read = Sentrius_mgmt_test_led,
	},
	[SENTRIUS_MGMT_ID_CALIBRATE_THERMISTOR_VERSION_2] = {
		.mh_write = Sentrius_mgmt_calibrate_thermistor_version_2,
		.mh_read = Sentrius_mgmt_calibrate_thermistor_version_2,
	},
	[SENTRIUS_MGMT_ID_SET_RTC] = {
		.mh_write = Sentrius_mgmt_set_rtc,
		.mh_read = Sentrius_mgmt_set_rtc,
	},
	[SENTRIUS_MGMT_ID_GET_RTC] = {
		.mh_write = Sentrius_mgmt_get_rtc,
		.mh_read = Sentrius_mgmt_get_rtc,
	},
	[SENTRIUS_MGMT_ID_LOAD_PARAMETER_FILE] = {
		.mh_write = Sentrius_mgmt_load_parameter_file,
		.mh_read = Sentrius_mgmt_load_parameter_file,
	},
	[SENTRIUS_MGMT_ID_DUMP_PARAMETER_FILE] = {
		.mh_write = Sentrius_mgmt_dump_parameter_file,
		.mh_read = Sentrius_mgmt_dump_parameter_file,
	},
	[SENTRIUS_MGMT_ID_PREPARE_LOG] = {
		.mh_write = Sentrius_mgmt_prepare_log,
		.mh_read = Sentrius_mgmt_prepare_log,
	},
	[SENTRIUS_MGMT_ID_ACK_LOG] = {
		.mh_write = Sentrius_mgmt_ack_log,
		.mh_read = Sentrius_mgmt_ack_log,
	},
	[SENTRIUS_MGMT_ID_FACTORY_RESET] = {
		.mh_write = Sentrius_mgmt_factory_reset,
		.mh_read = Sentrius_mgmt_factory_reset,
	},
	[SENTRIUS_MGMT_ID_PREPARE_TEST_LOG] = {
		.mh_write = Sentrius_mgmt_prepare_test_log,
		.mh_read = Sentrius_mgmt_prepare_test_log,
	},
	[SENTRIUS_MGMT_ID_CHECK_LOCK_STATUS] = {
		.mh_write = Sentrius_mgmt_check_lock_status,
		.mh_read = Sentrius_mgmt_check_lock_status,
	},
	[SENTRIUS_MGMT_ID_SET_LOCK_CODE] = {
		.mh_write = Sentrius_mgmt_set_lock_code,
		.mh_read = Sentrius_mgmt_set_lock_code,
	},
	[SENTRIUS_MGMT_ID_LOCK] = {
		.mh_write = Sentrius_mgmt_lock,
		.mh_read = Sentrius_mgmt_lock,
	},
	[SENTRIUS_MGMT_ID_UNLOCK] = {
		.mh_write = Sentrius_mgmt_unlock,
		.mh_read = Sentrius_mgmt_unlock,
	},
	[SENTRIUS_MGMT_ID_GET_UNLOCK_ERROR_CODE] = {
		.mh_write = Sentrius_mgmt_get_unlock_error_code,
		.mh_read = Sentrius_mgmt_get_unlock_error_code,
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
int Sentrius_mgmt_get_parameter(struct mgmt_ctxt *ctxt)
{
	long long unsigned int paramID = ATTR_TABLE_SIZE + 1;
	int readCbor = 0;
	int64_t intData;
	uint64_t uintData;
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
		return -EINVAL;
	}
	/* Just need the type from p1 look up id don't need the params_value
	 * structure
	 */
	parameterDataType =
		ParameterValueType((attr_idx_t)paramID, params_value);
	/* Encode the response. */
	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");

	/* Get the value */
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
		/* For unrecognised types, we return an error code and value */
		getResult = -EINVAL;
		err |= cbor_encode_text_stringz(&ctxt->encoder, "NULL");
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, getResult);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_rev_echo(struct mgmt_ctxt *ctxt)
{
	char echo_buf[SENTRIUS_MGMT_REV_ECHO_BUFFER_SIZE] = { 0 };
	char rev_buf[SENTRIUS_MGMT_REV_ECHO_BUFFER_SIZE] = { 0 };
	size_t echo_len = 0;

	const struct cbor_attr_t attrs[SENTRIUS_MGMT_NUM_PARAMETERS_REV_ECHO] =
	{
		[SENTRIUS_MGMT_P0_INDEX] = {
			.attribute = "d",
			.type = CborAttrTextStringType,
			.addr.string = echo_buf,
			.len = sizeof(echo_buf),
			.nodefault = true,
		},
		[SENTRIUS_MGMT_P1_INDEX] = {
			.attribute = NULL
		}
	};

	echo_buf[0] = '\0';

	if (cbor_read_object(&ctxt->it, attrs) != 0) {
		return -EINVAL;
	}

	echo_len = strlen(echo_buf);
	size_t i;
	for (i = 0; i < echo_len; i++) {
		rev_buf[i] = echo_buf[echo_len - 1 - i];
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_text_string(&ctxt->encoder, rev_buf, echo_len);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_set_parameter(struct mgmt_ctxt *ctxt)
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
	bool update_config = true;

	/* This is global so reset before use */
	paramTypeList.typeIndex = 0;

	/* Extract details of the message types */
	err = GetMessageElementTypes(&localCtxt.it, 0, ATTR_MAX_STR_SIZE,
				     &paramTypeList);

	/* Don't proceed if the types weren't extracted OK */
	if (err != 0) {
		return -EINVAL;
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
		return -EINVAL;
	}

	/* Type from p1 look up id number match to type */
	parameterDataType =
		ParameterValueType((attr_idx_t)paramID, params_value);

	/* Now check if the expected parameter type can be modified by CBOR to
	 * save space.
	 */
	if (Attribute_GetType(paramID) == ATTR_TYPE_FLOAT) {
		/* If it can, we need to remap the params_value structure so we
		 * can convert the data to internal format
		 */
		result = FloatParameterExternalValueType(
			paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
			params_value, &floatContainer);
		/* Again don't proceed if the type didn't convert */
		if (result != 0) {
			return -EINVAL;
		}
	}

	/* Now go ahead and read the CBOR value */
	readCbor = cbor_read_object(&dataCopy, params_value);
	if (readCbor != 0) {
		return -EINVAL;
	}

	/* Convert it back if needed */
	if (Attribute_GetType(paramID) == ATTR_TYPE_FLOAT) {
		result = FloatParameterExternalToInternal(
			paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
			ATTR_TYPE_FLOAT, params_value, &floatContainer,
			&paramFloat);

		/* And don't proceed if the type didn't convert */
		if (result != 0) {
			return -EINVAL;
		}
	}

	/* Is this a known attribute? */
	if (Attribute_GetType(paramID) == ATTR_TYPE_UNKNOWN) {
		/* No, so don't write anything and exit with an error code */
		setResult = -EINVAL;
	} else {
		/* OK to write the parameter value */
		setResult = SaveParameterValue((attr_idx_t)paramID,
					       parameterDataType, params_value,
					       &update_config);
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
	err |= cbor_encode_uint(&ctxt->encoder, paramID);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
	err |= cbor_encode_int(&ctxt->encoder, setResult);

	/* If no error update the device configuration id */
	if (!err && update_config) {
		Attribute_UpdateConfig();
	}
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_test_led(struct mgmt_ctxt *ctxt)
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
		return -EINVAL;
	}

	if (duration < UINT32_MAX) {
		r = UserInterfaceTask_LedTest(duration);
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_calibrate_thermistor(struct mgmt_ctxt *ctxt)
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
		return -EINVAL;
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
		return -EINVAL;
	}

	result = FloatParameterExternalValueType(
		paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
		&params_attr[SENTRIUS_MGMT_P1_INDEX], &floatContainerP2);

	if (result != 0) {
		return -EINVAL;
	}

	/* Read the values */
	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return -EINVAL;
	}

	/* Now convert them back again */
	result = FloatParameterExternalToInternal(
		paramTypeList.typeList[SENTRIUS_MGMT_P0_VALUE_INDEX],
		ATTR_TYPE_FLOAT, params_attr, &floatContainerP1, &c1);

	if (result != 0) {
		return -EINVAL;
	}

	result = FloatParameterExternalToInternal(
		paramTypeList.typeList[SENTRIUS_MGMT_P1_VALUE_INDEX],
		ATTR_TYPE_FLOAT, &params_attr[SENTRIUS_MGMT_P1_INDEX],
		&floatContainerP2, &c2);

	if (result != 0) {
		return -EINVAL;
	}
	/* Then perform the calibration */
	r = AdcBt6_CalibrateThermistor(c1, c2, &ge, &oe);

	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	/* If no error update the device configuration id */
	if (!err) {
		Attribute_UpdateConfig();
	}
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_calibrate_thermistor_version_2(struct mgmt_ctxt *ctxt)
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
		return -EINVAL;
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

	/* If no error update the device configuration id */
	if (!err) {
		Attribute_UpdateConfig();
	}
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_set_rtc(struct mgmt_ctxt *ctxt)
{
	int r = 0;
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
		return -EINVAL;
	}

	if (Attribute_IsLocked() == true) {
		r = -EPERM;
		t = lcz_qrtc_get_epoch();
	}

	if (r == 0 && epoch < UINT32_MAX) {
		r = Attribute_SetUint32(ATTR_INDEX_qrtc_last_set, epoch);
		t = lcz_qrtc_set_epoch(epoch);
	} else if (r == 0 && epoch >= UINT32_MAX) {
		r = -EINVAL;
		t = lcz_qrtc_get_epoch();
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "t");
	err |= cbor_encode_int(&ctxt->encoder, t);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_get_rtc(struct mgmt_ctxt *ctxt)
{
	int t = lcz_qrtc_get_epoch();

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "t");
	err |= cbor_encode_int(&ctxt->encoder, t);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_load_parameter_file(struct mgmt_ctxt *ctxt)
{
	int r = 0;

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
		return -EINVAL;
	}

	if (Attribute_IsLocked() == true) {
		r = -EPERM;
	}

	if (r == 0) {
		r = Attribute_Load(paramString,
				   SENTRIUS_MGMT_PARAMETER_FEEDBACK_PATH);
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Encode the feedback file path. */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "f");
	err |= cbor_encode_text_string(
		&ctxt->encoder, SENTRIUS_MGMT_PARAMETER_FEEDBACK_PATH,
		strlen(SENTRIUS_MGMT_PARAMETER_FEEDBACK_PATH));
	/* If no error update the device configuration id */
	if (r == 0 && !err) {
		Attribute_UpdateConfig();
	}
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_dump_parameter_file(struct mgmt_ctxt *ctxt)
{
	int r = -EPERM;
	long long unsigned int type = ULLONG_MAX;
	char *fstr = NULL;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &type,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return -EINVAL;
	}
	fsu_delete_abs(SENTRIUS_MGMT_PARAMETER_DUMP_PATH);
	/* This will malloc a string as large as maximum parameter file size. */
	if (type < UINT8_MAX) {
		r = Attribute_Dump(&fstr, type);
		if (r >= 0) {
			/* OK to set the file path now */
			strncpy(paramString, SENTRIUS_MGMT_PARAMETER_DUMP_PATH,
				sizeof(paramString));
			r = fsu_write_abs(paramString, fstr, strlen(fstr));
			k_free(fstr);
		}
	}

	CborError err = 0;
	/* Add file size */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add file path if successful */
	if (r >= 0) {
		err |= cbor_encode_text_stringz(&ctxt->encoder, "n");
		err |= cbor_encode_text_string(&ctxt->encoder, paramString,
					       strlen(paramString));
	}
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_prepare_log(struct mgmt_ctxt *ctxt)
{
	uint8_t n[LCZ_EVENT_MANAGER_FILENAME_SIZE];
	int r = -EINVAL;
	uint32_t s = 0;

	/* Check if we can prepare the log file OK */
	r = lcz_event_manager_prepare_log_file(n, &s);
	if (r != 0) {
		/* If not, blank the file path */
		n[0] = 0;
	}
	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log prepare */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add the file size */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "s");
	err |= cbor_encode_int(&ctxt->encoder, s);
	/* Add file path */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "n");
	err |= cbor_encode_text_string(&ctxt->encoder, n, strlen(n));
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_ack_log(struct mgmt_ctxt *ctxt)
{
	int r = -EINVAL;

	r = lcz_event_manager_delete_log_file();

	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log delete */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_factory_reset(struct mgmt_ctxt *ctxt)
{
	int r = 0;
	uint8_t factoryResetEnabled = 0;

	if (Attribute_IsLocked() == true) {
		r = -EPERM;
	}

	if (r == 0) {
		Attribute_Get(ATTR_INDEX_factory_reset_enable,
			      &factoryResetEnabled,
			      sizeof(factoryResetEnabled));

		if (factoryResetEnabled == 0) {
			r = -EPERM;
		}
	}

	if (r == 0) {
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
					      FWK_ID_USER_IF_TASK,
					      FMC_FACTORY_RESET);
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log delete */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_prepare_test_log(struct mgmt_ctxt *ctxt)
{
	uint8_t n[LCZ_EVENT_MANAGER_FILENAME_SIZE];
	uint32_t s = 0;
	int r = -EINVAL;
	long long unsigned int start_time_stamp = 0;
	long long unsigned int update_rate = 0;
	long long unsigned int event_type = 0;
	long long unsigned int event_count = 0;
	long long unsigned int event_data_type = 0;
	DummyLogFileProperties_t dummy_log_file_properties;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &start_time_stamp,
		  .nodefault = true },
		{ .attribute = "p2",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &update_rate,
		  .nodefault = true },
		{ .attribute = "p3",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &event_type,
		  .nodefault = true },
		{ .attribute = "p4",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &event_count,
		  .nodefault = true },
		{ .attribute = "p5",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &event_data_type,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return -EINVAL;
	}

	dummy_log_file_properties.start_time_stamp =
		((uint32_t)(start_time_stamp));
	dummy_log_file_properties.update_rate = ((uint32_t)(update_rate));
	dummy_log_file_properties.event_type = ((uint8_t)(event_type));
	dummy_log_file_properties.event_count = ((uint32_t)(event_count));
	dummy_log_file_properties.event_data_type =
		((uint8_t)(event_data_type));

	/* Check if we can prepare the log file OK */
	r = lcz_event_manager_prepare_test_log_file(&dummy_log_file_properties,
						    n, &s);
	if (r != 0) {
		/* If not, blank the file path */
		n[0] = 0;
	}
	/* Cbor encode result */
	CborError err = 0;
	/* Add result of log prepare */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add the file size */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "s");
	err |= cbor_encode_int(&ctxt->encoder, s);
	/* Add file path */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "n");
	err |= cbor_encode_text_string(&ctxt->encoder, n, strlen(n));
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_check_lock_status(struct mgmt_ctxt *ctxt)
{
	bool lock_enabled;
	bool lock_active = false;
	uint8_t lock_status;
	int r = 0;

	r = Attribute_Get(ATTR_INDEX_lock, &lock_enabled, sizeof(lock_enabled));

	if (r >= 0) {
		r = Attribute_Get(ATTR_INDEX_lock_status, &lock_status,
				  sizeof(lock_status));

		if (r >= 0) {
			if (lock_status == LOCK_STATUS_SETUP_ENGAGED) {
				lock_active = true;
			}

			/* Completed successfully so return success result
			 * code
			 */
			r = 0;
		}
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add if lock is enabled */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
	err |= cbor_encode_boolean(&ctxt->encoder, lock_enabled);
	/* Add if lock is engaged */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
	err |= cbor_encode_boolean(&ctxt->encoder, lock_active);
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_set_lock_code(struct mgmt_ctxt *ctxt)
{
	long long unsigned int lock_code_tmp = ULLONG_MAX;
	uint32_t lock_code;
	int r = 0;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &lock_code_tmp,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return -EINVAL;
	}

	if (Attribute_IsLocked() == true) {
		r = -EPERM;
	}

	if (r == 0) {
		lock_code = (uint32_t)lock_code_tmp;
		r = Attribute_SetUint32(ATTR_INDEX_settings_passcode,
					lock_code);
	}

	if (r == 0) {
		r = Attribute_SetUint32(ATTR_INDEX_lock, true);
	}

	if (r == 0) {
		/* This sets the lock ready to be engaged when the user
		 * disconnects, when the module is rebooted or when the user
		 * manually requests it with the lock command, but allows
		 * further configuration changes to the unit until then
		 */
		r = Attribute_SetUint32(ATTR_INDEX_lock_status,
					LOCK_STATUS_SETUP_DISENGAGED);
	}

	if (r == 0) {
		LoadSettingPasscode();
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_lock(struct mgmt_ctxt *ctxt)
{
	settingsLockErrorType_t passCodeStatus = SETTINGS_LOCK_ERROR_NO_STATUS;
	int r = 0;

	if (Attribute_IsLocked() == false) {
		/* Load the new passcode */
		LoadSettingPasscode();

		/* Lock the settings */
		Attribute_SetUint32(ATTR_INDEX_lock, true);
		Attribute_SetUint32(ATTR_INDEX_lock_status,
				    LOCK_STATUS_SETUP_ENGAGED);
		passCodeStatus = SETTINGS_LOCK_ERROR_VALID_CODE;

		/* Send feedback about the passcode */
		Attribute_SetUint32(ATTR_INDEX_settings_passcode_status,
				    passCodeStatus);
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */
	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_unlock(struct mgmt_ctxt *ctxt)
{
	settingsLockErrorType_t passCodeStatus = SETTINGS_LOCK_ERROR_NO_STATUS;
	long long unsigned int lock_code_tmp = ULLONG_MAX;
	uint32_t lock_code;
	uint32_t real_lock_code;
	bool permanent_unlock = false;
	int r = 0;

	struct cbor_attr_t params_attr[] = {
		{ .attribute = "p1",
		  .type = CborAttrUnsignedIntegerType,
		  .addr.uinteger = &lock_code_tmp,
		  .nodefault = true },
		{ .attribute = "p2",
		  .type = CborAttrBooleanType,
		  .addr.boolean = &permanent_unlock,
		  .nodefault = true },
		{ .attribute = NULL }
	};

	if (cbor_read_object(&ctxt->it, params_attr) != 0) {
		return -EINVAL;
	}

	if (Attribute_IsLocked() == true) {
		Attribute_GetUint32(&real_lock_code,
				    ATTR_INDEX_settings_passcode);
		lock_code = (uint32_t)lock_code_tmp;

		/* Check if the passcode entered matches */
		if (real_lock_code == lock_code) {
			/* Unlock the settings */
			Attribute_SetUint32(ATTR_INDEX_lock_status,
					    LOCK_STATUS_SETUP_DISENGAGED);
			passCodeStatus = SETTINGS_LOCK_ERROR_VALID_CODE;
		} else {
			passCodeStatus = SETTINGS_LOCK_ERROR_INVALID_CODE;
			r = -EINVAL;
		}

		/* Send feedback to APP about the passcode */
		Attribute_SetUint32(ATTR_INDEX_settings_passcode_status,
				    passCodeStatus);
	}

	if (permanent_unlock == true && Attribute_IsLocked() == false &&
	    r == 0) {
		/* User has requested to remove the lock entirely */
		Attribute_SetUint32(ATTR_INDEX_lock, false);
		Attribute_SetUint32(ATTR_INDEX_lock_status,
				    LOCK_STATUS_NOT_SETUP);
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Exit with result */

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_get_unlock_error_code(struct mgmt_ctxt *ctxt)
{
	settingsLockErrorType_t passCodeStatus = SETTINGS_LOCK_ERROR_NO_STATUS;
	int r = 0;

	r = Attribute_Get(ATTR_INDEX_settings_passcode_status, &passCodeStatus,
			  sizeof(passCodeStatus));

	if (r >= 0) {
		/* Clear status */
		Attribute_SetUint32(ATTR_INDEX_settings_passcode_status,
				    SETTINGS_LOCK_ERROR_NO_STATUS);
	}

	/* Cbor encode result */
	CborError err = 0;
	/* Add result */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	/* Add status */
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
	err |= cbor_encode_int(&ctxt->encoder, passCodeStatus);
	/* Exit with result */

	return (err != 0) ? -ENOMEM : 0;
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
	case ATTR_TYPE_S64:
		paramIint = LLONG_MAX;
		attrs->type = CborAttrIntegerType;
		attrs->addr.integer = &paramIint;
		break;
	case ATTR_TYPE_U8:
	case ATTR_TYPE_U16:
	case ATTR_TYPE_U32:
	case ATTR_TYPE_U64:
		paramUint = ULLONG_MAX;
		attrs->type = CborAttrUnsignedIntegerType;
		attrs->addr.uinteger = &paramUint;
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
			      struct cbor_attr_t *attrs, bool *modified)
{
	int status = -EINVAL;

	switch (dataType) {
	case CborAttrIntegerType:
		if (*attrs->addr.integer >= INT32_MIN &&
		    *attrs->addr.integer <= INT32_MAX) {
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.integer,
					       sizeof(int32_t), modified);
		}
		break;
	case CborAttrUnsignedIntegerType:
		if (*attrs->addr.uinteger >= 0 &&
		    *attrs->addr.uinteger <= UINT32_MAX) {
			status = Attribute_Set(id, Attribute_GetType(id),
					       attrs->addr.uinteger,
					       sizeof(uint32_t), modified);
		}
		break;

	case CborAttrTextStringType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.string,
				       strlen(attrs->addr.string), modified);
		break;

	case CborAttrFloatType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.fval, sizeof(float),
				       modified);
		break;

	case CborAttrBooleanType:
		status = Attribute_Set(id, Attribute_GetType(id),
				       attrs->addr.boolean, sizeof(bool),
				       modified);
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
