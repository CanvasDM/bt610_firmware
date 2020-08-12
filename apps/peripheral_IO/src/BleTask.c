/**
 * @file BleTask.c
 * @brief Communication over the ble connection using Zephyr drivers
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include "FrameworkIncludes.h"
#include "Bracket.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include "UserCommTask.h"
#include "BleTask.h"

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(BleTask);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define THIS_FILE "BleTask"
#if !BLE_TASK_USES_MAIN_THREAD
  #ifndef BLE_TASK_PRIORITY
    #define BLE_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef BLE_TASK_STACK_DEPTH
    #define BLE_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef BLE_TASK_QUEUE_DEPTH
  #define BLE_TASK_QUEUE_DEPTH 8
#endif

#define MAX_SENSOR_NAME_LENGTH      (23+1)

typedef struct BleTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
  bool initialized;
  struct bt_conn *default_conn;

} BleTaskObj_t;
/*
typedef struct
{
  FrameworkMsgTask_t msgTask;  // The name "task" is used by macros.

  uint16_t connectionHandle;
  ble_gap_addr_t bdAddr;
  QueueHandle_t pAdvQueue;
  bool advQueueBusy;
  bool sdInitialized;
  bool factoryReset;
  
  TimerHandle_t ShelfModeAdvTimerHandle;
  
} BleTaskObj_t;*/
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static BleTaskObj_t bleTaskObject;

K_THREAD_STACK_DEFINE(bleTaskStack, BLE_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(bleTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              BLE_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);

static struct bt_data ad[] = {};

static struct bt_le_scan_param scan_param = 
{
		.type       = BT_HCI_LE_SCAN_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 0x0010,
		.window     = 0x0010,
};
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void BleTaskThread(void *, void *, void *);
static void ConfigureAndStartBle(void);
static void ParamsInit(void);
static void AdvertisementEncoder(void);
static void BleReadyCB(int err);
static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
		    struct net_buf_simple *buf);

static DispatchResult_t BleStartMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
static DispatchResult_t BleReadyMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);



//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t BleTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  case FMC_CODE_BLE_START:     return BleStartMsgHandler;
  case FMC_CODE_SENSOR:        return BleReadyMsgHandler;
  //case FMC_PERIODIC:           return BleTaskPeriodicMsgHandler;
 // case FMC_WATCHDOG_CHALLENGE: return Watchdog_ChallengeHandler;
  default:                     return NULL;
  }
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
    k_thread_create(&bleTaskObject.msgTask.threadData, 
                    bleTaskStack,
                    K_THREAD_STACK_SIZEOF(bleTaskStack),
                    BleTaskThread,
                    &bleTaskObject, 
                    NULL, 
                    NULL,
                    BLE_TASK_PRIORITY, 
                    0, 
                    K_NO_WAIT);

  k_thread_name_set(bleTaskObject.msgTask.pTid, THIS_FILE);

//  bleTaskObject.pBracket = 		
//    Bracket_Initialize(CONFIG_JSON_BRACKET_BUFFER_SIZE,
//				   k_malloc(CONFIG_JSON_BRACKET_BUFFER_SIZE));
	//bleTaskObject.conn = NULL;
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void BleTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
    BleTaskObj_t *pObj = (BleTaskObj_t*)pArg1;


    while( true )
    {
    Framework_MsgReceiver(&pObj->msgTask.rxer);
    }
}
static void ConfigureAndStartBle(void)
{  
    uint32_t err_code;
  
  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  //uint32_t ramStart = 0;
  //err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ramStart);
  //FRAMEWORK_ASSERT(err_code == NRF_SUCCESS);

  //ConfigureMaxPacketsPerConnectionEvent(ramStart);

  // This must occur before the initialization items below.
  err_code = bt_enable(BleReadyCB);
  if (err_code) 
  {
      printk("Bluetooth init failed (err %d)\n", err_code);
      LOG_ERR("Bluetooth init failed (err %d)\n", err_code);
      //FRAMEWORK_ASSERT(err_code == NRF_SUCCESS); 
      return;
  }
}
static void ParamsInit(void)
{
  uint32_t err;
  //Device Name
  char name[MAX_SENSOR_NAME_LENGTH] = CONFIG_BT_DEVICE_NAME;
//  AttributeTask_GetString(name, ATTR_INDEX_sensorName, MAX_SENSOR_NAME_LENGTH);
  err = bt_set_name((const char *)name);
  if (err) 
  {
    LOG_ERR("Failed to set device name (%d)", err);
  }
  //FRAMEWORK_ASSERT(err == NRF_SUCCESS);

  //Security
  err = bt_conn_set_security(bleTaskObject.default_conn, BT_SECURITY_L3);
  if (err) 
  {
    LOG_ERR("Setting security failed (err %d)", err);
  }

}
#define OFFSETFLAG 4
static void AdvertisementEncoder(void)
{
  size_t index = 0; 
  static struct bt_data adTest[31] = {0};

 // index++;// = index + OFFSETFLAG;
  adTest[0] = BT_DATA_BYTES(BT_DATA_GAP_APPEARANCE, 0x00, 0x03);
}
static void BleReadyCB(int err)
{
  bleTaskObject.initialized = true;
  if (IS_ENABLED(CONFIG_SETTINGS)) 
  {
    ParamsInit();
    settings_load();
    FRAMEWORK_MSG_UNICAST_CREATE_AND_SEND(FWK_ID_BLE_TASK, FMC_CODE_SENSOR);
  }
}
static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
		    struct net_buf_simple *buf)
{
	//mfg_data[2]++;
}
/******************************************************************************/
/* Framework Message Functions                                               */
/******************************************************************************/
static DispatchResult_t BleStartMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsg);
    UNUSED_PARAMETER(pMsgRxer);
    if( !bleTaskObject.initialized )
    {
        ConfigureAndStartBle(); 
    }
    return DISPATCH_OK;
}
static DispatchResult_t BleReadyMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsg);
    UNUSED_PARAMETER(pMsgRxer);
	uint32_t err;
    if( bleTaskObject.initialized )
    {
        printk("Bluetooth initialized\n");

        err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
        if (err) 
        {
            printk("Advertising failed to start (err %d)\n", err);
            LOG_ERR("Advertising failed to start (err %d)\n", err);
            return;
        }
        LOG_DBG("Advertising\n");
    }
    return DISPATCH_OK;
}