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
#include "Attribute.h"
#include "BspSupport.h"
#include "Flags.h"
#include "Advertisement.h"
#include "UserInterfaceTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define BREAK_ON_ERROR(x)                                                      \
	if (x < 0) {                                                           \
		break;                                                         \
	}

#ifndef USER_IF_TASK_PRIORITY
#define USER_IF_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef USER_IF_TASK_STACK_DEPTH
#define USER_IF_TASK_STACK_DEPTH 2048
#endif

#ifndef USER_IF_TASK_QUEUE_DEPTH
#define USER_IF_TASK_QUEUE_DEPTH 8
#endif

/******************************************************************************/
/* Button Configuration                                                       */
/******************************************************************************/
#define BUTTON1_NODE DT_ALIAS(sw1)
#define BUTTON2_NODE DT_ALIAS(sw2)
#define BUTTON3_NODE DT_ALIAS(sw3)

#define FLAGS_OR_ZERO(node)                                                    \
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),                       \
		    (DT_GPIO_FLAGS(node, gpios)), (0))

struct button_cfg {
	const char *label;
	gpio_pin_t pin;
	gpio_flags_t flags;
	uint32_t edge;
	void (*isr)(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins);
};

#define BUTTON_CFG(x, _flags, _edge)                                           \
	{                                                                      \
		.label = DT_GPIO_LABEL(BUTTON##x##_NODE, gpios),               \
		.pin = DT_GPIO_PIN(BUTTON##x##_NODE, gpios),                   \
		.flags = (_flags) | FLAGS_OR_ZERO(BUTTON##x##NODE),            \
		.edge = (_edge), .isr = Button##x##HandlerIsr                  \
	}

#define DT_HAS_BUTTON_NODE(x) DT_NODE_HAS_STATUS(BUTTON##x##_NODE, okay)

/******************************************************************************/
/* LED Configuration                                                          */
/******************************************************************************/
#define MINIMUM_LED_TEST_STEP_DURATION_MS (10)

#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/* clang-format off */
#define LED1_DEV  DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1      DT_GPIO_PIN(LED1_NODE, gpios)
#define LED2_DEV  DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2      DT_GPIO_PIN(LED2_NODE, gpios)
/* clang-format on */

typedef struct UserIfTaskTag {
	FwkMsgTask_t msgTask;
} UserIfTaskObj_t;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserIfTaskThread(void *, void *, void *);

static int InitializeButtons(void);
static void TamperSwitchStatus(void);
static void SendUIEvent(SensorEventType_t type, SensorEventData_t data);

static void Button1HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static void Button2HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static void Button3HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static Dispatch_t AliveMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t TamperMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t ExitShelfModeMsgHandler(FwkMsgRxer_t *pMsgRxer,
					  FwkMsg_t *pMsg);
static Dispatch_t UiFactoryResetMsgHandler(FwkMsgRxer_t *pMsgRxer,
					   FwkMsg_t *pMsg);
static Dispatch_t AmrLedOnMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t LedsOffMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void green_led_on(void);
static void green_led_off(void);
static void red_led_on(void);
static void red_led_off(void);
#endif

static bool ValidAliveDuration(int64_t duration);
static bool ValidExitShelfModeDuration(int64_t duration);
static bool ValidFactoryResetDuration(int64_t duration);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserIfTaskObj_t userIfTaskObject;

static struct gpio_callback button_cb_data[CONFIG_UI_NUMBER_OF_BUTTONS];

K_THREAD_STACK_DEFINE(userIfTaskStack, USER_IF_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(userIfTaskQueue, FWK_QUEUE_ENTRY_SIZE, USER_IF_TASK_QUEUE_DEPTH,
	      FWK_QUEUE_ALIGNMENT);

#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static const lcz_led_configuration_t LED_CONFIGURATION[] = {
	{ GREEN_LED, green_led_on, green_led_off },
	{ RED_LED, red_led_on, red_led_off }
};
#else
/* todo: retest */
static const lcz_led_configuration_t LED_CONFIGURATION[] = {
	{ GREEN_LED, LED2_DEV, LED2, LED_ACTIVE_HIGH },
	{ RED_LED, LED1_DEV, LED1, LED_ACTIVE_HIGH }
};
#endif

static const struct button_cfg BUTTON_CFG[] = {
#if DT_HAS_BUTTON_NODE(1)
	BUTTON_CFG(1, (GPIO_INPUT | GPIO_PULL_UP), GPIO_INT_EDGE_BOTH),
#endif
#if DT_HAS_BUTTON_NODE(2)
	BUTTON_CFG(2, (GPIO_INPUT | GPIO_PULL_UP), GPIO_INT_EDGE_BOTH),
#endif
#if DT_HAS_BUTTON_NODE(3)
	BUTTON_CFG(3, GPIO_INPUT, GPIO_INT_EDGE_BOTH),
#endif
};
BUILD_ASSERT(
	ARRAY_SIZE(BUTTON_CFG) == CONFIG_UI_NUMBER_OF_BUTTONS,
	"Unsupported board: sw1, sw2, and sw3 devicetree aliases must be defined");

/* clang-format off */
static const struct lcz_led_blink_pattern ALIVE_PATTERN = {
	.on_time = 1000,
	.off_time = 0,
	.repeat_count = 0
};

static const struct lcz_led_blink_pattern TAMPER_PATTERN = {
	.on_time = 50,
	.off_time = 1950,
	.repeat_count = 60-1
};

static const struct lcz_led_blink_pattern EXIT_SHELF_MODE_PATTERN = {
	.on_time = 100,
	.off_time = 900,
	.repeat_count = 30 - 1
};

static const struct lcz_led_blink_pattern FACTORY_RESET_PATTERN = {
	.on_time = 5000,
	.off_time = 0,
	.repeat_count = 0
};
/* clang-format on */

static int64_t swEventTime;
static int64_t amrEventTime;

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t UserIfTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:         return Framework_UnknownMsgHandler;
	case FMC_ALIVE:           return AliveMsgHandler;
	case FMC_TAMPER:          return TamperMsgHandler;
	case FMC_EXIT_SHELF_MODE: return ExitShelfModeMsgHandler;
	case FMC_FACTORY_RESET:   return UiFactoryResetMsgHandler;
	case FMC_AMR_LED_ON:      return AmrLedOnMsgHandler;
	case FMC_LEDS_OFF:        return LedsOffMsgHandler;
	default:                  return NULL;
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
	userIfTaskObject.msgTask.timerPeriodTicks = K_MSEC(0);
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

int UserInterfaceTask_LedTest(uint32_t duration)
{
	uint32_t delay = MAX(MINIMUM_LED_TEST_STEP_DURATION_MS, duration);
	LOG_DBG("delay %u", delay);

	lcz_led_turn_on(GREEN_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_on(RED_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_off(GREEN_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_off(RED_LED);
	k_sleep(K_MSEC(delay));

	return 0;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserIfTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	UserIfTaskObj_t *pObj = (UserIfTaskObj_t *)pArg1;

	InitializeButtons();

	lcz_led_init((lcz_led_configuration_t *)LED_CONFIGURATION,
		     ARRAY_SIZE(LED_CONFIGURATION));

#ifdef CONFIG_UI_LED_TEST_ON_RESET
	UserInterfaceTask_LedTest(CONFIG_UI_LED_TEST_ON_RESET_DURATION_MS);
#endif
	/*Check the current state of the tamper switch*/
	TamperSwitchStatus();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static int InitializeButtons(void)
{
	int r = -EPERM;
	const struct device *dev;
	size_t i;

	for (i = 0; i < CONFIG_UI_NUMBER_OF_BUTTONS; i++) {
		dev = device_get_binding(BUTTON_CFG[i].label);
		if (dev == NULL) {
			r = -EPERM;
			break;
		}

		r = gpio_pin_configure(dev, BUTTON_CFG[i].pin,
				       BUTTON_CFG[i].flags);
		BREAK_ON_ERROR(r);

		r = gpio_pin_interrupt_configure(dev, BUTTON_CFG[i].pin,
						 BUTTON_CFG[i].edge);
		BREAK_ON_ERROR(r);

		gpio_init_callback(&button_cb_data[i], BUTTON_CFG[i].isr,
				   BIT(BUTTON_CFG[i].pin));

		r = gpio_add_callback(dev, &button_cb_data[i]);
		BREAK_ON_ERROR(r);
	}

	if (r < 0) {
		LOG_ERR("Configuration failed for %s", BUTTON_CFG[i].label);
	}

	return r;
}
static void TamperSwitchStatus(void)
{
	int v = BSP_PinGet(SW2_PIN);
	if (v >= 0) {
		Attribute_SetUint32(ATTR_INDEX_tamperSwitchStatus, (uint32_t)v);
		Flags_Set(FLAG_TAMPER_SWITCH_STATE, v);
		if (v == 1) {
			SensorEventData_t eventTamper;
			lcz_led_blink(RED_LED, &TAMPER_PATTERN);
			/*Send Event Message*/
			eventTamper.u16 = v;
			SendUIEvent(SENSOR_EVENT_TAMPER, eventTamper);
		} else {
			red_led_off();
		}
	}
}
static void SendUIEvent(SensorEventType_t type, SensorEventData_t data)
{
	EventLogMsg_t *pMsgSend =
		(EventLogMsg_t *)BufferPool_Take(sizeof(EventLogMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_EVENT_TRIGGER;
		pMsgSend->header.txId = FWK_ID_USER_IF_TASK;
		pMsgSend->header.rxId = FWK_ID_EVENT_TASK;
		pMsgSend->eventType = type;
		pMsgSend->eventData = data;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
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

static Dispatch_t AliveMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	LOG_DBG("Button 1");
	/* yellow */
	lcz_led_blink(GREEN_LED, &ALIVE_PATTERN);
	lcz_led_blink(RED_LED, &ALIVE_PATTERN);
	return DISPATCH_OK;
}

static Dispatch_t TamperMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	LOG_DBG("Button 2 (Tamper)");

	TamperSwitchStatus();
	return DISPATCH_OK;
}

static Dispatch_t AmrLedOnMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	lcz_led_turn_on(GREEN_LED);
	return DISPATCH_OK;
}

static Dispatch_t LedsOffMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	lcz_led_turn_off(GREEN_LED);
	lcz_led_turn_off(RED_LED);
	return DISPATCH_OK;
}

static Dispatch_t ExitShelfModeMsgHandler(FwkMsgRxer_t *pMsgRxer,
					  FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	lcz_led_blink(GREEN_LED, &EXIT_SHELF_MODE_PATTERN);
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, FWK_ID_SENSOR_TASK,
				      FMC_ENTER_ACTIVE_MODE);
	//Advertisement_ExtendedSet(false);				  
	return DISPATCH_OK;
}

static Dispatch_t UiFactoryResetMsgHandler(FwkMsgRxer_t *pMsgRxer,
					   FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	uint8_t factoryResetEnabled = 0;
	DispatchResult_t result;
	Attribute_Get(ATTR_INDEX_factoryResetEnable, &factoryResetEnabled,
			    sizeof(factoryResetEnabled));
	if (factoryResetEnabled == 1) {
		/* yellow */
		lcz_led_blink(GREEN_LED, &FACTORY_RESET_PATTERN);
		lcz_led_blink(RED_LED, &FACTORY_RESET_PATTERN);
		/* Forward to control task (instead of broadcasting from ISR) */
		FRAMEWORK_MSG_SEND_TO(FWK_ID_CONTROL_TASK, pMsg);
		result = DISPATCH_DO_NOT_FREE;
	} else {
		/* red, do not perform reset */
		lcz_led_blink(RED_LED, &FACTORY_RESET_PATTERN);
		result = DISPATCH_OK;
	}

	return result;
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void Button1HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	int64_t delta;

	if (gpio_pin_get(dev, BUTTON_CFG[0].pin)) {
		LOG_DBG("Rising");
		delta = k_uptime_delta(&swEventTime);

		if (ValidAliveDuration(delta)) {
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
						      FWK_ID_USER_IF_TASK,
						      FMC_ALIVE);
		}
		if (ValidExitShelfModeDuration(delta)) {
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
						      FWK_ID_USER_IF_TASK,
						      FMC_EXIT_SHELF_MODE);
		}
		if (ValidFactoryResetDuration(delta)) {
			FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
						      FWK_ID_USER_IF_TASK,
						      FMC_FACTORY_RESET);
		}
	} else {
		LOG_DBG("Falling");
		(void)k_uptime_delta(&swEventTime);
	}
}

static void Button2HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, FWK_ID_USER_IF_TASK,
				      FMC_TAMPER);
}

static void Button3HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	FwkMsgCode_t code = FMC_LEDS_OFF;

	if (gpio_pin_get(dev, BUTTON_CFG[2].pin)) {
		LOG_DBG("Rising amr");
		if (ValidExitShelfModeDuration(k_uptime_delta(&amrEventTime))) {
			code = FMC_EXIT_SHELF_MODE;
		}
	} else {
		LOG_DBG("Falling amr");
		(void)k_uptime_delta(&amrEventTime);
		code = FMC_AMR_LED_ON;
	}

	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, FWK_ID_SENSOR_TASK,
				      FMC_MAGNET_STATE);

	FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, code);
}

static bool ValidAliveDuration(int64_t duration)
{
	if (duration < CONFIG_UI_MAX_ALIVE_MS) {
		return true;
	} else {
		return false;
	}
}

static bool ValidExitShelfModeDuration(int64_t duration)
{
	if (duration > CONFIG_UI_PAIR_MIN_MS &&
	    duration < CONFIG_UI_PAIR_MAX_MS) {
		LOG_DBG("exit shelf mode button or AMR");
		return true;
	} else {
		return false;
	}
}

static bool ValidFactoryResetDuration(int64_t duration)
{
	if (duration > CONFIG_UI_MIN_FACTORY_RESET_MS &&
	    duration < CONFIG_UI_MAX_FACTORY_RESET_MS) {
		LOG_DBG("factory reset");
		return true;
	} else {
		return false;
	}
}
