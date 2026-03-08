/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DRV_AES_H_
#define _DRV_AES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define AES_IV_BYTESIZE                (16)
#define AES_128KEY_BYTESIZE            (16)
#define AES_256KEY_BYTESIZE            (32)
#if defined(CONFIG_SECURE_AES256_SHA384)
#define AES_KEY_BYTESIZE               (AES_256KEY_BYTESIZE)
#else
#define AES_KEY_BYTESIZE               (AES_128KEY_BYTESIZE)
#endif
#define AES_UNIT_SIZE                  (2)
#define AESDMA_ENGINE_CBC              (0xA100)
#define AESDMA_ENGINE_ECB              (0x8100)
#define AES_DELAY_10                   (10)
#define AES_TIMEOUT_VALUE              (1200000)

//--------------------------------------------------------------------------------------------------
//  Type and Structure
//--------------------------------------------------------------------------------------------------
typedef enum
{
    E_DRVAESDMA_CIPHER_ECB = 0,
    E_DRVAESDMA_CIPHER_CTR,
    E_DRVAESDMA_CIPHER_CBC,
    E_DRVAESDMA_CIPHER_DES_ECB,
    E_DRVAESDMA_CIPHER_DES_CTR,
    E_DRVAESDMA_CIPHER_DES_CBC,
    E_DRVAESDMA_CIPHER_TDES_ECB,
    E_DRVAESDMA_CIPHER_TDES_CTR,
    E_DRVAESDMA_CIPHER_TDES_CBC,
    E_DRVAESDMA_CIPHER_CTS_CBC,
    E_DRVAESDMA_CIPHER_CTS_ECB,
    E_DRVAESDMA_CIPHER_DES_CTS_CBC,
    E_DRVAESDMA_CIPHER_DES_CTS_ECB,
    E_DRVAESDMA_CIPHER_TDES_CTS_CBC,
    E_DRVAESDMA_CIPHER_TDES_CTS_ECB,
    E_DRVAESDMA_CIPHER_NONE,  //memory copy
    E_DRVAESDMA_CIPHER_SM4_ECB,
    E_DRVAESDMA_CIPHER_SM4_CTR,
    E_DRVAESDMA_CIPHER_SM4_CBC,
    E_DRVAESDMA_CIPHER_CBCS,
    E_DRVAESDMA_CIPHER_CENS,

    E_DRVAESDMA_CIPHER_MODE_MAX,
} DrvAESDMA_CipherMode;

typedef enum
{
    E_AESDMA_SWKEY = 0x0,
    E_AESDMA_SW256KEY = 0x2,
    E_AESDMA_EFUSEKEY0 = 0x10,
    E_AESDMA_EFUSEKEY1,
    E_AESDMA_EFUSE256KEY0 = 0x20,
    E_AESDMA_EFUSE256KEY1,
    E_AESDMA_UNIFORMKEY0 = 0x100,
    E_AESDMA_UNIFORMKEY1,
    E_AESDMA_UNIFORM256KEY0 = 0x110,
    E_AESDMA_UNIFORM256KEY1
}E_AESDMA_KEYTYPE;

//--------------------------------------------------------------------------------------------------
//  Function
//--------------------------------------------------------------------------------------------------
void array_reverse(unsigned char* ary, unsigned int len);
int HW_AESDMA_Encrypt_Ext(unsigned long long inaddr, unsigned long long outaddr, unsigned int len, unsigned int engine, E_AESDMA_KEYTYPE keytype, unsigned char *key, unsigned char *iv);
int HW_AESDMA_Dncrypt_Ext(unsigned long long inaddr, unsigned long long outaddr, unsigned int len, unsigned int engine, E_AESDMA_KEYTYPE keytype, unsigned char *key, unsigned char *iv);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _DRV_AES_H_
