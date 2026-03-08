// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <secure/crypto_rsa.h>
#include <secure/crypto_sha.h>
#include <secure/crypto_func.h>


// mp_func Table
void _null(void) { while (1) ;}


mp_func_table _mp_func =
{
    _mp_conv_from_octets,
    _mp_mod_exp,
    _mp_equal,

    _mp_square,
    _mp_multiply,
    _mp_divide,
    _mp_short_div,
    _mp_shift_left,
    _mp_shift_right,
    _sp_multiply,
    _sp_divide,

   { _null, _null, _null, _null, _null, _null, _null, _null,
    _null, _null, _null, _null, _null},
};

mp_func_table *mp_func = &_mp_func;

