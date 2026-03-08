// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "reg_rsa.h"
#include "hal_rsa.h"

static REG_SHARNGCtrl        *_SHARNGCtrl = (REG_SHARNGCtrl*)REG_SHARNGCTRL_BASE;

#define RSA_REG32_W(reg, value)     do {                                   \
                                    (reg)->H = ((value) >> SHIFT_BITS_16); \
                                    (reg)->L = ((value) & RSA_LOWU16MASK); \
                                    } while(0)

unsigned int RSA_REG32_R(REG32 *reg)
{
    unsigned int value;

    value = (reg)->H << SHIFT_BITS_16;
    value |= (reg)->L;

    return value;
}

void HAL_RSA_ClearInt(void)
{
    //RSA interrupt clear
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_RData), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_RData)|(RSA_INT_CLR));
}

void HAL_RSA_Reset(void)
{
    //RSA Rst
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)|(RSA_CTRL_RSA_RST));
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)&(~RSA_CTRL_RSA_RST));

    //add polling RSA status before load data to SRAM
    while ((HAL_RSA_GetStatus() & RSA_STATUS_RSA_BUSY) != 0);
}

void HAL_RSA_Ind32Ctrl(unsigned char dirction)
{
    //[1] reg_ind32_direction 0: Read. 1: Write
    if (dirction == 1)
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Start), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Start)|(RSA_IND32_CTRL_DIRECTION_WRITE));
    }
    else
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Start), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Start)&(~RSA_IND32_CTRL_DIRECTION_WRITE));
    }

    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Start), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Start)|(RSA_IND32_CTRL_ADDR_AUTO_INC|RSA_IND32_CTRL_ACCESS_AUTO_START));
}

void HAL_RSA_LoadSram(unsigned int *Buf, RSA_IND32Address eMode)
{
    unsigned int Cmd = 0;
    unsigned int i = 0;
    unsigned int WData_H = 0;
    unsigned int WData_L = 0;

    switch (eMode)
    {
    case E_RSA_ADDRESS_E:
        Cmd |= (RSA_E_BASE_ADDR);
        break;

    case E_RSA_ADDRESS_N:
        Cmd |= (RSA_N_BASE_ADDR);
        break;

    case E_RSA_ADDRESS_A:
        Cmd |= (RSA_A_BASE_ADDR);
        break;

    default:
        return;
    }

    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Addr), ((RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Addr)&(~RSA_ADDRESS_MASK))|Cmd));
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Start), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Start)|(RSA_INDIRECT_START));

    for (i=0; i<RSA_DATALEN; i++)
    {
        if (eMode == E_RSA_ADDRESS_E)
        {
            WData_L = (((*(Buf+i))>>SHIFT_BITS_8)&RSA_HIGHU8MASK)|(((*(Buf+i))>>SHIFT_BITS_24)&RSA_LOWU8MASK);
            WData_H = (((*(Buf+i))>>SHIFT_BITS_8)&RSA_LOWU8MASK)|(((*(Buf+i))<<SHIFT_BITS_8)&RSA_HIGHU8MASK);
        }
        else
        {
            WData_L = (((*(Buf-i))>>SHIFT_BITS_8)&RSA_HIGHU8MASK)|(((*(Buf-i))>>SHIFT_BITS_24)&RSA_LOWU8MASK);
            WData_H = (((*(Buf-i))>>SHIFT_BITS_8)&RSA_LOWU8MASK)|(((*(Buf-i))<<SHIFT_BITS_8)&RSA_HIGHU8MASK);
        }

        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Addr), ((RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Addr)&(~RSA_WDATA_MASK_L))|(WData_L<<SHIFT_BITS_16)));
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_WData), ((RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_WData)&(~RSA_WDATA_MASK_H))|WData_H));
    }

}

void HAL_RSA_SetKeyLength(unsigned int keylen)
{
    //[13:8] n_len_e: key length
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), (RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)&(~RSA_CTRL_KEY_LENGTH_MASK))|(keylen<<SHIFT_BITS_8));
}

void HAL_RSA_SetKeyType(unsigned char hwkey, unsigned char pubkey)
{
    //[1] hw_key_e : 0 : software key, 1: hardware key
    //[2] e_pub_e : 0: pvivate key, 1: public key
    if (hwkey == 1)
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)|(RSA_CTRL_SEL_HW_KEY));
    }
    else
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)&(~RSA_CTRL_SEL_HW_KEY));
    }

    if (pubkey == 1)
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)|(RSA_CTRL_SEL_PUBLIC_KEY));
    }
    else
    {
        RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ctrl), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl)&(~RSA_CTRL_SEL_PUBLIC_KEY));
    }

}

void HAL_RSA_ExponetialStart(void)
{
    //RSA exp start
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_RData), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_RData)|(RSA_EXP_START));
}

unsigned int HAL_RSA_GetStatus(void)
{
    return RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ctrl) & RSA_STATUS_MASK;
}

void HAL_RSA_FileOutStart(void)
{
    //RSA ind32_start
    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Start), RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Start)|(RSA_INDIRECT_START));
}

void HAL_RSA_SetFileOutAddr(unsigned int offset)
{
    unsigned long Addr = 0;
    Addr = RSA_Z_BASE_ADDR + offset;

    RSA_REG32_W((&_SHARNGCtrl[0].Rsa_Ind32_Addr), ((RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_Addr)&(~RSA_ADDRESS_MASK))|Addr));
}

unsigned int HAL_RSA_FileOut(void)
{
    unsigned int out_L = 0;
    unsigned int out_H = 0;

    out_L = (RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_WData)&RSA_RDATA_MASK_L)>>SHIFT_BITS_16;
    out_H = (RSA_REG32_R(&_SHARNGCtrl[0].Rsa_Ind32_RData)&RSA_RDATA_MASK_H)<<SHIFT_BITS_16;

    return (out_H|out_L);
}

unsigned int HAL_RSA_Get_RSA_IsFinished(void)
{
    return (HAL_RSA_GetStatus() & RSA_STATUS_RSA_BUSY);
}

