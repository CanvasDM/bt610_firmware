/**
 * @file AnalogSensorTask.c
 * @brief Blank is better that repeating the information in header.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(AnalogSensor);
#define THIS_FILE "AnalogSensor"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <stdlib.h>

#include "FrameworkIncludes.h"
#include "AdcRead.h"
#include "BspSupport.h"
#include "AnalogSensorTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

#ifndef ANALOG_SENSOR_TASK_PRIORITY
#define ANALOG_SENSOR_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef ANALOG_SENSOR_TASK_STACK_DEPTH
#define ANALOG_SENSOR_TASK_STACK_DEPTH 4096
#endif

#ifndef ANALOG_SENSOR_TASK_QUEUE_DEPTH
#define ANALOG_SENSOR_TASK_QUEUE_DEPTH 8
#endif

#define EXPANDER_ADDRESS         (0X70)
#define TCA9538_REG_INPUT		 (0x00)
#define TCA9538_REG_OUTPUT		 (0x01)
#define TCA9538_REG_POL_INV		 (0x02)
#define TCA9538_REG_CONFIG		 (0x03)
#define OUTPUT_CONFIG		     (0xC0) //pins 6 and 7 still set as inputs

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c0), okay)
#define I2C_DEV_NAME	DT_LABEL(DT_NODELABEL(i2c0))
#else
#error "Please set the correct I2C device"
#endif
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
typedef struct AnalogSensorTaskTag {
    FwkMsgTask_t msgTask; 
} AnalogSensorTaskObj_t;
/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static AnalogSensorTaskObj_t analogSensorTaskObject;

K_THREAD_STACK_DEFINE(analogSensorTaskStack, ANALOG_SENSOR_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(analogSensorTaskQueue, FWK_QUEUE_ENTRY_SIZE,
	      ANALOG_SENSOR_TASK_QUEUE_DEPTH, FWK_QUEUE_ALIGNMENT);

uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;              
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void AnalogSensorTaskThread(void *, void *, void *);            
static void ControlAnalogEnablePin(bool enable);
static void ControlThermistorEnablePin(bool enable);
static bool ExpanderAnalogSetup(uint8_t analogInput);

static DispatchResult_t ControlEnablePinsMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						    FwkMsg_t *pMsg);
static DispatchResult_t AnalogInputMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg);
static DispatchResult_t ReadAnalogPinMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg);
static DispatchResult_t ReadThermistorPinMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						    FwkMsg_t *pMsg);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t AnalogSensorTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
    case FMC_INVALID:            return Framework_UnknownMsgHandler;
    case FMC_CONTROL_ENABLE:     return ControlEnablePinsMsgHandler;
    case FMC_ANALOG_INPUT:       return AnalogInputMsgHandler;
    case FMC_READ_ANALOG:        return ReadAnalogPinMsgHandler;
    case FMC_READ_THERM:         return ReadThermistorPinMsgHandler;
    default:                     return NULL;
  }
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void AnalogSensorTask_Initialize(void)
{
  memset(&analogSensorTaskObject, 0, sizeof(AnalogSensorTaskObj_t));

  analogSensorTaskObject.msgTask.rxer.id               = FWK_ID_ANALOG_SENSOR_TASK;
  analogSensorTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
	analogSensorTaskObject.msgTask.rxer.pMsgDispatcher =
		AnalogSensorTaskMsgDispatcher;
  analogSensorTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
	analogSensorTaskObject.msgTask.timerPeriodTicks =
		K_MSEC(0); // 0 for one shot
  analogSensorTaskObject.msgTask.rxer.pQueue           = &analogSensorTaskQueue;
  
  Framework_RegisterTask(&analogSensorTaskObject.msgTask);
  
  analogSensorTaskObject.msgTask.pTid = 
    k_thread_create(&analogSensorTaskObject.msgTask.threadData, 
                    analogSensorTaskStack,
                    K_THREAD_STACK_SIZEOF(analogSensorTaskStack),
				AnalogSensorTaskThread, &analogSensorTaskObject,
				NULL, NULL, ANALOG_SENSOR_TASK_PRIORITY, 0,
                    K_NO_WAIT);

  k_thread_name_set(analogSensorTaskObject.msgTask.pTid, THIS_FILE);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void AnalogSensorTaskThread(void *pArg1, void *pArg2, void *pArg3)
{  
	AnalogSensorTaskObj_t *pObj = (AnalogSensorTaskObj_t *)pArg1;

	while (true) {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}
static void ControlAnalogEnablePin(bool enable)
{
 //   LOG_DBG("Analog Enable Set\n");
  BSP_PinSet(ANALOG_ENABLE_PIN, enable);    
}
static void ControlThermistorEnablePin(bool enable)
{
  LOG_DBG("Therm Enable Set\n");
  BSP_PinSet(THERM_ENABLE_PIN, enable);      
}

static bool ExpanderAnalogSetup(uint8_t analogInput)
{
	unsigned char datas[2];
	struct device *i2c_dev = device_get_binding(I2C_DEV_NAME);

	if (!i2c_dev) {
		LOG_ERR("Cannot get I2C device\n");
		return (0);
	}

	/* 1. Verify i2c_configure() */
	if (i2c_configure(i2c_dev, i2c_cfg)) {
		LOG_ERR("I2C config failed\n");
		return (0);
	}

	datas[0] = TCA9538_REG_CONFIG;
	datas[1] = OUTPUT_CONFIG;

	/* 2. verify i2c_write() */
	if (i2c_write(i2c_dev, datas, 2, EXPANDER_ADDRESS)) {
		LOG_ERR("Fail to configure expander\n");
		return (0);
	}

	datas[0] = TCA9538_REG_OUTPUT;
	datas[1] = analogInput;
    LOG_DBG("Analog input Set %d\n", analogInput);
	if (i2c_write(i2c_dev, datas, 2, EXPANDER_ADDRESS)) {
		LOG_ERR("Fail to configure output\n");
		return (0);
	}

	k_sleep(K_MSEC(1));
	(void)memset(datas, 0, sizeof(datas));
	return (1);
}

static DispatchResult_t ControlEnablePinsMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						    FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    SetEnablePinMsg_t *pControlPinMsg = (SetEnablePinMsg_t *)pMsg;

    ControlAnalogEnablePin(pControlPinMsg->control.analogEnable);
    ControlThermistorEnablePin(pControlPinMsg->control.thermEnable);
    return DISPATCH_OK;
}
static DispatchResult_t AnalogInputMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					      FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    bool analogExpanderResult = 0;
    uint8_t activeOutputs = 0;
    AnalogPinMsg_t *pAnalogMsg = (AnalogPinMsg_t *)pMsg;

    //The ain selector is zero base no one. Need to subtract 1
    activeOutputs = pAnalogMsg->externalPin - 1;
    //AIN_A0 and AIN_A1 are the high nibble
    activeOutputs = activeOutputs << 4;
	if (pAnalogMsg->inputConfig == CURRENT_AIN) {
        //For Current the output pins need to high
        activeOutputs = activeOutputs | 0X0F;
    }

    analogExpanderResult = ExpanderAnalogSetup(activeOutputs);
	if ((analogExpanderResult == 1) &&
	    ((pAnalogMsg->inputConfig == CURRENT_AIN) ||
	     (pAnalogMsg->inputConfig == VOLTAGE_AIN))) {
		FRAMEWORK_MSG_SEND_TO_SELF(
			analogSensorTaskObject.msgTask.rxer.id,
			FMC_READ_ANALOG);
	} else if ((analogExpanderResult == 1) &&
		   (pAnalogMsg->inputConfig == THERMISTOR)) {
		FRAMEWORK_MSG_SEND_TO_SELF(
			analogSensorTaskObject.msgTask.rxer.id, FMC_READ_THERM);
	} else {
        LOG_ERR("Fail to change I/O expander\n");
    }

    return DISPATCH_OK;
}
static DispatchResult_t ReadAnalogPinMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    UNUSED_PARAMETER(pMsg);
    uint32_t adcReading;
	uint8_t index = 0;
	for (index = 0; index < 10; index++) {
        adcReading = AnalogRead(ANALOG_SENSOR_1_CH);
        LOG_DBG("Analog Reading = %d\n", adcReading);
    }
    return DISPATCH_OK;
}
static DispatchResult_t ReadThermistorPinMsgHandler(FwkMsgReceiver_t *pMsgRxer,
						    FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    UNUSED_PARAMETER(pMsg);
    uint32_t adcReading;
	uint8_t index = 0;
	for (index = 0; index < 10; index++) {
        adcReading = AnalogRead(THERMISTOR_SENSOR_2_CH);
        LOG_DBG("Thermistor Reading = %d\n", adcReading);
    }
    return DISPATCH_OK;
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
