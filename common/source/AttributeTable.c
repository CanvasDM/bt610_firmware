/**
 * @file AttributeTable.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
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

#define u UNSIGNED_TYPE
#define i SIGNED_TYPE
#define f FLOAT_TYPE
#define s STRING_TYPE

/* Add things to the end!  Do not remove items. Change them to deprecated. */
typedef struct RwAttributesTag {
	/* pystart - rw attributes */
	char sensorName[23 + 1];
	char sensorLocation[32 + 1];
	uint16_t advertisingInterval;
	uint16_t advertisingDuration;
	char passkey[6 + 1];
	uint8_t lock;
	uint16_t batterySenseInterval;
	uint16_t temperatureSenseInterval;
	uint8_t temperatureAggregationCount;
	uint16_t digitalOutput1Mv;
	uint16_t digitalOutput2Mv;
	uint8_t digitalInput1;
	uint8_t digitalInput2;
	uint8_t analogInput1Type;
	uint8_t analogInput2Type;
	uint8_t analogInput3Type;
	uint8_t analogInput4Type;
	int16_t highTemp1Thresh1;
	int16_t highTemp1Thresh2;
	int16_t lowTemp1Thresh1;
	int16_t lowTemp1Thresh2;
	uint16_t temp1DeltaThresh;
	int16_t highTemp2Thresh1;
	int16_t highTemp2Thresh2;
	int16_t lowTemp2Thresh1;
	int16_t lowTemp2Thresh2;
	uint16_t temp2DeltaThresh;
	int16_t highTemp3Thresh1;
	int16_t highTemp3Thresh2;
	int16_t lowTemp3Thresh1;
	int16_t lowTemp3Thresh2;
	uint16_t temp3DeltaThresh;
	int16_t highTemp4Thresh1;
	int16_t highTemp4Thresh2;
	int16_t lowTemp4Thresh1;
	int16_t lowTemp4Thresh2;
	uint16_t temp4DeltaThresh;
	uint16_t highAnalog1Thresh1;
	uint16_t highAnalog1Thresh2;
	uint16_t lowAnalog1Thresh1;
	uint16_t lowAnalog1Thresh2;
	uint16_t analog1DeltaThresh;
	uint16_t highAnalog2Thresh1;
	uint16_t highAnalog2Thresh2;
	uint16_t lowAnalog2Thresh1;
	uint16_t lowAnalog2Thresh2;
	uint16_t analog2DeltaThresh;
	uint16_t highAnalog3Thresh1;
	uint16_t highAnalog3Thresh2;
	uint16_t lowAnalog3Thresh1;
	uint16_t lowAnalog3Thresh2;
	uint16_t analog3DeltaThresh;
	uint16_t highAnalog4Thresh1;
	uint16_t highAnalog4Thresh2;
	uint16_t lowAnalog4Thresh1;
	uint16_t lowAnalog4Thresh2;
	uint16_t analog4DeltaThresh;
	uint8_t activeMode;
	uint8_t useCodedPhy;
	int8_t txPower;
	uint16_t networkId;
	uint8_t configVersion;
	uint8_t configType;
	uint8_t hardwareMinorVersion;
	uint8_t ledTestActive;
	float coefficient1;
	float coefficient2;
	float coefficientA;
	float coefficientB;
	float coefficientC;
	uint8_t thermistorIndex;
	/* pyend */
} RwAttribute_t;

static const RwAttribute_t DEFAULT_RW_ATTRIBUTE_VALUES = {
	/* pystart - rw defaults */
	.sensorName = "BT610",
	.sensorLocation = "",
	.advertisingInterval = 1000,
	.advertisingDuration = 0,
	.passkey = "123456",
	.lock = 0,
	.batterySenseInterval = 0,
	.temperatureSenseInterval = 0,
	.temperatureAggregationCount = 1,
	.digitalOutput1Mv = 0,
	.digitalOutput2Mv = 0,
	.digitalInput1 = 0,
	.digitalInput2 = 0,
	.analogInput1Type = 0,
	.analogInput2Type = 0,
	.analogInput3Type = 0,
	.analogInput4Type = 0,
	.highTemp1Thresh1 = 127,
	.highTemp1Thresh2 = 127,
	.lowTemp1Thresh1 = -127,
	.lowTemp1Thresh2 = -127,
	.temp1DeltaThresh = 255,
	.highTemp2Thresh1 = 127,
	.highTemp2Thresh2 = 127,
	.lowTemp2Thresh1 = -127,
	.lowTemp2Thresh2 = -127,
	.temp2DeltaThresh = 255,
	.highTemp3Thresh1 = 127,
	.highTemp3Thresh2 = 127,
	.lowTemp3Thresh1 = -127,
	.lowTemp3Thresh2 = -127,
	.temp3DeltaThresh = 255,
	.highTemp4Thresh1 = 127,
	.highTemp4Thresh2 = 127,
	.lowTemp4Thresh1 = -127,
	.lowTemp4Thresh2 = -127,
	.temp4DeltaThresh = 255,
	.highAnalog1Thresh1 = 4096,
	.highAnalog1Thresh2 = 4096,
	.lowAnalog1Thresh1 = 0,
	.lowAnalog1Thresh2 = 0,
	.analog1DeltaThresh = 4096,
	.highAnalog2Thresh1 = 4096,
	.highAnalog2Thresh2 = 4096,
	.lowAnalog2Thresh1 = 0,
	.lowAnalog2Thresh2 = 0,
	.analog2DeltaThresh = 4096,
	.highAnalog3Thresh1 = 4096,
	.highAnalog3Thresh2 = 4096,
	.lowAnalog3Thresh1 = 0,
	.lowAnalog3Thresh2 = 0,
	.analog3DeltaThresh = 4096,
	.highAnalog4Thresh1 = 4096,
	.highAnalog4Thresh2 = 4096,
	.lowAnalog4Thresh1 = 0,
	.lowAnalog4Thresh2 = 0,
	.analog4DeltaThresh = 4096,
	.activeMode = 0,
	.useCodedPhy = 0,
	.txPower = 0,
	.networkId = 0,
	.configVersion = 0,
	.configType = 0,
	.hardwareMinorVersion = 0,
	.ledTestActive = 0,
	.coefficient1 = 220.7351,
	.coefficient2 = 3.98e3,
	.coefficientA = 0.001132,
	.coefficientB = 0.0002338,
	.coefficientC = 8.780e-8,
	.thermistorIndex = 0
	/* pyend */
};

typedef struct RoAttributesTag {
	/* pystart - ro attributes */
	int32_t temperatureResult1;
	int32_t temperatureResult2;
	int32_t temperatureResult3;
	int32_t temperatureResult4;
	uint16_t batteryVoltageMv;
	uint8_t digitalInput1Alarm;
	uint8_t digitalInput2Alarm;
	uint8_t currentReadingMa;
	uint8_t highTemperature1Alarm;
	uint8_t lowTemperature1Alarm;
	uint8_t deltaTemperature1Alarm;
	uint8_t highTemperature2Alarm;
	uint8_t lowTemperature2Alarm;
	uint8_t deltaTemperature2Alarm;
	uint8_t highTemperature3Alarm;
	uint8_t lowTemperature3Alarm;
	uint8_t deltaTemperature3Alarm;
	uint8_t highTemperature4Alarm;
	uint8_t lowTemperature4Alarm;
	uint8_t deltaTemperature4Alarm;
	uint8_t highAnalog1Alarm;
	uint8_t lowAnalog1Alarm;
	uint8_t deltaAnalog1Alarm;
	uint8_t highAnalog2Alarm;
	uint8_t lowAnalog2Alarm;
	uint8_t deltaAnalog2Alarm;
	uint8_t highAnalog3Alarm;
	uint8_t lowAnalog3Alarm;
	uint8_t deltaAnalog3Alarm;
	uint8_t highAnalog4Alarm;
	uint8_t lowAnalog4Alarm;
	uint8_t deltaAnalog4Alarm;
	char hwVersion[0 + 1];
	char firmwareVersion[11 + 1];
	char resetReason[8 + 1];
	char bluetoothAddress[12 + 1];
	uint8_t mtu;
	uint32_t flags;
	uint16_t resetCount;
	uint8_t digitalInput1Mv;
	uint8_t digitalInput2Mv;
	uint8_t magnetState;
	char bootloaderVersion[11 + 1];
	/* pyend */
} RoAttribute_t;

static const RoAttribute_t DEFAULT_RO_ATTRIBUTE_VALUES = {
	/* pystart - ro defaults */
	.temperatureResult1 = 0,
	.temperatureResult2 = 0,
	.temperatureResult3 = 0,
	.temperatureResult4 = 0,
	.batteryVoltageMv = 0,
	.digitalInput1Alarm = 0,
	.digitalInput2Alarm = 0,
	.currentReadingMa = 0,
	.highTemperature1Alarm = 0,
	.lowTemperature1Alarm = 0,
	.deltaTemperature1Alarm = 0,
	.highTemperature2Alarm = 0,
	.lowTemperature2Alarm = 0,
	.deltaTemperature2Alarm = 0,
	.highTemperature3Alarm = 0,
	.lowTemperature3Alarm = 0,
	.deltaTemperature3Alarm = 0,
	.highTemperature4Alarm = 0,
	.lowTemperature4Alarm = 0,
	.deltaTemperature4Alarm = 0,
	.highAnalog1Alarm = 0,
	.lowAnalog1Alarm = 0,
	.deltaAnalog1Alarm = 0,
	.highAnalog2Alarm = 0,
	.lowAnalog2Alarm = 0,
	.deltaAnalog2Alarm = 0,
	.highAnalog3Alarm = 0,
	.lowAnalog3Alarm = 0,
	.deltaAnalog3Alarm = 0,
	.highAnalog4Alarm = 0,
	.lowAnalog4Alarm = 0,
	.deltaAnalog4Alarm = 0,
	.hwVersion = "0",
	.firmwareVersion = "0",
	.resetReason = "0",
	.bluetoothAddress = "0",
	.mtu = 20,
	.flags = 0,
	.resetCount = 0,
	.digitalInput1Mv = 0,
	.digitalInput2Mv = 0,
	.magnetState = 0,
	.bootloaderVersion = "0"
	/* pyend */
};

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static RwAttribute_t rw;
static RoAttribute_t ro;

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
bool AttributeValidator_Passkey(uint32_t Index, void *pValue, size_t Length,
				bool DoWrite);
bool AttributeValidator_TxPower(uint32_t Index, void *pValue, size_t Length,
				bool DoWrite);

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/* Expander for string and other values
 *
 *...............name...value...default....size...writable..readable
 */
#define RW_ATTRS(n) STRINGIFY(n), rw.n, DRW.n, sizeof(rw.n), true, true
#define RW_ATTRX(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n), true, true
#define RO_ATTRS(n) STRINGIFY(n), ro.n, DRO.n, sizeof(ro.n), false, true
#define RO_ATTRX(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n), false, true

/* If min == max then range isn't checked. */

/* index.....name.......................................type.backup.lockable.broadcast.validator.................min...max. */
/* clang-format off */
AttributeEntry_t attrTable[ATTR_TABLE_SIZE] = {
  	/* pystart - attribute table */
    [0  ] = { RW_ATTRS(sensorName)                            , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [1  ] = { RW_ATTRS(sensorLocation)                        , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [2  ] = { RW_ATTRX(advertisingInterval)                   , i, false, false, false, AttributeValidator_uint16        , 20, 10000 },
    [3  ] = { RW_ATTRX(advertisingDuration)                   , i, false, false, false, AttributeValidator_uint16        , 0, 65535 },
    [4  ] = { RW_ATTRS(passkey)                               , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [5  ] = { RW_ATTRX(lock)                                  , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [6  ] = { RW_ATTRX(batterySenseInterval)                  , i, false, false, false, AttributeValidator_uint16        , 0, 86400 },
    [7  ] = { RW_ATTRX(temperatureSenseInterval)              , i, false, false, false, AttributeValidator_uint16        , 0, 86400 },
    [8  ] = { RW_ATTRX(temperatureAggregationCount)           , i, false, false, false, AttributeValidator_uint8         , 1, 32 },
    [9  ] = { RW_ATTRX(digitalOutput1Mv)                      , i, false, false, false, AttributeValidator_uint16        , 0, 30000 },
    [10 ] = { RW_ATTRX(digitalOutput2Mv)                      , i, false, false, false, AttributeValidator_uint16        , 0, 30000 },
    [11 ] = { RW_ATTRX(digitalInput1)                         , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [12 ] = { RW_ATTRX(digitalInput2)                         , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [13 ] = { RW_ATTRX(analogInput1Type)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [14 ] = { RW_ATTRX(analogInput2Type)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [15 ] = { RW_ATTRX(analogInput3Type)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [16 ] = { RW_ATTRX(analogInput4Type)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [17 ] = { RW_ATTRX(highTemp1Thresh1)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [18 ] = { RW_ATTRX(highTemp1Thresh2)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [19 ] = { RW_ATTRX(lowTemp1Thresh1)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [20 ] = { RW_ATTRX(lowTemp1Thresh2)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [21 ] = { RW_ATTRX(temp1DeltaThresh)                      , i, false, false, false, AttributeValidator_uint16        , 0, 255 },
    [22 ] = { RW_ATTRX(highTemp2Thresh1)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [23 ] = { RW_ATTRX(highTemp2Thresh2)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [24 ] = { RW_ATTRX(lowTemp2Thresh1)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [25 ] = { RW_ATTRX(lowTemp2Thresh2)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [26 ] = { RW_ATTRX(temp2DeltaThresh)                      , i, false, false, false, AttributeValidator_uint16        , 0, 255 },
    [27 ] = { RW_ATTRX(highTemp3Thresh1)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [28 ] = { RW_ATTRX(highTemp3Thresh2)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [29 ] = { RW_ATTRX(lowTemp3Thresh1)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [30 ] = { RW_ATTRX(lowTemp3Thresh2)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [31 ] = { RW_ATTRX(temp3DeltaThresh)                      , i, false, false, false, AttributeValidator_uint16        , 0, 255 },
    [32 ] = { RW_ATTRX(highTemp4Thresh1)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [33 ] = { RW_ATTRX(highTemp4Thresh2)                      , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [34 ] = { RW_ATTRX(lowTemp4Thresh1)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [35 ] = { RW_ATTRX(lowTemp4Thresh2)                       , i, false, false, false, AttributeValidator_int16         , (int16_t)-128, 127 },
    [36 ] = { RW_ATTRX(temp4DeltaThresh)                      , i, false, false, false, AttributeValidator_uint16        , 0, 255 },
    [37 ] = { RW_ATTRX(highAnalog1Thresh1)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [38 ] = { RW_ATTRX(highAnalog1Thresh2)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [39 ] = { RW_ATTRX(lowAnalog1Thresh1)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [40 ] = { RW_ATTRX(lowAnalog1Thresh2)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [41 ] = { RW_ATTRX(analog1DeltaThresh)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [42 ] = { RW_ATTRX(highAnalog2Thresh1)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [43 ] = { RW_ATTRX(highAnalog2Thresh2)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [44 ] = { RW_ATTRX(lowAnalog2Thresh1)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [45 ] = { RW_ATTRX(lowAnalog2Thresh2)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [46 ] = { RW_ATTRX(analog2DeltaThresh)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [47 ] = { RW_ATTRX(highAnalog3Thresh1)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [48 ] = { RW_ATTRX(highAnalog3Thresh2)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [49 ] = { RW_ATTRX(lowAnalog3Thresh1)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [50 ] = { RW_ATTRX(lowAnalog3Thresh2)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [51 ] = { RW_ATTRX(analog3DeltaThresh)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [52 ] = { RW_ATTRX(highAnalog4Thresh1)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [53 ] = { RW_ATTRX(highAnalog4Thresh2)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [54 ] = { RW_ATTRX(lowAnalog4Thresh1)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [55 ] = { RW_ATTRX(lowAnalog4Thresh2)                     , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [56 ] = { RW_ATTRX(analog4DeltaThresh)                    , i, false, false, false, AttributeValidator_uint16        , 0, 4096 },
    [57 ] = { RW_ATTRX(activeMode)                            , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [58 ] = { RW_ATTRX(useCodedPhy)                           , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [59 ] = { RW_ATTRX(txPower)                               , i, false, false, false, AttributeValidator_int8          , (int8_t)-40, 8 },
    [60 ] = { RW_ATTRX(networkId)                             , i, false, false, false, AttributeValidator_uint16        , 0, 65535 },
    [61 ] = { RW_ATTRX(configVersion)                         , i, false, false, false, AttributeValidator_uint8         , 0, 255 },
    [62 ] = { RW_ATTRX(configType)                            , i, false, false, false, AttributeValidator_uint8         , 0, 255 },
    [63 ] = { RW_ATTRX(hardwareMinorVersion)                  , i, false, false, false, AttributeValidator_uint8         , 0, 9 },
    [64 ] = { RW_ATTRX(ledTestActive)                         , i, false, false, false, AttributeValidator_uint8         , 10, 1000 },
    [65 ] = { RW_ATTRX(coefficient1)                          , f, false, false, false, AttributeValidator_float         , (float)0.0, (float)1000.0 },
    [66 ] = { RW_ATTRX(coefficient2)                          , f, false, false, false, AttributeValidator_float         , (float)0.0, (float)10000.0 },
    [67 ] = { RW_ATTRX(coefficientA)                          , f, false, false, false, AttributeValidator_float         , (float)0.0, (float)1000.0 },
    [68 ] = { RW_ATTRX(coefficientB)                          , f, false, false, false, AttributeValidator_float         , (float)0.0, (float)1000.0 },
    [69 ] = { RW_ATTRX(coefficientC)                          , f, false, false, false, AttributeValidator_float         , (float)0.0, (float)10000.0 },
    [70 ] = { RW_ATTRX(thermistorIndex)                       , i, false, false, false, AttributeValidator_uint8         , 0, 3 },
    [71 ] = { RO_ATTRX(temperatureResult1)                    , i, false, false, false, AttributeValidator_int32         , (int32_t)-128000, 127000 },
    [72 ] = { RO_ATTRX(temperatureResult2)                    , i, false, false, false, AttributeValidator_int32         , (int32_t)-128000, 127000 },
    [73 ] = { RO_ATTRX(temperatureResult3)                    , i, false, false, false, AttributeValidator_int32         , (int32_t)-128000, 127000 },
    [74 ] = { RO_ATTRX(temperatureResult4)                    , i, false, false, false, AttributeValidator_int32         , (int32_t)-128000, 127000 },
    [75 ] = { RO_ATTRX(batteryVoltageMv)                      , i, false, false, false, AttributeValidator_uint16        , 0, 3800 },
    [76 ] = { RO_ATTRX(digitalInput1Alarm)                    , i, false, false, false, AttributeValidator_uint8         , 0, 0 },
    [77 ] = { RO_ATTRX(digitalInput2Alarm)                    , i, false, false, false, AttributeValidator_uint8         , 0, 0 },
    [78 ] = { RO_ATTRX(currentReadingMa)                      , i, false, false, false, AttributeValidator_uint8         , 0, 20 },
    [79 ] = { RO_ATTRX(highTemperature1Alarm)                 , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [80 ] = { RO_ATTRX(lowTemperature1Alarm)                  , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [81 ] = { RO_ATTRX(deltaTemperature1Alarm)                , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [82 ] = { RO_ATTRX(highTemperature2Alarm)                 , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [83 ] = { RO_ATTRX(lowTemperature2Alarm)                  , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [84 ] = { RO_ATTRX(deltaTemperature2Alarm)                , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [85 ] = { RO_ATTRX(highTemperature3Alarm)                 , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [86 ] = { RO_ATTRX(lowTemperature3Alarm)                  , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [87 ] = { RO_ATTRX(deltaTemperature3Alarm)                , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [88 ] = { RO_ATTRX(highTemperature4Alarm)                 , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [89 ] = { RO_ATTRX(lowTemperature4Alarm)                  , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [90 ] = { RO_ATTRX(deltaTemperature4Alarm)                , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [91 ] = { RO_ATTRX(highAnalog1Alarm)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [92 ] = { RO_ATTRX(lowAnalog1Alarm)                       , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [93 ] = { RO_ATTRX(deltaAnalog1Alarm)                     , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [94 ] = { RO_ATTRX(highAnalog2Alarm)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [95 ] = { RO_ATTRX(lowAnalog2Alarm)                       , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [96 ] = { RO_ATTRX(deltaAnalog2Alarm)                     , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [97 ] = { RO_ATTRX(highAnalog3Alarm)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [98 ] = { RO_ATTRX(lowAnalog3Alarm)                       , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [99 ] = { RO_ATTRX(deltaAnalog3Alarm)                     , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [100] = { RO_ATTRX(highAnalog4Alarm)                      , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [101] = { RO_ATTRX(lowAnalog4Alarm)                       , i, false, false, false, AttributeValidator_uint8         , 0, 2 },
    [102] = { RO_ATTRX(deltaAnalog4Alarm)                     , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [103] = { RO_ATTRS(hwVersion)                             , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [104] = { RO_ATTRS(firmwareVersion)                       , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [105] = { RO_ATTRS(resetReason)                           , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [106] = { RO_ATTRS(bluetoothAddress)                      , s, false, false, false, AttributeValidator_string        , 0, 0 },
    [107] = { RO_ATTRX(mtu)                                   , i, false, false, false, AttributeValidator_uint8         , 20, 244 },
    [108] = { RO_ATTRX(flags)                                 , i, false, false, false, AttributeValidator_uint32        , 0, 65535 },
    [109] = { RO_ATTRX(resetCount)                            , i, false, false, false, AttributeValidator_uint16        , 0, 65535 },
    [110] = { RO_ATTRX(digitalInput1Mv)                       , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [111] = { RO_ATTRX(digitalInput2Mv)                       , i, false, false, false, AttributeValidator_uint8         , 0, 1 },
    [112] = { RO_ATTRX(magnetState)                           , i, false, false, false, AttributeValidator_uint8         , 0, 255 },
    [113] = { RO_ATTRS(bootloaderVersion)                     , s, false, false, false, AttributeValidator_string        , 0, 0 }
	/* pyend */
};
/* clang-format on */

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void AttributeTable_Initialize(void)
{
	memcpy(&rw, &DRW, sizeof(RwAttribute_t));
	memcpy(&ro, &DRO, sizeof(RoAttribute_t));
}

/* set non-backup values to default */
void AttributeTable_FactoryReset(void)
{
	size_t i = 0;
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (!attrTable[i].backup) {
			memcpy(attrTable[i].pData, attrTable[i].pDefault,
			       attrTable[i].size);
		}
	}
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
#ifdef STILL_WORKING_ON
bool AttributeValidator_Passkey(uint32_t Index, void *pValue, size_t Length,
				bool DoWrite)
{
	AttributeEntry_t *pEntry = &attrTable[Index];
	char *p = (char *)pValue;

	if (Length == (PASSKEY_LENGTH - 1)) // -1 to account for NUL
	{
		size_t i;
		for (i = 0; i < Length; i++) {
			if ((p[i] < '0') || (p[i] > '9')) {
				return false;
			}
		}

		// Don't use strncmp because pValue isn't NUL terminated when coming from JSON
		if (DoWrite && ((memcmp(pEntry->pData, pValue, Length) != 0))) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, pValue, Length);
		}
		return true;
	}
	return false;
}

bool AttributeValidator_TxPower(uint32_t Index, void *pValue, size_t Length,
				bool DoWrite)
{
	UNUSED_PARAMETER(Length);
	AttributeEntry_t *pEntry = &attrTable[Index];
	int32_t value = *((int32_t *)pValue);

	// Values supported by nRF52840
	bool valid = false;
	switch (value) {
	case -40:
	case -20:
	case -16:
	case -12:
	case -8:
	case -4:
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		valid = true;
		break;

	default:
		valid = false;
		break;
	}

	if (valid) {
		if (DoWrite && value != *((int32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		return true;
	}
	return false;
}
#endif
