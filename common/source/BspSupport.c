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
static struct gpio_callback uart0cts_cb_data;
/* Delayed work queue item used to hold off on shutting the UART */
/* down to avoid interfering with shell startup                  */
static struct k_delayed_work uart0_shut_off_delayed_work;
/* This is a copy of the context taken from the shell uart before */
/* it gets switched off.                                          */
void *uart0_ctx = NULL;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void DigitalIn1HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);
static void DigitalIn2HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);
static void UART0CTSHandlerIsr(const struct device *port,
				struct gpio_callback *cb,
				gpio_port_pins_t pins);

static void ConfigureOutputs(void);
//static void SendDigitalInputStatus(uint16_t pin, uint8_t status);

static void UART0InitialiseSWFlowControl(void);
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
}

int BSP_PinSet(uint8_t pin, int value)
{
	int gpioReturn = 0;
	switch (pin) {
	/*Port 0*/
	case THERM_ENABLE_PIN:
	case DO2_PIN:
	case DO1_PIN:
	case BATT_OUT_ENABLE_PIN:
		gpioReturn = gpio_pin_set(port0, GPIO_PIN_MAP(pin), value);
		break;
	/*Port 1*/
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
	if (pin > GPIO_PIN_MAX) {
		return -ENODEV;
	} else if (pin > GPIO_PER_PORT) {
		return gpio_pin_get(port1, GPIO_PIN_MAP(pin));
	} else {
		return gpio_pin_get(port0, GPIO_PIN_MAP(pin));
	}
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
	}
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
			   GPIO_OUTPUT_HIGH); /* active low */

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

	/* RTS line as output and set low */
	/* This will permanently signal to clients data can be sent */
	r = gpio_pin_configure(port0, GPIO_PIN_MAP(UART_0_RTS_PIN),
			       GPIO_OUTPUT_LOW);

	/* CTS line as an input pulled up high */
	if (!r){

		r = gpio_pin_configure(port0, GPIO_PIN_MAP(UART_0_CTS_PIN),
					GPIO_INPUT | GPIO_INT_DISABLE |
					GPIO_PULL_UP);
	}

	/* When this gets pulled down, it indicates a client is connected */
	/* and the UART needing to be enabled. When pulled up, the client */
	/* is disconnected and the UART switched off. */
	if (!r){

		r = gpio_pin_interrupt_configure(port0, 
						GPIO_PIN_MAP(UART_0_CTS_PIN),
						GPIO_INT_EDGE_BOTH);
	}

	if (!r){

		/* Set up the callback called when the CTS changes   */
		gpio_init_callback(&uart0cts_cb_data, UART0CTSHandlerIsr,
				   BIT(GPIO_PIN_MAP(UART_0_CTS_PIN)));

		gpio_add_callback(port0, &uart0cts_cb_data);

		/* Set up the delayed work structure used to disable */
		/* the UART                                          */
		k_delayed_work_init(&uart0_shut_off_delayed_work,
							UART0WorkqHandler);

		UART0SetStatus(true);
	}
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

/** @brief IRQ handler for changes to UART 0's CTS line. Used to switch the
 *         UART on and off dependent upon the state of the CTS line.
 *
 *  @param [in]port - The port instance where the IRQ occurred.
 *  @param [in]cb - IRQ callback data.
 *  @param [in]pins - Pin status associated with the IRQ.
 */
static void UART0CTSHandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins)
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

	if (uart_dev){

		/* If high, a client has been disconnected and the    */
		/* UART needs to be shut off.                         */
		if (pinStatus){
			/* If we're starting up, we need to allow the */
			/* shell to finish starting up. If we don't   */
			/* do this, we won't be able to reconnect at  */
			/* a later stage.                             */
			if (isStartup){

				k_delayed_work_submit(&uart0_shut_off_delayed_work,
					K_MSEC(BSP_SUPPORT_SHELL_STARTUP_DELAY_MS));
			}
			else{
				/* If we've already started up, a client has */
				/* disconnected so we can safely disconnect  */
				/* the UART with a reduced delay             */
				k_delayed_work_submit(&uart0_shut_off_delayed_work,
					K_MSEC(BSP_SUPPORT_SHELL_RUNTIME_DELAY_MS));
			}
		}
		else{
			/* Don't do anything if we're starting up. If so, */
			/* the UART is already enabled and the context    */
			/* pointer will be NULL. We need to go through    */
			/* procedure to get a copy of the context data.   */
			if (!isStartup){

				/* If low, a client is connected so       */
				/* enable the UART                        */
				rc = device_set_power_state(uart_dev,
							DEVICE_PM_ACTIVE_STATE,
							NULL,
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
static void UART0WorkqHandler(struct k_work *item){

	const struct device *uart_dev;

	/* Shut off logging if it's enabled */
	UART0LoggingDisable();

	/* Get details for UART 0 */
	uart_dev = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));

	/* And shut it off */
	if (uart_dev){

		/* Ignoring the return code here - if it's non-zero */
		/* the UART is already off.                         */
		(void)device_set_power_state(uart_dev,
						DEVICE_PM_OFF_STATE,
						NULL,
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
	if (uart0_backend != NULL){
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
	if (uart0_backend != NULL){
		log_backend_enable(uart0_backend,
					uart0_ctx,
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
		(uart0_backend == NULL)&&
		(null_backend_found == false);
			backend_idx++){
		/* Get the next backend */
		backend = log_backend_get(backend_idx);
		/* If it's NULL, stop here */
		if (backend != NULL){
			/* Is it UART0? */
			if (!strcmp(backend->name,"shell_uart_backend")){
				/* Found it */
				uart0_backend = backend;
			}
		}
		else{
			null_backend_found = true;
		}
	}
	return(uart0_backend);
}
#endif
