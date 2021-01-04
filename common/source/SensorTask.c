/**
 * @file SensorTask.c
 * @brief Functions used to interface with the I/O
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(Sensor);
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
#include "SensorTask.h"
#include "BspSupport.h"
#include "AdcBt6.h"

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

} SensorTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static SensorTaskObj_t digitalIOTaskObject;

K_THREAD_STACK_DEFINE(digitalIOTaskStack, SENSOR_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(digitalIOTaskQueue, FWK_QUEUE_ENTRY_SIZE, SENSOR_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void SensorTaskThread(void *, void *, void *);
static void CheckBattery(void);
static DispatchResult_t
SensorTaskAttributeChangedMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				     FwkMsg_t *pMsg);
static DispatchResult_t
SensorTaskDigitalInputdMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t
SensorTaskDigitalInAlarmSetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				      FwkMsg_t *pMsg);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t SensorTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:       	return Framework_UnknownMsgHandler;
	case FMC_ATTR_CHANGED:  	return SensorTaskAttributeChangedMsgHandler;
	case FMC_DIGITAL_IN: 		return SensorTaskDigitalInputdMsgHandler;
	case FMC_DIGITAL_IN_ALARM:  return SensorTaskDigitalInAlarmSetMsgHandler;
	default:                	return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SensorTask_Initialize(void)
{
	digitalIOTaskObject.msgTask.rxer.id = FWK_ID_SENSOR_TASK;
	digitalIOTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	digitalIOTaskObject.msgTask.rxer.pMsgDispatcher =
		SensorTaskMsgDispatcher;
	digitalIOTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	digitalIOTaskObject.msgTask.timerPeriodTicks =
		K_MSEC(0); // 0 for one shot
	digitalIOTaskObject.msgTask.rxer.pQueue = &digitalIOTaskQueue;

	Framework_RegisterTask(&digitalIOTaskObject.msgTask);

	digitalIOTaskObject.msgTask.pTid =
		k_thread_create(&digitalIOTaskObject.msgTask.threadData,
				digitalIOTaskStack,
				K_THREAD_STACK_SIZEOF(digitalIOTaskStack),
				SensorTaskThread, &digitalIOTaskObject, NULL,
				NULL, SENSOR_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(digitalIOTaskObject.msgTask.pTid, THIS_FILE);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SensorTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	SensorTaskObj_t *pObj = (SensorTaskObj_t *)pArg1;

	/*Get the current status of the digital inputs connections*/
	BSP_DigitalPinsStatus();

	CheckBattery();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void CheckBattery(void)
{
	int16_t raw = 0;
	int32_t mv = 0;
	AdcBt6_ReadBatteryMv(&raw, &mv);

	Attribute_SetSigned32(ATTR_INDEX_batteryVoltageMv, mv);
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
			//FRAMEWORK_MSG_CREATE_AND_SEND(
			//	FWK_ID_SENSOR_TASK,
			//	FWK_ID_SENSOR_TASK,
			//	MSG_CODE_READ_BATTERY);
			break;

		case ATTR_INDEX_temperatureSenseInterval:
			//FRAMEWORK_MSG_CREATE_AND_SEND(
			//	FWK_ID_SENSOR_TASK,
			//	FWK_ID_SENSOR_TASK,
			//	MSG_CODE_TEMPERATURE_MEASURE);
			break;

		case ATTR_INDEX_digitalInput1:
		case ATTR_INDEX_digitalInput2:
			FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_SENSOR_TASK,
						   FMC_DIGITAL_IN_ALARM);
			break;
		case ATTR_INDEX_analogInput1Type:
		case ATTR_INDEX_analogInput2Type:
		case ATTR_INDEX_analogInput3Type:
		case ATTR_INDEX_analogInput4Type:
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
SensorTaskDigitalInputdMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	DigitalInMsg_t *pSensorMsg = (DigitalInMsg_t *)pMsg;
	uint8_t pinStatus = pSensorMsg->status;

	if (pSensorMsg->pin == DIN1_MCU_PIN) {
		Attribute_SetUint32(ATTR_INDEX_digitalInput1Status, pinStatus);
	} else if (pSensorMsg->pin == DIN2_MCU_PIN) {
		Attribute_SetUint32(ATTR_INDEX_digitalInput2Status, pinStatus);
	} else {
		/* Error, only 2 digital pins */
	}

	return DISPATCH_OK;
}
static DispatchResult_t
SensorTaskDigitalInAlarmSetMsgHandler(FwkMsgReceiver_t *pMsgRxer,
				      FwkMsg_t *pMsg)
{
	uint8_t input1TriggerLevel;
	uint8_t input2TriggerLevel;
	Attribute_Get(ATTR_INDEX_digitalInput1, &input1TriggerLevel,
		      sizeof(uint8_t));
	Attribute_Get(ATTR_INDEX_digitalInput2, &input2TriggerLevel,
		      sizeof(uint8_t));

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
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
