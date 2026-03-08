/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

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

