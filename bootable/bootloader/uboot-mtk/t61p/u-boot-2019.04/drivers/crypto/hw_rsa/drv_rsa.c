// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "hal_rsa.h"
#include "u-boot/drv_rsa.h"

int _HW_RSA_Calculate(Drv_RSASig *pstSign, Drv_RSAKey *pstKey, Drv_RSAMode eMode)
{
    unsigned char Public = 0;
    unsigned int KeyLen = 0;
    unsigned int *pSig = (unsigned int *)((void*)pstSign);
    unsigned int *pKey = (unsigned int *)((void*)pstKey);

    HAL_RSA_Reset();
    HAL_RSA_Ind32Ctrl(1);//ind32_ctrl=0xE0

    switch (eMode) {
    case E_DRV_RSA1024_PUBLIC:
        {
            KeyLen = E_DRV_RSA1024_KeyLen;
            Public = 1;
            break;
        }
    case E_DRV_RSA1024_PRIVATE:
        {
            KeyLen = E_DRV_RSA1024_KeyLen;
            Public = 0;
            break;
        }
    case E_DRV_RSA2048_PUBLIC:
        {
            KeyLen = E_DRV_RSA2048_KeyLen;
            Public = 1;
            break;
        }
    case E_DRV_RSA2048_PRIVATE:
        {
            KeyLen = E_DRV_RSA2048_KeyLen;
            Public = 0;
            break;
        }
    case E_DRV_RSA256_PUBLIC:
        {
            KeyLen = E_DRV_RSA256_KeyLen;
            Public = 1;
            break;
        }
    case E_DRV_RSA256_PRIVATE:
        {
            KeyLen = E_DRV_RSA256_KeyLen;
            Public = 0;
            break;
        }
    default:
        return -1;
    }

    HAL_RSA_LoadSram((pSig+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)-1),E_RSA_ADDRESS_A);
    HAL_RSA_LoadSram((pKey+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)-1),E_RSA_ADDRESS_N);
    HAL_RSA_LoadSram((pKey+(RSA_INPUT_SIZE/RSA_UNIT_SIZE)),E_RSA_ADDRESS_E);

    HAL_RSA_SetKeyLength((KeyLen/RSA_UNIT_SIZE)-1);
    HAL_RSA_SetKeyType(0,Public); //sw key, public key

    HAL_RSA_ExponetialStart();

    return 0;
}

int HW_RSA_Calculate(Drv_RSASig *pstSign, Drv_RSAKey *pstKey, Drv_RSAMode eMode)
{
    return _HW_RSA_Calculate(pstSign,pstKey,eMode);
}

int _HW_RSA_IsFinished(void)
{
    if(!HAL_RSA_Get_RSA_IsFinished())
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int HW_RSA_IsFinished(void)
{
    return _HW_RSA_IsFinished();
}

int _HW_RSA_Output(Drv_RSAMode eMode, Drv_RSAOut *pstRSAOut)
{
    unsigned int i = 0;
    unsigned int KeyLen = 0;

    unsigned int *pRSAOut = (unsigned int *)((void*)pstRSAOut);

    HAL_RSA_Ind32Ctrl(0);//ind32_ctrl=0xC0

    switch (eMode)
    {
    case E_DRV_RSA1024_PUBLIC:
    case E_DRV_RSA1024_PRIVATE:
        KeyLen = E_DRV_RSA1024_KeyLen;
        break;
    case E_DRV_RSA2048_PUBLIC:
    case E_DRV_RSA2048_PRIVATE:
        KeyLen = E_DRV_RSA2048_KeyLen;
        break;
    case E_DRV_RSA256_PUBLIC:
    case E_DRV_RSA256_PRIVATE:
        KeyLen = E_DRV_RSA256_KeyLen;
        break;
    default:
        return -1;
    }

    for( i = 0; i<(KeyLen/RSA_UNIT_SIZE); i++)
    {
        HAL_RSA_SetFileOutAddr(i);
        HAL_RSA_FileOutStart();
        *(pRSAOut+i) = HAL_RSA_FileOut();
    }

    //clear the used key whenever calculating is finished
    HAL_RSA_Reset();

    return 0;
}

int HW_RSA_Output(Drv_RSAMode eMode, Drv_RSAOut *pstRSAOut)
{
    return _HW_RSA_Output(eMode,pstRSAOut);
}

