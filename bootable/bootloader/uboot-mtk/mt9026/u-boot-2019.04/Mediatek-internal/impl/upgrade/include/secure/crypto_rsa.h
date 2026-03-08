/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _RSA_H_
#define _RSA_H_

#ifdef CRYPTO_RSA_C
#define EXTERN
#else
#define EXTERN extern
#endif

//-------------------------------------------------------------------------------------------------
// Type definition
//-------------------------------------------------------------------------------------------------
typedef unsigned int                DIGIT_T;
#define DIGIT_S                     sizeof(DIGIT_T)
#define RSA_KEY_DIGI_LEN            (256)                                // digit_t count
typedef unsigned int                digit_t;

EXTERN int _mp_conv_from_octets(digit_t a[], int ndigits, const unsigned char *c, int nbytes);
EXTERN int _mp_mod_exp(digit_t yout[], const digit_t x[], const digit_t e[], const digit_t m[], int ndigits);
EXTERN int _mp_equal(const digit_t a[], const digit_t b[], int ndigits);
EXTERN int _mp_square(digit_t w[], const digit_t x[], int ndigits);
EXTERN int _mp_multiply(digit_t w[], const digit_t u[], const digit_t v[], int ndigits);
EXTERN int _mp_divide(digit_t q[], digit_t r[], const digit_t u[], int udigits, digit_t v[], int vdigits);
EXTERN digit_t _mp_short_div(digit_t q[], const digit_t u[], digit_t v, int ndigits);
EXTERN digit_t _mp_shift_left(digit_t a[], const digit_t *b, int shift, int ndigits);
EXTERN digit_t _mp_shift_right(digit_t a[], const digit_t b[], int shift, int ndigits);
EXTERN int _sp_multiply(digit_t p[2], digit_t x, digit_t y);
EXTERN digit_t _sp_divide(digit_t *q, digit_t *r, const digit_t u[2], digit_t v);
EXTERN int rsa_main(unsigned char *signature, unsigned char *public_key_n, unsigned char *public_key_e, unsigned char *sim_sign_out);

#undef EXTERN

#endif

