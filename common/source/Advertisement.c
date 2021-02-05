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

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "laird_bluetooth.h"
#include "Attribute.h"
#include "Advertisement.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/**@brief Macro for converting milliseconds to ticks.
 *
 * @param[in] TIME          Number of milliseconds to convert.
 * @param[in] RESOLUTION    Unit to be converted to in [us/ticks].
 */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME)*1000) / (RESOLUTION))

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static LczSensorAdEvent_t ad;
static LczSensorRspWithHeader_t rsp;
static uint8_t pairCheck;
static uint8_t passCheck;

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
#endif

static struct bt_data bt_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &ad, sizeof(ad))
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
static char *ble_addr(struct bt_conn *conn);
static void adv_disconnected(struct bt_conn *conn, uint8_t reason);

static void AuthPasskeyDisplayCb(struct bt_conn *conn, unsigned int passkey);
static void AuthPasskeyEntryCb(struct bt_conn *conn);
static void AuthCancelCb(struct bt_conn *conn);
static void AuthPairingConfirmCb(struct bt_conn *conn);
static void AuthPairingCompleteCb(struct bt_conn *conn, bool bonded);
static void AuthPasskeyConfirmCb(struct bt_conn *conn, unsigned int passkey);
static void AuthPairingFailedCb(struct bt_conn *conn,
				enum bt_security_err reason);

static void DurationTimerCallbackIsr(struct k_timer *timer_id);

static void SetPasskey(void);

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

	k_timer_init(&advertisementDurationTimer, DurationTimerCallbackIsr,
		     NULL);

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

	connection_callbacks.disconnected = adv_disconnected;
	bt_conn_cb_register(&connection_callbacks);

	const struct bt_conn_auth_cb auth_callback = {
		.passkey_display = AuthPasskeyDisplayCb,
		.passkey_entry = AuthPasskeyEntryCb,
		.passkey_confirm = AuthPasskeyConfirmCb,
		.cancel = AuthCancelCb,
		.pairing_confirm = AuthPairingConfirmCb,
		.pairing_complete = AuthPairingCompleteCb,
		.pairing_failed = AuthPairingFailedCb
	};
	r = bt_conn_auth_cb_register(&auth_callback);

	SetPasskey();

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
	ad.networkId = 0;
	ad.flags = 0;

	ad.recordType = 0;
	ad.id = 0;
	ad.epoch = 0;
	ad.data = 0;

	rsp.rsp.productId = 0;
	rsp.rsp.configVersion = 0;

	int r = bt_le_adv_update_data(bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
				      ARRAY_SIZE(bt_rsp));
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

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SetPasskey(void)
{
	bt_passkey_set(BT_PASSKEY_INVALID);
	bt_passkey_set(123456);
}

static char *ble_addr(struct bt_conn *conn)
{
	static char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	return addr;
}
static void AuthPasskeyDisplayCb(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Passkey for %s: %06u\n", log_strdup(addr), passkey);
}
static void AuthPasskeyEntryCb(struct bt_conn *conn)
{
	LOG_DBG("Not expected for peripheral");
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
void AuthPairingConfirmCb(struct bt_conn *conn)
{
	char *addr = ble_addr(conn);

	pairCheck = 1;

	bt_conn_auth_pairing_confirm(conn);
	LOG_INF("Pairing confirmed: %s", log_strdup(addr));
}
static void AuthPairingCompleteCb(struct bt_conn *conn, bool bonded)
{
	char *addr = ble_addr(conn);

	LOG_INF("Pairing completed: %s, bonded: %d", log_strdup(addr), bonded);
}

static void AuthPasskeyConfirmCb(struct bt_conn *conn, unsigned int passkey)
{
	passCheck = 1;
}
static void AuthPairingFailedCb(struct bt_conn *conn,
				enum bt_security_err reason)
{
	LOG_DBG(".");
	///if (conn == st.conn) {
	//		st.paired = false;
	//}
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void DurationTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_BLE_END_ADVERTISING);
}