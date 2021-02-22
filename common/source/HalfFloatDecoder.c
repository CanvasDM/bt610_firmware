/**
 * @file HalfFloatDecoder.c
 * @brief Half Float decoding routine for use with incoming CBOR messages.
 *
 *        Refer to https://github.com/ekmett/half
 *        Used with permission of Edward Kmett.
 *
 * Copyright 2014 Edward Kmett
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include "HalfFloatDecoder.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define HALF_FLOAT_DECODER_HALF_PRECISION_SIGN_ROTATES			15
#define HALF_FLOAT_DECODER_HALF_PRECISION_EXPONENT_ROTATES		10
#define HALF_FLOAT_DECODER_HALF_PRECISION_MANTISSA_MASK			0x3FF
#define HALF_FLOAT_DECODER_HALF_PRECISION_DENORMALISED_MANTISSA_MASK	0x400
#define HALF_FLOAT_DECODER_HALF_PRECISION_EXPONENT_MAX			31
#define HALF_FLOAT_DECODER_SINGLE_PRECISION_SIGN_ROTATES		31
#define HALF_FLOAT_DECODER_SINGLE_PRECISION_NAN_MASK			0x7f800000
#define HALF_FLOAT_DECODER_SINGLE_PRECISION_MANTISSA_ROTATES		13
#define HALF_FLOAT_DECODER_SINGLE_PRECISION_EXPONENT_CORRECTION		112
#define HALF_FLOAT_DECODER_SINGLE_PRECISION_EXPONENT_ROTATES		23

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static uint32_t HalfToFloatPrivate (uint16_t c);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/

/**@brief Public interface to Half-float to Single Precision floating point
 *        conversion.
 *
 * @param [in]c - The half-precision floating point value to convert.
 * @retval The single precision floating point result.
 */
float HalfToFloat(uint16_t c) {

	union { float d; uint32_t i; } u;
	u.i = HalfToFloatPrivate(c);
	return u.d;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/

/**@brief Private interface to Half-float to Single Precision floating point
 *        conversion.
 *
 * @param [in]c - The half-precision floating point value to convert.
 * @retval The four single precision floating point bytes.
 */
static uint32_t HalfToFloatPrivate (uint16_t c) {

	uint32_t s = (c >> HALF_FLOAT_DECODER_HALF_PRECISION_SIGN_ROTATES) & 0x001;
	uint32_t e = (c >> HALF_FLOAT_DECODER_HALF_PRECISION_EXPONENT_ROTATES) & 0x01f;
	uint32_t m = c & HALF_FLOAT_DECODER_HALF_PRECISION_MANTISSA_MASK;
	if (e == 0) {
		if (m == 0){
			/* +/- 0 */
			return (s << HALF_FLOAT_DECODER_SINGLE_PRECISION_SIGN_ROTATES);
                }
		/* denormalized, renormalize it */
		while (!(m & HALF_FLOAT_DECODER_HALF_PRECISION_DENORMALISED_MANTISSA_MASK)) {
			m <<= 1;
			e -=  1;
		}
		e += 1;
		m &= ~HALF_FLOAT_DECODER_HALF_PRECISION_DENORMALISED_MANTISSA_MASK;
	}
	else if (e == HALF_FLOAT_DECODER_HALF_PRECISION_EXPONENT_MAX){
		/* NaN or +/- infinity */
		return (s << HALF_FLOAT_DECODER_SINGLE_PRECISION_SIGN_ROTATES)|
			HALF_FLOAT_DECODER_SINGLE_PRECISION_NAN_MASK|
				(m << HALF_FLOAT_DECODER_SINGLE_PRECISION_MANTISSA_ROTATES);
        }
	e += HALF_FLOAT_DECODER_SINGLE_PRECISION_EXPONENT_CORRECTION;
	m <<= HALF_FLOAT_DECODER_SINGLE_PRECISION_MANTISSA_ROTATES;
	return (s << HALF_FLOAT_DECODER_SINGLE_PRECISION_SIGN_ROTATES)|
		(e << HALF_FLOAT_DECODER_SINGLE_PRECISION_EXPONENT_ROTATES)|
			m;
}
