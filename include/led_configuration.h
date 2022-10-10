/**
 * @file led_configuration.h
 * @brief The BT6xx has a red + green LED.
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LED_CONFIGURATION_H__
#define __LED_CONFIGURATION_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include "lcz_led.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Board definitions                                                          */
/******************************************************************************/

enum led_index { GREEN_LED, RED_LED };

#ifdef __cplusplus
}
#endif

#endif /* __LED_CONFIGURATION_H__ */
