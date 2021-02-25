/**
 * @file Flags.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(Flags, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "Attribute.h"
#include "Flags.h"
#include "SensorTask.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
K_MUTEX_DEFINE(flags_mutex);

struct {
	uint32_t data;

} flags;
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void SetFlag(uint32_t Mask, uint32_t Position, uint32_t Value);
/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void Flags_Init(void)
{
	flags.data = 0;
}

void Flags_Set(uint32_t Mask, uint32_t Position, uint32_t Value)
{
	k_mutex_lock(&flags_mutex, K_FOREVER);
	SetFlag(Mask, Position, Value);
	if ((flags.data & ANY_ALARM_MASK) != 0) {
		SetFlag(FLAG_ANY_ALARM, 1);
	}
	k_mutex_unlock(&flags_mutex);
	/* The flags are used in the advertisment and shadowed in the attribute table.*/
	Attribute_SetUint32(ATTR_INDEX_flags, flags.data);
}

uint32_t Flags_Get(void)
{
	return flags.data;
}
/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void SetFlag(uint32_t Mask, uint32_t Position, uint32_t Value)
{
	flags.data &= ~(Mask << Position);
	flags.data |= (Value & Mask) << Position;
}
/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
