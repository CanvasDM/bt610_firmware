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
#include "Framework.h"
#include "Bracket.h"
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <stdlib.h>


#include "UserInterfaceTask.h"
#include "AdcRead.h"

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

typedef enum AnalogSelectonTag
{
  UNKOWN_READING = 0,
  VOLTAGE_READING,
  CURRENT_READING,
  THERMISTOR_READING,
} AnalogType_t;

typedef struct UserIfTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
  AnalogType_t AnalogType;

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
  AIN1 = 1,
  AIN2,
  AIN3,
  AIN4,
};              
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserIfTaskThread(void *, void *, void *);

static void InitializeButton(void);
static void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb, u32_t pins);

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

  userIfTaskObject.pBracket = Bracket_Initialize(1536);
	//userIfTaskObject.conn = NULL;
  userIfTaskObject.AnalogType = UNKOWN_READING;

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
  int ret;

	buttonDevice = device_get_binding(DT_ALIAS_SW0_GPIOS_CONTROLLER);
	if (buttonDevice == NULL) 
  {
		printk("Error: didn't find %s device\n",
			DT_ALIAS_SW0_GPIOS_CONTROLLER);
		return;
	}

  ret = gpio_pin_configure(buttonDevice, DT_ALIAS_SW0_GPIOS_PIN,
				 DT_ALIAS_SW0_GPIOS_FLAGS | GPIO_INPUT);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure pin %d '%s'\n",
			ret, DT_ALIAS_SW0_GPIOS_PIN, DT_ALIAS_SW0_LABEL);
		return;
	}

	ret = gpio_pin_interrupt_configure(buttonDevice, DT_ALIAS_SW0_GPIOS_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure interrupt on pin %d '%s'\n",
			ret, DT_ALIAS_SW0_GPIOS_PIN, DT_ALIAS_SW0_LABEL);
		return;
	}

  gpio_init_callback(&button_cb_data, ButtonHandlerIsr,
			   BIT(DT_ALIAS_SW0_GPIOS_PIN));
	gpio_add_callback(buttonDevice, &button_cb_data);
  
}

static int ennableAnalogPin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enabled = 0;
  enabled= atoi(argv[1]);


  shell_print(shell, "ANALOG_EN = %d \n",enabled);

  return(0);
}
static int batteryMeasurement(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
	ARG_UNUSED(argv);
  static volatile uint32_t adcTestValue = 0;
  uint8_t index = 0;
  uint8_t maxReadings = 10;

  for(index =0; index < maxReadings; index++)
  {
    adcTestValue = ADC_GetBatteryMv();
    shell_print(shell,"Battery %d = %d mv\n", index, adcTestValue);
  }

  return(0);
}
static int readAin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
	uint8_t ainSelected = 0;
  ainSelected= atoi(argv[1]);
  
  if(userIfTaskObject.AnalogType == UNKOWN_READING)
  {
    shell_print(shell, "Analog Type not set");
  }
  else
  {
    switch(ainSelected)
    {
      case AIN1:
        break;
      case AIN2:
        break;
      case AIN3:
        break;
      case AIN4:
        break;       
      default:
        FRAMEWORK_ASSERT(FORCED);
      break;
    }
    shell_print(shell, "Analog pin %d", ainSelected);
  }

  return(0);
}
static int analogVoltage(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  userIfTaskObject.AnalogType = VOLTAGE_READING;

  shell_print(shell, "Set To Voltage\n");

  return(0);
}
static int analogCurrent(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  userIfTaskObject.AnalogType = CURRENT_READING;

  shell_print(shell, "Set To Current\n");

  return(0);
}
static int ennableThermistorPin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enabled = 0;
  enabled= atoi(argv[1]);
  userIfTaskObject.AnalogType = THERMISTOR_READING;

  shell_print(shell, "THERM_EN = %d \n",enabled);

  return(0);
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb,
		    u32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

/******************************************************************************/
/* SHELL Service                                                  */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_inputs, 
  SHELL_CMD(enableAlog, NULL, "Enable/Disable Analog", ennableAnalogPin), 
  SHELL_CMD(battery, NULL, "Take Battery Measure", batteryMeasurement),
  SHELL_CMD(ain, NULL, "Read AINx", readAin),  
  SHELL_CMD(voltage, NULL, "Voltage Input", analogVoltage),
  SHELL_CMD(current, NULL, "Current Input", analogCurrent),
  SHELL_CMD(enableTherm, NULL, "Enable/Disable Thermistor", ennableThermistorPin),  
  SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(Test, &sub_inputs, "Test", NULL);


