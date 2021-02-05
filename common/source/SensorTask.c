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

#include "SensorTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

#ifndef SENSOR_TASK_PRIORITY
#define SENSOR_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef SENSOR_TASK_STACK_DEPTH
#define SENSOR_TASK_STACK_DEPTH 4096
#endif

#ifndef SENSOR_TASK_QUEUE_DEPTH
#define SENSOR_TASK_QUEUE_DEPTH 8
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi1), okay)
#define SPI_DEV_NAME DT_LABEL(DT_NODELABEL(spi1))
#else
#error "Please set the correct spi device"
#endif

typedef struct SensorTaskTag {
	FwkMsgTask_t msgTask;
	uint8_t localActiveMode;
	struct k_timer *batteryTimer;
} SensorTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static SensorTaskObj_t sensorTaskObject;

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
SensorTaskDigitalInputMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t
SensorTaskDigitalInAlarmSetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				      FwkMsg_t *pMsg);
static DispatchResult_t MagnetStateMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg);
static DispatchResult_t ReadBatteryMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg);

static void SensorConfigChange(void);
static void SensorOutput1Control();
static void SensorOutput2Control();

static void UpdateDin1(void);
static void UpdateDin2(void);
static void UpdateMagnet(void);

static int Analog(size_t channel, AdcPwrSequence_t power);
static int Thermistor(size_t channel, AdcPwrSequence_t power);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t SensorTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:           return Framework_UnknownMsgHandler;
	case FMC_ATTR_CHANGED:      return SensorTaskAttributeChangedMsgHandler;
	case FMC_DIGITAL_IN:        return SensorTaskDigitalInputMsgHandler;
	case FMC_DIGITAL_IN_ALARM:  return SensorTaskDigitalInAlarmSetMsgHandler;
	case FMC_MAGNET_STATE:      return MagnetStateMsgHandler;
	case FMC_READ_BATTERY:      return ReadBatteryMsgHandler;
	default:                    return NULL;
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
	sensorTaskObject.msgTask.rxer.pMsgDispatcher =
		SensorTaskMsgDispatcher;
	sensorTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	sensorTaskObject.msgTask.timerPeriodTicks =
		K_MSEC(0); // 0 for one shot
	sensorTaskObject.msgTask.rxer.pQueue = &sensorTaskQueue;

	Framework_RegisterTask(&sensorTaskObject.msgTask);

	sensorTaskObject.msgTask.pTid =
		k_thread_create(&sensorTaskObject.msgTask.threadData,
				sensorTaskStack,
				K_THREAD_STACK_SIZEOF(sensorTaskStack),
				SensorTaskThread, &sensorTaskObject, NULL,
				NULL, SENSOR_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(sensorTaskObject.msgTask.pTid, THIS_FILE);
}

int AttributePrepare_batteryVoltageMv(void)
{
	int16_t raw = 0;
	int32_t mv = 0;
	int r = AdcBt6_ReadBatteryMv(&raw, &mv);

	if (r >= 0) {
		r = Attribute_SetSigned32(ATTR_INDEX_batteryVoltageMv, mv);
	}
	return r;
}

int AttributePrepare_analogInput1(void)
{
	return Analog(0, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_analogInput2(void)
{
	return Analog(1, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_analogInput3(void)
{
	return Analog(2, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_analogInput4(void)
{
	return Analog(3, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_temperatureResult1(void)
{
	return Thermistor(0, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_temperatureResult2(void)
{
	return Thermistor(1, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_temperatureResult3(void)
{
	return Thermistor(2, ADC_PWR_SEQ_SINGLE);
}

int AttributePrepare_temperatureResult4(void)
{
	return Thermistor(3, ADC_PWR_SEQ_SINGLE);
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

	/* todo: Enable Din1 and Din2 based on configuration. */
	BSP_PinSet(DIN1_ENABLE_PIN, 1);
	BSP_PinSet(DIN2_ENABLE_PIN, 1);

	SensorOutput1Control();
	SensorOutput2Control();

	AttributePrepare_batteryVoltageMv();
	UpdateDin1();
	UpdateDin2();
	UpdateMagnet();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static DispatchResult_t
SensorTaskAttributeChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	//if (activeMode) {
	AttrChangedMsg_t *pAttrMsg = (AttrChangedMsg_t *)pMsg;
	size_t i;
	for (i = 0; i < pAttrMsg->count; i++) {
		switch (pAttrMsg->list[i]) {
		case ATTR_INDEX_batterySenseInterval:
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_SENSOR_TASK,
						      FWK_ID_SENSOR_TASK,
						      FMC_READ_BATTERY);
			break;

		case ATTR_INDEX_temperatureSenseInterval:
			//FRAMEWORK_MSG_CREATE_AND_SEND(
			//	FWK_ID_SENSOR_TASK,
			//	FWK_ID_SENSOR_TASK,
			//	MSG_CODE_TEMPERATURE_MEASURE);
			break;

			//		case ATTR_INDEX_digitalInput1:
			//		case ATTR_INDEX_digitalInput2:
			//			FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK,
			//						   FMC_DIGITAL_IN_ALARM);
			//			break;
		case ATTR_INDEX_analogInput1Type:
		case ATTR_INDEX_analogInput2Type:
		case ATTR_INDEX_analogInput3Type:
		case ATTR_INDEX_analogInput4Type:
			break;
		case ATTR_INDEX_configType:
			//FRAMEWORK_MSG_CREATE_AND_BROADCAST(FWK_ID_SENSOR_TASK, FMC_SENSOR_CONFIG_CHANGE);
			SensorConfigChange();
			break;

		case ATTR_INDEX_digitalOutput1Enable:
			SensorOutput1Control();
			break;
		case ATTR_INDEX_digitalOutput2Enable:
			SensorOutput2Control();
			break;
		case ATTR_INDEX_activeMode:
			// It isn't expected that host will write this.
			break;

		default:
			// don't do anything - this is a broadcast
			break;
		}
	}
	//}

	return DISPATCH_OK;
}
static DispatchResult_t
SensorTaskDigitalInputMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	DigitalInMsg_t *pSensorMsg = (DigitalInMsg_t *)pMsg;

	if (pSensorMsg->pin == DIN1_MCU_PIN) {
		UpdateDin1();
	} else if (pSensorMsg->pin == DIN2_MCU_PIN) {
		UpdateDin2();
	}

	return DISPATCH_OK;
}
static DispatchResult_t
SensorTaskDigitalInAlarmSetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				      FwkMsg_t *pMsg)
{
	uint8_t input1TriggerLevel;
	uint8_t input2TriggerLevel;
	//Attribute_Get(ATTR_INDEX_digitalInput1, &input1TriggerLevel,
	//	      sizeof(uint8_t));
	//Attribute_Get(ATTR_INDEX_digitalInput2, &input2TriggerLevel,
	//	      sizeof(uint8_t));

	if (input1TriggerLevel == 0) {
		/*Set alarm when input is low*/
	} else {
		/*Set alarm when input is High*/
	}
	if (input2TriggerLevel == 0) {
		/*Set alarm when input is low*/
	} else {
		/*Set alarm when input is High*/
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
	/*Generate Battery Event Message*/
	/*TODO: ADD FUNCTION*/
	if (sensorTaskObject.localActiveMode == true) {
		uint32_t intervalSeconds = 0;
		Attribute_GetUint32(&intervalSeconds,
				    ATTR_INDEX_batterySenseInterval);
		if (intervalSeconds != 0) {
			k_timer_start(sensorTaskObject.batteryTimer,
				      K_SECONDS(intervalSeconds), K_NO_WAIT);
		}
	}
	return DISPATCH_OK;
}
static void SensorConfigChange(void)
{
	uint32_t configurationType;
	uint32_t thermistorsConfig = 0;
	analogConfigType_t analog1Config = ANALOG_UNUSED;
	analogConfigType_t analog2Config = ANALOG_UNUSED;
	analogConfigType_t analog3Config = ANALOG_UNUSED;
	analogConfigType_t analog4Config = ANALOG_UNUSED;
	Attribute_GetUint32(&configurationType, ATTR_INDEX_configType);

	switch (configurationType) {
	case CONFIG_UNDEFINED:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		/*Disable all analogs*/
		Attribute_SetUint32(ATTR_INDEX_analogInput1, analog1Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput2, analog2Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput3, analog3Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput4, analog4Config);
		break;
	case CONFIG_ANALOG_VOLTAGE:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		/*Configure all analogs to voltage*/
		Attribute_SetUint32(ATTR_INDEX_analogInput1, ANALOG_VOLTAGE);
		Attribute_SetUint32(ATTR_INDEX_analogInput2, ANALOG_VOLTAGE);
		Attribute_SetUint32(ATTR_INDEX_analogInput3, ANALOG_VOLTAGE);
		Attribute_SetUint32(ATTR_INDEX_analogInput4, ANALOG_VOLTAGE);
		break;
	case CONFIG_DIGITAL:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		/*Disable all analogs*/
		Attribute_SetUint32(ATTR_INDEX_analogInput1, analog1Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput2, analog2Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput3, analog3Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput4, analog4Config);
		break;
	case CONFIG_TEMPERATURE:
		/*Enable all the thermistors*/
		thermistorsConfig = 0x0F;
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		break;
	case CONFIG_ANALOG_CURRENT:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		/*Configure all analogs to current*/
		Attribute_SetUint32(ATTR_INDEX_analogInput1, ANALOG_CURRENT);
		Attribute_SetUint32(ATTR_INDEX_analogInput2, ANALOG_CURRENT);
		Attribute_SetUint32(ATTR_INDEX_analogInput3, ANALOG_CURRENT);
		Attribute_SetUint32(ATTR_INDEX_analogInput4, ANALOG_CURRENT);
		break;
	case CONFIG_ULTRASONIC_PRESSURE:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		break;
	case CONFIG_SPI_I2C:
		/*Disable all the thermistors*/
		Attribute_SetUint32(ATTR_INDEX_thermistorConfig,
				    thermistorsConfig);
		/*Disable all analogs*/
		Attribute_SetUint32(ATTR_INDEX_analogInput1, analog1Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput2, analog2Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput3, analog3Config);
		Attribute_SetUint32(ATTR_INDEX_analogInput4, analog4Config);
		break;
	default:
		/*Set to undefined*/
		Attribute_SetUint32(ATTR_INDEX_configType, CONFIG_UNDEFINED);
		break;
	}
}
static void SensorOutput1Control()
{
	uint8_t outputStatus = 0;
	Attribute_Get(ATTR_INDEX_digitalOutput1Enable, &outputStatus,
		      sizeof(outputStatus));

	/*Need to inverse the status logic to control the FET*/
	BSP_PinSet(DO1_PIN, !(outputStatus));
}
static void SensorOutput2Control()
{
	uint8_t outputStatus = 0;
	Attribute_Get(ATTR_INDEX_digitalOutput2Enable, &outputStatus,
		      sizeof(outputStatus));

	/*Need to inverse the status logic to control the FET*/
	BSP_PinSet(DO2_PIN, !(outputStatus));
}
static void UpdateDin1(void)
{
	int v = BSP_PinGet(DIN1_MCU_PIN);

	if (v >= 0) {
		Attribute_SetMask32(ATTR_INDEX_digitalInput, 0, v);
	}

	/* todo: refactor for 1/2?; check alarm; update alarm */
	/* maybe having two attributes is better */
}

static void UpdateDin2(void)
{
	int v = BSP_PinGet(DIN2_MCU_PIN);

	if (v >= 0) {
		Attribute_SetMask32(ATTR_INDEX_digitalInput, 1, v);
	}
}

static void UpdateMagnet(void)
{
	int v = BSP_PinGet(MAGNET_MCU_PIN);

	if (v >= 0) {
		v = Attribute_SetUint32(ATTR_INDEX_magnetState, v);
	}

	/* todo: set flag or deprecate flag bits for bt6 */
}

static int Analog(size_t channel, AdcPwrSequence_t power)
{
	int r = -EPERM;
	float result = 0.0;
	int16_t raw = 0;

	attr_idx_t base = ATTR_INDEX_analogInput1Type;
	uint32_t config = Attribute_AltGetUint32(base + channel, 0);

	switch (config) {
	case ANALOG_INPUT_VOLTAGE:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_VOLTAGE, power);
		if (r >= 0) {
			result = AdcBt6_ConvertVoltage(raw);
		}
		break;

	case ANALOG_INPUT_CURRENT:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_CURRENT, power);
		if (r >= 0) {
			result = AdcBt6_ConvertCurrent(raw);
		}
		break;

	case ANALOG_INPUT_PRESSURE:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_PRESSURE, power);
		if (r >= 0) {
			result = AdcBt6_ConvertPressure(raw);
		}
		break;

	case ANALOG_INPUT_ULTRASONIC:
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_ULTRASONIC, power);
		if (r >= 0) {
			result = AdcBt6_ConvertUltrasonic(raw);
		}
		break;

	default:
		r = 0;
		LOG_DBG("Analog input channel %d disabled", channel + 1);
		break;
	}

	if (r >= 0) {
		r = Attribute_SetFloat(ATTR_INDEX_analogInput1 + channel,
				       result);
	}

	/* todo: alarm handler */

	return r;
}

static int Thermistor(size_t channel, AdcPwrSequence_t power)
{
	int r = -EPERM;
	float result = 0.0;
	int16_t raw = 0;

	uint32_t config =
		Attribute_AltGetUint32(ATTR_INDEX_thermistorConfig, 0);

	if (config & BIT(channel)) {
		r = AdcBt6_Measure(&raw, channel, ADC_TYPE_THERMISTOR, power);
		if (r >= 0) {
			result = AdcBt6_ConvertThermToTemperature(raw);
		}
	} else {
		LOG_DBG("Thermistor channel %d not enabled", channel + 1);
		r = 0;
	}

	if (r >= 0) {
		r = Attribute_SetFloat(ATTR_INDEX_temperatureResult1 + channel,
				       result);
	}

	return r;
}
