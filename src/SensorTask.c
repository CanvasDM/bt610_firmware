/**
 * @file SensorTask.c
 * @brief Functions used to interface with the I/O
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(Sensor, CONFIG_SENSOR_TASK_LOG_LEVEL);
#define THIS_FILE "Sensor"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <stdlib.h>
#include <stdio.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <drivers/spi.h>
#include <time.h>

#include "FrameworkIncludes.h"
#include "attr.h"
#include "BspSupport.h"
#include "AdcBt6.h"
#include "AnalogInput.h"
#include "SensorTask.h"
#include "attr_custom_validator.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "Flags.h"

/* LWM2M telemetry additions */
#ifdef CONFIG_LCZ_LWM2M_CLIENT
#include "lcz_lwm2m_client.h"
#endif
#ifdef CONFIG_LWM2M_IPSO_CURRENT_SENSOR
#include "lcz_lwm2m_current.h"
#endif
#ifdef CONFIG_LWM2M_IPSO_PRESSURE_SENSOR
#include "lcz_lwm2m_pressure.h"
#endif
#ifdef CONFIG_LWM2M_IPSO_FILLING_SENSOR
#include "lcz_lwm2m_fill_level.h"
#endif
#ifdef CONFIG_LWM2M_IPSO_TEMP_SENSOR
#include "lcz_lwm2m_temperature.h"
#endif

/* Any of these being set indicates lwm2m telemetry support is needed */
#if defined CONFIG_LCZ_LWM2M_BATTERY || \
	defined CONFIG_LWM2M_IPSO_CURRENT_SENSOR || \
	defined CONFIG_LWM2M_IPSO_PRESSURE_SENSOR || \
	defined CONFIG_LWM2M_IPSO_FILLING_SENSOR || \
	defined CONFIG_LWM2M_IPSO_TEMP_SENSOR
	#define LWM2M_TELEMETRY_SUPPORT_ENABLED
#endif

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef SENSOR_TASK_PRIORITY
#define SENSOR_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef SENSOR_TASK_STACK_DEPTH
#define SENSOR_TASK_STACK_DEPTH 8192
#endif

#ifndef SENSOR_TASK_QUEUE_DEPTH
#define SENSOR_TASK_QUEUE_DEPTH 32
#endif

#define POWER_BAD_VOLTAGE (3.00)
#define DIGITAL_IN_ALARM_MASK (0x03)
#define DIGITAL_IN_ENABLE_MASK (0x80)
#define DIGITAL_IN_DISABLE_MASK (0x00)
#define DIGITAL_IN_BIT_SHIFT (7)
/* 0x0F is all the thermisters enabled*/
#define ALL_THERMISTORS (0x0F)

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi1), okay)
#define SPI_DEV_NAME DT_LABEL(DT_NODELABEL(spi1))
#else
#error "Please set the correct spi device"
#endif

typedef enum {
	NO_ALARM = 0,
	FALLING_EDGE_ALARM,
	RISING_EDGE_ALARM,
	BOTH_EDGE_ALARM
} digitalAlarm_t;

typedef struct SensorTaskTag {
	FwkMsgTask_t msgTask;
	uint8_t digitalIn1Enabled;
	uint8_t digitalIn2Enabled;
	digitalAlarm_t input1Alarm;
	digitalAlarm_t input2Alarm;
} SensorTaskObj_t;

#ifdef LWM2M_TELEMETRY_SUPPORT_ENABLED
#define EXPECT_OK() if (r != 0) { LOG_ERR("Failed to create LWM2M object!"); }
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static SensorTaskObj_t sensorTaskObject;
static struct k_timer powerTimer;
static struct k_timer temperatureReadTimer;
static struct k_timer analogReadTimer;

K_THREAD_STACK_DEFINE(sensorTaskStack, SENSOR_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(sensorTaskQueue, FWK_QUEUE_ENTRY_SIZE, SENSOR_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void SensorTaskThread(void *, void *, void *);
static DispatchResult_t
SensorTaskAttributeChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				     FwkMsg_t *pMsg);
static DispatchResult_t
SensorTaskDigitalInAlarmMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t
SensorTaskDigitalInConfigMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

static DispatchResult_t MagnetStateMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg);
static DispatchResult_t ReadPowerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					    FwkMsg_t *pMsg);
static DispatchResult_t MeasureTemperatureMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg);
static DispatchResult_t AnalogReadMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					     FwkMsg_t *pMsg);
static DispatchResult_t EnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg);
static DispatchResult_t EnterShelfModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);
static DispatchResult_t ClearInputConfigChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg);

static void LoadSensorConfiguration(void);
static void ClearInputConfigChangedFlag(void);
static void SensorConfigChange(bool bootup);
static void SensorOutput1Control(void);
static void SensorOutput2Control(void);

#ifdef CONFIG_LOG
static void printRTCTime(void);
#else
#define printRTCTime()
#endif

static void UpdateDin1(void);
static void UpdateDin2(void);
static void DisableDigitalIO(void);
static gpio_flags_t GetEdgeType(digitalAlarm_t alarm);
static void UpdateMagnet(void);
static void InitializeIntervalTimers(void);
static void StartAnalogInterval(void);
static void StartTemperatureInterval(void);
static void StartPowerInterval(void);
static void DisableAnalogReadings(void);
static void DisableThermistorReadings(void);

static int MeasureAnalogInput(size_t channel, AdcPwrSequence_t power,
			      float *result);
static int MeasureThermistor(size_t channel, AdcPwrSequence_t power,
			     float *result);
static void SendEvent(SensorEventType_t type, SensorEventData_t data);
static SensorEventType_t AnalogConfigType(size_t channel);

/* LWM2M telemetry update */
static int update_lwm2m_temperature(int index, float temperature);
static int update_lwm2m_pressure(int index, float pressure);
static int update_lwm2m_fill_level(int index, float fill_level);
static int update_lwm2m_current(int index, float current);
static int update_lwm2m_battery(lcz_lwm2m_client_device_battery_status_t status, float voltage);

static void powerTimerCallbackIsr(struct k_timer *timer_id);
static void temperatureReadTimerCallbackIsr(struct k_timer *timer_id);
static void analogReadTimerCallbackIsr(struct k_timer *timer_id);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t *SensorTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:             return Framework_UnknownMsgHandler;
	case FMC_ATTR_CHANGED:        return SensorTaskAttributeChangedMsgHandler;
	case FMC_DIGITAL_IN:          return SensorTaskDigitalInAlarmMsgHandler;
	case FMC_DIGITAL_IN_CONFIG:   return SensorTaskDigitalInConfigMsgHandler;
	case FMC_MAGNET_STATE:        return MagnetStateMsgHandler;
	case FMC_READ_POWER:          return ReadPowerMsgHandler;
	case FMC_TEMPERATURE_MEASURE: return MeasureTemperatureMsgHandler;
	case FMC_ANALOG_MEASURE:      return AnalogReadMsgHandler;
	case FMC_ENTER_ACTIVE_MODE:   return EnterActiveModeMsgHandler;
	case FMC_ENTER_SHELF_MODE:    return EnterShelfModeMsgHandler;
	case FMC_CLEAR_INPUT_CONFIG_CHANGED: return ClearInputConfigChangedMsgHandler;
	default:                      return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SensorTask_Initialize(void)
{
	#ifdef LWM2M_TELEMETRY_SUPPORT_ENABLED
	int r = 0;
	int channel_index;
	int object_index;
	uint8_t config_type;
	uint8_t analog_input_type[TOTAL_ANALOG_CH];
	uint8_t thermistor_config;
	#endif

	sensorTaskObject.msgTask.rxer.id = FWK_ID_SENSOR_TASK;
	sensorTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	sensorTaskObject.msgTask.rxer.pMsgDispatcher = SensorTaskMsgDispatcher;
	sensorTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	sensorTaskObject.msgTask.timerPeriodTicks = K_MSEC(0); /* One shot */
	sensorTaskObject.msgTask.rxer.pQueue = &sensorTaskQueue;

	sensorTaskObject.digitalIn1Enabled = NO_ALARM;
	sensorTaskObject.digitalIn2Enabled = NO_ALARM;
	sensorTaskObject.input1Alarm = 0;
	sensorTaskObject.input2Alarm = 0;

	/* Read back sensor configuration for telemetry object setup
	 * if telemetry is enabled.
	 */
	#ifdef LWM2M_TELEMETRY_SUPPORT_ENABLED
	/* Over-arching configuration setting */
	(void)attr_get(ATTR_ID_config_type, &config_type, sizeof(config_type));
	/* Analog input configurations */
	for (channel_index = 0; channel_index < TOTAL_ANALOG_CH; channel_index++) {
		(void)attr_get(ATTR_ID_analog_input_1_type + channel_index,
				&analog_input_type[channel_index],
				sizeof(analog_input_type[channel_index]));
	}
	/* Thermistor configuration */
	(void)attr_get(ATTR_ID_thermistor_config, &thermistor_config, sizeof(thermistor_config));
	#endif

	/* Instantiate LWM2M telemetry objects */
	#ifdef CONFIG_LWM2M_IPSO_TEMP_SENSOR
	if (config_type == CONFIG_TYPE_TEMPERATURE) {
		/* Check if a thermistor is configured on this input */
		for (channel_index = 0; (channel_index < TOTAL_THERM_CH) && (!r); channel_index++) {
			if (thermistor_config & BIT(channel_index)) {
				r = lcz_lwm2m_temperature_create(channel_index);
				EXPECT_OK();
			}
		}
	}
	#endif

	#ifdef CONFIG_LWM2M_IPSO_PRESSURE_SENSOR
	if (config_type == CONFIG_TYPE_ULTRASONIC_PRESSURE) {
		/* Check if a pressure sensor is configured on this input */
		for (channel_index = 0, object_index = 0; (channel_index < TOTAL_ANALOG_CH) && (!r)
			&& (object_index < ANALOG_INPUTS_MAX_PRESSURE_SENSORS); channel_index++) {
			if (analog_input_type[channel_index] == ANALOG_INPUT_1_TYPE_PRESSURE) {
				/* Keep track of the number of pressure sensors created.
				 * Leave as soon as two are allocated.
				 */
				object_index++;
				r = lcz_lwm2m_pressure_create(channel_index);
				EXPECT_OK();
			}
		}
	}
	#endif

	#ifdef CONFIG_LWM2M_IPSO_FILLING_SENSOR
	if (config_type == CONFIG_TYPE_ULTRASONIC_PRESSURE) {
		/* Check if an ultrasonic sensor is configured on this input */
		for (channel_index = 0, object_index = 0; (channel_index < TOTAL_ANALOG_CH) && (!r)
			&& (object_index < ANALOG_INPUTS_MAX_ULTRASONIC); channel_index++) {
			if (analog_input_type[channel_index] == ANALOG_INPUT_1_TYPE_ULTRASONIC) {
				/* Only one ultrasonic sensor is allowed */
				object_index++;
				r = lcz_lwm2m_fill_level_create(channel_index);
				EXPECT_OK();
			}
		}
	}
	#endif

	#ifdef CONFIG_LWM2M_IPSO_CURRENT_SENSOR
	if ((config_type == CONFIG_TYPE_ANALOG) || (config_type == CONFIG_TYPE_CURRENT)) {
		/* Check if a current sensor is configured on this input */
		for (channel_index = 0; (channel_index < TOTAL_ANALOG_CH) && (!r); channel_index++) {
			if ((analog_input_type[channel_index] ==
				ANALOG_INPUT_1_TYPE_CURRENT_4MA_TO_20MA) ||
				(analog_input_type[channel_index] ==
				ANALOG_INPUT_1_TYPE_AC_CURRENT_20A) ||
				(analog_input_type[channel_index] ==
				ANALOG_INPUT_1_TYPE_AC_CURRENT_150A) ||
				(analog_input_type[channel_index] ==
				ANALOG_INPUT_1_TYPE_AC_CURRENT_500A)) {
				r = lcz_lwm2m_current_create(channel_index);
				EXPECT_OK();
			}
		}
	}
	#endif

	/* Register the Sensor Task for Framework services */
	Framework_RegisterTask(&sensorTaskObject.msgTask);

	sensorTaskObject.msgTask.pTid =
		k_thread_create(&sensorTaskObject.msgTask.threadData,
				sensorTaskStack,
				K_THREAD_STACK_SIZEOF(sensorTaskStack),
				SensorTaskThread, &sensorTaskObject, NULL, NULL,
				SENSOR_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(sensorTaskObject.msgTask.pTid, THIS_FILE);
}

int attr_prepare_power_voltage(void)
{
	int16_t raw = 0;
	float volts = 0;
	SensorEventData_t eventAlarm;
	int r = AdcBt6_read_power_volts(&raw, &volts);
	#ifdef CONFIG_LCZ_LWM2M_CLIENT
	static lcz_lwm2m_client_device_battery_status_t battery_status;
	#endif

	if (r >= 0) {
		r = attr_set_signed32(ATTR_ID_power_voltage, volts);
		if (volts > POWER_BAD_VOLTAGE) {
			eventAlarm.f = volts;
			SendEvent(SENSOR_EVENT_BATTERY_GOOD, eventAlarm);
			Flags_Set(FLAG_LOW_BATTERY_ALARM, 0);
			#ifdef CONFIG_LCZ_LWM2M_CLIENT
			battery_status = LCZ_LWM2M_CLIENT_DEV_BATT_STAT_NORMAL;
			#endif
		} else {
			eventAlarm.f = volts;
			SendEvent(SENSOR_EVENT_BATTERY_BAD, eventAlarm);
			Flags_Set(FLAG_LOW_BATTERY_ALARM, 1);
			#ifdef CONFIG_LCZ_LWM2M_CLIENT
			battery_status = LCZ_LWM2M_CLIENT_DEV_BATT_STAT_LOW;
			#endif
		}
		#ifdef CONFIG_LCZ_LWM2M_CLIENT
		/* Update Object 3 in either case */
		(void)update_lwm2m_battery(battery_status, volts);
		#endif
	}
	return r;
}

int attr_prepare_analog_input_1(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_1, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int attr_prepare_analog_input_2(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_2, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int attr_prepare_analog_input_3(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_3, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int attr_prepare_analog_input_4(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_4, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int attr_prepare_temperature_result_1(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_1, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int attr_prepare_temperature_result_2(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_2, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int attr_prepare_temperature_result_3(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_3, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int attr_prepare_temperature_result_4(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_4, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int attr_prepare_digital_input(void)
{
	UpdateDin1();
	UpdateDin2();
	return 0;
}

#ifdef CONFIG_LOG
void SensorTask_GetTimeString(uint8_t *time_string)
{
	struct tm *tm;
	uint32_t savedRTCSeconds = 0;
	time_t time;

	attr_get(ATTR_ID_qrtc_last_set, &savedRTCSeconds,
		 sizeof(savedRTCSeconds));
	time = (time_t)(savedRTCSeconds);

	tm = gmtime(&time);
	snprintf(time_string, SENSOR_TASK_RTC_TIMESTAMP_SIZE, "%.2d:%.2d:%.2d",
		 tm->tm_hour, tm->tm_min, tm->tm_sec);
}
#endif

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SensorTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	SensorTaskObj_t *pObj = (SensorTaskObj_t *)pArg1;
	int r;

	r = AdcBt6_Init();
	if (r < 0) {
		LOG_ERR("BT6 ADC module init error: %d", r);
	}

	LoadSensorConfiguration();

	SensorOutput1Control();
	SensorOutput2Control();

	attr_prepare_power_voltage();
	InitializeIntervalTimers();
	UpdateMagnet();

	/* The last message to process in this list will clear the input config
	 * changed flag and unblock access to telemetry objects if enabled.
	 */
	ClearInputConfigChangedFlag();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static DispatchResult_t
SensorTaskAttributeChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	attr_changed_msg_t *pAttrMsg = (attr_changed_msg_t *)pMsg;
	size_t i;
	bool updateAnalogInterval = false;
	uint8_t activeMode = 0;
	bool input_config_changed = false;

	for (i = 0; i < pAttrMsg->count; i++) {
		switch (pAttrMsg->list[i]) {
		case ATTR_ID_power_sense_interval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_READ_POWER);
			break;

		case ATTR_ID_temperature_sense_interval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_TEMPERATURE_MEASURE);
			break;
		case ATTR_ID_analog_sense_interval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_ANALOG_MEASURE);
			break;
		case ATTR_ID_digital_input_1_config:
		case ATTR_ID_digital_input_2_config:
			FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK,
						   FMC_DIGITAL_IN_CONFIG);
			input_config_changed = true;
			break;
		case ATTR_ID_thermistor_config:
			StartTemperatureInterval();
			input_config_changed = true;
			break;
		case ATTR_ID_analog_input_1_type:
		case ATTR_ID_analog_input_2_type:
		case ATTR_ID_analog_input_3_type:
		case ATTR_ID_analog_input_4_type:
			updateAnalogInterval = true;
			input_config_changed = true;
			break;
		case ATTR_ID_config_type:
			SensorConfigChange(false);
			input_config_changed = true;
			break;

		case ATTR_ID_qrtc_last_set:
			printRTCTime();
			/* RTC was set by external device */
			Flags_Set(FLAG_TIME_WAS_SET, 1);
			break;

		case ATTR_ID_digital_output_1_state:
			SensorOutput1Control();
			break;
		case ATTR_ID_digital_output_2_state:
			SensorOutput2Control();
			break;
		case ATTR_ID_active_mode:
			/* This is to handle direct calls from remote clients
			 * but also when set via the local interfaces. In the
			 * case of local interfaces, it only ever gets set to
			 * true, and so always routed to the Enter Active
			 * handler.
			 */
			attr_get(ATTR_ID_active_mode, &activeMode,
				      sizeof(activeMode));
			FRAMEWORK_MSG_CREATE_AND_SEND(
				FWK_ID_SENSOR_TASK, FWK_ID_SENSOR_TASK,
				(!activeMode ? FMC_ENTER_SHELF_MODE :
						     FMC_ENTER_ACTIVE_MODE));
			break;
		default:
			/* Don't do anything - this is a broadcast */
			break;
		}
	}
	if (updateAnalogInterval == true) {
		StartAnalogInterval();
	}
	if (input_config_changed == true) {
		(void)attr_set_bool(ATTR_ID_input_config_changed, true);
	}

	return DISPATCH_OK;
}

static DispatchResult_t
SensorTaskDigitalInAlarmMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	DigitalInMsg_t *pSensorMsg = (DigitalInMsg_t *)pMsg;
	SensorEventData_t eventAlarm;
	uint32_t digitalAlarm = 0;

	if ((pSensorMsg->pin == DIN1_MCU_PIN) &&
	    (sensorTaskObject.digitalIn1Enabled == 1)) {
		UpdateDin1();
		/* Alarm from interrupt */
	} else if ((pSensorMsg->pin == DIN2_MCU_PIN) &&
		   (sensorTaskObject.digitalIn2Enabled == 1)) {
		UpdateDin2();
	}

	eventAlarm.u32 = digitalAlarm;
	SendEvent(SENSOR_EVENT_DIGITAL_ALARM, eventAlarm);

	return DISPATCH_OK;
}

static DispatchResult_t
SensorTaskDigitalInConfigMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	uint8_t input1Config = 0;
	uint8_t input2Config = 0;
	gpio_flags_t edgeTypeDin1;
	gpio_flags_t edgeTypeDin2;

	/* Digital Input 1 */
	attr_get(ATTR_ID_digital_input_1_config, &input1Config,
		 sizeof(input1Config));
	sensorTaskObject.input1Alarm = input1Config & DIGITAL_IN_ALARM_MASK;
	input1Config = (input1Config & DIGITAL_IN_ENABLE_MASK);
	input1Config = input1Config >> DIGITAL_IN_BIT_SHIFT;
	sensorTaskObject.digitalIn1Enabled = input1Config;
	edgeTypeDin1 = GetEdgeType(sensorTaskObject.input1Alarm);
	if (sensorTaskObject.digitalIn1Enabled == 1) {
		BSP_ConfigureDigitalInputs(DIN1_MCU_PIN, GPIO_INPUT,
					   edgeTypeDin1);
		BSP_PinSet(DIN1_ENABLE_PIN, 1);
	} else {
		BSP_ConfigureDigitalInputs(DIN1_MCU_PIN, GPIO_DISCONNECTED,
					   edgeTypeDin1);
		BSP_PinSet(DIN1_ENABLE_PIN, 0);
	}

	/* Digital Input 2 */
	attr_get(ATTR_ID_digital_input_2_config, &input2Config,
		 sizeof(input2Config));
	sensorTaskObject.input2Alarm = input2Config & DIGITAL_IN_ALARM_MASK;
	input2Config = (input2Config & DIGITAL_IN_ENABLE_MASK);
	input2Config = input2Config >> DIGITAL_IN_BIT_SHIFT;
	sensorTaskObject.digitalIn2Enabled = input2Config;
	edgeTypeDin2 = GetEdgeType(sensorTaskObject.input2Alarm);
	if (sensorTaskObject.digitalIn2Enabled == 1) {
		BSP_ConfigureDigitalInputs(DIN2_MCU_PIN, GPIO_INPUT,
					   edgeTypeDin2);
		BSP_PinSet(DIN2_ENABLE_PIN, 1);
	} else {
		BSP_ConfigureDigitalInputs(DIN2_MCU_PIN, GPIO_DISCONNECTED,
					   edgeTypeDin2);
		BSP_PinSet(DIN2_ENABLE_PIN, 0);
	}

	return DISPATCH_OK;
}

static DispatchResult_t MagnetStateMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	UpdateMagnet();

	return DISPATCH_OK;
}

static DispatchResult_t ReadPowerMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					    FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ARG_UNUSED(pMsgRxer);
	attr_prepare_power_voltage();
	StartPowerInterval();

	return DISPATCH_OK;
}

static DispatchResult_t MeasureTemperatureMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ARG_UNUSED(pMsgRxer);
	size_t index = 0;
	int r;
	float temperature;

	for (index = 0; index < TOTAL_THERM_CH; index++) {
		r = MeasureThermistor(index, ADC_PWR_SEQ_SINGLE, &temperature);
		if (r == 0) {
			SendEvent((SensorEventType_t)(SENSOR_EVENT_TEMPERATURE_1 + index),
				(SensorEventData_t)temperature);
			(void)update_lwm2m_temperature(index, temperature);
		}
	}
	StartTemperatureInterval();

	return DISPATCH_OK;
}

static DispatchResult_t AnalogReadMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					     FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ARG_UNUSED(pMsgRxer);
	size_t index = 0;
	int r;
	float analogValue;

	for (index = 0; index < TOTAL_ANALOG_CH; index++) {
		r = MeasureAnalogInput(index, ADC_PWR_SEQ_SINGLE, &analogValue);
		if (r == 0) {
			SendEvent(AnalogConfigType(index), (SensorEventData_t)analogValue);
		}
	}
	StartAnalogInterval();

	return DISPATCH_OK;
}

static DispatchResult_t EnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);

	/* This handler triggers a change to 1M PHY. This
	 * may be ignored if a connection is already active.
	 */
	Flags_Set(FLAG_ACTIVE_MODE, 1);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_BLE_TASK,
				      FMC_ENTER_ACTIVE_MODE);

	/* Start sensor reading */
	StartPowerInterval();
	StartTemperatureInterval();
	StartAnalogInterval();

	return DISPATCH_OK;
}

static DispatchResult_t EnterShelfModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);

	/* The BT610 needs to perform a reset after leaving
	 * active mode to enter shelf mode. This is so it can
	 * disable broadcasting and go through the start up
	 * sequence where it advertises in 1M then disables
	 * advertising altogether.
	 */
	Flags_Set(FLAG_ACTIVE_MODE, 0);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_CONTROL_TASK,
				      FMC_SOFTWARE_RESET);

	LOG_WRN("Entering shelf mode");

	return DISPATCH_OK;
}

static DispatchResult_t ClearInputConfigChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						 FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);

	/* Clear the input config changed flag here - it will have been set
	 * during start-up, but only changes post start-up should be taken
	 * into account.
	 */
	(void)attr_set_bool(ATTR_ID_input_config_changed, false);

	return DISPATCH_OK;
}

static void LoadSensorConfiguration(void)
{
	SensorConfigChange(true);
	FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK, FMC_DIGITAL_IN_CONFIG);
}

static void ClearInputConfigChangedFlag(void)
{
	FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK, FMC_CLEAR_INPUT_CONFIG_CHANGED);
}

static void SensorConfigChange(bool bootup)
{
	uint32_t configurationType;
	attr_copy_uint32(&configurationType, ATTR_ID_config_type);

	if ((bootup == false) || (configurationType == ANALOG_INPUT_1_TYPE_UNUSED)) {
		/* Disable all the thermistors */
		DisableThermistorReadings();
		/* Disable all analogs */
		DisableAnalogReadings();
		/* Disable Digital */
		DisableDigitalIO();
	}
}

static void SensorOutput1Control(void)
{
	uint8_t outputStatus = 0;
	attr_get(ATTR_ID_digital_output_1_state, &outputStatus,
		 sizeof(outputStatus));

	BSP_PinSet(DO1_PIN, (outputStatus));
}

static void SensorOutput2Control(void)
{
	uint8_t outputStatus = 0;
	attr_get(ATTR_ID_digital_output_2_state, &outputStatus,
		 sizeof(outputStatus));

	BSP_PinSet(DO2_PIN, (outputStatus));
}

#ifdef CONFIG_LOG
static void printRTCTime(void)
{
	uint8_t time_string[SENSOR_TASK_RTC_TIMESTAMP_SIZE];

	SensorTask_GetTimeString(time_string);
	LOG_DBG("Time = %s", time_string);
}
#endif

static void UpdateDin1(void)
{
	/* Check if the input is enabled first */
	int v = BSP_PinGet(DIN1_MCU_PIN);

	if (v >= 0) {
		attr_set_mask32(ATTR_ID_digital_input, 0, v);
		Flags_Set(FLAG_DIGITAL_IN1_STATE, v);
	}
}

static void UpdateDin2(void)
{
	int v = BSP_PinGet(DIN2_MCU_PIN);

	if (v >= 0) {
		attr_set_mask32(ATTR_ID_digital_input, 1, v);
		Flags_Set(FLAG_DIGITAL_IN2_STATE, v);
	}
}

static void DisableDigitalIO(void)
{
	/* Disable the digital inputs */
	attr_set_uint32(ATTR_ID_digital_input_1_config,
			DIGITAL_IN_DISABLE_MASK);
	attr_set_uint32(ATTR_ID_digital_input_2_config,
			DIGITAL_IN_DISABLE_MASK);

	/* Disable the digital outputs */
	BSP_PinSet(DO1_PIN, (0));
	BSP_PinSet(DO2_PIN, (0));
}

static gpio_flags_t GetEdgeType(digitalAlarm_t alarm)
{
	gpio_flags_t edgeType = GPIO_INT_DISABLE;
	switch (alarm) {
	case NO_ALARM:
		edgeType = GPIO_INT_DISABLE;
		break;
	case FALLING_EDGE_ALARM:
		edgeType = GPIO_INT_EDGE_FALLING;
		break;
	case RISING_EDGE_ALARM:
		edgeType = GPIO_INT_EDGE_RISING;
		break;
	case BOTH_EDGE_ALARM:
		edgeType = GPIO_INT_EDGE_BOTH;
		break;
	default:
		edgeType = GPIO_INT_DISABLE;
		break;
	}
	return edgeType;
}

static void UpdateMagnet(void)
{
	int v = BSP_PinGet(MAGNET_MCU_PIN);

	if (v >= 0) {
		/* Far = 0 and Near = 1. The mag switch
		 * used to sense the magnet is set to a logic 1
		 * when no magnet is present, and a logic 0 when
		 * a magnet is applied. Both values set below
		 * are inverted so these map correctly.
		 */
		bool magnet_state = !((bool)(v));
		attr_set_uint32(ATTR_ID_magnet_state, magnet_state);
		Flags_Set(FLAG_MAGNET_STATE, (int)magnet_state);
	}
}

static void InitializeIntervalTimers(void)
{
	/* Power Interval timer */
	k_timer_init(&powerTimer, powerTimerCallbackIsr, NULL);
	StartPowerInterval();

	/* Temperture Interval timer */
	k_timer_init(&temperatureReadTimer, temperatureReadTimerCallbackIsr,
		     NULL);
	StartTemperatureInterval();

	/* Analog Interval timer */
	k_timer_init(&analogReadTimer, analogReadTimerCallbackIsr, NULL);
	StartAnalogInterval();
}

static void StartAnalogInterval(void)
{
	uint8_t analog1ConfigEnable;
	uint8_t analog2ConfigEnable;
	uint8_t analog3ConfigEnable;
	uint8_t analog4ConfigEnable;
	uint8_t activeModeStatus = 0;
	uint32_t analogTimer = 0;

	attr_get(ATTR_ID_active_mode, &activeModeStatus,
		 sizeof(activeModeStatus));

	attr_get(ATTR_ID_analog_input_1_type, &analog1ConfigEnable,
		 sizeof(analog1ConfigEnable));
	attr_get(ATTR_ID_analog_input_2_type, &analog2ConfigEnable,
		 sizeof(analog2ConfigEnable));
	attr_get(ATTR_ID_analog_input_3_type, &analog3ConfigEnable,
		 sizeof(analog3ConfigEnable));
	attr_get(ATTR_ID_analog_input_4_type, &analog4ConfigEnable,
		 sizeof(analog4ConfigEnable));
	/* Check if the timer is already running */
	analogTimer = k_timer_remaining_get(&analogReadTimer);

	if ((activeModeStatus == true) && (analogTimer == 0) &&
	    ((analog1ConfigEnable > 0) || (analog2ConfigEnable > 0) ||
	     (analog3ConfigEnable > 0) || (analog4ConfigEnable > 0))) {
		uint32_t intervalSeconds = 0;
		attr_copy_uint32(&intervalSeconds,
				 ATTR_ID_analog_sense_interval);
		if (intervalSeconds != 0) {
			k_timer_start(&analogReadTimer,
				      K_SECONDS(intervalSeconds), K_NO_WAIT);
		}
	}
}

static void StartTemperatureInterval(void)
{
	uint8_t thermConfigEnable;
	uint8_t activeModeStatus = 0;
	uint32_t tempTimer = 0;

	attr_get(ATTR_ID_active_mode, &activeModeStatus,
		 sizeof(activeModeStatus));
	attr_get(ATTR_ID_thermistor_config, &thermConfigEnable,
		 sizeof(thermConfigEnable));

	/* Check if the timer is already running */
	tempTimer = k_timer_remaining_get(&temperatureReadTimer);
	if ((activeModeStatus == true) && (thermConfigEnable > 0) &&
	    (tempTimer == 0)) {
		uint32_t intervalSeconds = 0;
		attr_copy_uint32(&intervalSeconds,
				 ATTR_ID_temperature_sense_interval);
		if (intervalSeconds != 0) {
			k_timer_start(&temperatureReadTimer,
				      K_SECONDS(intervalSeconds), K_NO_WAIT);
		}
	}
}

static void StartPowerInterval(void)
{
	uint8_t activeModeStatus = 0;

	attr_get(ATTR_ID_active_mode, &activeModeStatus,
		 sizeof(activeModeStatus));

	if (activeModeStatus == true) {
		uint32_t intervalSeconds = 0;
		attr_copy_uint32(&intervalSeconds,
				 ATTR_ID_power_sense_interval);
		if (intervalSeconds != 0) {
			k_timer_start(&powerTimer, K_SECONDS(intervalSeconds),
				      K_NO_WAIT);
		} else {
			k_timer_stop(&powerTimer);
		}
	}
}

static void DisableAnalogReadings(void)
{
	attr_set_uint32(ATTR_ID_analog_input_1_type, ANALOG_INPUT_1_TYPE_UNUSED);
	attr_set_uint32(ATTR_ID_analog_input_2_type, ANALOG_INPUT_1_TYPE_UNUSED);
	attr_set_uint32(ATTR_ID_analog_input_3_type, ANALOG_INPUT_1_TYPE_UNUSED);
	attr_set_uint32(ATTR_ID_analog_input_4_type, ANALOG_INPUT_1_TYPE_UNUSED);
	/* Turn off the timer */
	k_timer_stop(&analogReadTimer);
}

static void DisableThermistorReadings(void)
{
	uint32_t thermistorsConfig = 0;
	attr_set_uint32(ATTR_ID_thermistor_config, thermistorsConfig);
	/* Turn off the timer */
	k_timer_stop(&temperatureReadTimer);
}

static int MeasureAnalogInput(size_t channel, AdcPwrSequence_t power,
			      float *result)
{
	int r = -EPERM;
	int16_t raw = 0;
	*result = 0.0;

	/* Setup the AIN SEL pins on the multiplexer for the Analog pin config */
	r = AdcBt6_ConfigAinSelects();
	if (r == 0) {
		attr_id_t base = ATTR_ID_analog_input_1_type;
		enum analog_input_1_type config = attr_get_uint32(base +
								  channel, 0);

		switch (config) {
		case ANALOG_INPUT_1_TYPE_VOLTAGE_0V_TO_10V_DC:
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertVoltage(channel, raw);
				/* Convert to miliVolts */
				*result = *result * 1000;
			}
			break;

		case ANALOG_INPUT_1_TYPE_CURRENT_4MA_TO_20MA:
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_CURRENT,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertCurrent(channel, raw);
				(void)update_lwm2m_current(channel, *result);
			}
			break;

		case ANALOG_INPUT_1_TYPE_PRESSURE:
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_PRESSURE,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertPressure(channel, raw);
				(void)update_lwm2m_pressure(channel, *result);
			}
			break;

		case ANALOG_INPUT_1_TYPE_ULTRASONIC:
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_ULTRASONIC,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertUltrasonic(channel, raw);
				(void)update_lwm2m_fill_level(channel, *result);
			}
			break;

		case ANALOG_INPUT_1_TYPE_AC_CURRENT_20A:
			/* Configured for a voltage measurement */
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
			if (r >= 0) {
				*result = AdcBt6_ConvertACCurrent20(channel, raw);
				(void)update_lwm2m_current(channel, *result);
			}
			break;

		case ANALOG_INPUT_1_TYPE_AC_CURRENT_150A:
			/* Configured for a voltage measurement */
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertACCurrent150(channel, raw);
				(void)update_lwm2m_current(channel, *result);
			}
			break;

		case ANALOG_INPUT_1_TYPE_AC_CURRENT_500A:
			/* Configured for a voltage measurement */
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
			if (r >= 0) {
				*result = AdcBt6_ConvertACCurrent500(channel, raw);
				(void)update_lwm2m_current(channel, *result);
			}
			break;
		default:
			LOG_DBG("Analog input channel %d disabled", channel + 1);
			r = -ENODEV;
			break;
		}

		if (r >= 0) {
			r = attr_set_float(ATTR_ID_analog_input_1 + channel, *result);
		}
	} else {
		/* Shouldn't get into this failure state */
		LOG_ERR("AIN SEL Pin Failure");
	}
	return r;
}

static int MeasureThermistor(size_t channel, AdcPwrSequence_t power,
			     float *result)
{
	int r = -EPERM;
	int16_t raw = 0;
	*result = 0.0;

	uint32_t config = attr_get_uint32(ATTR_ID_thermistor_config, 0);

	if (config & BIT(channel)) {
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_THERMISTOR, power);
		if (r >= 0) {
			*result =
				AdcBt6_ConvertThermToTemperature(channel, raw);
		}
	} else {
		LOG_DBG("Thermistor channel %d not enabled", channel + 1);
		r = -ENODEV;
	}

	if (r >= 0) {
		r = attr_set_float(ATTR_ID_temperature_result_1 + channel,
				   *result);
	}

	return r;
}

static void SendEvent(SensorEventType_t type, SensorEventData_t data)
{
	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_EVENT_TRIGGER;
		pMsgSend->header.txId = FWK_ID_SENSOR_TASK;
		pMsgSend->header.rxId = FWK_ID_EVENT_TASK;
		pMsgSend->eventType = type;
		pMsgSend->eventData = data;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
}

static SensorEventType_t AnalogConfigType(size_t channel)
{
	enum analog_input_1_type configType;
	SensorEventType_t eventTypeReturn;
	attr_get((ATTR_ID_analog_input_1_type + channel), &configType,
		  sizeof(uint8_t));

	switch (configType) {
	case ANALOG_INPUT_1_TYPE_VOLTAGE_0V_TO_10V_DC:
		eventTypeReturn = SENSOR_EVENT_VOLTAGE_1 + channel;
		break;
	case ANALOG_INPUT_1_TYPE_CURRENT_4MA_TO_20MA:
		eventTypeReturn = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	case ANALOG_INPUT_1_TYPE_PRESSURE:
	case ANALOG_INPUT_1_TYPE_ULTRASONIC:
		eventTypeReturn = SENSOR_EVENT_ULTRASONIC_1 + (channel);
		break;
	case ANALOG_INPUT_1_TYPE_AC_CURRENT_20A:
	case ANALOG_INPUT_1_TYPE_AC_CURRENT_150A:
	case ANALOG_INPUT_1_TYPE_AC_CURRENT_500A:
		eventTypeReturn = SENSOR_EVENT_CURRENT_1 + channel;
		break;
	default:
		/*should not get here but just in case still return something*/
		eventTypeReturn = SENSOR_EVENT_VOLTAGE_1;
		break;
	}
	return (eventTypeReturn);
}

static int update_lwm2m_temperature(int index, float temperature)
{
	int r = 0;

	#ifdef CONFIG_LWM2M_IPSO_TEMP_SENSOR
	bool input_config_changed;

	input_config_changed = attr_get_bool(ATTR_ID_input_config_changed);

	/* Block update of telemetry objects until configuration is stable */
	if (input_config_changed == false) {
		r = lcz_lwm2m_temperature_set(index, (double)temperature);
		if (r < 0) {
			LOG_ERR("Could not set temperature instance %d [%d]",
					index, r);
		}
	}
	#endif
	return(r);
}

static int update_lwm2m_pressure(int index, float pressure)
{
	int r = 0;

	#ifdef CONFIG_LWM2M_IPSO_PRESSURE_SENSOR
	bool input_config_changed;

	input_config_changed = attr_get_bool(ATTR_ID_input_config_changed);

	/* Block update of telemetry objects until input configuration is stable */
	if (input_config_changed == false) {
		r = lcz_lwm2m_pressure_set(index, (double)pressure);
		if (r < 0) {
			LOG_ERR("Could not set pressure instance %d [%d]", index, r);
		}
	}
	#endif
	return(r);
}

static int update_lwm2m_fill_level(int index, float fill_level)
{
	int r = 0;

	ARG_UNUSED(index);

	#ifdef CONFIG_LWM2M_IPSO_FILLING_SENSOR
	bool input_config_changed;

	input_config_changed = attr_get_bool(ATTR_ID_input_config_changed);

	/* Block telemetry updates if the input config has changed */
	if (input_config_changed == false) {
		/* Only one ultrasonic sensor permitted */
		r = lcz_lwm2m_fill_level_set(index, (double)fill_level);
		if (r < 0) {
			LOG_ERR("Could not set filling sensor instance %d [%d]", index, r);
		}
	}
	#endif
	return(r);
}

static int update_lwm2m_current(int index, float current)
{
	int r = 0;

	#ifdef CONFIG_LWM2M_IPSO_CURRENT_SENSOR
	bool input_config_changed;

	input_config_changed = attr_get_bool(ATTR_ID_input_config_changed);

	/* Don't update telemetry objects if the input configuration has changed */
	if (input_config_changed == false) {
		r = lcz_lwm2m_current_set(index, (double)current);
		if (r < 0) {
			LOG_ERR("Could not set current sensor instance %d [%d]",
					index, r);
		}
	}
	#endif
	return(r);
}

static int update_lwm2m_battery(lcz_lwm2m_client_device_battery_status_t status, float voltage)
{
	int r = 0;

	#ifdef CONFIG_LCZ_LWM2M_CLIENT
	static int32_t pwr_src_mv;
	static lcz_lwm2m_client_device_power_source_t pwr_src = LCZ_LWM2M_CLIENT_DEV_PWR_SRC_INT_BATT;

	/* Update power source */
	r = lcz_lwm2m_client_set_available_power_source(0, &pwr_src);
	if (r < 0) {
		LOG_ERR("Could not set power source [%d]", r);
	}

	/* Update device errors if needed */
	if (status == LCZ_LWM2M_CLIENT_DEV_BATT_STAT_LOW) {
		r = lcz_lwm2m_client_device_set_err(LWM2M_DEVICE_ERROR_LOW_POWER);
		if (r < 0) {
			LOG_ERR("Could not set device error [%d]", r);
		}
	}

	/* Update battery state */
	r = lcz_lwm2m_client_set_battery_status(&status);
	if (r < 0) {
		LOG_ERR("Could not set battery state [%d]", r);
	}

	/* Update battery voltage in millivolts */
	pwr_src_mv = (int32_t)(voltage * 1000.0);
	r = lcz_lwm2m_client_set_power_source_voltage(0, &pwr_src_mv);
	if (r < 0) {
		LOG_ERR("Could not set battery voltage [%d]", r);
	}
	#endif
	return(r);
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void powerTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_SENSOR_TASK,
				      FMC_READ_POWER);
}

static void temperatureReadTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_SENSOR_TASK,
				      FMC_TEMPERATURE_MEASURE);
}

static void analogReadTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_SENSOR_TASK,
				      FMC_ANALOG_MEASURE);
}
