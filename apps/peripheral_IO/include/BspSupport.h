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

/* (Remove Empty Sections) */
/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
//***Inputs*****
#define DIN1_MCU_PIN               (9) //SIO_09 Port0
#define DIN2_MCU_PIN               (43)//SIO_43 Port1

//***Output*****
//PORT0
#define THERM_ENABLE_PIN           (10)//SIO_10 Port0
#define DO2_PIN                    (11)//SIO_11 Port0
#define DO1_PIN                    (12)//SIO_12 Port0
#define BATT_OUT_ENABLE_PIN        (30)//SIO_30 Port0
//PORT1
#define DIN1_ENABLE_PIN            (5)//SIO_37 Port1
#define FIVE_VOLT_ENABLE_PIN       (12)//SIO_44 Port1
#define DIN2_ENABLE_PIN            (10)//SIO_42 Port1
#define ANALOG_ENABLE_PIN          (13)//SIO_45 Port1

#define GPIO_PIN_MAP(p)           ((p>32)?(p-32):p)

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
uint16_t BSP_PinSet(uint8_t pin, uint16_t value);
#ifdef __cplusplus
}
#endif

#endif /* __BSP_SUPPORT_H__ */
