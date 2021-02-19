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

/* clang-format off */
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
	char sensorName[23 + 1];
	char sensorLocation[32 + 1];
	uint16_t advertisingInterval;
	uint16_t advertisingDuration;
	uint32_t passkey;
	uint8_t lock;
	uint32_t batterySenseInterval;
	uint32_t temperatureSenseInterval;
	uint8_t AggregationCount;
	uint8_t digitalOutput1Enable;
	uint8_t digitalOutput2Enable;
	float highTemp1Thresh1;
	float highTemp1Thresh2;
	float lowTemp1Thresh1;
	float lowTemp1Thresh2;
	float temp1DeltaThresh;
	float highTemp2Thresh1;
	float highTemp2Thresh2;
	float lowTemp2Thresh1;
	float lowTemp2Thresh2;
	float temp2DeltaThresh;
	float highTemp3Thresh1;
	float highTemp3Thresh2;
	float lowTemp3Thresh1;
	float lowTemp3Thresh2;
	float temp3DeltaThresh;
	float highTemp4Thresh1;
	float highTemp4Thresh2;
	float lowTemp4Thresh1;
	float lowTemp4Thresh2;
	float temp4DeltaThresh;
	float highAnalog1Thresh1;
	float highAnalog1Thresh2;
	float lowAnalog1Thresh1;
	float lowAnalog1Thresh2;
	float analog1DeltaThresh;
	float highAnalog2Thresh1;
	float highAnalog2Thresh2;
	float lowAnalog2Thresh1;
	float lowAnalog2Thresh2;
	float analog2DeltaThresh;
	float highAnalog3Thresh1;
	float highAnalog3Thresh2;
	float lowAnalog3Thresh1;
	float lowAnalog3Thresh2;
	float analog3DeltaThresh;
	float highAnalog4Thresh1;
	float highAnalog4Thresh2;
	float lowAnalog4Thresh1;
	float lowAnalog4Thresh2;
	float analog4DeltaThresh;
	uint8_t activeMode;
	uint8_t useCodedPhy;
	int8_t txPower;
	uint16_t networkId;
	uint8_t configVersion;
	uint8_t configType;
	uint8_t hardwareMinorVersion;
	float coefficientA;
	float coefficientB;
	float coefficientC;
	uint8_t thermistorConfig;
	uint32_t temperatureAlarms;
	uint32_t digitalAlarms;
	uint8_t digitalInput1Config;
	uint8_t digitalInput2Config;
	uint32_t analogAlarms;
	uint8_t analogInput1Type;
	uint8_t analogInput2Type;
	uint8_t analogInput3Type;
	uint8_t analogInput4Type;
	uint32_t flags;
	uint32_t qrtcLastSet;
	float shOffset;
	uint32_t analogSenseInterval;
	/* pyend */
} RwAttribute_t;

static const RwAttribute_t DEFAULT_RW_ATTRIBUTE_VALUES = {
	/* pystart - rw defaults */
	.sensorName = "BT610",
	.sensorLocation = "",
	.advertisingInterval = 1000,
	.advertisingDuration = 0,
	.passkey = 123456,
	.lock = 0,
	.batterySenseInterval = 0,
	.temperatureSenseInterval = 0,
	.AggregationCount = 1,
	.digitalOutput1Enable = 0,
	.digitalOutput2Enable = 0,
	.highTemp1Thresh1 = 127,
	.highTemp1Thresh2 = 127,
	.lowTemp1Thresh1 = -127,
	.lowTemp1Thresh2 = -127,
	.temp1DeltaThresh = -1,
	.highTemp2Thresh1 = 127,
	.highTemp2Thresh2 = 127,
	.lowTemp2Thresh1 = -127,
	.lowTemp2Thresh2 = -127,
	.temp2DeltaThresh = -1,
	.highTemp3Thresh1 = 127,
	.highTemp3Thresh2 = 127,
	.lowTemp3Thresh1 = -127,
	.lowTemp3Thresh2 = -127,
	.temp3DeltaThresh = -1,
	.highTemp4Thresh1 = 127,
	.highTemp4Thresh2 = 127,
	.lowTemp4Thresh1 = -127,
	.lowTemp4Thresh2 = -127,
	.temp4DeltaThresh = -1,
	.highAnalog1Thresh1 = 4095,
	.highAnalog1Thresh2 = 4095,
	.lowAnalog1Thresh1 = 4095,
	.lowAnalog1Thresh2 = 4095,
	.analog1DeltaThresh = 4095,
	.highAnalog2Thresh1 = 4095,
	.highAnalog2Thresh2 = 4095,
	.lowAnalog2Thresh1 = 4095,
	.lowAnalog2Thresh2 = 4095,
	.analog2DeltaThresh = 4095,
	.highAnalog3Thresh1 = 4095,
	.highAnalog3Thresh2 = 4095,
	.lowAnalog3Thresh1 = 4095,
	.lowAnalog3Thresh2 = 4095,
	.analog3DeltaThresh = 4095,
	.highAnalog4Thresh1 = 4095,
	.highAnalog4Thresh2 = 4095,
	.lowAnalog4Thresh1 = 4095,
	.lowAnalog4Thresh2 = 4095,
	.analog4DeltaThresh = 4095,
	.activeMode = 0,
	.useCodedPhy = 0,
	.txPower = 0,
	.networkId = 0,
	.configVersion = 0,
	.configType = 0,
	.hardwareMinorVersion = 0,
	.coefficientA = 0.001132,
	.coefficientB = 0.0002338,
	.coefficientC = 8.780e-8,
	.thermistorConfig = 0,
	.temperatureAlarms = 0,
	.digitalAlarms = 0,
	.digitalInput1Config = 131,
	.digitalInput2Config = 131,
	.analogAlarms = 0,
	.analogInput1Type = 0,
	.analogInput2Type = 0,
	.analogInput3Type = 0,
	.analogInput4Type = 0,
	.flags = 0,
	.qrtcLastSet = 0,
	.shOffset = 273.15,
	.analogSenseInterval = 0
	/* pyend */
};

typedef struct RoAttributesTag {
	/* pystart - ro attributes */
	char firmwareVersion[11 + 1];
	char resetReason[8 + 1];
	char bluetoothAddress[12 + 1];
	uint32_t resetCount;
	char bootloaderVersion[11 + 1];
	int64_t upTime;
	float ge;
	float oe;
	float temperatureResult1;
	float temperatureResult2;
	float temperatureResult3;
	float temperatureResult4;
	uint32_t temperatureAlarms;
	uint16_t batteryVoltageMv;
	uint8_t digitalInput;
	uint32_t digitalAlarms;
	float analogInput1;
	float analogInput2;
	float analogInput3;
	float analogInput4;
	uint32_t analogAlarms;
	uint32_t flags;
	uint8_t magnetState;
	char paramPath[8 + 1];
	uint32_t batteryAge;
	char apiVersion[11 + 1];
	uint32_t qrtc;
	uint8_t tamperSwitchStatus;
	/* pyend */
} RoAttribute_t;

static const RoAttribute_t DEFAULT_RO_ATTRIBUTE_VALUES = {
	/* pystart - ro defaults */
	.firmwareVersion = "0.0.0",
	.resetReason = "0",
	.bluetoothAddress = "0",
	.resetCount = 0,
	.bootloaderVersion = "0.0",
	.upTime = 0,
	.ge = 0.0,
	.oe = 0.0,
	.temperatureResult1 = 0,
	.temperatureResult2 = 0,
	.temperatureResult3 = 0,
	.temperatureResult4 = 0,
	.temperatureAlarms = 0,
	.batteryVoltageMv = 0,
	.digitalInput = 0,
	.digitalAlarms = 0,
	.analogInput1 = 0,
	.analogInput2 = 0,
	.analogInput3 = 0,
	.analogInput4 = 0,
	.analogAlarms = 0,
	.flags = 0,
	.magnetState = 0,
	.paramPath = "/ext",
	.batteryAge = 0,
	.apiVersion = "1.20",
	.qrtc = 0,
	.tamperSwitchStatus = 0
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
#define RW_ATTRS(n) STRINGIFY(n), rw.n, DRW.n, sizeof(rw.n)
#define RW_ATTRX(n) STRINGIFY(n), &rw.n, &DRW.n, sizeof(rw.n)
#define RO_ATTRS(n) STRINGIFY(n), ro.n, DRO.n, sizeof(ro.n)
#define RO_ATTRX(n) STRINGIFY(n), &ro.n, &DRO.n, sizeof(ro.n)

#define y true
#define n false

/* If min == max then range isn't checked. */

/* index.....name............................type.savable.writable.readable.lockable.broadcast.validator..min.max. */
/* clang-format off */
AttributeEntry_t attrTable[ATTR_TABLE_SIZE] = {
    /* pystart - attribute table */
    [0  ] = { RW_ATTRS(sensorName)                    , s  , y, y, y, n, y, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [1  ] = { RW_ATTRS(sensorLocation)                , s  , y, y, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [2  ] = { RW_ATTRX(advertisingInterval)           , u16, y, y, y, n, y, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 20.0      , .max.ux = 10000.0    },
    [3  ] = { RW_ATTRX(advertisingDuration)           , u16, y, y, y, n, y, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 0.0       , .max.ux = 65535.0    },
    [4  ] = { RW_ATTRX(passkey)                       , u32, y, y, y, n, y, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [5  ] = { RW_ATTRX(lock)                          , u8 , y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [6  ] = { RW_ATTRX(batterySenseInterval)          , u32, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [7  ] = { RW_ATTRX(temperatureSenseInterval)      , u32, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [8  ] = { RW_ATTRX(AggregationCount)              , u8 , y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 1.0       , .max.ux = 32.0       },
    [9  ] = { RW_ATTRX(digitalOutput1Enable)          , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [10 ] = { RW_ATTRX(digitalOutput2Enable)          , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [11 ] = { RO_ATTRS(firmwareVersion)               , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [12 ] = { RO_ATTRS(resetReason)                   , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [13 ] = { RO_ATTRS(bluetoothAddress)              , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [14 ] = { RO_ATTRX(resetCount)                    , u32, n, n, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [15 ] = { RO_ATTRS(bootloaderVersion)             , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [16 ] = { RO_ATTRX(upTime)                        , i64, n, n, y, n, n, n, AttributeValidator_int64    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [17 ] = { RW_ATTRX(highTemp1Thresh1)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [18 ] = { RW_ATTRX(highTemp1Thresh2)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [19 ] = { RW_ATTRX(lowTemp1Thresh1)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [20 ] = { RW_ATTRX(lowTemp1Thresh2)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [21 ] = { RW_ATTRX(temp1DeltaThresh)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [22 ] = { RW_ATTRX(highTemp2Thresh1)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [23 ] = { RW_ATTRX(highTemp2Thresh2)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [24 ] = { RW_ATTRX(lowTemp2Thresh1)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [25 ] = { RW_ATTRX(lowTemp2Thresh2)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [26 ] = { RW_ATTRX(temp2DeltaThresh)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [27 ] = { RW_ATTRX(highTemp3Thresh1)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [28 ] = { RW_ATTRX(highTemp3Thresh2)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [29 ] = { RW_ATTRX(lowTemp3Thresh1)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [30 ] = { RW_ATTRX(lowTemp3Thresh2)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [31 ] = { RW_ATTRX(temp3DeltaThresh)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [32 ] = { RW_ATTRX(highTemp4Thresh1)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [33 ] = { RW_ATTRX(highTemp4Thresh2)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [34 ] = { RW_ATTRX(lowTemp4Thresh1)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [35 ] = { RW_ATTRX(lowTemp4Thresh2)               , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [36 ] = { RW_ATTRX(temp4DeltaThresh)              , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [37 ] = { RW_ATTRX(highAnalog1Thresh1)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [38 ] = { RW_ATTRX(highAnalog1Thresh2)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [39 ] = { RW_ATTRX(lowAnalog1Thresh1)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [40 ] = { RW_ATTRX(lowAnalog1Thresh2)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [41 ] = { RW_ATTRX(analog1DeltaThresh)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [42 ] = { RW_ATTRX(highAnalog2Thresh1)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [43 ] = { RW_ATTRX(highAnalog2Thresh2)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [44 ] = { RW_ATTRX(lowAnalog2Thresh1)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [45 ] = { RW_ATTRX(lowAnalog2Thresh2)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [46 ] = { RW_ATTRX(analog2DeltaThresh)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [47 ] = { RW_ATTRX(highAnalog3Thresh1)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [48 ] = { RW_ATTRX(highAnalog3Thresh2)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [49 ] = { RW_ATTRX(lowAnalog3Thresh1)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [50 ] = { RW_ATTRX(lowAnalog3Thresh2)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [51 ] = { RW_ATTRX(analog3DeltaThresh)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [52 ] = { RW_ATTRX(highAnalog4Thresh1)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [53 ] = { RW_ATTRX(highAnalog4Thresh2)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [54 ] = { RW_ATTRX(lowAnalog4Thresh1)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [55 ] = { RW_ATTRX(lowAnalog4Thresh2)             , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [56 ] = { RW_ATTRX(analog4DeltaThresh)            , f  , y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [57 ] = { RW_ATTRX(activeMode)                    , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [58 ] = { RW_ATTRX(useCodedPhy)                   , u8 , y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [59 ] = { RW_ATTRX(txPower)                       , i8 , y, y, y, n, y, n, AttributeValidator_int8     , NULL                                      , .min.sx = -40.0     , .max.sx = 8.0        },
    [60 ] = { RW_ATTRX(networkId)                     , u16, y, y, y, n, y, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 0.0       , .max.ux = 65535.0    },
    [61 ] = { RW_ATTRX(configVersion)                 , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
    [62 ] = { RW_ATTRX(configType)                    , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
    [63 ] = { RW_ATTRX(hardwareMinorVersion)          , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 9.0        },
    [64 ] = { RO_ATTRX(ge)                            , f  , n, n, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [65 ] = { RO_ATTRX(oe)                            , f  , n, n, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [66 ] = { RW_ATTRX(coefficientA)                  , f  , y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [67 ] = { RW_ATTRX(coefficientB)                  , f  , y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [68 ] = { RW_ATTRX(coefficientC)                  , f  , y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [69 ] = { RW_ATTRX(thermistorConfig)              , u8 , y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 15.0       },
    [70 ] = { RO_ATTRX(temperatureResult1)            , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult1       , .min.fx = -175.0    , .max.fx = 175.0      },
    [71 ] = { RO_ATTRX(temperatureResult2)            , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult2       , .min.fx = -175.0    , .max.fx = 175.0      },
    [72 ] = { RO_ATTRX(temperatureResult3)            , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult3       , .min.fx = -175.0    , .max.fx = 175.0      },
    [73 ] = { RO_ATTRX(temperatureResult4)            , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult4       , .min.fx = -175.0    , .max.fx = 175.0      },
    [74 ] = { RO_ATTRX(temperatureAlarms)             , u32, n, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [75 ] = { RO_ATTRX(batteryVoltageMv)              , u16, n, n, y, n, n, n, AttributeValidator_uint16   , AttributePrepare_batteryVoltageMv         , .min.ux = 0.0       , .max.ux = 3800.0     },
    [76 ] = { RO_ATTRX(digitalInput)                  , u8 , n, n, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 3.0        },
    [77 ] = { RO_ATTRX(digitalAlarms)                 , u32, n, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 3.0        },
    [78 ] = { RW_ATTRX(digitalInput1Config)           , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [79 ] = { RW_ATTRX(digitalInput2Config)           , u8 , y, y, y, n, y, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [80 ] = { RO_ATTRX(analogInput1)                  , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput1             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [81 ] = { RO_ATTRX(analogInput2)                  , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput2             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [82 ] = { RO_ATTRX(analogInput3)                  , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput3             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [83 ] = { RO_ATTRX(analogInput4)                  , f  , n, n, y, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput4             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [84 ] = { RO_ATTRX(analogAlarms)                  , u32, n, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [85 ] = { RW_ATTRX(analogInput1Type)              , u8 , y, y, y, n, y, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 4.0        },
    [86 ] = { RW_ATTRX(analogInput2Type)              , u8 , y, y, y, n, y, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 4.0        },
    [87 ] = { RW_ATTRX(analogInput3Type)              , u8 , y, y, y, n, y, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 4.0        },
    [88 ] = { RW_ATTRX(analogInput4Type)              , u8 , y, y, y, n, y, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 4.0        },
    [89 ] = { RO_ATTRX(flags)                         , u32, n, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [90 ] = { RO_ATTRX(magnetState)                   , u8 , n, n, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [91 ] = { RO_ATTRS(paramPath)                     , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [92 ] = { RO_ATTRX(batteryAge)                    , u32, n, n, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [93 ] = { RO_ATTRS(apiVersion)                    , s  , n, n, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [94 ] = { RO_ATTRX(qrtc)                          , u32, n, n, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [95 ] = { RW_ATTRX(qrtcLastSet)                   , u32, y, n, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [96 ] = { RW_ATTRX(shOffset)                      , f  , y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [97 ] = { RW_ATTRX(analogSenseInterval)           , u32, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [98 ] = { RO_ATTRX(tamperSwitchStatus)            , u8 , n, n, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        }
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

		/* Don't use strncmp because pValue may not be NUL terminated */
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

/* pystart - prepare for read weak implementations */
__weak int AttributePrepare_temperatureResult1(void)
{
	return 0;
}

__weak int AttributePrepare_temperatureResult2(void)
{
	return 0;
}

__weak int AttributePrepare_temperatureResult3(void)
{
	return 0;
}

__weak int AttributePrepare_temperatureResult4(void)
{
	return 0;
}

__weak int AttributePrepare_batteryVoltageMv(void)
{
	return 0;
}

__weak int AttributePrepare_analogInput1(void)
{
	return 0;
}

__weak int AttributePrepare_analogInput2(void)
{
	return 0;
}

__weak int AttributePrepare_analogInput3(void)
{
	return 0;
}

__weak int AttributePrepare_analogInput4(void)
{
	return 0;
}

/* pyend */