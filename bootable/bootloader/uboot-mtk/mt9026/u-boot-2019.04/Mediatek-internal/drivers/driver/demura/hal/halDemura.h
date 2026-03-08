// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _HAL_DEMURA_H_
#define _HAL_DEMURA_H_

#define CONFIG_TARGET_MT5896_DEMURA 1

#if CONFIG_TARGET_MT5896_DEMURA
#include "DEMURA_BKA377.h"
#include "TCON_ADL_BKA367.h"
#endif

#define DEMURA_MAX_LAYER             8
#define MSTAR_DEMURA_DAT_BIT_LEN     10
#define MSTAR_CHIP_ID  0x00E8  //for7322

#if CONFIG_TARGET_MT5896_DEMURA //bank*0x200+addr*4
#define REG_DEMURA_BASE              0x146EE00 //0xA377*0x200 --> Bank_0xA377
#define REG_DEMURA_DMA_BASE          0x146CE00 //0xA367*0x200 --> Bank_0xA367
#else
#define REG_DEMURA_BASE              0x037700  // --> Bank_0x1377 (Main 0x102F, Sub 0x77)
#define REG_DEMURA_DMA_BASE          0x036700  // --> Bank_0x1367 (Main 0x102F, Sub 0x67)
#endif
#define DEMURA_DMA_ADDR_UNIT         32
#define DEMURA_DMA_REQ_LENGTH        4

#define REG_DMC_DGA_EN_BIT           BIT7
#define REG_DMC_W_EN_BIT             BIT6


#define BK_REG_L( x, y )            ((x) | (((y) << 1)))
#define BK_REG_H( x, y )            (((x) | (((y) << 1))) + 1)

// Demura info
#define L_BK_DEMURA(x)               BK_REG_L(REG_DEMURA_BASE, x)
#define H_BK_DEMURA(x)               BK_REG_H(REG_DEMURA_BASE, x)
#define L_BK_DEMURA_DMA(x)           BK_REG_L(REG_DEMURA_DMA_BASE, x)
#define H_BK_DEMURA_DMA(x)           BK_REG_H(REG_DEMURA_DMA_BASE, x)


// RBGW Panel Register info
#define REG_CHPI_CLKEN           (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[6] = 1 (16 bit mode)
#define REG_CHPI_BYPASS          (0x232600 + (0x01<<1))  // --> 0x3326_0x01[0] = 0 (16 bit mode)

#define REG_CEDS_CLKEN           (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[7] = 1 (16 bit mode)
#define REG_CEDS_BYPASS          (0x232600 + (0x01<<1))  // --> 0x3326_0x01[0] = 0 (16 bit mode)

#define REG_EPI_CLKEN            (0x003200 + (0x2F<<1))  // --> 0x1032_0x2f[7] = 1 (16 bit mode)
#define REG_EPI_BYPASS           (0x230300 + (0x01<<1))  // --> 0x3303_0x01[5] = 0 (16 bit mode)


MS_U16 HAL_DEMURA_Get_PanelWidth(DeMuraBinHeader *pHeader);
MS_U16 HAL_DEMURA_Get_PanelHeight(DeMuraBinHeader *pHeader);

#if 1
void HAL_DEMURA_SetDL_BaseAddr(MS_U32 u32Addr);
void HAL_DEMURA_SetDL_SramIni_Addr(MS_U16 offset);
void HAL_DEMURA_SetDL_Depth(MS_U16 HNodeNum);
void HAL_DEMURA_SetDL_ReqLenth(MS_U16 ReqLenth);
void HAL_DEMURA_SetDL_TriggerMode(AUTODOWNLOAD_TRIG_e eTrigMode);
void HAL_DEMURA_Enable_DL(MS_BOOL bEnable);
#endif
void HAL_DEMURA_EnableDemuraBypass(MS_BOOL bEnable);
void HAL_DEMURA_EnableDemura(MS_BOOL bEnable, DEMURA_PANEL_TYPE ptype);
void HAL_DEMURA_Dump_Layerlevel(void);

MS_U32 HAL_DEMURA_Cal_LutSize(MS_U32 levelCnt, MS_U32 hNode, MS_U32 vNode, MS_U32 sepType);
MS_U32 HAL_DEMURA_Max_RegSize(void);
MS_U32 HAL_DEMURA_Max_RegCount(void);
MS_U32 HAL_DEMURA_Get_PacketType(void);
DEMURA_PANEL_TYPE HAL_DEMURA_Get_PanelType(void);

MS_U8 HAL_DEMURA_Get_HBlockShift(MS_U8 u8Hblock);
MS_U8 HAL_DEMURA_Get_VBlockShift(MS_U8 u8Vblock);
#endif
