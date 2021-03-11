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
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef EVENT_TASK_PRIORITY
#define EVENT_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef EVENT_TASK_STACK_DEPTH
#define EVENT_TASK_STACK_DEPTH 2048
#endif

#ifndef EVENT_TASK_QUEUE_DEPTH
#define EVENT_TASK_QUEUE_DEPTH 8
#endif

/* This is the total number of events available */
#define TOTAL_NUMBER_EVENTS                                                    \
	(CONFIG_LCZ_EVENT_MANAGER_NUMBER_OF_FILES *                            \
	 CONFIG_LCZ_EVENT_MANAGER_EVENTS_PER_FILE)

typedef struct EventTaskTag {
	FwkMsgTask_t msgTask;
	uint32_t timeStampBuffer[TOTAL_NUMBER_EVENTS];
	uint32_t advertisingEvent;
	uint32_t eventID;
} EventTaskObj_t;
/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static EventTaskObj_t eventTaskObject;

K_THREAD_STACK_DEFINE(eventTaskStack, EVENT_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(eventTaskQueue, FWK_QUEUE_ENTRY_SIZE, EVENT_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void EventTaskThread(void *, void *, void *);
static uint32_t GetAdvertEventId(void);
static void SetAdvertEventId(uint32_t id);
static uint32_t GetEventTimestamp(uint32_t id);
static DispatchResult_t EventTriggerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
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

	memset(eventTaskObject.timeStampBuffer, 0, TOTAL_NUMBER_EVENTS);
	eventTaskObject.eventID = 0;
	eventTaskObject.advertisingEvent = 0;
}

void GetCurrentEvent(uint32_t *id, SensorEvent_t *event)
{
	uint16_t eventCount = 0;
	uint16_t eventIndex = 0;
	SensorEvent_t *readEvent;
	uint32_t advertId = eventTaskObject.advertisingEvent;

	//*id = GetAdvertEventId();	
	memcpy(id, &advertId, sizeof(uint32_t));
	//event->timestamp = GetEventTimestamp(*id);
	event->timestamp = eventTaskObject.timeStampBuffer[eventTaskObject.advertisingEvent];
	readEvent = lcz_event_manager_get_next_event(event->timestamp, &eventCount, eventIndex);

	memcpy(event, readEvent, sizeof(SensorEvent_t));
	/*if(currentEvent.event == NULL)
	{
		LOG_WRN("Failed to set advertising event\n");
	}*/
}

int EventLog_Prepare(void)
{
	// determine how many logs there are
	// increment active log index
	return -1;
}

int EventLog_Ack(void)
{
	// delete log
	return -1;
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

static uint32_t GetAdvertEventId(void)
{
	return eventTaskObject.advertisingEvent;
}
static void SetAdvertEventId(uint32_t id)
{
	eventTaskObject.advertisingEvent = id;
}
static uint32_t GetEventTimestamp(uint32_t id)
{
	uint32_t timestamp = 0;
	if (id <= TOTAL_NUMBER_EVENTS) {
		timestamp = eventTaskObject.timeStampBuffer[id];
	}
	return (timestamp);
}

static DispatchResult_t EventTriggerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);

	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	eventTaskObject.timeStampBuffer[eventTaskObject.eventID] =
		lcz_event_manager_add_sensor_event(pEventMsg->eventType,
						   &pEventMsg->eventData);

	eventTaskObject.eventID = eventTaskObject.eventID + 1;

	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_EVENT_TASK, FWK_ID_BLE_TASK,
				      FMC_SENSOR_EVENT);

	return DISPATCH_OK;
}
