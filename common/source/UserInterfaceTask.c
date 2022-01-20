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
#include "attr.h"
#include "BspSupport.h"
#include "Flags.h"
#include "Advertisement.h"
#include "UserInterfaceTask.h"
#include "LEDs.h"

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
#define USER_IF_TASK_QUEUE_DEPTH 32
#endif

/* IDs used to access buttons in the BUTTON_CFG array */
typedef enum {
	BUTTON_CFG_ID_BOOT = 0,
	BUTTON_CFG_ID_TAMPER,
	BUTTON_CFG_ID_MAG,
	BUTTON_CFG_ID_COUNT
} button_cfg_id;

#define MINIMUM_LED_TEST_STEP_DURATION_MS (10)

/******************************************************************************/
/* Button Configuration                                                       */
/******************************************************************************/
#define BUTTON0_NODE DT_ALIAS(sw0)
#define BUTTON1_NODE DT_ALIAS(sw1)
#define BUTTON2_NODE DT_ALIAS(sw2)

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

static void Button0HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static void Button1HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static void Button2HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins);

static Dispatch_t AliveMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t TamperMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t EnterActiveModeMsgHandler(FwkMsgRxer_t *pMsgRxer,
					    FwkMsg_t *pMsg);
static Dispatch_t UiFactoryResetMsgHandler(FwkMsgRxer_t *pMsgRxer,
					   FwkMsg_t *pMsg);
static Dispatch_t AmrLedOnMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);
static Dispatch_t LedsOffMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg);

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

static const struct button_cfg BUTTON_CFG[] = {
#if DT_HAS_BUTTON_NODE(0)
	BUTTON_CFG(0, (GPIO_INPUT | GPIO_PULL_UP), GPIO_INT_EDGE_BOTH),
#endif
#if DT_HAS_BUTTON_NODE(1)
	BUTTON_CFG(1, (GPIO_INPUT | GPIO_PULL_UP), GPIO_INT_EDGE_BOTH),
#endif
#if DT_HAS_BUTTON_NODE(2)
	BUTTON_CFG(2, GPIO_INPUT, GPIO_INT_EDGE_BOTH),
#endif
};
BUILD_ASSERT(
	ARRAY_SIZE(BUTTON_CFG) == CONFIG_UI_NUMBER_OF_BUTTONS,
	"Unsupported board: sw0, sw1, and sw2 devicetree aliases must be defined");

BUILD_ASSERT(
	CONFIG_UI_MAX_ALIVE_MS < CONFIG_UI_PAIR_MIN_MS,
	"Error: CONFIG_UI_MAX_ALIVE_MS must be less than CONFIG_UI_PAIR_MIN_MS");
BUILD_ASSERT(
	CONFIG_UI_PAIR_MIN_MS < CONFIG_UI_MIN_FACTORY_RESET_MS,
	"Error: CONFIG_UI_PAIR_MIN_MS must be less than CONFIG_UI_MIN_FACTORY_RESET_MS");

static int64_t swEventTime;
static int64_t amrEventTime;

static bool mag_switch_last_state = false;
static bool tamper_switch_last_state = false;

static bool button0_pressed = false;

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t *UserIfTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:                 return Framework_UnknownMsgHandler;
	case FMC_ALIVE:                   return AliveMsgHandler;
	case FMC_TAMPER:                  return TamperMsgHandler;
	case FMC_ENTER_ACTIVE_MODE:       return EnterActiveModeMsgHandler;
	case FMC_FACTORY_RESET:           return UiFactoryResetMsgHandler;
	case FMC_AMR_LED_ON:              return AmrLedOnMsgHandler;
	case FMC_LEDS_OFF:                return LedsOffMsgHandler;
	default:                          return NULL;
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

	led_test(delay);

	return 0;
}

int UserInterfaceTask_UpdateMagSwitchSimulatedStatus(
	bool simulation_enabled, bool last_simulation_enabled)
{
	bool live_input_state;
	const struct device *dev;
	int result = 0;
	bool update_needed = false;
	bool last_simulated_state;

	/* Simulation being enabled? */
	if ((simulation_enabled) && (!last_simulation_enabled)) {
		/* If so, read the current live mag switch state */
		mag_switch_last_state = (bool)BSP_PinGet(MAGNET_MCU_PIN);
		/* Simulation being disabled? */
	} else if ((!simulation_enabled) && (last_simulation_enabled)) {
		/* If simulation has been disabled we need to know
		 * whether we need to update the live value.
		 */
		if ((result = attr_get(ATTR_ID_mag_switch_simulated_value,
				       &last_simulated_state,
				       sizeof(last_simulated_state))) ==
		    sizeof(last_simulated_state)) {
			/* Has there been a change in the live mag switch state? */
			live_input_state = (bool)BSP_PinGet(MAGNET_MCU_PIN);
			if (mag_switch_last_state != live_input_state) {
				/* Yes, so we need to update the system */
				update_needed = true;
			} else {
				/* Or a change in the live and simulated state? */
				if (last_simulated_state != live_input_state) {
					update_needed = true;
				}
			}
		}
		if (update_needed) {
			/* Update needed to the system */
			dev = device_get_binding(
				BUTTON_CFG[BUTTON_CFG_ID_MAG].label);
			Button2HandlerIsr(dev, NULL, 0);
		}
	}
	return (result);
}

int UserInterfaceTask_UpdateMagSwitchSimulatedValue(bool simulated_value,
						    bool last_simulated_value)
{
	int result = 0;
	bool simulation_enabled;
	const struct device *dev;

	/* Simulation enabled? */
	if ((result = attr_get(ATTR_ID_mag_switch_simulated,
			       &simulation_enabled,
			       sizeof(simulation_enabled))) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* Change in value? */
			if (simulated_value != last_simulated_value) {
				dev = device_get_binding(
					BUTTON_CFG[BUTTON_CFG_ID_MAG].label);
				Button2HandlerIsr(dev, NULL, 0);
			}
		}
	}
	return (result);
}

int UserInterfaceTask_UpdateTamperSwitchSimulatedStatus(
	bool simulation_enabled, bool last_simulation_enabled)
{
	bool live_input_state;
	const struct device *dev;
	bool update_needed = false;
	int result = 0;
	bool last_simulated_state;

	/* Simulation being enabled? */
	if ((simulation_enabled) && (!last_simulation_enabled)) {
		/* If so, read the current live tamper switch state */
		tamper_switch_last_state = (bool)BSP_PinGet(SW2_PIN);
		/* Simulation being disabled? */
	} else if ((!simulation_enabled) && (last_simulation_enabled)) {
		/* If so, has there been a change in tamper switch state?
		 * Get the last simulated value.
		 */
		if ((result = attr_get(ATTR_ID_tamper_switch_simulated_value,
				       &last_simulated_state,
				       sizeof(last_simulated_state))) ==
		    sizeof(last_simulated_state)) {
			/* First check if the live input state has changed */
			live_input_state = (bool)BSP_PinGet(SW2_PIN);
			if (tamper_switch_last_state != live_input_state) {
				update_needed = true;
			} else {
				/* And also that the live state is
				 * not different from the last simulated state
				 */
				if (live_input_state != last_simulated_state) {
					update_needed = true;
				}
			}
		}
		if (update_needed) {
			/* Yes, so we need to update the system */
			dev = device_get_binding(
				BUTTON_CFG[BUTTON_CFG_ID_TAMPER].label);
			Button1HandlerIsr(dev, NULL, 0);
		}
	}
	return (result);
}

int UserInterfaceTask_UpdateTamperSwitchSimulatedValue(
	bool simulated_value, bool last_simulated_value)
{
	int result = 0;
	bool simulation_enabled;
	const struct device *dev;

	/* Simulation enabled? */
	if ((result = attr_get(ATTR_ID_tamper_switch_simulated,
			       &simulation_enabled,
			       sizeof(simulation_enabled))) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* Change in value? */
			if (simulated_value != last_simulated_value) {
				dev = device_get_binding(
					BUTTON_CFG[BUTTON_CFG_ID_TAMPER].label);
				Button1HandlerIsr(dev, NULL, 0);
			}
		}
	}
	return (result);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserIfTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	UserIfTaskObj_t *pObj = (UserIfTaskObj_t *)pArg1;

	InitializeButtons();
	InitialiseLEDs();

#ifdef CONFIG_UI_LED_TEST_ON_RESET
	UserInterfaceTask_LedTest(CONFIG_UI_LED_TEST_ON_RESET_DURATION_MS);
#endif
	/* Check the current state of the tamper switch */
	TamperSwitchStatus();

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static int InitializeButtons(void)
{
	int r = -EIO;
	const struct device *dev;
	size_t i;

	for (i = 0; i < CONFIG_UI_NUMBER_OF_BUTTONS; i++) {
		dev = device_get_binding(BUTTON_CFG[i].label);
		if (dev == NULL) {
			r = -EIO;
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
	uint8_t activeMode = 0;
	if (v >= 0) {
		attr_set_uint32(ATTR_ID_tamper_switch_status, (uint32_t)v);
		Flags_Set(FLAG_TAMPER_SWITCH_STATE, v);
		if (v == 1) {
			SensorEventData_t eventTamper;
			/* Send Event Message */
			eventTamper.u16 = v;
			SendUIEvent(SENSOR_EVENT_TAMPER, eventTamper);

			/* Only turn on LED when in active mode */
			attr_get(ATTR_ID_active_mode, &activeMode,
				 sizeof(activeMode));
			if (activeMode) {
				led_blink(LED_COLOR_RED, LED_PATTERN_TAMPER);
			}

		} else {
			led_off(LED_COLOR_RED);
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

static Dispatch_t AliveMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	LOG_DBG("Button 1");
	/* Amber */
	led_blink(LED_COLOR_AMBER, LED_PATTERN_ALIVE);
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
	led_on(LED_COLOR_GREEN);
	return DISPATCH_OK;
}

static Dispatch_t LedsOffMsgHandler(FwkMsgRxer_t *pMsgRxer, FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);
	/* Turning off Amber turns off both LEDs */
	led_off(LED_COLOR_AMBER);
	return DISPATCH_OK;
}

static Dispatch_t EnterActiveModeMsgHandler(FwkMsgRxer_t *pMsgRxer,
					    FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	ARG_UNUSED(pMsg);

	/* This starts the green LED flashing for 30s */
	led_blink(LED_COLOR_GREEN, LED_PATTERN_EXIT_SHELF_MODE);

	/* Set the Active Mode flag - all other activities are
	 * handled by the Sensor Task.
	 */
	attr_set_uint32(ATTR_ID_active_mode, 1);

	return DISPATCH_OK;
}

static Dispatch_t UiFactoryResetMsgHandler(FwkMsgRxer_t *pMsgRxer,
					   FwkMsg_t *pMsg)
{
	ARG_UNUSED(pMsgRxer);
	uint8_t factoryResetEnabled = 0;
	DispatchResult_t result;
	attr_get(ATTR_ID_factory_reset_enable, &factoryResetEnabled,
		 sizeof(factoryResetEnabled));
	if (factoryResetEnabled == 1) {
		/* Amber */
		led_blink(LED_COLOR_AMBER, LED_PATTERN_FACTORY_RESET);
		/* Forward to control task (instead of broadcasting from ISR) */
		FRAMEWORK_MSG_SEND_TO(FWK_ID_CONTROL_TASK, pMsg);
		result = DISPATCH_DO_NOT_FREE;
	} else {
		/* Red, do not perform reset */
		led_blink(LED_COLOR_RED, LED_PATTERN_FACTORY_RESET);
		result = DISPATCH_OK;
	}

	return result;
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void Button0HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	int64_t delta;

	if (gpio_pin_get(dev, BUTTON_CFG[0].pin)) {
		delta = k_uptime_delta(&swEventTime);
		if (button0_pressed == true) {
			LOG_DBG("Rising for %lld", delta);

			/* Sorted by longest to shortest */
			if (ValidFactoryResetDuration(delta)) {
				FRAMEWORK_MSG_CREATE_AND_SEND(
							FWK_ID_USER_IF_TASK,
							FWK_ID_USER_IF_TASK,
							FMC_FACTORY_RESET);
			} else if (ValidExitShelfModeDuration(delta)) {
				FRAMEWORK_MSG_CREATE_AND_SEND(
							FWK_ID_USER_IF_TASK,
							FWK_ID_USER_IF_TASK,
							FMC_ENTER_ACTIVE_MODE);
				LOG_DBG("Active");
			} else if (ValidAliveDuration(delta)) {
				FRAMEWORK_MSG_CREATE_AND_SEND(
							FWK_ID_USER_IF_TASK,
							FWK_ID_USER_IF_TASK,
							FMC_ALIVE);
			}
		} else {
			LOG_ERR("Button0 was released and ignored, no press "
				"was detected. Delta: %lld", delta);
		}
	} else {
		LOG_DBG("Falling");
		(void)k_uptime_delta(&swEventTime);
		button0_pressed = true;
	}
}

static void Button1HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, FWK_ID_USER_IF_TASK,
				      FMC_TAMPER);
}

static void Button2HandlerIsr(const struct device *dev,
			      struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);
	FwkMsgCode_t code = FMC_LEDS_OFF;
	int64_t delta;

	if (gpio_pin_get(dev, BUTTON_CFG[2].pin)) {
		delta = k_uptime_delta(&amrEventTime);
		LOG_DBG("Rising amr");
		if (ValidExitShelfModeDuration(delta)) {
			code = FMC_ENTER_ACTIVE_MODE;
		}
	} else {
		LOG_DBG("Falling amr");
		(void)k_uptime_delta(&amrEventTime);
		code = FMC_AMR_LED_ON;
	}

	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK, FWK_ID_SENSOR_TASK,
				      FMC_MAGNET_STATE);
	FRAMEWORK_MSG_SEND_TO_SELF(FWK_ID_USER_IF_TASK, code);
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
