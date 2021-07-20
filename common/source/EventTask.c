/**
 * @file EventTask.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(EventTask, LOG_LEVEL_DBG);
#define THIS_FILE "Event"
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>

#include "FrameworkIncludes.h"
#include "EventTask.h"
#include "Flags.h"
#include "Attribute.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef EVENT_TASK_PRIORITY
#define EVENT_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef EVENT_TASK_STACK_DEPTH
#define EVENT_TASK_STACK_DEPTH 4096
#endif

#ifndef EVENT_TASK_QUEUE_DEPTH
#define EVENT_TASK_QUEUE_DEPTH 32
#endif

typedef struct EventTaskTag {
	FwkMsgTask_t msgTask;
} EventTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static EventTaskObj_t eventTaskObject;

K_THREAD_STACK_DEFINE(eventTaskStack, EVENT_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(eventTaskQueue, FWK_QUEUE_ENTRY_SIZE, EVENT_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

/* Rolling id used to identify new events */
static uint32_t event_task_event_id = 0;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void EventTaskThread(void *, void *, void *);
static void SetDataloggerStatus(void);
static DispatchResult_t EventTriggerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg);
static DispatchResult_t EventAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg);
/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t EventTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:             return Framework_UnknownMsgHandler;
	case FMC_EVENT_TRIGGER:       return EventTriggerMsgHandler;
	case FMC_ATTR_CHANGED:        return EventAttrChangedMsgHandler;
	default:                      return NULL;
	}
	/* clang-format on */
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void EventTask_Initialize(void)
{
	eventTaskObject.msgTask.rxer.id = FWK_ID_EVENT_TASK;
	eventTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	eventTaskObject.msgTask.rxer.pMsgDispatcher = EventTaskMsgDispatcher;
	eventTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	eventTaskObject.msgTask.timerPeriodTicks = K_MSEC(0); // 0 for one shot
	eventTaskObject.msgTask.rxer.pQueue = &eventTaskQueue;

	Framework_RegisterTask(&eventTaskObject.msgTask);

	eventTaskObject.msgTask.pTid =
		k_thread_create(&eventTaskObject.msgTask.threadData,
				eventTaskStack,
				K_THREAD_STACK_SIZEOF(eventTaskStack),
				EventTaskThread, &eventTaskObject, NULL, NULL,
				EVENT_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(eventTaskObject.msgTask.pTid, THIS_FILE);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void EventTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	EventTaskObj_t *pObj = (EventTaskObj_t *)pArg1;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void SetDataloggerStatus(void)
{
	bool dataLogEnable;
	Attribute_Get(ATTR_INDEX_dataloggingEnable, &dataLogEnable,
		      sizeof(dataLogEnable));

	lcz_event_manager_set_logging_state(dataLogEnable);
}

static DispatchResult_t EventTriggerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	uint32_t local_timestamp;

	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	/* We always add events to the Event Manager for long term storage */
	local_timestamp = lcz_event_manager_add_sensor_event(
		pEventMsg->eventType, &pEventMsg->eventData);

	/* Now post the event to the BLE Task */
	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_SENSOR_EVENT;
		pMsgSend->header.txId = FWK_ID_EVENT_TASK;
		pMsgSend->header.rxId = FWK_ID_BLE_TASK;
		pMsgSend->eventType = pEventMsg->eventType;
		pMsgSend->eventData = pEventMsg->eventData;
		pMsgSend->id = event_task_event_id++;
		pMsgSend->timeStamp = local_timestamp;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
	return DISPATCH_OK;
}
static DispatchResult_t EventAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsgRxer);
	AttrChangedMsg_t *pb = (AttrChangedMsg_t *)pMsg;
	size_t i;
	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		case ATTR_INDEX_dataloggingEnable:
			SetDataloggerStatus();
			break;
		default:
			/* Don't care about this attribute. This is a broadcast. */
			break;
		}
	}
	return DISPATCH_OK;
}