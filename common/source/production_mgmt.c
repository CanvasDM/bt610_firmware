/**
 * @file production_mgmt.h
 *
 * @brief SMP interface for Attribute Command Group
 *
 * Copyright (c) 2021-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <limits.h>
#include <string.h>
#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_encode.h>
#include <zcbor_bulk/zcbor_bulk_priv.h>
#include "mgmt/mgmt.h"

#include "UserInterfaceTask.h"
#include "AdcBt6.h"
#include "attr.h"
#include "production_mgmt.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define PRODUCTION_MGMT_HANDLER_CNT                                                                \
	(sizeof production_mgmt_handlers / sizeof production_mgmt_handlers[0])

/* These are element sizes for specific messages */
#define PRODUCTION_MGMT_REV_ECHO_BUFFER_SIZE 64

/* These are defines for specific messages */
#define PRODUCTION_MGMT_CALIBRATION_SCALER 10000.0f

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int production_mgmt_init(const struct device *device);
static int production_mgmt_rev_echo(struct mgmt_ctxt *ctxt);
static int production_mgmt_test_led(struct mgmt_ctxt *ctxt);
static int production_mgmt_calibrate_thermistor(struct mgmt_ctxt *ctxt);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct mgmt_handler production_mgmt_handlers[] = {
	[PRODUCTION_MGMT_ID_REV_ECHO] = {
		production_mgmt_rev_echo, production_mgmt_rev_echo
	},
	[PRODUCTION_MGMT_ID_CALIBRATE_THERMISTOR] = {
		.mh_write = production_mgmt_calibrate_thermistor,
		.mh_read = production_mgmt_calibrate_thermistor,
	},
	[PRODUCTION_MGMT_ID_TEST_LED] = {
		.mh_write = production_mgmt_test_led,
		.mh_read = production_mgmt_test_led,
	},
};

static struct mgmt_group production_mgmt_group = {
	.mg_handlers = production_mgmt_handlers,
	.mg_handlers_count = PRODUCTION_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_PRODUCTION_CMD,
};
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SYS_INIT(production_mgmt_init, APPLICATION, 99);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int production_mgmt_init(const struct device *device)
{
	ARG_UNUSED(device);

	mgmt_register_group(&production_mgmt_group);

	return 0;
}

static int production_mgmt_rev_echo(struct mgmt_ctxt *ctxt)
{
	struct zcbor_string echo = { 0 };
	char rev_echo[PRODUCTION_MGMT_REV_ECHO_BUFFER_SIZE] = { 0 };
	struct zcbor_string key;
	int ok;
	zcbor_state_t *zsd = ctxt->cnbd->zs;
	zcbor_state_t *zse = ctxt->cnbe->zs;

	if (!zcbor_map_start_decode(zsd)) {
		return MGMT_ERR_EUNKNOWN;
	}

	do {
		ok = zcbor_tstr_decode(zsd, &key);

		if (ok) {
			if (key.len == 1 && *key.value == 'd') {
				ok = zcbor_tstr_decode(zsd, &echo);
				break;
			}

			ok = zcbor_any_skip(zsd, NULL);
		}
	} while (ok);

	if (!ok || !zcbor_map_end_decode(zsd)) {
		return MGMT_ERR_EUNKNOWN;
	}
	if (echo.len <= PRODUCTION_MGMT_REV_ECHO_BUFFER_SIZE) {
		size_t i;
		for (i = 0; i < echo.len; i++) {
			rev_echo[i] = echo.value[echo.len - 1 - i];
		}
	} else {
		return MGMT_ERR_ENOMEM;
	}
	ok = zcbor_tstr_put_lit(zse, "r") && zcbor_tstr_put_term(zse, rev_echo);

	return ok ? MGMT_ERR_EOK : MGMT_ERR_ENOMEM;
}

static int production_mgmt_test_led(struct mgmt_ctxt *ctxt)
{
	int r = 0;
	uint32_t duration = UINT32_MAX;
	zcbor_state_t *zse = ctxt->cnbe->zs;
	zcbor_state_t *zsd = ctxt->cnbd->zs;
	size_t decoded;
	int ok;

#ifdef CONFIG_ATTR_SETTINGS_LOCK
	if (attr_is_locked() == true) {
		r = -EACCES;
	} else {
#endif

		struct zcbor_map_decode_key_val production_test_led_decode[] = {
			ZCBOR_MAP_DECODE_KEY_VAL(p1, zcbor_uint32_decode, &duration),
		};
		ok = zcbor_map_decode_bulk(zsd, production_test_led_decode,
					   ARRAY_SIZE(production_test_led_decode), &decoded) == 0;

		if (!ok || decoded == 0) {
			return MGMT_ERR_EINVAL;
		}
		if (duration < UINT32_MAX) {
			r = UserInterfaceTask_LedTest(duration);
		}
#ifdef CONFIG_ATTR_SETTINGS_LOCK
	}
#endif

	/* Cbor encode result */
	ok = zcbor_tstr_put_lit(zse, "r") && zcbor_int32_put(zse, r);

	/* Exit with result */
	return ok ? MGMT_ERR_EOK : MGMT_ERR_ENOMEM;
}

static int production_mgmt_calibrate_thermistor(struct mgmt_ctxt *ctxt)
{
	int r = 0;
	uint32_t c1 = 0;
	uint32_t c2 = 0;
	float ge = 0.0;
	float oe = 0.0;
	zcbor_state_t *zse = ctxt->cnbe->zs;
	zcbor_state_t *zsd = ctxt->cnbd->zs;
	size_t decoded;
	int ok;

#ifdef CONFIG_ATTR_SETTINGS_LOCK
	if (attr_is_locked() == true) {
		r = -EACCES;
	} else {
#endif
		struct zcbor_map_decode_key_val production_cal_decode[] = {
			ZCBOR_MAP_DECODE_KEY_VAL(p1, zcbor_uint32_decode, &c1),
			ZCBOR_MAP_DECODE_KEY_VAL(p2, zcbor_uint32_decode, &c2),
		};
		ok = zcbor_map_decode_bulk(zsd, production_cal_decode,
					   ARRAY_SIZE(production_cal_decode), &decoded) == 0;

		if (!ok) {
			return MGMT_ERR_EINVAL;
		}
		/* Then perform the calibration */
		r = AdcBt6_CalibrateThermistor(((float)c1 / PRODUCTION_MGMT_CALIBRATION_SCALER),
					       ((float)c2 / PRODUCTION_MGMT_CALIBRATION_SCALER),
					       &ge, &oe);
#ifdef CONFIG_ATTR_SETTINGS_LOCK
	}
#endif

	/* Cbor encode result */
	ok = zcbor_tstr_put_lit(zse, "r") && zcbor_int32_put(zse, r) &&
	     zcbor_tstr_put_lit(zse, "ge") && zcbor_float32_put(zse, ge) &&
	     zcbor_tstr_put_lit(zse, "oe") && zcbor_float32_put(zse, oe);

	/* Exit with result */
	return ok ? MGMT_ERR_EOK : MGMT_ERR_ENOMEM;
}
