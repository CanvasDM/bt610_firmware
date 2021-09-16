/**
 * @file BspSupport.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BSP_SUPPORT_H__
#define __BSP_SUPPORT_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>
#include <drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
/* clang-format off */
/* Inputs */
#define UART_0_CTS_PIN (7)  /* SIO_7  Port0 */
#define DIN1_MCU_PIN   (9)  /* SIO_09 Port0 */
#define DIN2_MCU_PIN   (43) /* SIO_43 Port1 */
#define MAGNET_MCU_PIN (47) /* SIO_47 Port1 */
#define UART_0_RXD_PIN (5)  /* SIO_05 Port0 */
#define SW1_PIN        (24) /* SIO_24 Port0 */
#define SW2_PIN        (33) /* SIO_33 Port1 */

/* Digital Input pin masks - combined in the digitalInput attribute */
#define DIN1_PIN_MASK 0x1
#define DIN2_PIN_MASK 0x2

/* Outputs */
/* PORT0 */
#define UART_0_RTS_PIN       (5)  /* SIO_05 Port0 */
#define THERM_ENABLE_PIN     (10) /* SIO_10 Port0 */
#define DO2_PIN              (11) /* SIO_11 Port0 */
#define DO1_PIN              (12) /* SIO_12 Port0 */
#define BATT_OUT_ENABLE_PIN  (30) /* SIO_30 Port0 */
/* PORT1 */
#define DIN1_ENABLE_PIN      (37) /* SIO_37 Port1 */
#define FIVE_VOLT_ENABLE_PIN (44) /* SIO_44 Port1 */
#define DIN2_ENABLE_PIN      (42) /* SIO_42 Port1 */
#define ANALOG_ENABLE_PIN    (45) /* SIO_45 Port1 */
/* clang-format on */

#define GPIO_PER_PORT 32
#define GPIO_PIN_MAX 47

#define GPIO_PIN_MAP(p) ((p > GPIO_PER_PORT) ? (p - GPIO_PER_PORT) : p)

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 *
 * @param
 * @param
 *
 * @retval
 */
void BSP_Init(void);
int BSP_PinSet(uint8_t pin, int value);
int BSP_PinToggle(uint8_t pin);
int BSP_PinGet(uint8_t pin);
void BSP_DigitalPinsStatus(void);
void BSP_ConfigureDigitalInputs(uint8_t pin, gpio_flags_t enable,
				gpio_flags_t edge);
int BSP_UpdateDigitalInput1SimulatedStatus(bool simulation_enabled,
					   bool last_simulation_enabled);
int BSP_UpdateDigitalInput2SimulatedStatus(bool simulation_enabled,
					   bool last_simulation_enabled);
int BSP_UpdateDigitalInput1SimulatedValue(bool simulated_value,
					  bool last_simulated_value);
int BSP_UpdateDigitalInput2SimulatedValue(bool simulated_value,
					  bool last_simulated_value);
#ifdef __cplusplus
}
#endif

#endif /* __BSP_SUPPORT_H__ */
