/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _HAL_AES_H_
#define _HAL_AES_H_

#include "u-boot/drv_aes.h"
#include "mtk_mmu.h"

#define RIU                                  ((unsigned short volatile *) RIU_PM_BASE)

#define AESDMA_BASE_ADDR                     (0x29C400)
#define DSCRMB67_BASE_ADDR                   (0x29C580)
#define DSCRMB4_BASE_ADDR                    (0x29C300)

#define SHIFT_BITS_16                        (16)
#define MASK_16                              (0xFFFF)

#define MASK_AESDMA_FILEIN_ADDR_EXT          0x000FUL    //0x16[3:0], 0x56[3:0]
#define MASK_AESDMA_FILEOUT_SADDR_EXT        0x0F00UL    //0x16[11:8], 0x56[11:8]
#define MASK_AESDMA_FILEOUT_EADDR_EXT        0xF000UL    //0x16[15:12], 0x56[15:12]
#define AESDMA_FILEIN_ADDR_EXT_SHIFTBIT      (32)
#define AESDMA_FILEOUT_SADDR_EXT_SHIFTBIT    (AESDMA_FILEIN_ADDR_EXT_SHIFTBIT-8)

#define AESDMA_CTRL_XIU_SEL_CA9              0x1000

#define AESDMA_CTRL_FILE_ST                  0x0001
#define AESDMA_CTRL_FOUT_EN                  0x0100
#define AESDMA_CTRL_CIPHER_DECRYPT           0x0200
#define AESDMA_CTRL_USE_SECRET_KEY           0x1000
#define AESDMA_CTRL_RESERVE                  0x0200  // for  use efuse key1
#define AESDMA_CTRL_192KEY_EN                0x0400
#define AESDMA_CTRL_256KEY_EN                0x0080
#define AESDMA_IS_FINISHED                   0x0001

#define REG_DMAKEY_INDEX_MASK                0x00FF

#define REG_Dma_Addr_Ext_DSCMB3              0x16
#define REG_Dma_Ctrl                         0x50
#define REG_Eng_Ctrl                         0x51
#define REG_Filein_Addr_0                    0x52
#define REG_Filein_Addr_1                    0x53
#define REG_Filein_Num_0                     0x54
#define REG_Filein_Num_1                     0x55
#define REG_Fileout_Addr_0                   0x56
#define REG_Fileout_Addr_1                   0x57
#define REG_Dma4_Ctrl                        0x5D
#define REG_Dma2_Ctrl                        0x5E
#define REG_Eng2_Ctrl                        0x5F
#define REG_Cipher_Key_0                     0x60
#define REG_Cipher_Key_1                     0x60
#define REG_Cipher_IV                        0x68
#define REG_Dma_Done                         0x7F

void HAL_AESDMA_ResetSource(void);
void HAL_AESDMA_SetIV(unsigned int* iv);
void HAL_AESDMA_DisableXIUSelectCA9(void);
void HAL_AESDMA_Reset(void);
void HAL_AESDMA_SetEngine(unsigned int engine);
void HAL_AESDMA_SetDecrypt(unsigned int decrypt);
void HAL_AESDMA_SetKeyIndex(E_AESDMA_KEYTYPE keytype, unsigned int* key);
void HAL_AESDMA_SetFileInAddr(unsigned long long addr);
void HAL_AESDMA_SetFileOutAddr(unsigned long long addr);
void HAL_AESDMA_SetFileSize(unsigned int len);
void HAL_AESDMA_FileOutEnable(unsigned char FileOutEnable);
void HAL_AESDMA_Start(unsigned char AESDMAStart);
unsigned int HAL_AESDMA_IsFinished(void);

#endif // #ifndef _HAL_AES_H_