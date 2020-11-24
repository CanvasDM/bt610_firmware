/**
 * @file AdcBt6.c
 * @brief This will read the ADC pins and report back the value as a voltage
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(AdcBt6, CONFIG_ADC_BT6_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <drivers/adc.h>
#include <hal/nrf_saadc.h>
#include <drivers/i2c.h>
#include <string.h>

#include "BspSupport.h"
#include "laird_utility_macros.h"
#include "AdcBt6.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define ADC_DEVICE_NAME DT_LABEL(DT_INST(0, nordic_nrf_saadc))

/* clang-format off */
#if 0 //CONFIG_ADC_BT6_OVERSAMPLING > 0
#define ADC_RESOLUTION            14
#else
#define ADC_RESOLUTION            12
#endif
/* reference applications\nrf_desktop\src\hw_interface\battery_meas.c */
#define ADC_DEFAULT_GAIN          ADC_GAIN_1_6
#define ADC_DEFAULT_REFERENCE     ADC_REF_INTERNAL
#define ADC_GAIN_THERMISTOR       ADC_GAIN_1_4
#define ADC_REFERENCE_THERMISTOR  ADC_REF_VDD_1_4
#define ADC_ACQUISITION_TIME      ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
/* clang-format on */

/* clang-format off */
#define EXPANDER_ADDRESS    0x70
#define TCA9538_REG_INPUT   0x00
#define TCA9538_REG_OUTPUT  0x01
#define TCA9538_REG_POL_INV 0x02
#define TCA9538_REG_CONFIG  0x03
#define OUTPUT_CONFIG       0xC0 /* pins 6 and 7 are always inputs */
/* clang-format on */

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c0), okay)
#define I2C_DEV_NAME DT_LABEL(DT_NODELABEL(i2c0))
#else
#error "Please set the correct I2C device"
#endif

const uint32_t I2C_CFG = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_MASTER;

/* clang-format off */
#define POWER_ENABLE_DELAY_US  200
#define MUX_SWITCH_DELAY_US    100
#define V_5_ENABLE_DELAY_MS    3
#define B_PLUS_ENABLE_DELAY_MS 1
/* clang-format on */

#define ANALOG_VOLTAGE_CONVERSION_FACTOR 281.2
#define ANALOG_CURRENT_CONVERSION_FACTOR 71.875

struct expander_bits {
	uint8_t ain_sel : 4;
	/* controls analog and thermistor muxes */
	uint8_t mux : 2;
	/* top two bits are always inputs */
	uint8_t aux_inputs : 2;
};

struct expander {
	union {
		uint8_t byte;
		struct expander_bits bits;
	};
};
BUILD_ASSERT(sizeof(struct expander) == sizeof(uint8_t), "Union error");

typedef struct AdcObj {
	struct adc_channel_cfg channel_cfg;
	struct device *dev;
	struct device *i2c;
	struct expander expander;
	bool calibrate;
	bool five_enabled;
	bool b_plus_enabled;
	int32_t ref;
} AdcObj_t;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(adc_mutex);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static AdcObj_t adc_obj;
static struct adc_channel_cfg *const pcfg = &adc_obj.channel_cfg;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int sample_channel(int16_t *raw, AnalogChannel_t channel);
static int configure_channel(AnalogChannel_t channel);

static AnalogChannel_t get_channel(enum AdcMeasurementType type);
static int init_expander(void);
static int config_ain_selects(void);
static int config_mux(enum MuxInput input);
static void config_power(enum AdcMeasurementType type);
static void disable_power(void);
static bool valid_input_for_current_measurement(enum MuxInput input);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AdcBt6_Init(void)
{
	int status = 0;
	adc_obj.calibrate = true;
	adc_obj.dev = device_get_binding(ADC_DEVICE_NAME);
	if (adc_obj.dev == NULL) {
		__ASSERT(false, "Failed to get device binding");
		status = -1;
	}

	pcfg->acquisition_time = ADC_ACQUISITION_TIME;

	adc_obj.ref = adc_ref_internal(adc_obj.dev);
	LOG_DBG("Internal reference %u", adc_obj.ref);

	int16_t raw = 0;
	int32_t mv = 0;
	int rc = AdcBt6_ReadBatteryMv(&raw, &mv);
	LOG_INF("status: %d battery raw: %d mv: %d", rc, raw, mv);

	status = init_expander();

	return status;
}

int AdcBt6_ReadBatteryMv(int16_t *raw, int32_t *mv)
{
	k_mutex_lock(&adc_mutex, K_FOREVER);

	int rc = sample_channel(raw, BATTERY_ADC_CH);
	if (rc >= 0) {
		*mv = (int32_t)*raw;
		rc = adc_raw_to_millivolts(adc_obj.ref, pcfg->gain,
					   ADC_RESOLUTION, mv);
	}
	k_mutex_unlock(&adc_mutex);
	return rc;
}

int AdcBt6_Measure(int16_t *raw, enum MuxInput input,
		   enum AdcMeasurementType type)
{
	int rc = -EPERM;
	if (adc_obj.dev == NULL) {
		return rc;
	}

	if (type == ADC_TYPE_ANALOG_CURRENT) {
		if (!valid_input_for_current_measurement(input)) {
			LOG_ERR("Invalid input for current measurement");
			return rc;
		}
	}

	if (type < NUMBER_OF_ADC_TYPES) {
		k_mutex_lock(&adc_mutex, K_FOREVER);
		config_power(type);
		if (type == ADC_TYPE_VREF) {
			rc = 0;
		} else {
			rc = config_mux(input);
		}
		if (rc == 0) {
			rc = sample_channel(raw, get_channel(type));
		}
		disable_power();
		k_mutex_unlock(&adc_mutex);
	} else {
		LOG_ERR("Invalid measurement type");
	}
	return rc;
}

const char *AdcBt6_GetTypeString(enum AdcMeasurementType type)
{
	switch (type) {
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_ANALOG_VOLTAGE);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_ANALOG_CURRENT);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_THERMISTOR);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_VREF);
	default:
		return "Unknown";
	}
}

const char *AdcBt6_GetChannelString(enum AnalogChannel channel)
{
	switch (channel) {
		SWITCH_CASE_RETURN_STRING(BATTERY_ADC_CH);
		SWITCH_CASE_RETURN_STRING(ANALOG_SENSOR_1_CH);
		SWITCH_CASE_RETURN_STRING(THERMISTOR_SENSOR_2_CH);
		SWITCH_CASE_RETURN_STRING(VREF_5_CH);
	default:
		return "Unknown";
	}
}

int AdcBt6_FiveVoltEnable(void)
{
	int rc = -EPERM;
	if (!adc_obj.b_plus_enabled) {
		rc = BSP_PinSet(FIVE_VOLT_ENABLE_PIN, 1);
	} else {
		LOG_ERR("Enable 5V before enabling b+");
	}
	if (rc == 0) {
		k_sleep(K_MSEC(V_5_ENABLE_DELAY_MS));
		adc_obj.five_enabled = true;
	}
	return rc;
}

int AdcBt6_FiveVoltDisable(void)
{
	int rc = -EPERM;
	if (adc_obj.b_plus_enabled) {
		rc = BSP_PinSet(FIVE_VOLT_ENABLE_PIN, 0);
	} else {
		LOG_ERR("Disable 5V before disabling b+");
	}
	if (rc == 0) {
		adc_obj.five_enabled = false;
	}
	return rc;
}

int AdcBt6_BplusEnable(void)
{
	int rc = BSP_PinSet(BATT_OUT_ENABLE_PIN, 1);
	if (rc == 0) {
		k_sleep(K_MSEC(V_5_ENABLE_DELAY_MS));
		adc_obj.b_plus_enabled = true;
	}
	return rc;
}

int AdcBt6_BplusDisable(void)
{
	int rc = -EPERM;
	if (!adc_obj.five_enabled) {
		rc = BSP_PinSet(BATT_OUT_ENABLE_PIN, 0);
	} else {
		LOG_ERR("Disable 5V before disabling b+");
	}
	if (rc == 0) {
		adc_obj.b_plus_enabled = false;
	}
	return rc;
}

float AdcBt6_ConvertVoltage(int32_t raw)
{
	return ((float)raw) / ANALOG_VOLTAGE_CONVERSION_FACTOR;
}

float AdcBt6_ConvertCurrent(int32_t raw)
{
	return ((float)raw) / ANALOG_CURRENT_CONVERSION_FACTOR;
}

float AdcBt6_ConvertVref(int32_t raw)
{
	int32_t mv = raw;
	(void)adc_raw_to_millivolts(adc_obj.ref, ADC_DEFAULT_GAIN,
				    ADC_RESOLUTION, &mv);
	float f = (float)mv / 1000.0;
	return f;
}

float AdcBt6_ConvertTherm(int32_t raw)
{
	return (float)raw;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/

/* Thermistor enable is active low. */
static void config_power(enum AdcMeasurementType type)
{
	switch (type) {
	case ADC_TYPE_ANALOG_VOLTAGE:
		BSP_PinSet(ANALOG_ENABLE_PIN, 1);
		BSP_PinSet(THERM_ENABLE_PIN, 1);
		break;

	case ADC_TYPE_ANALOG_CURRENT:
		BSP_PinSet(ANALOG_ENABLE_PIN, 1);
		BSP_PinSet(THERM_ENABLE_PIN, 1);
		break;

	case ADC_TYPE_THERMISTOR:
		BSP_PinSet(ANALOG_ENABLE_PIN, 0);
		BSP_PinSet(THERM_ENABLE_PIN, 0);
		break;

	case ADC_TYPE_VREF:
		BSP_PinSet(ANALOG_ENABLE_PIN, 0);
		BSP_PinSet(THERM_ENABLE_PIN, 0);
		break;

	default:
		BSP_PinSet(ANALOG_ENABLE_PIN, 0);
		BSP_PinSet(THERM_ENABLE_PIN, 1);
		break;
	}

	k_busy_wait(POWER_ENABLE_DELAY_US);
}

static void disable_power(void)
{
	BSP_PinSet(ANALOG_ENABLE_PIN, 0);
	BSP_PinSet(THERM_ENABLE_PIN, 1);
}

static AnalogChannel_t get_channel(enum AdcMeasurementType type)
{
	/* clang-format off */
	switch (type) {
	case ADC_TYPE_ANALOG_VOLTAGE:   return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_ANALOG_CURRENT:   return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_THERMISTOR:		return THERMISTOR_SENSOR_2_CH;
	case ADC_TYPE_VREF:             return VREF_5_CH;
	default:                        return BATTERY_ADC_CH;
	}
	/* clang-format on */
}

/* Sample a single channel CONFIG_ADC_BT6_OVERSAMPLING times
 * VREF will hold a charge (if thermistor_en was ever active) if there isn't
 * a resistor or thermistor connected.
 */
static int sample_channel(int16_t *raw, AnalogChannel_t channel)
{
	int rc = -EPERM;
	/* clang-format off */
	const struct adc_sequence sequence = {
		.options = NULL,
		.channels = BIT(channel),
		.buffer = raw,
		.buffer_size = sizeof(*raw),
		.resolution = ADC_RESOLUTION,
		.oversampling = CONFIG_ADC_BT6_OVERSAMPLING,
		.calibrate = adc_obj.calibrate
	};
	/* clang-format on */

	if (adc_obj.dev) {
		rc = configure_channel(channel);
		if (rc == 0) {
			rc = adc_read(adc_obj.dev, &sequence);
		}
		if (rc < 0) {
			LOG_ERR("Unable to sample ADC");
		} else {
			adc_obj.calibrate = false;
		}
	}
	return rc;
}

static int configure_channel(AnalogChannel_t channel)
{
	LOG_DBG("%s", AdcBt6_GetChannelString(channel));

	pcfg->channel_id = channel;
	pcfg->input_positive = channel;

	switch (channel) {
	case ANALOG_SENSOR_1_CH:
		pcfg->gain = ADC_DEFAULT_GAIN;
		pcfg->reference = ADC_DEFAULT_REFERENCE;
		break;

	case THERMISTOR_SENSOR_2_CH:
		pcfg->gain = ADC_GAIN_THERMISTOR;
		pcfg->reference = ADC_REFERENCE_THERMISTOR;
		break;

	case VREF_5_CH:
		pcfg->gain = ADC_DEFAULT_GAIN;
		pcfg->reference = ADC_DEFAULT_REFERENCE;
		break;

	case BATTERY_ADC_CH:
	default:
		pcfg->gain = ADC_DEFAULT_GAIN;
		pcfg->reference = ADC_DEFAULT_REFERENCE;
		pcfg->input_positive = NRF_SAADC_INPUT_VDD;
		break;
	}

	return adc_channel_setup(adc_obj.dev, pcfg);
}

static int init_expander(void)
{
	int rc = -EPERM;
	adc_obj.i2c = device_get_binding(I2C_DEV_NAME);
	if (adc_obj.i2c == NULL) {
		LOG_ERR("Cannot get I2C device");
		return rc;
	}

	rc = i2c_configure(adc_obj.i2c, I2C_CFG);
	if (rc == 0) {
		uint8_t cmd[] = { TCA9538_REG_CONFIG, OUTPUT_CONFIG };
		rc = i2c_write(adc_obj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS);
	}

	if (rc < 0) {
		LOG_ERR("I2C failure");
		adc_obj.i2c = NULL;
	} else {
		rc = config_ain_selects();
	}
	return rc;
}

/* The AINx_SEL lines need to be maintained at all times.
 * There will always be a voltage or current applied at the terminal so
 * the proper terminal load is required.
 * (2M for voltage input, 250 ohm for current input)
 */
static int config_ain_selects(void)
{
	int rc = -EPERM;
	if (adc_obj.i2c == NULL) {
		return rc;
	}

	/* todo: read config */
	/* For now all are set to voltage */

	/* To measure current the correspoding output must be set to 1 */
	adc_obj.expander.bits.ain_sel = 0x00;
	uint8_t cmd[] = { TCA9538_REG_OUTPUT, adc_obj.expander.byte };
	if (i2c_write(adc_obj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS) < 0) {
		LOG_ERR("I2C Failure");
		rc = -EIO;
	}
	return rc;
}

/* Two of the expander outputs control the mux that selects one of the
 * 4 analog or thermistor inputs.
 */
static int config_mux(enum MuxInput input)
{
	if (adc_obj.i2c == NULL) {
		return -EPERM;
	}

	if (input >= NUMBER_OF_ANALOG_INPUTS) {
		LOG_ERR("Invalid input %d (required range 0-3)", input);
		return -EPERM;
	}

	adc_obj.expander.bits.mux = input;
	uint8_t cmd[] = { TCA9538_REG_OUTPUT, adc_obj.expander.byte };
	if (i2c_write(adc_obj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS) < 0) {
		LOG_ERR("I2C Failure");
		return -EIO;
	}
	k_busy_wait(MUX_SWITCH_DELAY_US);
	return 0;
}

static bool valid_input_for_current_measurement(enum MuxInput input)
{
	return ((BIT(input) & adc_obj.expander.bits.ain_sel) != 0);
}