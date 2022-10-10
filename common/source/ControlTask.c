/**
 * @file ControlTask.c
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(ControlTask, CONFIG_CONTROL_TASK_LOG_LEVEL);
#define THIS_FILE "ControlTask"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <sys/reboot.h>
#include <hal/nrf_power.h>
#include <logging/log_ctrl.h>
#include <mgmt/mgmt.h>
#include <img_mgmt/image.h>
#include <img_mgmt/img_mgmt.h>
#include <pm_config.h>
#include <lcz_nrf_reset_reason.h>

#include "FrameworkIncludes.h"
#include "BleTask.h"
#include "BspSupport.h"
#include "UserInterfaceTask.h"
#include "SensorTask.h"
#include "app_version.h"
#include "lcz_no_init_ram_var.h"
#include "NonInit.h"
#include "file_system_utilities.h"
#include "lcz_bluetooth.h"
#include "attr.h"
#include "lcz_qrtc.h"
#include "EventTask.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "ControlTask.h"

#ifdef CONFIG_FS_MGMT_FILE_ACCESS_HOOK
#include "FileAccess.h"
#endif

/******************************************************************************/
// Local Constant, Macro and Type Definitions
/******************************************************************************/
#if !CONTROL_TASK_USES_MAIN_THREAD
#ifndef CONTROL_TASK_PRIORITY
#define CONTROL_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef CONTROL_TASK_STACK_DEPTH
#define CONTROL_TASK_STACK_DEPTH 4096
#endif
#endif

#ifndef CONTROL_TASK_QUEUE_DEPTH
#define CONTROL_TASK_QUEUE_DEPTH 32
#endif

#define RESET_COUNT_FNAME CONFIG_FSU_MOUNT_POINT "/reset_count"

#define MS_TO_SECONDS_DIVISOR 1000

/* 10 minutes in ms, if the bootloader time is greater than this number, it's
 * value is ignored. This value is higher than the bootloader should ever need
 * to spend
 */
#define BOOTLOADER_MAX_TIME_SANITY_CHECK 600000

typedef struct ControlTaskTag {
	FwkMsgTask_t msgTask;
	uint32_t broadcastCount;
	bool factoryResetFlag;
        /* Flag used to determine when the thread has finished initialisation */
        bool task_started;
} ControlTaskObj_t;

#if defined(CONFIG_SETTINGS_FS_FILE) && defined(CONFIG_MAX_SETTINGS_FILE_SIZE) &&                  \
	CONFIG_MAX_SETTINGS_FILE_SIZE != 0
BUILD_ASSERT(CONFIG_MAX_SETTINGS_FILE_SIZE >= 256,
	     "Settings file maximum size must be at least 256 bytes");
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static ControlTaskObj_t cto;

#if !CONTROL_TASK_USES_MAIN_THREAD
K_THREAD_STACK_DEFINE(controlTaskStack, CONTROL_TASK_STACK_DEPTH);
#endif

K_MSGQ_DEFINE(controlTaskQueue, FWK_QUEUE_ENTRY_SIZE, CONTROL_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void ControlTaskThread(void *, void *, void *);

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

static DispatchResult_t HeartbeatMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

static DispatchResult_t AttrBroadcastMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

static void RebootHandler(void);

static void mcumgr_mgmt_callback(uint8_t opcode, uint16_t group, uint8_t id, void *arg);

static int upload_start_check(const struct img_mgmt_upload_req req,
			      const struct img_mgmt_upload_action action);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t *ControlTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:           return Framework_UnknownMsgHandler;
	case FMC_PERIODIC:          return HeartbeatMsgHandler;
	case FMC_SOFTWARE_RESET:    return SoftwareResetMsgHandler;
	case FMC_ATTR_CHANGED:      return AttrBroadcastMsgHandler;
	case FMC_FACTORY_RESET:     return FactoryResetMsgHandler;
	default:                    return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void ControlTask_Initialize(void)
{
	cto.msgTask.rxer.id = FWK_ID_CONTROL_TASK;
	cto.msgTask.rxer.rxBlockTicks = K_FOREVER;
	cto.msgTask.rxer.pMsgDispatcher = ControlTaskMsgDispatcher;
	cto.msgTask.timerDurationTicks = K_SECONDS(CONFIG_HEARTBEAT_SECONDS);
	cto.msgTask.timerPeriodTicks = K_MSEC(0);
	cto.msgTask.rxer.pQueue = &controlTaskQueue;

	Framework_RegisterTask(&cto.msgTask);
	cto.factoryResetFlag = false;

#if CONTROL_TASK_USES_MAIN_THREAD
	cto.msgTask.pTid = k_current_get();
#else
	cto.msgTask.pTid =
		k_thread_create(&cto.msgTask.threadData, controlTaskStack,
				K_THREAD_STACK_SIZEOF(controlTaskStack), ControlTaskThread, &cto,
				NULL, NULL, CONTROL_TASK_PRIORITY, 0, K_NO_WAIT);
#endif

	k_thread_name_set(cto.msgTask.pTid, THIS_FILE);
}

void ControlTask_Thread(void)
{
#if CONTROL_TASK_USES_MAIN_THREAD
	ControlTaskThread(&cto, NULL, NULL);
#endif
}

EXTERNED void Framework_AssertionHandler(char *file, int line)
{
	static bool busy = 0; /* Prevent recursion (buffer alloc fail, ...) */
	if (!busy) {
		busy = true;
		LOG_ERR("\r\n!-----> Assertion <-----! %s:%d\r\n", file, line);
		__NOP();
	}
}

int attr_prepare_uptime(void)
{
	int64_t uptimeMs = k_uptime_get();
	return (attr_set_signed64(ATTR_ID_uptime, uptimeMs));
}

void app_prepare_for_reboot(void)
{
	/* The BT6 has a custom sys_reboot_notification handler that
	 * will save attributes before rebooting. Due to the HW KEY
	 * module invoking system reboot, and the attributes requiring
	 * the HW KEY service to decrypt files, a flag is set upon
	 * completion of the Control Task's start up activities which
	 * then allows the save activity below to be performed. If the
	 * flag is not set, it is assumed the attributes are not yet
	 * ready, and the save operation is skipped.
	 */
	if (cto.task_started) {
		/* Save attributes only when safe to do so */
		(void)attr_force_save();
		non_init_save_data();
	}
}

void sys_reboot_notification(int type)
{
	ARG_UNUSED(type);
	app_prepare_for_reboot();
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ControlTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	ControlTaskObj_t *pObj = (ControlTaskObj_t *)pArg1;

	LOG_WRN("Version %s", APP_VERSION_STRING);

	RebootHandler();

	UserInterfaceTask_Initialize();

	BleTask_Initialize();

	SensorTask_Initialize();

	EventTask_Initialize();

	/* Register callbacks for mcumgr management events */
	mgmt_register_evt_cb(mcumgr_mgmt_callback);
	img_mgmt_set_upload_cb(upload_start_check);

#ifdef CONFIG_FS_MGMT_FILE_ACCESS_HOOK
	/* Setup file access control */
	file_access_setup();
#endif

	Framework_StartTimer(&pObj->msgTask);

	/* Flag thread initialisation complete to allow saving of attributes
	 * via sys_reboot_notification
	 */
	cto.task_started = true;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void RebootHandler(void)
{
	(void)attr_set_string(ATTR_ID_firmware_version, APP_VERSION_STRING, strlen(APP_VERSION_STRING));
	(void)attr_set_string(ATTR_ID_board, CONFIG_BOARD, strlen(CONFIG_BOARD));

	uint32_t reset_reason = lcz_nrf_reset_reason_get_and_clear_register();
	const char *s = lcz_nrf_reset_reason_get_string(reset_reason);
	LOG_WRN("reset reason: %s (%08X)", s, reset_reason);

	if (reset_reason == (POWER_RESETREAS_DOG_Detected << POWER_RESETREAS_DOG_Pos)) {
		LOG_ERR("*WARNING* Unit reboot was forced by watchdog timeout");
	}

	uint32_t reset_count = 0;

	fsu_read_abs(RESET_COUNT_FNAME, &reset_count, sizeof(reset_count));
	reset_count += 1;
	fsu_write_abs(RESET_COUNT_FNAME, &reset_count, sizeof(reset_count));

#if defined(CONFIG_SETTINGS_FS_FILE) && defined(CONFIG_MAX_SETTINGS_FILE_SIZE) &&                  \
	CONFIG_MAX_SETTINGS_FILE_SIZE > 0
	/* Check the size of the settings file */
	if (fsu_get_file_size_abs(CONFIG_SETTINGS_FS_FILE) >= CONFIG_MAX_SETTINGS_FILE_SIZE) {
		/* Settings file is too large, clear it so that there is
		 * sufficient space to save settings
		 */
		(void)fsu_delete_abs(CONFIG_SETTINGS_FS_FILE);
	}
#endif

	bool valid = lcz_no_init_ram_var_is_valid(pnird, SIZE_OF_NIRD);
	if (valid) {
		LOG_INF("Battery age: %u", pnird->battery_age);
		LOG_INF("Qrtc Epoch: %u", pnird->qrtc);
		LOG_INF("Bootloader time: %u", pnird->bootloader_time);
		LOG_INF("Execution time: %u", k_uptime_get_32());
		if (pnird->qrtc > 0) {
			if (pnird->bootloader_time >= BOOTLOADER_MAX_TIME_SANITY_CHECK) {
				LOG_ERR("Bootloader time is in excess of 10 minutes, "
					"ignoring value");
				pnird->bootloader_time = 0;
			}

			pnird->qrtc += ((pnird->bootloader_time + k_uptime_get_32()) /
					MS_TO_SECONDS_DIVISOR);
			lcz_qrtc_set_epoch(pnird->qrtc);

			LOG_INF("Device time: %u", pnird->qrtc);

			if (pnird->attribute_save_pending == true) {
				LOG_ERR("Device was rebooted with unsaved "
					"configuration changes!");
				pnird->attribute_save_pending = false;
			}

			attr_set_uint32(ATTR_ID_qrtc, pnird->qrtc);
		}
	} else {
		LOG_WRN("No init ram data is not valid");
		pnird->battery_age = 0;
		pnird->qrtc = 0;
	}

	/* Clear volatile config */
	pnird->bootloader_time = 0;
	pnird->attribute_save_pending = false;
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);

	/* Update attributes */
	attr_set_string(ATTR_ID_reset_reason, s, strlen(s));
	attr_set_uint32(ATTR_ID_reset_count, reset_count);
}

static DispatchResult_t HeartbeatMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ControlTaskObj_t *pObj = FWK_TASK_CONTAINER(ControlTaskObj_t);

	/* Any benefit of a writable battery age isn't worth the complexity. */
	pnird->battery_age += CONFIG_HEARTBEAT_SECONDS;
	attr_set_uint32(ATTR_ID_battery_age, pnird->battery_age);

	/* Read value from system because it should have less error than
	 * seconds maintained by this function.
	 * Except if factory reset is in progress
	 */
	if (cto.factoryResetFlag == false) {
		pnird->qrtc = lcz_qrtc_get_epoch();
		attr_set_uint32(ATTR_ID_qrtc, pnird->qrtc);
	}
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);

	Framework_StartTimer(&pObj->msgTask);
	return DISPATCH_OK;
}

static DispatchResult_t AttrBroadcastMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ControlTaskObj_t *pObj = FWK_TASK_CONTAINER(ControlTaskObj_t);
	attr_changed_msg_t *pb = (attr_changed_msg_t *)pMsg;
	size_t i;

	pObj->broadcastCount += 1;

	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		default:
			/* Don't care about this attribute. This is a
			 * broadcast.
			 */
			break;
		}
	}
	return DISPATCH_OK;
}

static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	LOG_WRN("Factory Reset");
	cto.factoryResetFlag = true;
	attr_factory_reset();
	/* Need reset to init all the values */
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_CONTROL_TASK, FWK_ID_CONTROL_TASK, FMC_SOFTWARE_RESET);
	return DISPATCH_OK;
}

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);

	app_prepare_for_reboot();

	if (cto.factoryResetFlag == true) {
		non_init_clear_qrtc();
	}

	LOG_PANIC();
	k_thread_priority_set(k_current_get(), -CONFIG_NUM_COOP_PRIORITIES);
	LOG_ERR("Software Reset in ~5 seconds");
	k_sleep(K_SECONDS(5));
	sys_reboot(SYS_REBOOT_WARM);
	return DISPATCH_OK;
}

static void mcumgr_mgmt_callback(uint8_t opcode, uint16_t group, uint8_t id, void *arg)
{
	/* We are only interested in the firmware upload complete event, skip
	 * all others
	 */
	if (opcode != MGMT_EVT_OP_CMD_STATUS || group != MGMT_GROUP_ID_IMAGE ||
	    id != IMG_MGMT_ID_UPLOAD ||
	    ((struct mgmt_evt_op_cmd_status_arg *)arg)->status !=
		    IMG_MGMT_ID_UPLOAD_STATUS_COMPLETE) {
		return;
	}

	/* Save the current PHY to the boot PHY attribute so it can be restored
	 * after rebooting
	 */
	attr_set_uint32(ATTR_ID_boot_phy, (ble_conn_last_was_le_coded() ? ADVERTISING_PHY_CODED :
									  ADVERTISING_PHY_1M));

	app_prepare_for_reboot();
}

static int upload_start_check(const struct img_mgmt_upload_req req,
			      const struct img_mgmt_upload_action action)
{
	int rc;
	struct image_version current_ver;
#if defined(CONFIG_MINIMUM_FIRMWARE_VERSION_FOTA_CHECK)
	struct image_version min_ver;
#endif
	bool downgrade_blocked = false;
	const struct image_header *hdr = (struct image_header *)req.img_data.value;

	/* Only check the first chunk */
	if (req.off == 0) {
#if defined(CONFIG_MINIMUM_FIRMWARE_VERSION_FOTA_CHECK)
		/* There is a minimum firmware version that can be loaded over
		 * FOTA, ensure that this version requirement is met
		 */
		min_ver.iv_major = CONFIG_MINIMUM_FIRMWARE_VERSION_MAJOR;
		min_ver.iv_minor = CONFIG_MINIMUM_FIRMWARE_VERSION_MINOR;
		min_ver.iv_revision = CONFIG_MINIMUM_FIRMWARE_VERSION_REVISION;
		if (img_mgmt_vercmp(&min_ver, &hdr->ih_ver) == 1) {
			/* Trying to downgrade to a pre-release firmware, block
			 * this action
			 */
			LOG_ERR("Attempted pre-release firmware downgrade "
				"blocked");
			return MGMT_ERR_EBADSTATE;
		}
#endif

		/* Check if this will fit into the secondary slot */
		if (hdr->ih_img_size > PM_MCUBOOT_SECONDARY_SIZE) {
			/* The FOTA file is too large for the slot, deny */
			LOG_ERR("Attempted too large firmware update "
				"blocked (0x%x bytes)",
				hdr->ih_img_size);
			return MGMT_ERR_EMSGSIZE;
		}

		/* Are we blocking downgrades? If not, allow downgrade */
		rc = attr_get(ATTR_ID_block_downgrades, &downgrade_blocked,
			      sizeof(downgrade_blocked));
		if (rc <= 0) {
			/* Failed to query, deny request  */
			LOG_ERR("Failed to query block downgrade status");
			return MGMT_ERR_EUNKNOWN;
		} else if (downgrade_blocked == false) {
			/* Queried successfully and downgrades are not
			 * blocked, continue
			 */
			return MGMT_ERR_EOK;
		}

		/* Downgrades are not allowed, get current firmware version to
		 * check
		 */
		rc = img_mgmt_my_version(&current_ver);
		if (rc != 0) {
			/* Failed to query current running firmware version, we
			 * cannot proceed
			 */
			LOG_ERR("Failed to query current firmware version");
			return MGMT_ERR_EUNKNOWN;
		}

		if (img_mgmt_vercmp(&current_ver, &hdr->ih_ver) == 1) {
			/* Current version is greater than new version, block
			 * downgrade
			 */
			LOG_ERR("Attempted firmware downgrade blocked");
			return MGMT_ERR_EBADSTATE;
		}
	}

	return MGMT_ERR_EOK;
}
