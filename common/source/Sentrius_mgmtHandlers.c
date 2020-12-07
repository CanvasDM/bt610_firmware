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
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "Sentrius_mgmt.h"
#include "Sentrius_mgmt_impl.h"
#include "Sentrius_mgmt_config.h"
/******************************************************************************/
// Local Constant, Macro and Type Definitions
/******************************************************************************/

//=================================================================================================
// Global Function Definitions
//=================================================================================================

// pystart - mgmt handlers
int Sentrius_mgmt_SetSensorName(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 1;
     int readCbor = 0;
     char sensorName[23+1];
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrTextStringType,
               .addr.string = sensorName,
               .len = sizeof(sensorName),
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetSensorNameResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_text_stringz(&ctxt->encoder, sensorName);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetSensorLocation(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 2;
     int readCbor = 0;
     char sensorLocation[32+1];
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrTextStringType,
               .addr.string = sensorLocation,
               .len = sizeof(sensorLocation),
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetSensorLocationResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_text_stringz(&ctxt->encoder, sensorLocation);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetBLEAdvertisingInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 3;
     int readCbor = 0;
     long long unsigned int advertisingInterval;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &advertisingInterval,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetBLEAdvertisingIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, advertisingInterval);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetBLEAdvertisingDuration(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 4;
     int readCbor = 0;
     long long unsigned int advertisingDuration;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &advertisingDuration,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetBLEAdvertisingDurationResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, advertisingDuration);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetBLEPasskey(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 5;
     int readCbor = 0;
     char passkey[6+1];
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrTextStringType,
               .addr.string = passkey,
               .len = sizeof(passkey),
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetBLEPasskeyResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_text_stringz(&ctxt->encoder, passkey);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetSettingsLock(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 6;
     int readCbor = 0;
     long long unsigned int lock;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lock,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetSettingsLockResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, lock);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetBatterySenseInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 7;
     int readCbor = 0;
     long long unsigned int batterySenseInterval;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &batterySenseInterval,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetBatterySenseIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, batterySenseInterval);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperatureSenseInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 8;
     int readCbor = 0;
     long long unsigned int temperatureSenseInterval;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temperatureSenseInterval,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperatureSenseIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, temperatureSenseInterval);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperatureAggregationValue(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 9;
     int readCbor = 0;
     long long unsigned int temperatureAggregationCount;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temperatureAggregationCount,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperatureAggregationValueResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, temperatureAggregationCount);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetDigitalOutput1(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 10;
     int readCbor = 0;
     long long unsigned int digitalOutput1Mv;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &digitalOutput1Mv,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetDigitalOutput1Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, digitalOutput1Mv);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetDigitalOutput2(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 11;
     int readCbor = 0;
     long long unsigned int digitalOutput2Mv;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &digitalOutput2Mv,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetDigitalOutput2Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, digitalOutput2Mv);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetDigitalInput1(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 12;
     int readCbor = 0;
     long long unsigned int digitalInput1;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &digitalInput1,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetDigitalInput1Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, digitalInput1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetDigitalInput2(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 13;
     int readCbor = 0;
     long long unsigned int digitalInput2;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &digitalInput2,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetDigitalInput2Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, digitalInput2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetAnalogInputType(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 14;
     int readCbor = 0;
     long long unsigned int analogInput1Type;
     long long unsigned int analogInput2Type;
     long long unsigned int analogInput3Type;
     long long unsigned int analogInput4Type;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analogInput1Type,
          },
          {
               .attribute = "p2",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analogInput2Type,
          },
          {
               .attribute = "p3",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analogInput3Type,
          },
          {
               .attribute = "p4",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analogInput4Type,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetAnalogInputTypeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, analogInput1Type);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_uint(&ctxt->encoder, analogInput2Type);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_uint(&ctxt->encoder, analogInput3Type);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, analogInput4Type);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperature1AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 15;
     int readCbor = 0;
     long long int highTemp1Thresh1;
     long long int highTemp1Thresh2;
     long long int lowTemp1Thresh1;
     long long int lowTemp1Thresh2;
     long long unsigned int temp1DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp1Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp1Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp1Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp1Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temp1DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperature1AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_int(&ctxt->encoder, highTemp1Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_int(&ctxt->encoder, highTemp1Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp1Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp1Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, temp1DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperature2AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 16;
     int readCbor = 0;
     long long int highTemp2Thresh1;
     long long int highTemp2Thresh2;
     long long int lowTemp2Thresh1;
     long long int lowTemp2Thresh2;
     long long unsigned int temp2DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp2Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp2Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp2Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp2Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temp2DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperature2AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_int(&ctxt->encoder, highTemp2Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_int(&ctxt->encoder, highTemp2Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp2Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp2Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, temp2DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperature3AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 17;
     int readCbor = 0;
     long long int highTemp3Thresh1;
     long long int highTemp3Thresh2;
     long long int lowTemp3Thresh1;
     long long int lowTemp3Thresh2;
     long long unsigned int temp3DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp3Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp3Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp3Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp3Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temp3DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperature3AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_int(&ctxt->encoder, highTemp3Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_int(&ctxt->encoder, highTemp3Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp3Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp3Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, temp3DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperature4AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 18;
     int readCbor = 0;
     long long int highTemp4Thresh1;
     long long int highTemp4Thresh2;
     long long int lowTemp4Thresh1;
     long long int lowTemp4Thresh2;
     long long unsigned int temp4DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp4Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrIntegerType,
               .addr.integer = &highTemp4Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp4Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrIntegerType,
               .addr.integer = &lowTemp4Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &temp4DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperature4AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_int(&ctxt->encoder, highTemp4Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_int(&ctxt->encoder, highTemp4Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp4Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_int(&ctxt->encoder, lowTemp4Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, temp4DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetAnalog1AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 19;
     int readCbor = 0;
     long long unsigned int highAnalog1Thresh1;
     long long unsigned int highAnalog1Thresh2;
     long long unsigned int lowAnalog1Thresh1;
     long long unsigned int lowAnalog1Thresh2;
     long long unsigned int analog1DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog1Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog1Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog1Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog1Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analog1DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetAnalog1AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog1Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog1Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog1Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog1Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, analog1DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetAnalog2AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 20;
     int readCbor = 0;
     long long unsigned int highAnalog2Thresh1;
     long long unsigned int highAnalog2Thresh2;
     long long unsigned int lowAnalog2Thresh1;
     long long unsigned int lowAnalog2Thresh2;
     long long unsigned int analog2DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog2Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog2Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog2Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog2Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analog2DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetAnalog2AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog2Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog2Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog2Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog2Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, analog2DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetAnalog3AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 21;
     int readCbor = 0;
     long long unsigned int highAnalog3Thresh1;
     long long unsigned int highAnalog3Thresh2;
     long long unsigned int lowAnalog3Thresh1;
     long long unsigned int lowAnalog3Thresh2;
     long long unsigned int analog3DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog3Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog3Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog3Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog3Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analog3DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetAnalog3AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog3Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog3Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog3Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog3Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, analog3DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetAnalog4AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 22;
     int readCbor = 0;
     long long unsigned int highAnalog4Thresh1;
     long long unsigned int highAnalog4Thresh2;
     long long unsigned int lowAnalog4Thresh1;
     long long unsigned int lowAnalog4Thresh2;
     long long unsigned int analog4DeltaThresh;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog4Thresh1,
          },
          {
               .attribute = "p2",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &highAnalog4Thresh2,
          },
          {
               .attribute = "p3",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog4Thresh1,
          },
          {
               .attribute = "p4",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &lowAnalog4Thresh2,
          },
          {
               .attribute = "p5",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &analog4DeltaThresh,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetAnalog4AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog4Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_uint(&ctxt->encoder, highAnalog4Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog4Thresh1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, lowAnalog4Thresh2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r5");
     err |= cbor_encode_uint(&ctxt->encoder, analog4DeltaThresh);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetActiveMode(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 23;
     int readCbor = 0;
     long long unsigned int activeMode;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &activeMode,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetActiveModeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, activeMode);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetUseCodedPhy(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 24;
     int readCbor = 0;
     long long unsigned int useCodedPhy;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &useCodedPhy,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetUseCodedPhyResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, useCodedPhy);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTxPower(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 25;
     int readCbor = 0;
     long long int txPower;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrIntegerType,
               .addr.integer = &txPower,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTxPowerResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_int(&ctxt->encoder, txPower);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetNetworkId(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 26;
     int readCbor = 0;
     long long unsigned int networkId;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &networkId,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetNetworkIdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, networkId);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetConfigVersion(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 27;
     int readCbor = 0;
     long long unsigned int configVersion;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &configVersion,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetConfigVersionResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, configVersion);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetConfigType(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 28;
     int readCbor = 0;
     long long unsigned int configType;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &configType,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetConfigTypeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, configType);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetHardwareVersion(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 29;
     int readCbor = 0;
     long long unsigned int hardwareMinorVersion;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &hardwareMinorVersion,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetHardwareVersionResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, hardwareMinorVersion);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetLedTest(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 30;
     int readCbor = 0;
     long long unsigned int ledTestActive;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &ledTestActive,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetLedTestResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_uint(&ctxt->encoder, ledTestActive);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAllTemperature(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 31;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAllTemperatureResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature1(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 32;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature1Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature2(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 33;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature2Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature3(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 34;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature3Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature4(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 35;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature4Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetCurrent(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 36;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetCurrentResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBatteryVoltage(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 37;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBatteryVoltageResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetDigitalInputAlarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 38;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetDigitalInputAlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature1Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 39;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature1AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature2Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 40;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature2AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature3Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 41;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature3AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature4Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 42;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature4AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog1Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 43;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog1AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog2Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 44;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog2AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog3Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 45;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog3AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog4Alarms(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 46;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog4AlarmsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetHardwareVersion(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 47;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetHardwareVersionResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetFirmwareVersion(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 48;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetFirmwareVersionResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetResetReason(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 49;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetResetReasonResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBluetoothMAC(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 50;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBluetoothMACResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBluetoothMTU(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 51;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBluetoothMTUResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetFlags(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 52;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetFlagsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetResetCount(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 53;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetResetCountResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetSensorName(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 54;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetSensorNameResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetSensorLocation(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 55;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetSensorLocationResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBLEAdvertisingInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 56;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBLEAdvertisingIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBLEAdvertisingDuration(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 57;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBLEAdvertisingDurationResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetSettingsLock(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 58;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetSettingsLockResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetBatterySenseInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 59;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetBatterySenseIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperatureSenseInterval(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 60;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperatureSenseIntervalResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperatureAggregationValue(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 61;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperatureAggregationValueResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetDigitalOutput1(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 62;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetDigitalOutput1Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetDigitalOutput2(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 63;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetDigitalOutput2Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetDigitalInput1(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 64;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetDigitalInput1Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetDigitalInput2(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 65;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetDigitalInput2Result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalogInput1Type(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 66;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalogInput1TypeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature1AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 67;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature1AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature2AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 68;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature2AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature3AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 69;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature3AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTemperature4AlarmThreshold(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 70;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTemperature4AlarmThresholdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog1AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 71;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog1AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog2AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 72;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog2AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog3AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 73;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog3AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetAnalog4AlarmThresholds(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 74;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetAnalog4AlarmThresholdsResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetActiveMode(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 75;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetActiveModeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetUseCodedPhy(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 76;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetUseCodedPhyResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetTxPower(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 77;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetTxPowerResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetNetworkId(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 78;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetNetworkIdResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetConfigVersion(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 79;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetConfigVersionResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetConfigType(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 80;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetConfigTypeResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_GetMagnetState(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 81;
     int readCbor = 0;
     const struct cbor_attr_t params_attr[] = {
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "GetMagnetStateResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetThermistorCalibration(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 82;
     int readCbor = 0;
     float coefficient1;
     float coefficient2;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p1",
               .type = CborAttrFloatType,
               .addr.fval = &coefficient1,
          },
          {
               .attribute = "p2",
               .type = CborAttrFloatType,
               .addr.fval = &coefficient2,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetThermistorCalibrationResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &coefficient1);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &coefficient2);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
int Sentrius_mgmt_SetTemperatureCoef(struct mgmt_ctxt *ctxt)
{
     const uint16_t msgID = 83;
     int readCbor = 0;
     float coefficientA;
     float coefficientB;
     float coefficientC;
     long long unsigned int thermistorIndex;
     const struct cbor_attr_t params_attr[] = {
          {
               .attribute = "p2",
               .type = CborAttrFloatType,
               .addr.fval = &coefficientA,
          },
          {
               .attribute = "p3",
               .type = CborAttrFloatType,
               .addr.fval = &coefficientB,
          },
          {
               .attribute = "p4",
               .type = CborAttrFloatType,
               .addr.fval = &coefficientC,
          },
          {
               .attribute = "p1",
               .type = CborAttrUnsignedIntegerType,
               .addr.uinteger = &thermistorIndex,
          },
     };
     readCbor = cbor_read_object(&ctxt->it, params_attr);
     if (readCbor != 0) {
          return MGMT_ERR_EINVAL;
     }
     CborError err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "id");
     err |= cbor_encode_uint(&ctxt->encoder, msgID);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "SetTemperatureCoefResult");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r1");
     err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &coefficientA);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r2");
     err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &coefficientB);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r3");
     err |= cbor_encode_floating_point(&ctxt->encoder, CborFloatType, &coefficientC);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "r4");
     err |= cbor_encode_uint(&ctxt->encoder, thermistorIndex);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "result");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "ok");
     return 0;
}
// pyend
