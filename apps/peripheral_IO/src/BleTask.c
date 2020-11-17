/**
 * @file BleTask.c
 * @brief Communication over the ble connection using Zephyr drivers
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(BleTask);
#define THIS_FILE "BleTask"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include "settings/settings.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include "FrameworkIncludes.h"
#include "Version.h"
#include "UserCommTask.h"
#include "LairdAdvFormat.h"
#include "BleTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef BLE_TASK_PRIORITY
#define BLE_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef BLE_TASK_STACK_DEPTH
#define BLE_TASK_STACK_DEPTH 4096
#endif

#ifndef BLE_TASK_QUEUE_DEPTH
#define BLE_TASK_QUEUE_DEPTH 8
#endif

#define BLE_ADV_LENGTH_MANUFACTURER_SPECIFIC            24

#define MAX_SENSOR_NAME_LENGTH (23 + 1)

#define UINT16_BYTE_0(v) (uint8_t)(((v)&0x00FF) >> 0)
#define UINT16_BYTE_1(v) (uint8_t)(((v)&0xFF00) >> 8)

#define UINT32_BYTE_0(v) (uint8_t)(((v)&0x000000FF) >> 0)
#define UINT32_BYTE_1(v) (uint8_t)(((v)&0x0000FF00) >> 8)
#define UINT32_BYTE_2(v) (uint8_t)(((v)&0x00FF0000) >> 16)
#define UINT32_BYTE_3(v) (uint8_t)(((v)&0xFF000000) >> 24)

typedef struct BleTaskTag {
	FwkMsgTask_t msgTask; 
    bt_addr_le_t bdAddr;
	bool initialized;
	struct bt_conn *default_conn;

} BleTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static BleTaskObj_t bleTaskObject;

K_THREAD_STACK_DEFINE(bleTaskStack, BLE_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(bleTaskQueue, FWK_QUEUE_ENTRY_SIZE, BLE_TASK_QUEUE_DEPTH,
              FWK_QUEUE_ALIGNMENT);

/*Advertisment Protocol Parameters*/
struct standAdvData {
	uint8_t companyId[2];
	uint8_t protocolId[2];
	uint8_t networkId[2];
	uint8_t flags[2];
	uint8_t bdAddress[6];
	uint8_t recordType;
	uint8_t recordNumber[2];
	uint8_t epoch[4];
	uint8_t data[4];
	uint8_t resetCount;
};
struct scanAdvData {
	uint8_t companyId[2];
	uint8_t protocolId[2];
	uint8_t productId[2];
	uint8_t FirmwareVersionMajor;
	uint8_t FirmwareVersionMinor;
	uint8_t FirmwareVersionPatch;
	uint8_t FirmwareType;
	uint8_t ConfigVersion;
	uint8_t BootLoaderVersionMajor;
	uint8_t BootLoaderVersionMinor;
	uint8_t BootLoaderVersionPatch;
	uint8_t HardwareVersion;	
};
struct standAdvData mfgData[] = { {
	.companyId = { 0xaa, 0xbb },
	.protocolId = { 0xcc, 0xdd },
	.networkId = { 0xee, 0xff },
	.flags = { 0x00, 0x11 },
	.bdAddress = { 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 },
		.recordType = 0x88,
	.epoch = { 0x99, 0xAA, 0xBB, 0xCC },
	.data = { 0xDD, 0xEE, 0xFF, 0xFF },
		.resetCount = 0x00,
} };
struct scanAdvData scanMfgData[] = { {
	.companyId = { 0xaa, 0xbb },
	.protocolId = { 0xcc, 0xdd },
	.productId = { 0xee, 0xff },
		.FirmwareVersionMajor =		0xDE,
		.FirmwareVersionMinor =		0xAD,
		.FirmwareVersionPatch =		0x00,
		.FirmwareType =			0xCA,
		.ConfigVersion =		0xFE,
		.BootLoaderVersionMajor =	0xBE,
		.BootLoaderVersionMinor =	0xEF,
		.BootLoaderVersionPatch =	0x00,
		.HardwareVersion =		0xFF,
} };

static struct bt_data standardAdvert[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfgData,
		BLE_ADV_LENGTH_MANUFACTURER_SPECIFIC)
};
static struct bt_data scanAdvert[] = { BT_DATA(
	BT_DATA_MANUFACTURER_DATA, scanMfgData,
	BLE_ADV_LENGTH_MANUFACTURER_SPECIFIC) };

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void BleTaskThread(void *, void *, void *);
static void ConfigureAndStartBle(void);
static void ParamsInit(void);
static void AdvertisingInit(void);
static void BleReadyCB(int err);

static DispatchResult_t BleStartMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					   FwkMsg_t *pMsg);
static DispatchResult_t BleReadyMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					   FwkMsg_t *pMsg);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t BleTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
		case FMC_INVALID:            return Framework_UnknownMsgHandler;
		case FMC_CODE_BLE_START:     return BleStartMsgHandler;
		case FMC_CODE_BLE_TRANSMIT:  return BleReadyMsgHandler;
		default:                     return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void BleTask_Initialize(void)

{
	memset(&bleTaskObject, 0, sizeof(BleTaskObj_t));

	bleTaskObject.msgTask.rxer.id               = FWK_ID_BLE_TASK;
	bleTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
	bleTaskObject.msgTask.rxer.pMsgDispatcher   = BleTaskMsgDispatcher;
	bleTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
	bleTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
	bleTaskObject.msgTask.rxer.pQueue           = &bleTaskQueue;

	Framework_RegisterTask(&bleTaskObject.msgTask);

	bleTaskObject.msgTask.pTid = 
		k_thread_create(&bleTaskObject.msgTask.threadData, bleTaskStack,
		    K_THREAD_STACK_SIZEOF(bleTaskStack),
				BleTaskThread, &bleTaskObject, NULL, NULL,
				BLE_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(bleTaskObject.msgTask.pTid, THIS_FILE);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void BleTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	BleTaskObj_t *pObj = (BleTaskObj_t *)pArg1;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}
static void ConfigureAndStartBle(void)
{  
	uint32_t err_code;

	/* This must occur before the initialization items below.*/
	err_code = bt_enable(BleReadyCB);
	if (err_code) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err_code);
		//FRAMEWORK_ASSERT(err_code == NRF_SUCCESS); 
		return;
	}
	AdvertisingInit();
}

static void ParamsInit(void)
{
	uint32_t err;
#if 0
	char name[MAX_SENSOR_NAME_LENGTH] = CONFIG_BT_DEVICE_NAME;
	//  AttributeTask_GetString(name, ATTR_INDEX_sensorName, MAX_SENSOR_NAME_LENGTH);
	err = bt_set_name((const char *)name);
	if (err) {
		LOG_ERR("Failed to set device name (%d)", err);
	}
#endif

	/*Security*/
	err = bt_conn_set_security(bleTaskObject.default_conn, BT_SECURITY_L3);
	if (err) {
		LOG_ERR("Setting security failed (err %d)", err);
	}
}
#if 0
static void UpdateNonEventDataInAdvertisement(void)
{
  // Update advertisement values that may be set during a connection.
  
  // network id
  uint32_t networkId = 0;
  AttributeTask_GetUint32(&networkId, ATTR_INDEX_networkId);
  bleAdvObj.enc_advdata[BLE_ENC_ADV_NETWORK_ID_OFFSET+0] = UINT32_BYTE_0(networkId);
  bleAdvObj.enc_advdata[BLE_ENC_ADV_NETWORK_ID_OFFSET+1] = UINT32_BYTE_1(networkId);
  // flags
  uint32_t flags = Flags_Get();
  bleAdvObj.enc_advdata[BLE_ENC_ADV_FLAGS_OFFSET+0] = UINT32_BYTE_0(flags);
  bleAdvObj.enc_advdata[BLE_ENC_ADV_FLAGS_OFFSET+1] = UINT32_BYTE_1(flags);

  uint32_t useCodedPhy = 0;
  AttributeTask_GetUint32(&useCodedPhy, ATTR_INDEX_useCodedPhy);
  UpdateConfigVersionInAdvertisement(useCodedPhy);
}
#endif
static void AdvertisingInit(void)
{
	mfgData->companyId[0] = UINT16_BYTE_0(LAIRD_CONNECTIVITY);
	mfgData->companyId[1] = UINT16_BYTE_1(LAIRD_CONNECTIVITY);

	/*Setup up either the scan response packet or the extended*/

	/* Read Bluetooth address and store it into attributes and save it for use when
	   building the manufacturer specific data.*/
	size_t size = 0;
	bt_id_get(&bleTaskObject.bdAddr, &size);
        char addr_str[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(&bleTaskObject.bdAddr, addr_str, sizeof(addr_str));
}
#if 0
static void AdvertisementEncoder(void)
{
	//size_t index = 0; 

	/* protocol id */
	mfgData->protocolId[0] = UINT16_BYTE_0(LAIRD_ADV_PROTOCOL_ID_SENTRIUS);
	mfgData->protocolId[1] = UINT16_BYTE_1(LAIRD_ADV_PROTOCOL_ID_SENTRIUS);

	/* network id */
	uint16_t networkId = 0;
	//AttributeTask_GetUint16(&networkId, ATTR_INDEX_networkId);
	mfgData->networkId[0] = UINT16_BYTE_0(networkId);
	mfgData->networkId[1] = UINT16_BYTE_1(networkId);

	/* flags */
	uint16_t flags = 0; //Flags_Get();
	mfgData->flags[0] = UINT16_BYTE_0(flags);
	mfgData->flags[1] = UINT16_BYTE_1(flags);

	/* BD addr */
	mfgData->bdAddress[0] = bleTaskObject.bdAddr.a.val[0];
	mfgData->bdAddress[1] = bleTaskObject.bdAddr.a.val[1];
	mfgData->bdAddress[2] = bleTaskObject.bdAddr.a.val[2];
	mfgData->bdAddress[3] = bleTaskObject.bdAddr.a.val[3];
	mfgData->bdAddress[4] = bleTaskObject.bdAddr.a.val[4];
	mfgData->bdAddress[5] = bleTaskObject.bdAddr.a.val[5];

	/* event type */
	//mfgData->recordType =pSensorMsg->event.type;

	/* id */
	mfgData->recordNumber[0] = UINT16_BYTE_0(0); //pSensorMsg->id);
	mfgData->recordNumber[1] = UINT16_BYTE_1(1); //pSensorMsg->id);

	/* epoch */
	mfgData->epoch[0] = UINT32_BYTE_0(0x0C); //pSensorMsg->event.timestamp);
	mfgData->epoch[1] = UINT32_BYTE_1(0x0C); //pSensorMsg->event.timestamp);
	mfgData->epoch[2] = UINT32_BYTE_2(0x0C); //pSensorMsg->event.timestamp);
	mfgData->epoch[3] = UINT32_BYTE_3(0x0C); //pSensorMsg->event.timestamp);

	/* data */
	mfgData->data[0] = UINT16_BYTE_0(0x0F); //pSensorMsg->event.data.u16);
	mfgData->data[1] = UINT16_BYTE_1(0x0F); //pSensorMsg->event.data.u16);
	mfgData->data[2] = 0;
	mfgData->data[3] = 0;

	/* reset count LSB */
	uint8_t resetCount = 0;
	//AttributeTask_GetUint32(&resetCount, ATTR_INDEX_resetCount);
	mfgData->resetCount = resetCount;
}
#endif

static void BleReadyCB(int err)
{
	bleTaskObject.initialized = true;
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		ParamsInit();

		settings_load();
		FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_BLE_TASK,
						      FMC_CODE_BLE_TRANSMIT);
	}
}

/******************************************************************************/
/* Framework Message Functions                                               */
/******************************************************************************/
static DispatchResult_t BleStartMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);
	UNUSED_PARAMETER(pMsgRxer);
	if (!bleTaskObject.initialized) {
		ConfigureAndStartBle(); 
	}
	return DISPATCH_OK;
}
static DispatchResult_t BleReadyMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					   FwkMsg_t *pMsg)
{
	UNUSED_PARAMETER(pMsg);
	UNUSED_PARAMETER(pMsgRxer);
	uint32_t err;

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, standardAdvert,
			      ARRAY_SIZE(standardAdvert), scanAdvert,
			      ARRAY_SIZE(scanAdvert));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)\n", err);
	}
	LOG_DBG("Advertising\n");

	return DISPATCH_OK;
}