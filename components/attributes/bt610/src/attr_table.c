/**
 * @file attr_table.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity LLC
 *
 * SPDX-License-Identifier: LicenseRef-LairdConnectivity-Clause
 */

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <string.h>
#include <errno_str.h>
#include "attr_validator.h"
#include "attr_table.h"

#ifdef CONFIG_ATTR_CUSTOM_VALIDATOR
#include "attr_custom_validator.h"
#endif

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define DRW DEFAULT_RW_ATTRIBUTE_VALUES
#define DRO DEFAULT_RO_ATTRIBUTE_VALUES
#define y true
#define n false

/* pystart - rw attributes */
typedef struct rw_attribute {
	char sensor_name[23 + 1];
	char location[32 + 1];
	uint16_t advertising_interval;
	uint16_t advertising_duration;
	uint32_t power_sense_interval;
	uint32_t temperature_sense_interval;
	bool digital_output_1_state;
	bool digital_output_2_state;
	bool active_mode;
	enum advertising_phy advertising_phy;
	int8_t tx_power;
	uint16_t network_id;
	enum config_type config_type;
	uint8_t thermistor_config;
	uint8_t digital_input_1_config;
	uint8_t digital_input_2_config;
	enum analog_input_1_type analog_input_1_type;
	uint8_t analog_input_2_type;
	uint8_t analog_input_3_type;
	uint8_t analog_input_4_type;
	uint32_t qrtc_last_set;
	float sh_offset;
	uint32_t analog_sense_interval;
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
	bool factory_reset_enable;
	enum boot_phy boot_phy;
	bool block_downgrades;
	char load_path[32 + 1];
	enum event_filter_flags event_filter_flags;
	bool log_on_boot;
	bool disable_flow_control;
	uint32_t baud_rate;
	char tel_trust_path[32 + 1];
	char tel_key_path[32 + 1];
	char dm_trust_path[32 + 1];
	char dm_key_path[32 + 1];
	char fs_trust_path[32 + 1];
	char fs_key_path[32 + 1];
	char p2p_trust_path[32 + 1];
	char p2p_key_path[32 + 1];
	char lwm2m_server_url[255 + 1];
	char lwm2m_endpoint[64 + 1];
	enum lwm2m_security lwm2m_security;
	char lwm2m_psk_id[64 + 1];
	uint8_t lwm2m_psk[16];
	bool lwm2m_bootstrap;
	uint16_t lwm2m_short_id;
	char lwm2m_mfg[32 + 1];
	char lwm2m_mn[32 + 1];
	char lwm2m_sn[64 + 1];
	char lwm2m_fw_ver[32 + 1];
	char lwm2m_sw_ver[32 + 1];
	char lwm2m_hw_ver[32 + 1];
	char lwm2m_fup_proxy_srv[255 + 1];
	uint16_t dm_cnx_delay;
	char factory_load_path[32 + 1];
	char device_id[64 + 1];
	bool smp_auth_req;
	uint32_t smp_auth_timeout;
	char shell_password[32 + 1];
	uint8_t shell_session_timeout;
} rw_attribute_t;
/* pyend */

/* pystart - rw defaults */
static const rw_attribute_t DEFAULT_RW_ATTRIBUTE_VALUES =  {
	.sensor_name = "",
	.location = "",
	.advertising_interval = 1000,
	.advertising_duration = 15000,
	.power_sense_interval = 0,
	.temperature_sense_interval = 60,
	.digital_output_1_state = 0,
	.digital_output_2_state = 0,
	.active_mode = 0,
	.advertising_phy = 1,
	.tx_power = 0,
	.network_id = 0,
	.config_type = 0,
	.thermistor_config = 0,
	.digital_input_1_config = 0,
	.digital_input_2_config = 0,
	.analog_input_1_type = 0,
	.analog_input_2_type = 0,
	.analog_input_3_type = 0,
	.analog_input_4_type = 0,
	.qrtc_last_set = 0,
	.sh_offset = 2.7315e+2,
	.analog_sense_interval = 60,
	.therm_1_coefficient_a = 1.132e-3,
	.therm_2_coefficient_a = 1.132e-3,
	.therm_3_coefficient_a = 1.132e-3,
	.therm_4_coefficient_a = 1.132e-3,
	.therm_1_coefficient_b = 2.338e-4,
	.therm_2_coefficient_b = 2.338e-4,
	.therm_3_coefficient_b = 2.338e-4,
	.therm_4_coefficient_b = 2.338e-4,
	.therm_1_coefficient_c = 8.78e-08,
	.therm_2_coefficient_c = 8.78e-08,
	.therm_3_coefficient_c = 8.78e-08,
	.therm_4_coefficient_c = 8.78e-08,
	.factory_reset_enable = 1,
	.boot_phy = 0,
	.block_downgrades = 0,
	.load_path = "/lfs1/enc/attr_load.txt",
	.event_filter_flags = 2097151,
	.log_on_boot = 0,
	.disable_flow_control = 0,
	.baud_rate = 115200,
	.tel_trust_path = "/lfs1/tel/trust",
	.tel_key_path = "/lfs1/enc/tel/key",
	.dm_trust_path = "/lfs1/dm/trust",
	.dm_key_path = "/lfs1/enc/dm/key",
	.fs_trust_path = "/lfs1/fs/trust",
	.fs_key_path = "/lfs1/enc/fs/key",
	.p2p_trust_path = "/lfs1/p2p/trust",
	.p2p_key_path = "/lfs1/enc/p2p/key",
	.lwm2m_server_url = "coap://leshan.eclipseprojects.io:5683",
	.lwm2m_endpoint = "my_device",
	.lwm2m_security = 3,
	.lwm2m_psk_id = "my_device",
	.lwm2m_psk = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
	.lwm2m_bootstrap = 0,
	.lwm2m_short_id = 1,
	.lwm2m_mfg = "Laird Connectivity",
	.lwm2m_mn = "model",
	.lwm2m_sn = "serial_number",
	.lwm2m_fw_ver = "0.0.0",
	.lwm2m_sw_ver = "0.0.0",
	.lwm2m_hw_ver = "0.0.0",
	.lwm2m_fup_proxy_srv = "coaps://coap-proxy.salticidae.net:5684",
	.dm_cnx_delay = 0,
	.factory_load_path = "/lfs1/enc/factory_load.txt",
	.device_id = "",
	.smp_auth_req = 0,
	.smp_auth_timeout = 300,
	.shell_password = "zephyr",
	.shell_session_timeout = 1
};
/* pyend */

/* pystart - ro attributes */
typedef struct ro_attribute {
	uint16_t reserved0;
	char firmware_version[64 + 1];
	char reset_reason[12 + 1];
	uint32_t reset_count;
	int64_t uptime;
	uint8_t config_version;
	float ge;
	float oe;
	float temperature_result_1;
	float temperature_result_2;
	float temperature_result_3;
	float temperature_result_4;
	float power_voltage;
	enum digital_input digital_input;
	float analog_input_1;
	float analog_input_2;
	float analog_input_3;
	float analog_input_4;
	enum magnet_state magnet_state;
	char param_path[8 + 1];
	uint32_t battery_age;
	char api_version[11 + 1];
	uint32_t qrtc;
	enum tamper_switch_status tamper_switch_status;
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
	bool power_volts_simulated;
	float power_volts_simulated_value;
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
	int8_t security_level;
	char dump_path[32 + 1];
	enum bluetooth_flags bluetooth_flags;
	char board[64 + 1];
	bool input_config_changed;
	enum lwm2m_pwr_src lwm2m_pwr_src;
	int32_t lwm2m_pwr_src_volt;
	enum lwm2m_batt_stat lwm2m_batt_stat;
	char lwm2m_fup_pkg_name[32 + 1];
	char lwm2m_fup_pkg_ver[32 + 1];
	char bluetooth_address[12 + 1];
	int16_t ble_rssi;
} ro_attribute_t;
/* pyend */

/* pystart - ro defaults */
static const ro_attribute_t DEFAULT_RO_ATTRIBUTE_VALUES =  {
	.reserved0 = 0,
	.firmware_version = "0.0.0+0",
	.reset_reason = "RESETPIN",
	.reset_count = 0,
	.uptime = 0,
	.config_version = 0,
	.ge = 1e+0,
	.oe = 0.0,
	.temperature_result_1 = 0.0,
	.temperature_result_2 = 0.0,
	.temperature_result_3 = 0.0,
	.temperature_result_4 = 0.0,
	.power_voltage = 0,
	.digital_input = 0,
	.analog_input_1 = 0.0,
	.analog_input_2 = 0.0,
	.analog_input_3 = 0.0,
	.analog_input_4 = 0.0,
	.magnet_state = 0,
	.param_path = "/ext",
	.battery_age = 0,
	.api_version = "0.0.2",
	.qrtc = 0,
	.tamper_switch_status = 0,
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
	.power_volts_simulated = 0,
	.power_volts_simulated_value = 0.0,
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
	.security_level = 0,
	.dump_path = "/lfs1/dump.txt",
	.bluetooth_flags = 0,
	.board = "my_board",
	.input_config_changed = 1,
	.lwm2m_pwr_src = 0,
	.lwm2m_pwr_src_volt = 0,
	.lwm2m_batt_stat = 6,
	.lwm2m_fup_pkg_name = "my_firmware",
	.lwm2m_fup_pkg_ver = "0.0.0",
	.bluetooth_address = "0",
	.ble_rssi = -128,
};
/* pyend */

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static rw_attribute_t rw;
static ro_attribute_t ro;

/**************************************************************************************************/
/* Global Data Definitions                                                                        */
/**************************************************************************************************/
/**
 * @brief Table shorthand
 *
 * @ref CreateStruct (Python script)
 * Writable but non-savable values are populated using RO macro.
 *
 *.........name...value...default....size...writable..readable..get enum str
 */
#define RW_ATTRS(n) STRINGIFY(n), rw.n, DRW.n, sizeof(rw.n), NULL
#define RW_ATTRX(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n), NULL
#define RW_ATTRE(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n), attr_get_string_ ## n
#define RO_ATTRS(n) STRINGIFY(n), ro.n, DRO.n, sizeof(ro.n), NULL
#define RO_ATTRX(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n), NULL
#define RO_ATTRE(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n), attr_get_string_ ## n

/* If min == max then range isn't checked.
 *
 * index....name.....................type.flags.validator..prepare..min.max.
 */
/* pystart - table */
const struct attr_table_entry ATTR_TABLE[ATTR_TABLE_SIZE] = {
	[0  ] = { RO_ATTRX(reserved0)                           , ATTR_TYPE_U16           , 0xa   , av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[1  ] = { RW_ATTRS(sensor_name)                         , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 0         , .max.ux = 23        },
	[2  ] = { RW_ATTRS(location)                            , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 0         , .max.ux = 32        },
	[3  ] = { RW_ATTRX(advertising_interval)                , ATTR_TYPE_U16           , 0x1b  , av_uint16           , NULL                                , .min.ux = 500       , .max.ux = 10000     },
	[4  ] = { RW_ATTRX(advertising_duration)                , ATTR_TYPE_U16           , 0x1b  , av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535     },
	[5  ] = { RW_ATTRX(power_sense_interval)                , ATTR_TYPE_U32           , 0x1b  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 86400     },
	[6  ] = { RW_ATTRX(temperature_sense_interval)          , ATTR_TYPE_U32           , 0x1b  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 86400     },
	[7  ] = { RW_ATTRX(digital_output_1_state)              , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[8  ] = { RW_ATTRX(digital_output_2_state)              , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[9  ] = { RO_ATTRS(firmware_version)                    , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 5         , .max.ux = 64        },
	[10 ] = { RO_ATTRS(reset_reason)                        , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 0         , .max.ux = 12        },
	[11 ] = { RO_ATTRX(reset_count)                         , ATTR_TYPE_U32           , 0x2   , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[12 ] = { RO_ATTRX(uptime)                              , ATTR_TYPE_S64           , 0x2   , av_int64            , attr_prepare_uptime                 , .min.sx = 0         , .max.sx = 0         },
	[13 ] = { RW_ATTRX(active_mode)                         , ATTR_TYPE_BOOL          , 0x1b  , av_cp8              , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[14 ] = { RW_ATTRE(advertising_phy)                     , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[15 ] = { RW_ATTRX(tx_power)                            , ATTR_TYPE_S8            , 0x1b  , av_tx_power         , NULL                                , .min.sx = -40       , .max.sx = 8         },
	[16 ] = { RW_ATTRX(network_id)                          , ATTR_TYPE_U16           , 0x1b  , av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 65535     },
	[17 ] = { RO_ATTRX(config_version)                      , ATTR_TYPE_U8            , 0xa   , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255       },
	[18 ] = { RW_ATTRE(config_type)                         , ATTR_TYPE_U8            , 0x1b  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 6         },
	[19 ] = { RO_ATTRX(ge)                                  , ATTR_TYPE_FLOAT         , 0x2   , av_float            , NULL                                , .min.fx = -5.0      , .max.fx = 3.4e+38   },
	[20 ] = { RO_ATTRX(oe)                                  , ATTR_TYPE_FLOAT         , 0x2   , av_float            , NULL                                , .min.fx = -16.0     , .max.fx = 3.4e+38   },
	[21 ] = { RW_ATTRX(thermistor_config)                   , ATTR_TYPE_U8            , 0x1b  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 15        },
	[22 ] = { RO_ATTRX(temperature_result_1)                , ATTR_TYPE_FLOAT         , 0x2002, av_float            , attr_prepare_temperature_result_1   , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[23 ] = { RO_ATTRX(temperature_result_2)                , ATTR_TYPE_FLOAT         , 0x2002, av_float            , attr_prepare_temperature_result_2   , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[24 ] = { RO_ATTRX(temperature_result_3)                , ATTR_TYPE_FLOAT         , 0x2002, av_float            , attr_prepare_temperature_result_3   , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[25 ] = { RO_ATTRX(temperature_result_4)                , ATTR_TYPE_FLOAT         , 0x2002, av_float            , attr_prepare_temperature_result_4   , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[26 ] = { RO_ATTRX(power_voltage)                       , ATTR_TYPE_FLOAT         , 0x2   , av_float            , attr_prepare_power_voltage          , .min.fx = 0.0       , .max.fx = 4.0       },
	[27 ] = { RO_ATTRE(digital_input)                       , ATTR_TYPE_U8            , 0x2   , av_uint8            , attr_prepare_digital_input          , .min.ux = 0         , .max.ux = 3         },
	[28 ] = { RW_ATTRX(digital_input_1_config)              , ATTR_TYPE_U8            , 0x1b  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 131       },
	[29 ] = { RW_ATTRX(digital_input_2_config)              , ATTR_TYPE_U8            , 0x1b  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 131       },
	[30 ] = { RO_ATTRX(analog_input_1)                      , ATTR_TYPE_FLOAT         , 0x2   , av_float            , attr_prepare_analog_input_1         , .min.fx = 0.0       , .max.fx = 15000.0   },
	[31 ] = { RO_ATTRX(analog_input_2)                      , ATTR_TYPE_FLOAT         , 0x2   , av_float            , attr_prepare_analog_input_2         , .min.fx = 0.0       , .max.fx = 15000.0   },
	[32 ] = { RO_ATTRX(analog_input_3)                      , ATTR_TYPE_FLOAT         , 0x2   , av_float            , attr_prepare_analog_input_3         , .min.fx = 0.0       , .max.fx = 15000.0   },
	[33 ] = { RO_ATTRX(analog_input_4)                      , ATTR_TYPE_FLOAT         , 0x2   , av_float            , attr_prepare_analog_input_4         , .min.fx = 0.0       , .max.fx = 15000.0   },
	[34 ] = { RW_ATTRE(analog_input_1_type)                 , ATTR_TYPE_U8            , 0x1b  , av_aic              , NULL                                , .min.ux = 0         , .max.ux = 7         },
	[35 ] = { RW_ATTRX(analog_input_2_type)                 , ATTR_TYPE_U8            , 0x1b  , av_aic              , NULL                                , .min.ux = 0         , .max.ux = 7         },
	[36 ] = { RW_ATTRX(analog_input_3_type)                 , ATTR_TYPE_U8            , 0x1b  , av_aic              , NULL                                , .min.ux = 0         , .max.ux = 7         },
	[37 ] = { RW_ATTRX(analog_input_4_type)                 , ATTR_TYPE_U8            , 0x1b  , av_aic              , NULL                                , .min.ux = 0         , .max.ux = 7         },
	[38 ] = { RO_ATTRE(magnet_state)                        , ATTR_TYPE_BOOL          , 0x2   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[39 ] = { RO_ATTRS(param_path)                          , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 2         , .max.ux = 8         },
	[40 ] = { RO_ATTRX(battery_age)                         , ATTR_TYPE_U32           , 0x2   , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[41 ] = { RO_ATTRS(api_version)                         , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 6         , .max.ux = 11        },
	[42 ] = { RO_ATTRX(qrtc)                                , ATTR_TYPE_U32           , 0x2   , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[43 ] = { RW_ATTRX(qrtc_last_set)                       , ATTR_TYPE_U32           , 0x1a  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[44 ] = { RW_ATTRX(sh_offset)                           , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[45 ] = { RW_ATTRX(analog_sense_interval)               , ATTR_TYPE_U32           , 0x1b  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 86400     },
	[46 ] = { RO_ATTRE(tamper_switch_status)                , ATTR_TYPE_BOOL          , 0xa   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[47 ] = { RW_ATTRX(therm_1_coefficient_a)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[48 ] = { RW_ATTRX(therm_2_coefficient_a)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[49 ] = { RW_ATTRX(therm_3_coefficient_a)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[50 ] = { RW_ATTRX(therm_4_coefficient_a)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[51 ] = { RW_ATTRX(therm_1_coefficient_b)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[52 ] = { RW_ATTRX(therm_2_coefficient_b)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[53 ] = { RW_ATTRX(therm_3_coefficient_b)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[54 ] = { RW_ATTRX(therm_4_coefficient_b)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[55 ] = { RW_ATTRX(therm_1_coefficient_c)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[56 ] = { RW_ATTRX(therm_2_coefficient_c)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[57 ] = { RW_ATTRX(therm_3_coefficient_c)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[58 ] = { RW_ATTRX(therm_4_coefficient_c)               , ATTR_TYPE_FLOAT         , 0x13  , av_float            , NULL                                , .min.fx = 1.2e-38   , .max.fx = 3.4e+38   },
	[59 ] = { RW_ATTRX(factory_reset_enable)                , ATTR_TYPE_BOOL          , 0x13  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[60 ] = { RO_ATTRX(adc_power_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[61 ] = { RO_ATTRX(adc_power_simulated_counts)          , ATTR_TYPE_S16           , 0x3   , av_int16            , NULL                                , .min.sx = 0         , .max.sx = 4095      },
	[62 ] = { RO_ATTRX(adc_analog_sensor_simulated)         , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[63 ] = { RO_ATTRX(adc_analog_sensor_simulated_counts)  , ATTR_TYPE_S16           , 0x3   , av_int16            , NULL                                , .min.sx = 0         , .max.sx = 4095      },
	[64 ] = { RO_ATTRX(adc_thermistor_simulated)            , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[65 ] = { RO_ATTRX(adc_thermistor_simulated_counts)     , ATTR_TYPE_S16           , 0x3   , av_int16            , NULL                                , .min.sx = 0         , .max.sx = 4095      },
	[66 ] = { RO_ATTRX(adc_vref_simulated)                  , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[67 ] = { RO_ATTRX(adc_vref_simulated_counts)           , ATTR_TYPE_S16           , 0x3   , av_int16            , NULL                                , .min.sx = 0         , .max.sx = 4095      },
	[68 ] = { RO_ATTRX(voltage_1_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[69 ] = { RO_ATTRX(voltage_1_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[70 ] = { RO_ATTRX(voltage_2_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[71 ] = { RO_ATTRX(voltage_2_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[72 ] = { RO_ATTRX(voltage_3_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[73 ] = { RO_ATTRX(voltage_3_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[74 ] = { RO_ATTRX(voltage_4_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[75 ] = { RO_ATTRX(voltage_4_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[76 ] = { RO_ATTRX(ultrasonic_simulated)                , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[77 ] = { RO_ATTRX(ultrasonic_simulated_value)          , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[78 ] = { RO_ATTRX(pressure_simulated)                  , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[79 ] = { RO_ATTRX(pressure_simulated_value)            , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[80 ] = { RO_ATTRX(current_1_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[81 ] = { RO_ATTRX(current_1_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[82 ] = { RO_ATTRX(current_2_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[83 ] = { RO_ATTRX(current_2_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[84 ] = { RO_ATTRX(current_3_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[85 ] = { RO_ATTRX(current_3_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[86 ] = { RO_ATTRX(current_4_simulated)                 , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[87 ] = { RO_ATTRX(current_4_simulated_value)           , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[88 ] = { RO_ATTRX(vref_simulated)                      , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[89 ] = { RO_ATTRX(vref_simulated_value)                , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[90 ] = { RO_ATTRX(temperature_1_simulated)             , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[91 ] = { RO_ATTRX(temperature_1_simulated_value)       , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[92 ] = { RO_ATTRX(temperature_2_simulated)             , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[93 ] = { RO_ATTRX(temperature_2_simulated_value)       , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[94 ] = { RO_ATTRX(temperature_3_simulated)             , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[95 ] = { RO_ATTRX(temperature_3_simulated_value)       , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[96 ] = { RO_ATTRX(temperature_4_simulated)             , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[97 ] = { RO_ATTRX(temperature_4_simulated_value)       , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = -3.4e+38  , .max.fx = 3.4e+38   },
	[98 ] = { RO_ATTRX(power_volts_simulated)               , ATTR_TYPE_BOOL          , 0x3   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[99 ] = { RO_ATTRX(power_volts_simulated_value)         , ATTR_TYPE_FLOAT         , 0x3   , av_float            , NULL                                , .min.fx = 0.0       , .max.fx = 4.0       },
	[100] = { RO_ATTRX(digital_input_1_simulated)           , ATTR_TYPE_BOOL          , 0x3   , av_din1simen        , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[101] = { RO_ATTRX(digital_input_1_simulated_value)     , ATTR_TYPE_BOOL          , 0x3   , av_din1sim          , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[102] = { RO_ATTRX(digital_input_2_simulated)           , ATTR_TYPE_BOOL          , 0x3   , av_din2simen        , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[103] = { RO_ATTRX(digital_input_2_simulated_value)     , ATTR_TYPE_BOOL          , 0x3   , av_din2sim          , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[104] = { RO_ATTRX(mag_switch_simulated)                , ATTR_TYPE_BOOL          , 0x3   , av_magsimen         , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[105] = { RO_ATTRX(mag_switch_simulated_value)          , ATTR_TYPE_BOOL          , 0x3   , av_magsim           , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[106] = { RO_ATTRX(tamper_switch_simulated)             , ATTR_TYPE_BOOL          , 0x3   , av_tampsimen        , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[107] = { RO_ATTRX(tamper_switch_simulated_value)       , ATTR_TYPE_BOOL          , 0x3   , av_tampsim          , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[108] = { RW_ATTRE(boot_phy)                            , ATTR_TYPE_U8            , 0x13  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 2         },
	[109] = { RO_ATTRX(mobile_app_disconnect)               , ATTR_TYPE_BOOL          , 0xb   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[110] = { RO_ATTRX(attr_save_error_code)                , ATTR_TYPE_S32           , 0xa   , av_int32            , NULL                                , .min.sx = 0         , .max.sx = 0         },
	[111] = { RW_ATTRX(block_downgrades)                    , ATTR_TYPE_BOOL          , 0x13  , av_block_downgrades , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[112] = { RO_ATTRX(security_level)                      , ATTR_TYPE_S8            , 0x2   , av_int8             , attr_prepare_security_level         , .min.sx = -1        , .max.sx = 4         },
	[113] = { RW_ATTRS(load_path)                           , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 0         , .max.ux = 32        },
	[114] = { RO_ATTRS(dump_path)                           , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 0         , .max.ux = 32        },
	[115] = { RO_ATTRE(bluetooth_flags)                     , ATTR_TYPE_U32           , 0xb   , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[116] = { RW_ATTRE(event_filter_flags)                  , ATTR_TYPE_U32           , 0x13  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 2097151   },
	[117] = { RO_ATTRS(board)                               , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 1         , .max.ux = 64        },
	[118] = { RW_ATTRX(log_on_boot)                         , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[119] = { RO_ATTRX(input_config_changed)                , ATTR_TYPE_BOOL          , 0x2   , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[120] = { RW_ATTRX(disable_flow_control)                , ATTR_TYPE_BOOL          , 0x13  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[121] = { RW_ATTRX(baud_rate)                           , ATTR_TYPE_U32           , 0x13  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 0         },
	[122] = { RW_ATTRS(tel_trust_path)                      , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[123] = { RW_ATTRS(tel_key_path)                        , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[124] = { RW_ATTRS(dm_trust_path)                       , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[125] = { RW_ATTRS(dm_key_path)                         , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[126] = { RW_ATTRS(fs_trust_path)                       , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[127] = { RW_ATTRS(fs_key_path)                         , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[128] = { RW_ATTRS(p2p_trust_path)                      , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[129] = { RW_ATTRS(p2p_key_path)                        , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[130] = { RW_ATTRS(lwm2m_server_url)                    , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 11        , .max.ux = 255       },
	[131] = { RW_ATTRS(lwm2m_endpoint)                      , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 64        },
	[132] = { RW_ATTRE(lwm2m_security)                      , ATTR_TYPE_U8            , 0x1b  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 4         },
	[133] = { RW_ATTRS(lwm2m_psk_id)                        , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 64        },
	[134] = { RW_ATTRX(lwm2m_psk)                           , ATTR_TYPE_BYTE_ARRAY    , 0x59  , av_array            , NULL                                , .min.ux = 16        , .max.ux = 16        },
	[135] = { RW_ATTRX(lwm2m_bootstrap)                     , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[136] = { RW_ATTRX(lwm2m_short_id)                      , ATTR_TYPE_U16           , 0x1b  , av_uint16           , NULL                                , .min.ux = 1         , .max.ux = 65534     },
	[137] = { RW_ATTRS(lwm2m_mfg)                           , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[138] = { RW_ATTRS(lwm2m_mn)                            , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[139] = { RW_ATTRS(lwm2m_sn)                            , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 64        },
	[140] = { RW_ATTRS(lwm2m_fw_ver)                        , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[141] = { RO_ATTRE(lwm2m_pwr_src)                       , ATTR_TYPE_U8            , 0xb   , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 7         },
	[142] = { RO_ATTRX(lwm2m_pwr_src_volt)                  , ATTR_TYPE_S32           , 0xa   , av_int32            , NULL                                , .min.sx = -600000   , .max.sx = 600000    },
	[143] = { RW_ATTRS(lwm2m_sw_ver)                        , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[144] = { RW_ATTRS(lwm2m_hw_ver)                        , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[145] = { RO_ATTRE(lwm2m_batt_stat)                     , ATTR_TYPE_U8            , 0xa   , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 6         },
	[146] = { RO_ATTRS(lwm2m_fup_pkg_name)                  , ATTR_TYPE_STRING        , 0xa   , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[147] = { RO_ATTRS(lwm2m_fup_pkg_ver)                   , ATTR_TYPE_STRING        , 0xa   , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[148] = { RW_ATTRS(lwm2m_fup_proxy_srv)                 , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 11        , .max.ux = 255       },
	[149] = { RO_ATTRS(bluetooth_address)                   , ATTR_TYPE_STRING        , 0x2   , av_string           , NULL                                , .min.ux = 12        , .max.ux = 12        },
	[150] = { RW_ATTRX(dm_cnx_delay)                        , ATTR_TYPE_U16           , 0x1b  , av_uint16           , NULL                                , .min.ux = 0         , .max.ux = 600       },
	[151] = { RW_ATTRS(factory_load_path)                   , ATTR_TYPE_STRING        , 0x13  , av_string           , NULL                                , .min.ux = 1         , .max.ux = 32        },
	[152] = { RW_ATTRS(device_id)                           , ATTR_TYPE_STRING        , 0x1b  , av_string           , NULL                                , .min.ux = 0         , .max.ux = 64        },
	[153] = { RO_ATTRX(ble_rssi)                            , ATTR_TYPE_S16           , 0xa   , av_int16            , NULL                                , .min.sx = 0         , .max.sx = 0         },
	[154] = { RW_ATTRX(smp_auth_req)                        , ATTR_TYPE_BOOL          , 0x1b  , av_bool             , NULL                                , .min.ux = 0         , .max.ux = 1         },
	[155] = { RW_ATTRX(smp_auth_timeout)                    , ATTR_TYPE_U32           , 0x1b  , av_uint32           , NULL                                , .min.ux = 0         , .max.ux = 86400     },
	[156] = { RW_ATTRS(shell_password)                      , ATTR_TYPE_STRING        , 0x91  , av_string           , NULL                                , .min.ux = 4         , .max.ux = 32        },
	[157] = { RW_ATTRX(shell_session_timeout)               , ATTR_TYPE_U8            , 0x13  , av_uint8            , NULL                                , .min.ux = 0         , .max.ux = 255       }
};
/* pyend */

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
/* pystart - get string */
const char *const attr_get_string_advertising_phy(int value)
{
	switch (value) {
		case 0:           return "1 M";
		case 1:           return "Coded";
		default:          return "?";
	}
}

const char *const attr_get_string_config_type(int value)
{
	switch (value) {
		case 0:           return "Not Configured";
		case 1:           return "Analog";
		case 2:           return "Digital";
		case 3:           return "Temperature";
		case 4:           return "Current";
		case 5:           return "Ultrasonic Pressure";
		case 6:           return "Spi Or I2 C";
		default:          return "?";
	}
}

const char *const attr_get_string_digital_input(int value)
{
	switch (value) {
		case 1:           return "Port1 Bitmask";
		case 2:           return "Port2 Bitmask";
		default:          return "?";
	}
}

const char *const attr_get_string_analog_input_1_type(int value)
{
	switch (value) {
		case 0:           return "Unused";
		case 1:           return "Voltage 0 V To 10 V DC";
		case 2:           return "Current 4 Ma To 20 Ma";
		case 3:           return "Pressure";
		case 4:           return "Ultrasonic";
		case 5:           return "AC Current 20 A";
		case 6:           return "AC Current 150 A";
		case 7:           return "AC Current 500 A";
		default:          return "?";
	}
}

const char *const attr_get_string_magnet_state(int value)
{
	switch (value) {
		case true:        return "Far";
		case false:       return "Near";
		default:          return "?";
	}
}

const char *const attr_get_string_tamper_switch_status(int value)
{
	switch (value) {
		case true:        return "ACtive Not Pressed";
		case false:       return "Inactive Pressed";
		default:          return "?";
	}
}

const char *const attr_get_string_boot_phy(int value)
{
	switch (value) {
		case 0:           return "Default";
		case 1:           return "Coded";
		case 2:           return "1 M";
		default:          return "?";
	}
}

const char *const attr_get_string_bluetooth_flags(int value)
{
	switch (value) {
		case 1:           return "Device Management Data Ready Bitmask";
		case 2:           return "Memfault Data Bitmask";
		case 4:           return "Time Was Set Bitmask";
		case 8:           return "ACtive Mode Bitmask";
		case 16:          return "Low Battery Alarm Bitmask";
		case 32:          return "Digital In1 State Bitmask";
		case 64:          return "Digital In2 State Bitmask";
		case 128:         return "Tamper Switch State Bitmask";
		case 256:         return "Magnet State Bitmask";
		case 512:         return "Unused 0";
		case 1024:        return "Unused 1";
		case 2048:        return "Unused 2";
		case 4096:        return "Unused 3";
		case 8192:        return "Unused 4";
		case 16384:       return "Unused 5";
		case 32768:       return "Unused 6";
		case 65536:       return "Unused 7";
		case 131072:      return "Unused 8";
		case 262144:      return "Unused 9";
		case 524288:      return "Unused 10";
		case 1048576:     return "Unused 11";
		case 2097152:     return "Unused 12";
		case 4194304:     return "Unused 13";
		case 8388608:     return "Unused 14";
		case 16777216:    return "Unused 15";
		case 33554432:    return "Unused 16";
		case 67108864:    return "Unused 17";
		case 134217728:   return "Unused 18";
		case 268435456:   return "Unused 19";
		case 536870912:   return "Unused 20";
		case 1073741824:  return "Unused 21";
		case 2147483648:  return "Unused 22";
		default:          return "?";
	}
}

const char *const attr_get_string_event_filter_flags(int value)
{
	switch (value) {
		case 1:           return "Temperature 1 Event Bitmask";
		case 2:           return "Temperature 2 Event Bitmask";
		case 4:           return "Temperature 3 Event Bitmask";
		case 8:           return "Temperature 4 Event Bitmask";
		case 16:          return "Voltage 1 Event Bitmask";
		case 32:          return "Voltage 2 Event Bitmask";
		case 64:          return "Voltage 3 Event Bitmask";
		case 128:         return "Voltage 4 Event Bitmask";
		case 256:         return "Current 1 Event Bitmask";
		case 512:         return "Current 2 Event Bitmask";
		case 1024:        return "Current 3 Event Bitmask";
		case 2048:        return "Current 4 Event Bitmask";
		case 4096:        return "Ultrasonic Event Bitmask";
		case 8192:        return "Pressure 1 Event Bitmask";
		case 16384:       return "Pressure 2 Event Bitmask";
		case 32768:       return "Tamper Switch Event Bitmask";
		case 65536:       return "Magnet Sense Event Bitmask";
		case 131072:      return "Battery Good Event Bitmask";
		case 262144:      return "Battery Bad Event Bitmask";
		case 524288:      return "Digital In1 Event Bitmask";
		case 1048576:     return "Digital In2 Event Bitmask";
		case 2097152:     return "Unused 0";
		case 4194304:     return "Unused 1";
		case 8388608:     return "Unused 2";
		case 16777216:    return "Unused 3";
		case 33554432:    return "Unused 4";
		case 67108864:    return "Unused 5";
		case 134217728:   return "Unused 6";
		case 268435456:   return "Unused 7";
		case 536870912:   return "Unused 8";
		case 1073741824:  return "Unused 9";
		case 2147483648:  return "Unused 10";
		default:          return "?";
	}
}

const char *const attr_get_string_lwm2m_security(int value)
{
	switch (value) {
		case 0:           return "PSK";
		case 1:           return "RPK";
		case 2:           return "Cert";
		case 3:           return "No Sec";
		case 4:           return "Cert Est";
		default:          return "?";
	}
}

const char *const attr_get_string_lwm2m_pwr_src(int value)
{
	switch (value) {
		case 0:           return "DC";
		case 1:           return "Int Batt";
		case 2:           return "Ext Batt";
		case 3:           return "Fuel Cell";
		case 4:           return "PoE";
		case 5:           return "USB";
		case 6:           return "AC";
		case 7:           return "Solar";
		default:          return "?";
	}
}

const char *const attr_get_string_lwm2m_batt_stat(int value)
{
	switch (value) {
		case 0:           return "Norm";
		case 1:           return "Charging";
		case 2:           return "Charge Comp";
		case 3:           return "Damaged";
		case 4:           return "Low";
		case 5:           return "Not Inst";
		case 6:           return "Unknown";
		default:          return "?";
	}
}

/* pyend */

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
void attr_table_initialize(void)
{
	memcpy(&rw, &DRW, sizeof(rw_attribute_t));
	memcpy(&ro, &DRO, sizeof(ro_attribute_t));
}

void attr_table_factory_reset(void)
{
	size_t i = 0;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		memcpy(ATTR_TABLE[i].pData, ATTR_TABLE[i].pDefault, ATTR_TABLE[i].size);
	}
}

const struct attr_table_entry *const attr_map(attr_id_t id)
{
	if (id == 0 || id > ATTR_TABLE_MAX_ID) {
		return NULL;
	} else {
		return &ATTR_TABLE[id];
	}
}

attr_index_t attr_table_index(const struct attr_table_entry *const entry)
{
	__ASSERT(PART_OF_ARRAY(ATTR_TABLE, entry), "Invalid entry");
	return (entry - &ATTR_TABLE[0]);
}
