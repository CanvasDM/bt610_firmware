/**
 * @file EventLog.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(EventLog, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "EventLog.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int EventLog_Initialize(void)
{
	return -1;
}

int EventLog_Append(void) // msg + mask
{
	// is size > max size
	return -1;
}

int EventLog_Prepare(void)
{
	// determine how many logs there are
	// increment active log index
	return -1;
}

int EventLog_Ack(void)
{
	// delete log
	return -1;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
int increment_active_index(void)
{
	// has the maximum number of logs been reached
	// delete next log
	return -1;
}
