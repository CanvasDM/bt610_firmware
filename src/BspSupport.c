/**
 * @file BspSupport.c
 * @brief This is the board support file for defining and configuring the GPIO pins
 *
 * Copyright (c) 2020-2021 Laird Connectivity
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
#include <pm/device.h>

#include "FrameworkIncludes.h"
#include "BspSupport.h"
#include "attr.h"
#include "attr_table.h"
#include "BleTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
/* Time (in ms) to delay checking the CTS pin state */
#define BSP_SUPPORT_UART_CTS_DEBOUNCE_TIME_MS 50
/* Time (in ms) before disabling the UART when CTS is inactive */
#define BSP_SUPPORT_UART_DISABLE_UART_TIMER_MS 5000
/* Name of UART shell for logging backend */
#define BSP_SUPPORT_LOGGER_UART_NAME "shell_uart_backend"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct device *port0;
static const struct device *port1;
static const struct device *uart0_dev;
static const struct device *uart1_dev;
static struct gpio_callback digitalIn1_cb_data;
static struct gpio_callback digitalIn2_cb_data;
#if defined(CONFIG_UART_SHUTOFF)
static struct gpio_callback uart0cts_cb_data;
static const struct log_backend *uart0LogBackend;
static bool uart0_on;
static bool restore_logging;
static struct k_work_delayable uart0_cts_debounce_delayed_work;
static struct k_work_delayable uart0_shut_off_delayed_work;
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
static void UART0WorkqHandler(struct k_work *item);
#endif
static void ConfigureOutputs(void);
static void SendDigitalInputStatus(uint16_t pin, uint8_t status);
static void UART0Initialise(void);
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
	port0 = device_get_binding(DEVICE_DT_NAME(DT_NODELABEL(gpio0)));
	if (!port0) {
		LOG_ERR("Cannot find %s!", DEVICE_DT_NAME(DT_NODELABEL(gpio0)));
	}

	port1 = device_get_binding(DEVICE_DT_NAME(DT_NODELABEL(gpio1)));
	if (!port1) {
		LOG_ERR("Cannot find %s!", DEVICE_DT_NAME(DT_NODELABEL(gpio1)));
	}

	uart0_dev = device_get_binding(DEVICE_DT_NAME(DT_NODELABEL(uart0)));
	if (!uart0_dev) {
		LOG_ERR("Cannot find %s!", DEVICE_DT_NAME(DT_NODELABEL(uart0)));
	}

	uart1_dev = device_get_binding(DEVICE_DT_NAME(DT_NODELABEL(uart1)));
	if (!uart1_dev) {
		LOG_ERR("Cannot find %s!", DEVICE_DT_NAME(DT_NODELABEL(uart1)));
	}

	ConfigureOutputs();
	UART0Initialise();
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
	const struct device *din_port = NULL;
	void *isr_function;
	struct gpio_callback *callback_data;
	gpio_flags_t *pin_irq_conf;

	if (pin == DIN1_MCU_PIN) {
		/* DIN1 */
		din_port = port0;
		isr_function = DigitalIn1HandlerIsr;
		callback_data = &digitalIn1_cb_data;
		pin_irq_conf = &digital_input_1_IRQ_config;
	} else if (pin == DIN2_MCU_PIN) {
		/* DIN2 */
		din_port = port1;
		isr_function = DigitalIn2HandlerIsr;
		callback_data = &digitalIn2_cb_data;
		pin_irq_conf = &digital_input_2_IRQ_config;
	} else {
		LOG_ERR("Invalid pin for digital input config: %d", pin);
		return;
	}

	if (din_port == NULL) {
		LOG_ERR("Port for pin %d is null", pin);
		return;
	}

	r = gpio_pin_configure(din_port, GPIO_PIN_MAP(pin), enable);

	if (r == 0) {
		r = gpio_pin_interrupt_configure(din_port, GPIO_PIN_MAP(pin),
						 edge);
	}

	if (r == 0) {
		gpio_init_callback(callback_data, isr_function,
				   BIT(GPIO_PIN_MAP(pin)));

		r = gpio_add_callback(din_port, callback_data);

		*pin_irq_conf = edge;
	}

	if (r != 0) {
		LOG_ERR("Failed configuring pin %d: %d", pin, r);
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
		if ((result = attr_get(ATTR_ID_digital_input_1_simulated_value,
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
		if ((result = attr_get(ATTR_ID_digital_input_2_simulated_value,
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

	if ((result = attr_get(ATTR_ID_digital_input_1_simulated,
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

	if ((result = attr_get(ATTR_ID_digital_input_2_simulated,
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

static void SendDigitalInputStatus(uint16_t pin, uint8_t status)
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

static void UART0Initialise(void)
{
	int r = 0;

#if defined(CONFIG_UART_SHUTOFF)
	/* When this gets pulled down, it indicates a client is connected
	 * and the UART needing to be enabled. When pulled up, the client
	 * is disconnected and the UART switched off.
	 */
	int backends = log_backend_count_get();
	int i = 0;
	while (i < backends) {
		uart0LogBackend = log_backend_get(i);
		if (strcmp(uart0LogBackend->name,
			   BSP_SUPPORT_LOGGER_UART_NAME) == 0) {
			/* Found UART shell log interface */
			break;
		}
		++i;
	}

	if (i == backends) {
		LOG_ERR("Could not find UART log backend");
		uart0LogBackend = NULL;
	}

	k_work_init_delayable(&uart0_shut_off_delayed_work, UART0WorkqHandler);
	k_work_init_delayable(&uart0_cts_debounce_delayed_work, UART0WorkqHandler);

	r = gpio_pin_interrupt_configure(port0, GPIO_PIN_MAP(UART_0_CTS_PIN), GPIO_INT_EDGE_BOTH);
	if (r < 0) {
		LOG_ERR("Could not config CTS interrupt");
	}
	gpio_init_callback(&uart0cts_cb_data, UART0CTSHandlerIsr,
			   BIT(GPIO_PIN_MAP(UART_0_CTS_PIN)));
	r = gpio_add_callback(port0, &uart0cts_cb_data);
	if (r < 0) {
		LOG_ERR("Could not add CTS callback");
	}

	uart0_on = true;
	/* trigger fist check of UART0 state */
	k_work_reschedule(&uart0_cts_debounce_delayed_work,
			  K_MSEC(BSP_SUPPORT_UART_CTS_DEBOUNCE_TIME_MS));

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

	LOG_DBG("Digital pin %d is %u", DIN1_MCU_PIN, pinStatus);
	SendDigitalInputStatus(DIN1_MCU_PIN, pinStatus);
}

static void DigitalIn2HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins)
{
	int pinStatus = gpio_pin_get(port1, GPIO_PIN_MAP(DIN2_MCU_PIN));

	LOG_DBG("Digital pin %d is %u", DIN2_MCU_PIN, pinStatus);
	SendDigitalInputStatus(DIN2_MCU_PIN, pinStatus);
}

#if defined(CONFIG_UART_SHUTOFF)

/** @brief IRQ handler for changes to UART 0's CTS line. Used to switch the
 *	   UART on and off dependent upon the state of the CTS line.
 *
 *  @param [in]port - The port instance where the IRQ occurred.
 *  @param [in]cb - IRQ callback data.
 *  @param [in]pins - Pin status associated with the IRQ.
 */
static void UART0CTSHandlerIsr(const struct device *port,
			       struct gpio_callback *cb, gpio_port_pins_t pins)
{
	k_work_reschedule(&uart0_cts_debounce_delayed_work,
			  K_MSEC(BSP_SUPPORT_UART_CTS_DEBOUNCE_TIME_MS));
}

/** @brief System work queue handler for initial shut off of the UART.
 *
 *  @param [in]item - Unused pointer to the work item.
 */
static void UART0WorkqHandler(struct k_work *item)
{
	struct k_work_delayable *wi = CONTAINER_OF(item, struct k_work_delayable, work);
	int pin_status_cts = gpio_pin_get_raw(port0, GPIO_PIN_MAP(UART_0_CTS_PIN));

	if (wi == &uart0_shut_off_delayed_work) {
		if (uart0_dev && pin_status_cts) {
			LOG_WRN("Shut off uart0");
			if (uart0LogBackend != NULL) {
				restore_logging = log_backend_is_active(uart0LogBackend);
				log_backend_deactivate(uart0LogBackend);
			}

			(void)pm_device_action_run(uart0_dev, PM_DEVICE_ACTION_SUSPEND);
			uart0_on = false;
		}
	} else if (wi == &uart0_cts_debounce_delayed_work) {
		if (pin_status_cts) {
			k_work_reschedule(&uart0_shut_off_delayed_work,
					  K_MSEC(BSP_SUPPORT_UART_DISABLE_UART_TIMER_MS));
		} else if (!uart0_on) {
			k_work_cancel_delayable(&uart0_shut_off_delayed_work);
			LOG_WRN("Turn on uart0");
			(void)pm_device_action_run(uart0_dev, PM_DEVICE_ACTION_RESUME);
			if ((uart0LogBackend != NULL) && restore_logging) {
				log_backend_activate(uart0LogBackend, uart0LogBackend->cb->ctx);
			}
			uart0_on = true;
		}
	}
}
#endif

/** @brief Disables UART1 for this revision - note in successive releases this
 *         will need to be updated depending upon the functionality required.
 *         Added here to tidy up after MCU Boot.
 */
static void UART1Initialise(void)
{
	/* Shut it off */
	if (uart1_dev) {
		/* Ignoring the return code here - if it's non-zero the UART is
		 * already off.
		 */
		(void)pm_device_action_run(uart1_dev, PM_DEVICE_ACTION_SUSPEND);
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
	if (attr_get(ATTR_ID_mag_switch_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_mag_switch_simulated_value,
				     &mag_switch_state,
				     sizeof(mag_switch_state)) ==
			    sizeof(mag_switch_state)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
				/* Also map the simulated value to the expected
				 * pin state. A simulated value of true sets a
				 * magnet state of Near, but this maps to a pin
				 * level of 0 or false.
				 */
				mag_switch_state = !mag_switch_state;
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
	if (attr_get(ATTR_ID_tamper_switch_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_tamper_switch_simulated_value,
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
	if (attr_get(ATTR_ID_digital_input_1_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_digital_input_1_simulated_value,
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

	if (attr_get(ATTR_ID_digital_input_2_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_digital_input_2_simulated_value,
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
