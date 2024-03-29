/**
 * @file AdcBt6.c
 * @brief This will read the ADC pins and report back the value as a voltage
 *
 * Copyright (c) 2020-2022 Laird Connectivity
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
#include <stdio.h>
#include <math.h>
#include <locking_defs.h>
#include <locking.h>

#include "BspSupport.h"
#include "laird_utility_macros.h"
#include "file_system_utilities.h"
#include "lcz_param_file.h"
#include "attr.h"
#include "AnalogInput.h"
#include "AdcBt6.h"
#include "SensorTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define ADC_DEVICE_NAME DEVICE_DT_NAME(DT_NODELABEL(adc))

/* clang-format off */
#define ADC_RESOLUTION            12
#define ADC_GAIN_DEFAULT          ADC_GAIN_1_6
#define ADC_GAIN_THERMISTOR       ADC_GAIN_1_4
#define ADC_REFERENCE_DEFAULT     ADC_REF_INTERNAL
#define ADC_REFERENCE_THERMISTOR  ADC_REF_VDD_1_4
#define ADC_ACQUISITION_TIME      ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
/* clang-format on */

/* clang-format off */
#define EXPANDER_ADDRESS     0x70
#define TCA9538_REG_INPUT    0x00
#define TCA9538_REG_OUTPUT   0x01
#define TCA9538_REG_POL_INV  0x02
#define TCA9538_REG_CONFIG   0x03
#define OUTPUT_CONFIG        0xC0 /* pins 6 and 7 are always inputs */
#define OUTPUT_CONFIG_v2     0x00 /* pins 6 and 7 are always Output */
/* clang-format on */

#if DT_NODE_HAS_STATUS(DT_NODELABEL(i2c0), okay)
#define I2C_DEV_NAME DEVICE_DT_NAME(DT_NODELABEL(i2c0))
#else
#error "Please set the correct I2C device"
#endif

const uint32_t I2C_CFG = I2C_SPEED_SET(I2C_SPEED_STANDARD) | I2C_MODE_CONTROLLER;

#define POWER_ENABLE_DELAY_US 200
#define MUX_SWITCH_DELAY_US 100
#define V_5_ENABLE_DELAY_MS 3
#define B_PLUS_ENABLE_DELAY_MS 1
#define ULTRASONIC_DELAY_MS 400 /* 2 measurements */
#define PRESSURE_DELAY_MS 100

/* Constants for converting ADC counts to voltage */
#define ANALOG_VOLTAGE_CONVERSION_FACTOR 281.2
#define ANALOG_CURRENT_CONVERSION_FACTOR 71.875
#define ULTRASONIC_CONVERSION_FACTOR (10240.0 / 5.0)
#define ACCURRENT_20AMP_CONVERSION_FACTOR (20.0 / 5.0)
#define ACCURRENT_150AMP_CONVERSION_FACTOR (150.0 / 5.0)
#define ACCURRENT_500AMP_CONVERSION_FACTOR (500.0 / 5.0)

/* Constants for Steinhart-Hart Equation for the Focus thermistor */
#define THERMISTOR_S_H_A 1.132e-3
#define THERMISTOR_S_H_B 2.338e-4
#define THERMISTOR_S_H_C 8.780e-8
#define THERMISTOR_S_H_OFFSET 273.15

/* Used to convert incoming simulated voltage values to millivolts */
#define ADC_BT6_VOLTS_TO_MILLIVOLTS 1000.0f

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
	const struct device *dev;
	const struct device *i2c;
	struct expander expander;
	bool calibrate;
	bool fiveEnabled;
	bool bPlusEnabled;
	int32_t ref;
	float ge;
	float oe;
} AdcObj_t;

#define CONFIG_ATTR_FLOAT_MAX_STR_SIZE 20
#define CONFIG_ATTR_FLOAT_FMT "%.4e"
#define F_FMT CONFIG_ATTR_FLOAT_FMT

#define THERM_CAL_FMT_STR                                                      \
	"c1: " F_FMT " c2: " F_FMT " ge: " F_FMT "oe: " F_FMT "\r\n"
#define THERM_CAL_FMT_STR_MAX_SIZE                                             \
	((4 * CONFIG_ATTR_FLOAT_MAX_STR_SIZE) + sizeof(THERM_CAL_FMT_STR))

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static AdcObj_t adcObj;
static struct adc_channel_cfg *const pcfg = &adcObj.channel_cfg;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int SampleChannel(int16_t *raw, AnalogChannel_t channel);
static int ConfigureChannel(AnalogChannel_t channel);

static AnalogChannel_t GetChannel(AdcMeasurementType_t type);
static int InitExpander(void);
static int ConfigMux(MuxInput_t input);
static void UltrasonicOrPressurePowerAndDelayHandler(AdcMeasurementType_t type);
static void UltrasonicOrPressureDisablePower(AdcMeasurementType_t type);
static bool ValidInputForCurrentMeasurement(MuxInput_t input);
static float Steinhart_Hart(float calibrated, float a, float b, float c);
static bool ADCChannelIsSimulated(AnalogChannel_t channel,
				  int16_t *simulated_value);
static bool VoltageIsSimulated(size_t channel, float *simulated_value);
static bool UltrasonicIsSimulated(float *simulated_value);
static bool PressureIsSimulated(float *simulated_value);
static bool CurrentIsSimulated(size_t channel, float *simulated_value);
static bool VrefIsSimulated(float *simulated_value);
static bool TemperatureIsSimulated(size_t channel, float *simulated_value);
static bool power_volt_is_simulated(float *simulated_value);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AdcBt6_Init(void)
{
	int status = 0;
	ssize_t readReturn;
	adcObj.calibrate = true;
	adcObj.dev = device_get_binding(ADC_DEVICE_NAME);

	if (adcObj.dev == NULL) {
		__ASSERT(false, "Failed to get device binding");
		status = -1;
	}

	pcfg->channel_id = INVALID_CH;
	pcfg->acquisition_time = ADC_ACQUISITION_TIME;

	adcObj.ref = adc_ref_internal(adcObj.dev);
	LOG_DBG("Internal reference %u", adcObj.ref);

	status = InitExpander();

	/* Calibration is independent of parameters/attributes module. */
	readReturn = lcz_param_file_read("ge", &adcObj.ge, sizeof(adcObj.ge));
	if (readReturn > 0) {
		attr_set_float(ATTR_ID_ge, adcObj.ge);
	} else {
		/* No calibration value saved, use default */
		attr_copy_float(&adcObj.ge, ATTR_ID_ge);
	}

	readReturn = lcz_param_file_read("oe", &adcObj.oe, sizeof(adcObj.oe));
	if (readReturn > 0) {
		attr_set_float(ATTR_ID_oe, adcObj.oe);
	} else {
		/* No calibration value saved, use default */
		attr_copy_float(&adcObj.oe, ATTR_ID_oe);
	}

	return status;
}

int AdcBt6_read_power_volts(int16_t *raw, float *v)
{
	int rc = 0;

	if (!power_volt_is_simulated(v)) {
		locking_take(LOCKING_ID_adc, K_FOREVER);

		rc = SampleChannel(raw, POWER_ADC_CH);
		if (rc >= 0) {
			*v = AdcBt6_ConvertVref(*raw);
		}

		locking_give(LOCKING_ID_adc);
	}
	return rc;
}

int AdcBt6_Measure(int16_t *raw, MuxInput_t input, AdcMeasurementType_t type,
		   AdcPwrSequence_t power)
{
	int rc = -EINVAL;
	if (adcObj.dev == NULL) {
		rc = -EIO;
		return rc;
	}

	if (type == ADC_TYPE_CURRENT) {
		if (!ValidInputForCurrentMeasurement(input)) {
			LOG_ERR("Invalid input for current measurement");
			return rc;
		}
	}

	/** @ref Hardware Sensor Measurement Procedures.docx */

	if (type < NUMBER_OF_ADC_TYPES) {
		locking_take(LOCKING_ID_adc, K_FOREVER);

		if (power == ADC_PWR_SEQ_SINGLE || power == ADC_PWR_SEQ_START) {
			AdcBt6_ConfigPower(type);
		}

		if (type == ADC_TYPE_VREF) {
			rc = 0;
		} else {
			rc = ConfigMux(input);
		}

		if (power != ADC_PWR_SEQ_BYPASS) {
			UltrasonicOrPressurePowerAndDelayHandler(type);
		}

		if (rc == 0) {
			rc = SampleChannel(raw, GetChannel(type));
		}

		if (power == ADC_PWR_SEQ_SINGLE || power == ADC_PWR_SEQ_END) {
			AdcBt6_DisablePower();
		}

		if (power != ADC_PWR_SEQ_BYPASS) {
			UltrasonicOrPressureDisablePower(type);
		}

		locking_give(LOCKING_ID_adc);
	} else {
		LOG_ERR("Invalid measurement type");
	}
	return rc;
}

int AdcBt6_CalibrateThermistor(float c1, float c2, float *ge, float *oe)
{
	int rc = -EINVAL;

	float divisor = c2 - c1;
	if (divisor == 0) {
		return rc;
	}

	char str[THERM_CAL_FMT_STR_MAX_SIZE];
	const int SAMPLES = CONFIG_ADC_BT6_THERMISTOR_CALIBRATION_SAMPLES;
	float m1 = 0.0;
	float m2 = 0.0;
	int status = 0;
	int16_t raw;
	size_t i;

	AdcBt6_ConfigPower(ADC_TYPE_THERMISTOR);
	for (i = 0; ((i < SAMPLES) && (status == 0)); i++) {
		status =
			AdcBt6_Measure(&raw, MUX_AIN1_THERM1,
				       ADC_TYPE_THERMISTOR, ADC_PWR_SEQ_BYPASS);
		m1 += (float)raw;
	}

	for (i = 0; ((i < SAMPLES) && (status == 0)); i++) {
		status =
			AdcBt6_Measure(&raw, MUX_AIN2_THERM2,
				       ADC_TYPE_THERMISTOR, ADC_PWR_SEQ_BYPASS);
		m2 += (float)raw;
	}
	AdcBt6_DisablePower();

	if (status == 0) {
		m1 /= (float)SAMPLES;
		m2 /= (float)SAMPLES;
		adcObj.ge = (m2 - m1) / (c2 - c1);
		adcObj.oe = m1 - (adcObj.ge * c1);
		*ge = adcObj.ge;
		*oe = adcObj.oe;
		snprintf(str, sizeof(str), THERM_CAL_FMT_STR, c1, c2, *ge, *oe);
		fsu_append(CONFIG_FSU_MOUNT_POINT, "thermistor_cal.txt", str,
			   strlen(str));
		lcz_param_file_write("ge", &adcObj.ge, sizeof(adcObj.ge));
		lcz_param_file_write("oe", &adcObj.oe, sizeof(adcObj.oe));
		attr_set_float(ATTR_ID_ge, adcObj.ge);
		attr_set_float(ATTR_ID_oe, adcObj.oe);
	} else {
		LOG_ERR("Thermistor calibration error");
	}
	return status;
}

const char *AdcBt6_GetTypeString(AdcMeasurementType_t type)
{
	switch (type) {
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_VOLTAGE);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_CURRENT);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_PRESSURE);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_ULTRASONIC);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_THERMISTOR);
		SWITCH_CASE_RETURN_STRING(ADC_TYPE_VREF);
	default:
		return "Unknown";
	}
}

int AdcBt6_FiveVoltEnable(void)
{
	int rc = -EINVAL;
	if (!adcObj.bPlusEnabled) {
		rc = BSP_PinSet(FIVE_VOLT_ENABLE_PIN, 1);
	} else {
		LOG_ERR("Enable 5V before enabling b+");
	}

	if (rc == 0) {
		k_sleep(K_MSEC(V_5_ENABLE_DELAY_MS));
		adcObj.fiveEnabled = true;
	}
	return rc;
}

int AdcBt6_FiveVoltDisable(void)
{
	int rc = -EINVAL;
	if (!adcObj.bPlusEnabled) {
		rc = BSP_PinSet(FIVE_VOLT_ENABLE_PIN, 0);
	} else {
		LOG_ERR("Disable b+ before disabling 5V");
	}
	if (rc == 0) {
		adcObj.fiveEnabled = false;
	}
	return rc;
}

int AdcBt6_BplusEnable(void)
{
	int rc = BSP_PinSet(BATT_OUT_ENABLE_PIN, 1);
	if (rc == 0) {
		k_sleep(K_MSEC(V_5_ENABLE_DELAY_MS));
		adcObj.bPlusEnabled = true;
	}
	return rc;
}

int AdcBt6_BplusDisable(void)
{
	int rc = -EINVAL;
	if (adcObj.fiveEnabled) {
		rc = BSP_PinSet(BATT_OUT_ENABLE_PIN, 0);
	} else {
		LOG_ERR("Disable 5V after disabling b+");
	}

	if (rc == 0) {
		adcObj.bPlusEnabled = false;
	}
	return rc;
}

float AdcBt6_ConvertVoltage(size_t channel, int32_t raw)
{
	float voltage;

	if (!VoltageIsSimulated(channel, &voltage)) {
		voltage = ((float)raw) / ANALOG_VOLTAGE_CONVERSION_FACTOR;
	}
	return(voltage);
}

float AdcBt6_ConvertUltrasonic(size_t channel, int32_t raw)
{
	float ultrasonic;

	if (!UltrasonicIsSimulated(&ultrasonic)) {
		ultrasonic = AdcBt6_ConvertVoltage(channel, raw) * ULTRASONIC_CONVERSION_FACTOR;
	}
	return(ultrasonic);
}

float AdcBt6_ConvertPressure(size_t channel, int32_t raw)
{
	float pressure;

	if (!PressureIsSimulated(&pressure)) {
		pressure = (AdcBt6_ConvertVoltage(channel, raw) * 75) - 37.5;
	}
	return (pressure);
}

float AdcBt6_ConvertCurrent(size_t channel, int32_t raw)
{
	float current;

	if (!CurrentIsSimulated(channel,&current)) {
		current = ((float)raw) / ANALOG_CURRENT_CONVERSION_FACTOR;
	}
	return(current);
}

float AdcBt6_ConvertACCurrent20(size_t channel, int32_t raw)
{
	float current;

	if (!CurrentIsSimulated(channel,&current)) {
		/* AC Current reading  = (reported Volts/5V)*Sensor current rating (in
		 * Amps rms) Sensor current ratings = 20A, 150A, or 500A
		 */
		current = (AdcBt6_ConvertVoltage(channel, raw) * ACCURRENT_20AMP_CONVERSION_FACTOR);
	}
	return(current);
}

float AdcBt6_ConvertACCurrent150(size_t channel, int32_t raw)
{
	float current;

	if (!CurrentIsSimulated(channel,&current)) {
		/* AC Current reading  = (reported Volts/5V)*Sensor current rating (in
		 * Amps rms) Sensor current ratings = 20A, 150A, or 500A
		 */
		current = (AdcBt6_ConvertVoltage(channel, raw) * ACCURRENT_150AMP_CONVERSION_FACTOR);
	}
	return(current);
}

float AdcBt6_ConvertACCurrent500(size_t channel, int32_t raw)
{
	float current;

	if (!CurrentIsSimulated(channel,&current)) {
		/* AC Current reading  = (reported Volts/5V)*Sensor current rating (in
		 * Amps rms) Sensor current ratings = 20A, 150A, or 500A
		 */
		current = (AdcBt6_ConvertVoltage(channel, raw) * ACCURRENT_500AMP_CONVERSION_FACTOR);
	}
	return(current);
}

float AdcBt6_ConvertVref(int32_t raw)
{
	float vref;

	if (!VrefIsSimulated(&vref)) {
		int32_t mv = raw;
		(void)adc_raw_to_millivolts(adcObj.ref, ADC_GAIN_DEFAULT,
					    ADC_RESOLUTION, &mv);
		vref = (float)mv / 1000.0;
	}
	return (vref);
}

float AdcBt6_ApplyThermistorCalibration(int32_t raw)
{
	float result = 0.0;
	if (adcObj.ge != 0) {
		result = ((float)raw - adcObj.oe) / adcObj.ge;
	}
	return result;
}

float AdcBt6_ConvertThermToTemperature(size_t channel, int32_t raw)
{
	float temperature;
	float calibrated;

	if (!TemperatureIsSimulated(channel,&temperature)) {
		calibrated = AdcBt6_ApplyThermistorCalibration(raw);
		uint16_t coefficientA = ATTR_ID_therm_1_coefficient_a + channel;
		uint16_t coefficientB = ATTR_ID_therm_1_coefficient_b + channel;
		uint16_t coefficientC = ATTR_ID_therm_1_coefficient_c + channel;
		temperature = Steinhart_Hart(
			       calibrated,
			       attr_get_float(coefficientA, THERMISTOR_S_H_A),
			       attr_get_float(coefficientB, THERMISTOR_S_H_B),
			       attr_get_float(coefficientC, THERMISTOR_S_H_C)) -
		       attr_get_float(ATTR_ID_sh_offset,
					     THERMISTOR_S_H_OFFSET);
	}
	return(temperature);
}

void AdcBt6_ConfigPower(AdcMeasurementType_t type)
{
	/* Thermistor enable is active low. */
	switch (type) {
	case ADC_TYPE_VOLTAGE:
	case ADC_TYPE_CURRENT:
	case ADC_TYPE_PRESSURE:
	case ADC_TYPE_ULTRASONIC:
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

void AdcBt6_DisablePower(void)
{
	BSP_PinSet(ANALOG_ENABLE_PIN, 0);
	BSP_PinSet(THERM_ENABLE_PIN, 1);
}

/* The AINx_SEL lines need to be maintained at all times.
 * There will always be a voltage or current applied at the terminal so
 * the proper terminal load is required.
 * (2M for voltage input, 250 ohm for current input)
 */
int AdcBt6_ConfigAinSelects(void)
{
	int rc = -EIO;

	if (adcObj.i2c == NULL) {
		return rc;
	}

	adcObj.expander.bits.ain_sel = 0;
	size_t i;
	enum analog_input_1_type config;
	for (i = 0; i < ANALOG_INPUT_NUMBER_OF_CHANNELS; i++) {
		config = attr_get_uint32(ATTR_ID_analog_input_1_type + i, 0);
		if (config == ANALOG_INPUT_1_TYPE_CURRENT_4MA_TO_20MA) {
			adcObj.expander.bits.ain_sel |= (1 << i);
		}
	}

	/* To measure current the correspoding output must be set to 1 */
	uint8_t cmd[] = { TCA9538_REG_OUTPUT, adcObj.expander.byte };
	if (i2c_write(adcObj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS) < 0) {
		LOG_ERR("I2C Failure");
	} else {
		rc = 0;
	}
	return rc;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static AnalogChannel_t GetChannel(AdcMeasurementType_t type)
{
	/* clang-format off */
	switch (type) {
	case ADC_TYPE_VOLTAGE:      return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_CURRENT:      return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_PRESSURE:     return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_ULTRASONIC:   return ANALOG_SENSOR_1_CH;
	case ADC_TYPE_THERMISTOR:   return THERMISTOR_SENSOR_2_CH;
	case ADC_TYPE_VREF:         return VREF_5_CH;
	default:                    return POWER_ADC_CH;
	}
	/* clang-format on */
}

/* Sample a single channel CONFIG_ADC_BT6_OVERSAMPLING times
 * VREF will hold a charge (if thermistor_en was ever active) if there isn't
 * a resistor or thermistor connected.
 */
static int SampleChannel(int16_t *raw, AnalogChannel_t channel)
{
	int rc = -EIO;
	const struct adc_sequence sequence = {
		.options = NULL,
		.channels = BIT(channel),
		.buffer = raw,
		.buffer_size = sizeof(*raw),
		.resolution = ADC_RESOLUTION,
		.oversampling = CONFIG_ADC_BT6_OVERSAMPLING,
		.calibrate = adcObj.calibrate
	};

	if (adcObj.dev) {
		rc = ConfigureChannel(channel);
		if (rc == 0) {
			if (!ADCChannelIsSimulated(channel, raw)) {
				rc = adc_read(adcObj.dev, &sequence);
			}
		}
		if (rc < 0) {
			LOG_ERR("Unable to sample ADC");
		} else {
			adcObj.calibrate = false;
		}
		/* Ignore negative results. */
		*raw = MAX(0, *raw);
	}
	return rc;
}

static int ConfigureChannel(AnalogChannel_t channel)
{
	if (channel == pcfg->channel_id) {
		return 0;
	}

	pcfg->channel_id = channel;
	pcfg->input_positive = channel;

	switch (channel) {
	case ANALOG_SENSOR_1_CH:
		pcfg->gain = ADC_GAIN_DEFAULT;
		pcfg->reference = ADC_REFERENCE_DEFAULT;
		break;

	case THERMISTOR_SENSOR_2_CH:
		pcfg->gain = ADC_GAIN_THERMISTOR;
		pcfg->reference = ADC_REFERENCE_THERMISTOR;
		break;

	case VREF_5_CH:
		pcfg->gain = ADC_GAIN_DEFAULT;
		pcfg->reference = ADC_REFERENCE_DEFAULT;
		break;

	case POWER_ADC_CH:
	default:
		pcfg->gain = ADC_GAIN_DEFAULT;
		pcfg->reference = ADC_REFERENCE_DEFAULT;
		pcfg->input_positive = NRF_SAADC_INPUT_VDD;
		break;
	}

	return adc_channel_setup(adcObj.dev, pcfg);
}

static int InitExpander(void)
{
	int rc = -EIO;
	adcObj.i2c = device_get_binding(I2C_DEV_NAME);
	if (adcObj.i2c == NULL) {
		LOG_ERR("Cannot get I2C device");
		return rc;
	}

	rc = i2c_configure(adcObj.i2c, I2C_CFG);
	if (rc == 0) {
		uint8_t cmd[] = { TCA9538_REG_CONFIG, OUTPUT_CONFIG };
		rc = i2c_write(adcObj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS);
	}

	if (rc < 0) {
		LOG_ERR("I2C failure");
		adcObj.i2c = NULL;
	} else {
		rc = AdcBt6_ConfigAinSelects();
	}
	return rc;
}

/* Two of the expander outputs control the mux that selects one of the
 * 4 analog or thermistor inputs.
 */
static int ConfigMux(MuxInput_t input)
{
	int rc = -EIO;
	if (adcObj.i2c == NULL) {
		return rc;
	}

	if (input >= NUMBER_OF_ANALOG_INPUTS) {
		LOG_ERR("Invalid input %d (required range 0-3)", input);
		return rc;
	}

	if (adcObj.expander.bits.mux != input) {
		adcObj.expander.bits.mux = input;
		uint8_t cmd[] = { TCA9538_REG_OUTPUT, adcObj.expander.byte };
		rc = i2c_write(adcObj.i2c, cmd, sizeof(cmd), EXPANDER_ADDRESS);
		if (rc < 0) {
			LOG_ERR("I2C Failure");
		} else {
			k_busy_wait(MUX_SWITCH_DELAY_US);
		}
	} else {
		rc = 0;
	}
	return rc;
}

static void UltrasonicOrPressurePowerAndDelayHandler(AdcMeasurementType_t type)
{
	if (type == ADC_TYPE_PRESSURE || type == ADC_TYPE_ULTRASONIC) {
		AdcBt6_FiveVoltEnable();
		AdcBt6_BplusEnable();
	}

	if (type == ADC_TYPE_PRESSURE) {
		k_sleep(K_MSEC(PRESSURE_DELAY_MS));
	}

	if (type == ADC_TYPE_ULTRASONIC) {
		k_sleep(K_MSEC(ULTRASONIC_DELAY_MS));
	}
}

static void UltrasonicOrPressureDisablePower(AdcMeasurementType_t type)
{
	if (type == ADC_TYPE_PRESSURE || type == ADC_TYPE_ULTRASONIC) {
		AdcBt6_BplusDisable();
		AdcBt6_FiveVoltDisable();
	}
}

static bool ValidInputForCurrentMeasurement(MuxInput_t input)
{
	return ((BIT(input) & adcObj.expander.bits.ain_sel) != 0);
}

static float Steinhart_Hart(float calibrated, float a, float b, float c)
{
	float result = 0.0f;
	float r = (10000 * calibrated) / (4096 - calibrated);
	float x = log(r);
	float cubed = x * x * x;
	float denominator = a + (b * x) + (c * cubed);

	/* Division by zero is safe for this architecture, but
	 * we need to handle this to protect upper level calls.
	 */
	if (fpclassify(denominator) == FP_NORMAL) {
		result = 1.0f / denominator;
	}

	return (result);
}

static bool ADCChannelIsSimulated(AnalogChannel_t channel,
				  int16_t *simulated_value)
{
	bool is_simulated = false;
	uint8_t channel_index;
	bool channel_found = false;
	bool simulation_enabled = false;

	const uint8_t channel_map[] = { POWER_ADC_CH, ANALOG_SENSOR_1_CH,
					THERMISTOR_SENSOR_2_CH, VREF_5_CH };

	const uint8_t enable_map[] = { ATTR_ID_adc_power_simulated,
				       ATTR_ID_adc_analog_sensor_simulated,
				       ATTR_ID_adc_thermistor_simulated,
				       ATTR_ID_adc_vref_simulated };

	const uint8_t value_map[] = { ATTR_ID_adc_power_simulated_counts,
				      ATTR_ID_adc_analog_sensor_simulated_counts,
				      ATTR_ID_adc_thermistor_simulated_counts,
				      ATTR_ID_adc_vref_simulated_counts };

	/* AD channels don't have incremental values so use a look up
	 * to find the index of the one being accessed.
	 */
	for (channel_index = 0;
	     (!channel_found) && (channel_index < NUMBER_OF_ANALOG_INPUTS);) {
		if (channel_map[channel_index] == channel) {
			channel_found = true;
		} else {
			channel_index++;
		}
	}

	if (channel_found) {
		/* Check if the channel is being simulated */
		if (attr_get(enable_map[channel_index],
				  &simulation_enabled,
				  sizeof(simulation_enabled)) ==
		    sizeof(simulation_enabled)) {
			if (simulation_enabled) {
				/* If so, try to read the simulated value */
				if (attr_get(value_map[channel_index],
						  simulated_value,
						  sizeof(*simulated_value)) ==
				    sizeof(*simulated_value)) {
					/* Only apply the value if safe to do so */
					is_simulated = true;
				}
			}
		}
	}

	return (is_simulated);
}

static bool VoltageIsSimulated(size_t channel, float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	const uint8_t enable_map[] = { ATTR_ID_voltage_1_simulated,
				       ATTR_ID_voltage_2_simulated,
				       ATTR_ID_voltage_3_simulated,
				       ATTR_ID_voltage_4_simulated };

	const uint8_t value_map[] = { ATTR_ID_voltage_1_simulated_value,
				      ATTR_ID_voltage_2_simulated_value,
				      ATTR_ID_voltage_3_simulated_value,
				      ATTR_ID_voltage_4_simulated_value };

	if (channel < TOTAL_ANALOG_CH) {
		/* Check if the voltage is being simulated */
		if (attr_get(enable_map[channel], &simulation_enabled,
				  sizeof(simulation_enabled)) ==
		    sizeof(simulation_enabled)) {
			if (simulation_enabled) {
				/* If so, try to read the simulated value */
				if (attr_get(value_map[channel],
						  simulated_value,
						  sizeof(*simulated_value)) ==
				    sizeof(*simulated_value)) {
					/* Only apply the value if safe to do so */
					is_simulated = true;
					/* And scale it down for correction by the
					 * MeasureAnalogInput function call in the
					 * Sensor Task.
					 */
					*simulated_value /=
						ADC_BT6_VOLTS_TO_MILLIVOLTS;
				}
			}
		}
	}

	return (is_simulated);
}

static bool UltrasonicIsSimulated(float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	if (attr_get(ATTR_ID_ultrasonic_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_ultrasonic_simulated_value,
				     simulated_value,
				     sizeof(*simulated_value)) ==
			    sizeof(*simulated_value)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
			}
		}
	}

	return(is_simulated);
}

static bool PressureIsSimulated(float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	if (attr_get(ATTR_ID_pressure_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_pressure_simulated_value,
				     simulated_value,
				     sizeof(*simulated_value)) ==
			    sizeof(*simulated_value)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
			}
		}
	}

	return(is_simulated);
}

static bool CurrentIsSimulated(size_t channel, float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	const uint8_t enable_map[] = { ATTR_ID_current_1_simulated,
				       ATTR_ID_current_2_simulated,
				       ATTR_ID_current_3_simulated,
				       ATTR_ID_current_4_simulated };

	const uint8_t value_map[] = { ATTR_ID_current_1_simulated_value,
				      ATTR_ID_current_2_simulated_value,
				      ATTR_ID_current_3_simulated_value,
				      ATTR_ID_current_4_simulated_value };

	if (channel < TOTAL_ANALOG_CH) {
		/* Check if the current is being simulated */
		if (attr_get(enable_map[channel], &simulation_enabled,
				  sizeof(simulation_enabled)) ==
		    sizeof(simulation_enabled)) {
			if (simulation_enabled) {
				/* If so, try to read the simulated value */
				if (attr_get(value_map[channel],
						  simulated_value,
						  sizeof(*simulated_value)) ==
				    sizeof(*simulated_value)) {
					/* Only apply the value if safe to do so */
					is_simulated = true;
				}
			}
		}
	}

	return (is_simulated);
}

static bool VrefIsSimulated(float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	if (attr_get(ATTR_ID_vref_simulated, &simulation_enabled,
			  sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_vref_simulated_value,
					  simulated_value,
					  sizeof(*simulated_value)) ==
			    sizeof(*simulated_value)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
			}
		}
	}

	return(is_simulated);
}

static bool TemperatureIsSimulated(size_t channel, float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	const uint8_t enable_map[] = { ATTR_ID_temperature_1_simulated,
				       ATTR_ID_temperature_2_simulated,
				       ATTR_ID_temperature_3_simulated,
				       ATTR_ID_temperature_4_simulated };

	const uint8_t value_map[] = { ATTR_ID_temperature_1_simulated_value,
				      ATTR_ID_temperature_2_simulated_value,
				      ATTR_ID_temperature_3_simulated_value,
				      ATTR_ID_temperature_4_simulated_value };

	if (channel < TOTAL_ANALOG_CH) {
		/* Check if the temperature is being simulated */
		if (attr_get(enable_map[channel], &simulation_enabled,
				  sizeof(simulation_enabled)) ==
		    sizeof(simulation_enabled)) {
			if (simulation_enabled) {
				/* If so, try to read the simulated value */
				if (attr_get(value_map[channel],
						  simulated_value,
						  sizeof(*simulated_value)) ==
				    sizeof(*simulated_value)) {
					/* Only apply the value if safe to do so */
					is_simulated = true;
				}
			}
		}
	}

	return (is_simulated);
}

static bool power_volt_is_simulated(float *simulated_value)
{
	bool is_simulated = false;
	bool simulation_enabled = false;

	if (attr_get(ATTR_ID_power_volts_simulated, &simulation_enabled,
		     sizeof(simulation_enabled)) ==
	    sizeof(simulation_enabled)) {
		if (simulation_enabled) {
			/* If so, try to read the simulated value */
			if (attr_get(ATTR_ID_power_volts_simulated_value, simulated_value,
				     sizeof(float)) == sizeof(float)) {
				/* Only apply the value if safe to do so */
				is_simulated = true;
			}
		}
	}

	return (is_simulated);
}
