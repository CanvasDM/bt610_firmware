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
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <drivers/spi.h>

#include "FrameworkIncludes.h"
#include "Attribute.h"
#include "BspSupport.h"
#include "AdcBt6.h"
#include "AnalogInput.h"
#include "AlarmControl.h"
#include "SensorTask.h"
#include "Flags.h"
#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "AggregationCount.h"

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

#define BATTERY_BAD_VOLTAGE (3000)
#define DIGITAL_IN_ALARM_MASK (0x03)
#define DIGITAL_IN_ENABLE_MASK (0x80)
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

	float previousTemp[TOTAL_THERM_CH];
	float magnitudeOfTempDifference[TOTAL_THERM_CH];
	float previousAnalogValue[TOTAL_ANALOG_CH];
	float magnitudeOfAnalogDifference[TOTAL_ANALOG_CH];
} SensorTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static SensorTaskObj_t sensorTaskObject;
static struct k_timer batteryTimer;
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
static DispatchResult_t ReadBatteryMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg);
static DispatchResult_t MeasureTemperatureMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg);
static DispatchResult_t AnalogReadMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					     FwkMsg_t *pMsg);
static DispatchResult_t EnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg);
static void LoadSensorConfiguration(void);
static void SensorConfigChange(bool bootup);
static void SensorOutput1Control(void);
static void SensorOutput2Control(void);

static void UpdateDin1(void);
static void UpdateDin2(void);
static void DisableDigitalIO(void);
static gpio_flags_t GetEdgeType(digitalAlarm_t alarm);
static void UpdateMagnet(void);
static void InitializeIntervalTimers(void);
static void StartAnalogInterval(void);
static void StartTemperatureInterval(void);
static void StartBatteryInterval(void);
static void DisableAnalogPins(void);
static void DisableThermistorPins(void);

static int MeasureAnalogInput(size_t channel, AdcPwrSequence_t power,
			      float *result);
static int MeasureThermistor(size_t channel, AdcPwrSequence_t power,
			     float *result);
static void SendEvent(SensorEventType_t type, SensorEventData_t data);

static void batteryTimerCallbackIsr(struct k_timer *timer_id);
static void temperatureReadTimerCallbackIsr(struct k_timer *timer_id);
static void analogReadTimerCallbackIsr(struct k_timer *timer_id);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t SensorTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:             return Framework_UnknownMsgHandler;
	case FMC_ATTR_CHANGED:        return SensorTaskAttributeChangedMsgHandler;
	case FMC_DIGITAL_IN:          return SensorTaskDigitalInAlarmMsgHandler;
	case FMC_DIGITAL_IN_CONFIG:   return SensorTaskDigitalInConfigMsgHandler;
	case FMC_MAGNET_STATE:        return MagnetStateMsgHandler;
	case FMC_READ_BATTERY:        return ReadBatteryMsgHandler;
	case FMC_TEMPERATURE_MEASURE: return MeasureTemperatureMsgHandler;
	case FMC_ANALOG_MEASURE:      return AnalogReadMsgHandler;
	case FMC_ENTER_ACTIVE_MODE:   return EnterActiveModeMsgHandler;	
	default:                      return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SensorTask_Initialize(void)
{
	sensorTaskObject.msgTask.rxer.id = FWK_ID_SENSOR_TASK;
	sensorTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	sensorTaskObject.msgTask.rxer.pMsgDispatcher = SensorTaskMsgDispatcher;
	sensorTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	sensorTaskObject.msgTask.timerPeriodTicks = K_MSEC(0); // 0 for one shot
	sensorTaskObject.msgTask.rxer.pQueue = &sensorTaskQueue;

	sensorTaskObject.digitalIn1Enabled = NO_ALARM;
	sensorTaskObject.digitalIn2Enabled = NO_ALARM;
	sensorTaskObject.input1Alarm = 0;
	sensorTaskObject.input2Alarm = 0;
	memset(sensorTaskObject.previousTemp, 0, TOTAL_THERM_CH);
	memset(sensorTaskObject.magnitudeOfTempDifference, 0, TOTAL_THERM_CH);
	memset(sensorTaskObject.previousAnalogValue, 0, TOTAL_ANALOG_CH);
	memset(sensorTaskObject.magnitudeOfAnalogDifference, 0,
	       TOTAL_ANALOG_CH);

	Framework_RegisterTask(&sensorTaskObject.msgTask);

	sensorTaskObject.msgTask.pTid =
		k_thread_create(&sensorTaskObject.msgTask.threadData,
				sensorTaskStack,
				K_THREAD_STACK_SIZEOF(sensorTaskStack),
				SensorTaskThread, &sensorTaskObject, NULL, NULL,
				SENSOR_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(sensorTaskObject.msgTask.pTid, THIS_FILE);
}

int AttributePrepare_batteryVoltageMv(void)
{
	int16_t raw = 0;
	int32_t mv = 0;
	SensorEventData_t eventAlarm;
	int r = AdcBt6_ReadBatteryMv(&raw, &mv);

	if (r >= 0) {
		r = Attribute_SetSigned32(ATTR_INDEX_batteryVoltageMv, mv);
		if (mv > BATTERY_BAD_VOLTAGE) {
			eventAlarm.s32 = mv;
			SendEvent(SENSOR_EVENT_BATTERY_GOOD, eventAlarm);

			Flags_Set(FLAG_LOW_BATTERY_ALARM, 0);
		} else {
			eventAlarm.s32 = mv;
			SendEvent(SENSOR_EVENT_BATTERY_BAD, eventAlarm);
			Flags_Set(FLAG_LOW_BATTERY_ALARM, 1);
		}
	}
	return r;
}

int AttributePrepare_analogInput1(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_1, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int AttributePrepare_analogInput2(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_2, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int AttributePrepare_analogInput3(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_3, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int AttributePrepare_analogInput4(void)
{
	float dummyResult;
	return MeasureAnalogInput(ANALOG_CH_4, ADC_PWR_SEQ_SINGLE,
				  &dummyResult);
}

int AttributePrepare_temperatureResult1(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_1, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int AttributePrepare_temperatureResult2(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_2, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int AttributePrepare_temperatureResult3(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_3, ADC_PWR_SEQ_SINGLE, &dummyResult);
}

int AttributePrepare_temperatureResult4(void)
{
	float dummyResult;
	return MeasureThermistor(THERM_CH_4, ADC_PWR_SEQ_SINGLE, &dummyResult);
}
int AttributePrepare_digitalInput(void)
{
	UpdateDin1();
	UpdateDin2();
	return 0;
}

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

	AttributePrepare_batteryVoltageMv();
	InitializeIntervalTimers();
	UpdateMagnet();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static DispatchResult_t
SensorTaskAttributeChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	AttrChangedMsg_t *pAttrMsg = (AttrChangedMsg_t *)pMsg;
	size_t i;
	bool updateAnalogInterval = false;
	for (i = 0; i < pAttrMsg->count; i++) {
		switch (pAttrMsg->list[i]) {
		case ATTR_INDEX_batterySenseInterval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_READ_BATTERY);
			break;

		case ATTR_INDEX_temperatureSenseInterval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_TEMPERATURE_MEASURE);
			break;
		case ATTR_INDEX_analogSenseInterval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_ANALOG_MEASURE);
			break;
		case ATTR_INDEX_digitalInput1Config:
		case ATTR_INDEX_digitalInput2Config:
			FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK,
						   FMC_DIGITAL_IN_CONFIG);
			break;
		case ATTR_INDEX_analogInput1Type:
		case ATTR_INDEX_analogInput2Type:
		case ATTR_INDEX_analogInput3Type:
		case ATTR_INDEX_analogInput4Type:
			updateAnalogInterval = true;
			break;
		case ATTR_INDEX_configType:
			SensorConfigChange(false);
			break;

		case ATTR_INDEX_digitalOutput1State:
			SensorOutput1Control();
			break;
		case ATTR_INDEX_digitalOutput2State:
			SensorOutput2Control();
			break;
		case ATTR_INDEX_activeMode:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_ENTER_ACTIVE_MODE);
		case ATTR_INDEX_settingsPasscode:
		default:
			// don't do anything - this is a broadcast
			break;
		}
	}
	if (updateAnalogInterval == true) {
		StartAnalogInterval();
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
		/*Alarm from interrupt*/
		Attribute_SetMask32(ATTR_INDEX_digitalAlarms, 0, 1);

	} else if ((pSensorMsg->pin == DIN2_MCU_PIN) &&
		   (sensorTaskObject.digitalIn2Enabled == 1)) {
		UpdateDin2();
		Attribute_SetMask32(ATTR_INDEX_digitalAlarms, 1, 1);
	}
	Attribute_Get(ATTR_INDEX_digitalAlarms, &digitalAlarm,
		      sizeof(digitalAlarm));
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

	/*Digital Input 1*/
	Attribute_Get(ATTR_INDEX_digitalInput1Config, &input1Config,
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

	/*Digital Input 2*/
	Attribute_Get(ATTR_INDEX_digitalInput2Config, &input2Config,
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
static DispatchResult_t ReadBatteryMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ARG_UNUSED(pMsgRxer);
	AttributePrepare_batteryVoltageMv();
	StartBatteryInterval();

	return DISPATCH_OK;
}
static DispatchResult_t MeasureTemperatureMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						     FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsg);
	ARG_UNUSED(pMsgRxer);
	uint8_t index = 0;
	uint8_t r;
	float temperature = 0.0;

	for (index = 0; index < TOTAL_THERM_CH; index++) {
		r = MeasureThermistor(index, ADC_PWR_SEQ_SINGLE, &temperature);
		if (r == 0) {
			HighTempAlarmCheck(index, temperature);
			LowTempAlarmCheck(index, temperature);
			DeltaTempAlarmCheck(
				index,
				sensorTaskObject
					.magnitudeOfTempDifference[index]);
			AggregationTempHandler(index, temperature);
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
	uint8_t index = 0;
	uint8_t r;
	float analogValue = 0.0;
	for (index = 0; index < TOTAL_ANALOG_CH; index++) {
		r = MeasureAnalogInput(index, ADC_PWR_SEQ_SINGLE, &analogValue);
		if (r == 0) {
			HighAnalogAlarmCheck(index, analogValue);
			LowAnalogAlarmCheck(index, analogValue);
			DeltaAnalogAlarmCheck(
				index,
				sensorTaskObject
					.magnitudeOfAnalogDifference[index]);
			AggregationAnalogHandler(index, analogValue);
		}
	}
	StartAnalogInterval();

	return DISPATCH_OK;
}

static DispatchResult_t EnterActiveModeMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						  FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	uint8_t activeModeStatus = 0;

	Attribute_Get(ATTR_INDEX_activeMode, &activeModeStatus,
		      sizeof(activeModeStatus));

	/*Button was pressed make sure device enters active mode*/
	if ((activeModeStatus == 0) &&
	    (pMsg->header.txId == FWK_ID_USER_IF_TASK)) {
		Attribute_SetUint32(ATTR_INDEX_activeMode, 1);
		activeModeStatus = 1;
		/*User set active mode back 0*/
	} else if ((activeModeStatus == 0) &&
		   (pMsg->header.txId == FWK_ID_SENSOR_TASK)) {
		/*The BT610 needs to perform reset after leaving active mode to enter shelf mode*/
		Flags_Set(FLAG_ACTIVE_MODE, 0);
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
					      FWK_ID_CONTROL_TASK,
					      FMC_SOFTWARE_RESET);
	}

	if (activeModeStatus == 1) {
		Flags_Set(FLAG_ACTIVE_MODE, 1);
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
					      FWK_ID_BLE_TASK,
					      FMC_BLE_START_ADVERTISING);
	}
	return DISPATCH_OK;
}

static void LoadSensorConfiguration(void)
{
	SensorConfigChange(true);
	//Attribute_SetUint32(ATTR_INDEX_activeMode, 1);
	FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK, FMC_DIGITAL_IN_CONFIG);
}

static void SensorConfigChange(bool bootup)
{
	uint32_t configurationType;
	Attribute_GetUint32(&configurationType, ATTR_INDEX_configType);

	/*Clear the Aggregation queue on event type change*/
	AggregationPurgeQueueHandler();
	
	switch (configurationType) {
	case CONFIG_UNDEFINED:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable all analogs*/
		DisableAnalogPins();

		/*Disable Digital*/
		DisableDigitalIO();
		break;
	case CONFIG_ANALOG_INPUT:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable Digital*/
		DisableDigitalIO();

		/*Disable all the Analog types, they are setup individually */
		if (bootup == false) {
			DisableAnalogPins();
		}
		break;
	case CONFIG_DIGITAL:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable all analogs*/
		DisableAnalogPins();

		/*Enable the digital inputs*/
		if (bootup == false) {
			Attribute_SetUint32(ATTR_INDEX_digitalInput1Config,
					    (DIGITAL_IN_ENABLE_MASK |
					     DIGITAL_IN_ALARM_MASK));
			Attribute_SetUint32(ATTR_INDEX_digitalInput2Config,
					    (DIGITAL_IN_ENABLE_MASK |
					     DIGITAL_IN_ALARM_MASK));
		}
		break;
	case CONFIG_TEMPERATURE:
		/*Enable all the thermistors */
		if (bootup == false) {
			Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
					    ALL_THERMISTORS);
			StartTemperatureInterval();
		}

		/*Disable all analogs*/
		DisableAnalogPins();

		/*Disable Digital*/
		DisableDigitalIO();

		break;
	case CONFIG_ANALOG_AC_CURRENT:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable Digital*/
		DisableDigitalIO();
		/*Disable all the Analog types, they are setup individually */
		if (bootup == false) {
			DisableAnalogPins();
		}
		break;
	case CONFIG_ULTRASONIC_PRESSURE:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable Digital*/
		DisableDigitalIO();

		/*Configure the first 3 analogs*/
		if (bootup == false) {
			Attribute_SetUint32(ATTR_INDEX_analogInput1Type,
					    ANALOG_ULTRASONIC);
			Attribute_SetUint32(ATTR_INDEX_analogInput2Type,
					    ANALOG_PRESSURE);
			Attribute_SetUint32(ATTR_INDEX_analogInput3Type,
					    ANALOG_PRESSURE);
			Attribute_SetUint32(ATTR_INDEX_analogInput4Type,
					    ANALOG_UNUSED);
		}

		break;
	case CONFIG_SPI_I2C:
		/*Disable all the thermistors*/
		DisableThermistorPins();

		/*Disable all analogs*/
		DisableAnalogPins();

		/*Disable Digital*/
		DisableDigitalIO();

		break;
	default:
		/*Set to undefined*/
		Attribute_SetUint32(ATTR_INDEX_configType, CONFIG_UNDEFINED);
		break;
	}
}
static void SensorOutput1Control(void)
{
	uint8_t outputStatus = 0;
	Attribute_Get(ATTR_INDEX_digitalOutput1State, &outputStatus,
		      sizeof(outputStatus));

	BSP_PinSet(DO1_PIN, (outputStatus));
}
static void SensorOutput2Control(void)
{
	uint8_t outputStatus = 0;
	Attribute_Get(ATTR_INDEX_digitalOutput2State, &outputStatus,
		      sizeof(outputStatus));

	BSP_PinSet(DO2_PIN, (outputStatus));
}
static void UpdateDin1(void)
{
	/*Check if the input is enabled first*/
	int v = BSP_PinGet(DIN1_MCU_PIN);

	if (v >= 0) {
		Attribute_SetMask32(ATTR_INDEX_digitalInput, 0, v);
		Flags_Set(FLAG_DIGITAL_IN1_STATE, v);
	}
}

static void UpdateDin2(void)
{
	int v = BSP_PinGet(DIN2_MCU_PIN);

	if (v >= 0) {
		Attribute_SetMask32(ATTR_INDEX_digitalInput, 1, v);
		Flags_Set(FLAG_DIGITAL_IN2_STATE, v);
	}
}
static void DisableDigitalIO(void)
{
	/*Disable the digital inputs*/
	Attribute_SetUint32(ATTR_INDEX_digitalInput1Config,
			    ((!DIGITAL_IN_ENABLE_MASK) |
			     DIGITAL_IN_ALARM_MASK));
	Attribute_SetUint32(ATTR_INDEX_digitalInput2Config,
			    ((!DIGITAL_IN_ENABLE_MASK) |
			     DIGITAL_IN_ALARM_MASK));

	/*Disable the digital outputs*/
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
		v = Attribute_SetUint32(ATTR_INDEX_magnetState, v);
		Flags_Set(FLAG_MAGNET_STATE, v);
	}
}
static void InitializeIntervalTimers(void)
{
	/*Battery Interval timer*/
	k_timer_init(&batteryTimer, batteryTimerCallbackIsr, NULL);
	StartBatteryInterval();

	/*Temperture Interval timer*/
	k_timer_init(&temperatureReadTimer, temperatureReadTimerCallbackIsr,
		     NULL);
	StartTemperatureInterval();

	/*Analog Interval timer*/
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

	Attribute_Get(ATTR_INDEX_activeMode, &activeModeStatus,
		      sizeof(activeModeStatus));

	Attribute_Get(ATTR_INDEX_analogInput1Type, &analog1ConfigEnable,
		      sizeof(analog1ConfigEnable));
	Attribute_Get(ATTR_INDEX_analogInput2Type, &analog2ConfigEnable,
		      sizeof(analog2ConfigEnable));
	Attribute_Get(ATTR_INDEX_analogInput3Type, &analog3ConfigEnable,
		      sizeof(analog3ConfigEnable));
	Attribute_Get(ATTR_INDEX_analogInput4Type, &analog4ConfigEnable,
		      sizeof(analog4ConfigEnable));

	if ((activeModeStatus == true) &&
	    ((analog1ConfigEnable > 0) || (analog2ConfigEnable > 0) ||
	     (analog3ConfigEnable > 0) || (analog4ConfigEnable > 0))) {
		uint32_t intervalSeconds = 0;
		Attribute_GetUint32(&intervalSeconds,
				    ATTR_INDEX_analogSenseInterval);
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

	Attribute_Get(ATTR_INDEX_activeMode, &activeModeStatus,
		      sizeof(activeModeStatus));
	Attribute_Get(ATTR_INDEX_thermistorConfig, &thermConfigEnable,
		      sizeof(thermConfigEnable));

	if ((activeModeStatus == true) && (thermConfigEnable > 0)) {
		uint32_t intervalSeconds = 0;
		Attribute_GetUint32(&intervalSeconds,
				    ATTR_INDEX_temperatureSenseInterval);
		if (intervalSeconds != 0) {
			k_timer_start(&temperatureReadTimer,
				      K_SECONDS(intervalSeconds), K_NO_WAIT);
		}
	}
}

static void StartBatteryInterval(void)
{
	uint8_t activeModeStatus = 0;

	Attribute_Get(ATTR_INDEX_activeMode, &activeModeStatus,
		      sizeof(activeModeStatus));

	if (activeModeStatus == true) {
		uint32_t intervalSeconds = 0;
		Attribute_GetUint32(&intervalSeconds,
				    ATTR_INDEX_batterySenseInterval);
		if (intervalSeconds != 0) {
			k_timer_start(&batteryTimer, K_SECONDS(intervalSeconds),
				      K_NO_WAIT);
		} else {
			k_timer_stop(&batteryTimer);
		}
	}
}

static void DisableAnalogPins(void)
{
	Attribute_SetUint32(ATTR_INDEX_analogInput1Type, ANALOG_UNUSED);
	Attribute_SetUint32(ATTR_INDEX_analogInput2Type, ANALOG_UNUSED);
	Attribute_SetUint32(ATTR_INDEX_analogInput3Type, ANALOG_UNUSED);
	Attribute_SetUint32(ATTR_INDEX_analogInput4Type, ANALOG_UNUSED);
}

static void DisableThermistorPins(void)
{
	uint32_t thermistorsConfig = 0;
	Attribute_SetUint32(ATTR_INDEX_thermistorConfig, thermistorsConfig);
}

static int MeasureAnalogInput(size_t channel, AdcPwrSequence_t power,
			      float *result)
{
	int r = -EPERM;
	int16_t raw = 0;
	*result = 0.0;

	attr_idx_t base = ATTR_INDEX_analogInput1Type;
	analogConfigType_t config = Attribute_AltGetUint32(base + channel, 0);

	switch (config) {
	case ANALOG_VOLTAGE:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertVoltage(channel, raw);
		}
		break;

	case ANALOG_CURRENT:
		r = AdcBt6_ConfigAinSelects();
		if (r == 0) {
			r = AdcBt6_Measure(&raw, channel, ADC_TYPE_CURRENT,
					   power);
			if (r >= 0) {
				*result = AdcBt6_ConvertCurrent(channel, raw);
			}
		}
		break;

	case ANALOG_PRESSURE:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_PRESSURE, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertPressure(channel, raw);
		}
		break;

	case ANALOG_ULTRASONIC:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_ULTRASONIC, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertUltrasonic(channel, raw);
		}
		break;
	case ANALOG_CURRENT20A:
		/*Configured for a voltage measurement*/
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertACCurrent20(channel, raw);
		}
		break;
	case ANALOG_CURRENT150A:
		/*Configured for a voltage measurement*/
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertACCurrent150(channel, raw);
		}
		break;
	case ANALOG_CURRENT500A:
		/*Configured for a voltage measurement*/
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
		if (r >= 0) {
			*result = AdcBt6_ConvertACCurrent500(channel, raw);
		}
		break;
	default:
		LOG_DBG("Analog input channel %d disabled", channel + 1);
		r = -ENODEV;
		break;
	}

	if (r >= 0) {
		r = Attribute_SetFloat(ATTR_INDEX_analogInput1 + channel,
				       *result);

		sensorTaskObject.magnitudeOfAnalogDifference[channel] =
			abs(*result -
			    sensorTaskObject.previousAnalogValue[channel]);
		sensorTaskObject.previousAnalogValue[channel] = *result;
	}

	return r;
}

static int MeasureThermistor(size_t channel, AdcPwrSequence_t power,
			     float *result)
{
	int r = -EPERM;
	int16_t raw = 0;
	*result = 0.0;

	uint32_t config =
		Attribute_AltGetUint32(ATTR_INDEX_thermistorConfig, 0);

	if (config & BIT(channel)) {
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_THERMISTOR, power);
		if (r >= 0) {
			*result =
				AdcBt6_ConvertThermToTemperature(raw, channel);

			sensorTaskObject.magnitudeOfTempDifference[channel] =
				abs(*result -
				    sensorTaskObject.previousTemp[channel]);
			sensorTaskObject.previousTemp[channel] = *result;
		}
	} else {
		LOG_DBG("Thermistor channel %d not enabled", channel + 1);
		r = -ENODEV;
	}

	if (r >= 0) {
		r = Attribute_SetFloat(ATTR_INDEX_temperatureResult1 + channel,
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
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void batteryTimerCallbackIsr(struct k_timer *timer_id)
{
	UNUSED_PARAMETER(timer_id);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK, FWK_ID_SENSOR_TASK,
				      FMC_READ_BATTERY);
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
