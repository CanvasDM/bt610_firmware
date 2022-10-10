/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -C -c -G -D -t -T --word-array-name=word_list --lookup-function-name=attr_id_from_hash --hash-function-name=gperf_hash --output-file=/home/jenkins/persistent/workspace/ephyr_BT610_DM_Firmware_GL_22796/bt6xx_firmware/peripheral_IO/../components/attributes/bt610/src/attr_hash.c /home/jenkins/persistent/workspace/ephyr_BT610_DM_Firmware_GL_22796/bt6xx_firmware/peripheral_IO/../components/attributes/bt610/src/attr.gperf  */
/* Computed positions: -k'1,7,9,13-16,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif


#include "attr_hash.h"

#define TOTAL_KEYWORDS 153
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 34
#define MIN_HASH_VALUE 7
#define MAX_HASH_VALUE 490
/* maximum key range = 484, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
gperf_hash (register const char *str, register size_t len)
{
  static const unsigned short asso_values[] =
    {
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491,  60,  15,
        5, 105,  70, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491,   5, 491,   0, 120,  40,
        0,   0,   5,  95, 105,  35, 491,   5,   5,  35,
        0,   5,  60,  50,   0,   0,   0,   5, 115,  90,
        0, 120, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491, 491, 491, 491, 491,
      491, 491, 491, 491, 491, 491
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[15]];
      /*FALLTHROUGH*/
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      /*FALLTHROUGH*/
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      /*FALLTHROUGH*/
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
      case 11:
      case 10:
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct attr_hash_entry word_list[] =
  {
    {"oe", 20},
    {"tx_power", 15},
    {"sh_offset", 44},
    {"uptime", 12},
    {"reset_reason", 10},
    {"lwm2m_sn", 135},
    {"device_id", 148},
    {"sensor_name", 1},
    {"location", 2},
    {"lwm2m_endpoint", 127},
    {"active_mode", 13},
    {"lwm2m_sw_ver", 139},
    {"log_on_boot", 117},
    {"lwm2m_fw_ver", 136},
    {"security_level", 112},
    {"attr_save_error_code", 110},
    {"smp_auth_timeout", 151},
    {"adc_vref_simulated", 66},
    {"analog_input_2", 31},
    {"temperature_result_2", 23},
    {"temperature_sense_interval", 6},
    {"adc_analog_sensor_simulated", 62},
    {"analog_input_2_type", 35},
    {"adc_vref_simulated_counts", 67},
    {"lwm2m_server_url", 126},
    {"event_filter_flags", 116},
    {"adc_analog_sensor_simulated_counts", 63},
    {"temperature_result_1", 22},
    {"api_version", 41},
    {"digital_output_2_state", 8},
    {"lwm2m_mn", 134},
    {"analog_input_1_type", 34},
    {"network_id", 16},
    {"magnet_state", 38},
    {"digital_input", 27},
    {"analog_input_1", 30},
    {"reset_count", 11},
    {"digital_output_1_state", 7},
    {"lwm2m_short_id", 132},
    {"qrtc_last_set", 43},
    {"config_version", 17},
    {"firmware_version", 9},
    {"smp_auth_req", 150},
    {"adc_thermistor_simulated", 64},
    {"analog_sense_interval", 45},
    {"temperature_2_simulated", 92},
    {"shell_password", 152},
    {"adc_thermistor_simulated_counts", 65},
    {"temperature_2_simulated_value", 93},
    {"digital_input_2_simulated", 102},
    {"lwm2m_psk_id", 129},
    {"temperature_1_simulated", 90},
    {"lwm2m_psk", 130},
    {"digital_input_2_simulated_value", 103},
    {"temperature_1_simulated_value", 91},
    {"digital_input_1_simulated", 100},
    {"pressure_simulated", 78},
    {"qrtc", 42},
    {"advertising_duration", 4},
    {"digital_input_1_simulated_value", 101},
    {"ge", 19},
    {"pressure_simulated_value", 79},
    {"temperature_result_4", 25},
    {"analog_input_4_type", 37},
    {"current_2_simulated", 82},
    {"current_2_simulated_value", 83},
    {"current_1_simulated", 80},
    {"power_sense_interval", 5},
    {"dm_key_path", 121},
    {"current_1_simulated_value", 81},
    {"fs_key_path", 123},
    {"lwm2m_hw_ver", 140},
    {"reserved0", 0},
    {"advertising_interval", 3},
    {"temperature_result_3", 24},
    {"dm_cnx_delay", 146},
    {"temperature_4_simulated", 96},
    {"analog_input_3_type", 36},
    {"lwm2m_batt_stat", 141},
    {"temperature_4_simulated_value", 97},
    {"ultrasonic_simulated", 76},
    {"factory_reset_enable", 59},
    {"ultrasonic_simulated_value", 77},
    {"adc_power_simulated", 60},
    {"lwm2m_pwr_src", 137},
    {"adc_power_simulated_counts", 61},
    {"thermistor_config", 21},
    {"ble_rssi", 149},
    {"analog_input_4", 33},
    {"vref_simulated", 88},
    {"digital_input_2_config", 29},
    {"temperature_3_simulated", 94},
    {"current_4_simulated", 86},
    {"config_type", 18},
    {"temperature_3_simulated_value", 95},
    {"current_4_simulated_value", 87},
    {"therm_2_coefficient_a", 48},
    {"digital_input_1_config", 28},
    {"voltage_2_simulated", 70},
    {"lwm2m_fup_pkg_name", 142},
    {"voltage_2_simulated_value", 71},
    {"therm_1_coefficient_a", 47},
    {"power_voltage", 26},
    {"voltage_1_simulated", 68},
    {"voltage_1_simulated_value", 69},
    {"mobile_app_disconnect", 109},
    {"current_3_simulated", 84},
    {"mag_switch_simulated", 104},
    {"current_3_simulated_value", 85},
    {"mag_switch_simulated_value", 105},
    {"dump_path", 114},
    {"therm_2_coefficient_c", 56},
    {"load_path", 113},
    {"tamper_switch_status", 46},
    {"block_downgrades", 111},
    {"dm_trust_path", 120},
    {"tel_trust_path", 118},
    {"therm_1_coefficient_c", 55},
    {"fs_trust_path", 122},
    {"analog_input_3", 32},
    {"param_path", 39},
    {"lwm2m_mfg", 133},
    {"therm_4_coefficient_a", 50},
    {"bluetooth_address", 145},
    {"lwm2m_pwr_src_volt", 138},
    {"voltage_4_simulated", 74},
    {"battery_age", 40},
    {"voltage_4_simulated_value", 75},
    {"tamper_switch_simulated", 106},
    {"lwm2m_bootstrap", 131},
    {"tamper_switch_simulated_value", 107},
    {"power_volts_simulated", 98},
    {"therm_3_coefficient_a", 49},
    {"power_volts_simulated_value", 99},
    {"voltage_3_simulated", 72},
    {"therm_4_coefficient_c", 58},
    {"p2p_trust_path", 124},
    {"voltage_3_simulated_value", 73},
    {"vref_simulated_value", 89},
    {"tel_key_path", 119},
    {"lwm2m_security", 128},
    {"therm_2_coefficient_b", 52},
    {"lwm2m_fup_pkg_ver", 143},
    {"therm_1_coefficient_b", 51},
    {"factory_load_path", 147},
    {"therm_3_coefficient_c", 57},
    {"lwm2m_fup_proxy_srv", 144},
    {"bluetooth_flags", 115},
    {"boot_phy", 108},
    {"p2p_key_path", 125},
    {"therm_4_coefficient_b", 54},
    {"therm_3_coefficient_b", 53},
    {"advertising_phy", 14}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,   1,   2,
     -1,   3,   4,   5,   6,  -1,   7,  -1,   8,   9,
     -1,  10,  11,  -1,  -1,  -1,  12,  13,  -1,  14,
     15,  16,  -1,  17,  18,  19,  20,  21,  -1,  22,
     23,  24,  -1,  25,  26,  27,  28,  29,  30,  31,
     32,  -1,  33,  34,  35,  -1,  36,  37,  -1,  38,
     -1,  -1,  -1,  39,  40,  -1,  41,  42,  -1,  43,
     -1,  44,  -1,  45,  46,  -1,  47,  -1,  -1,  48,
     49,  -1,  50,  51,  52,  -1,  53,  -1,  -1,  54,
     55,  -1,  -1,  56,  57,  58,  59,  60,  -1,  61,
     62,  -1,  -1,  -1,  63,  -1,  -1,  -1,  -1,  64,
     -1,  -1,  -1,  -1,  -1,  65,  -1,  -1,  -1,  66,
     67,  68,  -1,  -1,  -1,  69,  70,  71,  -1,  72,
     73,  -1,  -1,  -1,  -1,  74,  -1,  75,  76,  77,
     78,  -1,  -1,  -1,  79,  80,  -1,  -1,  -1,  -1,
     81,  82,  -1,  -1,  83,  -1,  -1,  -1,  84,  -1,
     -1,  85,  86,  87,  88,  -1,  -1,  -1,  -1,  89,
     -1,  -1,  90,  91,  92,  -1,  93,  -1,  -1,  94,
     95,  96,  97,  -1,  98,  -1,  -1,  -1,  99,  -1,
    100, 101,  -1, 102, 103,  -1,  -1,  -1,  -1,  -1,
    104, 105,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 106,
    107,  -1,  -1,  -1,  -1, 108, 109,  -1,  -1, 110,
     -1, 111,  -1,  -1, 112, 113, 114,  -1, 115, 116,
     -1, 117,  -1, 118, 119, 120,  -1,  -1,  -1, 121,
     -1,  -1,  -1,  -1,  -1,  -1, 122, 123, 124, 125,
     -1, 126,  -1,  -1,  -1, 127,  -1,  -1,  -1,  -1,
     -1,  -1,  -1, 128,  -1, 129,  -1,  -1,  -1, 130,
     -1,  -1,  -1,  -1,  -1,  -1, 131,  -1,  -1,  -1,
     -1, 132, 133,  -1, 134,  -1, 135,  -1,  -1, 136,
    137,  -1,  -1,  -1,  -1, 138,  -1, 139,  -1, 140,
     -1, 141, 142,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1, 143,  -1,  -1,  -1,  -1,  -1, 144,  -1,  -1,
     -1, 145,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1, 146,  -1,  -1,  -1,  -1,  -1,
    147,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1, 148,  -1,  -1,  -1, 149,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1, 150,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1, 151,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    152
  };

const struct attr_hash_entry *
attr_id_from_hash (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = gperf_hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = word_list[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &word_list[index];
            }
        }
    }
  return 0;
}
