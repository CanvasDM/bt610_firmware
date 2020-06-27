/**
 * @file AnalogSensorTask.c
 * @brief Blank is better that repeating the information in header.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */



/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include "FrameworkIncludes.h"
#include "Bracket.h"
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <stdlib.h>

#include "AnalogSensorTask.h"
#include "AdcRead.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(AnalogSensor);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define THIS_FILE "AnalogSensor"
#if !ANALOG_SENSOR_TASK_USES_MAIN_THREAD
  #ifndef ANALOG_SENSOR_TASK_PRIORITY
    #define ANALOG_SENSOR_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef ANALOG_SENSOR_TASK_STACK_DEPTH
    #define ANALOG_SENSOR_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef ANALOG_SENSOR_TASK_QUEUE_DEPTH
  #define ANALOG_SENSOR_TASK_QUEUE_DEPTH 8
#endif

#define ANALOG_ENABLE_PIN        (28)//45
#define THERM_ENABLE_PIN         (10)
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
#define FLAGS_OR_ZERO(node)						\
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),		\
		    (DT_GPIO_FLAGS(node, gpios)),			\
		    (0))

#define BUTTON1_NODE DT_ALIAS(sw1)
#if DT_NODE_HAS_STATUS(BUTTON1_NODE, okay)
#define BUTTON1_DEV DT_GPIO_LABEL(BUTTON1_NODE, gpios)
#define BUTTON1_PIN DT_GPIO_PIN(BUTTON1_NODE, gpios)
#define BUTTON1_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(BUTTON1_NODE))
#else
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

typedef struct AnalogSensorTaskTag
{
    FwkMsgTask_t msgTask; 
    BracketObj_t *pBracket; 
    struct device *port1;
} AnalogSensorTaskObj_t;
/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static AnalogSensorTaskObj_t analogSensorTaskObject;

K_THREAD_STACK_DEFINE(analogSensorTaskStack, ANALOG_SENSOR_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(analogSensorTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              ANALOG_SENSOR_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void AnalogSensorTaskThread(void *, void *, void *);            
static void InitializeEnablePins(void);
static void ControlAnalogEnablePin(uint8_t enable);
static void ControlThermistorEnablePin(uint8_t enable);

static DispatchResult_t ControlEnablePinsMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t AnalogSenosrTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  case FMC_CONTROL_ENABLE:     return ControlEnablePinsMsgHandler;
  //case FMC_PERIODIC:           return AnalogSensorTaskPeriodicMsgHandler;
 // case FMC_WATCHDOG_CHALLENGE: return Watchdog_ChallengeHandler;
  default:                     return NULL;
  }
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void AnalogSensorTask_Initialize(void)
{
  memset(&analogSensorTaskObject, 0, sizeof(AnalogSensorTaskObj_t));

  analogSensorTaskObject.msgTask.rxer.id               = FWK_ID_ANALOG_SENSOR_TASK;
  analogSensorTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
  analogSensorTaskObject.msgTask.rxer.pMsgDispatcher   = AnalogSenosrTaskMsgDispatcher;
  analogSensorTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
  analogSensorTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
  analogSensorTaskObject.msgTask.rxer.pQueue           = &analogSensorTaskQueue;
  
  Framework_RegisterTask(&analogSensorTaskObject.msgTask);
  
  analogSensorTaskObject.msgTask.pTid = 
    k_thread_create(&analogSensorTaskObject.msgTask.threadData, 
                    analogSensorTaskStack,
                    K_THREAD_STACK_SIZEOF(analogSensorTaskStack),
                    AnalogSensorTaskThread,
                    &analogSensorTaskObject, 
                    NULL, 
                    NULL,
                    ANALOG_SENSOR_TASK_PRIORITY, 
                    0, 
                    K_NO_WAIT);

  k_thread_name_set(analogSensorTaskObject.msgTask.pTid, THIS_FILE);

  analogSensorTaskObject.pBracket = 		
    Bracket_Initialize(CONFIG_JSON_BRACKET_BUFFER_SIZE,
				   k_malloc(CONFIG_JSON_BRACKET_BUFFER_SIZE));
	
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void AnalogSensorTaskThread(void *pArg1, void *pArg2, void *pArg3)
{  
  AnalogSensorTaskObj_t *pObj = (AnalogSensorTaskObj_t*)pArg1;

  InitializeEnablePins();


  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}
static void InitializeEnablePins(void)
{
    struct device *gpio0;
    struct device *gpio1;
    uint8_t configureResult;

    LOG_DBG("Analog Enable Init\n");
    analogSensorTaskObject.port0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (!analogSensorTaskObject.port0) 
    {
        LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio0)));
	}
    analogSensorTaskObject.port1 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio1)));
	if (!analogSensorTaskObject.port1) 
    {
        LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio1)));
	}

    gpio_pin_configure(analogSensorTaskObject.port1, ANALOG_ENABLE_PIN, GPIO_OUTPUT_LOW);
	if (configureResult != 0) 
    {
        LOG_ERR("configure failed %s port pin %d!\n", DT_LABEL(DT_NODELABEL(gpio1)),ANALOG_ENABLE_PIN);
	}

    gpio_pin_set(analogSensorTaskObject.port1, ANALOG_ENABLE_PIN, 0);

}
static DispatchResult_t ControlEnablePinsMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    SetEnablePinMsg_t *pControlPinMsg = (SetEnablePinMsg_t *)pMsg;

    ControlAnalogEnablePin(pControlPinMsg->control.analogEnable);
    return DISPATCH_OK;
}

static void ControlAnalogEnablePin(uint8_t enable)
{
    LOG_DBG("Analog Enable Set\n");
    gpio_pin_set(analogSensorTaskObject.port1, ANALOG_ENABLE_PIN, enable);
    
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
