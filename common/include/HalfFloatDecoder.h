/**
 * @file HalfFloatDecoder.h
 * @brief Interface to Half Float decoding routine for use with incoming CBOR
 *        messages.
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
#ifdef __HALF_FLOAT_DECODER__
	#error "HalfFloatDecoder.h Error - HalfFloatDecoder.h is already included."
#endif

#define __HALF_FLOAT_DECODER__

#ifndef __ZEPHYR__
	#error "HalfFloatDecoder.h Error - Zephyr.h must be included first."
#endif

/**@brief Public interface to Half-float to Single Precision floating point
 *        conversion.
 *
 * @param [in]c - The half-precision floating point value to convert.
 * @retval The single precision floating point result.
 */
float HalfToFloat(uint16_t c);
