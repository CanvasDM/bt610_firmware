/**
 * @file UserInterface.c
 * @brief Functions used to interface with the I/O
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(ui, CONFIG_UI_TASK_LOG_LEVEL);
#define THIS_FILE "ui"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#include "FrameworkIncludes.h"
#include "led_configuration.h"
#include "lcz_pwm_led.h"

#include "UserInterfaceTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef USER_IF_TASK_PRIORITY
#define USER_IF_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef USER_IF_TASK_STACK_DEPTH
#define USER_IF_TASK_STACK_DEPTH 1024
#endif

#ifndef USER_IF_TASK_QUEUE_DEPTH
#define USER_IF_TASK_QUEUE_DEPTH 8
#endif

#define FLAGS_OR_ZERO(node)                                                    \
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),                       \
		    (DT_GPIO_FLAGS(node, gpios)), (0))

#define BUTTON1_NODE DT_ALIAS(sw1)
#if DT_NODE_HAS_STATUS(BUTTON1_NODE, okay)
#define BUTTON1_DEV DT_GPIO_LABEL(BUTTON1_NODE, gpios)
#define BUTTON1_PIN DT_GPIO_PIN(BUTTON1_NODE, gpios)
#define BUTTON1_FLAGS (GPIO_INPUT | GPIO_PULL_UP | FLAGS_OR_ZERO(BUTTON1_NODE))
#else
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define BUTTON2_NODE DT_ALIAS(sw2)
#if DT_NODE_HAS_STATUS(BUTTON2_NODE, okay)
#define BUTTON2_DEV DT_GPIO_LABEL(BUTTON2_NODE, gpios)
#define BUTTON2_PIN DT_GPIO_PIN(BUTTON2_NODE, gpios)
#define BUTTON2_FLAGS (GPIO_INPUT | GPIO_PULL_UP | FLAGS_OR_ZERO(BUTTON2_NODE))
#else
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif

#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define LED1_DEV DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1 DT_GPIO_PIN(LED1_NODE, gpios)
#define LED2_DEV DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2 DT_GPIO_PIN(LED2_NODE, gpios)

typedef struct UserIfTaskTag {
	FwkMsgTask_t msgTask;
} UserIfTaskObj_t;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserIfTaskThread(void *, void *, void *);
static void InitializeButton(void);

static void Button1HandlerIsr(const struct device *dev, struct gpio_callback *cb,
			     uint32_t pins);

static void Button2HandlerIsr(const struct device *dev, struct gpio_callback *cb,
			     uint32_t pins);

static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					  FwkMsg_t *pMsg);

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void green_led_on(void);
static void green_led_off(void);
static void red_led_on(void);
static void red_led_off(void);
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserIfTaskObj_t userIfTaskObject;
static struct gpio_callback button1_cb_data;
static struct gpio_callback button2_cb_data;

K_THREAD_STACK_DEFINE(userIfTaskStack, USER_IF_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(userIfTaskQueue, FWK_QUEUE_ENTRY_SIZE, USER_IF_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static const lcz_led_configuration_t LED_CONFIGURATION[] = {
	{ GREEN_LED, green_led_on, green_led_off },
	{ RED_LED, red_led_on, red_led_off }
};
#else
static const lcz_led_configuration_t LED_CONFIGURATION[] = {
	{ GREEN_LED, LED2_DEV, LED2, LED_ACTIVE_HIGH },
	{ RED_LED, LED1_DEV, LED1, LED_ACTIVE_HIGH }
};
#endif

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t UserIfTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:     return Framework_UnknownMsgHandler;
	case FMC_LED_TEST:    return LedTestMsgHandler;
	default:              return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void UserInterfaceTask_Initialize(void)
{
	userIfTaskObject.msgTask.rxer.id = FWK_ID_USER_IF_TASK;
	userIfTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	userIfTaskObject.msgTask.rxer.pMsgDispatcher = UserIfTaskMsgDispatcher;
	userIfTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	userIfTaskObject.msgTask.timerPeriodTicks = K_MSEC(0); // 0 for one shot
	userIfTaskObject.msgTask.rxer.pQueue = &userIfTaskQueue;

	Framework_RegisterTask(&userIfTaskObject.msgTask);

	userIfTaskObject.msgTask.pTid =
		k_thread_create(&userIfTaskObject.msgTask.threadData,
				userIfTaskStack,
				K_THREAD_STACK_SIZEOF(userIfTaskStack),
				UserIfTaskThread, &userIfTaskObject, NULL, NULL,
				USER_IF_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(userIfTaskObject.msgTask.pTid, THIS_FILE);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserIfTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	UserIfTaskObj_t *pObj = (UserIfTaskObj_t *)pArg1;

	InitializeButton();

	lcz_led_init((lcz_led_configuration_t *)LED_CONFIGURATION,
		     ARRAY_SIZE(LED_CONFIGURATION));

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void InitializeButton(void)
{
	const struct device *button1Device;
	const struct device *button2Device;
	uint16_t ret;

	button1Device = device_get_binding(BUTTON1_DEV);
	if (button1Device == NULL) {
		LOG_DBG("Error: didn't find %s device", BUTTON1_DEV);
		return;
	}
	button2Device = device_get_binding(BUTTON2_DEV);
	if (button2Device == NULL) {
		printk("Error: didn't find %s device\n", BUTTON2_DEV);
		return;
	}

	ret = gpio_pin_configure(button1Device, BUTTON1_PIN, BUTTON1_FLAGS);
	if (ret != 0) {
		LOG_DBG("Error %d: failed to configure %s pin %d", ret,
			BUTTON1_DEV, BUTTON1_PIN);
		return;
	}

	ret = gpio_pin_interrupt_configure(button1Device, BUTTON1_PIN,
					   GPIO_INT_EDGE_FALLING);
	if (ret != 0) {
		LOG_DBG("Error %d: failed to configure interrupt on %s pin %d",
			ret, BUTTON1_DEV, BUTTON1_PIN);
		return;
	}
	ret = gpio_pin_interrupt_configure(button2Device, BUTTON2_PIN,
					   GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
		       ret, BUTTON2_DEV, BUTTON2_PIN);
		return;
	}

	gpio_init_callback(&button1_cb_data, Button1HandlerIsr, BIT(BUTTON1_PIN));
	gpio_init_callback(&button2_cb_data, Button2HandlerIsr, BIT(BUTTON2_PIN));
	gpio_add_callback(button1Device, &button1_cb_data);
	gpio_add_callback(button2Device, &button2_cb_data);
}

static DispatchResult_t LedTestMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					  FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsgRxer);
	UNUSED_PARAMETER(pMsg);
	const uint32_t delayMs = 1000;
	k_sleep(K_MSEC(delayMs));
	lcz_led_turn_on(GREEN_LED);
	k_sleep(K_MSEC(delayMs));
	lcz_led_turn_on(RED_LED);
	k_sleep(K_MSEC(delayMs));
	lcz_led_turn_off(GREEN_LED);
	k_sleep(K_MSEC(delayMs));
	lcz_led_turn_off(RED_LED);
	k_sleep(K_MSEC(delayMs));
	return DISPATCH_OK;
}

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void green_led_on(void)
{
	lcz_pwm_led_on(GREEN_LED, 1500, 700);
}

static void green_led_off(void)
{
	lcz_pwm_led_off(GREEN_LED);
}

static void red_led_on(void)
{
	lcz_pwm_led_on(RED_LED, 1500, 700);
}

static void red_led_off(void)
{
	lcz_pwm_led_off(RED_LED);
}
#endif

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
void Button1HandlerIsr(const struct device *dev, struct gpio_callback *cb,
		      uint32_t pins)
{
	LOG_DBG("Button pressed");
	FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
					      FMC_CODE_BLE_START_ADVERTISING);
}
void Button2HandlerIsr(const struct device *dev, struct gpio_callback *cb,
		      uint32_t pins)
{
	LOG_DBG("Button 2 pressed\n");
}
