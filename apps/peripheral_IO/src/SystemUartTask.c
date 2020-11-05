/**
 * @file SystemUartTask.c
 * @brief Blank is better that repeating the information in header.
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
#include <drivers/uart.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include "FrameworkIncludes.h"
#include "Bracket.h"
#include "cbor.h"

//#include "JsonBuilder.h"
#include "SystemUartTask.h"
#include "BspSupport.h"
#include <drivers/spi.h>

#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(SystemUart);

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define THIS_FILE "SystemUartTask"
#if !SYSTEM_UART_TASK_USES_MAIN_THREAD
  #ifndef SYSTEM_UART_TASK_PRIORITY
    #define SYSTEM_UART_TASK_PRIORITY K_PRIO_PREEMPT(1)
  #endif

  #ifndef SYSTEM_UART_TASK_STACK_DEPTH
    #define SYSTEM_UART_TASK_STACK_DEPTH 4096
  #endif
#endif

#ifndef SYSTEM_UART_TASK_QUEUE_DEPTH
  #define SYSTEM_UART_TASK_QUEUE_DEPTH 8
#endif

#define CBOR_START_BYTE 0xA4
#define CBOR_END_BYTE 0x30

static char fifo_data[] = "A46269641825666D6574686F646D47657453656E736F724E616D6566706172616D7380676A736F6E72706363322E309B4B";
//"This is a FIFO test.\r\n";

#define UART_DATA_SIZE	(sizeof(fifo_data) - 1)

#define UART_BUFFER_SIZE (128)
typedef struct
{
  size_t size;
  char Rxbuffer[UART_BUFFER_SIZE];
  char Txbuffer[UART_BUFFER_SIZE];
  bool wasValid;
  
} UartMessage_t;
typedef struct SystemUartTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
  bool data_received;
  bool data_transmitted;
  UartMessage_t uartData;

} SystemUartTaskObj_t;
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static SystemUartTaskObj_t systemUartTaskObject;

K_THREAD_STACK_DEFINE(systemUartTaskStack, SYSTEM_UART_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(systemUartTaskQueue, 
              FWK_QUEUE_ENTRY_SIZE, 
              SYSTEM_UART_TASK_QUEUE_DEPTH, 
              FWK_QUEUE_ALIGNMENT);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void SystemUartTaskThread(void *, void *, void *);
static void SetupUartRead(void);
static void DisableUartRead(void);
static void uartHandlerIsr(struct device *dev);
static DispatchResult_t TxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t SystemUartTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  case FMC_TRANSMIT_BUFFER:    return TxBufferMsgHandler;
  default:                     return NULL;
  }
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SystemUartTask_Initialize(bool Enable)
{
	memset(&systemUartTaskObject, 0, sizeof(SystemUartTaskObj_t));
	if( !Enable )
	{
		return;
	}

	systemUartTaskObject.msgTask.rxer.id               = FWK_ID_SYSTEM_UART_TASK;
	systemUartTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
	systemUartTaskObject.msgTask.rxer.pMsgDispatcher   = SystemUartTaskMsgDispatcher;
	systemUartTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
	systemUartTaskObject.msgTask.timerPeriodTicks      = K_MSEC(0); // 0 for one shot 
	systemUartTaskObject.msgTask.rxer.pQueue           = &systemUartTaskQueue;

	Framework_RegisterTask(&systemUartTaskObject.msgTask);

	systemUartTaskObject.msgTask.pTid = 
	k_thread_create(&systemUartTaskObject.msgTask.threadData, 
					systemUartTaskStack,
					K_THREAD_STACK_SIZEOF(systemUartTaskStack),
					SystemUartTaskThread,
					&systemUartTaskObject, 
					NULL, 
					NULL,
					SYSTEM_UART_TASK_PRIORITY, 
					0, 
					K_NO_WAIT);

	k_thread_name_set(systemUartTaskObject.msgTask.pTid, THIS_FILE);

	systemUartTaskObject.uartData.size = 0;

}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SystemUartTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	SystemUartTaskObj_t *pObj = (SystemUartTaskObj_t*)pArg1;

	BSP_ConfigureUART();
	ProtocolTask_Initialize();
	SetupUartRead();


	while( true )
	{
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

static void SetupUartRead(void)
{
	struct device *uart_dev = device_get_binding(UART_DEVICE_NAME);

	/* Verify uart_irq_callback_set() */
	uart_irq_callback_set(uart_dev, uartHandlerIsr);

	/* Enable Tx/Rx interrupt before using fifo */
	/* Verify uart_irq_rx_enable() */
	uart_irq_rx_enable(uart_dev);

	LOG_INF("Please send characters to serial console\n");


	/* Verify uart_irq_rx_disable() */
	//uart_irq_rx_disable(uart_dev);
}
static void DisableUartRead(void)
{
	struct device *uart_dev = device_get_binding(UART_DEVICE_NAME);
	uart_irq_rx_disable(uart_dev);
}
static DispatchResult_t TxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
	UartMsg_t *pTxMsg = (UartMsg_t *)pMsg;
	struct device *uart_dev = device_get_binding(UART_DEVICE_NAME);

	memset(systemUartTaskObject.uartData.Txbuffer, 0, UART_BUFFER_SIZE);
	/* Verify uart_irq_callback_set() */
	uart_irq_callback_set(uart_dev, uartHandlerIsr);
	memcpy(systemUartTaskObject.uartData.Txbuffer,pTxMsg->buffer, pTxMsg->size);
	/* Enable Tx/Rx interrupt before using fifo */
	/* Verify uart_irq_tx_enable() */
	uart_irq_tx_enable(uart_dev);
}

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void uartHandlerIsr(struct device *dev)
{
	uint8_t recvData = 0;
	static bool cborMessage = false;
	static int tx_data_idx;

	/* Verify uart_irq_update() */
	if (!uart_irq_update(dev)) {
		LOG_DBG("retval should always be 1\n");
		return;
	}

	/* Verify uart_irq_tx_ready() */
	/* Note that TX IRQ may be disabled, but uart_irq_tx_ready() may
	 * still return true when ISR is called for another UART interrupt,
	 * hence additional check for i < UART_DATA_SIZE.
	 */

	if (uart_irq_tx_ready(dev) && tx_data_idx < UART_DATA_SIZE) {
		/* We arrive here by "tx ready" interrupt, so should always
		 * be able to put at least one byte into a FIFO. If not,
		 * well, we'll fail test.
		 */
		if (uart_fifo_fill(dev,
				   (uint8_t *)systemUartTaskObject.uartData.Txbuffer[tx_data_idx++], 1) > 0) 
		{
			systemUartTaskObject.data_transmitted = true;
		}

		if (tx_data_idx == UART_DATA_SIZE) {
			/* If we transmitted everything, stop IRQ stream,
			 * otherwise main app might never run.
			 */
			uart_irq_tx_disable(dev);
		}
	}

	if (uart_irq_rx_ready(dev)) 
	{
		uart_fifo_read(dev, &recvData, 1);
		//LOG_DBG("%c", recvData);
		if( ((systemUartTaskObject.uartData.size == 0) &&
			(recvData == CBOR_START_BYTE)) ||
			(cborMessage == true) )
		{
			systemUartTaskObject.uartData.Rxbuffer[systemUartTaskObject.uartData.size] = recvData;
			systemUartTaskObject.uartData.size = systemUartTaskObject.uartData.size + 1;
			cborMessage = true; 
		}
		else
		{
			/* Not vaild CBOR message*/
		}
		if( (cborMessage == true) && (recvData == CBOR_END_BYTE))
		{
			cborMessage = false;
			UartMsg_t *pUartMsg = BufferPool_Take(sizeof(UartMsg_t));
			if( pUartMsg != NULL )
			{
				pUartMsg->size = systemUartTaskObject.uartData.size;//Bracket_Copy(pObj->pBracketObj, pUartMsg->buffer);
				pUartMsg->header.msgCode = FMC_READ_BUFFER;
				pUartMsg->header.txId = FWK_ID_SYSTEM_UART_TASK;
				pUartMsg->header.rxId = FWK_ID_PROTOCOL_TASK;
				memcpy(pUartMsg->buffer,systemUartTaskObject.uartData.Rxbuffer,systemUartTaskObject.uartData.size);
				FRAMEWORK_MSG_UNICAST(pUartMsg);
			}
			systemUartTaskObject.uartData.size = 0;
		}		
	}		
}