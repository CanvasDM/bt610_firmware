/**
 * @file AdcRead.c
 * @brief This will read the ADC pins and report back the value as a voltage
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "AdcRead.h"
#include <drivers/adc.h>
#include <string.h>
#include <hal/nrf_saadc.h>
#include <zephyr.h>
#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(AdcRead);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
// ADC Sampling Settings
//#define CONFIG_ADC_CONFIGURABLE_INPUTS
#define ADC_DEVICE_NAME                                                        \
	DT_LABEL(DT_INST(                                                      \
		0,                                                             \
		nordic_nrf_saadc)) //(DT_LABEL(DT_NODELABEL(adc)))//DT_ALIAS_ADC_0_LABEL
#define ADC_RESOLUTION		           (12)
#define ADC_GAIN	                   ADC_GAIN_1_6
#define ADC_REFERENCE                  ADC_REF_INTERNAL
#define ADC_GAIN_THERMISTOR		       ADC_GAIN_1_4
#define ADC_REFERENCE_THERMISTOR       ADC_REF_VDD_1_4
#define ADC_ACQUISITION_TIME	       ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 10)
#define ADC_VDD_CHANNEL_INPUT          NRF_SAADC_INPUT_VDD
#define BUFFER_SIZE			           (6)
#define BAD_ANALOG_READ                (0)
#define ADC_REF_VOLTAGE_IN_MILLIVOLTS                                          \
	600 //!< Reference voltage (in milli volts) used by ADC while doing conversion.
#define ADC_RES_10BIT                  1024 //!< Maximum digital value for 10-bit ADC conversion.
#define ADC_RES_12BIT                  4096
#define ADC_PRE_SCALING_COMPENSATION                                           \
	6 //!< The ADC is configured to use VDD with 1/3 prescaling as input. And hence the result of conversion is to be multiplied by 3 to get the actual value of the battery voltage.
#define ADC_RESULT_IN_MILLI_VOLTS(ADC_VALUE) \
	((((ADC_VALUE)*ADC_REF_VOLTAGE_IN_MILLIVOLTS) / ADC_RES_12BIT) *       \
	 ADC_PRE_SCALING_COMPENSATION)

#define ADC_IN1                 NRF_SAADC_INPUT_AIN0
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static bool _IsInitialized = false;
static uint8_t _LastChannel = 250;
static int16_t m_sample_buffer[BUFFER_SIZE];

// the channel configuration with channel not yet filled in
static struct adc_channel_cfg m_1st_channel_cfg = {
	.gain             = ADC_GAIN,
	.reference        = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id       = 0, // gets set during init
	.differential	  = 0,
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
	.input_positive   = ADC_VDD_CHANNEL_INPUT, // gets set during init
#endif
};

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static struct device *getAdcDevice(void);
static int16_t readOneChannel(AnalogTypesChannel_t channelReading,
			      uint8_t input);
struct device *init_adc(AnalogTypesChannel_t channelReading, uint8_t inputPin);
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
//-----------------------------------------------
// @brief 	This function returns the battery voltage.
uint32_t ADC_GetBatteryMv(void)
{
    uint32_t millivolts;
	int16_t analogValue;

    //TAKE_SEMAPHORE(adcControl.adcBusy);
    analogValue = readOneChannel(BATTERY_ADC_CH, NRF_SAADC_INPUT_VDD);
    //Reset battery math
    //adcControl.batteryMath = 0;
    //GIVE_SEMAPHORE(adcControl.adcBusy);
	//millivolts = ADC_RESULT_IN_MILLI_VOLTS(analogValue);
	millivolts = analogValue;
	//LOG_DBG("analogBatt = %d\n",analogValue);
	adc_raw_to_millivolts(ADC_REFERENCE, ADC_GAIN, ADC_RESOLUTION,
				&millivolts);
	return (analogValue);
}

// ------------------------------------------------
// high level read adc channel and convert to float voltage
// ------------------------------------------------
uint32_t AnalogRead(AnalogTypesChannel_t channelReading)
{
	int16_t sv = readOneChannel(channelReading, channelReading);
	//	if(sv == BAD_ANALOG_READ)
	//	{
	//		return sv;
	//	}

	// Convert the result to voltage
	// Result = [V(p) - V(n)] * GAIN/REFERENCE / 2^(RESOLUTION)
	int multip = 256;
	// find 2**adc_resolution
	switch (ADC_RESOLUTION) {
	default:
	case 8:
			multip = 256;
			break;
	case 10:
			multip = 1024;
			break;
	case 12:
			multip = 4096;
			break;
	case 14:
			multip = 16384;
			break;
	}

	// the 3.6 relates to the voltage divider being used in my circuit
	//float fout = (sv * 3.6 / multip); todo ?
	return (uint32_t)sv;
}

// ------------------------------------------------
// simple analog test code
// ------------------------------------------------
double TestAnalog(int channel)
{
	float ft1 = AnalogRead(channel);
	return ft1;
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
// initialize the adc channel
struct device *init_adc(AnalogTypesChannel_t channelReading, uint8_t inputPin)
{
	int ret;
	struct device *adc_dev = getAdcDevice();
    __ASSERT(adc_dev, "Failed to get device binding");

	if (_LastChannel != channelReading) {
		_IsInitialized = false;
		_LastChannel = channelReading;
	}

	if (adc_dev != NULL && !_IsInitialized) {
		m_1st_channel_cfg.channel_id = channelReading;
		if (channelReading == THERMISTOR_SENSOR_2_CH) {
			m_1st_channel_cfg.reference = ADC_REFERENCE_THERMISTOR;
			m_1st_channel_cfg.gain = ADC_GAIN_THERMISTOR;
		} else {
			m_1st_channel_cfg.reference = ADC_REFERENCE;
			m_1st_channel_cfg.gain = ADC_GAIN;
		}
#if defined(CONFIG_ADC_CONFIGURABLE_INPUTS)
		m_1st_channel_cfg.input_positive = inputPin; //channel+1,
#endif
		ret = adc_channel_setup(adc_dev, &m_1st_channel_cfg);
		if (ret != 0) {
			//ASeries.printf("Setting up of the first channel failed with code %d", ret);
			adc_dev = NULL;
		} else {
			_IsInitialized =
				true; // we don't have any other analog users
		}
	}

	memset(m_sample_buffer, 0, sizeof(m_sample_buffer));

	return adc_dev;
}
// return device* for the adc
static struct device *getAdcDevice(void)
{
	return device_get_binding(ADC_DEVICE_NAME);
}
// ------------------------------------------------
// read one channel of adc
// ------------------------------------------------
static int16_t readOneChannel(AnalogTypesChannel_t channelReading,
			      uint8_t inputPin)
{
	const struct adc_sequence sequence = {
		.options     = NULL,				// extra samples and callback
		.channels    = BIT(channelReading),		// bit mask of channels to read
		.buffer      = m_sample_buffer,		// where to put samples read
		.buffer_size = sizeof(m_sample_buffer),
		.resolution  = ADC_RESOLUTION,		// desired resolution
		.oversampling = 0,					// don't oversample
		.calibrate = 0						// don't calibrate
	};

	int ret;
	static volatile int16_t sample_value = BAD_ANALOG_READ;

	struct device *adc_dev = init_adc(channelReading, inputPin);

	if (adc_dev) {
		ret = adc_read(adc_dev, &sequence);
		if (ret == 0) {
			sample_value = m_sample_buffer[0];
		}
	}

	return sample_value;
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
