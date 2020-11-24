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
static int samples = 10;
static int delay = 1;

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int32_t get_fraction(float f)
{
	int32_t a = (int32_t)f;
	int32_t b = (int32_t)((f - a) * 1000);
	return b;
}

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
		  enum AdcMeasurementType type, int32_t factor,
		  float (*func)(int32_t))
{
	int16_t raw = 0;
	float foo = 0.0;
	int32_t avg_raw = 0;
	float avg_foo = 0.0;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status = AdcBt6_Measure(&raw, channel, type);
		foo = func(raw * factor);
		shell_print(shell, "[%u] status: %d raw: %d converted: %d.%d",
			    i, status, raw, (int32_t)foo, get_fraction(foo));
		if (status != 0) {
			break;
		}
		avg_raw += raw;
		avg_foo += foo;
		k_sleep(K_MSEC(delay));
	}
	avg_raw /= samples;
	avg_foo /= samples;
	shell_print(shell, "averages: raw: %d converted: %d.%d", avg_raw,
		    (int32_t)avg_foo, get_fraction(avg_foo));
	return 0;
}

static int vin(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	enum AdcMeasurementType type = ADC_TYPE_ANALOG_VOLTAGE;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, 1000, AdcBt6_ConvertVoltage);
}

static int cin(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	enum AdcMeasurementType type = ADC_TYPE_ANALOG_CURRENT;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, 1000, AdcBt6_ConvertCurrent);
}

static int therm(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = MAX(0, atoi(argv[1]));
	enum AdcMeasurementType type = ADC_TYPE_THERMISTOR;
	shell_print(shell, "ch: %d type: %s", ch, AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, 1, AdcBt6_ConvertTherm);
}

static int vref(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int ch = 0; /* don't care */
	enum AdcMeasurementType type = ADC_TYPE_VREF;
	shell_print(shell, "type: %s", AdcBt6_GetTypeString(type));
	return sample(shell, ch, type, 1, AdcBt6_ConvertVref);
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

/******************************************************************************/
/* SHELL Service                                                              */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_inputs,
	SHELL_CMD(battery, NULL, "Battery Measurement", batteryMeasurement),
	SHELL_CMD(vin, NULL, "Read AINx <channel 0-3>", vin),
	SHELL_CMD(cin, NULL, "Read AINx <channel 0-3>", cin),
	SHELL_CMD(therm, NULL, "Read AINx <channel 0-3>", therm),
	SHELL_CMD(vref, NULL, "Read AINx <channel 0-3>", vref),
	SHELL_CMD(five, NULL, "Set 5V", fiveSet),
	SHELL_CMD(b_plus, NULL, "Set Battery Out", batterySet),
	SHELL_CMD(toggleDO, NULL, "Toggle DO1 and DO2", DigitalOutputToggle),
	SHELL_CMD(dinEnable, NULL, "Set DIN1_EN and DIN2_EN value",
		  digitalEnable),
	SHELL_CMD(configure, NULL,
		  "Set <number of samples> <delay between samples (ms)>",
		  configure),

	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(test, &sub_inputs, "Test", NULL);