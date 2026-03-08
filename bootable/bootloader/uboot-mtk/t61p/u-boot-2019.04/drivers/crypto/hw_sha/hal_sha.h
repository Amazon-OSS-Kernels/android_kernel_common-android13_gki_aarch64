/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __HAL_SHA_H__
#define __HAL_SHA_H__

#include "reg_sha.h"
#include "u-boot/drv_sha.h"

#define DIGEST_SIZE_SHA1             5UL   //160-bit
#define DIGEST_SIZE_SHA256           8UL   //256-bit
#define DIGEST_SIZE_SHA384           12UL  //384-bit
#define DIGEST_SIZE_SHA512           16UL  //512-bit
#define DIGEST_OFFSET_SHA384_SHA512  3     //OFFSET

#define REG_BITS                     8
#define SHIFT_BITS                   16
#define SHA_REG_RANGE                7
#define LOW_16_MASK                  (0x0000FFFF)

void HAL_SHA_Reset(void);
unsigned int HAL_SHA_Get_Status(void);
void HAL_SHA_SelMode(Drv_SHAMode eMode);
void HAL_SHA_SetAddress(unsigned long long Addr);
void HAL_SHA_SetLength(unsigned int Size);
void HAL_SHA_Start(unsigned char SHAStart);
void HAL_SHA_Out(Drv_SHAMode eMode, unsigned long long Buf);
void HAL_SHA_SetByPassTable(unsigned char bEnable);
unsigned char HAL_SHA_Calculate(Drv_SHAMode eMode, unsigned long long PAInBuf, unsigned int Size, unsigned long long PAOutBuf);

#endif // #ifndef __HAL_SHA_H__
