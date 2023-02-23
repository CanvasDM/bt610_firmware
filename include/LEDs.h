/**
 * @file LEDs.h
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __LEDS_H__
#define __LEDS_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        LED_COLOR_RED = 0,
        LED_COLOR_GREEN,
        LED_COLOR_AMBER,
        LED_COLOR_NONE
} ledColors_t;

typedef enum {
        LED_PATTERN_TAMPER = 0,
        LED_PATTERN_ALIVE,
        LED_PATTERN_EXIT_SHELF_MODE,
        LED_PATTERN_FACTORY_RESET,
        LED_PATTERN_BOOTLOADER_ACTIVE,
#if defined(CONFIG_LCZ_LWM2M_TRANSPORT_BLE_PERIPHERAL)
        LED_PATTERN_DM_CONNECTED
#endif
} ledPatterns_t;

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief Initialises the LEDs as outputs (off)
 */
void InitialiseLEDs(void);

/**
 * @brief Run LED test
 *
 * @param duration of each step in milliseconds
 */
void led_test(uint32_t delay);

/**
 * @brief Blinks LEDs in pattern
 *
 * @param LED(s) to use
 * @param pattern to use
 */
void led_blink(ledColors_t color, ledPatterns_t pattern);

/**
 * @brief Turns LEDs on
 *
 * @param LED(s) to use
 */
void led_on(ledColors_t color);

/**
 * @brief Turns LEDs off
 *
 * @param LED(s) to use
 */
void led_off(ledColors_t color);

#ifdef __cplusplus
}
#endif

#endif /* __LEDS_H__ */
