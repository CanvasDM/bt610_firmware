/**
 * @file BleTask.c
 * @brief Communication over the ble connection using Zephyr drivers
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(BleTask, CONFIG_LOG_LEVEL_BLE_TASK);
#define THIS_FILE "BleTask"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include "settings/settings.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#if 0
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>
#endif

#if defined(CONFIG_BT_SETTINGS) && defined(CONFIG_FILE_SYSTEM_LITTLEFS)
#include <fs/fs.h>
#include <fs/littlefs.h>
#endif

#include "FrameworkIncludes.h"
#include "laird_bluetooth.h"
#include "Advertisement.h"
#include "lcz_params.h"
#include "Attribute.h"
#include "BleTask.h"

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(mount_mutex);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef BLE_TASK_PRIORITY
#define BLE_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef BLE_TASK_STACK_DEPTH
#define BLE_TASK_STACK_DEPTH 4096
#endif

#ifndef BLE_TASK_QUEUE_DEPTH
#define BLE_TASK_QUEUE_DEPTH 8
#endif

typedef struct BleTaskTag {
	FwkMsgTask_t msgTask;
	bt_addr_le_t bdAddr;
	struct bt_conn *conn;
	bool lfs_mounted;
} BleTaskObj_t;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void BleTaskThread(void *, void *, void *);

static void DisconnectedCallback(struct bt_conn *conn, uint8_t reason);
static void ConnectedCallback(struct bt_conn *conn, uint8_t r);

static DispatchResult_t StartAdvertisingMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg);

static DispatchResult_t EndAdvertisingMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);

static DispatchResult_t BleAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);

static int BluetoothInit(void);
static int UpdateName(void);
static void AdvertisementAtrributeControl(void);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static BleTaskObj_t bto;

K_THREAD_STACK_DEFINE(bleTaskStack, BLE_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(bleTaskQueue, FWK_QUEUE_ENTRY_SIZE, BLE_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

static struct bt_conn_cb connectionCallbacks = {
	.connected = ConnectedCallback,
	.disconnected = DisconnectedCallback,
};

#if defined(CONFIG_BT_SETTINGS) && defined(CONFIG_FILE_SYSTEM_LITTLEFS)
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(config);
/* clang-format off */
static struct fs_mount_t settings_mnt = {
	.type = FS_LITTLEFS,
	.fs_data = &config,
	.storage_dev = (void *)FLASH_AREA_ID(storage),
	.mnt_point = CONFIG_SETTINGS_MOUNT_POINT
};
/* clang-format on */
#endif

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t BleTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:                     return Framework_UnknownMsgHandler;
	case FMC_BLE_START_ADVERTISING:       return StartAdvertisingMsgHandler;
	case FMC_BLE_END_ADVERTISING:         return EndAdvertisingMsgHandler;
	case FMC_ATTR_CHANGED:                return BleAttrChangedMsgHandler;
	default:                              return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void BleTask_Initialize(void)
{
	bto.msgTask.rxer.id = FWK_ID_BLE_TASK;
	bto.msgTask.rxer.rxBlockTicks = K_FOREVER;
	bto.msgTask.rxer.pMsgDispatcher = BleTaskMsgDispatcher;
	bto.msgTask.timerDurationTicks = K_MSEC(1000);
	bto.msgTask.timerPeriodTicks = K_MSEC(0); /* 0 for one shot */
	bto.msgTask.rxer.pQueue = &bleTaskQueue;

	Framework_RegisterTask(&bto.msgTask);

	bto.msgTask.pTid =
		k_thread_create(&bto.msgTask.threadData, bleTaskStack,
				K_THREAD_STACK_SIZEOF(bleTaskStack),
				BleTaskThread, &bto, NULL, NULL,
				BLE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(bto.msgTask.pTid, THIS_FILE);
}

/* The Zephyr settings module and Laird settings both use internal flash
 * that has the default mount point of /lfs.
 */
int lcz_params_mount_fs(void)
{
	int r = 0;
	k_mutex_lock(&mount_mutex, K_FOREVER);
	if (!bto.lfs_mounted) {
		r = fs_mount(&settings_mnt);
		if (r != 0) {
			LOG_ERR("settings lfs mount: %d", r);
		} else {
			bto.lfs_mounted = true;
		}
	}
	k_mutex_unlock(&mount_mutex);
	return r;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int BluetoothInit(void)
{
	int r = 0;
	do {
		if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
			r = lcz_params_mount_fs();
			if (r != 0) {
				break;
			}
		}

		r = bt_enable(NULL);
		if (r != 0) {
			LOG_ERR("Bluetooth init: %d", r);
			break;
		}

		if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
			r = settings_load();
			if (r != 0) {
				LOG_ERR("settings load: %d", r);
				break;
			}
		}

		bt_conn_cb_register(&connectionCallbacks);

		r = UpdateName();
		if (r != 0) {
			break;
		}

		r = Advertisement_Init();
		if (r != 0) {
			LOG_ERR("Init advertisement error: %d", r);
			break;
		}
		r = Advertisement_IntervalUpdate();
		if (r != 0) {
			LOG_ERR("Advertisment Interval error: %d", r);
			break;
		}

	} while (0);

	return r;
}

static void BleTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	int r;
	BleTaskObj_t *pObj = (BleTaskObj_t *)pArg1;

	r = BluetoothInit();
	while (r != 0) {
		k_sleep(K_SECONDS(1));
	}

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

/******************************************************************************/
/* Framework Message Functions                                               */
/******************************************************************************/
static DispatchResult_t StartAdvertisingMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);
	UNUSED_PARAMETER(pMsgRxer);

	Advertisement_Start();
	return DISPATCH_OK;
}
static DispatchResult_t EndAdvertisingMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);
	UNUSED_PARAMETER(pMsgRxer);

	Advertisement_End();
	return DISPATCH_OK;
}

static DispatchResult_t BleAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsgRxer);
	AttrChangedMsg_t *pb = (AttrChangedMsg_t *)pMsg;
	size_t i;
	uint8_t updateData = false;
	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		case ATTR_INDEX_sensorName:
			UpdateName();
			updateData = true;
			break;
		case ATTR_INDEX_advertisingInterval:
			Advertisement_IntervalUpdate();
			break;
		case ATTR_INDEX_advertisingDuration:
			//();
			break;
		case ATTR_INDEX_advertiseBegin:
			AdvertisementAtrributeControl();
			break;
		case ATTR_INDEX_networkId:
		case ATTR_INDEX_configVersion:
			updateData = true;
			break;

		default:
			/* Don't care about this attribute. This is a broadcast. */
			break;
		}
	}

	if (updateData == true) {
		Advertisement_Update();
	}
	return DISPATCH_OK;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ConnectedCallback(struct bt_conn *conn, uint8_t r)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (r) {
		LOG_ERR("Failed to connect to central %s (%u)",
			log_strdup(addr), r);
		bt_conn_unref(conn);
		bto.conn = NULL;
	} else {
		LOG_INF("Connected: %s", log_strdup(addr));
		bto.conn = bt_conn_ref(conn);

		/* stop advertising so another central cannot connect */
		bt_le_adv_stop();

		r = bt_conn_set_security(bto.conn, BT_SECURITY_L3);
		LOG_DBG("Setting security status: %d", r);
	}
}

static void DisconnectedCallback(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected: %s reason: %s", log_strdup(addr),
		lbt_get_hci_err_string(reason));
	bt_conn_unref(conn);
	bto.conn = NULL;
	Advertisement_Start();
}

/* Update name in Bluetooth stack.  Zephyr will handle updating name in
 * advertisement.
 */
static int UpdateName(void)
{
	int r = -EPERM;
	char name[ATTR_MAX_STR_SIZE] = { 0 };

	Attribute_Get(ATTR_INDEX_sensorName, name, ATTR_MAX_STR_LENGTH);
	r = bt_set_name(name);
	if (r < 0) {
		LOG_ERR("bt_set_name: %s %d", log_strdup(name), r);
	}
	return r;
}
/* The attribute parameter can control when advertisments start and stop
 * without having to press a button.
 */
static void AdvertisementAtrributeControl(void)
{
	uint8_t avertControl;

	Attribute_Get(ATTR_INDEX_advertiseBegin, &avertControl,
		      sizeof(avertControl));

	if (avertControl == 1) {
		Advertisement_Start();
	} else {
		Advertisement_End();
	}
}
