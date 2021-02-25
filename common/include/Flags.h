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
#define FLAG_LOW_BATTERY_ALARM         0x1, 7
#define FLAG_TEMP_HIGH_START_BIT       8
#define FLAG_HIGH_TEMP1_ALARM          0x3, 8
#define FLAG_HIGH_TEMP2_ALARM          0x3, 10
#define FLAG_HIGH_TEMP3_ALARM          0x3, 12
#define FLAG_HIGH_TEMP4_ALARM          0x3, 14
#define FLAG_TEMP_LOW_START_BIT        16
#define FLAG_LOW_TEMP1_ALARM           0x3, 16
#define FLAG_LOW_TEMP2_ALARM           0x3, 18
#define FLAG_LOW_TEMP3_ALARM           0x3, 20
#define FLAG_LOW_TEMP4_ALARM           0x3, 22
#define FLAG_TEMP_DELTA_START_BIT      24
#define FLAG_DELTA_TEMP1_ALARM         0x1, 24
#define FLAG_DELTA_TEMP2_ALARM         0x1, 25
#define FLAG_DELTA_TEMP3_ALARM         0x1, 26
#define FLAG_DELTA_TEMP4_ALARM         0x1, 27
#define FLAG_ANALOG_HIGH_START_BIT     28
#define FLAG_HIGH_ANALOG1_ALARM        0x3, 28
#define FLAG_HIGH_ANALOG2_ALARM        0x3, 30
#define FLAG_HIGH_ANALOG3_ALARM        0x3, 32
#define FLAG_HIGH_ANALOG4_ALARM        0x3, 34
#define FLAG_ANALOG_LOW_START_BIT      36
#define FLAG_LOW_ANALOG1_ALARM         0x3, 36
#define FLAG_LOW_ANALOG2_ALARM         0x3, 38
#define FLAG_LOW_ANALOG3_ALARM         0x3, 40
#define FLAG_LOW_ANALOG4_ALARM         0x3, 42
#define FLAG_ANALOG_DELTA_START_BIT    44
#define FLAG_DELTA_ANALOG1_ALARM       0x1, 44
#define FLAG_DELTA_ANALOG2_ALARM       0x1, 45
#define FLAG_DELTA_ANALOG3_ALARM       0x1, 46
#define FLAG_DELTA_ANALOG4_ALARM       0x1, 47
#define FLAG_DIGITAL_IN1_STATE         0x1, 48
#define FLAG_DIGITAL_IN2_STATE         0x1, 49
#define FLAG_MAGNET_STATE              0x1, 51
#define FLAG_TAMPER_SWITCH_STATE       0x1, 52

#define ANY_ALARM_MASK    0x00007F10
#define TEMP_ALARM_MASK   0x3
#define ANALOG_ALARM_MASK 0x3
#define DELTA_ALARM_MASK  0x1
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
