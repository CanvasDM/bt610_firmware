/**
 * @file TestMenu.c
 * @brief These are shell commands are used for testing
 *
 * Copyright (c) 2020-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(TestMenu);

#include "lcz_sensor_event.h"
#include "lcz_event_manager.h"
#include "EventTask.h"
#include "BspSupport.h"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <shell/shell.h>
#include <sys/util.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <attr.h>

#include "AdcBt6.h"
#include "BspSupport.h"
#include "EventTask.h"
#include "FrameworkIncludes.h"

/******************************************************************************/
// Local Constant, Macro and Type Definitions
/******************************************************************************/
#define TEST_MENU_VIN_ARG_COUNT 2
#define TEST_MENU_CIN_ARG_COUNT 2
#define TEST_MENU_THERM_ARG_COUNT 2
#define TEST_MENU_TEMP_ARG_COUNT 2
#define TEST_MENU_CONFIGURE_ARG_COUNT 3
#define TEST_MENU_FIVE_SET_ARG_COUNT 2
#define TEST_MENU_BATTERY_SET_ARG_COUNT 2
#define TEST_MENU_DIGITAL_ENABLE_ARG_COUNT 2
#define TEST_MENU_CAL_ARG_COUNT 3
#define TEST_MENU_I2C_PULL_ARG_COUNT 2

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static int samples = 1;
static int delay = 1;

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int battery_measurement(const struct shell *shell, size_t argc,
			       char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int result = -EPERM;

	if (attr_is_locked() == false) {
		int16_t raw = 0;
		float volts = 0;
		int32_t avg_raw = 0;
		float avg_volts = 0;
		int status;
		size_t i;

		for (i = 0; i < samples; i++) {
			status = AdcBt6_read_power_volts(&raw, &volts);
			shell_print(shell, "[%u] status: %d raw: %d volts: %e", i,
				    status, raw, volts);
			if (status != 0) {
				break;
			}
			avg_raw += raw;
			avg_volts += volts;
			k_sleep(K_MSEC(delay));
		}
		avg_raw /= samples;
		avg_volts /= samples;
		shell_print(shell, "averages: raw: %d volts: %e", avg_raw, avg_volts);
		result = 0;
	} else {
        #warning "TODO Bug 22561 - Remove lock references"
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int sample_with_channel(const struct shell *shell, int channel,
			       AdcMeasurementType_t type,
			       float (*func)(size_t, int32_t))
{
	int16_t raw = 0;
	int32_t avg_raw = 0;
	float conv = 0.0;
	float avg_conv = 0.0;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status =
			AdcBt6_Measure(&raw, channel, type, ADC_PWR_SEQ_SINGLE);
		conv = func(channel, raw);
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

static int sample_no_channel(const struct shell *shell,
			     AdcMeasurementType_t type, float (*func)(int32_t))
{
	int16_t raw = 0;
	int32_t avg_raw = 0;
	float conv = 0.0;
	float avg_conv = 0.0;
	int status;
	size_t i;
	for (i = 0; i < samples; i++) {
		status = AdcBt6_Measure(&raw, 0, type, ADC_PWR_SEQ_SINGLE);
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

/* Return channel from 1 to x */
static int convert_channel(const char *str)
{
	return MAX(1, atoi(str));
}

static int vin(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_VIN_ARG_COUNT) {
			shell_print(shell, "Please enter a channel from 1 - 4");
			result = -EINVAL;
		} else {
			int ch = convert_channel(argv[1]);
			AdcMeasurementType_t type = ADC_TYPE_VOLTAGE;
			shell_print(shell, "ch: %d type: %s", ch,
				    AdcBt6_GetTypeString(type));
			result = sample_with_channel(shell, ch - 1, type,
						     AdcBt6_ConvertVoltage);
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int cin(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_CIN_ARG_COUNT) {
			shell_print(shell, "Please enter a channel from 1 - 4");
			result = -EINVAL;
		} else {
			int ch = convert_channel(argv[1]);
			AdcMeasurementType_t type = ADC_TYPE_CURRENT;
			shell_print(shell, "ch: %d type: %s", ch,
				    AdcBt6_GetTypeString(type));
			result = sample_with_channel(shell, ch - 1, type,
						     AdcBt6_ConvertCurrent);
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int therm(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_THERM_ARG_COUNT) {
			shell_print(shell, "Please enter a channel from 1 - 4");
			result = -EINVAL;
		} else {
			int ch = convert_channel(argv[1]);
			AdcMeasurementType_t type = ADC_TYPE_THERMISTOR;
			shell_print(shell, "ch: %d type: %s", ch,
				    AdcBt6_GetTypeString(type));
			result = sample_no_channel(shell, type,
					AdcBt6_ApplyThermistorCalibration);
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int temp(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_TEMP_ARG_COUNT) {
			shell_print(shell, "Please enter a channel from 1 - 4");
			result = -EINVAL;
		} else {
			int ch = convert_channel(argv[1]);
			AdcMeasurementType_t type = ADC_TYPE_THERMISTOR;
			shell_print(shell, "ch: %d type: %s", ch,
				    AdcBt6_GetTypeString(type));
			result = sample_with_channel(shell, ch - 1, type,
					AdcBt6_ConvertThermToTemperature);
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int vref(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	int result = -EPERM;

	if (attr_is_locked() == false) {
		AdcMeasurementType_t type = ADC_TYPE_VREF;
		shell_print(shell, "type: %s", AdcBt6_GetTypeString(type));
		result = sample_no_channel(shell, type, AdcBt6_ConvertVref);
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int configure(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_CONFIGURE_ARG_COUNT) {
			shell_print(shell,
				    "Please enter a sample count and delay time");
			result = -EINVAL;
		} else {
			samples = MAX(1, atoi(argv[1]));
			delay = MAX(1, atoi(argv[2]));
			shell_print(shell, "samples: %d delay (ms): %d", samples,
				    delay);
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int five_set(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_FIVE_SET_ARG_COUNT) {
			shell_print(shell, "Please enter 0 for off or 1 for on");
			result = -EINVAL;
		} else {
			int value = atoi(argv[1]);
			int rc = (value == 0) ? AdcBt6_FiveVoltDisable() :
						      AdcBt6_FiveVoltEnable();
			shell_print(shell, "Set 5V: %d status: %d", value, rc);
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int battery_set(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_BATTERY_SET_ARG_COUNT) {
			shell_print(shell, "Please enter 0 for off or 1 for on");
			result = -EINVAL;
		} else {
			int value = atoi(argv[1]);
			int rc = (value == 0) ? AdcBt6_BplusDisable() :
						      AdcBt6_BplusEnable();
			shell_print(shell, "Set Battery Enable: %d status: %d",
				    value, rc);
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int digital_output_toggle(const struct shell *shell, size_t argc,
				 char **argv)
{
	ARG_UNUSED(argc);

	int result = -EPERM;

	if (attr_is_locked() == false) {
		int status1 = BSP_PinToggle(DO1_PIN);
		int status2 = BSP_PinToggle(DO2_PIN);
		shell_print(shell, "Toggle DO1 and DO2 status: %d %d", status1,
			    status2);
		result = 0;
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int digital_enable(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_DIGITAL_ENABLE_ARG_COUNT) {
			shell_print(shell, "Please enter 0 for off or 1 for on");
			result = -EINVAL;
		} else {
			int value = atoi(argv[1]);
			int status1 = BSP_PinSet(DIN1_ENABLE_PIN, value);
			int status2 = BSP_PinSet(DIN2_ENABLE_PIN, value);
			shell_print(shell, "Set To DIN_EN: %d %d %d", value,
				    status1, status2);
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int cal(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_CAL_ARG_COUNT) {
			shell_print(shell,
				    "Please enter c1 and c2 constants values");
			result = -EINVAL;
		} else {
			float c1 = atof(argv[1]);
			float c2 = atof(argv[2]);
			shell_print(shell, "c1 set to %.4e", c1);
			shell_print(shell, "c2 set to %.4e", c2);

			float ge;
			float oe;
			int status;
			size_t i;
			for (i = 0; i < samples; i++) {
				status = AdcBt6_CalibrateThermistor(c1, c2, &ge,
								    &oe);
				if (status != 0) {
					shell_print(shell, "error: %d", status);
					break;
				}
				shell_print(shell, "ge: %.4e oe: %.4e", ge, oe);
				k_sleep(K_MSEC(delay));
			}
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int advertise(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int result = -EPERM;

	if (attr_is_locked() == false) {
		shell_print(shell, "Starting advertising . . .\n");
		FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_USER_IF_TASK,
					      FWK_ID_USER_IF_TASK,
					      FMC_ENTER_ACTIVE_MODE);
		result = 0;
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

static int i2cpull(const struct shell *shell, size_t argc, char **argv)
{
	int result = -EPERM;

	if (attr_is_locked() == false) {
		if (argc < TEST_MENU_I2C_PULL_ARG_COUNT) {
			shell_print(shell, "Please enter 0 for off or 1 for on");
			result = -EINVAL;
		} else {
			int state = atoi(argv[1]);

			if (state) {
				shell_print(shell,
					    "Enabling I2C pullups . . .\n");
				NRF_P0->PIN_CNF[26] &= ~0xC;
				NRF_P0->PIN_CNF[26] |= 0xC;
				NRF_P0->PIN_CNF[27] &= ~0xC;
				NRF_P0->PIN_CNF[27] |= 0xC;
			} else {
				shell_print(shell,
					    "Disabling I2C pullups . . .\n");
				NRF_P0->PIN_CNF[26] &= ~0xC;
				NRF_P0->PIN_CNF[27] &= ~0xC;
			}
			result = 0;
		}
	} else {
		shell_error(shell, "Configuration lock is engaged, "
				   "test functions are unavailable");
	}

	return result;
}

/******************************************************************************/
/* SHELL Service                                                              */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_inputs,
	SHELL_CMD(battery, NULL, "Battery Measurement", battery_measurement),
	SHELL_CMD(
		  vin, NULL,
		  "Read analog voltage <channel 1-4>\n"
		  "Set channel to voltage sense using 'attr set analogInput<1-4>Type 1'",
		  vin),
	SHELL_CMD(
		  cin, NULL,
		  "Read current <channel 1-4>\n"
		  "Set channel to current sense using 'attr set analogInput<1-4>Type 2'",
		  cin),
	SHELL_CMD(therm, NULL, "Read thermistor <channel 1-4>", therm),
	SHELL_CMD(temp, NULL, "Get temperature (therm) <channel 1-4>", temp),
	SHELL_CMD(vref, NULL, "Read vref", vref),
	SHELL_CMD(five, NULL, "Set 5V", five_set),
	SHELL_CMD(b_plus, NULL, "Set Battery Out", battery_set),
	SHELL_CMD(toggledo, NULL, "Toggle DO1 and DO2", digital_output_toggle),
	SHELL_CMD(dinenable, NULL, "Set DIN1_EN and DIN2_EN value",
		  digital_enable),
	SHELL_CMD(cal, NULL,
		  "Calibrate Thermistor <c1 float> <c2 float>\n"
		  "Example: test cal 220.7351 3.98e3",
		  cal),
	SHELL_CMD(configure, NULL,
		  "Set <number of samples> <delay between samples (ms)>",
		  configure),
	SHELL_CMD(advertise, NULL, "Advertises . . .", advertise),
	SHELL_CMD(i2cpull, NULL, "Configures I2C pullups . . .", i2cpull),

	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(test, &sub_inputs, "Test", NULL);
