/**
 * @file UserCommTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __USER_COMM_TASK_H__
#define __USER_COMM_TASK_H__

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
typedef enum 
{
    UART_COMM = 0,
    I2C_COMM,
    SPI_CS1_COMM,
    SPI_CS2_COMM,
    NUMBER_COMMS,
}commType_t;

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 *  The setup of the thread parameters
 * @param
 * @param
 *
 * @retval
 */
void UserCommTask_Initialize(void);
/******************************************************************************/
/**
 * @brief
 *  Send a message out using one of the COMM ports (UART,SPI,I2C)
 * @param
 * @param
 *
 * @retval
 */
uint8_t UserCommTask_SendData(commType_t comm,
			  const uint8_t *data, size_t len);

//void UserCommTask_ConfigSPI(struct spi_config *spi_cfg);

#ifdef __cplusplus
}
#endif

#endif /* __TEMPLATE_H__ */
