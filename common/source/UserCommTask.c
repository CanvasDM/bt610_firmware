/**
 * @file UserCommTask.c
 * @brief Functions used to interface with the I/O
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(UserComm);
#define THIS_FILE "UserComm"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <drivers/spi.h>

#include "FrameworkIncludes.h"
#include "UserCommTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

#ifndef USER_COMM_TASK_PRIORITY
#define USER_COMM_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef USER_COMM_TASK_STACK_DEPTH
#define USER_COMM_TASK_STACK_DEPTH 4096
#endif

#ifndef USER_COMM_TASK_QUEUE_DEPTH
#define USER_COMM_TASK_QUEUE_DEPTH 8
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(spi1), okay)
#define SPI_DEV_NAME DT_LABEL(DT_NODELABEL(spi1))
#else
#error "Please set the correct spi device"
#endif

typedef struct UserCommTaskTag {
	FwkMsgTask_t msgTask;

} UserCommTaskObj_t;

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static UserCommTaskObj_t userCommTaskObject;

K_THREAD_STACK_DEFINE(userCommTaskStack, USER_COMM_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(userCommTaskQueue, FWK_QUEUE_ENTRY_SIZE,
	      USER_COMM_TASK_QUEUE_DEPTH, FWK_QUEUE_ALIGNMENT);
//default values can be changed
static struct spi_config spi_conf = {
	.frequency = 10000,
	.operation = (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) |
		      SPI_LINES_SINGLE),
	.slave = 0,
	.cs = NULL,
};

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void UserCommTaskThread(void *, void *, void *);
static uint8_t UserCommSpiSend(const struct device *spi,
			       const struct spi_config *spi_cfg,
			       const uint8_t *data, size_t len);

//static DispatchResult_t UserCommTaskPeriodicMsgHandler(FwkMsgReceiver_t *pMsgRxer, FwkMsg_t *pMsg);

/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t UserCommTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
	case FMC_INVALID:  return Framework_UnknownMsgHandler;
	default:           return NULL;
	}
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void UserCommTask_Initialize(void)
{
	userCommTaskObject.msgTask.rxer.id = FWK_ID_USER_COMM_TASK;
	userCommTaskObject.msgTask.rxer.rxBlockTicks = K_FOREVER;
	userCommTaskObject.msgTask.rxer.pMsgDispatcher =
		UserCommTaskMsgDispatcher;
	userCommTaskObject.msgTask.timerDurationTicks = K_MSEC(1000);
	userCommTaskObject.msgTask.timerPeriodTicks =
		K_MSEC(0); // 0 for one shot
	userCommTaskObject.msgTask.rxer.pQueue = &userCommTaskQueue;

	Framework_RegisterTask(&userCommTaskObject.msgTask);

	userCommTaskObject.msgTask.pTid =
		k_thread_create(&userCommTaskObject.msgTask.threadData,
				userCommTaskStack,
				K_THREAD_STACK_SIZEOF(userCommTaskStack),
				UserCommTaskThread, &userCommTaskObject, NULL,
				NULL, USER_COMM_TASK_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(userCommTaskObject.msgTask.pTid, THIS_FILE);

	//  userCommTaskObject.pBracket =
	//    Bracket_Initialize(CONFIG_JSON_BRACKET_BUFFER_SIZE,
	//				   k_malloc(CONFIG_JSON_BRACKET_BUFFER_SIZE));
	//userCommTaskObject.conn = NULL;
}
//void UserCommTask_ConfigSPI(spi_config *config)
//{
//memset(&spi_conf,spi_cfg, sizeof(spi_cfg));
//}
uint8_t UserCommTask_SendData(commType_t comm, const uint8_t *data, size_t len)
{
	uint8_t returnStatus = 0;
	const struct device *spiDevice;
	switch (comm) {
	case UART_COMM:
		break;
	case I2C_COMM:
		break;
	case SPI_CS1_COMM:
		spiDevice = device_get_binding(SPI_DEV_NAME);
		returnStatus = UserCommSpiSend(spiDevice, &spi_conf, data, len);
		if (returnStatus != 0) {
			LOG_ERR("Code %d", returnStatus);
		}
		break;
	case SPI_CS2_COMM:
		break;
	default:
		LOG_ERR("Not valid COMM");
		break;
	};
	if (returnStatus == 1) {
		LOG_ERR("Comm Type %d Code %d failed", comm, returnStatus);
	}
	return (returnStatus);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void UserCommTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	UserCommTaskObj_t *pObj = (UserCommTaskObj_t *)pArg1;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}

void uartCallBack(struct device *x)
{
	//	uart_irq_update(x);
	//	int data_length = 0;

	//	if (uart_irq_rx_ready(x)) {
	//		data_length = uart_fifo_read(x, uart_buf, sizeof(uart_buf));
	//		uart_buf[data_length] = 0;
	//	}
	//	printk("%s", uart_buf);
}
static uint8_t UserCommSpiSend(const struct device *spi,
			       const struct spi_config *spi_cfg,
			       const uint8_t *data, size_t len)
{
	const struct spi_buf_set tx = {
		.buffers =
			&(const struct spi_buf){
				.buf = (uint8_t *)data,
				.len = len,
			},
		.count = 1,
	};

	return spi_write(spi, spi_cfg, &tx);
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
