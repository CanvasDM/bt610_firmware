 /**
 * @file attr_custom_validator.h
 * @brief Validators custom to a particular project.
 *
 * Copyright (c) 2022 Laird Connectivity LLC
 *
 * SPDX-License-Identifier: LicenseRef-LairdConnectivity-Clause
 */

#ifndef __ATTR_CUSTOM_VALIDATOR_H__
#define __ATTR_CUSTOM_VALIDATOR_H__

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <zephyr/types.h>
#include <stddef.h>

#include "attr_defs.h"
#include "attr_custom_validator_bluetooth_flags.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/
/* Global Function Prototypes                                                                     */
/**************************************************************************************************/
/**
 * @param do_write true if attribute should be changed, false if pv should
 * be validated but not written.
 *
 * @note Power must be set to broadcast (and subsequently updated in radio).
 *
 * @retval Validators return negative error code, 0 on success
 */
int av_tx_power(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_aic(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_din1simen(const ate_t *const entry, void *pv, size_t vlen,
		 bool do_write);

int av_din1sim(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_din2simen(const ate_t *const entry, void *pv, size_t vlen,
		 bool do_write);

int av_din2sim(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_magsimen(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_magsim(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_tampsimen(const ate_t *const entry, void *pv, size_t vlen,
		 bool do_write);

int av_tampsim(const ate_t *const entry, void *pv, size_t vlen, bool do_write);

int av_block_downgrades(const ate_t *const entry, void *pv, size_t vlen,
			bool do_write);

#ifdef __cplusplus
}
#endif

#endif /* __ATTR_CUSTOM_VALIDATOR_H__ */
