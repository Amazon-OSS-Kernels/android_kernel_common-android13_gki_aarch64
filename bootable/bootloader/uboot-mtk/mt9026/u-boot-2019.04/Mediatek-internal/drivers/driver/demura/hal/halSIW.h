// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _HAL_SIW_H_
#define _HAL_SIW_H_

/* SIW Flash Layout define */
#define SIW_LUT_OFFSET              0x1000      // 0x1000: data include 0~0x1000 in LXS format, 0: exclude 0~0x1000 in LXS format
#define SIW_LUT_SIZE                0xEEAE1     // 0xEFAE0-0x1000+1
#define SIW_LUT_CRC_SIZE            0x2         // 0xEFAE2-0xEFAE1+1
#define SIW_SPOC_REG_OFFSET         0xEFAE3
#define SIW_SPOC_REG_SIZE           0x2F        // 0xEFB11-0xEFAE3+1
#define SIW_SPOC_REG_CRC_SIZE       0x2         // 0xEFB13-0xEFB12+1
#define SIW_SPOC_REG_NUM            47

#define SIW_DATA_SIZE 1048576
// #define SIW_DATA_SIZE 977684 // SIW_LUT_SIZE + SIW_LUT_CRC_SIZE + SIW_SPOC_REG_SIZE + SIW_SPOC_REG_CRC_SIZE

#define DEMURA_DMA_ADDR_UNIT        32

void HAL_SIW_Demura_Big_Endian(MS_BOOL bBigEndian);
void HAL_SIW_Demura_Enable_DL(MS_BOOL bEnable);
void HAL_SIW_Demura_SetDL_Depth(void);
void HAL_SIW_Demura_SetDL_ReqLenth(void);
void HAL_SIW_Demura_SetDL_BaseAddr(MS_PHYADDR u32Addr);
void HAL_SIW_Demura_RD_Done(MS_BOOL bDone);
void HAL_SIW_Demura_SPOC_Reg(MS_U8 *pReg);

#if(TEST_SIW_DEMURA == 1)
    void HAL_SIW_Demura_Bypass(MS_BOOL b_bypass);
#endif

#endif
