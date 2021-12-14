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
#include <Framework.h>
#include <FrameworkMacros.h>
#include <FrameworkMsgTypes.h>
#include <framework_ids.h>
#include <framework_msgcodes.h>

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "lcz_bluetooth.h"
#include "attr.h"
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

#define BIT_SET (1)
#define BIT_CLEAR (0)
#define NUMBER_OF_CHANNELS (4)
#define CHANNEL_0_MASK (0x1F)
#define CHANNEL_1_MASK (0x3E0)
#define CHANNEL_2_MASK (0x7C00)
#define CHANNEL_3_MASK (0xF8000)
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void AlarmTypeHandler(SensorEventType_t alarmType);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static uint32_t alarmChannelMask[NUMBER_OF_CHANNELS] = {
	CHANNEL_0_MASK, CHANNEL_1_MASK, CHANNEL_2_MASK, CHANNEL_3_MASK
};
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void DeactivateTempAlarm(size_t channel)
{
	uint32_t tempAlarmsEnabled = 0;
	uint32_t tempAlarmFlags = 0;

	LOG_DBG("turn off Temp alarm %x", ~alarmChannelMask[channel]);

	/* Turn off the enabled alarm */
	attr_copy_uint32(&tempAlarmsEnabled,
			 ATTR_ID_temperature_alarms_enable);
	tempAlarmsEnabled = tempAlarmsEnabled & ~alarmChannelMask[channel];
	attr_set_uint32(ATTR_ID_temperature_alarms_enable, tempAlarmsEnabled);

	/* Clear the alarm flags if they were on */
	attr_copy_uint32(&tempAlarmFlags, ATTR_ID_temperature_alarms);
	tempAlarmFlags = tempAlarmFlags & ~alarmChannelMask[channel];
	attr_set_uint32(ATTR_ID_temperature_alarms, tempAlarmFlags);

	/* Turn off the channel alarm flag */
	Flags_Set(TEMP_ALARM_MASK, (FLAG_TEMP_ALARM_START_BIT + channel),
		  BIT_CLEAR);
}

int HighTempAlarmCheck(size_t channel, float value)
{
	float highTempAlarm1 = 0;
	float highTempAlarm2 = 0;
	uint32_t highTempAlarmEnable = 0;
	int r = -EPERM;

	attr_copy_float(&highTempAlarm1, ATTR_ID_high_temp_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	attr_copy_float(&highTempAlarm2, ATTR_ID_high_temp_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = attr_copy_uint32(&highTempAlarmEnable,
			     ATTR_ID_temperature_alarms_enable);

	if (r == 0) {
		uint8_t high1TempBit =
			HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t high2TempBit =
			HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value >= highTempAlarm1) &&
		    (highTempAlarmEnable & BIT(high1TempBit))) {
			attr_set_mask32(ATTR_ID_temperature_alarms,
					high1TempBit, BIT_SET);

		} else {
			/* Alarm not enabled clear the active parameter
			 * if it was set
			 */
			attr_set_mask32(ATTR_ID_temperature_alarms,
					high1TempBit, BIT_CLEAR);
		}

		if ((value >= highTempAlarm2) &&
		    (highTempAlarmEnable & BIT(high2TempBit))) {
			attr_set_mask32(ATTR_ID_temperature_alarms,
					high2TempBit, BIT_SET);

		} else {
			/* Alarm not enabled clear the active parameter
			 * if it was set
			 */
			attr_set_mask32(ATTR_ID_temperature_alarms,
					high2TempBit, BIT_CLEAR);
		}
	}
	return r;
}

int LowTempAlarmCheck(size_t channel, float value)
{
	float lowTempAlarm1 = 0;
	float lowTempAlarm2 = 0;
	uint32_t lowTempAlarmEnable = 0;
	int r = -EPERM;

	attr_copy_float(&lowTempAlarm1, ATTR_ID_low_temp_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	attr_copy_float(&lowTempAlarm2, ATTR_ID_low_temp_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = attr_copy_uint32(&lowTempAlarmEnable,
			     ATTR_ID_temperature_alarms_enable);
	if (r == 0) {
		uint8_t low1TempBit =
			LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t low2TempBit =
			LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value <= lowTempAlarm1) &&
		    (lowTempAlarmEnable & BIT(low1TempBit))) {
			attr_set_mask32(ATTR_ID_temperature_alarms, low1TempBit,
					BIT_SET);
		} else {
			attr_set_mask32(ATTR_ID_temperature_alarms, low1TempBit,
					BIT_CLEAR);
		}
		if ((value <= lowTempAlarm2) &&
		    (lowTempAlarmEnable & BIT(low2TempBit))) {
			attr_set_mask32(ATTR_ID_temperature_alarms, low2TempBit,
					BIT_SET);
		} else {
			attr_set_mask32(ATTR_ID_temperature_alarms, low2TempBit,
					BIT_CLEAR);
		}
	}
	return r;
}

int DeltaTempAlarmCheck(size_t channel, float tempDifference)
{
	float threshold = 0;
	uint32_t deltaTempAlarmEnable = 0;
	uint8_t deltaBit = DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

	attr_copy_float(&threshold, ATTR_ID_temp_1_delta_thresh +
					       (NUMBER_ALARM_TYPES * channel));

	attr_copy_uint32(&deltaTempAlarmEnable,
			 ATTR_ID_temperature_alarms_enable);

	if ((tempDifference > threshold) &&
	    (deltaTempAlarmEnable & BIT(deltaBit))) {
		attr_set_mask32(ATTR_ID_temperature_alarms, deltaBit, BIT_SET);

	} else {
		attr_set_mask32(ATTR_ID_temperature_alarms, deltaBit, 0);
	}
	return 0;
}
int TempAlarmFlagCheck(size_t channel)
{
	uint32_t temperatureAlarms = 0;
	uint8_t flagTempBit = 0;

	flagTempBit = FLAG_TEMP_ALARM_START_BIT + channel;

	attr_copy_uint32(&temperatureAlarms, ATTR_ID_temperature_alarms);

	if (temperatureAlarms > 0) {
		Flags_Set(TEMP_ALARM_MASK, flagTempBit, BIT_SET);
		AlarmTypeHandler(SENSOR_EVENT_TEMPERATURE_ALARM);
	} else {
		Flags_Set(TEMP_ALARM_MASK, flagTempBit, BIT_CLEAR);
	}
	return 0;
}

void DeactivateAnalogAlarm(size_t channel)
{
	uint32_t analogAlarmsEnabled = 0;
	uint32_t analogAlarmFlags = 0;

	LOG_DBG("turn off Analog alarm %x", ~alarmChannelMask[channel]);

	/* Turn off the enabled alarm */
	attr_copy_uint32(&analogAlarmsEnabled, ATTR_ID_analog_alarms_enable);
	analogAlarmsEnabled = analogAlarmsEnabled & ~alarmChannelMask[channel];
	attr_set_uint32(ATTR_ID_analog_alarms_enable, analogAlarmsEnabled);

	/* Clear the alarm flags if they were on */
	attr_copy_uint32(&analogAlarmFlags, ATTR_ID_analog_alarms);
	analogAlarmFlags = analogAlarmFlags & ~alarmChannelMask[channel];
	attr_set_uint32(ATTR_ID_analog_alarms, analogAlarmFlags);

	/* Turn off the channel alarm flag */
	Flags_Set(ANALOG_ALARM_MASK, (FLAG_ANALOG_ALARM_START_BIT + channel),
		  BIT_CLEAR);
}

int HighAnalogAlarmCheck(size_t channel, float value)
{
	float highAnalogAlarm1 = 0;
	float highAnalogAlarm2 = 0;
	uint32_t highAnalogAlarmEnable = 0;
	int r = -EPERM;

	attr_copy_float(&highAnalogAlarm1, ATTR_ID_high_analog_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	attr_copy_float(&highAnalogAlarm2, ATTR_ID_high_analog_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = attr_copy_uint32(&highAnalogAlarmEnable,
			     ATTR_ID_analog_alarms_enable);
	LOG_DBG("high Enabled 1 %x", highAnalogAlarmEnable);

	if (r == 0) {
		uint8_t high1AnalogBit =
			HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t high2AnalogBit =
			HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value >= highAnalogAlarm1) &&
		    (highAnalogAlarmEnable & BIT(high1AnalogBit))) {
			attr_set_mask32(ATTR_ID_analog_alarms, high1AnalogBit,
					BIT_SET);

		} else {
			attr_set_mask32(ATTR_ID_analog_alarms, high1AnalogBit,
					BIT_CLEAR);
		}

		if ((value >= highAnalogAlarm2) &&
		    (highAnalogAlarmEnable & BIT(high2AnalogBit))) {
			attr_set_mask32(ATTR_ID_analog_alarms, high2AnalogBit,
					BIT_SET);

		} else {
			attr_set_mask32(ATTR_ID_analog_alarms, high2AnalogBit,
					BIT_CLEAR);
		}
	}
	return r;
}
int LowAnalogAlarmCheck(size_t channel, float value)
{
	float lowAnalogAlarm1 = 0;
	float lowAnalogAlarm2 = 0;
	uint32_t lowAnalogAlarmEnable = 0;
	int r = -EPERM;

	attr_copy_float(&lowAnalogAlarm1, ATTR_ID_low_analog_1_thresh_1 +
				   (NUMBER_ALARM_TYPES * channel));
	attr_copy_float(&lowAnalogAlarm2, ATTR_ID_low_analog_1_thresh_2 +
				   (NUMBER_ALARM_TYPES * channel));

	r = attr_copy_uint32(&lowAnalogAlarmEnable,
			     ATTR_ID_analog_alarms_enable);

	if (r == 0) {
		uint8_t low1AnalogBit =
			LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);
		uint8_t low2AnalogBit =
			LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

		if ((value <= lowAnalogAlarm1) &&
		    (lowAnalogAlarmEnable & BIT(low1AnalogBit))) {
			attr_set_mask32(ATTR_ID_analog_alarms, low1AnalogBit,
					BIT_SET);

		} else {
			attr_set_mask32(ATTR_ID_analog_alarms, low1AnalogBit,
					BIT_CLEAR);
		}
		if ((value <= lowAnalogAlarm2) &&
		    (lowAnalogAlarmEnable & BIT(low2AnalogBit))) {
			attr_set_mask32(ATTR_ID_analog_alarms, low2AnalogBit,
					BIT_SET);
		} else {
			attr_set_mask32(ATTR_ID_analog_alarms, low2AnalogBit,
					BIT_CLEAR);
		}
	}
	return r;
}

int DeltaAnalogAlarmCheck(size_t channel, float analogDifference)
{
	float threshold = 0;
	uint32_t deltaAnalogAlarmEnable = 0;
	uint8_t deltaBit = DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

	attr_copy_uint32(&deltaAnalogAlarmEnable, ATTR_ID_analog_alarms_enable);

	attr_copy_float(&threshold, ATTR_ID_analog_1_delta_thresh +
					       (NUMBER_ALARM_TYPES * channel));

	if ((analogDifference > threshold) &&
	    (deltaAnalogAlarmEnable & BIT(deltaBit))) {
		attr_set_mask32(ATTR_ID_analog_alarms, deltaBit, BIT_SET);

	} else {
		attr_set_mask32(ATTR_ID_analog_alarms, deltaBit, BIT_CLEAR);
	}
	return 0;
}
int AnalogAlarmFlagCheck(size_t channel)
{
	uint32_t analogAlarms = 0;
	uint8_t flagAnalogBit = 0;

	flagAnalogBit = FLAG_ANALOG_ALARM_START_BIT + channel;

	attr_copy_uint32(&analogAlarms, ATTR_ID_analog_alarms);

	if (analogAlarms & alarmChannelMask[channel]) {
		Flags_Set(ANALOG_ALARM_MASK, flagAnalogBit, BIT_SET);
		AlarmTypeHandler(SENSOR_EVENT_ANALOG_ALARM);
	} else {
		Flags_Set(ANALOG_ALARM_MASK, flagAnalogBit, BIT_CLEAR);
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
		attr_get(ATTR_ID_temperature_alarms, &tempAlarm,
			 sizeof(tempAlarm));

		eventAlarm.u32 = tempAlarm;
	} else if (alarmType == SENSOR_EVENT_ANALOG_ALARM) {
		uint32_t analogAlarm = 0;
		attr_get(ATTR_ID_analog_alarms, &analogAlarm,
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
