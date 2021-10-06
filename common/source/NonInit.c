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
#if defined(CONFIG_MCUBOOT)
#include <string.h>
#else
#include "lcz_qrtc.h"
#endif

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
no_init_ram_t *pnird = (no_init_ram_t *)PM_LCZ_NOINIT_SRAM_ADDRESS;

#if defined(CONFIG_MCUBOOT)
void non_init_set_bootloader_time(uint32_t time)
{
	bool valid = lcz_no_init_ram_var_is_valid(pnird, SIZE_OF_NIRD);
	if (!valid) {
		/* If the header is not valid, this write would make it valid,
		 * therefore clear all data prior to saving the time so that
		 * the booted application knows there is no data
		 */
		memset(pnird, 0, sizeof(no_init_ram_t));
	}

	pnird->bootloader_time = time;
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);
}
#else
void non_init_save_data(void)
{
	/* Save time to non-initialised section */
	pnird->qrtc = lcz_qrtc_get_epoch();
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);
}
void non_init_clear_qrtc(void)
{
	/* Save time to be zero in the non-initialised section */
	pnird->qrtc = 0;
	lcz_no_init_ram_var_update_header(pnird, SIZE_OF_NIRD);
}
void non_init_set_save_flag(bool status)
{
        pnird->attribute_save_pending = status;
        non_init_save_data();
}
#endif
