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
#include <shell/shell.h>
#include <sys/util.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "AdcBt6.h"
#include "BspSupport.h"

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static int samples = 1;
static int delay = 1;

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int batteryMeasurement(const struct shell *shell, size_t argc,
			      char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	int16_t raw = 0;
	int32_t mv = 0;
	int32_t avg_raw = 0;
	int32_t avg_mv = 0;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status = AdcBt6_ReadBatteryMv(&raw, &mv);
		shell_print(shell, "[%u] status: %d raw: %d mv: %d", i, status,
			    raw, mv);
		if (status != 0) {
			break;
		}
		avg_raw += raw;
		avg_mv += mv;
		k_sleep(K_MSEC(delay));
	}
	avg_raw /= samples;
	avg_mv /= samples;
	shell_print(shell, "averages: raw: %d mv: %d", avg_raw, avg_mv);
	return 0;
}

static int sample(const struct shell *shell, int channel,
		  AdcMeasurementType_t type, float (*func)(int32_t))
{
	int16_t raw = 0;
	int32_t avg_raw = 0;
	float conv = 0.0;
	float avg_conv = 0.0;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status = AdcBt6_Measure(&raw, channel, type, MEASURE_SINGLE);
		conv = func(raw);
		shell_print(shell, "[%u] status: %d raw: %d converted: %.4e", i,
			    status, raw, conv);
		if (status != 0) {
			break;
		}
		avg_raw += raw;
		avg_conv += conv;
		k_sleep(K_MSEC(delay));
	}
	avg_raw /= samples;
	avg_conv /= samples;
	shell_print(shell, "averages: raw: %d converted: %.4e", avg_raw,
		    avg_conv);
	return 0;
}

static int vin(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	AdcMeasurementType_t type = ADC_TYPE_ANALOG_VOLTAGE;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, AdcBt6_ConvertVoltage);
}

static int cin(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	AdcMeasurementType_t type = ADC_TYPE_ANALOG_CURRENT;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, AdcBt6_ConvertCurrent);
}

static int ain(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int mask = MAX(0, atoi(argv[1]));
	shell_print(shell, "ch0: %d ch1: %d ch2: %d ch3: %d", mask & 0x1,
		    mask & 0x2, mask & 0x4, mask & 0x8);
	shell_print(shell, "status: %d", AdcBt6_SetAinSelect(mask));
	return 0;
}

static int therm(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	AdcMeasurementType_t type = ADC_TYPE_THERMISTOR;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, AdcBt6_ApplyThermistorCalibration);
}

static int temp(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	AdcMeasurementType_t type = ADC_TYPE_THERMISTOR;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	int rc = sample(shell, ch, type, AdcBt6_ConvertThermToTemperature);
	return rc;
}

static int vref(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = 0; /* don't care */
	AdcMeasurementType_t type = ADC_TYPE_VREF;
	shell_print(shell, "type: %s", AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, AdcBt6_ConvertVref);
}

static int configure(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	samples = MAX(1, atoi(argv[1]));
	delay = MAX(1, atoi(argv[2]));
	shell_print(shell, "samples: %d delay (ms): %d", samples, delay);
	return 0;
}

static int fiveSet(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int value = atoi(argv[1]);
	int rc = (value == 0) ? AdcBt6_FiveVoltDisable() :
				      AdcBt6_FiveVoltEnable();
	shell_print(shell, "Set 5V: %d status: %d", value, rc);
	return 0;
}

static int batterySet(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int value = atoi(argv[1]);
	int rc = (value == 0) ? AdcBt6_BplusDisable() : AdcBt6_BplusEnable();
	shell_print(shell, "Set Battery Enable: %d status: %d", value, rc);
	return 0;
}

static int DigitalOutputToggle(const struct shell *shell, size_t argc,
			       char **argv)
{
	ARG_UNUSED(argc);
	int status1 = BSP_PinToggle(DO1_PIN);
	int status2 = BSP_PinToggle(DO2_PIN);
	shell_print(shell, "Toggle DO1 and DO2 status: %d %d", status1,
		    status2);
	return 0;
}

static int digitalEnable(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int value = atoi(argv[1]);
	int status1 = BSP_PinSet(DIN1_ENABLE_PIN, value);
	int status2 = BSP_PinSet(DIN2_ENABLE_PIN, value);
	shell_print(shell, "Set To DIN_EN: %d %d %d", value, status1, status2);
	return 0;
}

static int cal(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	float c1 = atof(argv[1]);
	float c2 = atof(argv[2]);
	shell_print(shell, "c1 set to %.4e", c1);
	shell_print(shell, "c2 set to %.4e", c2);

	float ge;
	float oe;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status = AdcBt6_CalibrateThermistor(c1, c2, &ge, &oe);
		if (status != 0) {
			shell_print(shell, "error: %d", status);
			break;
		}
		shell_print(shell, "ge: %.4e oe: %.4e", ge, oe);
		k_sleep(K_MSEC(delay));
	}

	return 0;
}

/******************************************************************************/
/* SHELL Service                                                              */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_inputs,
	SHELL_CMD(battery, NULL, "Battery Measurement", batteryMeasurement),
	SHELL_CMD(vin, NULL, "Read analog voltage <channel 0-3>", vin),
	SHELL_CMD(cin, NULL, "Read current <channel 0-3>", cin),
	SHELL_CMD(ain, NULL, "Set analog input select bitmask <0 - 15>", ain),
	SHELL_CMD(therm, NULL, "Read thermistor <channel 0-3>", therm),
	SHELL_CMD(temp, NULL, "Get temperature (therm) <channel 0-3>", temp),
	SHELL_CMD(vref, NULL, "Read vref", vref),
	SHELL_CMD(five, NULL, "Set 5V", fiveSet),
	SHELL_CMD(b_plus, NULL, "Set Battery Out", batterySet),
	SHELL_CMD(toggleDO, NULL, "Toggle DO1 and DO2", DigitalOutputToggle),
	SHELL_CMD(dinEnable, NULL, "Set DIN1_EN and DIN2_EN value",
		  digitalEnable),
	SHELL_CMD(cal, NULL,
		  "Calibrate Thermistor <c1 float> <c2 float>\n"
		  "Example: test cal 220.7351 3.98e3",
		  cal),
	SHELL_CMD(configure, NULL,
		  "Set <number of samples> <delay between samples (ms)>",
		  configure),

	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(test, &sub_inputs, "Test", NULL);