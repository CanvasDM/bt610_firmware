/**
 * @file LEDs.c
 * @brief LED interopability layer
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
#if !defined(CONFIG_MCUBOOT)
LOG_MODULE_REGISTER(leds, CONFIG_UI_TASK_LOG_LEVEL);
#else
LOG_MODULE_REGISTER(leds, 0);
#endif
#define THIS_FILE "leds"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#include "LEDs.h"
#include "led_configuration.h"
#if defined(CONFIG_LCZ_PWM_LED)
#include "lcz_pwm_led.h"
#endif

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

/* clang-format off */
#define LED0_DEV   DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_FLAGS DT_GPIO_FLAGS(LED0_NODE, gpios)
#define LED0       DT_GPIO_PIN(LED0_NODE, gpios)
#define LED1_DEV   DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1_FLAGS DT_GPIO_FLAGS(LED1_NODE, gpios)
#define LED1       DT_GPIO_PIN(LED1_NODE, gpios)
/* clang-format on */

#define ADVERTISE_30SEC_TIMER 30

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void green_led_on(void);
static void green_led_off(void);
static void red_led_on(void);
static void red_led_off(void);
#endif

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static const lcz_led_configuration_t LED_CONFIGURATION[] = { { GREEN_LED, green_led_on,
							       green_led_off },
							     { RED_LED, red_led_on, red_led_off } };
#else
static const lcz_led_configuration_t LED_CONFIGURATION[] = {
	{ GREEN_LED, LED1_DEV, LED1, LED1_FLAGS },
	{ RED_LED, LED0_DEV, LED0, LED0_FLAGS }
};
#endif

/* clang-format off */
static const struct lcz_led_blink_pattern ALIVE_PATTERN = {
	.on_time = 1000,
	.off_time = 0,
	.repeat_count = 0
};

static const struct lcz_led_blink_pattern TAMPER_PATTERN = {
	.on_time = 50,
	.off_time = 1950,
	.repeat_count = 60-1
};

static const struct lcz_led_blink_pattern EXIT_SHELF_MODE_PATTERN = {
	.on_time = 100,
	.off_time = 900,
	.repeat_count = ADVERTISE_30SEC_TIMER - 1
};

static const struct lcz_led_blink_pattern FACTORY_RESET_PATTERN = {
	.on_time = 5000,
	.off_time = 0,
	.repeat_count = 0
};

static const struct lcz_led_blink_pattern BOOTLOADER_ACTIVE_PATTERN = {
	.on_time = 500,
	.off_time = 500,
	.repeat_count = REPEAT_INDEFINITELY
};
/* clang-format on */

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
void InitialiseLEDs(void)
{
	lcz_led_init((lcz_led_configuration_t *)LED_CONFIGURATION, ARRAY_SIZE(LED_CONFIGURATION));
}

void led_blink(ledColors_t color, ledPatterns_t pattern)
{
	struct lcz_led_blink_pattern const *pPattern;
	/* Turn all LEDs off */
	led_off(LED_COLOR_NONE);

	if (pattern == LED_PATTERN_ALIVE) {
		pPattern = &ALIVE_PATTERN;
	} else if (pattern == LED_PATTERN_TAMPER) {
		pPattern = &TAMPER_PATTERN;
	} else if (pattern == LED_PATTERN_EXIT_SHELF_MODE) {
		pPattern = &EXIT_SHELF_MODE_PATTERN;
	} else if (pattern == LED_PATTERN_FACTORY_RESET) {
		pPattern = &FACTORY_RESET_PATTERN;
	} else if (pattern == LED_PATTERN_BOOTLOADER_ACTIVE) {
		pPattern = &BOOTLOADER_ACTIVE_PATTERN;
	} else {
		LOG_ERR("Invalid LED pattern: %d", pattern);
		return;
	}

	if (color == LED_COLOR_GREEN) {
		lcz_led_blink(GREEN_LED, pPattern);
	} else if (color == LED_COLOR_RED) {
		lcz_led_blink(RED_LED, pPattern);
	} else if (color == LED_COLOR_AMBER) {
		lcz_led_blink(RED_LED, pPattern);
		lcz_led_blink(GREEN_LED, pPattern);
	}
}

void led_on(ledColors_t color)
{
	/* Turn all LEDs off */
	led_off(LED_COLOR_NONE);

	if (color == LED_COLOR_GREEN) {
		lcz_led_turn_on(GREEN_LED);
	} else if (color == LED_COLOR_RED) {
		lcz_led_turn_on(RED_LED);
	} else if (color == LED_COLOR_AMBER) {
		lcz_led_turn_on(GREEN_LED);
		lcz_led_turn_on(RED_LED);
	}
}

void led_off(ledColors_t color)
{
	if (color == LED_COLOR_GREEN) {
#if defined(CONFIG_LCZ_PWM_LED)
		lcz_pwm_led_off(GREEN_LED);
#endif
		lcz_led_turn_off(GREEN_LED);
	} else if (color == LED_COLOR_RED) {
#if defined(CONFIG_LCZ_PWM_LED)
		lcz_pwm_led_off(RED_LED);
#endif
		lcz_led_turn_off(RED_LED);
	} else if ((color == LED_COLOR_NONE) || (color == LED_COLOR_AMBER)) {
		/* Both LEDs will be turned off */
#if defined(CONFIG_LCZ_PWM_LED)
		lcz_pwm_led_off(GREEN_LED);
		lcz_pwm_led_off(RED_LED);
#endif
		lcz_led_turn_off(GREEN_LED);
		lcz_led_turn_off(RED_LED);
	}
}

void led_test(uint32_t delay)
{
	lcz_led_turn_on(GREEN_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_on(RED_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_off(GREEN_LED);
	k_sleep(K_MSEC(delay));

	lcz_led_turn_off(RED_LED);
	k_sleep(K_MSEC(delay));
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
#ifdef CONFIG_LCZ_LED_CUSTOM_ON_OFF
static void green_led_on(void)
{
	lcz_pwm_led_on(GREEN_LED, 1500, 700);
}

static void green_led_off(void)
{
	lcz_pwm_led_off(GREEN_LED);
}

static void red_led_on(void)
{
	lcz_pwm_led_on(RED_LED, 1500, 700);
}

static void red_led_off(void)
{
	lcz_pwm_led_off(RED_LED);
}
#endif

#if defined(CONFIG_MCUBOOT)
static int bootloader_leds_init(const struct device *device)
{
	/* This is used by the bootloader at start-up to switch between the
	 * LEDs to indicate that it is busy with an operation
	 */
	ARG_UNUSED(device);
	InitialiseLEDs();
	led_blink(LED_COLOR_AMBER, LED_PATTERN_BOOTLOADER_ACTIVE);

	return 0;
}

SYS_INIT(bootloader_leds_init, APPLICATION, 0);
#endif
