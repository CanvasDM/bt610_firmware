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
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>
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
#include "lcz_bluetooth.h"
#include "Advertisement.h"
#include "Attribute.h"
#include "BleTask.h"
#include "EventTask.h"

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
#define BLE_TASK_QUEUE_DEPTH 32
#endif
#define BOOTUP_ADVERTISMENT_TIME_MS (15000)

typedef struct BleTaskTag {
	FwkMsgTask_t msgTask;
	bt_addr_le_t bdAddr;
	struct bt_conn *conn;
	bool lfs_mounted;
	uint32_t durationTimeMs;
	bool activeModeStatus;
} BleTaskObj_t;

typedef enum {
	POWER_LEVEL_PLUS_8 = 0,
	POWER_LEVEL_PLUS_7,
	POWER_LEVEL_PLUS_6,
	POWER_LEVEL_PLUS_5,
	POWER_LEVEL_PLUS_4,
	POWER_LEVEL_PLUS_3,
	POWER_LEVEL_PLUS_2,
	POWER_LEVEL_ZERO,
	POWER_LEVEL_MIN_4,
	POWER_LEVEL_MIN_8,
	POWER_LEVEL_MIN_12,
	POWER_LEVEL_MIN_16,
	POWER_LEVEL_MIN_20,
	POWER_LEVEL_MIN_40,
} TxPowerLevel_t;

/* When the Advertising Duration is set to zero by the user, we scale it
 * against the Advertising Interval by this amount.
 */
#define BLE_TASK_ADV_DUR_AT_ZERO_SCALE 15

/* This is the size of the queue used to store events locally */
#define BLE_TASK_ADVERT_QUEUE_SIZE 32

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
static DispatchResult_t BleSensorUpdateMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg);
static DispatchResult_t BleSensorEventMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);

static int BluetoothInit(void);
static int UpdateName(void);
static void TransmitPower(void);
static void set_tx_power(uint8_t handle_type, uint16_t handle,
			 int8_t tx_pwr_lvl);
static void RequestDisconnect(struct bt_conn *ConnectionHandle);
static uint32_t GetAdvertisingDuration(void);
static void DurationTimerCallbackIsr(struct k_timer *timer_id);
static void BootAdvertTimerCallbackIsr(struct k_timer *timer_id);

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static BleTaskObj_t bto;
static struct k_timer durationTimer;
static struct k_timer bootAdvertTimer;

K_THREAD_STACK_DEFINE(bleTaskStack, BLE_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(bleTaskQueue, FWK_QUEUE_ENTRY_SIZE, BLE_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

static struct bt_conn_cb connectionCallbacks = {
	.connected = ConnectedCallback,
	.disconnected = DisconnectedCallback,
	.le_param_updated = le_param_updated,
	.le_param_req = le_param_req,
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

/* This is the local queue used to store an immediate log of events for
 * for advertisements. When full, additional events are still logged to
 * to the file system but discarded from inclusion in advertisements.
 */
K_MSGQ_DEFINE(ble_task_advert_queue, sizeof(SensorMsg_t),
	      BLE_TASK_ADVERT_QUEUE_SIZE, FWK_QUEUE_ALIGNMENT);

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
	case FMC_SENSOR_EVENT:                return BleSensorEventMsgHandler;
	case FMC_SENSOR_UPDATE:               return BleSensorUpdateMsgHandler;
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

	bto.durationTimeMs = 0;
	bto.activeModeStatus = false;
	Framework_RegisterTask(&bto.msgTask);

	bto.msgTask.pTid =
		k_thread_create(&bto.msgTask.threadData, bleTaskStack,
				K_THREAD_STACK_SIZEOF(bleTaskStack),
				BleTaskThread, &bto, NULL, NULL,
				BLE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(bto.msgTask.pTid, THIS_FILE);
}

/* The Zephyr settings module and Laird Connectivity settings both use internal
 * flash that has the default mount point of /lfs.
 */
int lcz_param_file_mount_fs(void)
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
			r = lcz_param_file_mount_fs();
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
		TransmitPower();

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
	uint8_t activeMode = 0;

	k_timer_init(&durationTimer, DurationTimerCallbackIsr, NULL);
	k_timer_init(&bootAdvertTimer, BootAdvertTimerCallbackIsr, NULL);
	r = BluetoothInit();
	while (r != 0) {
		k_sleep(K_SECONDS(1));
	}
	Attribute_Get(ATTR_INDEX_activeMode, &activeMode, sizeof(activeMode));

	if (activeMode == false) {
		Advertisement_ExtendedSet(false);
		k_timer_start(&bootAdvertTimer,
			      K_MSEC(BOOTUP_ADVERTISMENT_TIME_MS), K_NO_WAIT);

	} else {
		Advertisement_Start();
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
	uint8_t codedPhySelected = 0;

	Attribute_Get(ATTR_INDEX_useCodedPhy, &codedPhySelected,
		      sizeof(codedPhySelected));

	/*If the magnet activated the advertisment send non coded PHY message*/
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
	/* Dummy event passed to Advertisement update */
	SensorMsg_t dummy_event = { .event.type = SENSOR_EVENT_RESERVED };

	UNUSED_PARAMETER(pMsgRxer);
	AttrChangedMsg_t *pb = (AttrChangedMsg_t *)pMsg;
	size_t i;
	uint8_t updateData = false;
	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		case ATTR_INDEX_passkey:
			SetPasskey();
			break;
		case ATTR_INDEX_sensorName:
			UpdateName();
			updateData = true;
			break;
		case ATTR_INDEX_advertisingInterval:
			Advertisement_IntervalUpdate();
			break;
		case ATTR_INDEX_txPower:
			TransmitPower();
			break;
		case ATTR_INDEX_activeMode:
			Attribute_Get(ATTR_INDEX_activeMode,
				      &bto.activeModeStatus,
				      sizeof(bto.activeModeStatus));
			break;
		case ATTR_INDEX_networkId:
		case ATTR_INDEX_configVersion:
			//case ATTR_INDEX_flags:
			updateData = true;
			break;
		case ATTR_INDEX_useCodedPhy:
			Advertisement_ExtendedSet(Attribute_CodedEnableCheck());
		default:
			/* Don't care about this attribute. This is a broadcast. */
			break;
		}
	}

	if (updateData == true) {
		Advertisement_Update(&dummy_event);
	}
	return DISPATCH_OK;
}
static DispatchResult_t BleSensorUpdateMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg)
{
	uint8_t activeMode = 0;
	SensorMsg_t sensor_event;
	bool restart_duration_timer = false;

	Attribute_Get(ATTR_INDEX_activeMode, &activeMode, sizeof(activeMode));
	if (activeMode) {
		/* If the duration timer is not already running start it */
		volatile uint32_t timerLeft =
			k_timer_remaining_get(&durationTimer);
		if (timerLeft == 0) {
			/* Any events available? */
			if (k_msgq_num_used_get(&ble_task_advert_queue)) {
				/* Read the next */
				k_msgq_get(&ble_task_advert_queue,
					   &sensor_event, K_FOREVER);
				/* Update the advertisement */
				Advertisement_Update(&sensor_event);
			}
			/* Restart the duration timer */
			restart_duration_timer = true;
		}
	}
	if (restart_duration_timer) {
		/* Get the duration time */
		uint32_t timeMilliSeconds = GetAdvertisingDuration();

		k_timer_start(&durationTimer, K_MSEC(timeMilliSeconds),
			      K_NO_WAIT);
	}
	return DISPATCH_OK;
}

static DispatchResult_t BleSensorEventMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	SensorMsg_t local_event;
	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;
	uint8_t activeMode = 0;

	/* Only store events when in active mode */
	Attribute_Get(ATTR_INDEX_activeMode, &activeMode, sizeof(activeMode));
	if (activeMode) {
		/* If there's space, add this event to our local advert queue */
		if (k_msgq_num_free_get(&ble_task_advert_queue)) {
			/* Store event details */
			local_event.event.timestamp = pEventMsg->timeStamp;
			local_event.event.type = pEventMsg->eventType;
			local_event.event.data = pEventMsg->eventData;
			local_event.id = pEventMsg->id;
			/* Set event details in the advert queue */
			k_msgq_put(&ble_task_advert_queue, &local_event, K_NO_WAIT);
			/* And update the advertisement */
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
						      FMC_SENSOR_UPDATE);
			LOG_DBG("Added Event to advert queue!");
		}
		else {
			LOG_DBG("Advert queue is full!");
		}
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

		r = bt_conn_set_security(bto.conn, BT_SECURITY_L2);
		LOG_DBG("Setting security status: %d", r);

		/*Set the power for the connection*/
		TransmitPower();

		/*Pause the duration timer if it is running*/
		bto.durationTimeMs = k_timer_remaining_get(&durationTimer);
		k_timer_stop(&durationTimer);
	}
}

static void DisconnectedCallback(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected: %s reason: %s", log_strdup(addr),
		lbt_get_hci_err_string(reason));
	bt_conn_unref(bto.conn);
	bto.conn = NULL;

	/*Start the advertisment again*/
	if (Attribute_CodedEnableCheck() == true) {
		Advertisement_ExtendedSet(true);
	}
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_BLE_START_ADVERTISING);

	/*restart the duration timer*/
	if (bto.durationTimeMs > 0) {
		k_timer_start(&durationTimer, K_MSEC(bto.durationTimeMs),
			      K_NO_WAIT);
	}
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

static void TransmitPower(void)
{
	TxPowerLevel_t txPower = 0;
	int8_t powerLevel = 0;
	uint16_t connectionHandle = 0;
	int r = 0;

	Attribute_Get(ATTR_INDEX_txPower, &txPower, sizeof(txPower));

	switch (txPower) {
	case POWER_LEVEL_PLUS_8:
		powerLevel = 8;
		break;
	case POWER_LEVEL_PLUS_7:
		powerLevel = 7;
		break;
	case POWER_LEVEL_PLUS_6:
		powerLevel = 6;
		break;
	case POWER_LEVEL_PLUS_5:
		powerLevel = 5;
		break;
	case POWER_LEVEL_PLUS_4:
		powerLevel = 4;
		break;
	case POWER_LEVEL_PLUS_3:
		powerLevel = 3;
		break;
	case POWER_LEVEL_PLUS_2:
		powerLevel = 2;
		break;
	case POWER_LEVEL_ZERO:
		powerLevel = 0;
		break;
	case POWER_LEVEL_MIN_4:
		powerLevel = -4;
		break;
	case POWER_LEVEL_MIN_8:
		powerLevel = -8;
		break;
	case POWER_LEVEL_MIN_12:
		powerLevel = -12;
		break;
	case POWER_LEVEL_MIN_16:
		powerLevel = -16;
		break;
	case POWER_LEVEL_MIN_20:
		powerLevel = -20;
		break;
	case POWER_LEVEL_MIN_40:
		powerLevel = -40;
		break;
	default:
		/*This is the default power level for this device*/
		powerLevel = 8;
		break;
	}
	/*TX power level when connected*/
	r = bt_hci_get_conn_handle(bto.conn, &connectionHandle);
	if (r >= 0) {
		set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN, connectionHandle,
			     powerLevel);
	} else {
		/*TX power level when advertising*/
		set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, powerLevel);
	}
}
static void set_tx_power(uint8_t handle_type, uint16_t handle,
			 int8_t tx_pwr_lvl)
{
	struct bt_hci_cp_vs_write_tx_power_level *cp;
	struct bt_hci_rp_vs_write_tx_power_level *rp;
	struct net_buf *buf, *rsp = NULL;
	int err;

	buf = bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, sizeof(*cp));
	if (!buf) {
		printk("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = handle;
	cp->handle_type = handle_type;
	cp->tx_power_level = tx_pwr_lvl;

	err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf,
				   &rsp);
	if (err) {
		uint8_t reason =
			rsp ? ((struct bt_hci_rp_vs_write_tx_power_level *)
				       rsp->data)
					->status :
				    0;
		printk("Set Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}
	rp = (void *)rsp->data;
	printk("Actual Tx Power: %d\n", rp->selected_tx_power);

	net_buf_unref(rsp);
}

static void RequestDisconnect(struct bt_conn *ConnectionHandle)
{
	int r = 0;
	if (ConnectionHandle != NULL) {
		r = bt_conn_disconnect(ConnectionHandle,
				       BT_HCI_ERR_REMOTE_USER_TERM_CONN);

		if (r < 0) {
			LOG_ERR("dissconnect error: %d", r);
		}
	}
}

static uint32_t GetAdvertisingDuration(void)
{
	uint16_t advertising_duration;
	uint16_t advertising_interval;
	bool get_failed = true;

	if (Attribute_Get(ATTR_INDEX_advertisingDuration, &advertising_duration,
			  sizeof(advertising_duration)) ==
	    sizeof(advertising_duration)) {
		if (Attribute_Get(ATTR_INDEX_advertisingInterval,
				  &advertising_interval,
				  sizeof(advertising_interval)) ==
		    sizeof(advertising_interval)) {
			get_failed = false;
			if (advertising_duration == 0) {
				advertising_duration =
					advertising_interval *
					BLE_TASK_ADV_DUR_AT_ZERO_SCALE;
			}
		}
	}
	if (get_failed) {
		/* If either get failed exit with the max duration */
		advertising_duration = UINT16_MAX;		
	}
	return ((uint32_t)(advertising_duration));
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void DurationTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);

	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_SENSOR_UPDATE);
}
static void BootAdvertTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);

	/*If active mode hasn't been turned on at this point turn off the adverisments*/
	if (bto.activeModeStatus == false) {
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
					      FMC_BLE_END_ADVERTISING);
	}
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
	LOG_ERR("Got an LE Param Updated! Interval = %d Latency = %d Timeout = %d", interval, latency, timeout);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	LOG_ERR("Got an LE Param Update Request!");
	
	//param->timeout = 500;


	return(true);
}