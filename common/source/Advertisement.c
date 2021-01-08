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
#include "Attribute.h"
#include "Advertisement.h"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static LczSensorAdEvent_t ad;
static LczSensorRspWithHeader_t rsp;

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
/* clang-format off */
static struct bt_data bt_rsp[] = {
    BT_DATA(BT_DATA_MANUFACTURER_DATA, &rsp, sizeof(rsp))
};
/* clang-format on */

static struct bt_conn_cb connection_callbacks;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void adv_disconnected(struct bt_conn *conn, uint8_t reason);

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

	connection_callbacks.disconnected = adv_disconnected;
	bt_conn_cb_register(&connection_callbacks);

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

int Advertisement_Start(void)
{
	int r = 0;

#ifndef CONFIG_ADVERTISEMENT_DISABLE
	if (!advertising) {
		r = bt_le_adv_start(&bt_param, bt_ad, ARRAY_SIZE(bt_ad), bt_rsp,
				    ARRAY_SIZE(bt_rsp));
		advertising = (r == 0);
		LOG_INF("Advertising start (%d)", r);
	}
#endif
	return r;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void adv_disconnected(struct bt_conn *conn, uint8_t reason)
{
	advertising = false;
	Advertisement_Start();
}