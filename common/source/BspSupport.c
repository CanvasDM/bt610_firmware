/**
 * @file BspSupport.c
 * @brief This is the board support file for defining and configuring the GPIO pins
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#include <logging/log_ctrl.h>
#define LOG_LEVEL
LOG_MODULE_REGISTER(BspSupport, CONFIG_BSP_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>

#include "FrameworkIncludes.h"
#include "BspSupport.h"
#include "Attribute.h"
#include "AttributeTable.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Number of ms waited to shut UART 0 down at start up */
#define BSP_SUPPORT_SHELL_STARTUP_DELAY_MS 10000
/* Number of ms waited to shut UART 0 down at run time */
#define BSP_SUPPORT_SHELL_RUNTIME_DELAY_MS 250

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct device *port0;
static const struct device *port1;
static struct gpio_callback digitalIn1_cb_data;
static struct gpio_callback digitalIn2_cb_data;
#if defined(CONFIG_UART_SHUTOFF)
static struct gpio_callback uart0cts_cb_data;
/* Delayed work queue item used to hold off on shutting the UART down to avoid
 * interfering with shell startup
 */
static struct k_work_delayable uart0_shut_off_delayed_work;
/* This is a copy of the context taken from the shell uart before it gets
 * switched off
 */
void *uart0_ctx = NULL;
#endif
/* Backup of digital input IRQ configuration for simulation purposes */
static gpio_flags_t digital_input_1_IRQ_config = 0;
static gpio_flags_t digital_input_2_IRQ_config = 0;
/* Backup of digital input states when simulation is enabled. These are
 * used to determine whether the system needs to be updated for a change
 * in input state when simulation is disabled
 */
static bool digital_input_1_last_state = false;
static bool digital_input_2_last_state = false;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void DigitalIn1HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);
static void DigitalIn2HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);
#if defined(CONFIG_UART_SHUTOFF)
static void UART0CTSHandlerIsr(const struct device *port,
			       struct gpio_callback *cb, gpio_port_pins_t pins);
static void UART0SetStatus(bool isStartup);
static void UART0WorkqHandler(struct k_work *item);
#ifdef CONFIG_LOG
static void UART0LoggingDisable(void);
static void UART0LoggingEnable(void);
static const struct log_backend *UART0FindBackend(void);
#else
#define UART0LoggingDisable()
#define UART0LoggingEnable()
#endif
#endif

static void ConfigureOutputs(void);
//static void SendDigitalInputStatus(uint16_t pin, uint8_t status);
static void UART0InitialiseSWFlowControl(void);
static void UART1Initialise(void);
static bool MagSwitchIsSimulated(int *simulated_value);
static bool TamperSwitchIsSimulated(int *simulated_value);
static bool DigitalInput1IsSimulated(int *simulated_value);
static bool DigitalInput2IsSimulated(int *simulated_value);
static bool DigitalInputIRQNeeded(bool new_state, bool old_state,
				  gpio_flags_t pin_config);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void BSP_Init(void)
{
	port0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (!port0) {
		LOG_ERR("Cannot find %s!", DT_LABEL(DT_NODELABEL(gpio0)));
	}

	port1 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio1)));
	if (!port1) {
		LOG_ERR("Cannot find %s!", DT_LABEL(DT_NODELABEL(gpio1)));
	}

	ConfigureOutputs();
	UART0InitialiseSWFlowControl();
	UART1Initialise();
}

int BSP_PinSet(uint8_t pin, int value)
{
	int gpioReturn = 0;
	switch (pin) {
	/* Port 0 */
	case THERM_ENABLE_PIN:
	case DO2_PIN:
	case DO1_PIN:
	case BATT_OUT_ENABLE_PIN:
		gpioReturn = gpio_pin_set(port0, GPIO_PIN_MAP(pin), value);
		break;
	/* Port 1 */
	case DIN1_ENABLE_PIN:
	case FIVE_VOLT_ENABLE_PIN:
	case DIN2_ENABLE_PIN:
	case ANALOG_ENABLE_PIN:
		gpioReturn = gpio_pin_set(port1, GPIO_PIN_MAP(pin), value);
		break;
	default:
		LOG_ERR("Cannot find output pin");
		gpioReturn = -ENODEV; /* No such device */
		break;
	}
	return (gpioReturn);
}

int BSP_PinToggle(uint8_t pin)
{
	int gpioReturn;
	switch (pin) {
	case THERM_ENABLE_PIN:
	case DO2_PIN:
	case DO1_PIN:
	case BATT_OUT_ENABLE_PIN:
		gpioReturn = gpio_pin_toggle(port0, GPIO_PIN_MAP(pin));
		break;
	case DIN1_ENABLE_PIN:
	case FIVE_VOLT_ENABLE_PIN:
	case DIN2_ENABLE_PIN:
	case ANALOG_ENABLE_PIN:
		gpioReturn = gpio_pin_toggle(port1, GPIO_PIN_MAP(pin));
		break;
	default:
		LOG_ERR("Cannot find output pin");
		gpioReturn = -ENODEV; /* No such device */
		break;
	}
	return (gpioReturn);
}

int BSP_PinGet(uint8_t pin)
{
	int result;
	bool pin_found = false;

	if (pin > GPIO_PIN_MAX) {
		result = -ENODEV;
	} else {
		if (pin == MAGNET_MCU_PIN) {
			if (MagSwitchIsSimulated(&result)) {
				pin_found = true;
			}
		} else if (pin == SW2_PIN) {
			if (TamperSwitchIsSimulated(&result)) {
				pin_found = true;
			}
		} else if (pin == DIN1_MCU_PIN) {
			if (DigitalInput1IsSimulated(&result)) {
				pin_found = true;
			}
		} else if (pin == DIN2_MCU_PIN) {
			if (DigitalInput2IsSimulated(&result)) {
				pin_found = true;
			}
		}
		if (!pin_found) {
			if (pin > GPIO_PER_PORT) {
				result = gpio_pin_get(port1, GPIO_PIN_MAP(pin));
			} else {
				result = gpio_pin_get(port0, GPIO_PIN_MAP(pin));
			}
		}
	}
	return (result);
}

void BSP_ConfigureDigitalInputs(uint8_t pin, gpio_flags_t enable,
				gpio_flags_t edge)
{
	int r = 0;
	/* DIN1 */
	if (pin == DIN1_MCU_PIN) {
		r = gpio_pin_configure(port0, GPIO_PIN_MAP(DIN1_MCU_PIN),
				       enable);
		LOG_INF("DIN 1 config (%d)", r);
		r = gpio_pin_interrupt_configure(
			port0, GPIO_PIN_MAP(DIN1_MCU_PIN), (edge));
		LOG_INF("DIN 1 interrupt (%d)", r);
		gpio_init_callback(&digitalIn1_cb_data, DigitalIn1HandlerIsr,
				   BIT(GPIO_PIN_MAP(DIN1_MCU_PIN)));
		gpio_add_callback(port0, &digitalIn1_cb_data);
		digital_input_1_IRQ_config = edge;
	}
	/* DIN2 */
	else if (pin == DIN2_MCU_PIN) {
		gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN),
				   GPIO_INPUT);
		gpio_pin_interrupt_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN),
					     (edge));

		gpio_init_callback(&digitalIn2_cb_data, DigitalIn2HandlerIsr,
				   BIT(GPIO_PIN_MAP(DIN2_MCU_PIN)));
		gpio_add_callback(port1, &digitalIn2_cb_data);
		digital_input_2_IRQ_config = edge;
	}
}

int BSP_UpdateDigitalInput1SimulatedStatus(bool simulation_enabled,
					   bool last_simulation_enabled)
{
	bool live_input_state;
	bool irq_needed = false;
	int result = 0;
	bool last_simulated_state;

	/* Are we enabling simulation? */
	if ((simulation_enabled) && (!last_simulation_enabled)) {
		/* If enabling simulation, store the live input
		 * value for use later.
		 */
		digital_input_1_last_state = (bool)BSP_PinGet(DIN1_MCU_PIN);
	}
	/* Or disabling simulation? */
	else if ((!simulation_enabled) && (last_simulation_enabled)) {
		/* If disabling, find out if we need to update
		 * the input state.
		 */
		if ((result = Attribute_Get(
			     ATTR_INDEX_digitalInput1SimulatedValue,
			     &last_simulated_state,
			     sizeof(last_simulated_state))) ==
		    sizeof(last_simulated_state)) {
			/* First check if the actual input state has changed */
			live_input_state = (bool)BSP_PinGet(DIN1_MCU_PIN);
			irq_needed = DigitalInputIRQNeeded(
				live_input_state, digital_input_1_last_state,
				digital_input_1_IRQ_config);
			/* Then if there was a transition from simulated to live */
			if (!irq_needed) {
				irq_needed = DigitalInputIRQNeeded(
					live_input_state, last_simulated_state,
					digital_input_1_IRQ_config);
			}
		}
		if (irq_needed) {
			SendDigitalInputStatus(DIN1_MCU_PIN,
					       (uint8_t)live_input_state);
		}
	}
	return (result);
}

int BSP_UpdateDigitalInput2SimulatedStatus(bool simulation_enabled,
					   bool last_simulation_enabled)
{
	bool live_input_state;
	bool irq_needed = false;
	int result = 0;
	bool last_simulated_state;

	/* Are we enabling simulation? */
	if ((simulation_enabled) && (!last_simulation_enabled)) {
		/* If enabling simulation, store the live input
		 * value for use later.
		 */
		digital_input_2_last_state = (bool)BSP_PinGet(DIN2_MCU_PIN);
	}
	/* Or disabling simulation? */
	else if ((!simulation_enabled) && (last_simulation_enabled)) {
		/* If disabling, find out if we need to update
		 * the input state.
		 */
		if ((result = Attribute_Get(
			     ATTR_INDEX_digitalInput2SimulatedValue,
			     &last_simulated_state,
			     sizeof(last_simulated_state))) ==
		    sizeof(last_simulated_state)) {
			/* First check if the live input state has changed */
			live_input_state = (bool)BSP_PinGet(DIN2_MCU_PIN);
			irq_needed = DigitalInputIRQNeeded(
				live_input_state, digital_input_2_last_state,
				digital_input_2_IRQ_config);
			/* Then the simulated to live value */
			if (!irq_needed) {
				irq_needed = DigitalInputIRQNeeded(
					live_input_state, last_simulated_state,
					digital_input_2_IRQ_config);
			}
		}
		if (irq_needed) {
			SendDigitalInputStatus(DIN2_MCU_PIN,
					       (uint8_t)live_input_state);
		}
	}
	return (result);
}

int BSP_UpdateDigitalInput1SimulatedValue(bool simulated_value,
					  bool last_simulated_value)
{
	int result = 0;
	bool fire_interrupt = false;
	bool simulation_enabled = false;

	if ((result = Attribute_Get(ATTR_INDEX_digitalInput1Simulated,
				    &simulation_enabled,
				    sizeof(simulation_enabled))) ==
	    sizeof(simulation_enabled)) {
		/* Should we apply the value? */
		if (simulation_enabled) {
			/* IRQ needed for this input? */
			fire_interrupt = DigitalInputIRQNeeded(
				simulated_value, last_simulated_value,
				digital_input_1_IRQ_config);

			/* Check if we need to trigger the IRQ */
			if (fire_interrupt) {
				SendDigitalInputStatus(
					DIN1_MCU_PIN, (uint8_t)simulated_value);
			}
		}
	}
	return (result);
}

int BSP_UpdateDigitalInput2SimulatedValue(bool simulated_value,
					  bool last_simulated_value)
{
	int result = 0;
	bool fire_interrupt = false;
	bool simulation_enabled = false;

	if ((result = Attribute_Get(ATTR_INDEX_digitalInput2Simulated,
				    &simulation_enabled,
				    sizeof(simulation_enabled))) ==
	    sizeof(simulation_enabled)) {
		/* Should we apply the value? */
		if (simulation_enabled) {
			/* IRQ needed for this input? */
			fire_interrupt = DigitalInputIRQNeeded(
				simulated_value, last_simulated_value,
				digital_input_2_IRQ_config);

			/* Check if we need to trigger the IRQ */
			if (fire_interrupt) {
				SendDigitalInputStatus(
					DIN2_MCU_PIN, (uint8_t)simulated_value);
			}
		}
	}
	return (result);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ConfigureOutputs(void)
{
	/* Port0 */
	gpio_pin_configure(port0, GPIO_PIN_MAP(BATT_OUT_ENABLE_PIN),
			   GPIO_OUTPUT_LOW);
	gpio_pin_configure(port0, GPIO_PIN_MAP(DO1_PIN), GPIO_OUTPUT_LOW);
	gpio_pin_configure(port0, GPIO_PIN_MAP(DO2_PIN), GPIO_OUTPUT_LOW);
	gpio_pin_configure(port0, GPIO_PIN_MAP(THERM_ENABLE_PIN),
			   GPIO_OUTPUT_HIGH); /* Active low */

	/* Port1 */
	gpio_pin_configure(port1, GPIO_PIN_MAP(FIVE_VOLT_ENABLE_PIN),
			   GPIO_OUTPUT_LOW);
	gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_ENABLE_PIN),
			   GPIO_OUTPUT_HIGH);
	gpio_pin_configure(port1, GPIO_PIN_MAP(DIN1_ENABLE_PIN),
			   GPIO_OUTPUT_HIGH);
	gpio_pin_configure(port1, GPIO_PIN_MAP(ANALOG_ENABLE_PIN),
			   GPIO_OUTPUT_LOW);
}

void SendDigitalInputStatus(uint16_t pin, uint8_t status)
{
	DigitalInMsg_t *pMsgSend =
		(DigitalInMsg_t *)BufferPool_Take(sizeof(DigitalInMsg_t));

	if (pMsgSend != NULL) {
		pMsgSend->header.msgCode = FMC_DIGITAL_IN;
		pMsgSend->header.txId = FWK_ID_RESERVED;
		pMsgSend->header.rxId = FWK_ID_SENSOR_TASK;
		pMsgSend->status = status;
		pMsgSend->pin = pin;
		FRAMEWORK_MSG_SEND(pMsgSend);
	}
}

/** @brief Initialises UART0's software flow control.
 *
 *  NOTE - This relies on the UART CTS & RTS pins being decoupled
 *         from the UART using property delete operations in an
 *         overlay. It will not work if the pins are not decoupled
 *         due to them being under control of the UART.
 */
static void UART0InitialiseSWFlowControl(void)
{
	int r = 0;

	/* RTS line as output and set low. This will permanently signal to
	 * clients data can be sent
	 */
	r = gpio_pin_configure(port0, GPIO_PIN_MAP(UART_0_RTS_PIN),
			       GPIO_OUTPUT_LOW);

	/* CTS line as an input pulled up high */
	if (!r) {
		r = gpio_pin_configure(port0, GPIO_PIN_MAP(UART_0_CTS_PIN),
				       GPIO_INPUT | GPIO_PULL_UP);
	}

#if defined(CONFIG_UART_SHUTOFF)
	/* When this gets pulled down, it indicates a client is connected
	 * and the UART needing to be enabled. When pulled up, the client
	 * is disconnected and the UART switched off.
	 */
	if (!r) {
		r = gpio_pin_interrupt_configure(port0,
						 GPIO_PIN_MAP(UART_0_CTS_PIN),
						 GPIO_INT_EDGE_BOTH);
	}

	if (!r) {
		/* Set up the callback called when the CTS changes */
		gpio_init_callback(&uart0cts_cb_data, UART0CTSHandlerIsr,
				   BIT(GPIO_PIN_MAP(UART_0_CTS_PIN)));

		gpio_add_callback(port0, &uart0cts_cb_data);

		/* Set up the delayed work structure used to disable
		 * the UART
		 */
		k_work_init_delayable(&uart0_shut_off_delayed_work,
				      UART0WorkqHandler);

		UART0SetStatus(true);
	}
#endif
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void DigitalIn1HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins)
{
	int pinStatus = gpio_pin_get(port0, GPIO_PIN_MAP(DIN1_MCU_PIN));

	LOG_DBG("Digital pin%d is %u", DIN1_MCU_PIN, pinStatus);
	SendDigitalInputStatus(DIN1_MCU_PIN, pinStatus);
}

static void DigitalIn2HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins)
{
	int pinStatus = gpio_pin_get(port1, GPIO_PIN_MAP(DIN2_MCU_PIN));

	LOG_DBG("Digital pin%d is %u", DIN2_MCU_PIN, pinStatus);
	SendDigitalInputStatus(DIN2_MCU_PIN, pinStatus);
}

#if defined(CONFIG_UART_SHUTOFF)
/** @brief IRQ handler for changes to UART 0's CTS line. Used to switch the
 *         UART on and off dependent upon the state of the CTS line.
 *
 *  @param [in]port - The port instance where the IRQ occurred.
 *  @param [in]cb - IRQ callback data.
 *  @param [in]pins - Pin status associated with the IRQ.
 */
static void UART0CTSHandlerIsr(const struct device *port,
			       struct gpio_callback *cb, gpio_port_pins_t pins)
{
	/* Update the UART status depending upon the CTS line state */
	UART0SetStatus(false);
}

/** @brief Enables or disables UART 0 depending upon the state of the CTS pin.
 *
 *  @param [in]isStartup - Passed as true during the initial call to the
 *                         function. It was found during testing that if the
 *                         UART was shut off before the Shell had output its
 *                         prompt to the terminal, that it would not restart
 *                         again upon restarting the UART.
 *
 *                         This parameter is used to trigger an initial delay
 *                         before shutting the UART off to allow the Shell to
 *                         finish its initialisation.
 *
 *                         Successive calls to this function pass this
 *                         parameter as false such that no delay is incurred
 *                         for successive calls.
 */
static void UART0SetStatus(bool isStartup)
{
	const struct device *uart_dev;
	int rc = -EINVAL;

	/* Get the port pin status */
	int pinStatus = gpio_pin_get(port0, GPIO_PIN_MAP(UART_0_CTS_PIN));
	/* And details of the UART */
	uart_dev = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));

	if (uart_dev) {
		/* If high, a client has been disconnected and the UART needs
		 * to be shut off.
		 */
		if (pinStatus) {
			/* If we're starting up, we need to allow the shell to
			 * finish starting up. If we don't do this, we won't be
			 * able to reconnect at a later stage.
			 */
			if (isStartup) {
				k_work_reschedule(
					&uart0_shut_off_delayed_work,
					K_MSEC(BSP_SUPPORT_SHELL_STARTUP_DELAY_MS));
			} else {
				/* If we've already started up, a client has
				 * disconnected so we can safely disconnect the
				 * UART with a reduced delay
				 */
				k_work_reschedule(
					&uart0_shut_off_delayed_work,
					K_MSEC(BSP_SUPPORT_SHELL_RUNTIME_DELAY_MS));
			}
		} else {
			/* Don't do anything if we're starting up. If so, the
			 * UART is already enabled and the context pointer will
			 * be NULL. We need to go through procedure to get a
			 * copy of the context data.
			 */
			if (!isStartup) {
				/* If low, a client is connected so enable
				 * the UART
				 */
				rc = pm_device_state_set(
					uart_dev, PM_DEVICE_STATE_ACTIVE, NULL,
					NULL);
				/* Safe to resume logging now */
				UART0LoggingEnable();
			}
		}
	}
}

/** @brief System work queue handler for initial shut off of the UART. Refer to
 *         the UART0SetStatus header for further details.
 *
 *  @param [in]item - Unused pointer to the work item.
 */
static void UART0WorkqHandler(struct k_work *item)
{
	const struct device *uart_dev;

	/* Shut off logging if it's enabled */
	UART0LoggingDisable();

	/* Get details for UART 0 */
	uart_dev = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));

	/* And shut it off */
	if (uart_dev) {
		/* Ignoring the return code here - if it's non-zero the UART is
		 * already off.
		 */
		(void)pm_device_state_set(
			uart_dev, PM_DEVICE_STATE_OFF, NULL,
			NULL);
	}
}

#ifdef CONFIG_LOG
/** @brief Disables the logging subsystem backend UART.
 *
 */
static void UART0LoggingDisable(void)
{
	const struct log_backend *uart0_backend;

	uart0_backend = UART0FindBackend();
	if (uart0_backend != NULL) {
		/* Store the context before disabling */
		uart0_ctx = uart0_backend->cb->ctx;
		/* Then disable it */
		log_backend_disable(uart0_backend);
	}
}

/** @brief Enables the logging subsystem backend UART.
 *
 */
static void UART0LoggingEnable(void)
{
	const struct log_backend *uart0_backend;

	uart0_backend = UART0FindBackend();
	if (uart0_backend != NULL) {
		log_backend_enable(uart0_backend, uart0_ctx,
				   CONFIG_LOG_MAX_LEVEL);
	}
}

/** @brief Finds the logging subsystem backend pointer for UART0.
 *
 *  @returns A pointer to the UART0 backend, NULL if not found.
 */
static const struct log_backend *UART0FindBackend(void)
{
	const struct log_backend *backend;
	const struct log_backend *uart0_backend = NULL;
	uint32_t backend_idx;
	bool null_backend_found = false;

	for (backend_idx = 0;
	     (uart0_backend == NULL) && (null_backend_found == false);
	     backend_idx++) {
		/* Get the next backend */
		backend = log_backend_get(backend_idx);
		/* If it's NULL, stop here */
		if (backend != NULL) {
			/* Is it UART0? */
			if (!strcmp(backend->name, "shell_uart_backend")) {
				/* Found it */
				uart0_backend = backend;
			}
		} else {
			null_backend_found = true;
		}
	}

	return (uart0_backend);
}
#endif
#endif

/** @brief Disables UART1 for this revision - note in successive releases this
 *         will need to be updated depending upon the functionality required.
 *         Added here to tidy up after MCU Boot.
 */
static void UART1Initialise(void)
{
	const struct device *uart_dev;

	/* Get details for UART 1 */
	uart_dev = device_get_binding(DT_LABEL(DT_NODELABEL(uart1)));

	/* And shut it off */
	if (uart_dev){

		/* Ignoring the return code here - if it's non-zero the UART is
		 * already off.
		 */
		(void)pm_device_state_set(
			uart_dev, PM_DEVICE_STATE_OFF, NULL,
			NULL);
	}
}

static bool MagSwitchIsSimulated(int *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;
	bool mag_switch_state;

	/* First check we can read back the simulation enabled state and
	 * that it's enabled.
	 */
	if (Attribute_Get(ATTR_INDEX_magSwitchSimulated, &simulation_enabled,
			  sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (Attribute_Get(ATTR_INDEX_magSwitchSimulatedValue,
					  &mag_switch_state,
					  sizeof(mag_switch_state)) ==
			    sizeof(mag_switch_state)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
				*simulated_value = (int)mag_switch_state;
			}
		}
	}

	return (is_simulated);
}

static bool TamperSwitchIsSimulated(int *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;
	bool tamper_switch_state;

	/* First check we can read back the simulation enabled state and
	 * that it's enabled.
	 */
	if (Attribute_Get(ATTR_INDEX_tamperSwitchSimulated, &simulation_enabled,
			  sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (Attribute_Get(ATTR_INDEX_tamperSwitchSimulatedValue,
					  &tamper_switch_state,
					  sizeof(tamper_switch_state)) ==
			    sizeof(tamper_switch_state)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
				*simulated_value = (int)tamper_switch_state;
			}
		}
	}

	return (is_simulated);
}

static bool DigitalInput1IsSimulated(int *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;
	bool simulated_input_state;

	/* First check we can read back the simulation enabled state and
	 * that it's enabled.
	 */
	if (Attribute_Get(ATTR_INDEX_digitalInput1Simulated,
			  &simulation_enabled, sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (Attribute_Get(ATTR_INDEX_digitalInput1SimulatedValue,
					  &simulated_input_state,
					  sizeof(simulated_input_state)) ==
			    sizeof(simulated_input_state)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
				*simulated_value = (int)simulated_input_state;
			}
		}
	}

	return (is_simulated);
}

static bool DigitalInput2IsSimulated(int *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;
	bool simulated_input_state;

	if (Attribute_Get(ATTR_INDEX_digitalInput2Simulated,
			  &simulation_enabled, sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (Attribute_Get(ATTR_INDEX_digitalInput2SimulatedValue,
					  &simulated_input_state,
					  sizeof(simulated_input_state)) ==
			    sizeof(simulated_input_state)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
				*simulated_value = (int)simulated_input_state;
			}
		}
	}

	return (is_simulated);
}

static bool DigitalInputIRQNeeded(bool new_state, bool old_state,
				  gpio_flags_t pin_config)
{
	bool irq_needed = false;

	/* IRQ enabled for this input? */
	if (pin_config & GPIO_INT_ENABLE) {
		/* Was there a change? */
		if (new_state != old_state) {
			/* Is the interrupt for either direction? */
			if (pin_config & GPIO_INT_EDGE_BOTH) {
				irq_needed = true;
			}
			/* Low to high transition and rising edge? */
			else if (pin_config & GPIO_INT_EDGE_RISING) {
				if ((old_state == false) &&
				    (new_state == true)) {
					irq_needed = true;
				}
			}
			/* High to low transition and falling edge? */
			else if (pin_config & GPIO_INT_EDGE_FALLING) {
				if ((old_state == true) &&
				    (new_state == false)) {
					irq_needed = true;
				}
			}
		}
	}

	return (irq_needed);
}
