/**
 * @file AdcBt6.h
 * @brief ADC functions for BT6xx.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BT6_ADC_H__
#define __BT6_ADC_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
typedef enum AnalogChannel {
	BATTERY_ADC_CH = 0,
	ANALOG_SENSOR_1_CH = 1, /* AIN 0 */
	THERMISTOR_SENSOR_2_CH = 2, /* AIN 1 */
	VREF_5_CH = 6, /* AIN 5 */
	INVALID_CH
} AnalogChannel_t;

/* The mux is indexed from 0 to 3 */
typedef enum MuxInput {
	MUX_AIN1_THERM1 = 0,
	MUX_AIN2_THERM2,
	MUX_AIN3_THERM3,
	MUX_AIN4_THERM4,
	NUMBER_OF_ANALOG_INPUTS
} MuxInput_t;

typedef enum AdcMeasurementType {
	ADC_TYPE_VOLTAGE = 0,
	ADC_TYPE_CURRENT,
	ADC_TYPE_PRESSURE,
	ADC_TYPE_ULTRASONIC,
	ADC_TYPE_THERMISTOR,
	ADC_TYPE_VREF,
	NUMBER_OF_ADC_TYPES
} AdcMeasurementType_t;

/* The 5V for pressure/ultrasonic is enabled during conversions when required. */
typedef enum AdcPwrSequence {
	/* Enable/Disable power required for measurement */
	ADC_PWR_SEQ_SINGLE = 0,
	/* Enable analog/thermistor circuit power.
	 * Analog and Thermistor cannot be interleaved.
	 */
	ADC_PWR_SEQ_START,
	/* Circuit Power setting remains unchanged */
	ADC_PWR_SEQ_CONTINUE,
	/* Disable Power when done*/
	ADC_PWR_SEQ_END,
	/* Don't do anything with power */
	ADC_PWR_SEQ_BYPASS
} AdcPwrSequence_t;

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

/**
 * @brief Initialize ADC for BT6xx
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_Init(void);

/**
 * @brief Read battery voltage
 *
 * @param pointer to raw
 * @param pointer to millivolts
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_ReadBatteryMv(int16_t *raw, int32_t *mv);

/**
 * @brief Measure analog input or thermistor input
 *
 * @param pointer to raw
 * @param input is the input (0-3) to measure
 * @param type of measurement to perform
 * @param pwr Use a power sequence otherwise the user must
 * call ConfigPower and DisablePower before and this operation.
 *
 * @retval 0 on success, negative otherwise
 */
int AdcBt6_Measure(int16_t *raw, MuxInput_t input, AdcMeasurementType_t type,
		   AdcPwrSequence_t power);

/**
 * @brief Calibrate thermistor with therm1 connected to a  ~560 Ohm resistor
 * and therm2 connected to a ~330K Ohm resistor.
 *
 * @param c1 is the ideal ADC counts for the 560 Ohm resistor
 * @param c2 is the ideal ADC count for the 330K Ohm resistor
 * @param ge is the gain error
 * @param oe is the offset error
 *
 * @note Example
 * R1 = 559.4 ohms --> C1 = 4096 * 559.4/(10000 + 559.4) = 217.0
 * R2 = 331.9k --> C2 = 4096 * 331900/(10000 + 331900) = 3976.2
 * Measure R1 repeatedly to get average M1.
 * Measure R2 repeatedly to get average M2.
 * GE = (M2 – M1)/ 3759.2
 * OE = M1 – (GE * 217)
 *
 * @retval 0 on success, negative error code otherwise
 *
 */
int AdcBt6_CalibrateThermistor(float c1, float c2, float *ge, float *oe);

/**
 * @brief Enable power to analog, thermistor, OR vref circuitry.
 *
 * @note Busy waits for a power-up delay after setting enable.
 *
 * @param type of measurement to perform
 */
void AdcBt6_ConfigPower(AdcMeasurementType_t type);

/**
 * @brief Disable power to analog, thermistor, and vref circuitry.
 */
void AdcBt6_DisablePower(void);

/**
 * @brief Helper Functions
 *
 * @note These functions include the required switching time delays when
 * power is enabled. Enable 5V output before enabling b+.  Disable 5V
 * after disabling b+.
 *
 * @retval negative on error, 0 on success.
 */
int AdcBt6_FiveVoltEnable(void);
int AdcBt6_FiveVoltDisable(void);
int AdcBt6_BplusEnable(void);
int AdcBt6_BplusDisable(void);

/**
 * @brief Conversion function
 *
 * @retval analog input voltage scaled to circuitry (Volts)
 */
float AdcBt6_ConvertVoltage(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval millimeters
 */
float AdcBt6_ConvertUltrasonic(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval PSI
 */
float AdcBt6_ConvertPressure(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval analog input current scaled to circuitry (Volts)
 */
float AdcBt6_ConvertCurrent(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval analog input AC current scaled to circuitry (Volts)
 */
float AdcBt6_ConvertACCurrent20(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval analog input AC current scaled to circuitry (Volts)
 */
float AdcBt6_ConvertACCurrent150(int32_t raw);

/**
 * @brief Conversion function
 *
 * @retval analog input AC current scaled to circuitry (Volts)
 */
float AdcBt6_ConvertACCurrent500(int32_t raw);

/**
 * @brief Configure the analog input selection.
 *
 * @note
 * The AINx_SEL lines need to be maintained at all times.
 * There will always be a voltage or current applied at the terminal so
 * the proper terminal load is required.
 * (2M for voltage input, 250 ohm for current input)
 *
 * @retval negative error code, 0 on success
 */
/* todo: this needs to be called from sensor task when type 1-4 changes */
int AdcBt6_ConfigAinSelects(void);

/**
 * @brief Conversion function
 *
 * @retval vref in Volts
 */
float AdcBt6_ConvertVref(int32_t raw);

/**
 * @brief Conversion function that applies thermistor calibration
 * to result.
 *
 * @retval (raw - oe)/ge
 */
float AdcBt6_ApplyThermistorCalibration(int32_t raw);

/**
 * @brief Use Steinhart-Hart equation to calculate temperature.
 *
 * @retval temperature in Celsius
 */
float AdcBt6_ConvertThermToTemperature(int32_t raw, size_t channel);

/**
 * @brief Get type enum as string.
 */
const char *AdcBt6_GetTypeString(AdcMeasurementType_t type);

#ifdef __cplusplus
}
#endif

#endif /* __BT6_ADC_H__ */
