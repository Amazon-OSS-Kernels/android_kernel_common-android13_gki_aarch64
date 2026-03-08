/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef _FUNC_H_
#define _FUNC_H_


typedef int (*fp_mp_mod_exp)(digit_t yout[], const digit_t x[], const digit_t e[], const digit_t m[], int ndigits);

typedef int (*fp_mp_conv_from_octets)(digit_t a[], int ndigits, const unsigned char *c, int nbytes);

typedef int (*fp_mp_equal)(const digit_t a[], const digit_t b[], int ndigits);

typedef void (*fp_null)(void);

//int mpSquare(digit_t w[], const digit_t x[], int ndigits)
typedef int (*fp_mp_square)(digit_t w[], const digit_t x[], int ndigits);

//int mpMultiply(digit_t w[], const digit_t u[], const digit_t v[], int ndigits)
typedef int (*fp_mp_multiply)(digit_t w[], const digit_t u[], const digit_t v[], int ndigits);

//int mpDivide(digit_t q[], digit_t r[], const digit_t u[], int udigits, digit_t v[], int vdigits)
typedef int (*fp_mp_divide)(digit_t q[], digit_t r[], const digit_t u[], int udigits, digit_t v[], int vdigits);

//digit_t mpShortDiv(digit_t q[], const digit_t u[], digit_t v, int ndigits)
typedef digit_t (*fp_mp_short_div)(digit_t q[], const digit_t u[], digit_t v, int ndigits);

//digit_t mpShiftLeft(digit_t a[], const digit_t *b, int shift, int ndigits)
typedef digit_t (*fp_mp_shift_left)(digit_t a[], const digit_t *b, int shift, int ndigits);

//digit_t mpShiftRight(digit_t a[], const digit_t b[], int shift, int ndigits)
typedef digit_t (*fp_mp_shift_right)(digit_t a[], const digit_t b[], int shift, int ndigits);

//int sp_multiply(digit_t p[2], digit_t x, digit_t y)
typedef int (*fp_sp_multiply)(digit_t p[2], digit_t x, digit_t y);

//digit_t sp_divide(digit_t *q, digit_t *r, const digit_t u[2], digit_t v)
typedef digit_t (*fp_sp_divide)(digit_t *q, digit_t *r, const digit_t u[2], digit_t v);

typedef struct
{
    fp_mp_conv_from_octets conv_from_octets;
    fp_mp_mod_exp       mod_exp;
    fp_mp_equal         equal;

    fp_mp_square        square;
    fp_mp_multiply      multiply;
    fp_mp_divide        divide;
    fp_mp_short_div     short_div;
    fp_mp_shift_left    shift_left;
    fp_mp_shift_right   shift_right;

    fp_sp_multiply      sp_multiply;
    fp_sp_divide        sp_divide;

    fp_null             reserve_1[13];
} mp_func_table;

extern mp_func_table        *mp_func;
extern mp_func_table        _mp_func;

#endif

