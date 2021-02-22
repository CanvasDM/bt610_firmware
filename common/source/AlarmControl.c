/**
 * @file AlarmControl.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(AlarmControl, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "laird_bluetooth.h"
#include "Attribute.h"
#include "AlarmControl.h"
#include "SensorTask.h"

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
/* Local Data Definitions                                                     */
/******************************************************************************/
/*static void AlarmTypeHandler(SensorEventType_t alarmType, float temperature,
			     float alarmThreshold);*/
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int HighTempAlarmCheck(size_t channel)
{
	float highTempAlarm1 = 0;
	float highTempAlarm2 = 0;
	float currentTemp = 0;
	int r = -EPERM;

	Attribute_GetFloat(&highTempAlarm1,
			   ATTR_INDEX_highTemp1Thresh1 + channel);
	Attribute_GetFloat(&highTempAlarm2,
			   ATTR_INDEX_highTemp1Thresh2 + channel);
	r = Attribute_GetFloat(&currentTemp,
			       ATTR_INDEX_temperatureResult1 + channel);
	if (r == 0) {
		if (currentTemp >= highTempAlarm1) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_1,
					 currentTemp, highTempAlarm1);*/
			uint8_t highTemp1Bit =
				HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_temperatureAlarms,
					    highTemp1Bit, 1);

			/*todo:Flags_Set(FLAG_HIGH_TEMP_ALARM, 1);*/

		} else if (currentTemp >= highTempAlarm2) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_2,
					 currentTemp, highTempAlarm2);*/
			uint8_t highTemp2Bit =
				HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_temperatureAlarms,
					    highTemp2Bit, 1);

			/*todo:Flags_Set*/
		}
	}
	return r;
}
int LowTempAlarmCheck(size_t channel)
{
	float lowTempAlarm1 = 0;
	float lowTempAlarm2 = 0;
	float currentTemp = 0;
	int r = -EPERM;

	Attribute_GetFloat(&lowTempAlarm1,
			   ATTR_INDEX_lowTemp1Thresh1 + channel);
	Attribute_GetFloat(&lowTempAlarm2,
			   ATTR_INDEX_lowTemp1Thresh2 + channel);
	r = Attribute_GetFloat(&currentTemp,
			       ATTR_INDEX_temperatureResult1 + channel);
	if (r == 0) {
		if (currentTemp <= lowTempAlarm1) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_1,
					 currentTemp, lowTempAlarm1);*/
			uint8_t lowTemp1Bit =
				LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_temperatureAlarms,
					    lowTemp1Bit, 1);

			/*todo:Flags_Set(FLAG_HIGH_TEMP_ALARM, 1);*/

		} else if (currentTemp <= lowTempAlarm2) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_2,
					 currentTemp, lowTempAlarm2);*/
			uint8_t lowTemp2Bit =
				LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_temperatureAlarms,
					    lowTemp2Bit, 1);

			/*todo:Flags_Set*/
		}
	}
	return r;
}
int DeltaTempAlarmCheck(size_t channel, float tempDifference)
{
	float threshold = 0;
	Attribute_GetFloat(&threshold, ATTR_INDEX_temp1DeltaThresh + channel);

	if (tempDifference > threshold) {
		/*AlarmTypeHandler(SENSOR_EVENT_ALARM_DELTA_TEMP,
				 pObj->currentTemp, threshold);*/

		uint8_t deltaBit =
			DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

		Attribute_SetMask32(ATTR_INDEX_temperatureAlarms, deltaBit, 1);
		/*todo:Flags_Set(FLAG_DELTA_TEMP_ALARM, 1);*/
	} else {
		/*todo: Flags_Set*/
	}
	return 0;
}

int HighAnalogAlarmCheck(size_t channel)
{
	float highAnalogAlarm1 = 0;
	float highAnalogAlarm2 = 0;
	float currentAnalogValue = 0;
	int r = -EPERM;

	Attribute_GetFloat(&highAnalogAlarm1,
			   ATTR_INDEX_highAnalog1Thresh1 + channel);
	Attribute_GetFloat(&highAnalogAlarm2,
			   ATTR_INDEX_highAnalog1Thresh2 + channel);
	r = Attribute_GetFloat(&currentAnalogValue,
			       ATTR_INDEX_analogInput1 + channel);
	if (r == 0) {
		if (currentAnalogValue >= highAnalogAlarm1) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_1,
					 currentAnalog, highAnalogAlarm1);*/
			uint8_t highAnalog1Bit =
				HIGH_THRESH_1 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_analogAlarms,
					    highAnalog1Bit, 1);

			/*todo: Flags_Set(FLAG_HIGH_TEMP_ALARM, 1);*/

		} else if (currentAnalogValue >= highAnalogAlarm2) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_2,
					 currentAnalog, highAnalogAlarm2);*/
			uint8_t highAnalog2Bit =
				HIGH_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_analogAlarms,
					    highAnalog2Bit, 1);

			/*todo:Flags_Set*/
		}
	}
	return r;
}
int LowAnalogAlarmCheck(size_t channel)
{
	float lowAnalogAlarm1 = 0;
	float lowAnalogAlarm2 = 0;
	float currentAnalogValue = 0;
	int r = -EPERM;

	Attribute_GetFloat(&lowAnalogAlarm1,
			   ATTR_INDEX_lowAnalog1Thresh1 + channel);
	Attribute_GetFloat(&lowAnalogAlarm2,
			   ATTR_INDEX_lowAnalog1Thresh2 + channel);
	r = Attribute_GetFloat(&currentAnalogValue,
			       ATTR_INDEX_analogInput1 + channel);
	if (r == 0) {
		if (currentAnalogValue <= lowAnalogAlarm1) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_1,
					 currentAnalog, lowAnalogAlarm1);*/
			uint8_t lowAnalog1Bit =
				LOW_THRESH_1 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_analogAlarms,
					    lowAnalog1Bit, 1);

			/*todo:Flags_Set(FLAG_HIGH_TEMP_ALARM, 1);*/

		} else if (currentAnalogValue <= lowAnalogAlarm2) {
			/*AlarmTypeHandler(SENSOR_EVENT_ALARM_HIGH_TEMP_2,
					 currentAnalog, lowAnalogAlarm2);*/
			uint8_t lowAnalog2Bit =
				LOW_THRESH_2 + (NUMBER_ALARM_TYPES * channel);

			Attribute_SetMask32(ATTR_INDEX_analogAlarms,
					    lowAnalog2Bit, 1);

			/* todo: Flags_Set function*/
		}
	}
	return r;
}

int DeltaAnalogAlarmCheck(size_t channel, float analogDifference)
{
	float threshold = 0;
	Attribute_GetFloat(&threshold, ATTR_INDEX_analog1DeltaThresh + channel);

	if (analogDifference > threshold) {
		/*AlarmTypeHandler(SENSOR_EVENT_ALARM_DELTA_ANALOG,
				 pObj->currentAnalog, threshold);*/

		uint8_t deltaBit =
			DELTA_THRESH + (NUMBER_ALARM_TYPES * channel);

		Attribute_SetMask32(ATTR_INDEX_analogAlarms, deltaBit, 1);
		/*todo:Flags_Set(FLAG_DELTA_ANALOG_ALARM, 1);*/
	} else {
		/*todo:Flags_Set*/
	}
	return 0;
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/*static void AlarmTypeHandler(SensorEventType_t alarmType, float temperature,
			     float alarmThreshold)
{
	LOG_DBG("Alarm %s at %d", GetSensorEventTypeString(alarmType),
		alarmThreshold);

	SensorMsg_t *pMsg = (SensorMsg_t *)BufferPool_Take(sizeof(SensorMsg_t));
	if (pMsg != NULL) {
		pMsg->header.msgCode = MSG_CODE_SENSOR;
		pMsg->header.txId = FRAMEWORK_SENSOR_TASK_ID;
		pMsg->header.rxId = FRAMEWORK_SENSOR_LOG_TASK_ID;
		pMsg->event.timestamp = Rtc_SysTimeGetSeconds();
		pMsg->event.data.temperatureCc = temperature;
		pMsg->event.type = alarmType;
		FRAMEWORK_MSG_SEND(pMsg);
	}
}*/
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
