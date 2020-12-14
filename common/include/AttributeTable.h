/**
 * @file AttributeTable.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
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
	UNKNOWN_TYPE = 0,
	UNSIGNED_TYPE,
	SIGNED_TYPE,
	FLOAT_TYPE,
	STRING_TYPE
} AttributeType_t;

typedef struct AttributeEntry AttributeEntry_t;

struct AttributeEntry {
	const char *const name;
	void *pData;
	void const *const pDefault;
	const size_t size;
	const bool writeable;
	const bool readable;
	const AttributeType_t type;
	const bool backup; /* not factory resetable */
	const bool lockable;
	const bool broadcast;
	int (*pValidator)(AttributeEntry_t *, void *, size_t, bool);
	const uint32_t min;
	const uint32_t max;
	bool modified;
	bool deprecated;
};

/* pystart - attribute table size */
#define ATTR_TABLE_SIZE 114

/* pyend */

/* The code generator tool should generate this */
#define ATTR_MAX_STR_LENGTH 100

#define ATTR_MAX_STR_SIZE (ATTR_MAX_STR_LENGTH + 1)

#define ATTR_MAX_VERSION_LENGTH 11

/******************************************************************************/
/* Indices                                                                    */
/******************************************************************************/
/* clang-format off */
/* pystart - attribute indices */
#define ATTR_INDEX_sensorName                            0
#define ATTR_INDEX_sensorLocation                        1
#define ATTR_INDEX_advertisingInterval                   2
#define ATTR_INDEX_advertisingDuration                   3
#define ATTR_INDEX_passkey                               4
#define ATTR_INDEX_lock                                  5
#define ATTR_INDEX_batterySenseInterval                  6
#define ATTR_INDEX_temperatureSenseInterval              7
#define ATTR_INDEX_temperatureAggregationCount           8
#define ATTR_INDEX_digitalOutput1Mv                      9
#define ATTR_INDEX_digitalOutput2Mv                      10
#define ATTR_INDEX_digitalInput1                         11
#define ATTR_INDEX_digitalInput2                         12
#define ATTR_INDEX_analogInput1Type                      13
#define ATTR_INDEX_analogInput2Type                      14
#define ATTR_INDEX_analogInput3Type                      15
#define ATTR_INDEX_analogInput4Type                      16
#define ATTR_INDEX_highTemp1Thresh1                      17
#define ATTR_INDEX_highTemp1Thresh2                      18
#define ATTR_INDEX_lowTemp1Thresh1                       19
#define ATTR_INDEX_lowTemp1Thresh2                       20
#define ATTR_INDEX_temp1DeltaThresh                      21
#define ATTR_INDEX_highTemp2Thresh1                      22
#define ATTR_INDEX_highTemp2Thresh2                      23
#define ATTR_INDEX_lowTemp2Thresh1                       24
#define ATTR_INDEX_lowTemp2Thresh2                       25
#define ATTR_INDEX_temp2DeltaThresh                      26
#define ATTR_INDEX_highTemp3Thresh1                      27
#define ATTR_INDEX_highTemp3Thresh2                      28
#define ATTR_INDEX_lowTemp3Thresh1                       29
#define ATTR_INDEX_lowTemp3Thresh2                       30
#define ATTR_INDEX_temp3DeltaThresh                      31
#define ATTR_INDEX_highTemp4Thresh1                      32
#define ATTR_INDEX_highTemp4Thresh2                      33
#define ATTR_INDEX_lowTemp4Thresh1                       34
#define ATTR_INDEX_lowTemp4Thresh2                       35
#define ATTR_INDEX_temp4DeltaThresh                      36
#define ATTR_INDEX_highAnalog1Thresh1                    37
#define ATTR_INDEX_highAnalog1Thresh2                    38
#define ATTR_INDEX_lowAnalog1Thresh1                     39
#define ATTR_INDEX_lowAnalog1Thresh2                     40
#define ATTR_INDEX_analog1DeltaThresh                    41
#define ATTR_INDEX_highAnalog2Thresh1                    42
#define ATTR_INDEX_highAnalog2Thresh2                    43
#define ATTR_INDEX_lowAnalog2Thresh1                     44
#define ATTR_INDEX_lowAnalog2Thresh2                     45
#define ATTR_INDEX_analog2DeltaThresh                    46
#define ATTR_INDEX_highAnalog3Thresh1                    47
#define ATTR_INDEX_highAnalog3Thresh2                    48
#define ATTR_INDEX_lowAnalog3Thresh1                     49
#define ATTR_INDEX_lowAnalog3Thresh2                     50
#define ATTR_INDEX_analog3DeltaThresh                    51
#define ATTR_INDEX_highAnalog4Thresh1                    52
#define ATTR_INDEX_highAnalog4Thresh2                    53
#define ATTR_INDEX_lowAnalog4Thresh1                     54
#define ATTR_INDEX_lowAnalog4Thresh2                     55
#define ATTR_INDEX_analog4DeltaThresh                    56
#define ATTR_INDEX_activeMode                            57
#define ATTR_INDEX_useCodedPhy                           58
#define ATTR_INDEX_txPower                               59
#define ATTR_INDEX_networkId                             60
#define ATTR_INDEX_configVersion                         61
#define ATTR_INDEX_configType                            62
#define ATTR_INDEX_hardwareMinorVersion                  63
#define ATTR_INDEX_ledTestActive                         64
#define ATTR_INDEX_coefficient1                          65
#define ATTR_INDEX_coefficient2                          66
#define ATTR_INDEX_coefficientA                          67
#define ATTR_INDEX_coefficientB                          68
#define ATTR_INDEX_coefficientC                          69
#define ATTR_INDEX_thermistorIndex                       70
#define ATTR_INDEX_temperatureResult1                    71
#define ATTR_INDEX_temperatureResult2                    72
#define ATTR_INDEX_temperatureResult3                    73
#define ATTR_INDEX_temperatureResult4                    74
#define ATTR_INDEX_batteryVoltageMv                      75
#define ATTR_INDEX_digitalInput1Alarm                    76
#define ATTR_INDEX_digitalInput2Alarm                    77
#define ATTR_INDEX_currentReadingMa                      78
#define ATTR_INDEX_highTemperature1Alarm                 79
#define ATTR_INDEX_lowTemperature1Alarm                  80
#define ATTR_INDEX_deltaTemperature1Alarm                81
#define ATTR_INDEX_highTemperature2Alarm                 82
#define ATTR_INDEX_lowTemperature2Alarm                  83
#define ATTR_INDEX_deltaTemperature2Alarm                84
#define ATTR_INDEX_highTemperature3Alarm                 85
#define ATTR_INDEX_lowTemperature3Alarm                  86
#define ATTR_INDEX_deltaTemperature3Alarm                87
#define ATTR_INDEX_highTemperature4Alarm                 88
#define ATTR_INDEX_lowTemperature4Alarm                  89
#define ATTR_INDEX_deltaTemperature4Alarm                90
#define ATTR_INDEX_highAnalog1Alarm                      91
#define ATTR_INDEX_lowAnalog1Alarm                       92
#define ATTR_INDEX_deltaAnalog1Alarm                     93
#define ATTR_INDEX_highAnalog2Alarm                      94
#define ATTR_INDEX_lowAnalog2Alarm                       95
#define ATTR_INDEX_deltaAnalog2Alarm                     96
#define ATTR_INDEX_highAnalog3Alarm                      97
#define ATTR_INDEX_lowAnalog3Alarm                       98
#define ATTR_INDEX_deltaAnalog3Alarm                     99
#define ATTR_INDEX_highAnalog4Alarm                      100
#define ATTR_INDEX_lowAnalog4Alarm                       101
#define ATTR_INDEX_deltaAnalog4Alarm                     102
#define ATTR_INDEX_hwVersion                             103
#define ATTR_INDEX_firmwareVersion                       104
#define ATTR_INDEX_resetReason                           105
#define ATTR_INDEX_bluetoothAddress                      106
#define ATTR_INDEX_mtu                                   107
#define ATTR_INDEX_flags                                 108
#define ATTR_INDEX_resetCount                            109
#define ATTR_INDEX_digitalInput1Mv                       110
#define ATTR_INDEX_digitalInput2Mv                       111
#define ATTR_INDEX_magnetState                           112
#define ATTR_INDEX_bootloaderVersion                     113
/* pyend */
/* clang-format on */

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
int AttributeValidator_string(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite);
int AttributeValidator_float(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TABLE_H__ */
