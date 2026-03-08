// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "hal_sha.h"

static REG_SHARNGCtrl        *_SHARNGCtrl = (REG_SHARNGCtrl*)REG_SHARNGCTRL_BASE;
static REG_AES_DSCMB7Ctrl    *_AESDSCMB7Ctrl = (REG_AES_DSCMB7Ctrl*)REG_AES_DSCMB7_BASE;

#define SHA_REG32_W(reg, value)    do {                                \
                                   (reg)->H = ((value) >> SHIFT_BITS); \
                                   (reg)->L = ((value) & LOW_16_MASK); \
                                   } while(0)

unsigned int SHA_REG32_R(REG32 *reg)
{
    unsigned int value;

    value = (reg)->H << SHIFT_BITS;
    value |= (reg)->L;

    return value;
}

void HAL_SHA_Reset(void)
{
    unsigned long Reg_SHARNG;
    unsigned int i = 0;
    Reg_SHARNG = (unsigned long)(&_SHARNGCtrl[0].Sha_Ctrl);
    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHARNG_CTRL_SHA_RST); // 0->1
    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), (SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)&(~SHARNG_CTRL_SHA_RST))); // 1->0

    // reset all SHA & RNG register
    for (i=0; i<SHA_REG_RANGE; i++)
    {
        SHA_REG32_W((REG32 *)(Reg_SHARNG+(i*REG_BITS)), 0x00000000);
    }
}

unsigned int HAL_SHA_Get_Status(void)
{
    return SHA_REG32_R(&_SHARNGCtrl[0].Sha_Status);
}

void HAL_SHA_SelMode(Drv_SHAMode eMode)
{
    unsigned int Cmd = 0;
    unsigned int Cmd2 = 0;

    switch (eMode)
    {
    case E_DRV_SHA1:
        Cmd |= (SHARNG_CTRL_SHA_MSG_BLOCK_NUM);
        break;

    case E_DRV_SHA256:
        Cmd |= (SHARNG_CTRL_SHA_SEL_SHA256 | SHARNG_CTRL_SHA_MSG_BLOCK_NUM);
        break;

    case E_DRV_SHA384:
        Cmd |= (SHARNG_CTRL_SHA_MSG_BLOCK_NUM);
        Cmd2 |= SHARNG_CTRL_SHA_SEL_SHA384;
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Status), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Status) | Cmd2);
        break;

    case E_DRV_SHA512:
        Cmd |= (SHARNG_CTRL_SHA_MSG_BLOCK_NUM);
        Cmd2 |= SHARNG_CTRL_SHA_SEL_SHA512;
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Status), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Status) | Cmd2);
        break;

    default:
        Cmd |= (SHARNG_CTRL_SHA_MSG_BLOCK_NUM);
        break;
    }

    //Turn off speedup mode
    //u32Cmd |= SHARNG_CTRL_SPEED_MODE_N;

    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl) | Cmd);
}

void HAL_SHA_SetAddress(unsigned long long Addr)
{
    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Start), Addr);
    SHA_REG32_W((&_AESDSCMB7Ctrl[0].Dma_Addr_Ext_DSCMB5), (SHA_REG32_R(&_AESDSCMB7Ctrl[0].Dma_Addr_Ext_DSCMB5) | ((Addr>>SHIFT_BITS)&MASK_SHA_START_EXT)));
}

void HAL_SHA_SetLength(unsigned int Size)
{
    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Length), Size);
}

void HAL_SHA_Start(unsigned char SHAStart)
{
    if(SHAStart)
    {
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)&~(SHARNG_CTRL_SHA_FIRE_ONCE));
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)|(SHARNG_CTRL_SHA_FIRE_ONCE));
    }
    else
    {
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)&~(SHARNG_CTRL_SHA_FIRE_ONCE));
    }
}

void HAL_SHA_Out(Drv_SHAMode eMode, unsigned long long Buf)
{
    unsigned int Index, Index2, Size, offset, offset2;
    switch (eMode)
    {
        case E_DRV_SHA1:
            Size = DIGEST_SIZE_SHA1;
            offset = DIGEST_SIZE_SHA256 - DIGEST_SIZE_SHA1;
            break;

        case E_DRV_SHA256:
            Size = DIGEST_SIZE_SHA256;
            offset = 0;
            break;

        case E_DRV_SHA384:
            Size = DIGEST_SIZE_SHA384;
            offset = DIGEST_SIZE_SHA384 - DIGEST_SIZE_SHA256;
            offset2 = DIGEST_OFFSET_SHA384_SHA512;
            break;

        case E_DRV_SHA512:
            Size = DIGEST_SIZE_SHA512;
            offset = 0;
            offset2 = DIGEST_OFFSET_SHA384_SHA512;
            break;

        default:
            Size = DIGEST_SIZE_SHA256;
            offset = 0;
            offset2 = 0;
    }

    if (eMode <= E_DRV_SHA256) //sha 1, sha 256
    {
        for(Index=0; Index<Size; Index++)
        {
            *((unsigned int *)Buf + Index) = SHA_REG32_R(&_SHARNGCtrl[0].Sha_Out[Index + offset]);
        }
    }
    else if (eMode == E_DRV_SHA384) //sha 384
    {
        for (Index=0; Index<offset; Index++)
        {
            *((unsigned int *)Buf + Index) = SHA_REG32_R(&_SHARNGCtrl[0].Sha_Out[Index + offset]);
        }
        for (Index2=0; Index<Size; Index2++, Index++)
        {
            *((unsigned int *)Buf + Index) = SHA_REG32_R(&_SHARNGCtrl[0].Sha_Out2[Index2 + offset2]);
        }
    }
    else //sha 512
    {
        for (Index=0; Index<DIGEST_SIZE_SHA256; Index++)
        {
            *((unsigned int *)Buf + Index) = SHA_REG32_R(&_SHARNGCtrl[0].Sha_Out[Index + offset]);
        }
        for (Index2=0; Index<Size; Index2++, Index++)
        {
            *((unsigned int *)Buf + Index) = SHA_REG32_R(&_SHARNGCtrl[0].Sha_Out2[Index2 + offset2]);
        }
    }

    //Set "1" to idle state after reg_read_sha_ready = 1
    SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)|(SHARNG_CTRL_SHA_CLR));
}

void HAL_SHA_SetByPassTable(unsigned char bEnable)
{
    if (bEnable == 1)
    {
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)|SHARNG_CTRL_SHA_BYPASS_TABLE_EN);
    }
    else
    {
        SHA_REG32_W((&_SHARNGCtrl[0].Sha_Ctrl), SHA_REG32_R(&_SHARNGCtrl[0].Sha_Ctrl)&~(SHARNG_CTRL_SHA_BYPASS_TABLE_EN));
    }
}

unsigned char HAL_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf)
{
    HAL_SHA_SetAddress(PAInBuf);
    HAL_SHA_SetLength(Size);
    HAL_SHA_SelMode(eMode);
    HAL_SHA_SetByPassTable(1);
    return 0;
}

