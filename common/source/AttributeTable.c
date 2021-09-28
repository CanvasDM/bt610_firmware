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
	char sensorName[23 + 1];
	char sensorLocation[32 + 1];
	uint16_t advertisingInterval;
	uint16_t advertisingDuration;
	uint32_t passkey;
	bool lock;
	uint32_t batterySenseInterval;
	uint32_t temperatureSenseInterval;
	uint8_t AggregationCount;
	bool digitalOutput1State;
	bool digitalOutput2State;
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
	bool activeMode;
	bool useCodedPhy;
	uint8_t txPower;
	uint16_t networkId;
	uint8_t configVersion;
	uint8_t configType;
	uint8_t hardwareMinorVersion;
	float oldCoefficientA;
	float oldCoefficientB;
	float oldCoefficientC;
	uint8_t thermistorConfig;
	uint32_t temperatureAlarms;
	uint8_t digitalAlarms;
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
	bool tamperSwitchStatus;
	uint8_t connectionTimeoutSec;
	uint32_t settingsPasscode;
	float therm1CoefficientA;
	float therm2CoefficientA;
	float therm3CoefficientA;
	float therm4CoefficientA;
	float therm1CoefficientB;
	float therm2CoefficientB;
	float therm3CoefficientB;
	float therm4CoefficientB;
	float therm1CoefficientC;
	float therm2CoefficientC;
	float therm3CoefficientC;
	float therm4CoefficientC;
	bool dataloggingEnable;
	bool factoryResetEnable;
	uint32_t temperatureAlarmsEnable;
	uint32_t analogAlarmsEnable;
	bool adcBatterySimulated;
	int16_t adcBatterySimulatedCounts;
	bool adcAnalogSensorSimulated;
	int16_t adcAnalogSensorSimulatedCounts;
	bool adcThermistorSimulated;
	int16_t adcThermistorSimulatedCounts;
	bool adcVRefSimulated;
	int16_t adcVRefSimulatedCounts;
	bool voltage1Simulated;
	float voltage1SimulatedValue;
	bool voltage2Simulated;
	float voltage2SimulatedValue;
	bool voltage3Simulated;
	float voltage3SimulatedValue;
	bool voltage4Simulated;
	float voltage4SimulatedValue;
	bool ultrasonicSimulated;
	float ultrasonicSimulatedValue;
	bool pressureSimulated;
	float pressureSimulatedValue;
	bool current1Simulated;
	float current1SimulatedValue;
	bool current2Simulated;
	float current2SimulatedValue;
	bool current3Simulated;
	float current3SimulatedValue;
	bool current4Simulated;
	float current4SimulatedValue;
	bool vrefSimulated;
	float vrefSimulatedValue;
	bool temperature1Simulated;
	float temperature1SimulatedValue;
	bool temperature2Simulated;
	float temperature2SimulatedValue;
	bool temperature3Simulated;
	float temperature3SimulatedValue;
	bool temperature4Simulated;
	float temperature4SimulatedValue;
	bool batterymvSimulated;
	int32_t batterymvSimulatedValue;
	bool digitalInput1Simulated;
	bool digitalInput1SimulatedValue;
	bool digitalInput2Simulated;
	bool digitalInput2SimulatedValue;
	bool magSwitchSimulated;
	bool magSwitchSimulatedValue;
	bool tamperSwitchSimulated;
	bool tamperSwitchSimulatedValue;
	uint8_t bootPHY;
	bool mobileAppDisconnect;
	/* pyend */
} RwAttribute_t;

static const RwAttribute_t DEFAULT_RW_ATTRIBUTE_VALUES = {
	/* pystart - rw defaults */
	.sensorName = "BT610",
	.sensorLocation = "",
	.advertisingInterval = 1000,
	.advertisingDuration = 15000,
	.passkey = 123456,
	.lock = 0,
	.batterySenseInterval = 0,
	.temperatureSenseInterval = 60,
	.AggregationCount = 1,
	.digitalOutput1State = 0,
	.digitalOutput2State = 0,
	.highTemp1Thresh1 = 1.27e+2,
	.highTemp1Thresh2 = 1.27e+2,
	.lowTemp1Thresh1 = -1.28e+2,
	.lowTemp1Thresh2 = -1.28e+2,
	.temp1DeltaThresh = -1.0e+0,
	.highTemp2Thresh1 = 1.27e+2,
	.highTemp2Thresh2 = 1.27e+2,
	.lowTemp2Thresh1 = -1.28e+2,
	.lowTemp2Thresh2 = -1.28e+2,
	.temp2DeltaThresh = -1.0e+0,
	.highTemp3Thresh1 = 1.27e+2,
	.highTemp3Thresh2 = 1.27e+2,
	.lowTemp3Thresh1 = -1.28e+2,
	.lowTemp3Thresh2 = -1.28e+2,
	.temp3DeltaThresh = -1.0e+0,
	.highTemp4Thresh1 = 1.27e+2,
	.highTemp4Thresh2 = 1.27e+2,
	.lowTemp4Thresh1 = -1.28e+2,
	.lowTemp4Thresh2 = -1.28e+2,
	.temp4DeltaThresh = -1.0e+0,
	.highAnalog1Thresh1 = 4.095e+3,
	.highAnalog1Thresh2 = 4.095e+3,
	.lowAnalog1Thresh1 = 4.095e+3,
	.lowAnalog1Thresh2 = 4.095e+3,
	.analog1DeltaThresh = 4.095e+3,
	.highAnalog2Thresh1 = 4.095e+3,
	.highAnalog2Thresh2 = 4.095e+3,
	.lowAnalog2Thresh1 = 4.095e+3,
	.lowAnalog2Thresh2 = 4.095e+3,
	.analog2DeltaThresh = 4.095e+3,
	.highAnalog3Thresh1 = 4.095e+3,
	.highAnalog3Thresh2 = 4.095e+3,
	.lowAnalog3Thresh1 = 4.095e+3,
	.lowAnalog3Thresh2 = 4.095e+3,
	.analog3DeltaThresh = 4.095e+3,
	.highAnalog4Thresh1 = 4.095e+3,
	.highAnalog4Thresh2 = 4.095e+3,
	.lowAnalog4Thresh1 = 4.095e+3,
	.lowAnalog4Thresh2 = 4.095e+3,
	.analog4DeltaThresh = 4.095e+3,
	.activeMode = 0,
	.useCodedPhy = 1,
	.txPower = 0,
	.networkId = 0,
	.configVersion = 0,
	.configType = 0,
	.hardwareMinorVersion = 0,
	.oldCoefficientA = 1.132e-3,
	.oldCoefficientB = 2.338e-4,
	.oldCoefficientC = 8.780e-8,
	.thermistorConfig = 0,
	.temperatureAlarms = 0,
	.digitalAlarms = 0,
	.digitalInput1Config = 0,
	.digitalInput2Config = 0,
	.analogAlarms = 0,
	.analogInput1Type = 0,
	.analogInput2Type = 0,
	.analogInput3Type = 0,
	.analogInput4Type = 0,
	.flags = 0,
	.qrtcLastSet = 0,
	.shOffset = 2.7315e+2,
	.analogSenseInterval = 60,
	.tamperSwitchStatus = 0,
	.connectionTimeoutSec = 60,
	.settingsPasscode = 123456,
	.therm1CoefficientA = 1.132e-3,
	.therm2CoefficientA = 1.132e-3,
	.therm3CoefficientA = 1.132e-3,
	.therm4CoefficientA = 1.132e-3,
	.therm1CoefficientB = 2.338e-4,
	.therm2CoefficientB = 2.338e-4,
	.therm3CoefficientB = 2.338e-4,
	.therm4CoefficientB = 2.338e-4,
	.therm1CoefficientC = 8.780e-8,
	.therm2CoefficientC = 8.780e-8,
	.therm3CoefficientC = 8.780e-8,
	.therm4CoefficientC = 8.780e-8,
	.dataloggingEnable = 0,
	.factoryResetEnable = 1,
	.temperatureAlarmsEnable = 0,
	.analogAlarmsEnable = 0,
	.adcBatterySimulated = 0,
	.adcBatterySimulatedCounts = 0,
	.adcAnalogSensorSimulated = 0,
	.adcAnalogSensorSimulatedCounts = 0,
	.adcThermistorSimulated = 0,
	.adcThermistorSimulatedCounts = 0,
	.adcVRefSimulated = 0,
	.adcVRefSimulatedCounts = 0,
	.voltage1Simulated = 0,
	.voltage1SimulatedValue = 0.0,
	.voltage2Simulated = 0,
	.voltage2SimulatedValue = 0.0,
	.voltage3Simulated = 0,
	.voltage3SimulatedValue = 0.0,
	.voltage4Simulated = 0,
	.voltage4SimulatedValue = 0.0,
	.ultrasonicSimulated = 0,
	.ultrasonicSimulatedValue = 0.0,
	.pressureSimulated = 0,
	.pressureSimulatedValue = 0.0,
	.current1Simulated = 0,
	.current1SimulatedValue = 0.0,
	.current2Simulated = 0,
	.current2SimulatedValue = 0.0,
	.current3Simulated = 0,
	.current3SimulatedValue = 0.0,
	.current4Simulated = 0,
	.current4SimulatedValue = 0.0,
	.vrefSimulated = 0,
	.vrefSimulatedValue = 0.0,
	.temperature1Simulated = 0,
	.temperature1SimulatedValue = 0.0,
	.temperature2Simulated = 0,
	.temperature2SimulatedValue = 0.0,
	.temperature3Simulated = 0,
	.temperature3SimulatedValue = 0.0,
	.temperature4Simulated = 0,
	.temperature4SimulatedValue = 0.0,
	.batterymvSimulated = 0,
	.batterymvSimulatedValue = 0,
	.digitalInput1Simulated = 0,
	.digitalInput1SimulatedValue = 0,
	.digitalInput2Simulated = 0,
	.digitalInput2SimulatedValue = 0,
	.magSwitchSimulated = 0,
	.magSwitchSimulatedValue = 0,
	.tamperSwitchSimulated = 0,
	.tamperSwitchSimulatedValue = 0,
	.bootPHY = 0,
	.mobileAppDisconnect = 0
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
	float analogInput1;
	float analogInput2;
	float analogInput3;
	float analogInput4;
	uint32_t flags;
	bool magnetState;
	char paramPath[8 + 1];
	uint32_t batteryAge;
	char apiVersion[11 + 1];
	uint32_t qrtc;
	uint8_t connectionTimeoutSec;
	uint8_t logFileStatus;
	bool adcBatterySimulated;
	int16_t adcBatterySimulatedCounts;
	bool adcAnalogSensorSimulated;
	int16_t adcAnalogSensorSimulatedCounts;
	bool adcThermistorSimulated;
	int16_t adcThermistorSimulatedCounts;
	bool adcVRefSimulated;
	int16_t adcVRefSimulatedCounts;
	bool voltage1Simulated;
	float voltage1SimulatedValue;
	bool voltage2Simulated;
	float voltage2SimulatedValue;
	bool voltage3Simulated;
	float voltage3SimulatedValue;
	bool voltage4Simulated;
	float voltage4SimulatedValue;
	bool ultrasonicSimulated;
	float ultrasonicSimulatedValue;
	bool pressureSimulated;
	float pressureSimulatedValue;
	bool current1Simulated;
	float current1SimulatedValue;
	bool current2Simulated;
	float current2SimulatedValue;
	bool current3Simulated;
	float current3SimulatedValue;
	bool current4Simulated;
	float current4SimulatedValue;
	bool vrefSimulated;
	float vrefSimulatedValue;
	bool temperature1Simulated;
	float temperature1SimulatedValue;
	bool temperature2Simulated;
	float temperature2SimulatedValue;
	bool temperature3Simulated;
	float temperature3SimulatedValue;
	bool temperature4Simulated;
	float temperature4SimulatedValue;
	bool batterymvSimulated;
	int32_t batterymvSimulatedValue;
	bool digitalInput1Simulated;
	bool digitalInput1SimulatedValue;
	bool digitalInput2Simulated;
	bool digitalInput2SimulatedValue;
	bool magSwitchSimulated;
	bool magSwitchSimulatedValue;
	bool tamperSwitchSimulated;
	bool tamperSwitchSimulatedValue;
	bool mobileAppDisconnect;
	int32_t attrSaveErrorCode;
	uint8_t settingsPasscodeStatus;
	uint8_t recoverSettingsCount;
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
	.ge = 1e+0,
	.oe = 0.0,
	.temperatureResult1 = 0,
	.temperatureResult2 = 0,
	.temperatureResult3 = 0,
	.temperatureResult4 = 0,
	.temperatureAlarms = 0,
	.batteryVoltageMv = 0,
	.digitalInput = 0,
	.analogInput1 = 0,
	.analogInput2 = 0,
	.analogInput3 = 0,
	.analogInput4 = 0,
	.flags = 0,
	.magnetState = 0,
	.paramPath = "/ext",
	.batteryAge = 0,
	.apiVersion = "1.78",
	.qrtc = 0,
	.connectionTimeoutSec = 60,
	.logFileStatus = 0,
	.adcBatterySimulated = 0,
	.adcBatterySimulatedCounts = 0,
	.adcAnalogSensorSimulated = 0,
	.adcAnalogSensorSimulatedCounts = 0,
	.adcThermistorSimulated = 0,
	.adcThermistorSimulatedCounts = 0,
	.adcVRefSimulated = 0,
	.adcVRefSimulatedCounts = 0,
	.voltage1Simulated = 0,
	.voltage1SimulatedValue = 0.0,
	.voltage2Simulated = 0,
	.voltage2SimulatedValue = 0.0,
	.voltage3Simulated = 0,
	.voltage3SimulatedValue = 0.0,
	.voltage4Simulated = 0,
	.voltage4SimulatedValue = 0.0,
	.ultrasonicSimulated = 0,
	.ultrasonicSimulatedValue = 0.0,
	.pressureSimulated = 0,
	.pressureSimulatedValue = 0.0,
	.current1Simulated = 0,
	.current1SimulatedValue = 0.0,
	.current2Simulated = 0,
	.current2SimulatedValue = 0.0,
	.current3Simulated = 0,
	.current3SimulatedValue = 0.0,
	.current4Simulated = 0,
	.current4SimulatedValue = 0.0,
	.vrefSimulated = 0,
	.vrefSimulatedValue = 0.0,
	.temperature1Simulated = 0,
	.temperature1SimulatedValue = 0.0,
	.temperature2Simulated = 0,
	.temperature2SimulatedValue = 0.0,
	.temperature3Simulated = 0,
	.temperature3SimulatedValue = 0.0,
	.temperature4Simulated = 0,
	.temperature4SimulatedValue = 0.0,
	.batterymvSimulated = 0,
	.batterymvSimulatedValue = 0,
	.digitalInput1Simulated = 0,
	.digitalInput1SimulatedValue = 0,
	.digitalInput2Simulated = 0,
	.digitalInput2SimulatedValue = 0,
	.magSwitchSimulated = 0,
	.magSwitchSimulatedValue = 0,
	.tamperSwitchSimulated = 0,
	.tamperSwitchSimulatedValue = 0,
	.mobileAppDisconnect = 0,
	.attrSaveErrorCode = 0,
	.settingsPasscodeStatus = 0,
	.recoverSettingsCount = 0
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

/* index.....name............................type.savable.writable.readable.lockable.broadcast.deprecated.donotdumpvalidator..min.max. */
/* clang-format off */
AttributeEntry_t attrTable[ATTR_TABLE_SIZE] = {
    /* pystart - attribute table */
    [0  ] = { RW_ATTRS(sensorName)                    , s  , y, y, y, y, y, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [1  ] = { RW_ATTRS(sensorLocation)                , s  , y, y, y, y, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [2  ] = { RW_ATTRX(advertisingInterval)           , u16, y, y, y, y, y, n, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 500.0     , .max.ux = 10000.0    },
    [3  ] = { RW_ATTRX(advertisingDuration)           , u16, y, y, y, y, y, n, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 2000.0    , .max.ux = 65535.0    },
    [4  ] = { RW_ATTRX(passkey)                       , u32, y, y, y, y, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 999999.0   },
    [5  ] = { RW_ATTRX(lock)                          , b  , y, n, y, y, n, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [6  ] = { RW_ATTRX(batterySenseInterval)          , u32, y, y, y, y, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [7  ] = { RW_ATTRX(temperatureSenseInterval)      , u32, y, y, y, y, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [8  ] = { RW_ATTRX(AggregationCount)              , u8 , y, y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 1.0       , .max.ux = 32.0       },
    [9  ] = { RW_ATTRX(digitalOutput1State)           , b  , y, y, y, y, y, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [10 ] = { RW_ATTRX(digitalOutput2State)           , b  , y, y, y, y, y, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [11 ] = { RO_ATTRS(firmwareVersion)               , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [12 ] = { RO_ATTRS(resetReason)                   , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [13 ] = { RO_ATTRS(bluetoothAddress)              , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [14 ] = { RO_ATTRX(resetCount)                    , u32, n, n, y, n, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [15 ] = { RO_ATTRS(bootloaderVersion)             , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [16 ] = { RO_ATTRX(upTime)                        , i64, n, n, y, n, n, n, n, AttributeValidator_int64    , AttributePrepare_upTime                   , .min.ux = 0.0       , .max.ux = 0.0        },
    [17 ] = { RW_ATTRX(highTemp1Thresh1)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [18 ] = { RW_ATTRX(highTemp1Thresh2)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [19 ] = { RW_ATTRX(lowTemp1Thresh1)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [20 ] = { RW_ATTRX(lowTemp1Thresh2)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [21 ] = { RW_ATTRX(temp1DeltaThresh)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [22 ] = { RW_ATTRX(highTemp2Thresh1)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [23 ] = { RW_ATTRX(highTemp2Thresh2)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [24 ] = { RW_ATTRX(lowTemp2Thresh1)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [25 ] = { RW_ATTRX(lowTemp2Thresh2)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [26 ] = { RW_ATTRX(temp2DeltaThresh)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [27 ] = { RW_ATTRX(highTemp3Thresh1)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [28 ] = { RW_ATTRX(highTemp3Thresh2)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [29 ] = { RW_ATTRX(lowTemp3Thresh1)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [30 ] = { RW_ATTRX(lowTemp3Thresh2)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [31 ] = { RW_ATTRX(temp3DeltaThresh)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [32 ] = { RW_ATTRX(highTemp4Thresh1)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [33 ] = { RW_ATTRX(highTemp4Thresh2)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [34 ] = { RW_ATTRX(lowTemp4Thresh1)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [35 ] = { RW_ATTRX(lowTemp4Thresh2)               , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -128.0    , .max.fx = 127.0      },
    [36 ] = { RW_ATTRX(temp4DeltaThresh)              , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -1.0      , .max.fx = 255.0      },
    [37 ] = { RW_ATTRX(highAnalog1Thresh1)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [38 ] = { RW_ATTRX(highAnalog1Thresh2)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [39 ] = { RW_ATTRX(lowAnalog1Thresh1)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [40 ] = { RW_ATTRX(lowAnalog1Thresh2)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [41 ] = { RW_ATTRX(analog1DeltaThresh)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [42 ] = { RW_ATTRX(highAnalog2Thresh1)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [43 ] = { RW_ATTRX(highAnalog2Thresh2)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [44 ] = { RW_ATTRX(lowAnalog2Thresh1)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [45 ] = { RW_ATTRX(lowAnalog2Thresh2)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [46 ] = { RW_ATTRX(analog2DeltaThresh)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [47 ] = { RW_ATTRX(highAnalog3Thresh1)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [48 ] = { RW_ATTRX(highAnalog3Thresh2)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [49 ] = { RW_ATTRX(lowAnalog3Thresh1)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [50 ] = { RW_ATTRX(lowAnalog3Thresh2)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [51 ] = { RW_ATTRX(analog3DeltaThresh)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [52 ] = { RW_ATTRX(highAnalog4Thresh1)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [53 ] = { RW_ATTRX(highAnalog4Thresh2)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [54 ] = { RW_ATTRX(lowAnalog4Thresh1)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [55 ] = { RW_ATTRX(lowAnalog4Thresh2)             , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [56 ] = { RW_ATTRX(analog4DeltaThresh)            , f  , y, y, y, y, y, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 0.0       , .max.fx = 4096.0     },
    [57 ] = { RW_ATTRX(activeMode)                    , b  , y, y, y, n, y, n, n, AttributeValidator_cp8      , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [58 ] = { RW_ATTRX(useCodedPhy)                   , b  , y, y, y, y, y, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [59 ] = { RW_ATTRX(txPower)                       , u8 , y, y, y, n, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 13.0       },
    [60 ] = { RW_ATTRX(networkId)                     , u16, y, y, y, y, y, n, n, AttributeValidator_uint16   , NULL                                      , .min.ux = 0.0       , .max.ux = 65535.0    },
    [61 ] = { RW_ATTRX(configVersion)                 , u8 , y, y, y, n, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
    [62 ] = { RW_ATTRX(configType)                    , u8 , y, y, y, y, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 6.0        },
    [63 ] = { RW_ATTRX(hardwareMinorVersion)          , u8 , y, y, y, n, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 9.0        },
    [64 ] = { RO_ATTRX(ge)                            , f  , n, n, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -5.0      , .max.fx = 3.4e+38    },
    [65 ] = { RO_ATTRX(oe)                            , f  , n, n, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = -16.0     , .max.fx = 3.4e+38    },
    [66 ] = { RW_ATTRX(oldCoefficientA)               , f  , y, y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [67 ] = { RW_ATTRX(oldCoefficientB)               , f  , y, y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [68 ] = { RW_ATTRX(oldCoefficientC)               , f  , y, y, y, y, n, y, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [69 ] = { RW_ATTRX(thermistorConfig)              , u8 , y, y, y, y, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 15.0       },
    [70 ] = { RO_ATTRX(temperatureResult1)            , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult1       , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
    [71 ] = { RO_ATTRX(temperatureResult2)            , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult2       , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
    [72 ] = { RO_ATTRX(temperatureResult3)            , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult3       , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
    [73 ] = { RO_ATTRX(temperatureResult4)            , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_temperatureResult4       , .min.fx = -3.4e+38  , .max.fx = 3.4e+38    },
    [74 ] = { RO_ATTRX(temperatureAlarms)             , u32, n, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
    [75 ] = { RO_ATTRX(batteryVoltageMv)              , u16, n, n, y, n, n, n, n, AttributeValidator_uint16   , AttributePrepare_batteryVoltageMv         , .min.ux = 0.0       , .max.ux = 3800.0     },
    [76 ] = { RO_ATTRX(digitalInput)                  , u8 , n, n, y, y, n, n, n, AttributeValidator_uint8    , AttributePrepare_digitalInput             , .min.ux = 0.0       , .max.ux = 3.0        },
    [77 ] = { RW_ATTRX(digitalAlarms)                 , u8 , y, y, y, y, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 3.0        },
    [78 ] = { RW_ATTRX(digitalInput1Config)           , u8 , y, y, y, y, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 131.0      },
    [79 ] = { RW_ATTRX(digitalInput2Config)           , u8 , y, y, y, y, y, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 131.0      },
    [80 ] = { RO_ATTRX(analogInput1)                  , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput1             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [81 ] = { RO_ATTRX(analogInput2)                  , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput2             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [82 ] = { RO_ATTRX(analogInput3)                  , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput3             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [83 ] = { RO_ATTRX(analogInput4)                  , f  , n, n, y, n, n, n, n, AttributeValidator_float    , AttributePrepare_analogInput4             , .min.fx = 0.0       , .max.fx = 4095.0     },
    [84 ] = { RW_ATTRX(analogAlarms)                  , u32, y, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
    [85 ] = { RW_ATTRX(analogInput1Type)              , u8 , y, y, y, y, y, n, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
    [86 ] = { RW_ATTRX(analogInput2Type)              , u8 , y, y, y, y, y, n, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
    [87 ] = { RW_ATTRX(analogInput3Type)              , u8 , y, y, y, y, y, n, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
    [88 ] = { RW_ATTRX(analogInput4Type)              , u8 , y, y, y, y, y, n, n, AttributeValidator_aic      , NULL                                      , .min.ux = 0.0       , .max.ux = 7.0        },
    [89 ] = { RO_ATTRX(flags)                         , u32, n, y, y, n, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [90 ] = { RO_ATTRX(magnetState)                   , b  , n, n, y, n, n, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [91 ] = { RO_ATTRS(paramPath)                     , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [92 ] = { RO_ATTRX(batteryAge)                    , u32, n, n, y, n, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [93 ] = { RO_ATTRS(apiVersion)                    , s  , n, n, y, n, n, n, n, AttributeValidator_string   , NULL                                      , .min.ux = 0         , .max.ux = 0          },
    [94 ] = { RO_ATTRX(qrtc)                          , u32, n, n, y, n, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [95 ] = { RW_ATTRX(qrtcLastSet)                   , u32, y, n, y, n, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [96 ] = { RW_ATTRX(shOffset)                      , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [97 ] = { RW_ATTRX(analogSenseInterval)           , u32, y, y, y, y, y, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 86400.0    },
    [98 ] = { RW_ATTRX(tamperSwitchStatus)            , b  , y, n, y, n, y, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [99 ] = { RO_ATTRX(connectionTimeoutSec)          , u8 , n, y, y, y, y, y, n, AttributeValidator_cp8      , NULL                                      , .min.ux = 0.0       , .max.ux = 255.0      },
    [100] = { RW_ATTRX(settingsPasscode)              , u32, y, y, n, n, y, n, n, AttributeValidator_cp32     , NULL                                      , .min.ux = 0.0       , .max.ux = 999999.0   },
    [101] = { RW_ATTRX(therm1CoefficientA)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [102] = { RW_ATTRX(therm2CoefficientA)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [103] = { RW_ATTRX(therm3CoefficientA)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [104] = { RW_ATTRX(therm4CoefficientA)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [105] = { RW_ATTRX(therm1CoefficientB)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [106] = { RW_ATTRX(therm2CoefficientB)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [107] = { RW_ATTRX(therm3CoefficientB)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [108] = { RW_ATTRX(therm4CoefficientB)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [109] = { RW_ATTRX(therm1CoefficientC)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [110] = { RW_ATTRX(therm2CoefficientC)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [111] = { RW_ATTRX(therm3CoefficientC)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [112] = { RW_ATTRX(therm4CoefficientC)            , f  , y, y, y, y, n, n, n, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [113] = { RW_ATTRX(dataloggingEnable)             , b  , y, y, y, y, y, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0         , .max.ux = 1          },
    [114] = { RW_ATTRX(factoryResetEnable)            , b  , y, y, y, y, n, n, n, AttributeValidator_bool     , NULL                                      , .min.ux = 0         , .max.ux = 1          },
    [115] = { RO_ATTRX(logFileStatus)                 , u8 , n, n, y, n, n, n, n, AttributeValidator_uint8    , AttributePrepare_logFileStatus            , .min.ux = 0         , .max.ux = 3          },
    [116] = { RW_ATTRX(temperatureAlarmsEnable)       , u32, y, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
    [117] = { RW_ATTRX(analogAlarmsEnable)            , u32, y, y, y, y, n, n, n, AttributeValidator_uint32   , NULL                                      , .min.ux = 0.0       , .max.ux = 1048575.0  },
    [118] = { RO_ATTRX(adcBatterySimulated)           , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [119] = { RO_ATTRX(adcBatterySimulatedCounts)     , i16, n, y, y, n, n, n, y, AttributeValidator_int16    , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
    [120] = { RO_ATTRX(adcAnalogSensorSimulated)      , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [121] = { RO_ATTRX(adcAnalogSensorSimulatedCounts), i16, n, y, y, n, n, n, y, AttributeValidator_int16    , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
    [122] = { RO_ATTRX(adcThermistorSimulated)        , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [123] = { RO_ATTRX(adcThermistorSimulatedCounts)  , i16, n, y, y, n, n, n, y, AttributeValidator_int16    , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
    [124] = { RO_ATTRX(adcVRefSimulated)              , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [125] = { RO_ATTRX(adcVRefSimulatedCounts)        , i16, n, y, y, n, n, n, y, AttributeValidator_int16    , NULL                                      , .min.ux = 0.0       , .max.ux = 4095.0     },
    [126] = { RO_ATTRX(voltage1Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [127] = { RO_ATTRX(voltage1SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [128] = { RO_ATTRX(voltage2Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [129] = { RO_ATTRX(voltage2SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [130] = { RO_ATTRX(voltage3Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [131] = { RO_ATTRX(voltage3SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [132] = { RO_ATTRX(voltage4Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [133] = { RO_ATTRX(voltage4SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [134] = { RO_ATTRX(ultrasonicSimulated)           , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [135] = { RO_ATTRX(ultrasonicSimulatedValue)      , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [136] = { RO_ATTRX(pressureSimulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [137] = { RO_ATTRX(pressureSimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [138] = { RO_ATTRX(current1Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [139] = { RO_ATTRX(current1SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [140] = { RO_ATTRX(current2Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [141] = { RO_ATTRX(current2SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [142] = { RO_ATTRX(current3Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [143] = { RO_ATTRX(current3SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [144] = { RO_ATTRX(current4Simulated)             , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [145] = { RO_ATTRX(current4SimulatedValue)        , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [146] = { RO_ATTRX(vrefSimulated)                 , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [147] = { RO_ATTRX(vrefSimulatedValue)            , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [148] = { RO_ATTRX(temperature1Simulated)         , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [149] = { RO_ATTRX(temperature1SimulatedValue)    , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [150] = { RO_ATTRX(temperature2Simulated)         , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [151] = { RO_ATTRX(temperature2SimulatedValue)    , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [152] = { RO_ATTRX(temperature3Simulated)         , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [153] = { RO_ATTRX(temperature3SimulatedValue)    , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [154] = { RO_ATTRX(temperature4Simulated)         , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [155] = { RO_ATTRX(temperature4SimulatedValue)    , f  , n, y, y, n, n, n, y, AttributeValidator_float    , NULL                                      , .min.fx = 1.2e-38   , .max.fx = 3.4e+38    },
    [156] = { RO_ATTRX(batterymvSimulated)            , b  , n, y, y, n, n, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [157] = { RO_ATTRX(batterymvSimulatedValue)       , i32, n, y, y, n, n, n, y, AttributeValidator_int32    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [158] = { RO_ATTRX(digitalInput1Simulated)        , b  , n, y, y, n, n, n, y, AttributeValidator_din1simen, NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [159] = { RO_ATTRX(digitalInput1SimulatedValue)   , b  , n, y, y, n, n, n, y, AttributeValidator_din1sim  , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [160] = { RO_ATTRX(digitalInput2Simulated)        , b  , n, y, y, n, n, n, y, AttributeValidator_din2simen, NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [161] = { RO_ATTRX(digitalInput2SimulatedValue)   , b  , n, y, y, n, n, n, y, AttributeValidator_din2sim  , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [162] = { RO_ATTRX(magSwitchSimulated)            , b  , n, y, y, n, n, n, y, AttributeValidator_magsimen , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [163] = { RO_ATTRX(magSwitchSimulatedValue)       , b  , n, y, y, n, n, n, y, AttributeValidator_magsim   , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [164] = { RO_ATTRX(tamperSwitchSimulated)         , b  , n, y, y, n, n, n, y, AttributeValidator_tampsimen, NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [165] = { RO_ATTRX(tamperSwitchSimulatedValue)    , b  , n, y, y, n, n, n, y, AttributeValidator_tampsim  , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [166] = { RW_ATTRX(bootPHY)                       , u8 , y, y, y, n, n, n, y, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 2.0        },
    [167] = { RO_ATTRX(mobileAppDisconnect)           , b  , n, y, y, n, y, n, y, AttributeValidator_bool     , NULL                                      , .min.ux = 0.0       , .max.ux = 1.0        },
    [168] = { RO_ATTRX(attrSaveErrorCode)             , i32, n, n, y, n, y, n, y, AttributeValidator_int32    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        },
    [169] = { RO_ATTRX(settingsPasscodeStatus)        , u8 , n, n, y, n, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 2.0        },
    [170] = { RO_ATTRX(recoverSettingsCount)          , u8 , n, n, y, n, n, n, n, AttributeValidator_uint8    , NULL                                      , .min.ux = 0.0       , .max.ux = 0.0        }
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
__weak int AttributePrepare_upTime(void)
{
	return 0;
}

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

__weak int AttributePrepare_digitalInput(void)
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

__weak int AttributePrepare_logFileStatus(void)
{
	return 0;
}

/* pyend */
