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
#include "laird_bluetooth.h"
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
typedef struct {
	SensorEvent_t event;
	uint32_t id;

} SensorMsg_t;
/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static LczSensorAdEvent_t ad;
static LczSensorAdExt_t ext;
static LczSensorRspWithHeader_t rsp;
static struct bt_le_ext_adv *adv;
static SensorMsg_t current;

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
static const struct bt_data AD[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x36, 0xa3, 0x4d, 0x40, 0xb6, 0x70,
		      0x69, 0xa6, 0xb1, 0x4e, 0x84, 0x9e, 0x60, 0x7c, 0x78,
		      0x43),
};
static struct bt_le_adv_param bt_param =
	BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_NAME,
			     BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX,
			     NULL);

static struct bt_le_ext_adv_start_param bt_extParam = { .timeout = 0,
							.num_events = 0 };
#endif

static struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &ad, sizeof(ad))
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

static struct bt_conn_cb connection_callbacks;
static struct k_timer advertisementDurationTimer;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void createAdvertisingCoded(void);
static char *ble_addr(struct bt_conn *conn);
static void adv_disconnected(struct bt_conn *conn, uint8_t reason);
static void AuthCancelCb(struct bt_conn *conn);
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

	/* remove ':' from default format */
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
	ext.rsp.productId = rsp.rsp.productId;
	ext.rsp.firmwareVersionMajor = rsp.rsp.firmwareVersionMajor;
	ext.rsp.firmwareVersionMinor = rsp.rsp.firmwareVersionMinor;
	ext.rsp.firmwareVersionPatch = rsp.rsp.firmwareVersionPatch;
	ext.rsp.firmwareType = rsp.rsp.firmwareType;
	ext.rsp.configVersion = rsp.rsp.configVersion;
	ext.rsp.hardwareVersion = rsp.rsp.hardwareVersion;

	connection_callbacks.disconnected = adv_disconnected;
	bt_conn_cb_register(&connection_callbacks);

	const struct bt_conn_auth_cb auth_callback = {
		.cancel = AuthCancelCb,
	};
	r = bt_conn_auth_cb_register(&auth_callback);

	//createAdvertisingCoded();
	SetPasskey();
	memset(&current, 0, sizeof(SensorMsg_t));
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

	return r;
}
int Advertisement_Update(void)
{
	uint16_t networkId = 0;
	uint8_t configVersion = 0;
	uint8_t codedPhySelected = 0;
	int r = 0;

	Attribute_Get(ATTR_INDEX_networkId, &networkId, sizeof(networkId));
	ad.networkId = networkId;
	ad.flags = Flags_Get();
	EventTask_GetCurrentEvent(&current.id, &current.event);

	ad.recordType = current.event.type;
	ad.id = current.id;
	ad.epoch = current.event.timestamp;
	ad.data = current.event.data;

	Attribute_Get(ATTR_INDEX_configVersion, &configVersion,
		      sizeof(configVersion));
	rsp.rsp.configVersion = configVersion;

	Attribute_Get(ATTR_INDEX_useCodedPhy, &codedPhySelected,
		      sizeof(codedPhySelected));

	if (codedPhySelected == 1) {
		ext.ad = ad;
		ext.rsp.configVersion = rsp.rsp.configVersion;

		r = bt_le_adv_update_data(bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
					  ARRAY_SIZE(bt_rsp));
	} else {
		r = bt_le_adv_update_data(bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
					  ARRAY_SIZE(bt_rsp));
	}

	if (r < 0) {
		LOG_INF("Failed to update advertising data (%d)", r);
	}
	return r;
}

int Advertisement_End(void)
{
	int r = 0;
	r = bt_le_adv_stop();
	LOG_INF("Advertising end (%d)", r);
	advertising = false;

	return r;
}
int Advertisement_ExtendedEnd(void)
{
	int r = 0;
	//r = bt_le_ext_adv_stop(adv);
	LOG_INF("Advertising end (%d)", r);
	advertising = false;

	return r;
}
int Advertisement_Start(void)
{
	int r = 0;

#ifndef CONFIG_ADVERTISEMENT_DISABLE
	uint32_t advertDuration = 0;

	if (!advertising) {
		r = bt_le_adv_start(&bt_param, bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
				    ARRAY_SIZE(bt_rsp));

		advertising = (r == 0);
		LOG_INF("Advertising start (%d)", r);
	}

	Attribute_Get(ATTR_INDEX_advertisingDuration, &advertDuration,
		      sizeof(advertDuration));
	if ((advertDuration != 0) && (advertDuration > bt_param.interval_max)) {
		k_timer_start(&advertisementDurationTimer,
			      K_MSEC(advertDuration), K_NO_WAIT);
	}
#endif
	return r;
}
int Advertisement_ExtendedStart(void)
{
	int r = 0;

#ifndef CONFIG_ADVERTISEMENT_DISABLE
	uint32_t advertDuration = 0;

	if (!advertising) {
		r = bt_le_ext_adv_start(adv, NULL);
		advertising = (r == 0);
		LOG_INF("ExtendedAdvert start (%d)", r);
	}

	Attribute_Get(ATTR_INDEX_advertisingDuration, &advertDuration,
		      sizeof(advertDuration));
	if ((advertDuration != 0) && (advertDuration > bt_param.interval_max)) {
		k_timer_start(&advertisementDurationTimer,
			      K_MSEC(advertDuration), K_NO_WAIT);
	}
#endif
	return r;
}
void SetPasskey(void)
{
	static uint32_t key = 0;

	Attribute_Get(ATTR_INDEX_passkey, &key, sizeof(key));

	bt_passkey_set(BT_PASSKEY_INVALID);
	bt_passkey_set(key);
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
void createAdvertisingCoded(void)
{
	int err = 0;
	struct bt_le_adv_param param =
		BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE,
				     BT_GAP_ADV_FAST_INT_MIN_1,
				     BT_GAP_ADV_FAST_INT_MIN_2, NULL);

	err = bt_le_ext_adv_create(&param, NULL, &adv);
	if (err) {
		LOG_WRN("Failed to create advertiser set (%d)\n", err);
	}

	LOG_INF("Created adv: %p\n", adv);

	err = bt_le_ext_adv_set_data(adv, AD, ARRAY_SIZE(AD), NULL, 0);
	if (err) {
		LOG_WRN("Failed to set advertising data (%d)\n", err);
	}
}

static char *ble_addr(struct bt_conn *conn)
{
	static char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	return addr;
}
static void adv_disconnected(struct bt_conn *conn, uint8_t reason)
{
	advertising = false;
}
static void AuthCancelCb(struct bt_conn *conn)
{
	char *addr = ble_addr(conn);

	LOG_INF("Pairing cancelled: %s", log_strdup(addr));
}