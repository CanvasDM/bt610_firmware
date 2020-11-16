//=================================================================================================
//!
<<<<<<< HEAD
=======
#define THIS_FILE "ControlTask"
>>>>>>> made changes to the firmware filese that were updated
//!
//! @copyright Copyright 2019 Laird
//!            All Rights Reserved.
//=================================================================================================

//=================================================================================================
// Includes
//=================================================================================================
<<<<<<< HEAD
#include <zephyr.h>
#include <power/reboot.h>
#include "FrameworkIncludes.h"
#include "Bracket.h"
//#include "SensorTask.h"
#include "BspSupport.h"
#include "SystemUartTask.h"
#include "ProtocolTask.h"
#include "UserInterfaceTask.h"
#include "UserCommTask.h"
#include "AnalogSensorTask.h"
#include "LedPwm.h"
#include "Version.h"

=======
//#include <misc/reboot.h>
#include "Framework.h"
//#include "SensorTask.h"

//#include "led.h"
>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
#define THIS_FILE "ControlTask"

=======
>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
=======
  FwkMsgTask_t msgTask;  
>>>>>>> made changes to the firmware filese that were updated

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
<<<<<<< HEAD
static void HardwareTestInit(void);
static DispatchResult_t SoftwareResetMsgHandler(FwkMsgTask_t *pMsgTask, FwkMsg_t *pMsg);
//static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t InitializeAllTasksMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
=======

static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg);
//static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg);
static DispatchResult_t InitializeAllTasks(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg);
static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg);
>>>>>>> made changes to the firmware filese that were updated


//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t ControlTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:           return Framework_UnknownMsgHandler;
//  case FMC_PERIODIC:          return PeriodicMsgHandler;
<<<<<<< HEAD
  case FMC_INIT_ALL_TASKS:    return InitializeAllTasksMsgHandler;
=======
  case FMC_INIT_ALL_TASKS:    return InitializeAllTasks;
>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
=======
  controlTaskObject.msgTask.pContainer            = &controlTaskObject;
>>>>>>> made changes to the firmware filese that were updated
  controlTaskObject.msgTask.rxer.pQueue           = &controlTaskQueue;
  
  Framework_RegisterTask(&controlTaskObject.msgTask);
  
#if CONTROL_TASK_USES_MAIN_THREAD
<<<<<<< HEAD
  controlTaskObject.msgTask.pTid = k_current_get();
  k_thread_name_set(controlTaskObject.msgTask.pTid, THIS_FILE " is main thread");
#else
  controlTaskObject.msgTask.pTid = 
=======
  controlTaskObject.msgTask->ptid = k_current_get();
  k_thread_name_set(controlTaskObject.msgTask.tid, THIS_FILE " is main thread");
#else
  controlTaskObject.msgTask->ptid = 
>>>>>>> made changes to the firmware filese that were updated
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

<<<<<<< HEAD
  k_thread_name_set(controlTaskObject.msgTask.pTid, THIS_FILE);

=======
  k_thread_name_set(controlTaskObject.msgTask.tid, THIS_FILE);
>>>>>>> made changes to the firmware filese that were updated
#endif
}


<<<<<<< HEAD
#ifdef TEST_ME 1
=======

>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
    ControlTaskObj_t *pObj = (ControlTaskObj_t*)pArg1;

    HardwareTestInit();
    BleTask_Initialize();
 // FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_INIT_NV);
 //Test only
    FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_LED_TEST);

  
    FRAMEWORK_MSG_SEND_TO_SELF(pObj->msgTask.rxer.id, FMC_INIT_ALL_TASKS);

    while( true )
    {
      Framework_MsgReceiver(&pObj->msgTask.rxer);
    }
}
static void HardwareTestInit(void)
{
    bool terminalPresent = false;
    terminalPresent = BSP_TestPinUartChecker();

    SystemUartTask_Initialize(terminalPresent);

    printk("Version ");
    printk(TIMESTAMPED_VERSION_STRING);
    printk("\r\n");
    //log_printk("Reset Reason %s\r\n", Bsp_GetResetReasonString());
}
static DispatchResult_t InitializeAllTasksMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
=======
  ControlTaskObj_t *pObj = (ControlTaskObj_t*)pArg1;

Led_Initialize();

   FRAMEWORK_MSG_SEND_DIRECT_TO_SELF(pObj->msgTask, FMC_INIT_NV);

  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask);
  }
}

static DispatchResult_t InitializeAllTasks(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
>>>>>>> made changes to the firmware filese that were updated
{
  UNUSED_PARAMETER(pMsg);

#if 0  
  char const * p = Bsp_GetResetReasonString();
  AttributeTask_SetWithString(ATTR_INDEX_resetReason, p, strlen(p));
#endif

//  SensorTask_Initialize();
<<<<<<< HEAD
  UserInterfaceTask_Initialize();  // sends messages to SensorTask
  UserCommTask_Initialize();
  AnalogSensorTask_Initialize();
  
=======
>>>>>>> made changes to the firmware filese that were updated
  
#if 0
  FRAMEWORK_MSG_CREATE_AND_SEND(FRAMEWORK_TASK_ID_CONTROL, 
                                FRAMEWORK_TASK_ID_SENSOR, 
                                FRAMEWORK_MSG_CODE_PING);
#endif
#if 0
  Framework_StartTimer(pMsgTask);
#endif

<<<<<<< HEAD
  // @ref ENABLE_BLE
  // If the softdevice is running, then breakpoints will cause a hardfault.
  //ControlTaskObj_t *pObj = (ControlTaskObj_t*)pMsgRxer->pContainer;
  FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_CONTROL_TASK, FMC_CODE_BLE_START);

=======
#if 0
  // @ref ENABLE_BLE
  // If the softdevice is running, then breakpoints will cause a hardfault.
  ControlTaskObj_t *pObj = (ControlTaskObj_t*)pMsgTask->pContainer;
  FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(pObj->msgTask.id, MSG_CODE_BLE_START);
#endif
>>>>>>> made changes to the firmware filese that were updated

  return DISPATCH_OK;
}
/*
<<<<<<< HEAD
static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  int32_t nextTickMs = appStateRunFsm();
=======
static DispatchResult_t PeriodicMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  s32_t nextTickMs = appStateRunFsm();
>>>>>>> made changes to the firmware filese that were updated
  Framework_ChangeTimerPeriod(pMsgTask, nextTickMs, 0);
  return DISPATCH_OK;
}
*/

<<<<<<< HEAD
static DispatchResult_t SoftwareResetMsgHandler(FwkMsgTask_t *pMsgTask, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  k_thread_priority_set(pMsgTask->pTid, -CONFIG_NUM_COOP_PRIORITIES);
=======
static DispatchResult_t SoftwareResetMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsg);
  k_thread_priority_set(pMsgTask->tid, -CONFIG_NUM_COOP_PRIORITIES);
>>>>>>> made changes to the firmware filese that were updated
  LOG_ERR("Software Reset in ~5 seconds");
  k_sleep(K_SECONDS(5));
  sys_reboot(SYS_REBOOT_WARM);
  return DISPATCH_OK;
}


/*
<<<<<<< HEAD
static DispatchResult_t InitNvMsgHander(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
=======
static DispatchResult_t InitNvMsgHander(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
static DispatchResult_t InitBleMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
=======
static DispatchResult_t InitBleMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
>>>>>>> made changes to the firmware filese that were updated
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
<<<<<<< HEAD
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
=======
static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
{
  UNUSED_PARAMETER(pMsgTask);
  LedTestMsg_t * pLedMsg = (LedTestMsg_t *)pMsg;
  uint32_t delayMs = pLedMsg->durationMs;
  delayMs = MAX(MINIMUM_LED_TEST_STEP_DURATION_MS, delayMs);
  /*Led_TurnOff();
  k_sleep(delayMs);
  Led_TurnOn(GREEN);
  k_sleep(delayMs);
  Led_TurnOn(RED);
  k_sleep(delayMs);
  Led_TurnOn(YELLOW);
  k_sleep(delayMs);
  Led_TurnOff();
  k_sleep(delayMs);
  */
  return DISPATCH_OK;
}
static DispatchResult_t AppReadyMsgHandler(FwkMsgReceiver_t *pMsgTask, FwkMsg_t *pMsg)
>>>>>>> made changes to the firmware filese that were updated
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
