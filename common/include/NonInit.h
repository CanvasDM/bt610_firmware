/**
 * @file NonInit.h
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __NON_INIT_H__
#define __NON_INIT_H__

#include <zephyr/types.h>
#include <stdbool.h>

#include "NonInitStruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
#if defined(CONFIG_MCUBOOT)
/**
 * @brief Saves bootloader start up time to non-init RAM section and updates
 * the header
 *
 * @param Run time of mcuboot in ms
 */
void non_init_set_bootloader_time(uint32_t time);
#else
/**
 * @brief Saves epoch to non-init RAM section and upates the header
 */
void non_init_save_data(void);

/**
 * @brief Sets the unsaved data present flag to be set or unset and saves this
 * to the non-initialised area in RAM
 *
 * @param true if there is pending unsaved data, false otherwide
 */
void non_init_set_save_flag(bool status);
/**
 * @brief Saves epoch of zero to non-init RAM section and upates the header
 */
void non_init_clear_qrtc(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __NON_INIT_H__ */
