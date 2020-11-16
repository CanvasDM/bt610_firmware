/**
 * @file TestMenu.c
 * @brief These are shell commands are used for testing
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(TestMenu);

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
#include <shell/shell.h>
#include <shell/shell_uart.h>
#include <stdlib.h>
#include "AdcRead.h"
#include "BspSupport.h"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static  AnalogInput_t AnalogType;

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int enableAnalogPin(const struct shell *shell, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    uint8_t enabled = 0;
	enabled = atoi(argv[1]);

	SetEnablePinMsg_t *pMsg =
		(SetEnablePinMsg_t *)BufferPool_Take(sizeof(SetEnablePinMsg_t));
	if (pMsg != NULL) 
	{
    	pMsg->header.msgCode = FMC_CONTROL_ENABLE;
    	pMsg->header.txId = FWK_ID_USER_IF_TASK;
    	pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;
    	pMsg->control.analogEnable = enabled; 
    	pMsg->control.thermEnable = 1; //disable
    	FRAMEWORK_MSG_SEND(pMsg);
  	} 
  
	shell_print(shell, "ANALOG_EN = %d \n", enabled);

	return (0);
}
static int batteryMeasurement(const struct shell *shell, size_t argc,
			      char **argv)
{
    ARG_UNUSED(argc);
	  ARG_UNUSED(argv);
    static volatile uint32_t adcTestValue = 0;
    uint8_t index = 0;
    uint8_t maxReadings = 10;

	for (index = 0; index < maxReadings; index++) 
	{
    	adcTestValue = ADC_GetBatteryMv();
    	shell_print(shell,"Battery%d = %d \n", index, adcTestValue);
  	}

	return (0);
}
static int readAin(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
	uint8_t ainSelected = 0;
	ainSelected = atoi(argv[1]);

	if (AnalogType == UNKOWN_INPUT) 
	{
    	shell_print(shell, "Analog Type not set");
  	}
  	else if( (ainSelected == 0) || (ainSelected > 4))
  	{
    	shell_print(shell, "Analog out of bounds");
  	}
  	else
  	{
    	AnalogPinMsg_t *pMsg = (AnalogPinMsg_t *)BufferPool_Take(
    	sizeof(AnalogPinMsg_t));
    	if(pMsg != NULL)
    	{
      		pMsg->header.msgCode = FMC_ANALOG_INPUT;
      		pMsg->header.txId = FWK_ID_USER_IF_TASK;
      		pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;  
      		pMsg->inputConfig = AnalogType; 
      		pMsg->externalPin = ainSelected;      
      		FRAMEWORK_MSG_SEND(pMsg);
    	} 

    	shell_print(shell, "Analog pin %d", ainSelected);
  	}

	return (0);
}
static int analogVoltage(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	ARG_UNUSED(argv);
  	AnalogType = VOLTAGE_AIN;

  	shell_print(shell, "Set To Voltage\n");

	return (0);
}
static int analogCurrent(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	ARG_UNUSED(argv);
  	AnalogType = CURRENT_AIN;

  	shell_print(shell, "Set To Current\n");

	return (0);
}
static int enableThermistorPin(const struct shell *shell, size_t argc,
			       char **argv)
{
  	ARG_UNUSED(argc);
  	uint8_t enabled = 0;
	enabled = atoi(argv[1]);
  	AnalogType = THERMISTOR;

	SetEnablePinMsg_t *pMsg =
		(SetEnablePinMsg_t *)BufferPool_Take(sizeof(SetEnablePinMsg_t));
	if (pMsg != NULL) 
	{
    	pMsg->header.msgCode = FMC_CONTROL_ENABLE;
    	pMsg->header.txId = FWK_ID_USER_IF_TASK;
    	pMsg->header.rxId = FWK_ID_ANALOG_SENSOR_TASK;
    	pMsg->control.analogEnable = 0; 
    	pMsg->control.thermEnable = enabled;
    	FRAMEWORK_MSG_SEND(pMsg);
  	} 
	shell_print(shell, "THERM_EN = %d \n", enabled);

	return (0);
}
static int readVrefPin(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	uint32_t adcVrefValue = 0;
  	uint8_t index = 0;
  	uint8_t maxReadings = 10;

	for (index = 0; index < maxReadings; index++) 
	{
    	adcVrefValue = AnalogRead(VREF_5_CH);
		shell_print(shell, "Vref = %d \n", adcVrefValue);
  	}

	return (0);
}
static int readTherm(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
	uint8_t thermSelected = 0;
	thermSelected = atoi(argv[1]);

	if ((thermSelected == 0) || (thermSelected > 4)) 
	{
    	shell_print(shell, "Analog out of bounds");
	} 
	else 
	{
		AnalogPinMsg_t *pMsg = (AnalogPinMsg_t *)BufferPool_Take(
			sizeof(AnalogPinMsg_t));
		if (pMsg != NULL) 
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

	return (0);
}
static int fiveEnable(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	uint8_t enable = 0;
	enable = atoi(argv[1]);

  	shell_print(shell, "Set To 5V\n");
  	BSP_PinSet(FIVE_VOLT_ENABLE_PIN, enable);

	return (0);
}
static int batteryEnable(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	uint8_t enable = 0;
	enable = atoi(argv[1]);

  	shell_print(shell, "Set To Battery Enable\n");
  	BSP_PinSet(BATT_OUT_ENABLE_PIN, enable);

	return (0);
}

static int DOtoggle(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	uint8_t enable = 0;
	enable = atoi(argv[1]);

  	shell_print(shell, "Toggle DO1 and DO2\n");
 	// gpio_pin_set(port0, DO1_PIN, 1);
  	BSP_PinToggle(DO1_PIN);
  	BSP_PinToggle(DO2_PIN);

	return (0);
}
static int digitalEnable(const struct shell *shell, size_t argc, char **argv)
{
  	ARG_UNUSED(argc);
  	uint8_t enable = 0;
	enable = atoi(argv[1]);

  	shell_print(shell, "Set To DIN_EN = %d\n", enable);
   	BSP_PinSet(DIN1_ENABLE_PIN, enable);
  	BSP_PinSet(DIN2_ENABLE_PIN, enable);
 
	return (0);
}

/******************************************************************************/
/* SHELL Service                                                  */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_inputs, 
	SHELL_CMD(enableAlog, NULL, "Enable/Disable Analog", enableAnalogPin),
  	SHELL_CMD(battery, NULL, "Take Battery Measure", batteryMeasurement),
  	SHELL_CMD(ain, NULL, "Read AINx", readAin),  
  	SHELL_CMD(voltage, NULL, "Voltage Input", analogVoltage),
  	SHELL_CMD(current, NULL, "Current Input", analogCurrent),
	SHELL_CMD(enableTherm, NULL, "Enable/Disable Thermistor",
		  enableThermistorPin),
  	SHELL_CMD(readVREF, NULL, "Read VREF ADC value", readVrefPin), 
  	SHELL_CMD(therm, NULL, "Read Thermx", readTherm),
  	SHELL_CMD(enable5v, NULL, "Enable 5V", fiveEnable),
  	SHELL_CMD(enableBatt, NULL, "Enable Battery Out", batteryEnable),
  	SHELL_CMD(toggleDO, NULL, "Toggle DO1 and DO2", DOtoggle),
	SHELL_CMD(enableDin, NULL, "Set DIN1_EN and DIN2_EN value",
		  digitalEnable),
  
  	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(Test, &sub_inputs, "Test", NULL);