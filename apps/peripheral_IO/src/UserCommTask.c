/**
 * @file UserCommTask.c
 * @brief Functions used to interface with the I/O
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

#include "UserCommTask.h"
#include "AdcRead.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(UserComm);
/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define THIS_FILE "UserCommTask"
#if !USER_COMM_TASK_USES_MAIN_THREAD
  #ifndef USER_COMM_TASK_PRIORITY
    #define USER_COMM_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef USER_COMM_TASK_STACK_DEPTH
    #define USER_COMM_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef USER_COMM_TASK_QUEUE_DEPTH
  #define USER_COMM_TASK_QUEUE_DEPTH 8
#endif

#define BUTTON_POLL_RATE_MS 100

#define QUICK_BUTTON_PRESS_MIN_DURATION_MS  100
#define QUICK_BUTTON_PRESS_MAX_DURATION_MS  1000
#define MEDIUM_BUTTON_PRESS_MIN_DURATION_MS 3000
#define MEDIUM_BUTTON_PRESS_MAX_DURATION_MS 10000
#define LONG_BUTTON_PRESS_MIN_DURATION_MS   10000
#define LONG_BUTTON_PRESS_MAX_DURATION_MS   20000

typedef struct UserCommTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 

} UserCommTaskObj_t;
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserCommTaskObj_t userCommTaskObject;

K_THREAD_STACK_DEFINE(userCommTaskStack, USER_COMM_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(userCommTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              USER_COMM_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserCommTaskThread(void *, void *, void *);


//static DispatchResult_t UserCommTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t UserCommTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  //case FMC_PERIODIC:           return UserCommTaskPeriodicMsgHandler;
 // case FMC_CODE_BUTTON_ISR:    return ButtonIsrMsgHandler;
 // case FMC_WATCHDOG_CHALLENGE: return Watchdog_ChallengeHandler;
  default:                     return NULL;
  }
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void UserCommTask_Initialize(void)

{
  memset(&userCommTaskObject, 0, sizeof(UserCommTaskObj_t));

  userCommTaskObject.msgTask.rxer.id               = FWK_ID_USER_COMM_TASK;
  userCommTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
  userCommTaskObject.msgTask.rxer.pMsgDispatcher   = UserCommTaskMsgDispatcher;
  userCommTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
  userCommTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
  userCommTaskObject.msgTask.rxer.pQueue           = &userCommTaskQueue;
  
  Framework_RegisterTask(&userCommTaskObject.msgTask);
  
  userCommTaskObject.msgTask.pTid = 
    k_thread_create(&userCommTaskObject.msgTask.threadData, 
                    userCommTaskStack,
                    K_THREAD_STACK_SIZEOF(userCommTaskStack),
                    UserCommTaskThread,
                    &userCommTaskObject, 
                    NULL, 
                    NULL,
                    USER_COMM_TASK_PRIORITY, 
                    0, 
                    K_NO_WAIT);

  k_thread_name_set(userCommTaskObject.msgTask.pTid, THIS_FILE);

  userCommTaskObject.pBracket = 		
    Bracket_Initialize(CONFIG_JSON_BRACKET_BUFFER_SIZE,
				   k_malloc(CONFIG_JSON_BRACKET_BUFFER_SIZE));
	//userCommTaskObject.conn = NULL;

}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserCommTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
   UserCommTaskObj_t *pObj = (UserCommTaskObj_t*)pArg1;

  
  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}

void uartCallBack(struct device *x)
{
//	uart_irq_update(x);
//	int data_length = 0;

//	if (uart_irq_rx_ready(x)) {
//		data_length = uart_fifo_read(x, uart_buf, sizeof(uart_buf));
//		uart_buf[data_length] = 0;
//	}
//	printk("%s", uart_buf);
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
