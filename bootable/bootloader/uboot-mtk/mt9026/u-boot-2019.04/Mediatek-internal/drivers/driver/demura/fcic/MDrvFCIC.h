// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MDRV_FCIC_H_
#define _MDRV_FCIC_H_

#define FCIC_CHECKSUM_SIZE    4
#define FCIC_CRC_SIZE         4
#define FCIC_EXTRA_SIZE      (FCIC_CHECKSUM_SIZE + FCIC_CRC_SIZE)

#if(CONFIG_DEMURA_FCIC_SQ1819)
#define FCIC_DATA_START      0x00000
#else
#define FCIC_DATA_START      0x60000
#endif

// Assuming FCICBin Store from Flash address(0x0000)
typedef struct
{
    // Offset: 0x00 ~ 0x03
    MS_U32 BinCRC;

    // Offset: 0x04 ~ 0x07
    MS_U32 LutFlashAddr;

    // Offset: 0x08 ~ 0x0B
    MS_U32 LutSize;

    // Offset: 0x0C ~ 0x0F
    MS_U32 VendorIdSize;

    // Offset: 0x10 ~ 0x13
    MS_U32 FileSize;

    // Offset: 0x14 ~ 0x40
    MS_U32 reserve[11];
} FCICBinHeader;


MS_BOOL MDrv_FCIC_SetIOMapBase(void);
MS_BOOL MDrv_FCIC_Checksum(MS_U8 *lut_buf, MS_U32 offset, MS_U32 lut_len);
MS_BOOL MDrv_FCIC_Set_BDMA(MS_PHYADDR lut_phyAddr, MS_U32 lut_size);
void MDrv_FCIC_TurnOn(MS_BOOL bOnOff);

#endif  //_MDRV_FCIC_H_
