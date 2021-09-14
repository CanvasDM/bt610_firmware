/**
 * @file Advertisement.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(Advertisement, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <stdlib.h>

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "lcz_bluetooth.h"
#include "Attribute.h"
#include "Advertisement.h"
#include "EventTask.h"
#include "Flags.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/**@brief Macro for converting milliseconds to ticks.
 *
 * @param[in] TIME          Number of milliseconds to convert.
 * @param[in] RESOLUTION    Unit to be converted to in [us/ticks].
 */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME)*1000) / (RESOLUTION))
#define CODED_PHY_STRING "Coded"
#define STANDARD_PHY_STRING "1M PHY"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static LczSensorAdEvent_t ad;
static LczSensorAdExt_t ext;
static LczSensorRspWithHeader_t rsp;
static struct bt_le_ext_adv *adv;
static struct bt_le_ext_adv *extendedAdv;
static SensorMsg_t current;
static bool extendPhyEnbled = false;
static bool connected = false;

enum {
	/**< Number of microseconds in 0.625 milliseconds. */
	UNIT_0_625_MS = 625,
	/**< Number of microseconds in 1.25 milliseconds. */
	UNIT_1_25_MS = 1250,
	/**< Number of microseconds in 10 milliseconds. */
	UNIT_10_MS = 10000
};
#ifndef CONFIG_ADVERTISEMENT_DISABLE
static bool advertising;
static struct bt_le_adv_param bt_param =
	BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
			     BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX,
			     NULL);
static struct bt_le_adv_param bt_extendParam = BT_LE_ADV_PARAM_INIT(
	BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME |
		BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_CODED,
	BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX, NULL);
#endif

static struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &ad, sizeof(ad)),
};

static struct bt_data bt_extAd[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &ext, sizeof(ext))
};

/* When using BT_LE_ADV_OPT_USE_NAME, device name is added to scan response
 * data by controller.
 */
static struct bt_data bt_rsp[] = {
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &rsp, sizeof(rsp)),
};

/* Work queue item used to update advertisement */
struct ad_update_work_item_t {
	struct k_work work;
	SensorMsg_t sensor_event;
} ad_update_work_item;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void CreateAdvertisingParm(void);
static void AdvConnected(struct bt_conn *conn, uint8_t reason);
static void AdvDisconnected(struct bt_conn *conn, uint8_t reason);
static void CreateAdvertisingExtendedParm(void);
static void CreateAdvertisingStandardParm(void);
static void QueuedUpdateAdvertisement(struct k_work *item);

/******************************************************************************/
/* Connection callbacks.                                                      */
/*                                                                            */
/* Handlers for connection related events.                                    */
/*                                                                            */
/* NOTE these have to reside in RAM due to there being a next pointer in the  */
/* structure for appending further list entries.                              */
/******************************************************************************/
static struct bt_conn_cb connection_callbacks = {
	.connected = AdvConnected,
	.disconnected = AdvDisconnected,
	.le_param_req = NULL,
	.le_param_updated = NULL,
	.identity_resolved = NULL,
	.security_changed = NULL,
	.le_phy_updated = NULL,
};

/******************************************************************************/
/* Authorisation callbacks.                                                   */
/*                                                                            */
/* The device IO capabilities, used to determine the pairing method used, are */
/* defined depending upon which callbacks have implementations assigned. In   */
/* the BT6 case, we always want to indicate that we have no keyboard and a    */
/* display only. In this case, when a client connects with a keyboard and a   */
/* display, we enforce password entry pairing.                                */
/*                                                                            */
/* As the callback locations are not changed at runtime, these reside in ROM  */
/* to protect against corruption and hard faulting.                           */
/******************************************************************************/
const struct bt_conn_auth_cb auth_callback = {
	.passkey_display = NULL,
	.passkey_entry = NULL,
	.passkey_confirm = NULL,
	.oob_data_request = NULL,
	.cancel = NULL,
	.pairing_confirm = NULL,
	.pairing_complete = NULL,
	.pairing_failed = NULL,
	.bond_deleted = NULL
};

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int Advertisement_Init(void)
{
	int r = 0;
	size_t count = 1;
	bt_addr_le_t addr;
	char addr_str[BT_ADDR_LE_STR_LEN] = { 0 };
	char bd_addr[BT_ADDR_LE_STR_LEN];
	size_t size = Attribute_GetSize(ATTR_INDEX_bluetoothAddress);

	bt_id_get(&addr, &count);
	if (count < 1) {
		LOG_DBG("Creating new address");
		bt_addr_le_copy(&addr, BT_ADDR_LE_ANY);
		r = bt_id_create(&addr, NULL);
	}
	bt_addr_le_to_str(&addr, addr_str, sizeof(addr_str));
	LOG_INF("Bluetooth Address: %s count: %d status: %d",
		log_strdup(addr_str), count, r);

	/* Remove ':' from default format */
	size_t i;
	size_t j;
	for (i = 0, j = 0; j < size - 1; i++) {
		if (addr_str[i] != ':') {
			bd_addr[j] = addr_str[i];
			j += 1;
		}
	}
	bd_addr[j] = 0;
	Attribute_SetString(ATTR_INDEX_bluetoothAddress, bd_addr, size - 1);

	ad.companyId = LAIRD_CONNECTIVITY_MANUFACTURER_SPECIFIC_COMPANY_ID1;
	ad.protocolId = BTXXX_1M_PHY_AD_PROTOCOL_ID;
	ad.networkId = 0;
	ad.flags = 0;
	memcpy(&ad.addr, &addr.a, sizeof(bt_addr_t));
	ad.recordType = SENSOR_EVENT_RESET;
	ad.resetCount = 0;

	rsp.companyId = LAIRD_CONNECTIVITY_MANUFACTURER_SPECIFIC_COMPANY_ID1;
	rsp.protocolId = BTXXX_1M_PHY_RSP_PROTOCOL_ID;
	rsp.rsp.productId = BT6XX_PRODUCT_ID;
	rsp.rsp.firmwareVersionMajor = VERSION_MAJOR;
	rsp.rsp.firmwareVersionMinor = VERSION_MINOR;
	rsp.rsp.firmwareVersionPatch = VERSION_PATCH;
	rsp.rsp.firmwareType = 0;
	rsp.rsp.configVersion = 0;
	rsp.rsp.hardwareVersion = 0;

	ext.ad = ad;
	ext.ad.protocolId = BTXXX_CODED_PHY_AD_PROTOCOL_ID;
	ext.rsp.productId = rsp.rsp.productId;
	ext.rsp.firmwareVersionMajor = rsp.rsp.firmwareVersionMajor;
	ext.rsp.firmwareVersionMinor = rsp.rsp.firmwareVersionMinor;
	ext.rsp.firmwareVersionPatch = rsp.rsp.firmwareVersionPatch;
	ext.rsp.firmwareType = rsp.rsp.firmwareType;
	ext.rsp.configVersion = rsp.rsp.configVersion;
	ext.rsp.hardwareVersion = rsp.rsp.hardwareVersion;

	bt_conn_cb_register(&connection_callbacks);
	r = bt_conn_auth_cb_register(&auth_callback);

	CreateAdvertisingParm();
	memset(&current, 0, sizeof(SensorMsg_t));

	/* Delayed work item for ad update */
	k_work_init(&ad_update_work_item.work, QueuedUpdateAdvertisement);

	return r;
}

int Advertisement_IntervalUpdate(void)
{
	int r = 0;
	uint32_t advetInterval = 0;

	Attribute_Get(ATTR_INDEX_advertisingInterval, &advetInterval,
		      sizeof(advetInterval));

	advetInterval = MSEC_TO_UNITS(advetInterval, UNIT_0_625_MS);
	bt_param.interval_max = advetInterval + BT_GAP_ADV_FAST_INT_MAX_1;
	bt_param.interval_min = advetInterval;

	bt_extendParam.interval_max = bt_param.interval_max;
	bt_extendParam.interval_min = bt_param.interval_min;

	if (advertising == true) {
		r = Advertisement_End();
		if (r == 0) {
			if (extendPhyEnbled == true) {
				r = bt_le_ext_adv_update_param(extendedAdv,
							       &bt_extendParam);
			} else {
				r = bt_le_ext_adv_update_param(adv, &bt_param);
			}

			LOG_INF("update interval (%d)", r);
		}
		r = Advertisement_Start();
	} else {
		if (extendPhyEnbled == true) {
			r = bt_le_ext_adv_update_param(extendedAdv,
						       &bt_extendParam);
		} else {
			r = bt_le_ext_adv_update_param(adv, &bt_param);
		}

		LOG_INF("update interval (%d)", r);
	}

	return r;
}

int Advertisement_IntervalDefault(void)
{
	int r = 0;
	uint16_t advertIntervalDefault = 0;

	Attribute_GetDefault(ATTR_INDEX_advertisingInterval,
			     &advertIntervalDefault,
			     sizeof(advertIntervalDefault));

	advertIntervalDefault =
		MSEC_TO_UNITS(advertIntervalDefault, UNIT_0_625_MS);
	bt_param.interval_max =
		advertIntervalDefault + BT_GAP_ADV_FAST_INT_MAX_1;
	bt_param.interval_min = advertIntervalDefault;

	if (advertising == true) {
		r = Advertisement_End();
	}
	if (r == 0) {
		r = bt_le_ext_adv_update_param(adv, &bt_param);
	}
	LOG_INF("update interval to default(%d)", r);

	return r;
}

int Advertisement_Update(SensorMsg_t *sensor_event)
{
	ad_update_work_item.sensor_event = *sensor_event;
	k_work_submit(&ad_update_work_item.work);

	return 0;
}

int Advertisement_End(void)
{
	int r = 0;
	char *phyType;
	if (extendPhyEnbled == true) {
		r = bt_le_ext_adv_stop(extendedAdv);
		phyType = CODED_PHY_STRING;
	} else {
		r = bt_le_ext_adv_stop(adv);
		phyType = STANDARD_PHY_STRING;
	}

	LOG_INF("Advertising %s end (%d)", phyType, r);
	advertising = false;

	return r;
}

int Advertisement_Start(void)
{
	int r = 0;
	char *phyType;

#ifndef CONFIG_ADVERTISEMENT_DISABLE
	if (!advertising) {
		if (extendPhyEnbled == true) {
			r = bt_le_ext_adv_start(extendedAdv, NULL);
			phyType = CODED_PHY_STRING;
		} else {
			r = bt_le_ext_adv_start(adv, NULL);
			phyType = STANDARD_PHY_STRING;
		}

		advertising = (r == 0);
		LOG_INF("Advertising %s start (%d)", phyType, r);
	}

#endif

	return r;
}

void Advertisement_ExtendedSet(bool status)
{
	if ((extendPhyEnbled == true) && (status == false)) {
		if (advertising == true) {
			Advertisement_End();
		}
		/* Turn off extended adverts, enable standard */
		bt_le_ext_adv_delete(extendedAdv);
		extendPhyEnbled = status;
		CreateAdvertisingStandardParm();
		Advertisement_Start();
	} else if ((extendPhyEnbled == false) && (status == true)) {
		if (advertising == true) {
			Advertisement_End();
		}
		/* Turn on extended adverts, disable standard adverts */
		bt_le_ext_adv_delete(adv);
		extendPhyEnbled = status;
		CreateAdvertisingExtendedParm();
		Advertisement_Start();
	} else {
		/* Nothing to do here already configured */
	}
}

void TestEventMsg(uint16_t event)
{
	static uint32_t idTest = 0;
	uint32_t timeStamp = 0;
	uint32_t dataValue = 0;

	current.id = idTest;
	idTest = idTest + 1;
	current.event.type = event;

	Attribute_Get(ATTR_INDEX_qrtc, &timeStamp, sizeof(timeStamp));
	Attribute_Get(ATTR_INDEX_batteryAge, &dataValue, sizeof(dataValue));

	current.event.timestamp = timeStamp;

	current.event.data.u32 = dataValue;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
void CreateAdvertisingParm(void)
{
	if (Attribute_CodedEnableCheck() == true) {
		CreateAdvertisingExtendedParm();
		extendPhyEnbled = true;
	} else {
		CreateAdvertisingStandardParm();
		extendPhyEnbled = false;
	}
}

static void AdvConnected(struct bt_conn *conn, uint8_t reason)
{
	advertising = false;
	connected = true;
}

static void AdvDisconnected(struct bt_conn *conn, uint8_t reason)
{
	connected = false;
}

void CreateAdvertisingExtendedParm(void)
{
	int err = 0;
	err = bt_le_ext_adv_create(&bt_extendParam, NULL, &extendedAdv);
	if (err) {
		LOG_WRN("Failed to create advertiser set (%d)\n", err);
	}
	err = bt_le_ext_adv_set_data(extendedAdv, bt_extAd,
				     ARRAY_SIZE(bt_extAd), NULL, 0);
	if (err) {
		LOG_WRN("Failed to set advertising data (%d)\n", err);
	}
}

void CreateAdvertisingStandardParm(void)
{
	int err = 0;
	err = bt_le_ext_adv_create(&bt_param, NULL, &adv);
	if (err) {
		LOG_WRN("Failed to create advertiser set (%d)\n", err);
	}
	err = bt_le_ext_adv_set_data(adv, bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
				     ARRAY_SIZE(bt_rsp));
	if (err) {
		LOG_WRN("Failed to set advertising data (%d)\n", err);
	}
}

void QueuedUpdateAdvertisement(struct k_work *item)
{
	struct ad_update_work_item_t *ad_update =
		CONTAINER_OF(item, struct ad_update_work_item_t, work);

	uint16_t networkId = 0;
	uint8_t configVersion = 0;
	uint8_t codedPhySelected = 0;
	int r = 0;

	Attribute_Get(ATTR_INDEX_networkId, &networkId, sizeof(networkId));
	ad.networkId = networkId;
	ad.flags = Flags_Get();

	/* If no event was available, keep the last */
	if (ad_update->sensor_event.event.type != SENSOR_EVENT_RESERVED) {
		ad.recordType = ad_update->sensor_event.event.type;
		ad.id = ad_update->sensor_event.id;
		ad.epoch = ad_update->sensor_event.event.timestamp;
		ad.data = ad_update->sensor_event.event.data;
	}

	Attribute_Get(ATTR_INDEX_configVersion, &configVersion,
		      sizeof(configVersion));
	rsp.rsp.configVersion = configVersion;

	Attribute_Get(ATTR_INDEX_useCodedPhy, &codedPhySelected,
		      sizeof(codedPhySelected));

	ext.ad = ad;

	/* For Coded PHY, be sure to restore the protocol id here.
	 * This is the only field that is different between 1M and
	 * Coded PHY adverts for this part of the advert and we're
	 * using the 1M advert as the data source for both.
	 */
	ext.ad.protocolId = BTXXX_CODED_PHY_AD_PROTOCOL_ID;

	ext.rsp.configVersion = rsp.rsp.configVersion;

	/* Don't stop advertising if in a connection. We still want
	 * the advert content to be as up to date as possible when
	 * advertising restarts, so still update the content. But we
	 * want to avoid the stack issuing any errors by stopping a
	 * a non-advertising device.
	 */
	if (!connected) {
		Advertisement_End();
	}

	if (extendPhyEnbled == true) {
		r = bt_le_ext_adv_set_data(extendedAdv, bt_extAd,
						ARRAY_SIZE(bt_extAd), NULL, 0);
	} else {
		r = bt_le_ext_adv_set_data(adv, bt_ad, ARRAY_SIZE(bt_ad),
						   bt_rsp, ARRAY_SIZE(bt_rsp));
	}

	LOG_INF("update advertising data (%d)", r);

	if (r < 0) {
		LOG_INF("Failed to update advertising data (%d)", r);
	}
	/* Don't start advertising if in a connection. The advert content
	 * has been updated, but if we're in a connection we'll get an
	 * error code being issued due to starting advertising in a
	 * connection.
	 */
	if (!connected) {
		Advertisement_Start();
	}
}
