/**
 * @file ProtocolTask.c
 * @brief Blank is better that repeating the information in header.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#define LOG_LEVEL LOG_LEVEL_DBG
LOG_MODULE_REGISTER(ProtocolTask);
#define THIS_FILE "ProtocolTask"

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
#include <cbor.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <drivers/spi.h>

#include "FrameworkIncludes.h"
#include "CborBuilder.h"
#include "BspSupport.h"
#include "ProtocolTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#ifndef PROTOCOL_TASK_PRIORITY
#define PROTOCOL_TASK_PRIORITY K_PRIO_PREEMPT(1)
#endif

#ifndef PROTOCOL_TASK_STACK_DEPTH
#define PROTOCOL_TASK_STACK_DEPTH 4096
#endif

#ifndef PROTOCOL_TASK_QUEUE_DEPTH
#define PROTOCOL_TASK_QUEUE_DEPTH 8
#endif

#define CBOR_START_BYTE 0xA4
#define CBOR_END_BYTE 0x30

typedef struct ProtocolTaskTag {
  FwkMsgTask_t msgTask; 
  bool data_received;
  bool data_transmitted;
  //UartMessage_t uartData;

} ProtocolTaskObj_t;
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static ProtocolTaskObj_t protocolTaskObject;

K_THREAD_STACK_DEFINE(protocolTaskStack, PROTOCOL_TASK_STACK_DEPTH);

K_MSGQ_DEFINE(protocolTaskQueue, FWK_QUEUE_ENTRY_SIZE,
	      PROTOCOL_TASK_QUEUE_DEPTH, FWK_QUEUE_ALIGNMENT);
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void ProtocolTaskThread(void *, void *, void *);
static void indent(int nestingLevel);
static void dumpbytes(const uint8_t *buf, size_t len);
//static CborError cbor_stream(void *token, const char *fmt, ...);
static CborError dumprecursive(CborValue *it, int nestingLevel);
static void encodeStringCbor(JsonMsg_t *pJsonMsg, uint16_t id);
static DispatchResult_t ReadRxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg);
/******************************************************************************/
/* Framework Message Dispatcher                                               */
/******************************************************************************/
static FwkMsgHandler_t ProtocolTaskMsgDispatcher(FwkMsgCode_t MsgCode)
{
	/* clang-format off */
	switch (MsgCode) {
  case FMC_INVALID:            return Framework_UnknownMsgHandler;
  case FMC_READ_BUFFER:        return ReadRxBufferMsgHandler;
  default:                     return NULL;
  }
	/* clang-format on */
}

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void ProtocolTask_Initialize(void)

{
  memset(&protocolTaskObject, 0, sizeof(ProtocolTaskObj_t));

  protocolTaskObject.msgTask.rxer.id               = FWK_ID_PROTOCOL_TASK;
  protocolTaskObject.msgTask.rxer.rxBlockTicks     = K_FOREVER;
	protocolTaskObject.msgTask.rxer.pMsgDispatcher =
		ProtocolTaskMsgDispatcher;
  protocolTaskObject.msgTask.timerDurationTicks    = K_MSEC(1000);
	protocolTaskObject.msgTask.timerPeriodTicks =
		K_MSEC(0); // 0 for one shot
  protocolTaskObject.msgTask.rxer.pQueue           = &protocolTaskQueue;
  
  Framework_RegisterTask(&protocolTaskObject.msgTask);
  
  protocolTaskObject.msgTask.pTid = 
    k_thread_create(&protocolTaskObject.msgTask.threadData, 
                    protocolTaskStack,
                    K_THREAD_STACK_SIZEOF(protocolTaskStack),
				ProtocolTaskThread, &protocolTaskObject, NULL,
				NULL, PROTOCOL_TASK_PRIORITY, 0, K_NO_WAIT);

  k_thread_name_set(protocolTaskObject.msgTask.pTid, THIS_FILE);
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void ProtocolTaskThread(void *pArg1, void *pArg2, void *pArg3)
{
	ProtocolTaskObj_t *pObj = (ProtocolTaskObj_t *)pArg1;

	while (true) {
		Framework_MsgReceiver(&pObj->msgTask.rxer);
	}
}
static void indent(int nestingLevel)
{
	while (nestingLevel--) {
        puts("  ");
	}
}
static void dumpbytes(const uint8_t *buf, size_t len)
{
	while (len--) {
        printf("%02X ", *buf++);
	}
}

#if 0
static CborError cbor_stream(void *token, const char *fmt, ...)
{
	va_list ap;

	(void)token;
	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);
	
	return CborNoError;
}
#endif

static CborError dumprecursive(CborValue *it, int nestingLevel)
{
    static bool idFound = false;
    static uint16_t idValue = 0;
    
	while (!cbor_value_at_end(it)) {
        CborError err;
        CborType type = cbor_value_get_type(it);

        indent(nestingLevel);
        switch (type) {
        case CborArrayType:
		case CborMapType: {
            // recursive type
            CborValue recursed;
            //assert(cbor_value_is_container(it));
            FRAMEWORK_ASSERT(cbor_value_is_container(it));

            puts(type == CborArrayType ? "Array[" : "Map[");
            
            err = cbor_value_enter_container(it, &recursed);
            if (err)
                return err;       // parse error
            err = dumprecursive(&recursed, nestingLevel + 1);
            if (err)
                return err;       // parse error
            err = cbor_value_leave_container(it, &recursed);
            if (err)
                return err;       // parse error
            indent(nestingLevel);
            puts("]");
            continue;
        }

		case CborIntegerType: {
            int64_t val;
            cbor_value_get_int64(it, &val);     // can't fail
            printf("%lld\n", (long long)val);
			if ((idFound == false) && (nestingLevel > 0)) {
                //This is the ID number
                idValue = val;
                idFound = true;
            }                
            break;
        }

        case CborByteStringType: {
            uint8_t *buf;
            size_t n;
            err = cbor_value_dup_byte_string(it, &buf, &n, it);
            if (err)
                return err;     // parse error
            dumpbytes(buf, n);
            puts("");
            free(buf);
            continue;
        }

		case CborTextStringType: {
            char *buf;
            size_t n;
            err = cbor_value_dup_text_string(it, &buf, &n, it);
            if (err)
                return err;     // parse error
            puts(buf);
            free(buf);
            continue;
        }

        case CborTagType: {
            CborTag tag;
            cbor_value_get_tag(it, &tag);       // can't fail
            printf("Tag(%lld)\n", (long long)tag);
            break;
        }

        case CborSimpleType: {
            uint8_t type;
            cbor_value_get_simple_type(it, &type);  // can't fail
            printf("simple(%u)\n", type);
            break;
        }

        case CborNullType:
            puts("null");
            break;

        case CborUndefinedType:
            puts("undefined");
            break;

        case CborBooleanType: {
            bool val;
            cbor_value_get_boolean(it, &val);       // can't fail
            puts(val ? "true" : "false");
            break;
        }

        case CborDoubleType: {
            double val;
            if (false) {
                float f;
        case CborFloatType:
                cbor_value_get_float(it, &f);
                val = f;
            } else {
                cbor_value_get_double(it, &val);
            }
            printf("%g\n", val);
            break;
        }
        case CborHalfFloatType: {
            uint16_t val;
            cbor_value_get_half_float(it, &val);
            printf("__f16(%04x)\n", val);
            break;
        }

        case CborInvalidType:
            assert(false);      // can't happen
            break;
        }

        err = cbor_value_advance_fixed(it);
        if (err)
            return err;
    }
    //This is the end of the message build the reply
	JsonMsg_t *pRsp = (JsonMsg_t *)BufferPool_Take(sizeof(JsonMsg_t));
	//    JsonBuilder_Start(pRsp, idValue);
	//    JsonBuilder_FinalizeOk(pRsp);
    encodeStringCbor(pRsp, idValue);

    return CborNoError;
}
static void encodeStringCbor(JsonMsg_t *pJsonMsg, uint16_t id)
{
    UartMsg_t *pUartMsg = BufferPool_Take(sizeof(UartMsg_t));
	/*
    uint8_t buf[25] ={0};
    CborEncoder encoder, mapEncoder;
    cbor_encoder_init(&encoder, &buf, sizeof(buf), 0);
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_text_stringz(&mapEncoder, "id");
    cbor_encode_uint (&mapEncoder, id);
    cbor_encode_text_stringz(&mapEncoder, "result");
    cbor_encode_text_stringz(&mapEncoder, "ok");
    cbor_encoder_close_container(&encoder, &mapEncoder);
*/
    CborBuilder_Start(pUartMsg, id);
    CborBuilder_FinalizeOk(pUartMsg);
    //memcpy(pUartMsg->buffer, &buf, sizeof(buf));
    //pUartMsg->size = sizeof(buf);
    
	if (pUartMsg != NULL) {
        //pUartMsg->size = systemUartTaskObject.uartData.size;//Bracket_Copy(pObj->pBracketObj, pUartMsg->buffer);
        pUartMsg->header.msgCode = FMC_TRANSMIT_BUFFER;
        pUartMsg->header.txId = FWK_ID_PROTOCOL_TASK;
        pUartMsg->header.rxId = FWK_ID_SYSTEM_UART_TASK;
        //memcpy(pUartMsg->buffer,systemUartTaskObject.uartData.buffer,systemUartTaskObject.uartData.size);
        FRAMEWORK_MSG_UNICAST(pUartMsg);
    }
}

static DispatchResult_t ReadRxBufferMsgHandler(FwkMsgReceiver_t *pMsgRxer,
					       FwkMsg_t *pMsg)
{
    UNUSED_PARAMETER(pMsgRxer);
	UartMsg_t *pCborMsg = (UartMsg_t *)pMsg;

	CborParser parser; 
	CborValue value;

	cbor_parser_init(pCborMsg->buffer, 
		pCborMsg->size, 0, &parser, &value);

	dumprecursive(&value, 0);

	//cbor_value_to_pretty_stream(cbor_stream,NULL,&value,
	//	CborPrettyDefaultFlags);
    return DISPATCH_OK;
}
