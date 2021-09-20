/**
 * @file NonInit.c
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <pm_config.h>

#include "NonInit.h"
#include "lcz_qrtc.h"

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
no_init_ram_t *pnird = (no_init_ram_t *)PM_LCZ_NOINIT_SRAM_ADDRESS;

void non_init_save_data(void)
{
	/* Save time to non-initialised section */
	pnird->qrtc = lcz_qrtc_get_epoch();
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);
}

void non_init_set_save_flag(bool status)
{
        pnird->attribute_save_pending = status;
        non_init_save_data();
}
