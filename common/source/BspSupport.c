/**
 * @file BspSupport.c
 * @brief This is the board support file for defining and configuring the GPIO pins
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
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
/* Local Data Definitions                                                     */
/******************************************************************************/
static const struct device *port0;
static const struct device *port1;
static struct gpio_callback digitalIn1_cb_data;
static struct gpio_callback digitalIn2_cb_data;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void DigitalIn1HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);
static void DigitalIn2HandlerIsr(const struct device *port,
				 struct gpio_callback *cb,
				 gpio_port_pins_t pins);

static void ConfigureOutputs(void);
//static void SendDigitalInputStatus(uint16_t pin, uint8_t status);

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