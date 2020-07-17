/**
 * @file UserInterface.c
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
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <stdlib.h>


#include "UserInterfaceTask.h"
#include "AdcRead.h"
#include "BspSupport.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(UserInterface);
/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define THIS_FILE "UserIfTask"
#if !USER_IF_TASK_USES_MAIN_THREAD
  #ifndef USER_IF_TASK_PRIORITY
    #define USER_IF_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef USER_IF_TASK_STACK_DEPTH
    #define USER_IF_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef USER_IF_TASK_QUEUE_DEPTH
  #define USER_IF_TASK_QUEUE_DEPTH 8
#endif

#define BUTTON_POLL_RATE_MS 100

#define QUICK_BUTTON_PRESS_MIN_DURATION_MS  100
#define QUICK_BUTTON_PRESS_MAX_DURATION_MS  1000
#define MEDIUM_BUTTON_PRESS_MIN_DURATION_MS 3000
#define MEDIUM_BUTTON_PRESS_MAX_DURATION_MS 10000
#define LONG_BUTTON_PRESS_MIN_DURATION_MS   10000
#define LONG_BUTTON_PRESS_MAX_DURATION_MS   20000

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

typedef struct UserIfTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
} UserIfTaskObj_t;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserIfTaskObj_t userIfTaskObject;
static struct gpio_callback button_cb_data;


K_THREAD_STACK_DEFINE(userIfTaskStack, USER_IF_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(userIfTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              USER_IF_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);

enum
{
  IN1 = 1,
  IN2,
  IN3,
  IN4,
};              
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserIfTaskThread(void *, void *, void *);
static void InitializeButton(void);

static void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb, uint32_t pins);



//static DispatchResult_t UserIfTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
//static DispatchResult_t ButtonIsrMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

//static bool isQuickPress(uint32_t Duration);
//static bool isMediumPress(uint32_t Duration);
//static bool isLongPress(uint32_t Duration);
//static bool IgnoreButton(uint32_t Duration);
//static int ennableAnalogPin(const struct shell *shell, size_t argc, char **argv);

//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t UserIfTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  //case FMC_PERIODIC:           return UserIfTaskPeriodicMsgHandler;
 // case FMC_CODE_BUTTON_ISR:    return ButtonIsrMsgHandler;
 // case FMC_WATCHDOG_CHALLENGE: return Watchdog_ChallengeHandler;
  default:                     return NULL;
  }
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void UserInterfaceTask_Initialize(void)
{
  memset(&userIfTaskObject, 0, sizeof(UserIfTaskObj_t));

  userIfTaskObject.msgTask.rxer.id               = FWK_ID_USER_IF_TASK;
  userIfTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
  userIfTaskObject.msgTask.rxer.pMsgDispatcher   = UserIfTaskMsgDispatcher;
  userIfTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
  userIfTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
  userIfTaskObject.msgTask.rxer.pQueue           = &userIfTaskQueue;
  
  Framework_RegisterTask(&userIfTaskObject.msgTask);
  
  userIfTaskObject.msgTask.pTid = 
    k_thread_create(&userIfTaskObject.msgTask.threadData, 
                    userIfTaskStack,
                    K_THREAD_STACK_SIZEOF(userIfTaskStack),
                    UserIfTaskThread,
                    &userIfTaskObject, 
                    NULL, 
                    NULL,
                    USER_IF_TASK_PRIORITY, 
                    0, 
                    K_NO_WAIT);

  k_thread_name_set(userIfTaskObject.msgTask.pTid, THIS_FILE);

  userIfTaskObject.pBracket = 		
    Bracket_Initialize(CONFIG_JSON_BRACKET_BUFFER_SIZE,
				   k_malloc(CONFIG_JSON_BRACKET_BUFFER_SIZE));
	
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserIfTaskThread(void *pArg1, void *pArg2, void *pArg3)
{  
  UserIfTaskObj_t *pObj = (UserIfTaskObj_t*)pArg1;

  InitializeButton();

  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}
static void InitializeButton(void)
{
  struct device *buttonDevice;
  uint16_t ret;

	buttonDevice = device_get_binding(BUTTON1_DEV);
	if (buttonDevice == NULL) 
  {
		printk("Error: didn't find %s device\n",
			BUTTON1_DEV);
		return;
	}

  ret = gpio_pin_configure(buttonDevice, BUTTON1_PIN, BUTTON1_FLAGS);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure %s pin %d\n",
			ret, BUTTON1_DEV, BUTTON1_PIN);
		return;
	}

	ret = gpio_pin_interrupt_configure(buttonDevice, BUTTON1_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, BUTTON1_DEV, BUTTON1_PIN);
		return;
	}

  gpio_init_callback(&button_cb_data, ButtonHandlerIsr,
			   BIT(BUTTON1_PIN));
	gpio_add_callback(buttonDevice, &button_cb_data);
  
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}




