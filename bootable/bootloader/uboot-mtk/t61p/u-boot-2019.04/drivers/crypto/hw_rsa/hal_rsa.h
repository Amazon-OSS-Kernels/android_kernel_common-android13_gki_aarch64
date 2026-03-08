/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __HAL_RSA_H__
#define __HAL_RSA_H__

#define RSA_LOWU8MASK            (0xFF)
#define RSA_LOWU16MASK           (0x0000FFFF)
#define RSA_HIGHU8MASK           (0xFF00)
#define RSA_DATALEN              64

#define SHIFT_BITS_8             8
#define SHIFT_BITS_16            16
#define SHIFT_BITS_24            24

typedef enum
{
    E_RSA_ADDRESS_E = 0,
    E_RSA_ADDRESS_N,
    E_RSA_ADDRESS_A,
    E_RSA_ADDRESS_Z,
} RSA_IND32Address;

void HAL_RSA_ClearInt(void);
void HAL_RSA_Reset(void);
void HAL_RSA_Ind32Ctrl(unsigned char dirction);
void HAL_RSA_LoadSram(unsigned int *Buf, RSA_IND32Address eMode);
void HAL_RSA_SetKeyLength(unsigned int keylen);
void HAL_RSA_SetKeyType(unsigned char hwkey, unsigned char pubkey);
void HAL_RSA_ExponetialStart(void);
unsigned int HAL_RSA_GetStatus(void);
void HAL_RSA_FileOutStart(void);
void HAL_RSA_SetFileOutAddr(unsigned int offset);
unsigned int HAL_RSA_FileOut(void);
unsigned int HAL_RSA_Get_RSA_IsFinished(void);

#endif // #ifndef __HAL_RSA_H__