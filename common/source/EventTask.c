/**
 * @file EventTask.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(EventTask, CONFIG_EVENT_TASK_LOG_LEVEL);
#define THIS_FILE "Event"

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <device.h>

#include "FrameworkIncludes.h"
#include "EventTask.h"
#include "attr.h"
#include "Advertisement.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "attr_custom_validator_event_filter_flags.h"

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
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

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static EventTaskObj_t eventTaskObject;

K_THREAD_STACK_DEFINE(eventTaskStack, EVENT_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(eventTaskQueue, FWK_QUEUE_ENTRY_SIZE, EVENT_TASK_QUEUE_DEPTH, FWK_QUEUE_ALIGNMENT);

/* Rolling id used to identify new events */
static uint32_t event_task_event_id = 0;

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/

static void EventTaskThread(void *, void *, void *);
static void SetDataloggerStatus(void);
static DispatchResult_t EventLogTimeStampMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t EventNoTimeStampMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t EventAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static void SendEventDataAdvert(SensorMsg_t *sensor_event);
static bool eventFilter(SensorEventType_t eventType);

/**************************************************************************************************/
/* Framework Message Dispatcher                                                                   */
/**************************************************************************************************/
static FwkMsgHandler_t *EventTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:             return Framework_UnknownMsgHandler;
	case FMC_EVENT_TRIGGER:       return EventLogTimeStampMsgHandler;
	case FMC_AGGREGATION_EVENT:   return EventNoTimeStampMsgHandler;
	case FMC_ATTR_CHANGED:        return EventAttrChangedMsgHandler;
	default:                      return NULL;
	}
	/* clang-format on */
}

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
void EventTask_Initialize(void)
{
	eventTaskObject.msgTask.rxer.id = FWK_ID_EVENT_TASK;
	eventTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	eventTaskObject.msgTask.rxer.pMsgDispatcher = EventTaskMsgDispatcher;
	eventTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	eventTaskObject.msgTask.timerPeriodTicks = K_MSEC(0); /* One shot */
	eventTaskObject.msgTask.rxer.pQueue = &eventTaskQueue;

	Framework_RegisterTask(&eventTaskObject.msgTask);

	eventTaskObject.msgTask.pTid =
		k_thread_create(&eventTaskObject.msgTask.threadData, eventTaskStack,
				K_THREAD_STACK_SIZEOF(eventTaskStack), EventTaskThread,
				&eventTaskObject, NULL, NULL, EVENT_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(eventTaskObject.msgTask.pTid, THIS_FILE);
}

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
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
	attr_get(ATTR_ID_data_logging_enable, &dataLogEnable, sizeof(dataLogEnable));

	lcz_event_manager_set_logging_state(dataLogEnable);
}

static DispatchResult_t EventLogTimeStampMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	SensorMsg_t eventData;

	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	eventData.event.type = pEventMsg->eventType;
	eventData.event.data = pEventMsg->eventData;

	/* We always add events to the Event Manager for long term storage */
	eventData.event.timestamp =
		lcz_event_manager_add_sensor_event(pEventMsg->eventType, &pEventMsg->eventData);

	SendEventDataAdvert(&eventData);
	return DISPATCH_OK;
}
static DispatchResult_t EventNoTimeStampMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	SensorMsg_t eventData;
	EventLogMsg_t *pEventMsg = (EventLogMsg_t *)pMsg;

	eventData.event.type = pEventMsg->eventType;
	eventData.event.data = pEventMsg->eventData;
	eventData.event.timestamp = pEventMsg->timeStamp;

	SendEventDataAdvert(&eventData);
	return DISPATCH_OK;
}

static DispatchResult_t EventAttrChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsgRxer);
	attr_changed_msg_t *pb = (attr_changed_msg_t *)pMsg;
	size_t i;
	for (i = 0; i < pb->count; i++) {
		switch (pb->list[i]) {
		case ATTR_ID_data_logging_enable:
			SetDataloggerStatus();
			break;
		default:
			/* Don't care about this attribute. This is a
			 * broadcast.
			 */
			break;
		}
	}
	return DISPATCH_OK;
}
static void SendEventDataAdvert(SensorMsg_t *sensor_event)
{
	bool activeEvent = false;
	/*Check if the event flag is active */
	activeEvent = eventFilter(sensor_event->event.type);

	if (activeEvent == true) {
		/* Now post the event to the BLE Task */
		EventLogMsg_t *pMsgSend = (EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

		if (pMsgSend != NULL) {
			pMsgSend->header.msgCode = FMC_SENSOR_EVENT;
			pMsgSend->header.txId = FWK_ID_SENSOR_TASK;
			pMsgSend->header.rxId = FWK_ID_BLE_TASK;
			pMsgSend->eventType = sensor_event->event.type;
			pMsgSend->eventData = sensor_event->event.data;
			pMsgSend->id = event_task_event_id++;
			pMsgSend->timeStamp = sensor_event->event.timestamp;
			FRAMEWORK_MSG_SEND(pMsgSend);
		}
	}
}

static bool eventFilter(SensorEventType_t eventType)
{
	bool filterStatus = false;
	switch (eventType) {
	case SENSOR_EVENT_TEMPERATURE_1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_TEMPERATURE_1_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_TEMPERATURE_2:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_TEMPERATURE_2_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_TEMPERATURE_3:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_TEMPERATURE_3_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_TEMPERATURE_4:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_TEMPERATURE_4_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_VOLTAGE_1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_VOLTAGE_1_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_VOLTAGE_2:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_VOLTAGE_2_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_VOLTAGE_3:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_VOLTAGE_3_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_VOLTAGE_4:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_VOLTAGE_4_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_CURRENT_1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_CURRENT_1_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_CURRENT_2:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_CURRENT_2_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_CURRENT_3:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_CURRENT_3_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_CURRENT_4:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_CURRENT_4_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_ULTRASONIC_1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_ULTRASONIC_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_PRESSURE_1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_PRESSURE_1_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_PRESSURE_2:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags, FLAG_PRESSURE_2_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_TAMPER:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_TAMPER_SWITCH_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_MAGNET:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_MAGNET_SENSE_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_BATTERY_GOOD:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_BATTERY_GOOD_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_BATTERY_BAD:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_BATTERY_BAD_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_DIGITAL_IN1:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_DIGITAL_IN1_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	case SENSOR_EVENT_DIGITAL_IN2:
		if (attr_are_flags_set(ATTR_ID_event_filter_flags,
				       FLAG_DIGITAL_IN2_EVENT_BITMASK)) {
			filterStatus = true;
		}
		break;
	default:
		filterStatus = false;
		break;
	}

	return filterStatus;
}