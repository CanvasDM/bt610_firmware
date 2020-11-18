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

#ifndef H_FS_MGMT_
#define H_FS_MGMT_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Command IDs for file system management group.
 */
// pystart - mgmt function indices
#define BT610_MGMT_ID_SETSENSORNAME                         1
#define BT610_MGMT_ID_SETSENSORLOCATION                     2
#define BT610_MGMT_ID_SETBLEADVERTISINGINTERVAL             3
#define BT610_MGMT_ID_SETBLEADVERTISINGDURATION             4
#define BT610_MGMT_ID_SETBLEPASSKEY                         5
#define BT610_MGMT_ID_SETSETTINGSLOCK                       6
#define BT610_MGMT_ID_SETBATTERYSENSEINTERVAL               7
#define BT610_MGMT_ID_SETTEMPERATURESENSEINTERVAL           8
#define BT610_MGMT_ID_SETTEMPERATUREAGGREGATIONVALUE        9
#define BT610_MGMT_ID_SETDIGITALOUTPUT1                     10
#define BT610_MGMT_ID_SETDIGITALOUTPUT2                     11
#define BT610_MGMT_ID_SETDIGITALINPUT1                      12
#define BT610_MGMT_ID_SETDIGITALINPUT2                      13
#define BT610_MGMT_ID_SETANALOGINPUTTYPE                    14
#define BT610_MGMT_ID_SETTEMPERATURE1ALARMTHRESHOLD         15
#define BT610_MGMT_ID_SETTEMPERATURE2ALARMTHRESHOLD         16
#define BT610_MGMT_ID_SETTEMPERATURE3ALARMTHRESHOLD         17
#define BT610_MGMT_ID_SETTEMPERATURE4ALARMTHRESHOLD         18
#define BT610_MGMT_ID_SETANALOG1ALARMTHRESHOLDS             19
#define BT610_MGMT_ID_SETANALOG2ALARMTHRESHOLDS             20
#define BT610_MGMT_ID_SETANALOG3ALARMTHRESHOLDS             21
#define BT610_MGMT_ID_SETANALOG4ALARMTHRESHOLDS             22
#define BT610_MGMT_ID_SETACTIVEMODE                         23
#define BT610_MGMT_ID_SETUSECODEDPHY                        24
#define BT610_MGMT_ID_SETTXPOWER                            25
#define BT610_MGMT_ID_SETNETWORKID                          26
#define BT610_MGMT_ID_SETCONFIGVERSION                      27
#define BT610_MGMT_ID_SETHARDWAREVERSION                    28
#define BT610_MGMT_ID_SETLEDTEST                            29
#define BT610_MGMT_ID_GETALLTEMPERATURE                     30
#define BT610_MGMT_ID_GETTEMPERATURE1                       31
#define BT610_MGMT_ID_GETTEMPERATURE2                       32
#define BT610_MGMT_ID_GETTEMPERATURE3                       33
#define BT610_MGMT_ID_GETTEMPERATURE4                       34
#define BT610_MGMT_ID_GETCURRENT                            35
#define BT610_MGMT_ID_GETBATTERYVOLTAGE                     36
#define BT610_MGMT_ID_GETDIGITALINPUTALARMS                 37
#define BT610_MGMT_ID_GETTEMPERATURE1ALARMS                 38
#define BT610_MGMT_ID_GETTEMPERATURE2ALARMS                 39
#define BT610_MGMT_ID_GETTEMPERATURE3ALARMS                 40
#define BT610_MGMT_ID_GETTEMPERATURE4ALARMS                 41
#define BT610_MGMT_ID_GETANALOG1ALARMS                      42
#define BT610_MGMT_ID_GETANALOG2ALARMS                      43
#define BT610_MGMT_ID_GETANALOG3ALARMS                      44
#define BT610_MGMT_ID_GETANALOG4ALARMS                      45
#define BT610_MGMT_ID_GETHARDWAREVERSION                    46
#define BT610_MGMT_ID_GETFIRMWAREVERSION                    47
#define BT610_MGMT_ID_GETRESETREASON                        48
#define BT610_MGMT_ID_GETBLUETOOTHMAC                       49
#define BT610_MGMT_ID_GETBLUETOOTHMTU                       50
#define BT610_MGMT_ID_GETFLAGS                              51
#define BT610_MGMT_ID_GETRESETCOUNT                         52
#define BT610_MGMT_ID_GETSENSORNAME                         53
#define BT610_MGMT_ID_GETSENSORLOCATION                     54
#define BT610_MGMT_ID_GETBLEADVERTISINGINTERVAL             55
#define BT610_MGMT_ID_GETBLEADVERTISINGDURATION             56
#define BT610_MGMT_ID_GETSETTINGSLOCK                       57
#define BT610_MGMT_ID_GETBATTERYSENSEINTERVAL               58
#define BT610_MGMT_ID_GETTEMPERATURESENSEINTERVAL           59
#define BT610_MGMT_ID_GETTEMPERATUREAGGREGATIONVALUE        60
#define BT610_MGMT_ID_GETDIGITALOUTPUT1                     61
#define BT610_MGMT_ID_GETDIGITALOUTPUT2                     62
#define BT610_MGMT_ID_GETDIGITALINPUT1                      63
#define BT610_MGMT_ID_GETDIGITALINPUT2                      64
#define BT610_MGMT_ID_GETANALOGINPUT1TYPE                   65
#define BT610_MGMT_ID_GETTEMPERATURE1ALARMTHRESHOLD         66
#define BT610_MGMT_ID_GETTEMPERATURE2ALARMTHRESHOLD         67
#define BT610_MGMT_ID_GETTEMPERATURE3ALARMTHRESHOLD         68
#define BT610_MGMT_ID_GETTEMPERATURE4ALARMTHRESHOLD         69
#define BT610_MGMT_ID_GETANALOG1ALARMTHRESHOLDS             70
#define BT610_MGMT_ID_GETANALOG2ALARMTHRESHOLDS             71
#define BT610_MGMT_ID_GETANALOG3ALARMTHRESHOLDS             72
#define BT610_MGMT_ID_GETANALOG4ALARMTHRESHOLDS             73
#define BT610_MGMT_ID_GETACTIVEMODE                         74
#define BT610_MGMT_ID_GETUSECODEDPHY                        75
#define BT610_MGMT_ID_GETTXPOWER                            76
#define BT610_MGMT_ID_GETNETWORKID                          77
#define BT610_MGMT_ID_GETCONFIGVERSION                      78
#define BT610_MGMT_ID_GETMAGNETSTATE                        79
// pyend

/**
 * @brief Registers the file system management command handler group.
 */ 
void bt610_mgmt_register_group(void);

#ifdef __cplusplus
}
#endif

#endif
