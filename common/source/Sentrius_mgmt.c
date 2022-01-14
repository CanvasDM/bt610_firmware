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
#include <Framework.h>
#include <FrameworkMacros.h>
#include <FrameworkMsgTypes.h>
#include <framework_ids.h>
#include <framework_msgcodes.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "attr.h"
#include "UserInterfaceTask.h"
#include "AdcBt6.h"
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

#define LOCK_INVALID_WAIT_TIME_MS 1500

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
#define SENTRIUS_MGMT_NUM_PARAMETERS_REV_ECHO 2
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
static int FloatParameterExternalValueType(CborType cborType,
					   struct cbor_attr_t *attrs,
					   floatContainer_t *floatContainer);

static int FloatParameterExternalToInternal(CborType externalFormat,
					    enum attr_type internalFormat,
					    struct cbor_attr_t *attrs,
					    floatContainer_t *floatContainer,
					    float *outData);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct mgmt_handler sentrius_mgmt_handlers[] = {
	/* pystart - mgmt handlers */
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
	/* pyend */
};

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

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

	if (attr_is_locked() == true) {
		r = -EPERM;
	} else {
		if (cbor_read_object(&ctxt->it, params_attr) != 0) {
			return -EINVAL;
		}

		if (duration < UINT32_MAX) {
			r = UserInterfaceTask_LedTest(duration);
		}
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);

	return (err != 0) ? -ENOMEM : 0;
}

int Sentrius_mgmt_calibrate_thermistor(struct mgmt_ctxt *ctxt)
{
	int r = 0;
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

	if (attr_is_locked() == true) {
		r = -EPERM;
	}

	if (r == 0) {
		/* This is global so reset before use */
		paramTypeList.typeIndex = 0;

		/* Pull back the element types of the CBOR message */
		err = GetMessageElementTypes(&localCtxt.it, 0,
					     ATTR_MAX_STR_SIZE,
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
	}

	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	/* If no error update the device configuration id */
	if (!err && r == 0) {
		attr_update_config_version();
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

	if (attr_is_locked() == true) {
		r = -EPERM;
	} else {
		r = AdcBt6_CalibrateThermistor(
			((float)c1 / SENTRIUS_MGMT_CALIBRATION_SCALER),
			((float)c2 / SENTRIUS_MGMT_CALIBRATION_SCALER), &ge,
			&oe);
	}

	CborError err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "r");
	err |= cbor_encode_int(&ctxt->encoder, r);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "ge");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &ge);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "oe");
	err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &oe);

	/* If no error update the device configuration id */
	if (!err && r == 0) {
		attr_update_config_version();
	}
	return (err != 0) ? -ENOMEM : 0;
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
					    enum attr_type internalFormat,
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
