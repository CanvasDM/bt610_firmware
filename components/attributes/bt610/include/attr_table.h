/**
 * @file attr_table.h
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity LLC
 *
 * SPDX-License-Identifier: LicenseRef-LairdConnectivity-Clause
 */

#ifndef __ATTR_TABLE_H__
#define __ATTR_TABLE_H__

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>

#include "attr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/
/* Global Constants, Macros and Type Definitions                                                  */
/**************************************************************************************************/
/* pystart - enumerations */
enum advertising_phy {
	ADVERTISING_PHY_1M = 0,
	ADVERTISING_PHY_CODED = 1,
};

enum config_type {
	CONFIG_TYPE_NOT_CONFIGURED = 0,
	CONFIG_TYPE_ANALOG = 1,
	CONFIG_TYPE_DIGITAL = 2,
	CONFIG_TYPE_TEMPERATURE = 3,
	CONFIG_TYPE_CURRENT = 4,
	CONFIG_TYPE_ULTRASONIC_PRESSURE = 5,
	CONFIG_TYPE_SPI_OR_I2C = 6,
};

enum digital_input {
	DIGITAL_INPUT_PORT1_BITMASK = 1,
	DIGITAL_INPUT_PORT2_BITMASK = 2,
};

enum analog_input_1_type {
	ANALOG_INPUT_1_TYPE_UNUSED = 0,
	ANALOG_INPUT_1_TYPE_VOLTAGE_0V_TO_10V_DC = 1,
	ANALOG_INPUT_1_TYPE_CURRENT_4MA_TO_20MA = 2,
	ANALOG_INPUT_1_TYPE_PRESSURE = 3,
	ANALOG_INPUT_1_TYPE_ULTRASONIC = 4,
	ANALOG_INPUT_1_TYPE_AC_CURRENT_20A = 5,
	ANALOG_INPUT_1_TYPE_AC_CURRENT_150A = 6,
	ANALOG_INPUT_1_TYPE_AC_CURRENT_500A = 7,
};

enum magnet_state {
	MAGNET_STATE_FAR = false,
	MAGNET_STATE_NEAR = true,
};

enum tamper_switch_status {
	TAMPER_SWITCH_STATUS_ACTIVE_NOT_PRESSED = true,
	TAMPER_SWITCH_STATUS_INACTIVE_PRESSED = false,
};

enum boot_phy {
	BOOT_PHY_DEFAULT = 0,
	BOOT_PHY_CODED = 1,
	BOOT_PHY_1M = 2,
};

enum bluetooth_flags {
	BLUETOOTH_FLAGS_DEVICE_MANAGEMENT_DATA_READY_BITMASK = 1,
	BLUETOOTH_FLAGS_MEMFAULT_DATA_BITMASK = 2,
	BLUETOOTH_FLAGS_TIME_WAS_SET_BITMASK = 4,
	BLUETOOTH_FLAGS_ACTIVE_MODE_BITMASK = 8,
	BLUETOOTH_FLAGS_LOW_BATTERY_ALARM_BITMASK = 16,
	BLUETOOTH_FLAGS_DIGITAL_IN1_STATE_BITMASK = 32,
	BLUETOOTH_FLAGS_DIGITAL_IN2_STATE_BITMASK = 64,
	BLUETOOTH_FLAGS_TAMPER_SWITCH_STATE_BITMASK = 128,
	BLUETOOTH_FLAGS_MAGNET_STATE_BITMASK = 256,
	BLUETOOTH_FLAGS_UNUSED_0 = 512,
	BLUETOOTH_FLAGS_UNUSED_1 = 1024,
	BLUETOOTH_FLAGS_UNUSED_2 = 2048,
	BLUETOOTH_FLAGS_UNUSED_3 = 4096,
	BLUETOOTH_FLAGS_UNUSED_4 = 8192,
	BLUETOOTH_FLAGS_UNUSED_5 = 16384,
	BLUETOOTH_FLAGS_UNUSED_6 = 32768,
	BLUETOOTH_FLAGS_UNUSED_7 = 65536,
	BLUETOOTH_FLAGS_UNUSED_8 = 131072,
	BLUETOOTH_FLAGS_UNUSED_9 = 262144,
	BLUETOOTH_FLAGS_UNUSED_10 = 524288,
	BLUETOOTH_FLAGS_UNUSED_11 = 1048576,
	BLUETOOTH_FLAGS_UNUSED_12 = 2097152,
	BLUETOOTH_FLAGS_UNUSED_13 = 4194304,
	BLUETOOTH_FLAGS_UNUSED_14 = 8388608,
	BLUETOOTH_FLAGS_UNUSED_15 = 16777216,
	BLUETOOTH_FLAGS_UNUSED_16 = 33554432,
	BLUETOOTH_FLAGS_UNUSED_17 = 67108864,
	BLUETOOTH_FLAGS_UNUSED_18 = 134217728,
	BLUETOOTH_FLAGS_UNUSED_19 = 268435456,
	BLUETOOTH_FLAGS_UNUSED_20 = 536870912,
	BLUETOOTH_FLAGS_UNUSED_21 = 1073741824,
	BLUETOOTH_FLAGS_UNUSED_22 = 2147483648,
};

enum event_filter_flags {
	EVENT_FILTER_FLAGS_TEMPERATURE_1_EVENT_BITMASK = 1,
	EVENT_FILTER_FLAGS_TEMPERATURE_2_EVENT_BITMASK = 2,
	EVENT_FILTER_FLAGS_TEMPERATURE_3_EVENT_BITMASK = 4,
	EVENT_FILTER_FLAGS_TEMPERATURE_4_EVENT_BITMASK = 8,
	EVENT_FILTER_FLAGS_VOLTAGE_1_EVENT_BITMASK = 16,
	EVENT_FILTER_FLAGS_VOLTAGE_2_EVENT_BITMASK = 32,
	EVENT_FILTER_FLAGS_VOLTAGE_3_EVENT_BITMASK = 64,
	EVENT_FILTER_FLAGS_VOLTAGE_4_EVENT_BITMASK = 128,
	EVENT_FILTER_FLAGS_CURRENT_1_EVENT_BITMASK = 256,
	EVENT_FILTER_FLAGS_CURRENT_2_EVENT_BITMASK = 512,
	EVENT_FILTER_FLAGS_CURRENT_3_EVENT_BITMASK = 1024,
	EVENT_FILTER_FLAGS_CURRENT_4_EVENT_BITMASK = 2048,
	EVENT_FILTER_FLAGS_ULTRASONIC_EVENT_BITMASK = 4096,
	EVENT_FILTER_FLAGS_PRESSURE_1_EVENT_BITMASK = 8192,
	EVENT_FILTER_FLAGS_PRESSURE_2_EVENT_BITMASK = 16384,
	EVENT_FILTER_FLAGS_TAMPER_SWITCH_EVENT_BITMASK = 32768,
	EVENT_FILTER_FLAGS_MAGNET_SENSE_EVENT_BITMASK = 65536,
	EVENT_FILTER_FLAGS_BATTERY_GOOD_EVENT_BITMASK = 131072,
	EVENT_FILTER_FLAGS_BATTERY_BAD_EVENT_BITMASK = 262144,
	EVENT_FILTER_FLAGS_DIGITAL_IN1_EVENT_BITMASK = 524288,
	EVENT_FILTER_FLAGS_DIGITAL_IN2_EVENT_BITMASK = 1048576,
	EVENT_FILTER_FLAGS_UNUSED_0 = 2097152,
	EVENT_FILTER_FLAGS_UNUSED_1 = 4194304,
	EVENT_FILTER_FLAGS_UNUSED_2 = 8388608,
	EVENT_FILTER_FLAGS_UNUSED_3 = 16777216,
	EVENT_FILTER_FLAGS_UNUSED_4 = 33554432,
	EVENT_FILTER_FLAGS_UNUSED_5 = 67108864,
	EVENT_FILTER_FLAGS_UNUSED_6 = 134217728,
	EVENT_FILTER_FLAGS_UNUSED_7 = 268435456,
	EVENT_FILTER_FLAGS_UNUSED_8 = 536870912,
	EVENT_FILTER_FLAGS_UNUSED_9 = 1073741824,
	EVENT_FILTER_FLAGS_UNUSED_10 = 2147483648,
};

enum lwm2m_security {
	LWM2M_SECURITY_PSK = 0,
	LWM2M_SECURITY_RPK = 1,
	LWM2M_SECURITY_CERT = 2,
	LWM2M_SECURITY_NO_SEC = 3,
	LWM2M_SECURITY_CERT_EST = 4,
};

enum lwm2m_pwr_src {
	LWM2M_PWR_SRC_DC = 0,
	LWM2M_PWR_SRC_INT_BATT = 1,
	LWM2M_PWR_SRC_EXT_BATT = 2,
	LWM2M_PWR_SRC_FUEL_CELL = 3,
	LWM2M_PWR_SRC_POE = 4,
	LWM2M_PWR_SRC_USB = 5,
	LWM2M_PWR_SRC_AC = 6,
	LWM2M_PWR_SRC_SOLAR = 7,
};

enum lwm2m_batt_stat {
	LWM2M_BATT_STAT_NORM = 0,
	LWM2M_BATT_STAT_CHARGING = 1,
	LWM2M_BATT_STAT_CHARGE_COMP = 2,
	LWM2M_BATT_STAT_DAMAGED = 3,
	LWM2M_BATT_STAT_LOW = 4,
	LWM2M_BATT_STAT_NOT_INST = 5,
	LWM2M_BATT_STAT_UNKNOWN = 6,
};

/* pyend */

/* pystart - enum size check */
BUILD_ASSERT(sizeof(enum advertising_phy) == ATTR_SIZE_BOOL);
BUILD_ASSERT(sizeof(enum config_type) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum digital_input) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum analog_input_1_type) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum magnet_state) == ATTR_SIZE_BOOL);
BUILD_ASSERT(sizeof(enum tamper_switch_status) == ATTR_SIZE_BOOL);
BUILD_ASSERT(sizeof(enum boot_phy) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum bluetooth_flags) == ATTR_SIZE_U32);
BUILD_ASSERT(sizeof(enum event_filter_flags) == ATTR_SIZE_U32);
BUILD_ASSERT(sizeof(enum lwm2m_security) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum lwm2m_pwr_src) == ATTR_SIZE_U8);
BUILD_ASSERT(sizeof(enum lwm2m_batt_stat) == ATTR_SIZE_U8);
/* pyend */

/**************************************************************************************************/
/* Global Data Definitions                                                                        */
/**************************************************************************************************/
/* pystart - attribute ids */
#define ATTR_ID_reserved0                             0
#define ATTR_ID_sensor_name                           1
#define ATTR_ID_location                              2
#define ATTR_ID_advertising_interval                  3
#define ATTR_ID_advertising_duration                  4
#define ATTR_ID_power_sense_interval                  5
#define ATTR_ID_temperature_sense_interval            6
#define ATTR_ID_digital_output_1_state                7
#define ATTR_ID_digital_output_2_state                8
#define ATTR_ID_firmware_version                      9
#define ATTR_ID_reset_reason                          10
#define ATTR_ID_reset_count                           11
#define ATTR_ID_uptime                                12
#define ATTR_ID_active_mode                           13
#define ATTR_ID_advertising_phy                       14
#define ATTR_ID_tx_power                              15
#define ATTR_ID_network_id                            16
#define ATTR_ID_config_version                        17
#define ATTR_ID_config_type                           18
#define ATTR_ID_ge                                    19
#define ATTR_ID_oe                                    20
#define ATTR_ID_thermistor_config                     21
#define ATTR_ID_temperature_result_1                  22
#define ATTR_ID_temperature_result_2                  23
#define ATTR_ID_temperature_result_3                  24
#define ATTR_ID_temperature_result_4                  25
#define ATTR_ID_power_voltage                         26
#define ATTR_ID_digital_input                         27
#define ATTR_ID_digital_input_1_config                28
#define ATTR_ID_digital_input_2_config                29
#define ATTR_ID_analog_input_1                        30
#define ATTR_ID_analog_input_2                        31
#define ATTR_ID_analog_input_3                        32
#define ATTR_ID_analog_input_4                        33
#define ATTR_ID_analog_input_1_type                   34
#define ATTR_ID_analog_input_2_type                   35
#define ATTR_ID_analog_input_3_type                   36
#define ATTR_ID_analog_input_4_type                   37
#define ATTR_ID_magnet_state                          38
#define ATTR_ID_param_path                            39
#define ATTR_ID_battery_age                           40
#define ATTR_ID_api_version                           41
#define ATTR_ID_qrtc                                  42
#define ATTR_ID_qrtc_last_set                         43
#define ATTR_ID_sh_offset                             44
#define ATTR_ID_analog_sense_interval                 45
#define ATTR_ID_tamper_switch_status                  46
#define ATTR_ID_therm_1_coefficient_a                 47
#define ATTR_ID_therm_2_coefficient_a                 48
#define ATTR_ID_therm_3_coefficient_a                 49
#define ATTR_ID_therm_4_coefficient_a                 50
#define ATTR_ID_therm_1_coefficient_b                 51
#define ATTR_ID_therm_2_coefficient_b                 52
#define ATTR_ID_therm_3_coefficient_b                 53
#define ATTR_ID_therm_4_coefficient_b                 54
#define ATTR_ID_therm_1_coefficient_c                 55
#define ATTR_ID_therm_2_coefficient_c                 56
#define ATTR_ID_therm_3_coefficient_c                 57
#define ATTR_ID_therm_4_coefficient_c                 58
#define ATTR_ID_factory_reset_enable                  59
#define ATTR_ID_adc_power_simulated                   60
#define ATTR_ID_adc_power_simulated_counts            61
#define ATTR_ID_adc_analog_sensor_simulated           62
#define ATTR_ID_adc_analog_sensor_simulated_counts    63
#define ATTR_ID_adc_thermistor_simulated              64
#define ATTR_ID_adc_thermistor_simulated_counts       65
#define ATTR_ID_adc_vref_simulated                    66
#define ATTR_ID_adc_vref_simulated_counts             67
#define ATTR_ID_voltage_1_simulated                   68
#define ATTR_ID_voltage_1_simulated_value             69
#define ATTR_ID_voltage_2_simulated                   70
#define ATTR_ID_voltage_2_simulated_value             71
#define ATTR_ID_voltage_3_simulated                   72
#define ATTR_ID_voltage_3_simulated_value             73
#define ATTR_ID_voltage_4_simulated                   74
#define ATTR_ID_voltage_4_simulated_value             75
#define ATTR_ID_ultrasonic_simulated                  76
#define ATTR_ID_ultrasonic_simulated_value            77
#define ATTR_ID_pressure_simulated                    78
#define ATTR_ID_pressure_simulated_value              79
#define ATTR_ID_current_1_simulated                   80
#define ATTR_ID_current_1_simulated_value             81
#define ATTR_ID_current_2_simulated                   82
#define ATTR_ID_current_2_simulated_value             83
#define ATTR_ID_current_3_simulated                   84
#define ATTR_ID_current_3_simulated_value             85
#define ATTR_ID_current_4_simulated                   86
#define ATTR_ID_current_4_simulated_value             87
#define ATTR_ID_vref_simulated                        88
#define ATTR_ID_vref_simulated_value                  89
#define ATTR_ID_temperature_1_simulated               90
#define ATTR_ID_temperature_1_simulated_value         91
#define ATTR_ID_temperature_2_simulated               92
#define ATTR_ID_temperature_2_simulated_value         93
#define ATTR_ID_temperature_3_simulated               94
#define ATTR_ID_temperature_3_simulated_value         95
#define ATTR_ID_temperature_4_simulated               96
#define ATTR_ID_temperature_4_simulated_value         97
#define ATTR_ID_power_volts_simulated                 98
#define ATTR_ID_power_volts_simulated_value           99
#define ATTR_ID_digital_input_1_simulated             100
#define ATTR_ID_digital_input_1_simulated_value       101
#define ATTR_ID_digital_input_2_simulated             102
#define ATTR_ID_digital_input_2_simulated_value       103
#define ATTR_ID_mag_switch_simulated                  104
#define ATTR_ID_mag_switch_simulated_value            105
#define ATTR_ID_tamper_switch_simulated               106
#define ATTR_ID_tamper_switch_simulated_value         107
#define ATTR_ID_boot_phy                              108
#define ATTR_ID_mobile_app_disconnect                 109
#define ATTR_ID_attr_save_error_code                  110
#define ATTR_ID_block_downgrades                      111
#define ATTR_ID_security_level                        112
#define ATTR_ID_load_path                             113
#define ATTR_ID_dump_path                             114
#define ATTR_ID_bluetooth_flags                       115
#define ATTR_ID_event_filter_flags                    116
#define ATTR_ID_board                                 117
#define ATTR_ID_log_on_boot                           118
#define ATTR_ID_input_config_changed                  119
#define ATTR_ID_disable_flow_control                  120
#define ATTR_ID_baud_rate                             121
#define ATTR_ID_tel_trust_path                        122
#define ATTR_ID_tel_key_path                          123
#define ATTR_ID_dm_trust_path                         124
#define ATTR_ID_dm_key_path                           125
#define ATTR_ID_fs_trust_path                         126
#define ATTR_ID_fs_key_path                           127
#define ATTR_ID_p2p_trust_path                        128
#define ATTR_ID_p2p_key_path                          129
#define ATTR_ID_lwm2m_server_url                      130
#define ATTR_ID_lwm2m_endpoint                        131
#define ATTR_ID_lwm2m_security                        132
#define ATTR_ID_lwm2m_psk_id                          133
#define ATTR_ID_lwm2m_psk                             134
#define ATTR_ID_lwm2m_bootstrap                       135
#define ATTR_ID_lwm2m_short_id                        136
#define ATTR_ID_lwm2m_mfg                             137
#define ATTR_ID_lwm2m_mn                              138
#define ATTR_ID_lwm2m_sn                              139
#define ATTR_ID_lwm2m_fw_ver                          140
#define ATTR_ID_lwm2m_pwr_src                         141
#define ATTR_ID_lwm2m_pwr_src_volt                    142
#define ATTR_ID_lwm2m_sw_ver                          143
#define ATTR_ID_lwm2m_hw_ver                          144
#define ATTR_ID_lwm2m_batt_stat                       145
#define ATTR_ID_lwm2m_fup_pkg_name                    146
#define ATTR_ID_lwm2m_fup_pkg_ver                     147
#define ATTR_ID_lwm2m_fup_proxy_srv                   148
#define ATTR_ID_bluetooth_address                     149
#define ATTR_ID_dm_cnx_delay                          150
#define ATTR_ID_factory_load_path                     151
#define ATTR_ID_device_id                             152
#define ATTR_ID_ble_rssi                              153
#define ATTR_ID_smp_auth_req                          154
#define ATTR_ID_smp_auth_timeout                      155
#define ATTR_ID_shell_password                        156
#define ATTR_ID_shell_session_timeout                 157
/* pyend */

/* pystart - attribute constants */
#define ATTR_TABLE_SIZE                                             158
#define ATTR_TABLE_MAX_ID                                           157
#define ATTR_TABLE_WRITABLE_COUNT                                   122
#define ATTR_TABLE_CRC_OF_NAMES                                     0x4c735cc2
#define ATTR_MAX_STR_LENGTH                                         255
#define ATTR_MAX_STR_SIZE                                           256
#define ATTR_MAX_BIN_SIZE                                           16
#define ATTR_MAX_INT_SIZE                                           8
#define ATTR_MAX_KEY_NAME_SIZE                                      35
#define ATTR_MAX_VALUE_SIZE                                         256
#define ATTR_MAX_FILE_SIZE                                          5593
#define ATTR_ENABLE_FPU_CHECK                                       1

/* Attribute Max String Lengths */
#define ATTR_SENSOR_NAME_MAX_STR_SIZE                               24
#define ATTR_LOCATION_MAX_STR_SIZE                                  33
#define ATTR_FIRMWARE_VERSION_MAX_STR_SIZE                          65
#define ATTR_RESET_REASON_MAX_STR_SIZE                              13
#define ATTR_PARAM_PATH_MAX_STR_SIZE                                9
#define ATTR_API_VERSION_MAX_STR_SIZE                               12
#define ATTR_LOAD_PATH_MAX_STR_SIZE                                 33
#define ATTR_DUMP_PATH_MAX_STR_SIZE                                 33
#define ATTR_BOARD_MAX_STR_SIZE                                     65
#define ATTR_TEL_TRUST_PATH_MAX_STR_SIZE                            33
#define ATTR_TEL_KEY_PATH_MAX_STR_SIZE                              33
#define ATTR_DM_TRUST_PATH_MAX_STR_SIZE                             33
#define ATTR_DM_KEY_PATH_MAX_STR_SIZE                               33
#define ATTR_FS_TRUST_PATH_MAX_STR_SIZE                             33
#define ATTR_FS_KEY_PATH_MAX_STR_SIZE                               33
#define ATTR_P2P_TRUST_PATH_MAX_STR_SIZE                            33
#define ATTR_P2P_KEY_PATH_MAX_STR_SIZE                              33
#define ATTR_LWM2M_SERVER_URL_MAX_STR_SIZE                          256
#define ATTR_LWM2M_ENDPOINT_MAX_STR_SIZE                            65
#define ATTR_LWM2M_PSK_ID_MAX_STR_SIZE                              65
#define ATTR_LWM2M_MFG_MAX_STR_SIZE                                 33
#define ATTR_LWM2M_MN_MAX_STR_SIZE                                  33
#define ATTR_LWM2M_SN_MAX_STR_SIZE                                  65
#define ATTR_LWM2M_FW_VER_MAX_STR_SIZE                              33
#define ATTR_LWM2M_SW_VER_MAX_STR_SIZE                              33
#define ATTR_LWM2M_HW_VER_MAX_STR_SIZE                              33
#define ATTR_LWM2M_FUP_PKG_NAME_MAX_STR_SIZE                        33
#define ATTR_LWM2M_FUP_PKG_VER_MAX_STR_SIZE                         33
#define ATTR_LWM2M_FUP_PROXY_SRV_MAX_STR_SIZE                       256
#define ATTR_BLUETOOTH_ADDRESS_MAX_STR_SIZE                         13
#define ATTR_FACTORY_LOAD_PATH_MAX_STR_SIZE                         33
#define ATTR_DEVICE_ID_MAX_STR_SIZE                                 65
#define ATTR_SHELL_PASSWORD_MAX_STR_SIZE                            33

/* Attribute Byte Array Lengths */
#define ATTR_LWM2M_PSK_SIZE                                         16
/* pyend */

/**************************************************************************************************/
/* Global Function Prototypes                                                                     */
/**************************************************************************************************/
/* pystart - prepare for read */
int attr_prepare_uptime(void);
int attr_prepare_temperature_result_1(void);
int attr_prepare_temperature_result_2(void);
int attr_prepare_temperature_result_3(void);
int attr_prepare_temperature_result_4(void);
int attr_prepare_power_voltage(void);
int attr_prepare_digital_input(void);
int attr_prepare_analog_input_1(void);
int attr_prepare_analog_input_2(void);
int attr_prepare_analog_input_3(void);
int attr_prepare_analog_input_4(void);
int attr_prepare_security_level(void);
/* pyend */

/* pystart - get string */
const char *const attr_get_string_advertising_phy(int value);
const char *const attr_get_string_config_type(int value);
const char *const attr_get_string_digital_input(int value);
const char *const attr_get_string_analog_input_1_type(int value);
const char *const attr_get_string_magnet_state(int value);
const char *const attr_get_string_tamper_switch_status(int value);
const char *const attr_get_string_boot_phy(int value);
const char *const attr_get_string_bluetooth_flags(int value);
const char *const attr_get_string_event_filter_flags(int value);
const char *const attr_get_string_lwm2m_security(int value);
const char *const attr_get_string_lwm2m_pwr_src(int value);
const char *const attr_get_string_lwm2m_batt_stat(int value);
/* pyend */

#ifdef __cplusplus
}
#endif
#endif /* __ATTR_TABLE_H__ */
