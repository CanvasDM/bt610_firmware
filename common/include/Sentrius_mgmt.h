/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_SENTRIUS_MGMT_
#define H_SENTRIUS_MGMT_

#ifdef __cplusplus
extern "C" {
#endif

#include "mgmt/mgmt.h"

#define MGMT_GROUP_ID_SENTRIUS   65

// pystart - mgmt handler function defines
mgmt_handler_fn Sentrius_mgmt_SetSensorName;
mgmt_handler_fn Sentrius_mgmt_SetSensorLocation;
mgmt_handler_fn Sentrius_mgmt_SetBLEAdvertisingInterval;
mgmt_handler_fn Sentrius_mgmt_SetBLEAdvertisingDuration;
mgmt_handler_fn Sentrius_mgmt_SetBLEPasskey;
mgmt_handler_fn Sentrius_mgmt_SetSettingsLock;
mgmt_handler_fn Sentrius_mgmt_SetBatterySenseInterval;
mgmt_handler_fn Sentrius_mgmt_SetTemperatureSenseInterval;
mgmt_handler_fn Sentrius_mgmt_SetTemperatureAggregationValue;
mgmt_handler_fn Sentrius_mgmt_SetDigitalOutput1;
mgmt_handler_fn Sentrius_mgmt_SetDigitalOutput2;
mgmt_handler_fn Sentrius_mgmt_SetDigitalInput1;
mgmt_handler_fn Sentrius_mgmt_SetDigitalInput2;
mgmt_handler_fn Sentrius_mgmt_SetAnalogInputType;
mgmt_handler_fn Sentrius_mgmt_SetTemperature1AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_SetTemperature2AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_SetTemperature3AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_SetTemperature4AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_SetAnalog1AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_SetAnalog2AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_SetAnalog3AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_SetAnalog4AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_SetActiveMode;
mgmt_handler_fn Sentrius_mgmt_SetUseCodedPhy;
mgmt_handler_fn Sentrius_mgmt_SetTxPower;
mgmt_handler_fn Sentrius_mgmt_SetNetworkId;
mgmt_handler_fn Sentrius_mgmt_SetConfigVersion;
mgmt_handler_fn Sentrius_mgmt_SetHardwareVersion;
mgmt_handler_fn Sentrius_mgmt_SetLedTest;
mgmt_handler_fn Sentrius_mgmt_GetAllTemperature;
mgmt_handler_fn Sentrius_mgmt_GetTemperature1;
mgmt_handler_fn Sentrius_mgmt_GetTemperature2;
mgmt_handler_fn Sentrius_mgmt_GetTemperature3;
mgmt_handler_fn Sentrius_mgmt_GetTemperature4;
mgmt_handler_fn Sentrius_mgmt_GetCurrent;
mgmt_handler_fn Sentrius_mgmt_GetBatteryVoltage;
mgmt_handler_fn Sentrius_mgmt_GetDigitalInputAlarms;
mgmt_handler_fn Sentrius_mgmt_GetTemperature1Alarms;
mgmt_handler_fn Sentrius_mgmt_GetTemperature2Alarms;
mgmt_handler_fn Sentrius_mgmt_GetTemperature3Alarms;
mgmt_handler_fn Sentrius_mgmt_GetTemperature4Alarms;
mgmt_handler_fn Sentrius_mgmt_GetAnalog1Alarms;
mgmt_handler_fn Sentrius_mgmt_GetAnalog2Alarms;
mgmt_handler_fn Sentrius_mgmt_GetAnalog3Alarms;
mgmt_handler_fn Sentrius_mgmt_GetAnalog4Alarms;
mgmt_handler_fn Sentrius_mgmt_GetHardwareVersion;
mgmt_handler_fn Sentrius_mgmt_GetFirmwareVersion;
mgmt_handler_fn Sentrius_mgmt_GetResetReason;
mgmt_handler_fn Sentrius_mgmt_GetBluetoothMAC;
mgmt_handler_fn Sentrius_mgmt_GetBluetoothMTU;
mgmt_handler_fn Sentrius_mgmt_GetFlags;
mgmt_handler_fn Sentrius_mgmt_GetResetCount;
mgmt_handler_fn Sentrius_mgmt_GetSensorName;
mgmt_handler_fn Sentrius_mgmt_GetSensorLocation;
mgmt_handler_fn Sentrius_mgmt_GetBLEAdvertisingInterval;
mgmt_handler_fn Sentrius_mgmt_GetBLEAdvertisingDuration;
mgmt_handler_fn Sentrius_mgmt_GetSettingsLock;
mgmt_handler_fn Sentrius_mgmt_GetBatterySenseInterval;
mgmt_handler_fn Sentrius_mgmt_GetTemperatureSenseInterval;
mgmt_handler_fn Sentrius_mgmt_GetTemperatureAggregationValue;
mgmt_handler_fn Sentrius_mgmt_GetDigitalOutput1;
mgmt_handler_fn Sentrius_mgmt_GetDigitalOutput2;
mgmt_handler_fn Sentrius_mgmt_GetDigitalInput1;
mgmt_handler_fn Sentrius_mgmt_GetDigitalInput2;
mgmt_handler_fn Sentrius_mgmt_GetAnalogInput1Type;
mgmt_handler_fn Sentrius_mgmt_GetTemperature1AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_GetTemperature2AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_GetTemperature3AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_GetTemperature4AlarmThreshold;
mgmt_handler_fn Sentrius_mgmt_GetAnalog1AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_GetAnalog2AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_GetAnalog3AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_GetAnalog4AlarmThresholds;
mgmt_handler_fn Sentrius_mgmt_GetActiveMode;
mgmt_handler_fn Sentrius_mgmt_GetUseCodedPhy;
mgmt_handler_fn Sentrius_mgmt_GetTxPower;
mgmt_handler_fn Sentrius_mgmt_GetNetworkId;
mgmt_handler_fn Sentrius_mgmt_GetConfigVersion;
mgmt_handler_fn Sentrius_mgmt_GetMagnetState;
// pyend
/**
 * Command IDs for file system management group.
 */
// pystart - mgmt function indices
#define SENTRIUS_MGMT_ID_SETSENSORNAME                         1
#define SENTRIUS_MGMT_ID_SETSENSORLOCATION                     2
#define SENTRIUS_MGMT_ID_SETBLEADVERTISINGINTERVAL             3
#define SENTRIUS_MGMT_ID_SETBLEADVERTISINGDURATION             4
#define SENTRIUS_MGMT_ID_SETBLEPASSKEY                         5
#define SENTRIUS_MGMT_ID_SETSETTINGSLOCK                       6
#define SENTRIUS_MGMT_ID_SETBATTERYSENSEINTERVAL               7
#define SENTRIUS_MGMT_ID_SETTEMPERATURESENSEINTERVAL           8
#define SENTRIUS_MGMT_ID_SETTEMPERATUREAGGREGATIONVALUE        9
#define SENTRIUS_MGMT_ID_SETDIGITALOUTPUT1                     10
#define SENTRIUS_MGMT_ID_SETDIGITALOUTPUT2                     11
#define SENTRIUS_MGMT_ID_SETDIGITALINPUT1                      12
#define SENTRIUS_MGMT_ID_SETDIGITALINPUT2                      13
#define SENTRIUS_MGMT_ID_SETANALOGINPUTTYPE                    14
#define SENTRIUS_MGMT_ID_SETTEMPERATURE1ALARMTHRESHOLD         15
#define SENTRIUS_MGMT_ID_SETTEMPERATURE2ALARMTHRESHOLD         16
#define SENTRIUS_MGMT_ID_SETTEMPERATURE3ALARMTHRESHOLD         17
#define SENTRIUS_MGMT_ID_SETTEMPERATURE4ALARMTHRESHOLD         18
#define SENTRIUS_MGMT_ID_SETANALOG1ALARMTHRESHOLDS             19
#define SENTRIUS_MGMT_ID_SETANALOG2ALARMTHRESHOLDS             20
#define SENTRIUS_MGMT_ID_SETANALOG3ALARMTHRESHOLDS             21
#define SENTRIUS_MGMT_ID_SETANALOG4ALARMTHRESHOLDS             22
#define SENTRIUS_MGMT_ID_SETACTIVEMODE                         23
#define SENTRIUS_MGMT_ID_SETUSECODEDPHY                        24
#define SENTRIUS_MGMT_ID_SETTXPOWER                            25
#define SENTRIUS_MGMT_ID_SETNETWORKID                          26
#define SENTRIUS_MGMT_ID_SETCONFIGVERSION                      27
#define SENTRIUS_MGMT_ID_SETHARDWAREVERSION                    28
#define SENTRIUS_MGMT_ID_SETLEDTEST                            29
#define SENTRIUS_MGMT_ID_GETALLTEMPERATURE                     30
#define SENTRIUS_MGMT_ID_GETTEMPERATURE1                       31
#define SENTRIUS_MGMT_ID_GETTEMPERATURE2                       32
#define SENTRIUS_MGMT_ID_GETTEMPERATURE3                       33
#define SENTRIUS_MGMT_ID_GETTEMPERATURE4                       34
#define SENTRIUS_MGMT_ID_GETCURRENT                            35
#define SENTRIUS_MGMT_ID_GETBATTERYVOLTAGE                     36
#define SENTRIUS_MGMT_ID_GETDIGITALINPUTALARMS                 37
#define SENTRIUS_MGMT_ID_GETTEMPERATURE1ALARMS                 38
#define SENTRIUS_MGMT_ID_GETTEMPERATURE2ALARMS                 39
#define SENTRIUS_MGMT_ID_GETTEMPERATURE3ALARMS                 40
#define SENTRIUS_MGMT_ID_GETTEMPERATURE4ALARMS                 41
#define SENTRIUS_MGMT_ID_GETANALOG1ALARMS                      42
#define SENTRIUS_MGMT_ID_GETANALOG2ALARMS                      43
#define SENTRIUS_MGMT_ID_GETANALOG3ALARMS                      44
#define SENTRIUS_MGMT_ID_GETANALOG4ALARMS                      45
#define SENTRIUS_MGMT_ID_GETHARDWAREVERSION                    46
#define SENTRIUS_MGMT_ID_GETFIRMWAREVERSION                    47
#define SENTRIUS_MGMT_ID_GETRESETREASON                        48
#define SENTRIUS_MGMT_ID_GETBLUETOOTHMAC                       49
#define SENTRIUS_MGMT_ID_GETBLUETOOTHMTU                       50
#define SENTRIUS_MGMT_ID_GETFLAGS                              51
#define SENTRIUS_MGMT_ID_GETRESETCOUNT                         52
#define SENTRIUS_MGMT_ID_GETSENSORNAME                         53
#define SENTRIUS_MGMT_ID_GETSENSORLOCATION                     54
#define SENTRIUS_MGMT_ID_GETBLEADVERTISINGINTERVAL             55
#define SENTRIUS_MGMT_ID_GETBLEADVERTISINGDURATION             56
#define SENTRIUS_MGMT_ID_GETSETTINGSLOCK                       57
#define SENTRIUS_MGMT_ID_GETBATTERYSENSEINTERVAL               58
#define SENTRIUS_MGMT_ID_GETTEMPERATURESENSEINTERVAL           59
#define SENTRIUS_MGMT_ID_GETTEMPERATUREAGGREGATIONVALUE        60
#define SENTRIUS_MGMT_ID_GETDIGITALOUTPUT1                     61
#define SENTRIUS_MGMT_ID_GETDIGITALOUTPUT2                     62
#define SENTRIUS_MGMT_ID_GETDIGITALINPUT1                      63
#define SENTRIUS_MGMT_ID_GETDIGITALINPUT2                      64
#define SENTRIUS_MGMT_ID_GETANALOGINPUT1TYPE                   65
#define SENTRIUS_MGMT_ID_GETTEMPERATURE1ALARMTHRESHOLD         66
#define SENTRIUS_MGMT_ID_GETTEMPERATURE2ALARMTHRESHOLD         67
#define SENTRIUS_MGMT_ID_GETTEMPERATURE3ALARMTHRESHOLD         68
#define SENTRIUS_MGMT_ID_GETTEMPERATURE4ALARMTHRESHOLD         69
#define SENTRIUS_MGMT_ID_GETANALOG1ALARMTHRESHOLDS             70
#define SENTRIUS_MGMT_ID_GETANALOG2ALARMTHRESHOLDS             71
#define SENTRIUS_MGMT_ID_GETANALOG3ALARMTHRESHOLDS             72
#define SENTRIUS_MGMT_ID_GETANALOG4ALARMTHRESHOLDS             73
#define SENTRIUS_MGMT_ID_GETACTIVEMODE                         74
#define SENTRIUS_MGMT_ID_GETUSECODEDPHY                        75
#define SENTRIUS_MGMT_ID_GETTXPOWER                            76
#define SENTRIUS_MGMT_ID_GETNETWORKID                          77
#define SENTRIUS_MGMT_ID_GETCONFIGVERSION                      78
#define SENTRIUS_MGMT_ID_GETMAGNETSTATE                        79
// pyend

/**
 * @brief Registers the file system management command handler group.
 */ 
void Sentrius_mgmt_register_group(void);

#ifdef __cplusplus
}
#endif

#endif
