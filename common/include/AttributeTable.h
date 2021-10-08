/**
 * @file AttributeTable.h
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ATTRIBUTE_TABLE_H__
#define __ATTRIBUTE_TABLE_H__

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
typedef size_t attr_idx_t;

typedef enum {
	ATTR_TYPE_UNKNOWN = 0,
	ATTR_TYPE_BOOL,
	ATTR_TYPE_U8,
	ATTR_TYPE_U16,
	ATTR_TYPE_U32,
	ATTR_TYPE_U64,
	ATTR_TYPE_S8,
	ATTR_TYPE_S16,
	ATTR_TYPE_S32,
	ATTR_TYPE_S64,
	ATTR_TYPE_FLOAT,
	ATTR_TYPE_STRING,
	ATTR_TYPE_ANY
} AttrType_t;

#define ATTR_TYPE_BYTE_ARRAY ATTR_TYPE_STRING

typedef struct minmax {
	union {
		uint32_t ux;
		int32_t sx;
		float fx;
	};
} minmax_t;

typedef struct AttributeEntry AttributeEntry_t;

struct AttributeEntry {
	const char *const name;
	void *pData;
	void const *const pDefault;
	const size_t size;
	const AttrType_t type;
	const bool savable;
	const bool writable;
	const bool readable;
	const bool lockable;
	const bool broadcast;
	const bool deprecated;
	const bool donotdump;
	int (*const pValidator)(AttributeEntry_t *, void *, size_t, bool);
	int (*const pPrepare)(void);
	const minmax_t min;
	const minmax_t max;
	bool modified;
};

/* pystart - attribute table size */
#define ATTR_TABLE_SIZE 175

/* pyend */

/* The code generator tool should generate this */
#define ATTR_MAX_STR_LENGTH 100

#define ATTR_MAX_STR_SIZE (ATTR_MAX_STR_LENGTH + 1)

#define ATTR_MAX_HEX_SIZE 8

#define ATTR_MAX_VERSION_LENGTH 11

typedef enum
{
	CONFIG_UNDEFINED = 0,
	CONFIG_ANALOG_INPUT,
	CONFIG_DIGITAL,
	CONFIG_TEMPERATURE,
	CONFIG_ANALOG_AC_CURRENT,
	CONFIG_ULTRASONIC_PRESSURE,
	CONFIG_SPI_I2C,
} configType_t;

typedef enum
{
	ANALOG_UNUSED = 0,
	ANALOG_VOLTAGE,
	ANALOG_CURRENT,
	ANALOG_PRESSURE,
	ANALOG_ULTRASONIC,
	ANALOG_CURRENT20A,
	ANALOG_CURRENT150A,
	ANALOG_CURRENT500A,
} analogConfigType_t;

typedef enum
{
	BOOT_PHY_TYPE_DEFAULT = 0,
	BOOT_PHY_TYPE_1M,
	BOOT_PHY_TYPE_CODED,
	BOOT_PHY_TYPE_COUNT
} bootPHYType_t;

typedef enum
{
	LOCK_STATUS_NOT_SETUP = 0,
	LOCK_STATUS_SETUP_ENGAGED,
	LOCK_STATUS_SETUP_DISENGAGED,
	LOCK_STATUS_COUNT
} lockStatusType_t;

typedef enum {
        SETTINGS_LOCK_ERROR_NO_STATUS = 0,
        SETTINGS_LOCK_ERROR_VALID_CODE,
        SETTINGS_LOCK_ERROR_INVALID_CODE
} settingsLockErrorType_t;

/******************************************************************************/
/* Indices                                                                    */
/******************************************************************************/
/* clang-format off */
/* pystart - attribute indices */
#define ATTR_INDEX_sensor_name                           0
#define ATTR_INDEX_sensor_location                       1
#define ATTR_INDEX_advertising_interval                  2
#define ATTR_INDEX_advertising_duration                  3
#define ATTR_INDEX_passkey                               4
#define ATTR_INDEX_lock                                  5
#define ATTR_INDEX_power_sense_interval                  6
#define ATTR_INDEX_temperature_sense_interval            7
#define ATTR_INDEX_aggregation_count                     8
#define ATTR_INDEX_digital_output_1_state                9
#define ATTR_INDEX_digital_output_2_state                10
#define ATTR_INDEX_firmware_version                      11
#define ATTR_INDEX_reset_reason                          12
#define ATTR_INDEX_bluetooth_address                     13
#define ATTR_INDEX_reset_count                           14
#define ATTR_INDEX_bootloader_version                    15
#define ATTR_INDEX_up_time                               16
#define ATTR_INDEX_high_temp_1_thresh_1                  17
#define ATTR_INDEX_high_temp_1_thresh_2                  18
#define ATTR_INDEX_low_temp_1_thresh_1                   19
#define ATTR_INDEX_low_temp_1_thresh_2                   20
#define ATTR_INDEX_temp_1_delta_thresh                   21
#define ATTR_INDEX_high_temp_2_thresh_1                  22
#define ATTR_INDEX_high_temp_2_thresh_2                  23
#define ATTR_INDEX_low_temp_2_thresh_1                   24
#define ATTR_INDEX_low_temp_2_thresh_2                   25
#define ATTR_INDEX_temp_2_delta_thresh                   26
#define ATTR_INDEX_high_temp_3_thresh_1                  27
#define ATTR_INDEX_high_temp_3_thresh_2                  28
#define ATTR_INDEX_low_temp_3_thresh_1                   29
#define ATTR_INDEX_low_temp_3_thresh_2                   30
#define ATTR_INDEX_temp_3_delta_thresh                   31
#define ATTR_INDEX_high_temp_4_thresh_1                  32
#define ATTR_INDEX_high_temp_4_thresh_2                  33
#define ATTR_INDEX_low_temp_4_thresh_1                   34
#define ATTR_INDEX_low_temp_4_thresh_2                   35
#define ATTR_INDEX_temp_4_delta_thresh                   36
#define ATTR_INDEX_high_analog_1_thresh_1                37
#define ATTR_INDEX_high_analog_1_thresh_2                38
#define ATTR_INDEX_low_analog_1_thresh_1                 39
#define ATTR_INDEX_low_analog_1_thresh_2                 40
#define ATTR_INDEX_analog_1_delta_thresh                 41
#define ATTR_INDEX_high_analog_2_thresh_1                42
#define ATTR_INDEX_high_analog_2_thresh_2                43
#define ATTR_INDEX_low_analog_2_thresh_1                 44
#define ATTR_INDEX_low_analog_2_thresh_2                 45
#define ATTR_INDEX_analog_2_delta_thresh                 46
#define ATTR_INDEX_high_analog_3_thresh_1                47
#define ATTR_INDEX_high_analog_3_thresh_2                48
#define ATTR_INDEX_low_analog_3_thresh_1                 49
#define ATTR_INDEX_low_analog_3_thresh_2                 50
#define ATTR_INDEX_analog_3_delta_thresh                 51
#define ATTR_INDEX_high_analog_4_thresh_1                52
#define ATTR_INDEX_high_analog_4_thresh_2                53
#define ATTR_INDEX_low_analog_4_thresh_1                 54
#define ATTR_INDEX_low_analog_4_thresh_2                 55
#define ATTR_INDEX_analog_4_delta_thresh                 56
#define ATTR_INDEX_active_mode                           57
#define ATTR_INDEX_use_coded_phy                         58
#define ATTR_INDEX_tx_power                              59
#define ATTR_INDEX_network_id                            60
#define ATTR_INDEX_config_version                        61
#define ATTR_INDEX_config_type                           62
#define ATTR_INDEX_hardware_minor_version                63
#define ATTR_INDEX_ge                                    64
#define ATTR_INDEX_oe                                    65
#define ATTR_INDEX_old_coefficient_a                     66
#define ATTR_INDEX_old_coefficient_b                     67
#define ATTR_INDEX_old_coefficient_c                     68
#define ATTR_INDEX_thermistor_config                     69
#define ATTR_INDEX_temperature_result_1                  70
#define ATTR_INDEX_temperature_result_2                  71
#define ATTR_INDEX_temperature_result_3                  72
#define ATTR_INDEX_temperature_result_4                  73
#define ATTR_INDEX_temperature_alarms                    74
#define ATTR_INDEX_power_voltage_mv                      75
#define ATTR_INDEX_digital_input                         76
#define ATTR_INDEX_digital_alarms                        77
#define ATTR_INDEX_digital_input_1_config                78
#define ATTR_INDEX_digital_input_2_config                79
#define ATTR_INDEX_analog_input_1                        80
#define ATTR_INDEX_analog_input_2                        81
#define ATTR_INDEX_analog_input_3                        82
#define ATTR_INDEX_analog_input_4                        83
#define ATTR_INDEX_analog_alarms                         84
#define ATTR_INDEX_analog_input_1_type                   85
#define ATTR_INDEX_analog_input_2_type                   86
#define ATTR_INDEX_analog_input_3_type                   87
#define ATTR_INDEX_analog_input_4_type                   88
#define ATTR_INDEX_flags                                 89
#define ATTR_INDEX_magnet_state                          90
#define ATTR_INDEX_param_path                            91
#define ATTR_INDEX_battery_age                           92
#define ATTR_INDEX_api_version                           93
#define ATTR_INDEX_qrtc                                  94
#define ATTR_INDEX_qrtc_last_set                         95
#define ATTR_INDEX_sh_offset                             96
#define ATTR_INDEX_analog_sense_interval                 97
#define ATTR_INDEX_tamper_switch_status                  98
#define ATTR_INDEX_connection_timeout_sec                99
#define ATTR_INDEX_settings_passcode                     100
#define ATTR_INDEX_therm_1_coefficient_a                 101
#define ATTR_INDEX_therm_2_coefficient_a                 102
#define ATTR_INDEX_therm_3_coefficient_a                 103
#define ATTR_INDEX_therm_4_coefficient_a                 104
#define ATTR_INDEX_therm_1_coefficient_b                 105
#define ATTR_INDEX_therm_2_coefficient_b                 106
#define ATTR_INDEX_therm_3_coefficient_b                 107
#define ATTR_INDEX_therm_4_coefficient_b                 108
#define ATTR_INDEX_therm_1_coefficient_c                 109
#define ATTR_INDEX_therm_2_coefficient_c                 110
#define ATTR_INDEX_therm_3_coefficient_c                 111
#define ATTR_INDEX_therm_4_coefficient_c                 112
#define ATTR_INDEX_data_logging_enable                   113
#define ATTR_INDEX_factory_reset_enable                  114
#define ATTR_INDEX_log_file_status                       115
#define ATTR_INDEX_temperature_alarms_enable             116
#define ATTR_INDEX_analog_alarms_enable                  117
#define ATTR_INDEX_adc_power_simulated                   118
#define ATTR_INDEX_adc_power_simulated_counts            119
#define ATTR_INDEX_adc_analog_sensor_simulated           120
#define ATTR_INDEX_adc_analog_sensor_simulated_counts    121
#define ATTR_INDEX_adc_thermistor_simulated              122
#define ATTR_INDEX_adc_thermistor_simulated_counts       123
#define ATTR_INDEX_adc_vref_simulated                    124
#define ATTR_INDEX_adc_vref_simulated_counts             125
#define ATTR_INDEX_voltage_1_simulated                   126
#define ATTR_INDEX_voltage_1_simulated_value             127
#define ATTR_INDEX_voltage_2_simulated                   128
#define ATTR_INDEX_voltage_2_simulated_value             129
#define ATTR_INDEX_voltage_3_simulated                   130
#define ATTR_INDEX_voltage_3_simulated_value             131
#define ATTR_INDEX_voltage_4_simulated                   132
#define ATTR_INDEX_voltage_4_simulated_value             133
#define ATTR_INDEX_ultrasonic_simulated                  134
#define ATTR_INDEX_ultrasonic_simulated_value            135
#define ATTR_INDEX_pressure_simulated                    136
#define ATTR_INDEX_pressure_simulated_value              137
#define ATTR_INDEX_current_1_simulated                   138
#define ATTR_INDEX_current_1_simulated_value             139
#define ATTR_INDEX_current_2_simulated                   140
#define ATTR_INDEX_current_2_simulated_value             141
#define ATTR_INDEX_current_3_simulated                   142
#define ATTR_INDEX_current_3_simulated_value             143
#define ATTR_INDEX_current_4_simulated                   144
#define ATTR_INDEX_current_4_simulated_value             145
#define ATTR_INDEX_vref_simulated                        146
#define ATTR_INDEX_vref_simulated_value                  147
#define ATTR_INDEX_temperature_1_simulated               148
#define ATTR_INDEX_temperature_1_simulated_value         149
#define ATTR_INDEX_temperature_2_simulated               150
#define ATTR_INDEX_temperature_2_simulated_value         151
#define ATTR_INDEX_temperature_3_simulated               152
#define ATTR_INDEX_temperature_3_simulated_value         153
#define ATTR_INDEX_temperature_4_simulated               154
#define ATTR_INDEX_temperature_4_simulated_value         155
#define ATTR_INDEX_powermv_simulated                     156
#define ATTR_INDEX_powermv_simulated_value               157
#define ATTR_INDEX_digital_input_1_simulated             158
#define ATTR_INDEX_digital_input_1_simulated_value       159
#define ATTR_INDEX_digital_input_2_simulated             160
#define ATTR_INDEX_digital_input_2_simulated_value       161
#define ATTR_INDEX_mag_switch_simulated                  162
#define ATTR_INDEX_mag_switch_simulated_value            163
#define ATTR_INDEX_tamper_switch_simulated               164
#define ATTR_INDEX_tamper_switch_simulated_value         165
#define ATTR_INDEX_boot_phy                              166
#define ATTR_INDEX_mobile_app_disconnect                 167
#define ATTR_INDEX_attr_save_error_code                  168
#define ATTR_INDEX_settings_passcode_status              169
#define ATTR_INDEX_recover_settings_count                170
#define ATTR_INDEX_block_downgrades                      171
#define ATTR_INDEX_security_request                      172
#define ATTR_INDEX_security_level                        173
#define ATTR_INDEX_lock_status                           174
/* pyend */
/* clang-format on */

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @param DoWrite true if attribute should be changed, false if pValue should
 * be validated but not written.
 *
 * @retval Validators return negative error code, 0 on success
 */
int attribute_validator_string(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite);
int attribute_validator_uint64(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite);
int attribute_validator_uint32(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite);
int attribute_validator_uint16(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite);
int attribute_validator_bool(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int attribute_validator_uint8(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_int64(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_int32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_int16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_int8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int attribute_validator_float(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);

int attribute_validator_aic(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite);

int attribute_validator_cp32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int attribute_validator_cp16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int attribute_validator_cp8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite);
int attribute_validator_cpi32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_cpi16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int attribute_validator_cpi8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int attribute_validator_din1simen(AttributeEntry_t *pEntry, void *pValue,
				  size_t Length, bool DoWrite);
int attribute_validator_din1sim(AttributeEntry_t *pEntry, void *pValue,
				size_t Length, bool DoWrite);
int attribute_validator_din2simen(AttributeEntry_t *pEntry, void *pValue,
				  size_t Length, bool DoWrite);
int attribute_validator_din2sim(AttributeEntry_t *pEntry, void *pValue,
				size_t Length, bool DoWrite);
int attribute_validator_magsimen(AttributeEntry_t *pEntry, void *pValue,
				 size_t Length, bool DoWrite);
int attribute_validator_magsim(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite);
int attribute_validator_tampsimen(AttributeEntry_t *pEntry, void *pValue,
				  size_t Length, bool DoWrite);
int attribute_validator_tampsim(AttributeEntry_t *pEntry, void *pValue,
				size_t Length, bool DoWrite);
int attribute_validator_block_downgrades(AttributeEntry_t *pEntry, void *pValue,
					 size_t Length, bool DoWrite);

/* The weak implementations should be overridden application. */
/* pystart - prepare for read */
int AttributePrepare_up_time(void);
int AttributePrepare_temperature_result_1(void);
int AttributePrepare_temperature_result_2(void);
int AttributePrepare_temperature_result_3(void);
int AttributePrepare_temperature_result_4(void);
int AttributePrepare_power_voltage_mv(void);
int AttributePrepare_digital_input(void);
int AttributePrepare_analog_input_1(void);
int AttributePrepare_analog_input_2(void);
int AttributePrepare_analog_input_3(void);
int AttributePrepare_analog_input_4(void);
int AttributePrepare_log_file_status(void);
int AttributePrepare_security_level(void);
/* pyend */

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TABLE_H__ */
