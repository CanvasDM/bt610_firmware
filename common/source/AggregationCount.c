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
#include "Attribute.h"
#include "AggregationCount.h"
#include "SensorTask.h"
#include "EventTask.h"
#include "Flags.h"
#include "AnalogInput.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define AGGREGATION_MAX_SIZE (32)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
float temperatureBuffer[TOTAL_THERM_CH][AGGREGATION_MAX_SIZE];
float analogBuffer[TOTAL_ANALOG_CH][AGGREGATION_MAX_SIZE];
static void EventTypeHandler(SensorEventType_t eventType, float data);
static void AnalogConfigType(size_t channel, float analogValue);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AggregationTempHandler(size_t channel, float value)
{
	uint8_t aggCount = 0;
	static uint8_t currentAggregationNumber = 0;
	int r = -EPERM;

	r = Attribute_Get(ATTR_INDEX_AggregationCount, &aggCount,
			  sizeof(aggCount));

	if (r > 0) {
		if (aggCount > 1) {
			if (currentAggregationNumber <= aggCount) {
				temperatureBuffer[channel]
						 [currentAggregationNumber] =
							 value;
				currentAggregationNumber =
					currentAggregationNumber + 1;
			}
			if (currentAggregationNumber == aggCount) {
				EventTypeHandler(
					(SENSOR_EVENT_TEMPERATURE_1 + channel),
					value);
				/*TODO: Need to send out past temp value*/
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
	uint8_t aggCount = 0;
	static uint8_t currentAggregationNumber = 0;
	int r = -EPERM;

	r = Attribute_Get(ATTR_INDEX_AggregationCount, &aggCount,
			  sizeof(aggCount));

	if (r > 0) {
		if (aggCount > 1) {
			if (currentAggregationNumber <= aggCount) {
				analogBuffer[channel][currentAggregationNumber] =
					value;
				currentAggregationNumber =
					currentAggregationNumber + 1;
			}
			if (currentAggregationNumber == aggCount) {
				/*Send to event LOG */
				AnalogConfigType(channel, value);
			}
		} else {
			/*Send to event LOG */
			AnalogConfigType(channel, value);
		}
	}
	return r;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void EventTypeHandler(SensorEventType_t eventType, float data)
{
	SensorEventData_t eventData;
	LOG_DBG("Aggregation type %d", eventType);

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
static void AnalogConfigType(size_t channel, float analogValue)
{
	analogConfigType_t configType;
	SensorEventType_t eventType;
	Attribute_Get((ATTR_INDEX_analogInput1Type + channel), &configType,
		      sizeof(uint8_t));

	switch (configType) {
	case ANALOG_VOLTAGE:
		eventType = SENSOR_EVENT_VOLTAGE_1 + channel;
		break;
	case ANALOG_CURRENT:
		eventType = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	case ANALOG_PRESSURE:
		eventType = SENSOR_EVENT_PRESSURE_1 + channel;
		break;
	case ANALOG_ULTRASONIC:
		eventType = SENSOR_EVENT_ULTRASONIC_1;
		break;
	case ANALOG_Current20A:
	case ANALOG_Current150A:
	case ANALOG_Current500A:
		eventType = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	default:
		/*should not get here but just in case still return something*/
		eventType = SENSOR_EVENT_VOLTAGE_1;
		break;
	}
	EventTypeHandler(eventType, analogValue);
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
