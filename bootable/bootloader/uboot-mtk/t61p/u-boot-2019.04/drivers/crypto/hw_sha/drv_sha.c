// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <system_impl.h>
#include "hal_sha.h"
#include "u-boot/drv_sha.h"

int _HW_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf)
{
    unsigned int timeout_count = 0;
    unsigned long long VAOutBuf = 0;

    // Clear all SHA setting(clear the last outputs)
    HAL_SHA_Reset();
    // Error handler for 16-byte alignment limitation
    if(PAInBuf & WORD_MASK)
    {
        printf("Input Address should be 16-byte alignment\n");
        return -1;
    }
    if(HAL_SHA_Calculate(eMode, PAInBuf, Size, PAOutBuf) != 0)
        return -1;

    udelay(SHA_DELAY_40);
    flush_dcache_all();
    udelay(SHA_DELAY_200);

    HAL_SHA_Start(1);
    while ((HAL_SHA_Get_Status() & SHARNG_CTRL_SHA_READY) == 0)
    {
        udelay(SHA_DELAY_10);
        timeout_count++;
#if (SHA_WAIT == 1)
        //500ms timeout
        if(timeout_count > SHA_TIMEOUT_VALUE)
        {
            HAL_SHA_Reset();
            return -1;
        }
#endif
    }

    VAOutBuf = phyical_addr_to_virtual_addr(PAOutBuf);
    if(VAOutBuf == INVALID_ADDR)
    {
        return -1;
    }

    HAL_SHA_Out(eMode, VAOutBuf);
    while ((HAL_SHA_Get_Status() & SHARNG_CTRL_SHA_BUSY) != 0);

    HAL_SHA_Start(0);

    return 0;
}

int HW_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf)
{
    return _HW_SHA_Calculate(eMode, PAInBuf, Size, PAOutBuf);
}

