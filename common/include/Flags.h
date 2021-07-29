/**
 * @file Flags.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FLAGS_Hv__
#define __FLAGS_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define FLAG_TIME_WAS_SET              0x1, 0
#define FLAG_ACTIVE_MODE               0x1, 1
#define FLAG_ANY_ALARM                 0x1, 2
#define FLAG_TEMP_ALARM_START_BIT      3
#define FLAG_TEMP1_ALARM               0x1, 3
#define FLAG_TEMP2_ALARM               0x1, 4
#define FLAG_TEMP3_ALARM               0x1, 5
#define FLAG_TEMP4_ALARM               0x1, 6
#define FLAG_LOW_BATTERY_ALARM         0x1, 7
#define FLAG_ANALOG_ALARM_START_BIT    8
#define FLAG_ANALOG1_ALARM             0x1, 8
#define FLAG_ANALOG2_ALARM             0x1, 9
#define FLAG_ANALOG3_ALARM             0x1, 10
#define FLAG_ANALOG4_ALARM             0x1, 11
#define FLAG_DIGITAL_IN1_STATE         0x1, 12
#define FLAG_DIGITAL_IN2_STATE         0x1, 13
#define FLAG_TAMPER_SWITCH_STATE       0x1, 14
#define FLAG_MAGNET_STATE              0x1, 15

#define ANY_ALARM_MASK    0x00007F10
#define TEMP_ALARM_MASK   0x1
#define ANALOG_ALARM_MASK 0x1
/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
//-----------------------------------------------
//! @brief  This module uses a mutex.
//!
void Flags_Init(void);

//-----------------------------------------------
//! @brief  Set a single or multi-bit flag.  Handles setting of any alarm flag based upon
//! ANY_ALARM_MASK.
//!
void Flags_Set(uint32_t Mask, uint32_t Position, uint32_t Value);

//-----------------------------------------------
//! @retval The current value of the flags.
//!
uint32_t Flags_Get(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLAGS_H__ */
