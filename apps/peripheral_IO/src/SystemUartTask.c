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
#include <tinycbor/cbor_mbuf_writer.h>
#include <tinycbor/cbor_mbuf_reader.h>

#include "SystemUartTask.h"
#include "BspSupport.h"
#include <drivers/spi.h>

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

static char fifo_data[55] = "This is a FIFO test.\r\n";

#define UART_DATA_SIZE	55//(sizeof(fifo_data) - 1)

typedef struct SystemUartTaskTag
{
  FwkMsgTask_t msgTask; 
  BracketObj_t *pBracket; 
  bool data_received;
  bool data_transmitted;
  uint16_t char_sent;

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
static void ReadRxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);
//=================================================================================================
// Framework Message Dispatcher
//=================================================================================================

static FwkMsgHandler_t SystemUartTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
  switch( MsgCode )
  {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  case FMC_READ_UART_BUFFER:   return ReadRxBufferMsgHandler;
  default:                     return NULL;
  }
}
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void SystemUartTask_Initialize(void)

{
  memset(&systemUartTaskObject, 0, sizeof(SystemUartTaskObj_t));

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

  systemUartTaskObject.char_sent = 0;

}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SystemUartTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	SystemUartTaskObj_t *pObj = (SystemUartTaskObj_t*)pArg1;

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

	//systemUartTaskObject.data_received = false;
	///while (systemUartTaskObject.data_received == false) {
	//}

	/* Verify uart_irq_rx_disable() */
	//uart_irq_rx_disable(uart_dev);
}
static void DisableUartRead(void)
{
	struct device *uart_dev = device_get_binding(UART_DEVICE_NAME);
	uart_irq_rx_disable(uart_dev);
}
static void ReadRxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
    UNUSED_PARAMETER(pMsg);

    return DISPATCH_OK;
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
static void uartHandlerIsr(struct device *dev)
{
	uint8_t recvData;
	uint16_t rx = 0;
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
#ifdef TXTEST
	if (uart_irq_tx_ready(dev) && tx_data_idx < UART_DATA_SIZE) {
		/* We arrive here by "tx ready" interrupt, so should always
		 * be able to put at least one byte into a FIFO. If not,
		 * well, we'll fail test.
		 */
		if (uart_fifo_fill(dev,
				   (uint8_t *)&fifo_data[tx_data_idx++], 1) > 0) {
			systemUartTaskObject.data_transmitted = true;
			systemUartTaskObject.char_sent++;
		}

		if (tx_data_idx == UART_DATA_SIZE) {
			/* If we transmitted everything, stop IRQ stream,
			 * otherwise main app might never run.
			 */
			uart_irq_tx_disable(dev);
		}
	}
#endif

	/* Read all data off UART, and send to RX worker for unmuxing */
	while (uart_irq_update(dev) &&
	       uart_irq_rx_ready(dev)) 
	{
		rx = uart_fifo_read(dev, fifo_data,
				    UART_DATA_SIZE);
		LOG_DBG("%c", fifo_data);			

	}
	//	wrote = ring_buf_put(real_uart->rx_ringbuf,
	//			     real_uart->rx_buf, rx);
	//	if (wrote < rx) {
	//		LOG_ERR("Ring buffer full, drop %d bytes",
	//			rx - wrote);
	//	}
		//k_work_submit_to_queue(&uart_mux_workq, &real_uart->rx_work);
	
	UartMsg_t *pUartMsg = BufferPool_Take(sizeof(UartMsg_t));
    if( pUartMsg != NULL )
    {
		pUartMsg->size = UART_DATA_SIZE;//Bracket_Copy(pObj->pBracketObj, pUartMsg->buffer);
		pUartMsg->header.msgCode = FMC_READ_UART_BUFFER;
		pUartMsg->header.txId = FWK_ID_SYSTEM_UART_TASK;
		pUartMsg->header.rxId = FWK_ID_SYSTEM_UART_TASK;
		FRAMEWORK_MSG_UNICAST(pUartMsg);
	}
#ifdef RXTEST
	/* Verify uart_irq_rx_ready() */
	if (uart_irq_rx_ready(dev)) 
	{
		/* Verify uart_fifo_read() */
		uart_fifo_read(dev, &recvData, 1);
		LOG_DBG("%c", recvData);

		if ((recvData == '\n') || (recvData == '\r')) {
			systemUartTaskObject.data_received = true;
			DisableUartRead();
		}
	}
#endif
}