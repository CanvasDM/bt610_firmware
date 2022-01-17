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
#include <power/reboot.h>
#include <hal/nrf_power.h>
#include <logging/log_ctrl.h>
#include <mgmt/mgmt.h>
#include <img_mgmt/image.h>
#include <img_mgmt/img_mgmt.h>
#include <lcz_os_mgmt/os_mgmt_impl.h>
#include <pm_config.h>
#include <lcz_nrf_reset_reason.h>

#include "FrameworkIncludes.h"
#include "BleTask.h"
#include "BspSupport.h"
#include "UserInterfaceTask.h"
#include "SensorTask.h"
#include "Version.h"
#include "Sentrius_mgmt.h"
#include "mcumgr_wrapper.h"
#include "lcz_no_init_ram_var.h"
#include "NonInit.h"
#include "file_system_utilities.h"
#include "lcz_bluetooth.h"
#include "attr.h"
#include "lcz_qrtc.h"
#include "Flags.h"
#include "EventTask.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "ControlTask.h"

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
} ControlTaskObj_t;

#if defined(CONFIG_SETTINGS_FS_FILE) &&                                        \
	defined(CONFIG_MAX_SETTINGS_FILE_SIZE) &&                              \
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

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg);

static DispatchResult_t HeartbeatMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					    FwkMsg_t *pMsg);

static DispatchResult_t AttrBroadcastMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg);

static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg);

static void RebootHandler(void);

static void mcumgr_mgmt_callback(uint8_t opcode, uint16_t group, uint8_t id,
				 void *arg);

static int img_mgmt_vercmp(const struct image_version *a,
			   const struct image_version *b);

static int upload_start_check(uint32_t offset, uint32_t size,
			      const struct image_header *header, void *arg);

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
				K_THREAD_STACK_SIZEOF(controlTaskStack),
				ControlTaskThread, &cto, NULL, NULL,
				CONTROL_TASK_PRIORITY, 0, K_NO_WAIT);
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

int attr_prepare_up_time(void)
{
	int64_t uptimeMs = k_uptime_get();
	return (attr_set_signed64(ATTR_ID_up_time, uptimeMs));
}

int attr_prepare_log_file_status(void)
{
	uint32_t logFileStatus = lcz_event_manager_get_log_file_status();
	return (attr_set_uint32(ATTR_ID_log_file_status, logFileStatus));
}

void app_prepare_for_reboot(void)
{
	/* Save attributes */
	(void)attr_force_save();
	non_init_save_data();
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
	bool dataLogEnable;
	bool lock_enabled;

	/* Prevent 'lost' logs */
	k_sleep(K_SECONDS(1));

	LOG_WRN("Version %s", VERSION_STRING);

	/* Check if settings lock is enabled and set it up */
	attr_get(ATTR_ID_lock, &lock_enabled, sizeof(lock_enabled));

	attr_set_uint32(ATTR_ID_lock_status,
			    (lock_enabled == true ? LOCK_STATUS_SETUP_ENGAGED :
						    LOCK_STATUS_NOT_SETUP));

	/* Safe to read the data log enable flag after reading back all
	 * attributes. We use this to determine whether to disable or enable
	 * logging at startup.
	 */
	attr_get(ATTR_ID_data_logging_enable, &dataLogEnable,
		 sizeof(dataLogEnable));

	RebootHandler();

	mcumgr_wrapper_register_subsystems();

	Flags_Init();

	/* Start the Event Manager as early as possible before any events get
	 * posted to it by threads trumping this one.
	 */
	lcz_event_manager_initialise(dataLogEnable);

	UserInterfaceTask_Initialize();

	BleTask_Initialize();

	SensorTask_Initialize();

	EventTask_Initialize();

#ifdef CONFIG_MCUMGR_CMD_SENTRIUS_MGMT
	Sentrius_mgmt_register_group();
#endif

	/* Register callbacks for mcumgr management events */
	mgmt_register_evt_cb(mcumgr_mgmt_callback);
	img_mgmt_set_upload_cb(upload_start_check, 0);

	Framework_StartTimer(&pObj->msgTask);

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void RebootHandler(void)
{
	attr_set_string(ATTR_ID_firmware_version, VERSION_STRING,
			strlen(VERSION_STRING));

	uint32_t reset_reason = lcz_nrf_reset_reason_get_and_clear_register();
	const char *s =
		lcz_nrf_reset_reason_get_string(reset_reason);
	LOG_WRN("reset reason: %s (%08X)", s, reset_reason);

	if (reset_reason ==
	    (POWER_RESETREAS_DOG_Detected << POWER_RESETREAS_DOG_Pos)) {
		LOG_ERR("*WARNING* Unit reboot was forced by watchdog timeout");
	}

	uint32_t reset_count = 0;
	uint8_t recovery_count = 0;
	if (fsu_lfs_mount() == 0) {
		fsu_read_abs(RESET_COUNT_FNAME, &reset_count,
			     sizeof(reset_count));
		reset_count += 1;
		fsu_write_abs(RESET_COUNT_FNAME, &reset_count,
			      sizeof(reset_count));
	}

#if defined(CONFIG_SETTINGS_FS_FILE) &&                                        \
	defined(CONFIG_MAX_SETTINGS_FILE_SIZE) &&                              \
	CONFIG_MAX_SETTINGS_FILE_SIZE > 0
	/* Check the size of the settings file */
	if (fsu_get_file_size_abs(CONFIG_SETTINGS_FS_FILE) >=
	    CONFIG_MAX_SETTINGS_FILE_SIZE) {
		/* Settings file is too large, clear it so that there is
		 * sufficient space to save settings
		 */
		(void)fsu_delete_abs(CONFIG_SETTINGS_FS_FILE);
	}
#endif

	fsu_read_abs(CONFIG_RECOVERY_FILE_PATH, &recovery_count,
		     sizeof(recovery_count));

	bool valid = lcz_no_init_ram_var_is_valid(pnird, SIZE_OF_NIRD);
	if (valid) {
		LOG_INF("Battery age: %u", pnird->battery_age);
		LOG_INF("Qrtc Epoch: %u", pnird->qrtc);
		LOG_INF("Bootloader time: %u", pnird->bootloader_time);
		LOG_INF("Execution time: %u", k_uptime_get_32());
		if (pnird->qrtc > 0) {
			if (pnird->bootloader_time >=
			    BOOTLOADER_MAX_TIME_SANITY_CHECK) {
				LOG_ERR("Bootloader time is in excess of 10 minutes, "
					"ignoring value");
				pnird->bootloader_time = 0;
			}

			pnird->qrtc +=
				((pnird->bootloader_time + k_uptime_get_32()) /
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
	attr_set_uint32(ATTR_ID_recover_settings_count, recovery_count);
}

static DispatchResult_t HeartbeatMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					    FwkMsg_t *pMsg)
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

static DispatchResult_t AttrBroadcastMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg)
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

static DispatchResult_t FactoryResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	LOG_WRN("Factory Reset");
	cto.factoryResetFlag = true;
	attr_factory_reset();
	lcz_event_manager_factory_reset();
	/* Need reset to init all the values */
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_CONTROL_TASK, FWK_ID_CONTROL_TASK,
				      FMC_SOFTWARE_RESET);
	return DISPATCH_OK;
}

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg)
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

static void mcumgr_mgmt_callback(uint8_t opcode, uint16_t group, uint8_t id,
				 void *arg)
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
	attr_set_uint32(ATTR_ID_boot_phy, (ble_conn_last_was_le_coded() ?
							ADVERTISING_PHY_CODED :
							ADVERTISING_PHY_1M));

	app_prepare_for_reboot();
}

/**
 * Compares two image version numbers in a semver-compatible way.
 *
 * @param a                     The first version to compare.
 * @param b                     The second version to compare.
 *
 * @return                      -1 if a < b
 * @return                       0 if a = b
 * @return                       1 if a > b
 *
 * Taken from mcumgr file cmd/img_mgmt/port/zephyr/src/zephyr_img_mgmt.c
 */
static int img_mgmt_vercmp(const struct image_version *a,
			   const struct image_version *b)
{
	if (a->iv_major < b->iv_major) {
		return -1;
	} else if (a->iv_major > b->iv_major) {
		return 1;
	}

	if (a->iv_minor < b->iv_minor) {
		return -1;
	} else if (a->iv_minor > b->iv_minor) {
		return 1;
	}

	if (a->iv_revision < b->iv_revision) {
		return -1;
	} else if (a->iv_revision > b->iv_revision) {
		return 1;
	}

	/* Note: For semver compatibility, don't compare the 32-bit build num. */

	return 0;
}

static int upload_start_check(uint32_t offset, uint32_t size,
			      const struct image_header *header, void *arg)
{
	int rc;
	struct image_version current_ver;
#if defined(CONFIG_MINIMUM_FIRMWARE_VERSION_FOTA_CHECK)
	struct image_version min_ver;
#endif
	bool downgrade_blocked = false;

	/* Only check the first chunk */
	if (offset == 0) {
		/* Check if configuration is locked */
		if (attr_is_locked() == true) {
			/* Configuration is locked, deny firmware update request
			 * until the device has been unlocked to prevent
			 * unauthorised upgrading or downgrading of firmware.
			 * Use an EPERUSER error to distinguish this state for
			 * the mobile application
			 */
			LOG_ERR("Attempted firmware update whilst settings are "
				"locked");
			return MGMT_ERR_EPERUSER;
		}

#if defined(CONFIG_MINIMUM_FIRMWARE_VERSION_FOTA_CHECK)
		/* There is a minimum firmware version that can be loaded over
		 * FOTA, ensure that this version requirement is met
		 */
		min_ver.iv_major = CONFIG_MINIMUM_FIRMWARE_VERSION_MAJOR;
		min_ver.iv_minor = CONFIG_MINIMUM_FIRMWARE_VERSION_MINOR;
		min_ver.iv_revision = CONFIG_MINIMUM_FIRMWARE_VERSION_REVISION;
		if (img_mgmt_vercmp(&min_ver, &header->ih_ver) == 1) {
			/* Trying to downgrade to a pre-release firmware, block
			 * this action
			 */
			LOG_ERR("Attempted pre-release firmware downgrade "
				"blocked");
			return MGMT_ERR_EBADSTATE;
		}
#endif

		/* Check if this will fit into the secondary slot */
		if (header->ih_img_size > PM_MCUBOOT_SECONDARY_SIZE) {
			/* The FOTA file is too large for the slot, deny */
			LOG_ERR("Attempted too large firmware update "
				"blocked (0x%x bytes)",
				header->ih_img_size);
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

		if (img_mgmt_vercmp(&current_ver, &header->ih_ver) == 1) {
			/* Current version is greater than new version, block
			 * downgrade
			 */
			LOG_ERR("Attempted firmware downgrade blocked");
			return MGMT_ERR_EBADSTATE;
		}
	}

	return MGMT_ERR_EOK;
}
