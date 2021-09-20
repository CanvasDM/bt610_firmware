/**
 * @file NonInitStruct.h
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __NON_INIT_STRUCT__H__
#define __NON_INIT_STRUCT_H__

#include <zephyr/types.h>
#include <stdbool.h>

#include "lcz_no_init_ram_var.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
/**
 * @note Items in non-initialized RAM need to survive a reset.
 */
typedef struct no_init_ram {
	no_init_ram_header_t header;
	uint32_t battery_age;
	uint32_t qrtc;
	uint32_t bootloader_time;
	bool attribute_save_pending;
} no_init_ram_t;
#define SIZE_OF_NIRD (sizeof(no_init_ram_t) - sizeof(no_init_ram_header_t))

extern no_init_ram_t *pnird;

#ifdef __cplusplus
}
#endif

#endif /* __NON_INIT_STRUCT_H__ */
