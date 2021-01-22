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
	ATTR_TYPE_UNKNOWN = 0,
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
	int (*const pValidator)(AttributeEntry_t *, void *, size_t, bool);
	int (*const pPrepare)(void);
	const minmax_t min;
	const minmax_t max;
	bool modified;
};

/* pystart - attribute table size */
#define ATTR_TABLE_SIZE 97

/* pyend */

/* The code generator tool should generate this */
#define ATTR_MAX_STR_LENGTH 100

#define ATTR_MAX_STR_SIZE (ATTR_MAX_STR_LENGTH + 1)

#define ATTR_MAX_HEX_SIZE 8

#define ATTR_MAX_VERSION_LENGTH 11

typedef enum
{
	CONFIG_UNDEFINED = 0,
	CONFIG_ANALOG_VOLTAGE,
	CONFIG_DIGITAL,
	CONFIG_TEMPERATURE,
	CONFIG_ANALOG_CURRENT,
	CONFIG_ULTRASONIC_PRESSURE,
	CONFIG_SPI_I2C,
}configType_t;
typedef enum
{
	ANALOG_UNUSED = 0,
	ANALOG_VOLTAGE,
	ANALOG_CURRENT,
	ANALOG_PRESSURE,
	ANALOG_ULTRASONIC,
}analogConfigType_t;


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
#define ATTR_INDEX_digitalOutput1Enable                  9
#define ATTR_INDEX_digitalOutput2Enable                  10
#define ATTR_INDEX_firmwareVersion                       11
#define ATTR_INDEX_resetReason                           12
#define ATTR_INDEX_bluetoothAddress                      13
#define ATTR_INDEX_resetCount                            14
#define ATTR_INDEX_bootloaderVersion                     15
#define ATTR_INDEX_upTime                                16
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
#define ATTR_INDEX_ge                                    64
#define ATTR_INDEX_oe                                    65
#define ATTR_INDEX_coefficientA                          66
#define ATTR_INDEX_coefficientB                          67
#define ATTR_INDEX_coefficientC                          68
#define ATTR_INDEX_thermistorConfig                      69
#define ATTR_INDEX_temperatureResult1                    70
#define ATTR_INDEX_temperatureResult2                    71
#define ATTR_INDEX_temperatureResult3                    72
#define ATTR_INDEX_temperatureResult4                    73
#define ATTR_INDEX_temperatureAlarms                     74
#define ATTR_INDEX_batteryVoltageMv                      75
#define ATTR_INDEX_digitalInput                          76
#define ATTR_INDEX_digitalAlarms                         77
#define ATTR_INDEX_digitalInput1Config                   78
#define ATTR_INDEX_digitalInput2Config                   79
#define ATTR_INDEX_analogInput1                          80
#define ATTR_INDEX_analogInput2                          81
#define ATTR_INDEX_analogInput3                          82
#define ATTR_INDEX_analogInput4                          83
#define ATTR_INDEX_analogAlarms                          84
#define ATTR_INDEX_analogInput1Type                      85
#define ATTR_INDEX_analogInput2Type                      86
#define ATTR_INDEX_analogInput3Type                      87
#define ATTR_INDEX_analogInput4Type                      88
#define ATTR_INDEX_flags                                 89
#define ATTR_INDEX_magnetState                           90
#define ATTR_INDEX_paramPath                             91
#define ATTR_INDEX_batteryAge                            92
#define ATTR_INDEX_apiVersion                            93
#define ATTR_INDEX_qrtc                                  94
#define ATTR_INDEX_qrtcLastSet                           95
#define ATTR_INDEX_shOffset                              96
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
int AttributeValidator_string(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint64(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite);
int AttributeValidator_uint8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int64(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);
int AttributeValidator_int8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite);
int AttributeValidator_float(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite);

int AttributeValidator_aic(AttributeEntry_t *pEntry, void *pValue,
			   size_t Length, bool DoWrite);

/* The weak implementations should be overridden application. */
/* pystart - prepare for read */
int AttributePrepare_temperatureResult1(void);
int AttributePrepare_temperatureResult2(void);
int AttributePrepare_temperatureResult3(void);
int AttributePrepare_temperatureResult4(void);
int AttributePrepare_batteryVoltageMv(void);
int AttributePrepare_analogInput1(void);
int AttributePrepare_analogInput2(void);
int AttributePrepare_analogInput3(void);
int AttributePrepare_analogInput4(void);
/* pyend */

#ifdef __cplusplus
}
#endif

#endif /* __ATTRIBUTE_TABLE_H__ */
