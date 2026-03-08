// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <debug_impl.h>
#include "u-boot/drv_aes.h"
#include "hal_aes.h"

void array_reverse(unsigned char* ary, unsigned int len)
{
    int i, j;
    char tmp;
    for (i=0, j=len-1; i<j; ++i, --j)
    {
        tmp = ary[i];
        ary[i] = ary[j];
        ary[j] = tmp;
    }
}

int HW_AESDMA_Encrypt_Ext(unsigned long long inaddr, unsigned long long outaddr, unsigned int len, unsigned int engine, E_AESDMA_KEYTYPE keytype, unsigned char *key, unsigned char *iv)
{
    unsigned int timeout_count = 0;

    HAL_AESDMA_Reset();
    HAL_AESDMA_ResetSource();

    HAL_AESDMA_SetEngine(engine);
    HAL_AESDMA_SetDecrypt(0);
    HAL_AESDMA_SetKeyIndex(keytype, (unsigned int*)key);
    HAL_AESDMA_SetIV((unsigned int*) iv);
    HAL_AESDMA_SetFileInAddr(inaddr);
    HAL_AESDMA_SetFileOutAddr(outaddr);
    HAL_AESDMA_SetFileSize(len);
    HAL_AESDMA_FileOutEnable(1);

    flush_dcache_all();
    HAL_AESDMA_Start(1);

    while (!HAL_AESDMA_IsFinished())
    {
        udelay(AES_DELAY_10);
        timeout_count++;
        if(timeout_count > AES_TIMEOUT_VALUE)
        {
            UBOOT_ERROR("%s: timeout!\n", __func__);
            HAL_AESDMA_Reset();
            return -1;
        }
    }

    HAL_AESDMA_Reset();
    return 0;
}

int HW_AESDMA_Dncrypt_Ext(unsigned long long inaddr, unsigned long long outaddr, unsigned int len, unsigned int engine, E_AESDMA_KEYTYPE keytype, unsigned char *key, unsigned char *iv)
{
    unsigned int timeout_count = 0;

    HAL_AESDMA_Reset();
    HAL_AESDMA_ResetSource();

    HAL_AESDMA_SetEngine(engine);
    HAL_AESDMA_SetDecrypt(1);//Encrypt:0, Decrypt:1
    HAL_AESDMA_SetKeyIndex(keytype, (unsigned int*)key);
    HAL_AESDMA_SetIV((unsigned int*) iv);
    HAL_AESDMA_SetFileInAddr(inaddr);
    HAL_AESDMA_SetFileOutAddr(outaddr);
    HAL_AESDMA_SetFileSize(len);
    HAL_AESDMA_FileOutEnable(1);

    flush_dcache_all();
    HAL_AESDMA_Start(1);

    while (!HAL_AESDMA_IsFinished())
    {
        udelay(AES_DELAY_10);
        timeout_count++;
        if(timeout_count > AES_TIMEOUT_VALUE)
        {
            UBOOT_ERROR("%s: timeout!\n", __func__);
            HAL_AESDMA_Reset();
            return -1;
        }
    }

    HAL_AESDMA_Reset();
    return 0;
}
