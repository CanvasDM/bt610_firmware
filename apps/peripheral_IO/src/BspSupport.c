/**
 * @file BspSupport.c
 * @brief this is the board support file for defining and configuring the GPIO pins
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
#include "BspSupport.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(BspSupport);
/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static struct device *port0;
static  struct device *port1;
static struct gpio_callback digitalIn1_cb_data;
static struct gpio_callback digitalIn2_cb_data;
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void DigitalIn1HandlerIsr(struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void DigitalIn2HandlerIsr(struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void ConfigureInputs(void);
static void ConfigureOutputs(void);
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void BSP_Init(void)
{
  port0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (!port0) 
  {
    LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio0)));
	}
  port1 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio1)));
	if (!port1) 
  {
    LOG_ERR("Cannot find %s!\n", DT_LABEL(DT_NODELABEL(gpio1)));
	}
  ConfigureInputs();
  ConfigureOutputs();
}
uint16_t BSP_PinSet(uint8_t pin, uint16_t value)
{
  uint16_t gpioReturn;
  switch(pin)
  {
    case THERM_ENABLE_PIN:
    case DO2_PIN:
    case DO1_PIN:
    case BATT_OUT_ENABLE_PIN:
      gpioReturn = gpio_pin_set(port0, GPIO_PIN_MAP(pin), value);
      break;
    case DIN1_ENABLE_PIN:
    case FIVE_VOLT_ENABLE_PIN:
    case DIN2_ENABLE_PIN:
    case ANALOG_ENABLE_PIN:
      gpioReturn = gpio_pin_set(port1, GPIO_PIN_MAP(pin), value);
      break;
    default:
      LOG_ERR("Cannot find output pin\n");
      gpioReturn = ENODEV;	/* No such device */
      break;
  }
  return(gpioReturn);
}
uint16_t BSP_PinToggle(uint8_t pin)
{
  uint16_t gpioReturn;
  switch(pin)
  {
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
      LOG_ERR("Cannot find output pin\n");
      gpioReturn = ENODEV;	/* No such device */
      break;
  }
  return(gpioReturn);
}
uint16_t BSP_PinGet(uint8_t pin, uint16_t value)
{

}

/****Used for hardware test****/
void InitializeDigitalPinsPull(void)
{
  //DIN1
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN1_MCU_PIN), 
                          (GPIO_INPUT|GPIO_PULL_UP));
  gpio_pin_interrupt_configure(port0, GPIO_PIN_MAP(DIN1_MCU_PIN),
					   GPIO_INT_EDGE_BOTH);

  gpio_init_callback(&digitalIn1_cb_data, DigitalIn1HandlerIsr,
			   BIT(GPIO_PIN_MAP(DIN1_MCU_PIN)));
  gpio_add_callback(port0, &digitalIn1_cb_data);

  //DIN2
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN), 
                          (GPIO_INPUT|GPIO_PULL_UP));
  gpio_pin_interrupt_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN),
					   GPIO_INT_EDGE_BOTH);

  gpio_init_callback(&digitalIn2_cb_data, DigitalIn2HandlerIsr,
			   BIT(GPIO_PIN_MAP(DIN2_MCU_PIN)));
  gpio_add_callback(port1, &digitalIn2_cb_data);

}
void InitializeDigitalPinsNoPull(void)
{
  //DIN1
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN1_MCU_PIN), GPIO_INPUT);
  gpio_pin_interrupt_configure(port0, GPIO_PIN_MAP(DIN1_MCU_PIN),
					   GPIO_INT_EDGE_BOTH);

  gpio_init_callback(&digitalIn1_cb_data, DigitalIn1HandlerIsr,
			   BIT(GPIO_PIN_MAP(DIN1_MCU_PIN)));
	gpio_add_callback(port0, &digitalIn1_cb_data);

  //DIN2
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN), GPIO_INPUT);
  gpio_pin_interrupt_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN),
					   GPIO_INT_EDGE_BOTH);

  gpio_init_callback(&digitalIn2_cb_data, DigitalIn2HandlerIsr,
			   BIT(GPIO_PIN_MAP(DIN2_MCU_PIN)));
	gpio_add_callback(port1, &digitalIn2_cb_data);

}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ConfigureInputs(void)
{
  //Port0
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN1_MCU_PIN), GPIO_INPUT);
  //Port1
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_MCU_PIN), GPIO_INPUT);
}
static void ConfigureOutputs(void)
{
  //Port0
  gpio_pin_configure(port0, GPIO_PIN_MAP(BATT_OUT_ENABLE_PIN), GPIO_OUTPUT_LOW);    
  gpio_pin_configure(port0, GPIO_PIN_MAP(DO1_PIN), GPIO_OUTPUT_LOW);
  gpio_pin_configure(port0, GPIO_PIN_MAP(DO2_PIN), GPIO_OUTPUT_LOW);
  gpio_pin_configure(port0, GPIO_PIN_MAP(THERM_ENABLE_PIN), GPIO_OUTPUT_HIGH);

  //Port1
  gpio_pin_configure(port1, GPIO_PIN_MAP(FIVE_VOLT_ENABLE_PIN), GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN2_ENABLE_PIN), GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, GPIO_PIN_MAP(DIN1_ENABLE_PIN), GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, GPIO_PIN_MAP(ANALOG_ENABLE_PIN), GPIO_OUTPUT_LOW);

}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/

void DigitalIn1HandlerIsr(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_DBG("Digital pin%d is %u"  "\n",DIN1_MCU_PIN, 
    gpio_pin_get(port0, GPIO_PIN_MAP(DIN1_MCU_PIN))); 
}
void DigitalIn2HandlerIsr(struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_DBG("Digital pin%d is %u"  "\n",DIN2_MCU_PIN, 
    gpio_pin_get(port1, GPIO_PIN_MAP(DIN2_MCU_PIN)));
    //BSP_PinGet 
}