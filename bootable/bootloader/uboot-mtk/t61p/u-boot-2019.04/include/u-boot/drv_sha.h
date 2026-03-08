/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DRV_SHA_H_
#define _DRV_SHA_H_

#ifdef __cplusplus
extern "C"
{
#endif
//--------------------------------------------------------------------------------------------------
//  Defines
//--------------------------------------------------------------------------------------------------
#define WORD_MASK                    0x0000000F
#define SHA_WAIT                     1
#if (SHA_WAIT==1)
#define SHA_TIMEOUT_VALUE            1200000
#endif
#define SHA_DELAY_10                 10
#define SHA_DELAY_40                 40
#define SHA_DELAY_200                200
#define INVALID_ADDR                 (0xFFFFFFFF)

typedef enum
{
    E_DRV_SHA1 = 0,
    E_DRV_SHA256,
    E_DRV_SHA384,
    E_DRV_SHA512
} Drv_SHAMode;

//--------------------------------------------------------------------------------------------------
//  Function Prototype
//--------------------------------------------------------------------------------------------------
int HW_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf);
int _HW_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _DRV_SHA_H_