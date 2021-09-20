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
#include <img_mgmt/img_mgmt.h>
#include <lcz_os_mgmt/os_mgmt_impl.h>

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
#include "Attribute.h"
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

typedef struct ControlTaskTag {
	FwkMsgTask_t msgTask;
	uint32_t broadcastCount;
} ControlTaskObj_t;

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

int AttributePrepare_upTime(void)
{
	int64_t uptimeMs = k_uptime_get();
	return (Attribute_SetSigned64(ATTR_INDEX_upTime, uptimeMs));
}

int AttributePrepare_logFileStatus(void)
{
	uint32_t logFileStatus = lcz_event_manager_get_log_file_status();
	return (Attribute_SetUint32(ATTR_INDEX_logFileStatus, logFileStatus));
}

void app_prepare_for_reboot(void)
{
	/* Save attributes */
	(void)Attribute_Save_Now();
	non_init_save_data();
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ControlTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	ControlTaskObj_t *pObj = (ControlTaskObj_t *)pArg1;
	bool dataLogEnable;

	/* Prevent 'lost' logs */
	k_sleep(K_SECONDS(1));

	LOG_WRN("Version %s", VERSION_STRING);

	Attribute_Init();

	/* Safe to read the data log enable flag after reading back all
	 * attributes. We use this to determine whether to disable or enable
	 * logging at startup.
	 */
	Attribute_Get(ATTR_INDEX_dataloggingEnable, &dataLogEnable,
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

	/* Register a callback for mcumgr management events */
	mgmt_register_evt_cb(mcumgr_mgmt_callback);

	Framework_StartTimer(&pObj->msgTask);

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void RebootHandler(void)
{
	Attribute_SetString(ATTR_INDEX_firmwareVersion, VERSION_STRING,
			    strlen(VERSION_STRING));

	uint32_t reset_reason = lbt_get_and_clear_nrf52_reset_reason_register();
	const char *s =
		lbt_get_nrf52_reset_reason_string_from_register(reset_reason);
	LOG_WRN("reset reason: %s (%08X)", s, reset_reason);

	Attribute_SetString(ATTR_INDEX_resetReason, s, strlen(s));

	uint32_t count = 0;
	if (fsu_lfs_mount() == 0) {
		fsu_read_abs(RESET_COUNT_FNAME, &count, sizeof(count));
		count += 1;
		fsu_write_abs(RESET_COUNT_FNAME, &count, sizeof(count));
	}
	Attribute_SetUint32(ATTR_INDEX_resetCount, count);

	bool valid = lcz_no_init_ram_var_is_valid(pnird, SIZE_OF_NIRD);
	if (valid) {
		LOG_INF("Battery age: %u", pnird->battery_age);
		LOG_INF("Qrtc Epoch: %u", pnird->qrtc);

		lcz_qrtc_set_epoch(pnird->qrtc);

		if (pnird->attribute_save_pending == true) {
			LOG_ERR("Device was rebooted with unsaved "
				"configuration changes!");
			pnird->attribute_save_pending = false;
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
}

static DispatchResult_t HeartbeatMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					    FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ControlTaskObj_t *pObj = FWK_TASK_CONTAINER(ControlTaskObj_t);

	/* Any benefit of a writable battery age isn't worth the complexity. */
	pnird->battery_age += CONFIG_HEARTBEAT_SECONDS;
	Attribute_SetUint32(ATTR_INDEX_batteryAge, pnird->battery_age);

	/* Read value from system because it should have less error than
	 * seconds maintained by this function.
	 */
	pnird->qrtc = lcz_qrtc_get_epoch();
	Attribute_SetUint32(ATTR_INDEX_qrtc, pnird->qrtc);

	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);

	Framework_StartTimer(&pObj->msgTask);
	return DISPATCH_OK;
}

static DispatchResult_t AttrBroadcastMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg)
{
	ControlTaskObj_t *pObj = FWK_TASK_CONTAINER(ControlTaskObj_t);
	AttrChangedMsg_t *pb = (AttrChangedMsg_t *)pMsg;
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
	Attribute_FactoryReset();
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
	Attribute_SetUint32(ATTR_INDEX_bootPHY, (ble_conn_last_was_le_coded() ?
							       BOOT_PHY_TYPE_CODED :
							       BOOT_PHY_TYPE_1M));

	app_prepare_for_reboot();
}
