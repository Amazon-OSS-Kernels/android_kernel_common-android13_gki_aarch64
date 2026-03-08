/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DRV_RSA_H_
#define _DRV_RSA_H_

#ifdef __cplusplus
extern "C"
{
#endif

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------
#define RSA_DELAY_10 10
#define RSA_TIMEOUT_VALUE 1200000

typedef enum
{
    E_DRV_RSA1024_PUBLIC = 0,
    E_DRV_RSA1024_PRIVATE,
    E_DRV_RSA2048_PUBLIC,
    E_DRV_RSA2048_PRIVATE,
    E_DRV_RSA256_PUBLIC,
    E_DRV_RSA256_PRIVATE,
} Drv_RSAMode;

typedef enum
{
    E_DRV_RSA256_KeyLen = 32,
    E_DRV_RSA1024_KeyLen = 128,
    E_DRV_RSA2048_KeyLen = 256,
} Drv_RSAKeyLen;

#define RSA_INPUT_SIZE               256
#define RSA_UNIT_SIZE                4

//--------------------------------------------------------------------------------------------------
//  Type and Structure
//--------------------------------------------------------------------------------------------------
typedef struct
{
    unsigned int KeyN[64];
    unsigned int KeyE[64];

}Drv_RSAKey;

typedef struct
{
    unsigned int Sig[64];

}Drv_RSASig;

typedef struct
{
    unsigned int RSAOut[64];

}Drv_RSAOut;

//--------------------------------------------------------------------------------------------------
//  Function Prototype
//--------------------------------------------------------------------------------------------------
int HW_RSA_Calculate(Drv_RSASig *pstSign, Drv_RSAKey *pstKey, Drv_RSAMode eMode);
int HW_RSA_IsFinished(void);
int HW_RSA_Output(Drv_RSAMode eMode, Drv_RSAOut *pstRSAOut);
int _HW_RSA_Calculate(Drv_RSASig *pstSign, Drv_RSAKey *pstKey, Drv_RSAMode eMode);
int _HW_RSA_IsFinished(void);
int _HW_RSA_Output(Drv_RSAMode eMode, Drv_RSAOut *pstRSAOut);

#endif // #ifndef _DRV_RSA_H_