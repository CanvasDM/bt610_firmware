//=================================================================================================
//!
//!
//! @copyright Copyright 2019 Laird
//!            All Rights Reserved.
//=================================================================================================

//=================================================================================================
// Includes
//=================================================================================================
#include <zephyr.h>
#include <power/reboot.h>
#include "FrameworkIncludes.h"
#include "Bracket.h"
//#include "SensorTask.h"
#include "SystemUartTask.h"
#include "ProtocolTask.h"
#include "UserInterfaceTask.h"
#include "UserCommTask.h"
#include "AnalogSensorTask.h"
#include "LedPwm.h"

//#include "lte.h"
//#include "aws.h"
//#include "nv.h"
//#include "appState.h"
//#include "oob_ble.h"

#include "ControlTask.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG

LOG_MODULE_REGISTER(ControlTask);

//=================================================================================================
// Local Constant, Macro and Type Definitions
//=================================================================================================
#define THIS_FILE "ControlTask"

#if !CONTROL_TASK_USES_MAIN_THREAD
  #ifndef CONTROL_TASK_PRIORITY
    #define CONTROL_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef CONTROL_TASK_STACK_DEPTH
    #define CONTROL_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef CONTROL_TASK_QUEUE_DEPTH
  #define CONTROL_TASK_QUEUE_DEPTH 10
#endif

#define MINIMUM_LED_TEST_STEP_DURATION_MS  (10)

typedef struct ControlTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 

} ControlTaskObj_t;

//=================================================================================================
// Global Data Definitions
//=================================================================================================

//=================================================================================================
// Local Data Definitions
//=================================================================================================
static ControlTaskObj_t controlTaskObject;

#if !CONTROL_TASK_USES_MAIN_THREAD
  K_THREAD_STACK_DEFINE(controlTaskStack, CONTROL_TASK_STACK_DEPTH);
#endif

K_MSGQ_DEFINE(controlTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              CONTROL_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);

//=================================================================================================
// Local Function Prototypes
//=================================================================================================
static void ControlTaskThread(void *, void *, void *);

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgTask_t *pMsgTask, FwkMsg_t *pMsg);
//static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t InitializeAllTasksMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);


//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t ControlTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:           return Framework_UnknownMsgHandler;
//  case FMC_PERIODIC:          return PeriodicMsgHandler;
  case FMC_INIT_ALL_TASKS:    return InitializeAllTasksMsgHandler;
  case FMC_SOFTWARE_RESET:    return SoftwareResetMsgHandler;
 // case FRAMEWORK_MSG_CODE_INIT_NV:           return InitNvMsgHander;
//  case FRAMEWORK_MSG_CODE_INIT_BLE:          return InitBleMsgHandler;
//  case FRAMEWORK_MSG_CODE_APP_READY:         return AppReadyMsgHandler;
//  case FRAMEWORK_MSG_CODE_SENSOR_EVENT:      return appState_SensorDataMsgHandler;
//  case FRAMEWORK_MSG_CODE_SENSOR_TICK_EVENT: return appState_SensorDataMsgHandler;
  case FMC_LED_TEST:          return LedTestMsgHandler;
  default:                                   return NULL;
  }
}

//=================================================================================================
// Global Function Definitions
//=================================================================================================
void ControlTask_Initialize(void)
{
  memset(&controlTaskObject, 0, sizeof(ControlTaskObj_t));

  controlTaskObject.msgTask.rxer.id               = FWK_ID_CONTROL_TASK;
  controlTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
  controlTaskObject.msgTask.rxer.pMsgDispatcher   = ControlTaskMsgDispatcher;
  controlTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
  controlTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
  controlTaskObject.msgTask.rxer.pQueue           = &controlTaskQueue;
  
  Framework_RegisterTask(&controlTaskObject.msgTask);
  
#if CONTROL_TASK_USES_MAIN_THREAD
  controlTaskObject.msgTask.pTid = k_current_get();
  k_thread_name_set(controlTaskObject.msgTask.pTid, THIS_FILE " is main thread");
#else
  controlTaskObject.msgTask.pTid = 
    k_thread_create(&controlTaskObject.msgTask.threadData, 
                    controlTaskStack,
                    K_THREAD_STACK_SIZEOF(controlTaskStack),
                    ControlTaskThread,
                    &controlTaskObject, 
                    NULL, 
                    NULL,
                    CONTROL_TASK_PRIORITY, 
                    0, 
                    K_NO_WAIT);

  k_thread_name_set(controlTaskObject.msgTask.pTid, THIS_FILE);

#endif
}



void ControlTask_Thread(void)
{
#if CONTROL_TASK_USES_MAIN_THREAD
  ControlTaskThread(&controlTaskObject, NULL, NULL);
#endif
}

//=================================================================================================
// Local Function Definitions
//=================================================================================================
static void ControlTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
  ControlTaskObj_t *pObj = (ControlTaskObj_t*)pArg1;


    BleTask_Initialize();
 // FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_INIT_NV);
  FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_LED_TEST);

  //For test only
  FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_INIT_ALL_TASKS);

  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}

static DispatchResult_t InitializeAllTasksMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);

#if 0  
  char const * p = Bsp_GetResetReasonString();
  AttributeTask_SetWithString(ATTR_INDEX_resetReason, p, strlen(p));
#endif

//  SensorTask_Initialize();
  SystemUartTask_Initialize();
  UserInterfaceTask_Initialize();  // sends messages to SensorTask
  UserCommTask_Initialize();
  AnalogSensorTask_Initialize();
  ProtocolTask_Initialize();
  
  
#if 0
  FRAMEWORK_MSG_CREATE_AND_SEND(FRAMEWORK_TASK_ID_CONTROL, 
                                FRAMEWORK_TASK_ID_SENSOR, 
                                FRAMEWORK_MSG_CODE_PING);
#endif
#if 0
  Framework_StartTimer(pMsgTask);
#endif

  // @ref ENABLE_BLE
  // If the softdevice is running, then breakpoints will cause a hardfault.
  //ControlTaskObj_t *pObj = (ControlTaskObj_t*)pMsgRxer->pContainer;
  FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_CONTROL_TASK, FMC_CODE_BLE_START);


  return DISPATCH_OK;
}
/*
static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  int32_t nextTickMs = appStateRunFsm();
  Framework_ChangeTimerPeriod(pMsgTask, nextTickMs, 0);
  return DISPATCH_OK;
}
*/

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgTask_t *pMsgTask, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  k_thread_priority_set(pMsgTask->pTid, -CONFIG_NUM_COOP_PRIORITIES);
  LOG_ERR("Software Reset in ~5 seconds");
  k_sleep(K_SECONDS(5));
  sys_reboot(SYS_REBOOT_WARM);
  return DISPATCH_OK;
}


/*
static DispatchResult_t InitNvMsgHander(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	int rc = nvInit();
  LOG_INF("NV init (%d)", rc);
  pMsg->header.txId = pMsgTask->id;
  pMsg->header.rxId = pMsgTask->id;
	pMsg->header.msgCode = 
    (rc < 0) ? FRAMEWORK_MSG_CODE_SOFTWARE_RESET : FRAMEWORK_MSG_CODE_INIT_LTE;
  FRAMEWORK_MSG_SEND(pMsg);
  return DISPATCH_DO_NOT_FREE;  
}
*/
/*
static DispatchResult_t InitBleMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  appStateSetupBleCellularService();
  int rc = oob_ble_initialize();
  pMsg->header.txId = pMsgTask->id;
  pMsg->header.rxId = pMsgTask->id;
  pMsg->header.msgCode = 
    (rc < 0) ? FRAMEWORK_MSG_CODE_SOFTWARE_RESET : FRAMEWORK_MSG_CODE_INIT_AWS;
  FRAMEWORK_MSG_SEND(pMsg);
  return DISPATCH_DO_NOT_FREE; 
}
*/
static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsgRxer);
  LedTestMsg_t * pLedMsg = (LedTestMsg_t *)pMsg;
  uint32_t delayMs = 1000;//pLedMsg->durationMs;
  delayMs = MAX(MINIMUM_LED_TEST_STEP_DURATION_MS, delayMs);
  LedPwm_off(0);
  LedPwm_off(1);
  k_sleep(K_MSEC(delayMs));
  LedPwm_on(0,1500,700);
  k_sleep(K_MSEC(delayMs));
  LedPwm_on(1,1500,700);
  k_sleep(K_MSEC(delayMs));
  LedPwm_off(0);
  LedPwm_off(1);
  k_sleep(K_MSEC(delayMs));
  
  return DISPATCH_OK;
}
static DispatchResult_t AppReadyMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  //appStateSetReady();
  //Framework_StartTimer(pMsgTask);
  return DISPATCH_OK;
}

EXTERNED void Framework_AssertionHandler(char *file, int line)
{
  static bool busy = 0;  // prevent recursion (buffer alloc fail, ...)
  if( !busy )
  {
    busy = true;
    LOG_ERR("\r\n!-----> Assertion <-----! %s:%d\r\n", file, line);
    __NOP();
  }
}

// end
