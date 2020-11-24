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

#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Sentrius_mgmt.h"
#include "Sentrius_mgmt_impl.h"
#include "Sentrius_mgmt_config.h"

static const struct mgmt_handler sentrius_mgmt_handlers[] = {
	// pystart - mgmt handlers
    [SENTRIUS_MGMT_ID_GETALLTEMPERATURE] = {
         .mh_read = Sentrius_mgmt_GetAllTemperature
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE1] = {
         .mh_read = Sentrius_mgmt_GetTemperature1
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE2] = {
         .mh_read = Sentrius_mgmt_GetTemperature2
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE3] = {
         .mh_read = Sentrius_mgmt_GetTemperature3
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE4] = {
         .mh_read = Sentrius_mgmt_GetTemperature4
    },
    [SENTRIUS_MGMT_ID_GETCURRENT] = {
         .mh_read = Sentrius_mgmt_GetCurrent
    },
    [SENTRIUS_MGMT_ID_GETBATTERYVOLTAGE] = {
         .mh_read = Sentrius_mgmt_GetBatteryVoltage
    },
    [SENTRIUS_MGMT_ID_GETDIGITALINPUTALARMS] = {
         .mh_read = Sentrius_mgmt_GetDigitalInputAlarms
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE1ALARMS] = {
         .mh_read = Sentrius_mgmt_GetTemperature1Alarms
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE2ALARMS] = {
         .mh_read = Sentrius_mgmt_GetTemperature2Alarms
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE3ALARMS] = {
         .mh_read = Sentrius_mgmt_GetTemperature3Alarms
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE4ALARMS] = {
         .mh_read = Sentrius_mgmt_GetTemperature4Alarms
    },
    [SENTRIUS_MGMT_ID_GETANALOG1ALARMS] = {
         .mh_read = Sentrius_mgmt_GetAnalog1Alarms
    },
    [SENTRIUS_MGMT_ID_GETANALOG2ALARMS] = {
         .mh_read = Sentrius_mgmt_GetAnalog2Alarms
    },
    [SENTRIUS_MGMT_ID_GETANALOG3ALARMS] = {
         .mh_read = Sentrius_mgmt_GetAnalog3Alarms
    },
    [SENTRIUS_MGMT_ID_GETANALOG4ALARMS] = {
         .mh_read = Sentrius_mgmt_GetAnalog4Alarms
    },
    [SENTRIUS_MGMT_ID_GETHARDWAREVERSION] = {
         .mh_read = Sentrius_mgmt_GetHardwareVersion
    },
    [SENTRIUS_MGMT_ID_GETFIRMWAREVERSION] = {
         .mh_read = Sentrius_mgmt_GetFirmwareVersion
    },
    [SENTRIUS_MGMT_ID_GETRESETREASON] = {
         .mh_read = Sentrius_mgmt_GetResetReason
    },
    [SENTRIUS_MGMT_ID_GETBLUETOOTHMAC] = {
         .mh_read = Sentrius_mgmt_GetBluetoothMAC
    },
    [SENTRIUS_MGMT_ID_GETBLUETOOTHMTU] = {
         .mh_read = Sentrius_mgmt_GetBluetoothMTU
    },
    [SENTRIUS_MGMT_ID_GETFLAGS] = {
         .mh_read = Sentrius_mgmt_GetFlags
    },
    [SENTRIUS_MGMT_ID_GETRESETCOUNT] = {
         .mh_read = Sentrius_mgmt_GetResetCount
    },
    [SENTRIUS_MGMT_ID_GETSENSORNAME] = {
         .mh_read = Sentrius_mgmt_GetSensorName
    },
    [SENTRIUS_MGMT_ID_GETSENSORLOCATION] = {
         .mh_read = Sentrius_mgmt_GetSensorLocation
    },
    [SENTRIUS_MGMT_ID_GETBLEADVERTISINGINTERVAL] = {
         .mh_read = Sentrius_mgmt_GetBLEAdvertisingInterval
    },
    [SENTRIUS_MGMT_ID_GETBLEADVERTISINGDURATION] = {
         .mh_read = Sentrius_mgmt_GetBLEAdvertisingDuration
    },
    [SENTRIUS_MGMT_ID_GETSETTINGSLOCK] = {
         .mh_read = Sentrius_mgmt_GetSettingsLock
    },
    [SENTRIUS_MGMT_ID_GETBATTERYSENSEINTERVAL] = {
         .mh_read = Sentrius_mgmt_GetBatterySenseInterval
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURESENSEINTERVAL] = {
         .mh_read = Sentrius_mgmt_GetTemperatureSenseInterval
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATUREAGGREGATIONVALUE] = {
         .mh_read = Sentrius_mgmt_GetTemperatureAggregationValue
    },
    [SENTRIUS_MGMT_ID_GETDIGITALOUTPUT1] = {
         .mh_read = Sentrius_mgmt_GetDigitalOutput1
    },
    [SENTRIUS_MGMT_ID_GETDIGITALOUTPUT2] = {
         .mh_read = Sentrius_mgmt_GetDigitalOutput2
    },
    [SENTRIUS_MGMT_ID_GETDIGITALINPUT1] = {
         .mh_read = Sentrius_mgmt_GetDigitalInput1
    },
    [SENTRIUS_MGMT_ID_GETDIGITALINPUT2] = {
         .mh_read = Sentrius_mgmt_GetDigitalInput2
    },
    [SENTRIUS_MGMT_ID_GETANALOGINPUT1TYPE] = {
         .mh_read = Sentrius_mgmt_GetAnalogInput1Type
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE1ALARMTHRESHOLD] = {
         .mh_read = Sentrius_mgmt_GetTemperature1AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE2ALARMTHRESHOLD] = {
         .mh_read = Sentrius_mgmt_GetTemperature2AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE3ALARMTHRESHOLD] = {
         .mh_read = Sentrius_mgmt_GetTemperature3AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_GETTEMPERATURE4ALARMTHRESHOLD] = {
         .mh_read = Sentrius_mgmt_GetTemperature4AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_GETANALOG1ALARMTHRESHOLDS] = {
         .mh_read = Sentrius_mgmt_GetAnalog1AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_GETANALOG2ALARMTHRESHOLDS] = {
         .mh_read = Sentrius_mgmt_GetAnalog2AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_GETANALOG3ALARMTHRESHOLDS] = {
         .mh_read = Sentrius_mgmt_GetAnalog3AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_GETANALOG4ALARMTHRESHOLDS] = {
         .mh_read = Sentrius_mgmt_GetAnalog4AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_GETACTIVEMODE] = {
         .mh_read = Sentrius_mgmt_GetActiveMode
    },
    [SENTRIUS_MGMT_ID_GETUSECODEDPHY] = {
         .mh_read = Sentrius_mgmt_GetUseCodedPhy
    },
    [SENTRIUS_MGMT_ID_GETTXPOWER] = {
         .mh_read = Sentrius_mgmt_GetTxPower
    },
    [SENTRIUS_MGMT_ID_GETNETWORKID] = {
         .mh_read = Sentrius_mgmt_GetNetworkId
    },
    [SENTRIUS_MGMT_ID_GETCONFIGVERSION] = {
         .mh_read = Sentrius_mgmt_GetConfigVersion
    },
    [SENTRIUS_MGMT_ID_GETMAGNETSTATE] = {
         .mh_read = Sentrius_mgmt_GetMagnetState
    },
    [SENTRIUS_MGMT_ID_SETSENSORNAME] = {
         .mh_write = Sentrius_mgmt_SetSensorName
    },
    [SENTRIUS_MGMT_ID_SETSENSORLOCATION] = {
         .mh_write = Sentrius_mgmt_SetSensorLocation
    },
    [SENTRIUS_MGMT_ID_SETBLEADVERTISINGINTERVAL] = {
         .mh_write = Sentrius_mgmt_SetBLEAdvertisingInterval
    },
    [SENTRIUS_MGMT_ID_SETBLEADVERTISINGDURATION] = {
         .mh_write = Sentrius_mgmt_SetBLEAdvertisingDuration
    },
    [SENTRIUS_MGMT_ID_SETBLEPASSKEY] = {
         .mh_write = Sentrius_mgmt_SetBLEPasskey
    },
    [SENTRIUS_MGMT_ID_SETSETTINGSLOCK] = {
         .mh_write = Sentrius_mgmt_SetSettingsLock
    },
    [SENTRIUS_MGMT_ID_SETBATTERYSENSEINTERVAL] = {
         .mh_write = Sentrius_mgmt_SetBatterySenseInterval
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATURESENSEINTERVAL] = {
         .mh_write = Sentrius_mgmt_SetTemperatureSenseInterval
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATUREAGGREGATIONVALUE] = {
         .mh_write = Sentrius_mgmt_SetTemperatureAggregationValue
    },
    [SENTRIUS_MGMT_ID_SETDIGITALOUTPUT1] = {
         .mh_write = Sentrius_mgmt_SetDigitalOutput1
    },
    [SENTRIUS_MGMT_ID_SETDIGITALOUTPUT2] = {
         .mh_write = Sentrius_mgmt_SetDigitalOutput2
    },
    [SENTRIUS_MGMT_ID_SETDIGITALINPUT1] = {
         .mh_write = Sentrius_mgmt_SetDigitalInput1
    },
    [SENTRIUS_MGMT_ID_SETDIGITALINPUT2] = {
         .mh_write = Sentrius_mgmt_SetDigitalInput2
    },
    [SENTRIUS_MGMT_ID_SETANALOGINPUTTYPE] = {
         .mh_write = Sentrius_mgmt_SetAnalogInputType
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATURE1ALARMTHRESHOLD] = {
         .mh_write = Sentrius_mgmt_SetTemperature1AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATURE2ALARMTHRESHOLD] = {
         .mh_write = Sentrius_mgmt_SetTemperature2AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATURE3ALARMTHRESHOLD] = {
         .mh_write = Sentrius_mgmt_SetTemperature3AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_SETTEMPERATURE4ALARMTHRESHOLD] = {
         .mh_write = Sentrius_mgmt_SetTemperature4AlarmThreshold
    },
    [SENTRIUS_MGMT_ID_SETANALOG1ALARMTHRESHOLDS] = {
         .mh_write = Sentrius_mgmt_SetAnalog1AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_SETANALOG2ALARMTHRESHOLDS] = {
         .mh_write = Sentrius_mgmt_SetAnalog2AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_SETANALOG3ALARMTHRESHOLDS] = {
         .mh_write = Sentrius_mgmt_SetAnalog3AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_SETANALOG4ALARMTHRESHOLDS] = {
         .mh_write = Sentrius_mgmt_SetAnalog4AlarmThresholds
    },
    [SENTRIUS_MGMT_ID_SETACTIVEMODE] = {
         .mh_write = Sentrius_mgmt_SetActiveMode
    },
    [SENTRIUS_MGMT_ID_SETUSECODEDPHY] = {
         .mh_write = Sentrius_mgmt_SetUseCodedPhy
    },
    [SENTRIUS_MGMT_ID_SETTXPOWER] = {
         .mh_write = Sentrius_mgmt_SetTxPower
    },
    [SENTRIUS_MGMT_ID_SETNETWORKID] = {
         .mh_write = Sentrius_mgmt_SetNetworkId
    },
    [SENTRIUS_MGMT_ID_SETCONFIGVERSION] = {
         .mh_write = Sentrius_mgmt_SetConfigVersion
    },
    [SENTRIUS_MGMT_ID_SETHARDWAREVERSION] = {
         .mh_write = Sentrius_mgmt_SetHardwareVersion
    },
    [SENTRIUS_MGMT_ID_SETLEDTEST] = {
         .mh_write = Sentrius_mgmt_SetLedTest
    },
	// pyend
};

#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

void Sentrius_mgmt_register_group(void)
{
	mgmt_register_group(&sentrius_mgmt_group);
}
