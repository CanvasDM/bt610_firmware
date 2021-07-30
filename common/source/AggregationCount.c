/**
 * @file AggregationCount.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(AggregationCount, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "lcz_bluetooth.h"
#include "Advertisement.h"
#include "Attribute.h"
#include "AggregationCount.h"
#include "SensorTask.h"
#include "EventTask.h"
#include "Flags.h"
#include "AnalogInput.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define AGGREGATION_MAX_SIZE (32)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static void EventTypeHandler(SensorEventType_t eventType, float data);
static void AggregationEventTrigger(SensorMsg_t *sensor_event);
static SensorEventType_t AnalogConfigType(size_t channel);

/* This is the local queue used to store thermistor measurements 
* used in the aggragation count.
 */
K_MSGQ_DEFINE(aggregation_timestamp_queue, sizeof(SensorMsg_t),
	      AGGREGATION_MAX_SIZE, FWK_QUEUE_ALIGNMENT);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AggregationTempHandler(size_t channel, float value)
{
	uint8_t aggCount = 0;
	SensorMsg_t Tempevent;
	static uint8_t currentAggregationNumber = 0;
	int r = -EPERM;

	r = Attribute_Get(ATTR_INDEX_AggregationCount, &aggCount,
			  sizeof(aggCount));

	if (r > 0) {
		if (aggCount > 1) {
			if (currentAggregationNumber <= aggCount) {
				if (k_msgq_num_free_get(
					    &aggregation_timestamp_queue)) {

					Tempevent.event.type =
						SENSOR_EVENT_TEMPERATURE_1 +
						channel;
					Tempevent.event.data.f = value;
					/* We always add events to the Event Manager for long term storage */
					Tempevent.event.timestamp =
						lcz_event_manager_add_sensor_event(
							Tempevent.event.type,
							&Tempevent.event.data);

					currentAggregationNumber =
						currentAggregationNumber + 1;

					/* Set event details in the AGGREGATION queue */
					k_msgq_put(&aggregation_timestamp_queue,
						   &Tempevent.event.timestamp,
						   K_NO_WAIT);
				}
			}
			if (currentAggregationNumber == aggCount) {
				/*Need to send out past temp value*/
				while (k_msgq_num_used_get(
					&aggregation_timestamp_queue)) {
					k_msgq_get(&aggregation_timestamp_queue,
						   &Tempevent, K_FOREVER);
					AggregationEventTrigger(&Tempevent);
				};
				currentAggregationNumber = 0;
			}
		} else {
			/*Send to event LOG */
			EventTypeHandler((SENSOR_EVENT_TEMPERATURE_1 + channel),
					 value);
		}
	}
	return r;
}

int AggregationAnalogHandler(size_t channel, float value)
{
	int r = 0;
	/*Send to event LOG */
	EventTypeHandler(AnalogConfigType(channel), value);

	return r;
}

void AggregationPurgeQueueHandler(void)
{
	/* Purge the aggregation event queue so because the type
	 * has changed will fill with new data
	 */
	k_msgq_purge(&aggregation_timestamp_queue);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void EventTypeHandler(SensorEventType_t eventType, float data)
{
	SensorEventData_t eventData;
	LOG_DBG("Single Event type %d", eventType);

	eventData.f = data;

	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_EVENT_TRIGGER;
		pMsgSend->header.txId = FWK_ID_SENSOR_TASK;
		pMsgSend->header.rxId = FWK_ID_EVENT_TASK;
		pMsgSend->eventType = eventType;
		pMsgSend->eventData = eventData;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
}
static void AggregationEventTrigger(SensorMsg_t *sensor_event)
{
	/* Now post the event to the BLE Task */
	LOG_DBG("Aggregation type %d", sensor_event->event.type);
	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_AGGREGATION_EVENT;
		pMsgSend->header.txId = FWK_ID_SENSOR_TASK;
		pMsgSend->header.rxId = FWK_ID_EVENT_TASK;
		pMsgSend->eventType = sensor_event->event.type;
		pMsgSend->eventData = sensor_event->event.data;
		pMsgSend->id = 0;
		pMsgSend->timeStamp = sensor_event->event.timestamp;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
}
static SensorEventType_t AnalogConfigType(size_t channel)
{
	analogConfigType_t configType;
	SensorEventType_t eventTypeReturn;
	Attribute_Get((ATTR_INDEX_analogInput1Type + channel), &configType,
		      sizeof(uint8_t));

	switch (configType) {
	case ANALOG_VOLTAGE:
		eventTypeReturn = SENSOR_EVENT_VOLTAGE_1 + channel;
		break;
	case ANALOG_CURRENT:
		eventTypeReturn = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	case ANALOG_PRESSURE:
		eventTypeReturn = SENSOR_EVENT_PRESSURE_1 + channel;
		break;
	case ANALOG_ULTRASONIC:
		eventTypeReturn = SENSOR_EVENT_ULTRASONIC_1;
		break;
	case ANALOG_CURRENT20A:
	case ANALOG_CURRENT150A:
	case ANALOG_CURRENT500A:
		eventTypeReturn = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	default:
		/*should not get here but just in case still return something*/
		eventTypeReturn = SENSOR_EVENT_VOLTAGE_1;
		break;
	}
	return (eventTypeReturn);
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
