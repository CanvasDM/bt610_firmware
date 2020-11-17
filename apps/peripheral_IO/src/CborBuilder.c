/******************************************************************************/
//!
#define THIS_FILE "CborBuilder.c"
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <sys/util.h>
#include <cbor.h>

#include "FrameworkIncludes.h"
#include "CborBuilder.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

#define JSON_PROTOCOL_VERSION 2

#define MAX_B64_OUTPUT_LENGTH (1368 + 1)
#define MAXIMUM_LENGTH_OF_TO_STRING_OUTPUT                                                  \
	11 /* Includes                                                                   */ \
		a NUL character

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/

static CborEncoder encoder;
static CborEncoder mapEncoder;

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void CborBuilder_Start(UartMsg_t *pMsg, uint32_t Id)
{
	FRAMEWORK_ASSERT(pMsg != NULL);

    memset(pMsg->buffer, 0, UART_BUFFER_SIZE);
    pMsg->size = 0;
	cbor_encoder_init(&encoder, (cbor_encoder_writer *)pMsg->buffer, 0);
    cbor_encoder_create_map(&encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_text_stringz(&mapEncoder, "id");
	cbor_encode_uint(&mapEncoder, Id);
}

void CborBuilder_FinalizeOk(UartMsg_t *pMsg)
{
 // pJsonMsg->wasValid = true;
  
	FRAMEWORK_ASSERT(pMsg != NULL);
	FRAMEWORK_ASSERT(pMsg->size >= 15); // start must be called first
  
  cbor_encode_text_stringz(&mapEncoder, "result");
  cbor_encode_text_stringz(&mapEncoder, "ok");
  cbor_encoder_close_container(&encoder, &mapEncoder);
	pMsg->size = encoder.writer->bytes_written;
}

void CborBuilder_FinalizeIntegerResult(UartMsg_t *pJsonMsg, uint32_t Value)
{
  //pJsonMsg->wasValid = true;
  
	FRAMEWORK_ASSERT(pJsonMsg != NULL);
	FRAMEWORK_ASSERT(pJsonMsg->size >= 15); // start must be called first
	FRAMEWORK_ASSERT(pJsonMsg->buffer[pJsonMsg->size - 1] == ',');
}

void CborBuilder_FinalizeBase64Result(UartMsg_t *pJsonMsg, void *pData,
				      uint32_t Size)
{
 // pJsonMsg->wasValid = true;
  
	FRAMEWORK_ASSERT(pJsonMsg != NULL);
	FRAMEWORK_ASSERT(pJsonMsg->size >= 15); // start must be called first
	FRAMEWORK_ASSERT(pJsonMsg->buffer[pJsonMsg->size - 1] == ',');
}
void CborBuilder_FinalizeError(UartMsg_t *pJsonMsg, uint32_t Code,
			       char *pString)
{
 // pJsonMsg->wasValid = false;

	FRAMEWORK_ASSERT(pJsonMsg != NULL);
	FRAMEWORK_ASSERT(pJsonMsg->size >= 15); // start must be called first
	FRAMEWORK_ASSERT(pJsonMsg->buffer[pJsonMsg->size - 1] == ',');
	FRAMEWORK_ASSERT(pString != NULL);
	FRAMEWORK_ASSERT(strlen(pString) > 0);
}

void CborBuilder_AddPair(UartMsg_t *pJsonMsg, char *pKey, char *pValue,
			 bool IsNotString)
{
	FRAMEWORK_ASSERT(pJsonMsg != NULL);
	FRAMEWORK_ASSERT(pKey != NULL);
	FRAMEWORK_ASSERT(pValue != NULL);
	FRAMEWORK_ASSERT(strlen(pKey) > 0);
}

/******************************************************************************/
// Local Function Definitions
/******************************************************************************/

// end
