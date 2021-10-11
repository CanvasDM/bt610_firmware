/**
 * @file AttributeTable.c
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <string.h>

#include "AttributeTable.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define DRW DEFAULT_RW_ATTRIBUTE_VALUES
#define DRO DEFAULT_RO_ATTRIBUTE_VALUES

/* clang-format off */
#define b   ATTR_TYPE_BOOL
#define u8  ATTR_TYPE_U8
#define u16 ATTR_TYPE_U16
#define u32 ATTR_TYPE_U32
#define u64 ATTR_TYPE_U64
#define i8  ATTR_TYPE_S8
#define i16 ATTR_TYPE_S16
#define i32 ATTR_TYPE_S32
#define i64 ATTR_TYPE_S64
#define f   ATTR_TYPE_FLOAT
#define s   ATTR_TYPE_STRING
#define a   ATTR_TYPE_STRING
/* clang-format on */

/* Add things to the end!  Do not remove items. Change them to deprecated. */
typedef struct RwAttributesTag {
	/* pystart - rw attributes */
	char sensor_name[23 + 1];
	char sensor_location[32 + 1];
	uint16_t advertising_interval;
	uint16_t advertising_duration;
	uint32_t passkey;
	bool lock;
	uint32_t power_sense_interval;
	uint32_t temperature_sense_interval;
	uint8_t aggregation_count;
	bool digital_output_1_state;
	bool digital_output_2_state;
	float high_temp_1_thresh_1;
	float high_temp_1_thresh_2;
	float low_temp_1_thresh_1;
	float low_temp_1_thresh_2;
	float temp_1_delta_thresh;
	float high_temp_2_thresh_1;
	float high_temp_2_thresh_2;
	float low_temp_2_thresh_1;
	float low_temp_2_thresh_2;
	float temp_2_delta_thresh;
	float high_temp_3_thresh_1;
	float high_temp_3_thresh_2;
	float low_temp_3_thresh_1;
	float low_temp_3_thresh_2;
	float temp_3_delta_thresh;
	float high_temp_4_thresh_1;
	float high_temp_4_thresh_2;
	float low_temp_4_thresh_1;
	float low_temp_4_thresh_2;
	float temp_4_delta_thresh;
	float high_analog_1_thresh_1;
	float high_analog_1_thresh_2;
	float low_analog_1_thresh_1;
	float low_analog_1_thresh_2;
	float analog_1_delta_thresh;
	float high_analog_2_thresh_1;
	float high_analog_2_thresh_2;
	float low_analog_2_thresh_1;
	float low_analog_2_thresh_2;
	float analog_2_delta_thresh;
	float high_analog_3_thresh_1;
	float high_analog_3_thresh_2;
	float low_analog_3_thresh_1;
	float low_analog_3_thresh_2;
	float analog_3_delta_thresh;
	float high_analog_4_thresh_1;
	float high_analog_4_thresh_2;
	float low_analog_4_thresh_1;
	float low_analog_4_thresh_2;
	float analog_4_delta_thresh;
	bool active_mode;
	bool use_coded_phy;
	uint8_t tx_power;
	uint16_t network_id;
	uint8_t config_type;
	uint8_t hardware_minor_version;
	float old_coefficient_a;
	float old_coefficient_b;
	float old_coefficient_c;
	uint8_t thermistor_config;
	uint32_t temperature_alarms;
	uint8_t digital_alarms;
	uint8_t digital_input_1_config;
	uint8_t digital_input_2_config;
	uint32_t analog_alarms;
	uint8_t analog_input_1_type;
	uint8_t analog_input_2_type;
	uint8_t analog_input_3_type;
	uint8_t analog_input_4_type;
	uint32_t qrtc_last_set;
	float sh_offset;
	uint32_t analog_sense_interval;
	bool tamper_switch_status;
	uint8_t connection_timeout_sec;
	uint32_t settings_passcode;
	float therm_1_coefficient_a;
	float therm_2_coefficient_a;
	float therm_3_coefficient_a;
	float therm_4_coefficient_a;
	float therm_1_coefficient_b;
	float therm_2_coefficient_b;
	float therm_3_coefficient_b;
	float therm_4_coefficient_b;
	float therm_1_coefficient_c;
	float therm_2_coefficient_c;
	float therm_3_coefficient_c;
	float therm_4_coefficient_c;
	bool data_logging_enable;
	bool factory_reset_enable;
	uint32_t temperature_alarms_enable;
	uint32_t analog_alarms_enable;
	bool adc_power_simulated;
	int16_t adc_power_simulated_counts;
	bool adc_analog_sensor_simulated;
	int16_t adc_analog_sensor_simulated_counts;
	bool adc_thermistor_simulated;
	int16_t adc_thermistor_simulated_counts;
	bool adc_vref_simulated;
	int16_t adc_vref_simulated_counts;
	bool voltage_1_simulated;
	float voltage_1_simulated_value;
	bool voltage_2_simulated;
	float voltage_2_simulated_value;
	bool voltage_3_simulated;
	float voltage_3_simulated_value;
	bool voltage_4_simulated;
	float voltage_4_simulated_value;
	bool ultrasonic_simulated;
	float ultrasonic_simulated_value;
	bool pressure_simulated;
	float pressure_simulated_value;
	bool current_1_simulated;
	float current_1_simulated_value;
	bool current_2_simulated;
	float current_2_simulated_value;
	bool current_3_simulated;
	float current_3_simulated_value;
	bool current_4_simulated;
	float current_4_simulated_value;
	bool vref_simulated;
	float vref_simulated_value;
	bool temperature_1_simulated;
	float temperature_1_simulated_value;
	bool temperature_2_simulated;
	float temperature_2_simulated_value;
	bool temperature_3_simulated;
	float temperature_3_simulated_value;
	bool temperature_4_simulated;
	float temperature_4_simulated_value;
	bool powermv_simulated;
	int32_t powermv_simulated_value;
	bool digital_input_1_simulated;
	bool digital_input_1_simulated_value;
	bool digital_input_2_simulated;
	bool digital_input_2_simulated_value;
	bool mag_switch_simulated;
	bool mag_switch_simulated_value;
	bool tamper_switch_simulated;
	bool tamper_switch_simulated_value;
	uint8_t boot_phy;
	bool mobile_app_disconnect;
	bool block_downgrades;
	bool security_request;
	/* pyend */
} RwAttribute_t;

static const RwAttribute_t DEFAULT_RW_ATTRIBUTE_VALUES = {
	/* pystart - rw defaults */
	.sensor_name = "BT610",
	.sensor_location = "",
	.advertising_interval = 1000,
	.advertising_duration = 15000,
	.passkey = 123456,
	.lock = 0,
	.power_sense_interval = 0,
	.temperature_sense_interval = 60,
	.aggregation_count = 1,
	.digital_output_1_state = 0,
	.digital_output_2_state = 0,
	.high_temp_1_thresh_1 = 1.27e+2,
	.high_temp_1_thresh_2 = 1.27e+2,
	.low_temp_1_thresh_1 = -1.28e+2,
	.low_temp_1_thresh_2 = -1.28e+2,
	.temp_1_delta_thresh = 0,
	.high_temp_2_thresh_1 = 1.27e+2,
	.high_temp_2_thresh_2 = 1.27e+2,
	.low_temp_2_thresh_1 = -1.28e+2,
	.low_temp_2_thresh_2 = -1.28e+2,
	.temp_2_delta_thresh = 0,
	.high_temp_3_thresh_1 = 1.27e+2,
	.high_temp_3_thresh_2 = 1.27e+2,
	.low_temp_3_thresh_1 = -1.28e+2,
	.low_temp_3_thresh_2 = -1.28e+2,
	.temp_3_delta_thresh = 0,
	.high_temp_4_thresh_1 = 1.27e+2,
	.high_temp_4_thresh_2 = 1.27e+2,
	.low_temp_4_thresh_1 = -1.28e+2,
	.low_temp_4_thresh_2 = -1.28e+2,
	.temp_4_delta_thresh = 0,
	.high_analog_1_thresh_1 = 0,
	.high_analog_1_thresh_2 = 0,
	.low_analog_1_thresh_1 = 0,
	.low_analog_1_thresh_2 = 0,
	.analog_1_delta_thresh = 0,
	.high_analog_2_thresh_1 = 0,
	.high_analog_2_thresh_2 = 0,
	.low_analog_2_thresh_1 = 0,
	.low_analog_2_thresh_2 = 0,
	.analog_2_delta_thresh = 0,
	.high_analog_3_thresh_1 = 0,
	.high_analog_3_thresh_2 = 0,
	.low_analog_3_thresh_1 = 0,
	.low_analog_3_thresh_2 = 0,
	.analog_3_delta_thresh = 0,
	.high_analog_4_thresh_1 = 0,
	.high_analog_4_thresh_2 = 0,
	.low_analog_4_thresh_1 = 0,
	.low_analog_4_thresh_2 = 0,
	.analog_4_delta_thresh = 0,
	.active_mode = 0,
	.use_coded_phy = 1,
	.tx_power = 0,
	.network_id = 0,
	.config_type = 0,
	.hardware_minor_version = 0,
	.old_coefficient_a = 1.132e-3,
	.old_coefficient_b = 2.338e-4,
	.old_coefficient_c = 8.780e-8,
	.thermistor_config = 0,
	.temperature_alarms = 0,
	.digital_alarms = 0,
	.digital_input_1_config = 0,
	.digital_input_2_config = 0,
	.analog_alarms = 0,
	.analog_input_1_type = 0,
	.analog_input_2_type = 0,
	.analog_input_3_type = 0,
	.analog_input_4_type = 0,
	.qrtc_last_set = 0,
	.sh_offset = 2.7315e+2,
	.analog_sense_interval = 60,
	.tamper_switch_status = 0,
	.connection_timeout_sec = 60,
	.settings_passcode = 123456,
	.therm_1_coefficient_a = 1.132e-3,
	.therm_2_coefficient_a = 1.132e-3,
	.therm_3_coefficient_a = 1.132e-3,
	.therm_4_coefficient_a = 1.132e-3,
	.therm_1_coefficient_b = 2.338e-4,
	.therm_2_coefficient_b = 2.338e-4,
	.therm_3_coefficient_b = 2.338e-4,
	.therm_4_coefficient_b = 2.338e-4,
	.therm_1_coefficient_c = 8.780e-8,
	.therm_2_coefficient_c = 8.780e-8,
	.therm_3_coefficient_c = 8.780e-8,
	.therm_4_coefficient_c = 8.780e-8,
	.data_logging_enable = 0,
	.factory_reset_enable = 1,
	.temperature_alarms_enable = 0,
	.analog_alarms_enable = 0,
	.adc_power_simulated = 0,
	.adc_power_simulated_counts = 0,
	.adc_analog_sensor_simulated = 0,
	.adc_analog_sensor_simulated_counts = 0,
	.adc_thermistor_simulated = 0,
	.adc_thermistor_simulated_counts = 0,
	.adc_vref_simulated = 0,
	.adc_vref_simulated_counts = 0,
	.voltage_1_simulated = 0,
	.voltage_1_simulated_value = 0.0,
	.voltage_2_simulated = 0,
	.voltage_2_simulated_value = 0.0,
	.voltage_3_simulated = 0,
	.voltage_3_simulated_value = 0.0,
	.voltage_4_simulated = 0,
	.voltage_4_simulated_value = 0.0,
	.ultrasonic_simulated = 0,
	.ultrasonic_simulated_value = 0.0,
	.pressure_simulated = 0,
	.pressure_simulated_value = 0.0,
	.current_1_simulated = 0,
	.current_1_simulated_value = 0.0,
	.current_2_simulated = 0,
	.current_2_simulated_value = 0.0,
	.current_3_simulated = 0,
	.current_3_simulated_value = 0.0,
	.current_4_simulated = 0,
	.current_4_simulated_value = 0.0,
	.vref_simulated = 0,
	.vref_simulated_value = 0.0,
	.temperature_1_simulated = 0,
	.temperature_1_simulated_value = 0.0,
	.temperature_2_simulated = 0,
	.temperature_2_simulated_value = 0.0,
	.temperature_3_simulated = 0,
	.temperature_3_simulated_value = 0.0,
	.temperature_4_simulated = 0,
	.temperature_4_simulated_value = 0.0,
	.powermv_simulated = 0,
	.powermv_simulated_value = 0,
	.digital_input_1_simulated = 0,
	.digital_input_1_simulated_value = 0,
	.digital_input_2_simulated = 0,
	.digital_input_2_simulated_value = 0,
	.mag_switch_simulated = 0,
	.mag_switch_simulated_value = 0,
	.tamper_switch_simulated = 0,
	.tamper_switch_simulated_value = 0,
	.boot_phy = 0,
	.mobile_app_disconnect = 0,
	.block_downgrades = 0,
	.security_request = 0
	/* pyend */
};

typedef struct RoAttributesTag {
	/* pystart - ro attributes */
	char firmware_version[11 + 1];
	char reset_reason[8 + 1];
	char bluetooth_address[12 + 1];
	uint32_t reset_count;
	char bootloader_version[11 + 1];
	int64_t up_time;
	uint8_t config_version;
	float ge;
	float oe;
	float temperature_result_1;
	float temperature_result_2;
	float temperature_result_3;
	float temperature_result_4;
	uint32_t temperature_alarms;
	uint16_t power_voltage_mv;
	uint8_t digital_input;
	float analog_input_1;
	float analog_input_2;
	float analog_input_3;
	float analog_input_4;
	uint32_t flags;
	bool magnet_state;
	char param_path[8 + 1];
	uint32_t battery_age;
	char api_version[11 + 1];
	uint32_t qrtc;
	uint8_t connection_timeout_sec;
	uint8_t log_file_status;
	bool adc_power_simulated;
	int16_t adc_power_simulated_counts;
	bool adc_analog_sensor_simulated;
	int16_t adc_analog_sensor_simulated_counts;
	bool adc_thermistor_simulated;
	int16_t adc_thermistor_simulated_counts;
	bool adc_vref_simulated;
	int16_t adc_vref_simulated_counts;
	bool voltage_1_simulated;
	float voltage_1_simulated_value;
	bool voltage_2_simulated;
	float voltage_2_simulated_value;
	bool voltage_3_simulated;
	float voltage_3_simulated_value;
	bool voltage_4_simulated;
	float voltage_4_simulated_value;
	bool ultrasonic_simulated;
	float ultrasonic_simulated_value;
	bool pressure_simulated;
	float pressure_simulated_value;
	bool current_1_simulated;
	float current_1_simulated_value;
	bool current_2_simulated;
	float current_2_simulated_value;
	bool current_3_simulated;
	float current_3_simulated_value;
	bool current_4_simulated;
	float current_4_simulated_value;
	bool vref_simulated;
	float vref_simulated_value;
	bool temperature_1_simulated;
	float temperature_1_simulated_value;
	bool temperature_2_simulated;
	float temperature_2_simulated_value;
	bool temperature_3_simulated;
	float temperature_3_simulated_value;
	bool temperature_4_simulated;
	float temperature_4_simulated_value;
	bool powermv_simulated;
	int32_t powermv_simulated_value;
	bool digital_input_1_simulated;
	bool digital_input_1_simulated_value;
	bool digital_input_2_simulated;
	bool digital_input_2_simulated_value;
	bool mag_switch_simulated;
	bool mag_switch_simulated_value;
	bool tamper_switch_simulated;
	bool tamper_switch_simulated_value;
	bool mobile_app_disconnect;
	int32_t attr_save_error_code;
	uint8_t settings_passcode_status;
	uint8_t recover_settings_count;
	bool security_request;
	int8_t security_level;
	uint8_t lock_status;
	/* pyend */
} RoAttribute_t;

static const RoAttribute_t DEFAULT_RO_ATTRIBUTE_VALUES = {
	/* pystart - ro defaults */
	.firmware_version = "0.0.0",
	.reset_reason = "0",
	.bluetooth_address = "0",
	.reset_count = 0,
	.bootloader_version = "0.0",
	.up_time = 0,
	.config_version = 0,
	.ge = 1e+0,
	.oe = 0.0,
	.temperature_result_1 = 0,
	.temperature_result_2 = 0,
	.temperature_result_3 = 0,
	.temperature_result_4 = 0,
	.temperature_alarms = 0,
	.power_voltage_mv = 0,
	.digital_input = 0,
	.analog_input_1 = 0,
	.analog_input_2 = 0,
	.analog_input_3 = 0,
	.analog_input_4 = 0,
	.flags = 0,
	.magnet_state = 0,
	.param_path = "/ext",
	.battery_age = 0,
	.api_version = "1.92",
	.qrtc = 0,
	.connection_timeout_sec = 60,
	.log_file_status = 0,
	.adc_power_simulated = 0,
	.adc_power_simulated_counts = 0,
	.adc_analog_sensor_simulated = 0,
	.adc_analog_sensor_simulated_counts = 0,
	.adc_thermistor_simulated = 0,
	.adc_thermistor_simulated_counts = 0,
	.adc_vref_simulated = 0,
	.adc_vref_simulated_counts = 0,
	.voltage_1_simulated = 0,
	.voltage_1_simulated_value = 0.0,
	.voltage_2_simulated = 0,
	.voltage_2_simulated_value = 0.0,
	.voltage_3_simulated = 0,
	.voltage_3_simulated_value = 0.0,
	.voltage_4_simulated = 0,
	.voltage_4_simulated_value = 0.0,
	.ultrasonic_simulated = 0,
	.ultrasonic_simulated_value = 0.0,
	.pressure_simulated = 0,
	.pressure_simulated_value = 0.0,
	.current_1_simulated = 0,
	.current_1_simulated_value = 0.0,
	.current_2_simulated = 0,
	.current_2_simulated_value = 0.0,
	.current_3_simulated = 0,
	.current_3_simulated_value = 0.0,
	.current_4_simulated = 0,
	.current_4_simulated_value = 0.0,
	.vref_simulated = 0,
	.vref_simulated_value = 0.0,
	.temperature_1_simulated = 0,
	.temperature_1_simulated_value = 0.0,
	.temperature_2_simulated = 0,
	.temperature_2_simulated_value = 0.0,
	.temperature_3_simulated = 0,
	.temperature_3_simulated_value = 0.0,
	.temperature_4_simulated = 0,
	.temperature_4_simulated_value = 0.0,
	.powermv_simulated = 0,
	.powermv_simulated_value = 0,
	.digital_input_1_simulated = 0,
	.digital_input_1_simulated_value = 0,
	.digital_input_2_simulated = 0,
	.digital_input_2_simulated_value = 0,
	.mag_switch_simulated = 0,
	.mag_switch_simulated_value = 0,
	.tamper_switch_simulated = 0,
	.tamper_switch_simulated_value = 0,
	.mobile_app_disconnect = 0,
	.attr_save_error_code = 0,
	.settings_passcode_status = 0,
	.recover_settings_count = 0,
	.security_request = 0,
	.security_level = 0,
	.lock_status = 0
	/* pyend */
};

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static RwAttribute_t rw;
static RoAttribute_t ro;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/* Expander for string and other values
 *
 *...............name...value...default....size...writable..readable
 */
#define RW_ATTRS(n) STRINGIFY(n), rw.n, DRW.n, sizeof(rw.n)
#define RW_ATTRX(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n)
#define RO_ATTRS(n) STRINGIFY(n), ro.n, DRO.n, sizeof(ro.n)
#define RO_ATTRX(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n)

#define y true
#define n false

/* If min == max then range isn't checked. */

/* index.....name............................type.savable.writable.readable.lockable.broadcast.deprecated.donotdumpvalidator..min.max. */
/* clang-format off */
AttributeEntry_t attrTable[ATTR_TABLE_SIZE] = {
	/* pystart - attribute table */
	[0  ] = { RW_ATTRS(sensor_name)                   , s  , y, y, y, y, y, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[1  ] = { RW_ATTRS(sensor_location)               , s  , y, y, y, y, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[2  ] = { RW_ATTRX(advertising_interval)          , u16, y, y, y, y, y, n, n, attribute_validator_uint16         , NULL                                      , .min.ux = 500.0     , .max.ux = 10000.0    },
	[3  ] = { RW_ATTRX(advertising_duration)          , u16, y, y, y, y, y, n, n, attribute_validator_uint16         , NULL                                      , .min.ux = 2000.0    , .max.ux = 65535.0    },
	[4  ] = { RW_ATTRX(passkey)                       , u32, y, y, y, y, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 999999.0   },
	[5  ] = { RW_ATTRX(lock)                          , b  , y, n, y, y, n, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[6  ] = { RW_ATTRX(power_sense_interval)          , u32, y, y, y, y, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
	[7  ] = { RW_ATTRX(temperature_sense_interval)    , u32, y, y, y, y, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
	[8  ] = { RW_ATTRX(aggregation_count)             , u8 , y, y, y, y, n, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 1.0       , .max.ux = 32.0       },
	[9  ] = { RW_ATTRX(digital_output_1_state)        , b  , y, y, y, y, y, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[10 ] = { RW_ATTRX(digital_output_2_state)        , b  , y, y, y, y, y, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[11 ] = { RO_ATTRS(firmware_version)              , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[12 ] = { RO_ATTRS(reset_reason)                  , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[13 ] = { RO_ATTRS(bluetooth_address)             , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[14 ] = { RO_ATTRX(reset_count)                   , u32, n, n, y, n, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[15 ] = { RO_ATTRS(bootloader_version)            , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[16 ] = { RO_ATTRX(up_time)                       , i64, n, n, y, n, n, n, n, attribute_validator_int64          , AttributePrepare_up_time                  , .min.ux = 0.0       , .max.ux = 0.0        },
	[17 ] = { RW_ATTRX(high_temp_1_thresh_1)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[18 ] = { RW_ATTRX(high_temp_1_thresh_2)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[19 ] = { RW_ATTRX(low_temp_1_thresh_1)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[20 ] = { RW_ATTRX(low_temp_1_thresh_2)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[21 ] = { RW_ATTRX(temp_1_delta_thresh)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
	[22 ] = { RW_ATTRX(high_temp_2_thresh_1)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[23 ] = { RW_ATTRX(high_temp_2_thresh_2)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[24 ] = { RW_ATTRX(low_temp_2_thresh_1)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[25 ] = { RW_ATTRX(low_temp_2_thresh_2)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[26 ] = { RW_ATTRX(temp_2_delta_thresh)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
	[27 ] = { RW_ATTRX(high_temp_3_thresh_1)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[28 ] = { RW_ATTRX(high_temp_3_thresh_2)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[29 ] = { RW_ATTRX(low_temp_3_thresh_1)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[30 ] = { RW_ATTRX(low_temp_3_thresh_2)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[31 ] = { RW_ATTRX(temp_3_delta_thresh)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
	[32 ] = { RW_ATTRX(high_temp_4_thresh_1)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[33 ] = { RW_ATTRX(high_temp_4_thresh_2)          , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[34 ] = { RW_ATTRX(low_temp_4_thresh_1)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[35 ] = { RW_ATTRX(low_temp_4_thresh_2)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
	[36 ] = { RW_ATTRX(temp_4_delta_thresh)           , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
	[37 ] = { RW_ATTRX(high_analog_1_thresh_1)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[38 ] = { RW_ATTRX(high_analog_1_thresh_2)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[39 ] = { RW_ATTRX(low_analog_1_thresh_1)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[40 ] = { RW_ATTRX(low_analog_1_thresh_2)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[41 ] = { RW_ATTRX(analog_1_delta_thresh)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[42 ] = { RW_ATTRX(high_analog_2_thresh_1)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[43 ] = { RW_ATTRX(high_analog_2_thresh_2)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[44 ] = { RW_ATTRX(low_analog_2_thresh_1)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[45 ] = { RW_ATTRX(low_analog_2_thresh_2)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[46 ] = { RW_ATTRX(analog_2_delta_thresh)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[47 ] = { RW_ATTRX(high_analog_3_thresh_1)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[48 ] = { RW_ATTRX(high_analog_3_thresh_2)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[49 ] = { RW_ATTRX(low_analog_3_thresh_1)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[50 ] = { RW_ATTRX(low_analog_3_thresh_2)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[51 ] = { RW_ATTRX(analog_3_delta_thresh)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[52 ] = { RW_ATTRX(high_analog_4_thresh_1)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[53 ] = { RW_ATTRX(high_analog_4_thresh_2)        , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[54 ] = { RW_ATTRX(low_analog_4_thresh_1)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[55 ] = { RW_ATTRX(low_analog_4_thresh_2)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[56 ] = { RW_ATTRX(analog_4_delta_thresh)         , f  , y, y, y, y, y, n, n, attribute_validator_float          , NULL                                      , .min.fx = 0.0       , .max.fx = 15000.0    },
	[57 ] = { RW_ATTRX(active_mode)                   , b  , y, y, y, n, y, n, n, attribute_validator_cp8            , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[58 ] = { RW_ATTRX(use_coded_phy)                 , b  , y, y, y, y, y, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[59 ] = { RW_ATTRX(tx_power)                      , u8 , y, y, y, n, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 13.0       },
	[60 ] = { RW_ATTRX(network_id)                    , u16, y, y, y, y, y, n, n, attribute_validator_uint16         , NULL                                      , .min.ux = 0.0       , .max.ux = 65535.0    },
	[61 ] = { RO_ATTRX(config_version)                , u8 , n, n, y, n, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
	[62 ] = { RW_ATTRX(config_type)                   , u8 , y, y, y, y, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 6.0        },
	[63 ] = { RW_ATTRX(hardware_minor_version)        , u8 , y, y, y, n, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 9.0        },
	[64 ] = { RO_ATTRX(ge)                            , f  , n, n, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = -5.0      , .max.fx = 3.4e+38    },
	[65 ] = { RO_ATTRX(oe)                            , f  , n, n, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = -16.0     , .max.fx = 3.4e+38    },
	[66 ] = { RW_ATTRX(old_coefficient_a)             , f  , y, y, y, y, n, y, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[67 ] = { RW_ATTRX(old_coefficient_b)             , f  , y, y, y, y, n, y, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[68 ] = { RW_ATTRX(old_coefficient_c)             , f  , y, y, y, y, n, y, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[69 ] = { RW_ATTRX(thermistor_config)             , u8 , y, y, y, y, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 15.0       },
	[70 ] = { RO_ATTRX(temperature_result_1)          , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_temperature_result_1     , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[71 ] = { RO_ATTRX(temperature_result_2)          , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_temperature_result_2     , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[72 ] = { RO_ATTRX(temperature_result_3)          , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_temperature_result_3     , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[73 ] = { RO_ATTRX(temperature_result_4)          , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_temperature_result_4     , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[74 ] = { RO_ATTRX(temperature_alarms)            , u32, n, y, y, y, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
	[75 ] = { RO_ATTRX(power_voltage_mv)              , u16, n, n, y, n, n, n, n, attribute_validator_uint16         , AttributePrepare_power_voltage_mv         , .min.ux = 0.0       , .max.ux = 3800.0     },
	[76 ] = { RO_ATTRX(digital_input)                 , u8 , n, n, y, y, n, n, n, attribute_validator_uint8          , AttributePrepare_digital_input            , .min.ux = 0.0       , .max.ux = 3.0        },
	[77 ] = { RW_ATTRX(digital_alarms)                , u8 , y, y, y, y, n, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 3.0        },
	[78 ] = { RW_ATTRX(digital_input_1_config)        , u8 , y, y, y, y, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 131.0      },
	[79 ] = { RW_ATTRX(digital_input_2_config)        , u8 , y, y, y, y, y, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 131.0      },
	[80 ] = { RO_ATTRX(analog_input_1)                , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_analog_input_1           , .min.fx = 0.0       , .max.fx = 15000.0    },
	[81 ] = { RO_ATTRX(analog_input_2)                , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_analog_input_2           , .min.fx = 0.0       , .max.fx = 15000.0    },
	[82 ] = { RO_ATTRX(analog_input_3)                , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_analog_input_3           , .min.fx = 0.0       , .max.fx = 15000.0    },
	[83 ] = { RO_ATTRX(analog_input_4)                , f  , n, n, y, n, n, n, n, attribute_validator_float          , AttributePrepare_analog_input_4           , .min.fx = 0.0       , .max.fx = 15000.0    },
	[84 ] = { RW_ATTRX(analog_alarms)                 , u32, y, y, y, y, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
	[85 ] = { RW_ATTRX(analog_input_1_type)           , u8 , y, y, y, y, y, n, n, attribute_validator_aic            , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
	[86 ] = { RW_ATTRX(analog_input_2_type)           , u8 , y, y, y, y, y, n, n, attribute_validator_aic            , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
	[87 ] = { RW_ATTRX(analog_input_3_type)           , u8 , y, y, y, y, y, n, n, attribute_validator_aic            , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
	[88 ] = { RW_ATTRX(analog_input_4_type)           , u8 , y, y, y, y, y, n, n, attribute_validator_aic            , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
	[89 ] = { RO_ATTRX(flags)                         , u32, n, n, y, n, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[90 ] = { RO_ATTRX(magnet_state)                  , b  , n, n, y, n, n, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[91 ] = { RO_ATTRS(param_path)                    , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[92 ] = { RO_ATTRX(battery_age)                   , u32, n, n, y, n, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[93 ] = { RO_ATTRS(api_version)                   , s  , n, n, y, n, n, n, n, attribute_validator_string         , NULL                                      , .min.ux = 0         , .max.ux = 0          },
	[94 ] = { RO_ATTRX(qrtc)                          , u32, n, n, y, n, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[95 ] = { RW_ATTRX(qrtc_last_set)                 , u32, y, n, y, n, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[96 ] = { RW_ATTRX(sh_offset)                     , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[97 ] = { RW_ATTRX(analog_sense_interval)         , u32, y, y, y, y, y, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
	[98 ] = { RW_ATTRX(tamper_switch_status)          , b  , y, n, y, n, y, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[99 ] = { RO_ATTRX(connection_timeout_sec)        , u8 , n, y, y, y, y, y, n, attribute_validator_cp8            , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
	[100] = { RW_ATTRX(settings_passcode)             , u32, y, y, n, n, y, n, n, attribute_validator_cp32           , NULL                                      , .min.ux = 0.0       , .max.ux = 999999.0   },
	[101] = { RW_ATTRX(therm_1_coefficient_a)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[102] = { RW_ATTRX(therm_2_coefficient_a)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[103] = { RW_ATTRX(therm_3_coefficient_a)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[104] = { RW_ATTRX(therm_4_coefficient_a)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[105] = { RW_ATTRX(therm_1_coefficient_b)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[106] = { RW_ATTRX(therm_2_coefficient_b)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[107] = { RW_ATTRX(therm_3_coefficient_b)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[108] = { RW_ATTRX(therm_4_coefficient_b)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[109] = { RW_ATTRX(therm_1_coefficient_c)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[110] = { RW_ATTRX(therm_2_coefficient_c)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[111] = { RW_ATTRX(therm_3_coefficient_c)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[112] = { RW_ATTRX(therm_4_coefficient_c)         , f  , y, y, y, y, n, n, n, attribute_validator_float          , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
	[113] = { RW_ATTRX(data_logging_enable)           , b  , y, y, y, y, y, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0         , .max.ux = 1          },
	[114] = { RW_ATTRX(factory_reset_enable)          , b  , y, y, y, y, n, n, n, attribute_validator_bool           , NULL                                      , .min.ux = 0         , .max.ux = 1          },
	[115] = { RO_ATTRX(log_file_status)               , u8 , n, n, y, n, n, n, n, attribute_validator_uint8          , AttributePrepare_log_file_status          , .min.ux = 0         , .max.ux = 3          },
	[116] = { RW_ATTRX(temperature_alarms_enable)     , u32, y, y, y, y, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
	[117] = { RW_ATTRX(analog_alarms_enable)          , u32, y, y, y, y, n, n, n, attribute_validator_uint32         , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
	[118] = { RO_ATTRX(adc_power_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[119] = { RO_ATTRX(adc_power_simulated_counts)    , i16, n, y, y, n, n, n, y, attribute_validator_int16          , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
	[120] = { RO_ATTRX(adc_analog_sensor_simulated)   , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[121] = { RO_ATTRX(adc_analog_sensor_simulated_counts), i16, n, y, y, n, n, n, y, attribute_validator_int16          , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
	[122] = { RO_ATTRX(adc_thermistor_simulated)      , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[123] = { RO_ATTRX(adc_thermistor_simulated_counts), i16, n, y, y, n, n, n, y, attribute_validator_int16          , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
	[124] = { RO_ATTRX(adc_vref_simulated)            , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[125] = { RO_ATTRX(adc_vref_simulated_counts)     , i16, n, y, y, n, n, n, y, attribute_validator_int16          , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
	[126] = { RO_ATTRX(voltage_1_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[127] = { RO_ATTRX(voltage_1_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[128] = { RO_ATTRX(voltage_2_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[129] = { RO_ATTRX(voltage_2_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[130] = { RO_ATTRX(voltage_3_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[131] = { RO_ATTRX(voltage_3_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[132] = { RO_ATTRX(voltage_4_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[133] = { RO_ATTRX(voltage_4_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[134] = { RO_ATTRX(ultrasonic_simulated)          , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[135] = { RO_ATTRX(ultrasonic_simulated_value)    , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[136] = { RO_ATTRX(pressure_simulated)            , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[137] = { RO_ATTRX(pressure_simulated_value)      , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[138] = { RO_ATTRX(current_1_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[139] = { RO_ATTRX(current_1_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[140] = { RO_ATTRX(current_2_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[141] = { RO_ATTRX(current_2_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[142] = { RO_ATTRX(current_3_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[143] = { RO_ATTRX(current_3_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[144] = { RO_ATTRX(current_4_simulated)           , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[145] = { RO_ATTRX(current_4_simulated_value)     , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[146] = { RO_ATTRX(vref_simulated)                , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[147] = { RO_ATTRX(vref_simulated_value)          , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[148] = { RO_ATTRX(temperature_1_simulated)       , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[149] = { RO_ATTRX(temperature_1_simulated_value) , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[150] = { RO_ATTRX(temperature_2_simulated)       , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[151] = { RO_ATTRX(temperature_2_simulated_value) , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[152] = { RO_ATTRX(temperature_3_simulated)       , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[153] = { RO_ATTRX(temperature_3_simulated_value) , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[154] = { RO_ATTRX(temperature_4_simulated)       , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[155] = { RO_ATTRX(temperature_4_simulated_value) , f  , n, y, y, n, n, n, y, attribute_validator_float          , NULL                                      , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
	[156] = { RO_ATTRX(powermv_simulated)             , b  , n, y, y, n, n, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[157] = { RO_ATTRX(powermv_simulated_value)       , i32, n, y, y, n, n, n, y, attribute_validator_int32          , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[158] = { RO_ATTRX(digital_input_1_simulated)     , b  , n, y, y, n, n, n, y, attribute_validator_din1simen      , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[159] = { RO_ATTRX(digital_input_1_simulated_value), b  , n, y, y, n, n, n, y, attribute_validator_din1sim        , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[160] = { RO_ATTRX(digital_input_2_simulated)     , b  , n, y, y, n, n, n, y, attribute_validator_din2simen      , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[161] = { RO_ATTRX(digital_input_2_simulated_value), b  , n, y, y, n, n, n, y, attribute_validator_din2sim        , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[162] = { RO_ATTRX(mag_switch_simulated)          , b  , n, y, y, n, n, n, y, attribute_validator_magsimen       , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[163] = { RO_ATTRX(mag_switch_simulated_value)    , b  , n, y, y, n, n, n, y, attribute_validator_magsim         , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[164] = { RO_ATTRX(tamper_switch_simulated)       , b  , n, y, y, n, n, n, y, attribute_validator_tampsimen      , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[165] = { RO_ATTRX(tamper_switch_simulated_value) , b  , n, y, y, n, n, n, y, attribute_validator_tampsim        , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[166] = { RW_ATTRX(boot_phy)                      , u8 , y, y, y, n, n, n, y, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 2.0        },
	[167] = { RO_ATTRX(mobile_app_disconnect)         , b  , n, y, y, n, y, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[168] = { RO_ATTRX(attr_save_error_code)          , i32, n, n, y, n, y, n, y, attribute_validator_int32          , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[169] = { RO_ATTRX(settings_passcode_status)      , u8 , n, n, y, n, n, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 2.0        },
	[170] = { RO_ATTRX(recover_settings_count)        , u8 , n, n, y, n, n, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
	[171] = { RW_ATTRX(block_downgrades)              , b  , y, y, y, y, n, n, n, attribute_validator_block_downgrades, NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[172] = { RO_ATTRX(security_request)              , b  , n, y, n, n, y, n, y, attribute_validator_bool           , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
	[173] = { RO_ATTRX(security_level)                , i8 , n, n, y, n, n, n, n, attribute_validator_int8           , AttributePrepare_security_level           , .min.sx = -1.0      , .max.sx = 4.0        },
	[174] = { RO_ATTRX(lock_status)                   , u8 , n, n, y, n, n, n, n, attribute_validator_uint8          , NULL                                      , .min.ux = 0.0       , .max.ux = 2.0        }
	/* pyend */
};
/* clang-format on */

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
/**
 * @brief Copy defaults (before loading from flash)
 */
void AttributeTable_Initialize(void)
{
	memcpy(&rw, &DRW, sizeof(RwAttribute_t));
	memcpy(&ro, &DRO, sizeof(RoAttribute_t));
}

/**
 * @brief set non-backup values to default
 */
void AttributeTable_FactoryReset(void)
{
	size_t i = 0;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		memcpy(attrTable[i].pData, attrTable[i].pDefault,
		       attrTable[i].size);
	}
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* pystart - prepare for read weak implementations */
__weak int AttributePrepare_up_time(void)
{
	return 0;
}

__weak int AttributePrepare_temperature_result_1(void)
{
	return 0;
}

__weak int AttributePrepare_temperature_result_2(void)
{
	return 0;
}

__weak int AttributePrepare_temperature_result_3(void)
{
	return 0;
}

__weak int AttributePrepare_temperature_result_4(void)
{
	return 0;
}

__weak int AttributePrepare_power_voltage_mv(void)
{
	return 0;
}

__weak int AttributePrepare_digital_input(void)
{
	return 0;
}

__weak int AttributePrepare_analog_input_1(void)
{
	return 0;
}

__weak int AttributePrepare_analog_input_2(void)
{
	return 0;
}

__weak int AttributePrepare_analog_input_3(void)
{
	return 0;
}

__weak int AttributePrepare_analog_input_4(void)
{
	return 0;
}

__weak int AttributePrepare_log_file_status(void)
{
	return 0;
}

__weak int AttributePrepare_security_level(void)
{
	return 0;
}

/* pyend */
