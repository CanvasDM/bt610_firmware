/**
 * @file AlarmControl.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(AlarmControl, CONFIG_ALARM_CONTROL_LOG_LEVEL);

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
#include "AlarmControl.h"
#include "SensorTask.h"
#include "EventTask.h"
#include "Flags.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
typedef enum {
	HIGH_THRESH_1 = 0,
	HIGH_THRESH_2,
	LOW_THRESH_1,
	LOW_THRESH_2,
	DELTA_THRESH,
	NUMBER_ALARM_TYPES
} AlarmTypes_t;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void AlarmTypeHandler(SensorEventType_t alarmType);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int HighTempAlarmCheck(size_t channel, float value)
{
	float highTempAlarm1 = 0;
	float highTempAlarm2 = 0;
	bool highTempAlarmActive = false;
	uint32_t highTempAlarmEnable = 0;
	int r = -EPERM;

	Attribute_GetFloat(&highTempAlarm1,
			   ATTR_INDEX_high_temp_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	Attribute_GetFloat(&highTempAlarm2,
			   ATTR_INDEX_high_temp_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = Attribute_GetUint32(&highTempAlarmEnable,
				ATTR_INDEX_temperature_alarms_enable);

	if (r == 0) {
		uint8_t high1TempBit =
			HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t high2TempBit =
			HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value >= highTempAlarm1) &&
		    (highTempAlarmEnable & BIT(high1TempBit))) {
			highTempAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    high1TempBit, 1);

		} else {
			/* Alarm not enabled clear the active parameter
			 * if it was set
			 */
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    high1TempBit, 0);
		}

		if ((value >= highTempAlarm2) &&
		    (highTempAlarmEnable & BIT(high2TempBit))) {
			highTempAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    high2TempBit, 1);

		} else {
			/* Alarm not enabled clear the active parameter
			 * if it was set
			 */
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    high2TempBit, 0);
		}
		if (highTempAlarmActive == true) {
			/* Clear any Low alarm flags if they are set */
			Attribute_SetMask32(
				ATTR_INDEX_analog_alarms,
				(LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel)),
				0);
			Attribute_SetMask32(
				ATTR_INDEX_analog_alarms,
				(LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel)),
				0);
			AlarmTypeHandler(SENSOR_EVENT_TEMPERATURE_ALARM);
		}
	}
	return r;
}

int LowTempAlarmCheck(size_t channel, float value)
{
	float lowTempAlarm1 = 0;
	float lowTempAlarm2 = 0;
	uint32_t lowTempAlarmEnable = 0;
	bool lowTempAlarmActive = false;
	int r = -EPERM;

	Attribute_GetFloat(&lowTempAlarm1,
			   ATTR_INDEX_low_temp_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	Attribute_GetFloat(&lowTempAlarm2,
			   ATTR_INDEX_low_temp_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = Attribute_GetUint32(&lowTempAlarmEnable,
				ATTR_INDEX_temperature_alarms_enable);
	if (r == 0) {
		uint8_t low1TempBit =
			LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t low2TempBit =
			LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value <= lowTempAlarm1) &&
		    (lowTempAlarmEnable & BIT(low1TempBit))) {
			lowTempAlarmActive = true;
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    low1TempBit, 1);
		} else {
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    low1TempBit, 0);
		}
		if ((value <= lowTempAlarm2) &&
		    (lowTempAlarmEnable & BIT(low2TempBit))) {
			lowTempAlarmActive = true;
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    low2TempBit, 1);
		} else {
			Attribute_SetMask32(ATTR_INDEX_temperature_alarms,
					    low2TempBit, 0);
		}
		if (lowTempAlarmActive == true) {
			/* Clear any High alarm flags if they are set */
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    (HIGH_THRESH_1 +
					     (NUMBER_ALARM_TYPES * channel)),
					    0);
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    (HIGH_THRESH_2 +
					     (NUMBER_ALARM_TYPES * channel)),
					    0);
			AlarmTypeHandler(SENSOR_EVENT_TEMPERATURE_ALARM);
		}
	}
	return r;
}

int DeltaTempAlarmCheck(size_t channel, float tempDifference)
{
	float threshold = 0;
	uint32_t deltaTempAlarmEnable = 0;
	uint8_t deltaBit = DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

	Attribute_GetFloat(&threshold, ATTR_INDEX_temp_1_delta_thresh +
					       (NUMBER_ALARM_TYPES * channel));

	Attribute_GetUint32(&deltaTempAlarmEnable,
			    ATTR_INDEX_temperature_alarms_enable);

	if ((tempDifference > threshold) &&
	    (deltaTempAlarmEnable & BIT(deltaBit))) {
		AlarmTypeHandler(SENSOR_EVENT_TEMPERATURE_ALARM);

		Attribute_SetMask32(ATTR_INDEX_temperature_alarms, deltaBit, 1);

	} else {
		Attribute_SetMask32(ATTR_INDEX_temperature_alarms, deltaBit, 0);
	}
	return 0;
}
int TempAlarmFlagCheck(size_t channel)
{
	uint32_t temperatureAlarms = 0;
	uint8_t flagTempBit = 0;

	flagTempBit = FLAG_TEMP_ALARM_START_BIT + channel;

	Attribute_GetUint32(&temperatureAlarms, ATTR_INDEX_temperature_alarms);

	if (temperatureAlarms > 0) {
		Flags_Set(TEMP_ALARM_MASK, flagTempBit, 1);
	} else {
		Flags_Set(TEMP_ALARM_MASK, flagTempBit, 0);
	}
	return 0;
}

int HighAnalogAlarmCheck(size_t channel, float value)
{
	float highAnalogAlarm1 = 0;
	float highAnalogAlarm2 = 0;
	bool highAnalogAlarmActive = false;
	uint32_t highAnalogAlarmEnable = 0;
	int r = -EPERM;

	Attribute_GetFloat(&highAnalogAlarm1,
			   ATTR_INDEX_high_analog_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	Attribute_GetFloat(&highAnalogAlarm2,
			   ATTR_INDEX_high_analog_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = Attribute_GetUint32(&highAnalogAlarmEnable,
				ATTR_INDEX_analog_alarms_enable);

	if (r == 0) {
		uint8_t high1AnalogBit =
			HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t high2AnalogBit =
			HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value >= highAnalogAlarm1) &&
		    (highAnalogAlarmEnable & BIT(high1AnalogBit))) {
			highAnalogAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    high1AnalogBit, 1);

		} else {
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    high1AnalogBit, 0);
		}

		if ((value >= highAnalogAlarm2) &&
		    (highAnalogAlarmEnable & BIT(high2AnalogBit))) {
			highAnalogAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    high2AnalogBit, 1);

		} else {
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    high2AnalogBit, 0);
		}
		if (highAnalogAlarmActive == true) {
			/* Clear any Low alarm flags if they are set */
			Attribute_SetMask32(
				ATTR_INDEX_analog_alarms,
				(LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel)),
				0);
			Attribute_SetMask32(
				ATTR_INDEX_analog_alarms,
				(LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel)),
				0);
			AlarmTypeHandler(SENSOR_EVENT_ANALOG_ALARM);
		}
	}
	return r;
}
int LowAnalogAlarmCheck(size_t channel, float value)
{
	float lowAnalogAlarm1 = 0;
	float lowAnalogAlarm2 = 0;
	bool lowAnalogAlarmActive = false;
	uint32_t lowAnalogAlarmEnable = 0;
	int r = -EPERM;

	Attribute_GetFloat(&lowAnalogAlarm1,
			   ATTR_INDEX_low_analog_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	Attribute_GetFloat(&lowAnalogAlarm2,
			   ATTR_INDEX_low_analog_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = Attribute_GetUint32(&lowAnalogAlarmEnable,
				ATTR_INDEX_analog_alarms_enable);

	if (r == 0) {
		uint8_t low1AnalogBit =
			LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t low2AnalogBit =
			LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value <= lowAnalogAlarm1) &&
		    (lowAnalogAlarmEnable & BIT(low1AnalogBit))) {
			lowAnalogAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    low1AnalogBit, 1);

		} else {
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    low1AnalogBit, 0);
		}
		if ((value <= lowAnalogAlarm2) &&
		    (lowAnalogAlarmEnable & BIT(low2AnalogBit))) {
			lowAnalogAlarmActive = true;

			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    low2AnalogBit, 1);
		} else {
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    low2AnalogBit, 0);
		}
		if (lowAnalogAlarmActive == true) {
			/* Clear any High alarm flags if they are set */
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    (HIGH_THRESH_1 +
					     (NUMBER_ALARM_TYPES * channel)),
					    0);
			Attribute_SetMask32(ATTR_INDEX_analog_alarms,
					    (HIGH_THRESH_2 +
					     (NUMBER_ALARM_TYPES * channel)),
					    0);
			AlarmTypeHandler(SENSOR_EVENT_ANALOG_ALARM);
		}
	}
	return r;
}

int DeltaAnalogAlarmCheck(size_t channel, float analogDifference)
{
	float threshold = 0;
	uint32_t deltaAnalogAlarmEnable = 0;
	uint8_t deltaBit = DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

	Attribute_GetUint32(&deltaAnalogAlarmEnable,
			    ATTR_INDEX_analog_alarms_enable);

	Attribute_GetFloat(&threshold, ATTR_INDEX_analog_1_delta_thresh +
					       (NUMBER_ALARM_TYPES * channel));

	if ((analogDifference > threshold) &&
	    (deltaAnalogAlarmEnable & BIT(deltaBit))) {
		AlarmTypeHandler(SENSOR_EVENT_ANALOG_ALARM);

		Attribute_SetMask32(ATTR_INDEX_analog_alarms, deltaBit, 1);

	} else {
		Attribute_SetMask32(ATTR_INDEX_analog_alarms, deltaBit, 0);
	}
	return 0;
}
int AnalogAlarmFlagCheck(size_t channel)
{
	uint32_t analogAlarms = 0;
	uint8_t flagAnalogBit = 0;

	flagAnalogBit = FLAG_TEMP_ALARM_START_BIT + channel;

	Attribute_GetUint32(&analogAlarms, ATTR_INDEX_analog_alarms);

	if (analogAlarms > 0) {
		Flags_Set(ANALOG_ALARM_MASK, flagAnalogBit, 1);
	} else {
		Flags_Set(ANALOG_ALARM_MASK, flagAnalogBit, 0);
	}
	return 0;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void AlarmTypeHandler(SensorEventType_t alarmType)
{
	SensorEventData_t eventAlarm;
	LOG_DBG("Alarm type %d", alarmType);

	if (alarmType == SENSOR_EVENT_TEMPERATURE_ALARM) {
		uint32_t tempAlarm = 0;
		Attribute_Get(ATTR_INDEX_temperature_alarms, &tempAlarm,
			      sizeof(tempAlarm));

		eventAlarm.u32 = tempAlarm;
	} else if (alarmType == SENSOR_EVENT_ANALOG_ALARM) {
		uint32_t analogAlarm = 0;
		Attribute_Get(ATTR_INDEX_analog_alarms, &analogAlarm,
			      sizeof(analogAlarm));

		eventAlarm.u32 = analogAlarm;
	}

	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_EVENT_TRIGGER;
		pMsgSend->header.txId = FWK_ID_SENSOR_TASK;
		pMsgSend->header.rxId = FWK_ID_EVENT_TASK;
		pMsgSend->eventType = alarmType;
		pMsgSend->eventData = eventAlarm;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
}
