// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "u-boot/drv_aes.h"
#include "hal_aes.h"

void HAL_AESDMA_ResetSource(void)
{
    HAL_AESDMA_DisableXIUSelectCA9();
}

void HAL_AESDMA_SetIV(unsigned int *iv)
{
    int i;
    if (iv != NULL)
    {
        // SW IV
        for (i=0; i<(AES_IV_BYTESIZE/AES_UNIT_SIZE); i+=AES_UNIT_SIZE)
        {
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_IV+i)<<1))] = iv[i>>1] & MASK_16;
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_IV+i+1)<<1))] = (iv[i>>1]>>SHIFT_BITS_16) & MASK_16;
        }
    }
    else
    {
        // Zero IV
        for (i=0; i<(AES_IV_BYTESIZE/AES_UNIT_SIZE); i+=AES_UNIT_SIZE)
        {
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_IV+i)<<1))] = 0x0000;
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_IV+i+1)<<1))] = 0x0000;
        }
    }
}

void HAL_AESDMA_DisableXIUSelectCA9(void)
{
    //disable AESDMA XIU select CA9
    RIU[(AESDMA_BASE_ADDR+(REG_Eng2_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Eng2_Ctrl<<1))]) & (~AESDMA_CTRL_XIU_SEL_CA9));
}

void HAL_AESDMA_Reset(void)
{
    RIU[(AESDMA_BASE_ADDR+(REG_Eng2_Ctrl<<1))] = 0;
    RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = 0;
}

void HAL_AESDMA_SetEngine(unsigned int engine)
{
    RIU[(AESDMA_BASE_ADDR+(REG_Eng_Ctrl<<1))] = engine;
}

void HAL_AESDMA_SetDecrypt(unsigned int decrypt)
{
    if (decrypt)
    {
        RIU[(AESDMA_BASE_ADDR+(REG_Eng_Ctrl<<1))] |= AESDMA_CTRL_CIPHER_DECRYPT;
    }
}

void HAL_AESDMA_SetKeyIndex(E_AESDMA_KEYTYPE keytype, unsigned int *key)
{
    int i;
    // Reset all key select
    RIU[(AESDMA_BASE_ADDR+(REG_Dma2_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma2_Ctrl<<1))]) & (~(AESDMA_CTRL_USE_SECRET_KEY|AESDMA_CTRL_RESERVE)));
    RIU[DSCRMB67_BASE_ADDR] = RIU[DSCRMB67_BASE_ADDR] & (~REG_DMAKEY_INDEX_MASK);
    RIU[(AESDMA_BASE_ADDR+(REG_Dma4_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma4_Ctrl<<1))]) & (~(AESDMA_CTRL_192KEY_EN|AESDMA_CTRL_256KEY_EN)));

    if (key == NULL)
    {
        for (i=0; i<(AES_128KEY_BYTESIZE/AES_UNIT_SIZE); i+=AES_UNIT_SIZE)
        {
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i)<<1))] = 0x0;
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i+1)<<1))] = 0x0;
        }
    }
    else if (keytype == E_AESDMA_SW256KEY)
    {
        int j;
        for (i=0; i<(AES_128KEY_BYTESIZE/AES_UNIT_SIZE); i+=AES_UNIT_SIZE)
        {
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i)<<1))] = key[i>>1] & MASK_16;
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i+1)<<1))] = (key[i>>1]>>SHIFT_BITS_16) & MASK_16;
        }
        for (j=0; j<((AES_256KEY_BYTESIZE-AES_128KEY_BYTESIZE)/AES_UNIT_SIZE); i+=AES_UNIT_SIZE, j+=AES_UNIT_SIZE)
        {
            RIU[(DSCRMB4_BASE_ADDR+((REG_Cipher_Key_1+j)<<1))] = key[i>>1] & MASK_16;
            RIU[(DSCRMB4_BASE_ADDR+((REG_Cipher_Key_1+j+1)<<1))] = (key[i>>1]>>SHIFT_BITS_16) & MASK_16;
        }
        RIU[(AESDMA_BASE_ADDR+(REG_Dma4_Ctrl<<1))] = RIU[(AESDMA_BASE_ADDR+(REG_Dma4_Ctrl<<1))] | AESDMA_CTRL_256KEY_EN;
    }
    else
    {
        for (i=0; i<(AES_128KEY_BYTESIZE/AES_UNIT_SIZE); i+=AES_UNIT_SIZE)
        {
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i)<<1))] = key[i>>1] & MASK_16;
            RIU[(AESDMA_BASE_ADDR+((REG_Cipher_Key_0+i+1)<<1))] = (key[i>>1]>>SHIFT_BITS_16) & MASK_16;
        }
    }

}

void HAL_AESDMA_SetFileInAddr(unsigned long long addr)
{
    unsigned short Val = RIU[(DSCRMB67_BASE_ADDR+(REG_Dma_Addr_Ext_DSCMB3<<1))];

    RIU[(AESDMA_BASE_ADDR+(REG_Filein_Addr_0<<1))] = addr & MASK_16;
    RIU[(AESDMA_BASE_ADDR+(REG_Filein_Addr_1<<1))] = (addr>>SHIFT_BITS_16) & MASK_16;
    Val &= (~MASK_AESDMA_FILEIN_ADDR_EXT);
    RIU[(DSCRMB67_BASE_ADDR+(REG_Dma_Addr_Ext_DSCMB3<<1))] = (((addr>>AESDMA_FILEIN_ADDR_EXT_SHIFTBIT) & MASK_AESDMA_FILEIN_ADDR_EXT) | Val);
}

void HAL_AESDMA_SetFileOutAddr(unsigned long long addr)
{
    unsigned short Val = RIU[(DSCRMB67_BASE_ADDR+(REG_Dma_Addr_Ext_DSCMB3<<1))];

    RIU[(AESDMA_BASE_ADDR+(REG_Fileout_Addr_0<<1))] = addr & MASK_16;
    RIU[(AESDMA_BASE_ADDR+(REG_Fileout_Addr_1<<1))] = (addr>>SHIFT_BITS_16) & MASK_16;
    Val &= (~MASK_AESDMA_FILEOUT_SADDR_EXT);
    RIU[(DSCRMB67_BASE_ADDR+(REG_Dma_Addr_Ext_DSCMB3<<1))] = (((addr>>AESDMA_FILEOUT_SADDR_EXT_SHIFTBIT) & MASK_AESDMA_FILEOUT_SADDR_EXT) | Val);
}

void HAL_AESDMA_SetFileSize(unsigned int len)
{
    RIU[(AESDMA_BASE_ADDR+(REG_Filein_Num_0<<1))] = len & MASK_16;
    RIU[(AESDMA_BASE_ADDR+(REG_Filein_Num_1<<1))] = (len>>SHIFT_BITS_16) & MASK_16;
}

void HAL_AESDMA_FileOutEnable(unsigned char FileOutEnable)
{
    // AESDMA fout_en
    if(FileOutEnable==1)
    {
        RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))]) | (AESDMA_CTRL_FOUT_EN));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))]) & (~AESDMA_CTRL_FOUT_EN));
    }
}

void HAL_AESDMA_Start(unsigned char AESDMAStart)
{
    // AESDMA file start
    if(AESDMAStart==1)
    {
        RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))]) | (AESDMA_CTRL_FILE_ST));
        RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))]) & (~AESDMA_CTRL_FILE_ST));
    }
    else
    {
        RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))] = ((RIU[(AESDMA_BASE_ADDR+(REG_Dma_Ctrl<<1))]) & (~AESDMA_CTRL_FILE_ST));
    }
}

unsigned int HAL_AESDMA_IsFinished(void)
{
    return RIU[(AESDMA_BASE_ADDR+(REG_Dma_Done<<1))] & AESDMA_IS_FINISHED;
}

