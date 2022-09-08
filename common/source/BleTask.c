/**
 * @file BleTask.c
 * @brief Communication over the ble connection using Zephyr drivers
 *
 * Copyright (c) 2020-2021 Laird Connectivity
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
#include <sys/byteorder.h>
#include "settings/settings.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_vs.h>
#include <host/smp.h>

#if defined(CONFIG_BT_SETTINGS) && defined(CONFIG_FILE_SYSTEM_LITTLEFS)
#include <fs/fs.h>
#include <fs/littlefs.h>
#endif

#include "FrameworkIncludes.h"
#include "lcz_bluetooth.h"
#include "Advertisement.h"
#include "attr.h"
#include "BleTask.h"
#include "EventTask.h"
#include "attr_custom_validator.h"

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
#define BOOTUP_ADVERTISMENT_TIME_S (30)
#define BLE_TASK_FORCE_DISCONNECT_DELAY_S (2)

typedef struct BleTaskTag {
	FwkMsgTask_t msgTask;
	bt_addr_le_t bdAddr;
	struct bt_conn *conn;
	bool lfs_mounted;
	uint32_t durationTimeMs;
	bool activeModeStatus;
	bool codedPHYBroadcast;
	bool conn_from_le_coded;
} BleTaskObj_t;

/* The Advertising Duration must always be this times greater than the
 * Advertising Interval.
 */
#define BLE_TASK_ADV_DUR_SCALE 4

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
static DispatchResult_t SeverConnectionHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg);
static DispatchResult_t BleSensorEventMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);
static DispatchResult_t BleEnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg);

static int BluetoothInit(void);
static int UpdateName(void);
static void TransmitPower(void);
static void set_tx_power(uint8_t handle_type, uint16_t handle,
			 int8_t tx_pwr_lvl);
static void StartDisconnectTimer(void);
static void ResetAppDisconnectParam(void);
static void RequestDisconnect(struct bt_conn *ConnectionHandle);
static uint32_t GetAdvertisingDuration(void);
static void DurationTimerCallbackIsr(struct k_timer *timer_id);
static void BootAdvertTimerCallbackIsr(struct k_timer *timer_id);
static void EnterActiveModeTimerCallbackIsr(struct k_timer *timer_id);
static void upgrade_advert_phy_timer_callback_isr(struct k_timer *timer_id);
static void AppDisconnectCallbackIsr(struct k_timer *timer_id);

static void le_param_updated(struct bt_conn *conn, uint16_t interval,
			     uint16_t latency, uint16_t timeout);
static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static BleTaskObj_t bto;
static struct k_timer durationTimer;
static struct k_timer bootAdvertTimer;
static struct k_timer enterActiveModeTimer;
static struct k_timer upgrade_advert_phy_timer;
static struct k_timer mobileAppDisconnectTimer;

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
static FwkMsgHandler_t *BleTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:                 return Framework_UnknownMsgHandler;
	case FMC_BLE_START_ADVERTISING:   return StartAdvertisingMsgHandler;
	case FMC_BLE_END_ADVERTISING:     return EndAdvertisingMsgHandler;
	case FMC_ATTR_CHANGED:            return BleAttrChangedMsgHandler;
	case FMC_BLE_END_CONNECTION:      return SeverConnectionHandler;
	case FMC_SENSOR_EVENT:            return BleSensorEventMsgHandler;
	case FMC_SENSOR_UPDATE:           return BleSensorUpdateMsgHandler;
	case FMC_ENTER_ACTIVE_MODE:       return BleEnterActiveModeMsgHandler;
	default:                          return NULL;
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
	bto.codedPHYBroadcast = false;
	Framework_RegisterTask(&bto.msgTask);

	bto.msgTask.pTid =
		k_thread_create(&bto.msgTask.threadData, bleTaskStack,
				K_THREAD_STACK_SIZEOF(bleTaskStack),
				BleTaskThread, &bto, NULL, NULL,
				BLE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(bto.msgTask.pTid, THIS_FILE);
}

bool ble_is_connected(void)
{
	return (bto.conn != NULL ? true : false);
}

bool ble_conn_last_was_le_coded(void)
{
	return bto.conn_from_le_coded;
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

int attr_prepare_security_level(void)
{
	int level = -1;
	if (bto.conn != NULL) {
		level = bt_conn_get_security(bto.conn);
	}
        return (attr_set_signed32(ATTR_ID_security_level, level));
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int BluetoothInit(void)
{
	int r = 0;
	do {
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
	uint8_t force_phy = BOOT_PHY_DEFAULT;
	BleTaskObj_t *pObj = (BleTaskObj_t *)pArg1;

	k_timer_init(&durationTimer, DurationTimerCallbackIsr, NULL);
	k_timer_init(&bootAdvertTimer, BootAdvertTimerCallbackIsr, NULL);
	k_timer_init(&enterActiveModeTimer, EnterActiveModeTimerCallbackIsr,
		     NULL);
	k_timer_init(&upgrade_advert_phy_timer,
		     upgrade_advert_phy_timer_callback_isr, NULL);
	k_timer_init(&mobileAppDisconnectTimer, AppDisconnectCallbackIsr, NULL);

	r = BluetoothInit();
	while (r != 0) {
		k_sleep(K_SECONDS(1));
	}

	/* Initialise PHY and Shelf/Active state */
	attr_get(ATTR_ID_active_mode, &bto.activeModeStatus,
		      sizeof(bto.activeModeStatus));

	if (bto.activeModeStatus == 1) {
		attr_set_flags(ATTR_ID_bluetooth_flags, FLAG_ACTIVE_MODE_BITMASK);
	} else {
		attr_clear_flags(ATTR_ID_bluetooth_flags, FLAG_ACTIVE_MODE_BITMASK);
	}

	attr_get(ATTR_ID_advertising_phy, &bto.codedPHYBroadcast,
		      sizeof(bto.codedPHYBroadcast));

	attr_get(ATTR_ID_boot_phy, &force_phy, sizeof(force_phy));

	if (force_phy != BOOT_PHY_DEFAULT) {
		/* If a firmware update has taken place, re-advertise in PHY
		 * that was used prior to upgrade, then clear it
		 */
		Advertisement_ExtendedSet((force_phy == BOOT_PHY_CODED));
		Advertisement_Start();
		k_timer_start(&upgrade_advert_phy_timer,
			      K_SECONDS(BOOTUP_ADVERTISMENT_TIME_S), K_NO_WAIT);

		attr_set_uint32(ATTR_ID_boot_phy, BOOT_PHY_DEFAULT);
	} else if (bto.activeModeStatus == false) {
		/* If not in Active Mode, advertise for
		 * BOOTUP_ADVERTISMENT_TIME_MS in 1M
		 */
		Advertisement_ExtendedSet(false);
		Advertisement_Start();
		k_timer_start(&bootAdvertTimer,
			      K_SECONDS(BOOTUP_ADVERTISMENT_TIME_S), K_NO_WAIT);
	} else {
		/* Otherwise start advertising in configured broadcast PHY */
		Advertisement_Start();
	}

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

/******************************************************************************/
/* Framework Message Functions                                                */
/******************************************************************************/
static DispatchResult_t StartAdvertisingMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);
	UNUSED_PARAMETER(pMsgRxer);

	/* If in Active mode make sure we enable the broadcast PHY */
	if (bto.activeModeStatus) {
		Advertisement_ExtendedSet(bto.codedPHYBroadcast);
		/* Restart the duration timer */
		if (bto.durationTimeMs > 0) {
			k_timer_start(&durationTimer,
				      K_MSEC(bto.durationTimeMs), K_NO_WAIT);
		}
	} else {
		/* If not in active mode, we can restart the Shelf mode
		 * start-up timer, we'll already be in 1M PHY mode.
		 */
		k_timer_start(&bootAdvertTimer,
			      K_SECONDS(BOOTUP_ADVERTISMENT_TIME_S), K_NO_WAIT);
	}
	Advertisement_IntervalUpdate();
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
	attr_changed_msg_t *pb = (attr_changed_msg_t *)pMsg;
	size_t i;
	uint8_t updateData = false;

	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		case ATTR_ID_sensor_name:
			UpdateName();
			updateData = true;
			break;
		case ATTR_ID_advertising_interval:
			Advertisement_IntervalUpdate();
			break;
		case ATTR_ID_tx_power:
			TransmitPower();
			break;
		case ATTR_ID_mobile_app_disconnect:
			StartDisconnectTimer();
			break;
		case ATTR_ID_network_id:
		case ATTR_ID_config_version:
		case ATTR_ID_bluetooth_flags:
			updateData = true;
			break;
		case ATTR_ID_advertising_phy:
			attr_get(ATTR_ID_advertising_phy,
				      &bto.codedPHYBroadcast,
				      sizeof(bto.codedPHYBroadcast));
			/* Don't switch PHYs if in Shelf mode */
			if (bto.activeModeStatus) {
				Advertisement_ExtendedSet(
					bto.codedPHYBroadcast);
			}
			break;
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
	SensorMsg_t sensor_event;
	bool restart_duration_timer = false;

	if (bto.activeModeStatus) {
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

static DispatchResult_t SeverConnectionHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	RequestDisconnect(bto.conn);

	return DISPATCH_OK;
}

static DispatchResult_t BleSensorEventMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	SensorMsg_t local_event;
	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	/* Only store events when in active mode and not in a connection */
	if ((bto.activeModeStatus) && (bto.conn == NULL)) {
		/* If there's space, add this event to our local advert queue */
		if (k_msgq_num_free_get(&ble_task_advert_queue)) {
			/* Store event details */
			local_event.event.timestamp = pEventMsg->timeStamp;
			local_event.event.type = pEventMsg->eventType;
			local_event.event.data = pEventMsg->eventData;
			local_event.id = pEventMsg->id;
			/* Set event details in the advert queue */
			k_msgq_put(&ble_task_advert_queue, &local_event,
				   K_NO_WAIT);
			/* And update the advertisement */
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK,
						      FWK_ID_BLE_TASK,
						      FMC_SENSOR_UPDATE);
			LOG_DBG("Added Event to advert queue!");
		} else {
			LOG_DBG("Advert queue is full!");
		}
	}
	return DISPATCH_OK;
}

static DispatchResult_t BleEnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg)
{
	/* Always update the active mode flag. This can only be set once via
	 * attribute operations, but repeatedly by local user interfaces.
	 */
	bto.activeModeStatus = true;
	/* Is a connection active? If so, advertisement changes are also
	 * handled in the disconnect callback.
	 */
	if (bto.conn == NULL) {
		/* No, so we can go ahead and start advertising in
		 * 1M. First make sure the boot up timer isn't running
		 */
		k_timer_stop(&bootAdvertTimer);
		/* Stop advertising */
		Advertisement_End();
		/* Switch to 1M advertising */
		Advertisement_ExtendedSet(false);
		/* Change interval to faster rate in order to connect */
		Advertisement_IntervalDefault();
		/* Restart advertising */
		Advertisement_Start();
		/* Advertise for at most
		 * BLE_TASK_EXIT_SHELF_MODE_1M_ADVERTISE_TIME_S
		 * before switching back to the broadcast PHY.
		 */
		k_timer_start(&enterActiveModeTimer,
			      K_SECONDS(BOOTUP_ADVERTISMENT_TIME_S), K_NO_WAIT);
	}
	return DISPATCH_OK;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ConnectedCallback(struct bt_conn *conn, uint8_t r)
{
	char addr[BT_ADDR_LE_STR_LEN];
	struct bt_conn_info conn_info;
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	if (r) {
		LOG_ERR("Failed to connect to central %s (%u)",
			log_strdup(addr), r);
		bt_conn_unref(conn);
		bto.conn = NULL;
	} else {
		LOG_INF("Connected: %s", log_strdup(addr));
		bto.conn = bt_conn_ref(conn);

		/* Fetch PHY so we know what to advertise in if a firmware
		 * update takes places to re-allow connectivity
		 */
		bto.conn_from_le_coded = false;
		r = bt_conn_get_info(conn, &conn_info);
		if (!r && conn_info.le.phy->tx_phy == BT_GAP_LE_PHY_CODED) {
			bto.conn_from_le_coded = true;
		}

		r = bt_conn_set_security(bto.conn, BT_SECURITY_L2);
		LOG_DBG("Setting security status: %d", r);

		/* Set the power for the connection */
		TransmitPower();

		ResetAppDisconnectParam();

		/* Stop boot and active mode timers. We don't want
		 * any PHY changes to occur mid-connection.
		 */
		k_timer_stop(&bootAdvertTimer);
		k_timer_stop(&enterActiveModeTimer);

		/* Pause the duration timer if it is running */
		bto.durationTimeMs = k_timer_remaining_get(&durationTimer);
		k_timer_stop(&durationTimer);
	}
}

static void DisconnectedCallback(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bool lock_enabled;

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected: %s reason: %s", log_strdup(addr),
		lbt_get_hci_err_string(reason));

	/* Purge the advertising event queue so out of
	 * date events are not broadcast.
	 */
	k_msgq_purge(&ble_task_advert_queue);

	bt_conn_unref(bto.conn);
	bto.conn = NULL;

	/* Disconnect detected stop force disconnect */
	k_timer_stop(&mobileAppDisconnectTimer);

	/* Start the advertisement again */
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_BLE_START_ADVERTISING);

	/* Check if settings lock needs to be re-activated */
	attr_get(ATTR_ID_lock, &lock_enabled, sizeof(lock_enabled));

	if (lock_enabled == true) {
		attr_set_uint32(ATTR_ID_lock_status,
				    LOCK_STATUS_SETUP_ENGAGED);
	}
}

/* Update name in Bluetooth stack.  Zephyr will handle updating name in
 * advertisement.
 */
static int UpdateName(void)
{
	int r = -EPERM;
	char name[ATTR_MAX_STR_SIZE] = { 0 };

	attr_get(ATTR_ID_sensor_name, name, ATTR_MAX_STR_LENGTH);
	r = bt_set_name(name);
	if (r < 0) {
		LOG_ERR("bt_set_name: %s %d", log_strdup(name), r);
	}
	return r;
}

static void TransmitPower(void)
{
	int8_t powerLevel = 0;
	uint16_t connectionHandle = 0;
	int r = 0;

	attr_get(ATTR_ID_tx_power, &powerLevel, sizeof(powerLevel));

	if (bto.conn != NULL) {
		/* TX power level when connected */
		r = bt_hci_get_conn_handle(bto.conn, &connectionHandle);
		if (r >= 0) {
			set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_CONN,
				     connectionHandle, powerLevel);
		}
	}

	if (bto.conn == NULL || r < 0) {
		/* TX power level when advertising */
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
		LOG_ERR("Unable to allocate command buffer\n");
		return;
	}

	cp = net_buf_add(buf, sizeof(*cp));
	cp->handle = sys_cpu_to_le16(handle);
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
		LOG_ERR("Set Tx power err: %d reason 0x%02x\n", err, reason);
		return;
	}
	rp = (void *)rsp->data;
	LOG_INF("Actual Tx Power: %d\n", rp->selected_tx_power);

	net_buf_unref(rsp);
}

static void StartDisconnectTimer(void)
{
	bool disconnectFlag = false;
	attr_get(ATTR_ID_mobile_app_disconnect, &disconnectFlag,
		      sizeof(disconnectFlag));

	if (disconnectFlag == true) {
		k_timer_start(&mobileAppDisconnectTimer,
			      K_SECONDS(BLE_TASK_FORCE_DISCONNECT_DELAY_S),
			      K_NO_WAIT);
	}
}

static void ResetAppDisconnectParam(void)
{
	bool disconnectFlag = 0;

	attr_set_uint32(ATTR_ID_mobile_app_disconnect, disconnectFlag);
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

	if (attr_get(ATTR_ID_advertising_duration, &advertising_duration,
			  sizeof(advertising_duration)) ==
	    sizeof(advertising_duration)) {
		if (attr_get(ATTR_ID_advertising_interval,
				  &advertising_interval,
				  sizeof(advertising_interval)) ==
		    sizeof(advertising_interval)) {
			get_failed = false;
			/* If the Duration is less than BLE_TASK_ADV_DUR_SCALE
			 * times the Interval, clamp it to that value.
			 */
			if (advertising_duration <
			    (advertising_interval * BLE_TASK_ADV_DUR_SCALE)) {
				advertising_duration = advertising_interval *
						       BLE_TASK_ADV_DUR_SCALE;
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

	/* If active mode hasn't been turned on at this point turn off the
	 * adverisments
	 */
	if (bto.activeModeStatus == false) {
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
					      FMC_BLE_END_ADVERTISING);
	}
}

static void EnterActiveModeTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);

	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_BLE_START_ADVERTISING);
}

static void upgrade_advert_phy_timer_callback_isr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);

	/* Stop advertising in the designated PHY and advertise in the pre-set
	 * PHY or do not restart advertising if active mode is disabled
	 */
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
				      FMC_BLE_END_ADVERTISING);

	if (bto.activeModeStatus == true) {
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_BLE_TASK, FWK_ID_BLE_TASK,
					      FMC_BLE_START_ADVERTISING);
	}
}

static void AppDisconnectCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_BLE_TASK, FMC_BLE_END_CONNECTION);
}

static void le_param_updated(struct bt_conn *conn, uint16_t interval,
			     uint16_t latency, uint16_t timeout)
{
	LOG_DBG("Got an LE Param Updated! Interval = %d Latency = %d Timeout = %d",
		interval, latency, timeout);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	LOG_DBG("Got an LE Param Update Request!");

	return (true);
}
