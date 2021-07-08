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

/* This is the total number of events available */
#define TOTAL_NUMBER_EVENTS                                                    \
	(CONFIG_LCZ_EVENT_MANAGER_NUMBER_OF_FILES *                            \
	 CONFIG_LCZ_EVENT_MANAGER_EVENTS_PER_FILE)

typedef struct EventTaskTag {
	FwkMsgTask_t msgTask;
	uint32_t timeStampBuffer[TOTAL_NUMBER_EVENTS];
	uint32_t advertisingEvent;
	uint32_t eventID;
	uint32_t eventBufferRollover;
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
static uint32_t GetEventTimestamp(uint32_t id);
static void SetDataloggerStatus(void);
static void SaveTimeStamp(uint32_t timeStamp);
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

	memset(eventTaskObject.timeStampBuffer, 0, TOTAL_NUMBER_EVENTS);
	eventTaskObject.eventID = 0;
	eventTaskObject.advertisingEvent = 0;
	eventTaskObject.eventBufferRollover = 0;
}

void EventTask_GetCurrentEvent(uint32_t *id, SensorEvent_t *event)
{
	uint16_t eventCount = 0;
	static uint16_t currentEventCount = 0;
	static uint16_t eventIndex = 0;
	SensorEvent_t *readEvent;

	*id = GetAdvertEventId();
	event->timestamp = GetEventTimestamp(*id);
	readEvent = lcz_event_manager_get_next_event(event->timestamp,
						     &eventCount, eventIndex);
	LOG_INF("Current Event Id = %d", *id);
	LOG_INF("Current Event Type = %d", readEvent->type);
	memcpy(event, readEvent, sizeof(SensorEvent_t));
	if (eventCount == 1) {
		EventTask_IncrementEventId();
	} else if ((eventCount != 1) && (currentEventCount == 0)) {
		currentEventCount = eventCount - 1;
		eventIndex = eventIndex + 1;
	} else if (currentEventCount == 1) {
		EventTask_IncrementEventId();
		currentEventCount = 0;
		eventIndex = 0;
	} else {
		currentEventCount = currentEventCount - 1;
		eventIndex = eventIndex + 1;
	}
}

uint32_t EventTask_RemainingEvents(void)
{
	uint32_t numberEvents;
	if (eventTaskObject.eventBufferRollover != 0) {
		numberEvents =
			TOTAL_NUMBER_EVENTS - eventTaskObject.advertisingEvent;
	} else if (eventTaskObject.advertisingEvent < eventTaskObject.eventID) {
		numberEvents = eventTaskObject.eventID -
			       eventTaskObject.advertisingEvent;
	} else {
		numberEvents = 0;
	}

	return (numberEvents);
}

void EventTask_IncrementEventId(void)
{
	if (eventTaskObject.advertisingEvent < (TOTAL_NUMBER_EVENTS - 1)) {
		eventTaskObject.advertisingEvent =
			eventTaskObject.advertisingEvent + 1;
	} else {
		eventTaskObject.advertisingEvent = 0;
		/*clear the rollover flag*/
		eventTaskObject.eventBufferRollover = 0;
		LOG_INF("#####Roll Count Reset");
	}
	LOG_INF("AdvertEvent Count = %d", eventTaskObject.advertisingEvent);
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
static uint32_t GetEventTimestamp(uint32_t id)
{
	uint32_t timestamp = 0;
	if (id <= TOTAL_NUMBER_EVENTS) {
		timestamp = eventTaskObject.timeStampBuffer[id];
	}
	return (timestamp);
}

static void SetDataloggerStatus(void)
{
	bool dataLogEnable;
	Attribute_Get(ATTR_INDEX_dataloggingEnable, &dataLogEnable,
		      sizeof(dataLogEnable));

	lcz_event_manager_set_logging_state(dataLogEnable);
}
static void SaveTimeStamp(uint32_t timeStamp)
{
	eventTaskObject.timeStampBuffer[eventTaskObject.eventID] = timeStamp;
	if (eventTaskObject.eventID < (TOTAL_NUMBER_EVENTS - 1)) {
		eventTaskObject.eventID = eventTaskObject.eventID + 1;
	} else {
		eventTaskObject.eventID = 0;
		eventTaskObject.eventBufferRollover =
			eventTaskObject.eventBufferRollover + 1;
		LOG_INF("****Roll Count = %d",
			eventTaskObject.eventBufferRollover);
	}
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_EVENT_TASK, FWK_ID_BLE_TASK,
				      FMC_SENSOR_EVENT);
}

static DispatchResult_t EventTriggerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	uint32_t timeStamp = 0;
	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	timeStamp = lcz_event_manager_add_sensor_event(pEventMsg->eventType,
						       &pEventMsg->eventData);
	LOG_INF("Event Time1 = %d", timeStamp);

	if (eventTaskObject.eventID == 0) {
		/*Increment the buffer counter*/
		SaveTimeStamp(timeStamp);
	} else if ((eventTaskObject.eventID != 0) &&
		   (eventTaskObject.timeStampBuffer[(eventTaskObject.eventID -
						     1)] != timeStamp)) {
		SaveTimeStamp(timeStamp);
	} else {
		/*Timestamp the same don't increment the buffer count*/
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
