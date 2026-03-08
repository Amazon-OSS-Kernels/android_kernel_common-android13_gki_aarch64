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

#define CRYPTO_RSA_C
#include <secure/crypto_rsa.h>
#include <secure/crypto_func.h>
#include <secure/crypto_sha.h>
#include <secure/secure_common.h>
#include <common.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <MsTypes.h>
#include <upgrade_utility.h>
#include "u-boot/drv_rsa.h"


#if !defined(CONFIG_SECURE_HW_IP)
#define MAX_DIGIT                   0xFFFFFFFFUL
#define MAX_HALF_DIGIT              0xFFFFUL	/* NB 'L' */
#define BITS_PER_DIGIT              32
#define HIBITMASK                   0x80000000UL

#define BITS_PER_HALF_DIGIT         (BITS_PER_DIGIT / 2)
#define BYTES_PER_DIGIT             (BITS_PER_DIGIT / 8)

#define LOHALF(x)                   ((digit_t)((x) & MAX_HALF_DIGIT))
#define HIHALF(x)                   ((digit_t)((x) >> BITS_PER_HALF_DIGIT & MAX_HALF_DIGIT))
#define TOHIGH(x)                   ((digit_t)((x) << BITS_PER_HALF_DIGIT))

#define mp_next_bit_mask(mask, n) do{if(mask==1){mask=HIBITMASK;n--;}else{mask>>=1;}}while(0)
#endif


#if defined(CONFIG_SECURE_HW_IP)
void rsa_copy_reverse(unsigned char *sim_sign_out, unsigned char *rsa_out, int len)
{
    int i=0;
    for(i=0; i<len; i++)
    {
        sim_sign_out[i] = rsa_out[len-i-1];
    }
}

int secure_rsa2048decrypt_hw(unsigned char *signature, unsigned char *public_key_n, unsigned char *public_key_e, unsigned char *sim_sign_out)
{
    int result = 0;
    unsigned int timeout_count = 0;
    Drv_RSASig rsa_sig;
    Drv_RSAOut rsa_out;
    Drv_RSAKey rsa_key;

    UBOOT_TRACE("IN\n");
    if(NULL == signature || NULL == public_key_n || NULL == public_key_e || NULL == sim_sign_out)
    {
        UBOOT_ERROR("parameters Error\n");
        return -1;
    }

    memcpy(&rsa_sig, signature, sizeof(rsa_sig));
    memset(&rsa_out, 0, sizeof(rsa_out));
    memset(&rsa_key, 0, sizeof(rsa_key));
    memcpy((unsigned char*)(rsa_key.KeyN), public_key_n, RSA_PUBLIC_KEY_N_LEN);
    memcpy((unsigned char*)(rsa_key.KeyE), public_key_e, RSA_PUBLIC_KEY_E_LEN);
    flush_cache((unsigned long)&rsa_sig, SIGNATURE_LEN);

    result = HW_RSA_Calculate(&rsa_sig, &rsa_key, E_DRV_RSA2048_PUBLIC);
    if(result != 0)
    {
        UBOOT_DEBUG("RSA HW decrypt Calculate error\n");
        return -1;
    }

    memset((unsigned char*)&rsa_out, 0, sizeof(Drv_RSAOut));
    while(HW_RSA_IsFinished() != 0)
    {
        udelay(RSA_DELAY_10);
        timeout_count++;
        if(timeout_count > RSA_TIMEOUT_VALUE)
        {
            return -1;
        }
    }

    result = HW_RSA_Output(E_DRV_RSA2048_PUBLIC, &rsa_out);
    if(result != 0)
    {
        UBOOT_DEBUG("RSA HW decrypt Output error\n");
        return -1;
    }
    flush_cache((unsigned long)(&rsa_out), SIGNATURE_LEN);

    // the output from MDrv_RSA_Output is reverse order
    // need to be reversed again to normal order.
    rsa_copy_reverse(sim_sign_out, (unsigned char *)&rsa_out, sizeof(rsa_out));
    UBOOT_TRACE("OK\n");
    return 0;
}
#else
int _sp_multiply(digit_t p[2], digit_t x, digit_t y)
{
    /*  Computes p = x * y */
    /*  Ref: Arbitrary Precision Computation
    http://numbers.computation.free.fr/Constants/constants.html

         high    p1                p0     low
        +--------+--------+--------+--------+
        |      x1*y1      |      x0*y0      |
        +--------+--------+--------+--------+
               +-+--------+--------+
               |1| (x0*y1 + x1*y1) |
               +-+--------+--------+
                ^carry from adding (x0*y1+x1*y1) together
                        +-+
                        |1|< carry from adding LOHALF t
                        +-+  to high half of p0
    */
    digit_t x0, y0, x1, y1;
    digit_t t, u, carry;

    /*  Split each x,y into two halves
        x = x0 + B*x1
        y = y0 + B*y1
        where B = 2^16, half the digit size
        Product is
        xy = x0y0 + B(x0y1 + x1y0) + B^2(x1y1)
    */

    x0 = LOHALF(x);
    x1 = HIHALF(x);
    y0 = LOHALF(y);
    y1 = HIHALF(y);

    /* Calc low part - no carry */
    p[0] = x0 * y0;
    /* Calc middle part */
    t = x0 * y1;
    u = x1 * y0;
    t += u;
    if (t < u)
    {
        carry = 1;
    }
    else
    {
        carry = 0;
    }

    /*  This carry will go to high half of p[1]
        + high half of t into low half of p[1] */
    carry = TOHIGH(carry) + HIHALF(t);
    /* Add low half of t to high half of p[0] */
    t = TOHIGH(t);
    p[0] += t;
    if (p[0] < t)
    {
        carry++;
    }

    p[1] = x1 * y1;
    p[1] += carry;

    return 0;
}

static void _sp_mult_sub(digit_t uu[2], digit_t qhat, digit_t v1, digit_t v0)
{
    /*  Compute uu = uu - q(v1v0)
        where uu = u3u2u1u0, u3 = 0
        and u_n, v_n are all half-digits
        even though v1, v2 are passed as full digits.
    */
    digit_t p0, p1, t;

    p0 = qhat * v0;
    p1 = qhat * v1;
    t = p0 + TOHIGH(LOHALF(p1));
    uu[0] -= t;
    if (uu[0] > MAX_DIGIT - t)
    {
        uu[1]--;    /* Borrow */
    }
    uu[1] -= HIHALF(p1);
}

digit_t _sp_divide(digit_t *q, digit_t *r, const digit_t u[2], digit_t v)
{   /*  Computes quotient q = u / v, remainder r = u mod v
        where u is a double digit
        and q, v, r are single precision digits.
        Returns high digit of quotient (max value is 1)
        CAUTION: Assumes normalised such that v1 >= b/2
        where b is size of HALF_DIGIT
        i.e. the most significant bit of v should be one

        In terms of half-digits in Knuth notation:
        (q2q1q0) = (u4u3u2u1u0) / (v1v0)
        (r1r0) = (u4u3u2u1u0) mod (v1v0)
        for m = 2, n = 2 where u4 = 0
        q2 is either 0 or 1.
        We set q = (q1q0) and return q2 as "overflow"
    */
    digit_t qhat, rhat, t, v0, v1, u0, u1, u2, u3;
    digit_t uu[2], q2;

    /* Check for normalisation */
    if (!(v & HIBITMASK))
    {   /* Stop if assert is working, else return error */
        //assert(v & HIBITMASK);
        *q = *r = 0;
        return MAX_DIGIT;
    }

    /* Split up into half-digits */
    v0 = LOHALF(v);
    v1 = HIHALF(v);
    u0 = LOHALF(u[0]);
    u1 = HIHALF(u[0]);
    u2 = LOHALF(u[1]);
    u3 = HIHALF(u[1]);

    /* Do three rounds of Knuth Algorithm D Vol 2 p272 */

    /*  ROUND 1. Set j = 2 and calculate q2 */
    /*  Estimate qhat = (u4u3)/v1  = 0 or 1
        then set (u4u3u2) -= qhat(v1v0)
        where u4 = 0.
    */
/* [Replaced in Version 2] -->
    qhat = u3 / v1;
    if (qhat > 0)
    {
        rhat = u3 - qhat * v1;
        t = TOHIGH(rhat) | u2;
        if (qhat * v0 > t)
            qhat--;
    }
<-- */
    qhat = (u3 < v1 ? 0 : 1);
    if (qhat > 0)
    {   /* qhat is one, so no need to mult */
        rhat = u3 - v1;
        /* t = r.b + u2 */
        t = TOHIGH(rhat) | u2;
        if (v0 > t)
            qhat--;
    }

    uu[1] = 0;      /* (u4) */
    uu[0] = u[1];   /* (u3u2) */
    if (qhat > 0)
    {
        /* (u4u3u2) -= qhat(v1v0) where u4 = 0 */
        _sp_mult_sub(uu, qhat, v1, v0);
        if (HIHALF(uu[1]) != 0)
        {   /* Add back */
            qhat--;
            uu[0] += v;
            uu[1] = 0;
        }
    }
    q2 = qhat;

    /*  ROUND 2. Set j = 1 and calculate q1 */
    /*  Estimate qhat = (u3u2) / v1
        then set (u3u2u1) -= qhat(v1v0)
    */
    t = uu[0];
    qhat = t / v1;
    rhat = t - qhat * v1;
    /* Test on v0 */
    t = TOHIGH(rhat) | u1;
    if ((qhat == (MAX_HALF_DIGIT + 1)) || (qhat * v0 > t))
    {
        qhat--;
        rhat += v1;
        t = TOHIGH(rhat) | u1;
        if ((rhat < (MAX_HALF_DIGIT + 1)) && (qhat * v0 > t))
            qhat--;
    }

    /*  multiply and subtract
        (u3u2u1)' = (u3u2u1) - qhat(v1v0)
    */
    uu[1] = HIHALF(uu[0]);  /* (0u3) */
    uu[0] = TOHIGH(LOHALF(uu[0])) | u1; /* (u2u1) */
    _sp_mult_sub(uu, qhat, v1, v0);
    if (HIHALF(uu[1]) != 0)
    {   /* Add back */
        qhat--;
        uu[0] += v;
        uu[1] = 0;
    }

    /* q1 = qhat */
    *q = TOHIGH(qhat);

    /* ROUND 3. Set j = 0 and calculate q0 */
    /*  Estimate qhat = (u2u1) / v1
        then set (u2u1u0) -= qhat(v1v0)
    */
    t = uu[0];
    qhat = t / v1;
    rhat = t - qhat * v1;
    /* Test on v0 */
    t = TOHIGH(rhat) | u0;
    if ((qhat == (MAX_HALF_DIGIT + 1)) || (qhat * v0 > t))
    {
        qhat--;
        rhat += v1;
        t = TOHIGH(rhat) | u0;
        if ((rhat < (MAX_HALF_DIGIT + 1)) && (qhat * v0 > t))
            qhat--;
    }

    /*  multiply and subtract
        (u2u1u0)" = (u2u1u0)' - qhat(v1v0)
    */
    uu[1] = HIHALF(uu[0]);  /* (0u2) */
    uu[0] = TOHIGH(LOHALF(uu[0])) | u0; /* (u1u0) */
    _sp_mult_sub(uu, qhat, v1, v0);
    if (HIHALF(uu[1]) != 0)
    {   /* Add back */
        qhat--;
        uu[0] += v;
        uu[1] = 0;
    }

    /* q0 = qhat */
    *q |= LOHALF(qhat);

    /* Remainder is in (u1u0) i.e. uu[0] */
    *r = uu[0];
    return q2;
}

int _mp_sizeof(const digit_t a[], int ndigits)
{   /* Returns size of significant digits in a */

    while(ndigits--)
    {
        if (a[ndigits] != 0)
            return (++ndigits);
    }
    return 0;
}

void _mp_set_equal(digit_t a[], const digit_t b[], int ndigits)
{
    /* Sets a = b */
    int i;

    for (i = 0; i < ndigits; i++)
    {
        a[i] = b[i];
    }
}

void _mp_set_zero(digit_t a[], int ndigits)
{
    /* Sets a = 0 */

    /* Prevent optimiser ignoring this */
    volatile digit_t __attribute__((unused)) optdummy;
    digit_t *p = a;

    while (ndigits--)
        a[ndigits] = 0;

    optdummy = *p;
}

void _mp_set_digit(digit_t a[], digit_t d, int ndigits)
{
    /* Sets a = d where d is a single digit */
    int i;

    for (i = 1; i < ndigits; i++)
    {
        a[i] = 0;
    }
    a[0] = d;
}

digit_t _mp_shift_left(digit_t a[], const digit_t *b, int shift, int ndigits)
{
    /* Computes a = b << shift */
    /* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
    int i, y, nw, bits;
    digit_t mask, carry, nextcarry;

    /* Do we shift whole digits? */
    if (shift >= BITS_PER_DIGIT)
    {
        nw = shift / BITS_PER_DIGIT;
        i = ndigits;
        while (i--)
        {
            if (i >= nw)
                a[i] = b[i-nw];
            else
                a[i] = 0;
        }
        /* Call again to shift bits inside digits */
        bits = shift % BITS_PER_DIGIT;
        carry = b[ndigits-nw] << bits;
        if (bits)
            carry |= _mp_shift_left(a, a, bits, ndigits);
        return carry;
    }
    else
    {
        bits = shift;
    }

    /* Construct mask = high bits set */
    mask = ~(~(digit_t)0 >> bits);

    y = BITS_PER_DIGIT - bits;
    carry = 0;
    for (i = 0; i < ndigits; i++)
    {
        nextcarry = (b[i] & mask) >> y;
        a[i] = b[i] << bits | carry;
        carry = nextcarry;
    }

    return carry;
}

digit_t _mp_shift_right(digit_t a[], const digit_t b[], int shift, int ndigits)
{
    /* Computes a = b >> shift */
    /* [v2.1] Modified to cope with shift > BITS_PERDIGIT */
    int i, y, nw, bits;
    digit_t mask, carry, nextcarry;

    /* Do we shift whole digits? */
    if (shift >= BITS_PER_DIGIT)
    {
        nw = shift / BITS_PER_DIGIT;
        for (i = 0; i < ndigits; i++)
        {
            if ((i+nw) < ndigits)
                a[i] = b[i+nw];
            else
                a[i] = 0;
        }
        /* Call again to shift bits inside digits */
        bits = shift % BITS_PER_DIGIT;
        carry = b[nw-1] >> bits;
        if (bits)
            carry |= _mp_shift_right(a, a, bits, ndigits);
        return carry;
    }
    else
    {
        bits = shift;
    }

    /* Construct mask to set low bits */
    /* (thanks to Jesse Chisholm for suggesting this improved technique) */
    mask = ~(~(digit_t)0 << bits);

    y = BITS_PER_DIGIT - bits;
    carry = 0;
    i = ndigits;
    while (i--)
    {
        nextcarry = (b[i] & mask) << y;
        a[i] = b[i] >> bits | carry;
        carry = nextcarry;
    }

    return carry;
}

int _mp_bit_length(const digit_t d[], int ndigits)
/* Returns no of significant bits in d */
{
    int n, i, bits;
    digit_t mask;

    if (!d || ndigits == 0)
        return 0;

    n = _mp_sizeof(d, ndigits);
    if (0 == n)
        return 0;

    for (i = 0, mask = HIBITMASK; mask > 0; mask >>= 1, i++)
    {
        if (d[n-1] & mask)
            break;
    }

    bits = n * BITS_PER_DIGIT - i;

    return bits;
}

digit_t _mp_short_div(digit_t q[], const digit_t u[], digit_t v, int ndigits)
{
    /*  Calculates quotient q = u div v
        Returns remainder r = u mod v
        where q, u are multiprecision integers of ndigits each
        and r, v are single precision digits.

        Makes no assumptions about normalisation.

        Ref: Knuth Vol 2 Ch 4.3.1 Exercise 16 p625
    */
    int j;
    digit_t t[2], r;
    int shift;
    digit_t bitmask, overflow, *uu;

    if (ndigits == 0)
        return 0;
    if (v == 0)
        return 0;   /* divide by zero error */

    /*  Normalise first */
    /*  Requires high bit of V
        to be set, so find most signif. bit then shift left,
        i.e. d = 2^shift, u' = u * d, v' = v * d.
    */
    bitmask = HIBITMASK;
    for (shift = 0; shift < BITS_PER_DIGIT; shift++)
    {
        if (v & bitmask)
            break;
        bitmask >>= 1;
    }

    if(shift < 32)
        v <<= shift;
    else
        v = 0;

    overflow = mp_func->shift_left(q, u, shift, ndigits);
    uu = q;

    /* Step S1 - modified for extra digit. */
    r = overflow;   /* New digit Un */
    j = ndigits;
    while (j--)
    {
        /* Step S2. */
        t[1] = r;
        t[0] = uu[j];
        overflow = mp_func->sp_divide(&q[j], &r, t, v);
    }

    /* Unnormalise */
    if(shift < 32)
        r >>= shift;
    else
        r = 0;

    return r;
}

int _mp_equal(const digit_t a[], const digit_t b[], int ndigits)
{
    /*  Returns true if a == b, else false
    */

    if (ndigits == 0)
        return -1;

    while (ndigits--)
        {
        if (a[ndigits] != b[ndigits])
            return 0;   /* False */
        }

    return 1;    /* True */
}

int _mp_compare(const digit_t a[], const digit_t b[], int ndigits)
{
    /*  Returns sign of (a - b)
    */

    if (ndigits == 0)
        return 0;

    while (ndigits--)
    {
        if (a[ndigits] > b[ndigits])
            return 1;   /* GT */
        if (a[ndigits] < b[ndigits])
            return -1;  /* LT */
    }

    return 0;   /* EQ */
}

digit_t _mp_add(digit_t w[], const digit_t u[], const digit_t v[], int ndigits)
{
    /*  Calculates w = u + v
        where w, u, v are multiprecision integers of ndigits each
        Returns carry if overflow. Carry = 0 or 1.

        Ref: Knuth Vol 2 Ch 4.3.1 p 266 Algorithm A.
    */

    digit_t k;
    int j;

    //assert(w != v);

    /* Step A1. Initialise */
    k = 0;

    for (j = 0; j < ndigits; j++)
    {
        /*  Step A2. Add digits w_j = (u_j + v_j + k)
            Set k = 1 if carry (overflow) occurs
        */
        w[j] = u[j] + k;
        if (w[j] < k)
            k = 1;
        else
            k = 0;

        w[j] += v[j];
        if (w[j] < v[j])
            k++;

    }   /* Step A3. Loop on j */

    return k;   /* w_n = k */
}

int _mp_multiply(digit_t w[], const digit_t u[], const digit_t v[], int ndigits)
{
    /*  Computes product w = u * v
        where u, v are multiprecision integers of ndigits each
        and w is a multiprecision integer of 2*ndigits

        Ref: Knuth Vol 2 Ch 4.3.1 p 268 Algorithm M.
    */

    digit_t k, t[2];
    int i, j, m, n;

    //assert(w != u && w != v);

    m = n = ndigits;

    /* Step M1. Initialise */
    for (i = 0; i < 2 * m; i++)
        w[i] = 0;

    for (j = 0; j < n; j++)
    {
        /* Step M2. Zero multiplier? */
        if (v[j] == 0)
        {
            w[j + m] = 0;
        }
        else
        {
            /* Step M3. Initialise i */
            k = 0;
            for (i = 0; i < m; i++)
            {
                /* Step M4. multiply and add */
                /* t = u_i * v_j + w_(i+j) + k */
                mp_func->sp_multiply(t, u[i], v[j]);
                t[0] += k;
                if (t[0] < k)
                    t[1]++;
                t[0] += w[i+j];
                if (t[0] < w[i+j])
                    t[1]++;

                w[i+j] = t[0];
                k = t[1];
            }
            /* Step M5. Loop on i, set w_(j+m) = k */
            w[j+m] = k;
        }
    }   /* Step M6. Loop on j */

    return 0;
}

static digit_t _mp_mult_sub(digit_t wn, digit_t w[], const digit_t v[], digit_t q, int n)
{   /*  Compute w = w - qv
        where w = (WnW[n-1]...W[0])
        return modified Wn.
    */
    digit_t k, t[2];
    int i;

    if (q == 0) /* No change */
        return wn;

    k = 0;

    for (i = 0; i < n; i++)
    {
        mp_func->sp_multiply(t, q, v[i]);
        w[i] -= k;
        if (w[i] > MAX_DIGIT - k)
            k = 1;
        else
            k = 0;
        w[i] -= t[0];
        if (w[i] > MAX_DIGIT - t[0])
            k++;
        k += t[1];
    }

    /* Cope with Wn not stored in array w[0..n-1] */
    wn -= k;

    return wn;
}

static int _qhat_too_big(digit_t qhat, digit_t rhat, digit_t vn2, digit_t ujn2)
{
    //  Returns true if Qhat is too big
    //  i.e. if (Qhat * Vn-2) > (b.Rhat + Uj+n-2)
    //
    digit_t t[2];

    mp_func->sp_multiply(t, qhat, vn2);
    if (t[1] < rhat)
        return 0;
    else if (t[1] > rhat)
        return 1;
    else if (t[0] > ujn2)
        return 1;

    return 0;
}

int _mp_divide(digit_t q[], digit_t r[], const digit_t u[], int udigits, digit_t v[], int vdigits)
{
    /*  Computes quotient q = u / v and remainder r = u mod v
    where q, r, u are multiple precision digits
    all of udigits and the divisor v is vdigits.

    Ref: Knuth Vol 2 Ch 4.3.1 p 272 Algorithm D.

    Do without extra storage space, i.e. use r[] for
    normalised u[], unnormalise v[] at end, and cope with
    extra digit Uj+n added to u after normalisation.

    WARNING: this trashes q and r first, so cannot do
    u = u / v or v = u mod v.
    It also changes v temporarily so cannot make it const.
    */
    int shift;
    int n, m, j;
    digit_t bitmask, overflow;
    digit_t qhat, rhat, t[2];
    digit_t *uu, *ww;
    int qhatOK, cmp;

    /* Clear q and r */
    _mp_set_zero(q, udigits);
    _mp_set_zero(r, udigits);

    /* Work out exact sizes of u and v */
    n = (int)_mp_sizeof(v, vdigits);
    m = (int)_mp_sizeof(u, udigits);
    m -= n;

    /* Catch special cases */
    if (n == 0)
        return -1;  /* Error: divide by zero */

    if (n == 1)
    {   /* Use short division instead */
        r[0] = mp_func->short_div(q, u, v[0], udigits);
        return 0;
    }

    if (m < 0)
    {   /* v > u, so just set q = 0 and r = u */
        _mp_set_equal(r, u, udigits);
        return 0;
    }

    if (m == 0)
    {   /* u and v are the same length */
        cmp = _mp_compare(u, v, (int)n);
        if (cmp < 0)
        {   /* v > u, as above */
            _mp_set_equal(r, u, udigits);
            return 0;
        }
        else if (cmp == 0)
        {   /* v == u, so set q = 1 and r = 0 */
            _mp_set_digit(q, 1, udigits);
            return 0;
        }
    }

    /*  In Knuth notation, we have:
        Given
        u = (Um+n-1 ... U1U0)
        v = (Vn-1 ... V1V0)
        Compute
        q = u/v = (QmQm-1 ... Q0)
        r = u mod v = (Rn-1 ... R1R0)
    */

    /*  Step D1. Normalise */
    /*  Requires high bit of Vn-1
        to be set, so find most signif. bit then shift left,
        i.e. d = 2^shift, u' = u * d, v' = v * d.
    */
    bitmask = HIBITMASK;
    for (shift = 0; shift < BITS_PER_DIGIT; shift++)
    {
        if (v[n-1] & bitmask)
            break;
        bitmask >>= 1;
    }

    /* Normalise v in situ - NB only shift non-zero digits */
    overflow = mp_func->shift_left(v, v, shift, n);

    /* Copy normalised dividend u*d into r */
    overflow = mp_func->shift_left(r, u, shift, n + m);
    uu = r; /* Use ptr to keep notation constant */

    t[0] = overflow;    /* Extra digit Um+n */

    /* Step D2. Initialise j. Set j = m */
    for (j = m; j >= 0; j--)
    {
        /* Step D3. Set Qhat = [(b.Uj+n + Uj+n-1)/Vn-1]
           and Rhat = remainder */
        qhatOK = 0;
        t[1] = t[0];    /* This is Uj+n */
        t[0] = uu[j+n-1];
        overflow = mp_func->sp_divide(&qhat, &rhat, t, v[n-1]);

        /* Test Qhat */
        if (overflow)
        {   /* Qhat == b so set Qhat = b - 1 */
            qhat = MAX_DIGIT;
            rhat = uu[j+n-1];
            rhat += v[n-1];
            if (rhat < v[n-1])  /* Rhat >= b, so no re-test */
                qhatOK = 1;
        }
        /* [VERSION 2: Added extra test "qhat && "] */
        if (qhat && !qhatOK && _qhat_too_big(qhat, rhat, v[n-2], uu[j+n-2]))
        {   /* If Qhat.Vn-2 > b.Rhat + Uj+n-2
               decrease Qhat by one, increase Rhat by Vn-1
            */
            qhat--;
            rhat += v[n-1];
            /* Repeat this test if Rhat < b */
            if (!(rhat < v[n-1]))
            {
                if (_qhat_too_big(qhat, rhat, v[n-2], uu[j+n-2]))
                    qhat--;
            }
        }


        /* Step D4. multiply and subtract */
        ww = &uu[j];
        overflow = _mp_mult_sub(t[1], ww, v, qhat, (int)n);

        /* Step D5. Test remainder. Set Qj = Qhat */
        q[j] = qhat;
        if (overflow)
        {   /* Step D6. Add back if D4 was negative */
            q[j]--;
            overflow = _mp_add(ww, ww, v, (int)n);
            if (overflow)
                UBOOT_ERROR("_mp_add overflow \n");
        }

        t[0] = uu[j+n-1];   /* Uj+n on next round */

    }   /* Step D7. Loop on j */

    /* Clear high digits in uu */
    for (j = n; j < m+n; j++)
        uu[j] = 0;

    /* Step D8. Unnormalise. */

    mp_func->shift_right(r, r, shift, n);
    mp_func->shift_right(v, v, shift, n);

    return 0;
}

int _mp_square(digit_t w[], const digit_t x[], int ndigits)
{
    /*  Computes square w = x * x
        where x is a multiprecision integer of ndigits
        and w is a multiprecision integer of 2*ndigits

        Ref: Menezes p596 Algorithm 14.16 with errata.
    */

    digit_t k, p[2], u[2], cbit, carry, o1, o2;
    int i, j, t, i2, cpos;

    //assert(w != x);
    t = ndigits;

    /* 1. For i from 0 to (2t-1) do: w_i = 0 */
    i2 = t << 1;
    for (i = 0; i < i2; i++)
        w[i] = 0;
    carry = 0;
    cpos = i2-1;
    /* 2. For i from 0 to (t-1) do: */
    for (i = 0; i < t; i++)
    {
        /* 2.1 (uv) = w_2i + x_i * x_i, w_2i = v, c = u
           Careful, w_2i may be double-prec
        */
        i2 = i << 1; /* 2*i */

    	o1 = x[i] ;
    	o2 = x[i] ;

	   mp_func->sp_multiply(p, o1, o2);
        p[0] += w[i2];
        if (p[0] < w[i2])
            p[1]++;
        k = 0;  /* p[1] < b, so no overflow here */
        if (i2 == cpos && carry)
        {
            p[1] += carry;
            if (p[1] < carry)
                k++;
            carry = 0;
        }
        w[i2] = p[0];
        u[0] = p[1];
        u[1] = k;

        /* 2.2 for j from (i+1) to (t-1) do:
           (uv) = w_{i+j} + 2x_j * x_i + c,
           w_{i+j} = v, c = u,
           u is double-prec
           w_{i+j} is dbl if [i+j] == cpos
        */

        k = 0;
        for (j = i+1; j < t; j++)
        {
            /* p = x_j * x_i */
            mp_func->sp_multiply(p, x[j], x[i]);
            //_sp_multiply(p, x[j], x[i]);
            /* p = 2p <=> p <<= 1 */
            cbit = (p[0] & HIBITMASK) != 0;
            k =  (p[1] & HIBITMASK) != 0;
            p[0] <<= 1;
            p[1] <<= 1;
            p[1] |= cbit;
            /* p = p + c */
            p[0] += u[0];
            if (p[0] < u[0])
            {
                p[1]++;
                if (p[1] == 0)
                    k++;
            }
            p[1] += u[1];
            if (p[1] < u[1])
                k++;
            /* p = p + w_{i+j} */
            p[0] += w[i+j];
            if (p[0] < w[i+j])
            {
                p[1]++;
                if (p[1] == 0)
                    k++;
            }
            if ((i+j) == cpos && carry)
            {   /* catch overflow from last round */
                p[1] += carry;
                if (p[1] < carry)
                    k++;
                carry = 0;
            }
            /* w_{i+j} = v, c = u */
            w[i+j] = p[0];
            u[0] = p[1];
            u[1] = k;
        }
        /* 2.3 w_{i+t} = u */
        w[i+t] = u[0];
        /* remember overflow in w_{i+t} */
        carry = u[1];
        cpos = i+t;
    }

    /* (NB original step 3 deleted in errata) */

    /* Return w */

    return 0;
}

/* Version 2: added new funcs with temps
to avoid having to alloc and free repeatedly
and added _mp_square function for slight speed improvement
*/

static int _modulo_temp(digit_t r[], const digit_t u[], int udigits,
             digit_t v[], int vdigits, digit_t tqq[], digit_t trr[])
{
    /*  Calculates r = u mod v
        where r, v are multiprecision integers of length vdigits
        and u is a multiprecision integer of length udigits.
        r may overlap v.

        Same as mpModulo without allocs & free.
        Requires temp mp's tqq and trr of length udigits each
    */

    mp_func->divide(tqq, trr, u, udigits, v, vdigits);

    /* Final r is only vdigits long */
    _mp_set_equal(r, trr, vdigits);

    return 0;
}

static int _mod_mult_temp(digit_t a[], const digit_t x[], const digit_t y[],
              digit_t m[], int ndigits,
              digit_t temp[], digit_t tqq[], digit_t trr[])
{   /*  Computes a = (x * y) mod m */
    /*  Requires 3 x temp mp's of length 2 * ndigits each */

    /* Calc p[2n] = x * y */
    mp_func->multiply(temp, x, y, ndigits);

    /* Then modulo m */
    _modulo_temp(a, temp, ndigits * 2, m, ndigits, tqq, trr);

    return 0;
}

static int _mod_square_temp(digit_t a[], const digit_t x[],
              digit_t m[], int ndigits,
              digit_t temp[], digit_t tqq[], digit_t trr[])
{   /*  Computes a = (x * x) mod m */
    /*  Requires 3 x temp mp's of length 2 * ndigits each */

    /* Calc p[2n] = x^2 */
    mp_func->square(temp, x, ndigits);

    /* Then modulo m */
    _modulo_temp(a, temp, ndigits * 2, m, ndigits, tqq, trr);

    return 0;
}

static unsigned int t1[RSA_KEY_DIGI_LEN*2];
static unsigned int t2[RSA_KEY_DIGI_LEN*2];
static unsigned int t3[RSA_KEY_DIGI_LEN*2];
static unsigned int tm[RSA_KEY_DIGI_LEN];
static unsigned int y[RSA_KEY_DIGI_LEN];

int _mp_mod_exp(digit_t yout[], const digit_t x[], const digit_t e[], const digit_t m[], int ndigits)
{
    /*  Computes y = x^e mod m */
    /*  Binary left-to-right method */
    digit_t mask;
    unsigned int n, nn;
    //digit_t *t1, *t2, *t3, *tm, *y;

    if (ndigits == 0)
        return -1;

    /* Create some temps */
    nn = ndigits * 2;

    _mp_set_zero(t1, nn);
    _mp_set_zero(t2, nn);
    _mp_set_zero(t3, nn);
    _mp_set_zero(tm, ndigits);
    _mp_set_zero(y, ndigits);

    _mp_set_equal(tm, m, ndigits);

    /* Find second-most significant bit in e */
    n = _mp_sizeof(e, ndigits);
    for (mask = HIBITMASK; mask > 0; mask >>= 1)
    {
        if (e[n-1] & mask)
            break;
    }
    mp_next_bit_mask(mask, n);

    /* Set y = x */
    _mp_set_equal(y, x, ndigits);

    /* For bit j = k-2 downto 0 */

    while(n)
    {
        /* square y = y * y mod n */
        _mod_square_temp(y, y, tm, ndigits, t1, t2, t3);
        if (e[n-1] & mask)
        {
            /*  if e(j) == 1 then multiply y = y * x mod n */
            _mod_mult_temp(y, y, x, tm, ndigits, t1, t2, t3);
        }

        /* Move to next bit */
        mp_next_bit_mask(mask, n);
    }

    /* return y */
    _mp_set_equal(yout, y, ndigits);

    return 0;
}

int _mp_conv_from_octets(digit_t a[], int ndigits, const unsigned char *c, int nbytes)
/* Converts nbytes octets into big digit a of max size ndigits
   Returns actual number of digits set (may be larger than mpSizeof)
*/
{
    //mp_func=&_mp_func;  //-------------------------------
    int i;
    int j, k;
    digit_t t;

    _mp_set_zero(a, ndigits);

    /* Read in octets, least significant first */
    /* i counts into big_d, j along c, and k is # bits to shift */
    for (i = 0, j = nbytes - 1; i < ndigits && j >= 0; i++)
    {
        t = 0;
        for (k = 0; j >= 0 && k < BITS_PER_DIGIT; j--, k += 8)
        {
            t |= ((digit_t)c[j]) << k;
        }
        a[i] = t;
    }

    return i;
}

int _mp_conv_to_octets(const digit_t a[], int ndigits, unsigned char *c, int nbytes)
/* Convert big digit a into string of octets, in big-endian order,
   padding on the left to nbytes or truncating if necessary.
   Return number of octets required excluding leading zero bytes.
*/
{
    int j, k, len;
    digit_t t;
    int i, noctets, nbits;

    nbits = _mp_bit_length(a, ndigits);
    noctets = (nbits + 7) / 8;

    len = (int)nbytes;

    for (i = 0, j = len - 1; i < ndigits && j >= 0; i++)
    {
        t = a[i];
        for (k = 0; j >= 0 && k < BITS_PER_DIGIT; j--, k += 8)
        {
            c[j] = (unsigned char)(t >> k);
        }
    }

    for ( ; j >= 0; j--)
    {
        c[j] = 0;
    }

    return (int)noctets;
}
int secure_rsa2048decrypt_sw(unsigned char *signature, unsigned char *public_key_n, unsigned char *public_key_e, unsigned char *sim_sign_out)
{
    UBOOT_TRACE("IN\n");
    digit_t sim_signature[RSA_KEY_DIGI_LEN];
    digit_t sim_n[RSA_KEY_DIGI_LEN];
    digit_t sim_e[RSA_KEY_DIGI_LEN];

    _mp_set_zero(sim_signature, RSA_KEY_DIGI_LEN);
    _mp_set_zero(sim_n, RSA_KEY_DIGI_LEN);
    _mp_set_zero(sim_e, RSA_KEY_DIGI_LEN);
    _mp_set_zero((digit_t *)sim_sign_out, RSA_KEY_DIGI_LEN);

    mp_func->conv_from_octets(sim_signature, RSA_KEY_DIGI_LEN, signature, RSA_KEY_DIGI_LEN); //sizeof(signature)
    mp_func->conv_from_octets(sim_n, RSA_KEY_DIGI_LEN, public_key_n, RSA_KEY_DIGI_LEN); //sizeof(public_key_n)
    mp_func->conv_from_octets(sim_e, RSA_KEY_DIGI_LEN, public_key_e, 4); //sizeof(PublicKey_E)
    mp_func->mod_exp((digit_t *)sim_sign_out, sim_signature, sim_e, sim_n, RSA_KEY_DIGI_LEN);

    // revert the order of Sim_SignOut
    int i=0;
    unsigned char temp;
    for(i=0;i<(RSA_KEY_DIGI_LEN/2);i++)
    {
       temp= ((unsigned char *)sim_sign_out)[i];
       ((unsigned char *)sim_sign_out)[i]=((unsigned char *)sim_sign_out)[RSA_KEY_DIGI_LEN-1-i];
       ((unsigned char *)sim_sign_out)[RSA_KEY_DIGI_LEN-1-i]=temp;

    }

    UBOOT_TRACE("OK\n");
    return 0;
}
#endif


int rsa_main(unsigned char *signature, unsigned char *public_key_n, unsigned char *public_key_e, unsigned char *sim_sign_out)
{
    UBOOT_TRACE("IN\n");
    int ret = -1;

    #if defined(CONFIG_SECURE_HW_IP)
    // use HW driver
    UBOOT_DEBUG("\n\nUSE secure_rsa2048decrypt_hw\n\n");
    ret = secure_rsa2048decrypt_hw(signature,public_key_n,public_key_e,sim_sign_out);
    #else
    UBOOT_DEBUG("\n\nUSE secure_rsa2048decrypt_sw\n\n");
    ret = secure_rsa2048decrypt_sw(signature,public_key_n,public_key_e,sim_sign_out);
    #endif

    if(-1 == ret)
    {
        UBOOT_ERROR("RSA Decrypt Error\n");
    }
    else
    {
        UBOOT_TRACE("OK\n");
    }
	return ret;
}

#undef CRYPTO_RSA_C
