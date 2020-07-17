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
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void InitializeDigitalPinsNoPull(void);
static void InitializeDigitalPinsPull(void);
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
      gpioReturn = gpio_pin_set(port0, pin, value);
      break;
//    case DIN1_ENABLE_PIN:
//    case FIVE_VOLT_ENABLE_PIN:
//    case DIN2_ENABLE_PIN:
//    case ANALOG_ENABLE_PIN:
//      gpioReturn = gpio_pin_set(port1, pin, value);
//      break;
    default:
      LOG_ERR("Cannot find output pin\n");
      gpioReturn = ENODEV;	/* No such device */
      break;
  }
  return(gpioReturn);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ConfigureInputs(void)
{
  volatile uint8_t testpin;

  testpin = GPIO_PIN_MAP(DIN2_MCU_PIN);
  //Port0
  gpio_pin_configure(port1, DIN1_MCU_PIN, GPIO_INPUT);
  //Port1
  gpio_pin_configure(port1, testpin, GPIO_INPUT);
}
static void ConfigureOutputs(void)
{
  //Port0
  gpio_pin_configure(port0, BATT_OUT_ENABLE_PIN, GPIO_OUTPUT_LOW);    
  gpio_pin_configure(port0, DO1_PIN, GPIO_OUTPUT_LOW);
  gpio_pin_configure(port0, DO2_PIN, GPIO_OUTPUT_LOW);
  gpio_pin_configure(port0, THERM_ENABLE_PIN, GPIO_OUTPUT_HIGH);

  //Port1
  gpio_pin_configure(port1, FIVE_VOLT_ENABLE_PIN, GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, DIN2_ENABLE_PIN, GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, DIN1_ENABLE_PIN, GPIO_OUTPUT_LOW);
  gpio_pin_configure(port1, ANALOG_ENABLE_PIN, GPIO_OUTPUT_LOW);

}