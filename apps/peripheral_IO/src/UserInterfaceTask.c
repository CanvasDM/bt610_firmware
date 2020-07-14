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

#define BATT_OUT_ENABLE_PIN        (30)//SIO_30 Port0
#define FIVE_VOLT_ENABLE_PIN       (12)//SIO_44 Port1
#define DO1_PIN                    (12)//SIO_12 Port0
#define DO2_PIN                    (11)//SIO_11 Port0
#define DIN1_ENABLE_PIN            (05)//SIO_37 Port1
#define DIN2_ENABLE_PIN            (10)//SIO_42 Port1
#define DIN1_MCU_PIN               (09)//SIO_09 Port0
#define DIN2_MCU_PIN               (11)//SIO_43 Port1

typedef struct UserIfTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
  AnalogInput_t AnalogType;
  struct device *port0;
  struct device *port1;
} UserIfTaskObj_t;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserIfTaskObj_t userIfTaskObject;

static struct gpio_callback button_cb_data;
static struct gpio_callback digitalIn_cb_data;

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

static void InitializeEnablePins(void);
static void InitializeButton(void);
static void InitializeDigitalPinsNoPull(void);
static void InitializeDigitalPinsPull(void);
static void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void DigitalInHandlerIsr(struct device *dev, struct gpio_callback *cb, uint32_t pins);


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
	
  userIfTaskObject.AnalogType = UNKOWN_INPUT;

}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserIfTaskThread(void *pArg1, void *pArg2, void *pArg3)
{  
  UserIfTaskObj_t *pObj = (UserIfTaskObj_t*)pArg1;

  InitializeButton();
  InitializeEnablePins();


  while( true )
  {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
  }
}
static void InitializeEnablePins(void)
{
  userIfTaskObject.port0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (!userIfTaskObject.port0) 
    {
        LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio0)));
	}

  userIfTaskObject.port1 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio1)));
	if (!userIfTaskObject.port1) 
  {
        LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio1)));
	}

    //Port0
    gpio_pin_configure(userIfTaskObject.port0, BATT_OUT_ENABLE_PIN, GPIO_OUTPUT_LOW);
    gpio_pin_configure(userIfTaskObject.port0, DO1_PIN, GPIO_OUTPUT_LOW);
    gpio_pin_configure(userIfTaskObject.port0, DO2_PIN, GPIO_OUTPUT_LOW);
    //Port1
    gpio_pin_configure(userIfTaskObject.port1, FIVE_VOLT_ENABLE_PIN, GPIO_OUTPUT_LOW);
    gpio_pin_configure(userIfTaskObject.port1, DIN2_ENABLE_PIN, GPIO_OUTPUT_LOW);
    gpio_pin_configure(userIfTaskObject.port1, DIN1_ENABLE_PIN, GPIO_OUTPUT_LOW);


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
static void InitializeDigitalPinsNoPull(void)
{
  uint16_t ret;
  ret = gpio_pin_configure(userIfTaskObject.port1, DIN2_MCU_PIN, GPIO_INPUT);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure %s pin %d\n",
			ret, BUTTON1_DEV, DIN2_MCU_PIN);
		return;
	}
  ret = gpio_pin_interrupt_configure(userIfTaskObject.port1, DIN2_MCU_PIN,
					   GPIO_INT_EDGE_BOTH);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, BUTTON1_DEV, DIN2_MCU_PIN);
		return;
	}

  gpio_init_callback(&digitalIn_cb_data, DigitalInHandlerIsr,
			   BIT(DIN2_MCU_PIN));
	gpio_add_callback(userIfTaskObject.port1, &digitalIn_cb_data);

}
static void InitializeDigitalPinsPull(void)
{
  uint16_t ret;
  ret = gpio_pin_configure(userIfTaskObject.port1, DIN2_MCU_PIN, 
                          (GPIO_INPUT|GPIO_PULL_UP));
	if (ret != 0) 
  {
		printk("Error %d: failed to configure %s pin %d\n",
			ret, BUTTON1_DEV, DIN2_MCU_PIN);
		return;
	}
  ret = gpio_pin_interrupt_configure(userIfTaskObject.port1, DIN2_MCU_PIN,
					   GPIO_INT_EDGE_BOTH);
	if (ret != 0) 
  {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, BUTTON1_DEV, DIN2_MCU_PIN);
		return;
	}

  gpio_init_callback(&digitalIn_cb_data, DigitalInHandlerIsr,
			   BIT(DIN2_MCU_PIN));
	gpio_add_callback(userIfTaskObject.port1, &digitalIn_cb_data);

}

static int ennableAnalogPin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enabled = 0;
  enabled= atoi(argv[1]);

  SetEnablePinMsg_t * pMsg = (SetEnablePinMsg_t*)BufferPool_Take(sizeof(SetEnablePinMsg_t));
  if( pMsg != NULL )
  {
    pMsg->header.msgCode = FMC_CONTROL_ENABLE;
    pMsg->header.txId = FWK_ID_USER_IF_TASK;
    pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;
    pMsg->control.analogEnable = enabled; 
    pMsg->control.thermEnable = 1; //disable
    FRAMEWORK_MSG_SEND(pMsg);
  } 
  
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
    shell_print(shell,"Battery%d = %d \n", index, adcTestValue);
  }

  return(0);
}
static int readAin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
	uint8_t ainSelected = 0;
  ainSelected= atoi(argv[1]);

  
  if(userIfTaskObject.AnalogType == UNKOWN_INPUT)
  {
    shell_print(shell, "Analog Type not set");
  }
  else if( (ainSelected ==0) || (ainSelected > 4))
  {
    shell_print(shell, "Analog out of bounds");
  }
  else
  {
    AnalogPinMsg_t * pMsg = (AnalogPinMsg_t*)BufferPool_Take(sizeof(AnalogPinMsg_t));
    if( pMsg != NULL )
    {
      pMsg->header.msgCode = FMC_ANALOG_INPUT;
      pMsg->header.txId = FWK_ID_USER_IF_TASK;
      pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;  
      pMsg->inputConfig = userIfTaskObject.AnalogType; 
      pMsg->externalPin = ainSelected;      
      FRAMEWORK_MSG_SEND(pMsg);
    } 

    shell_print(shell, "Analog pin %d", ainSelected);
  }

  return(0);
}
static int analogVoltage(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  userIfTaskObject.AnalogType = VOLTAGE_AIN;

  shell_print(shell, "Set To Voltage\n");

  return(0);
}
static int analogCurrent(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);
  userIfTaskObject.AnalogType = CURRENT_AIN;

  shell_print(shell, "Set To Current\n");

  return(0);
}
static int ennableThermistorPin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enabled = 0;
  enabled= atoi(argv[1]);
  userIfTaskObject.AnalogType = THERMISTOR;

  SetEnablePinMsg_t * pMsg = (SetEnablePinMsg_t*)BufferPool_Take(sizeof(SetEnablePinMsg_t));
  if( pMsg != NULL )
  {
    pMsg->header.msgCode = FMC_CONTROL_ENABLE;
    pMsg->header.txId = FWK_ID_USER_IF_TASK;
    pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;
    pMsg->control.analogEnable = 0; 
    pMsg->control.thermEnable = enabled;
    FRAMEWORK_MSG_SEND(pMsg);
  } 
  shell_print(shell, "THERM_EN = %d \n",enabled);

  return(0);
}
static int readVrefPin(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint32_t adcVrefValue = 0;
  uint8_t index = 0;
  uint8_t maxReadings = 10;

  for(index =0; index < maxReadings; index++)
  {
    adcVrefValue = AnalogRead(ANALOG_SENSOR_5_CH);
    shell_print(shell,"Vref = %d \n", adcVrefValue);
  }

  return(0);
}
static int readTherm(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
	uint8_t thermSelected = 0;
  thermSelected= atoi(argv[1]);

  if( (thermSelected ==0) || (thermSelected > 4))
  {
    shell_print(shell, "Analog out of bounds");
  }
  else
  {
    AnalogPinMsg_t * pMsg = (AnalogPinMsg_t*)BufferPool_Take(sizeof(AnalogPinMsg_t));
    if( pMsg != NULL )
    {
      pMsg->header.msgCode = FMC_ANALOG_INPUT;
      pMsg->header.txId = FWK_ID_USER_IF_TASK;
      pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;     
      pMsg->inputConfig = THERMISTOR; 
      pMsg->externalPin = thermSelected;  
      FRAMEWORK_MSG_SEND(pMsg);
    } 

    shell_print(shell, "Analog pin %d", thermSelected);
  }

  return(0);
}
static int fiveEnable(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enable = 0;
  enable= atoi(argv[1]);

  shell_print(shell, "Set To 5V\n");
  gpio_pin_set(userIfTaskObject.port1, FIVE_VOLT_ENABLE_PIN, enable);

  return(0);
}
static int batteryEnable(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enable = 0;
  enable= atoi(argv[1]);

  shell_print(shell, "Set To Battery Enable\n");
  gpio_pin_set(userIfTaskObject.port0, BATT_OUT_ENABLE_PIN, enable);

  return(0);
}

static int DOtoggle(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enable = 0;
  enable= atoi(argv[1]);

  shell_print(shell, "Toggle DO1 and DO2\n");
  gpio_pin_toggle(userIfTaskObject.port0, DO1_PIN);
  gpio_pin_toggle(userIfTaskObject.port0, DO2_PIN);

  return(0);
}
static int digitalEnable(const struct shell *shell, size_t argc, char **argv)
{
  ARG_UNUSED(argc);
  uint8_t enable = 0;
  enable= atoi(argv[1]);

  shell_print(shell, "Set To DIN_EN =\n", enable);
  gpio_pin_set(userIfTaskObject.port1, DIN1_ENABLE_PIN, enable);
  gpio_pin_set(userIfTaskObject.port1, DIN2_ENABLE_PIN, enable);

  if(enable == 1)
  {
    InitializeDigitalPinsNoPull();
  }
  else
  {
    /* set as pull up */
    InitializeDigitalPinsPull();
  }
  
  return(0);
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
void ButtonHandlerIsr(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void DigitalInHandlerIsr(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
  if(pins == DIN2_MCU_PIN)
  {
	  LOG_DBG("Digital pin%d is %u"  "\n",pins, gpio_pin_get(userIfTaskObject.port1, pins));
  }
  else
  {
    LOG_DBG("Digital pin%d is %u"  "\n",pins, gpio_pin_get(userIfTaskObject.port0, pins));
  }
  
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
  SHELL_CMD(readVREF, NULL, "Read VREF ADC value", readVrefPin), 
  SHELL_CMD(therm, NULL, "Read Thermx", readTherm),
  SHELL_CMD(enable5v, NULL, "Enable 5V", fiveEnable),
  SHELL_CMD(enableBatt, NULL, "Enable Battery Out", batteryEnable),
  SHELL_CMD(toggleDO, NULL, "Toggle DO1 and DO2", DOtoggle),
  SHELL_CMD(enableDin, NULL, "Set DIN1_EN and DIN2_EN value", digitalEnable),
  SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(Test, &sub_inputs, "Test", NULL);


