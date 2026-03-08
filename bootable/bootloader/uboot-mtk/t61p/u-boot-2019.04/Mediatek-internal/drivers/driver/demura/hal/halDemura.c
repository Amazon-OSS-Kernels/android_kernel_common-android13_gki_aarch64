// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsCommon.h>
#include <command.h>
#include <debug_impl.h>
#include <apiPNL.h>
#include <linux/string.h>
#include <halRegOp.h>
#include <MDrvDemura.h>
#include "halDemura.h"
#include "ms_vector.h"
#include "demura.h"
#include "demura_common.h"

#define _1 1
#define _2 2
#define _3 3
#define _4 4
#define _5 5
#define _6 6
#define _7 7
#define _8 8
#define _16 16
#define _32 32
#define S7_MONO_reg_dmc_h_lut_num_max 132
#define DEMURA_ADL_ALGINED_BYTES (32)
#define DEMURA_GETS7_NODE_ALIGN(planes, mode, u8Nodes) \
({ if (mode) {u8Nodes = (planes >= 3)?2:4;} \
   else {u8Nodes = (planes >= 5)?4:((planes >= 3)?8:16);}})

MS_U16 HAL_DEMURA_Get_PanelWidth(DeMuraBinHeader *pHeader)
{
    MS_U16 panel_width;
    MS_U16 packet_nodes;

    if (pHeader->nPacketSize > 1)
    {
        DEBUG_DEMURA("Do not support nPacketSize = %d\n", pHeader->nPacketSize);
        return 0;
    }

    if (pHeader->u4EndLayer > 4)
    {
        packet_nodes = 1;
        if (pHeader->nPacketSize == 0)
        {
            DEBUG_DEMURA("This PacketSize(%d) can not contain Layers(%d)\n",
                         DEMURA_PACKET_SIZE(pHeader->nPacketSize), pHeader->u4EndLayer - pHeader->u4StartLayer);
            return 0;
        }
    }
    else if (pHeader->u4EndLayer > 2)
    {
        packet_nodes = (2 >> (1 - pHeader->nPacketSize));
    }
    else
    {
        packet_nodes = (4 >> (1 - pHeader->nPacketSize));
    }

    panel_width = packet_nodes * (1 << (pHeader->nHBlockSize)) * (pHeader->u16HNodeCount - 1);
    if (pHeader->bSeperateRGB == 0)
    {
        // Mono mode: R/G/B channel used the same table
        panel_width = panel_width * 4;
    }

    DEBUG_DEMURA("The panel_width in LUT is %d\n", panel_width);
    return panel_width;
}


MS_U16 HAL_DEMURA_Get_PanelHeight(DeMuraBinHeader *pHeader)
{
    MS_U16 panel_height;
    panel_height = (1 << (pHeader->nVBlockSize)) * (pHeader->u16VNodeCount - 1);
    DEBUG_DEMURA("The panel_height in LUT is %d\n", panel_height);
    return panel_height;
}

void HAL_DEMURA_SetDL_BaseAddr(MS_U32 u32Addr)
{
    MS_U32 base_addr = u32Addr / DEMURA_DMA_ADDR_UNIT;
    DEBUG_DEMURA("%s@%d Addr = %x  %x\n", __FUNCTION__,__LINE__, u32Addr, base_addr);

#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2Byte(REG_0018_TCON_ADL_BKA367, (MS_U16)(base_addr & 0xFFFF),
        (REG_0018_TCON_ADL_BKA367_REG_CLIENT3_BASEADR_0));
    HAL_DEMURA_Write2ByteMask(REG_001C_TCON_ADL_BKA367, (base_addr >> 16),
        (REG_001C_TCON_ADL_BKA367_REG_CLIENT3_BASEADR_1));
#else
    HAL_DEMURA_Write2Byte(L_BK_DEMURA_DMA(0x06), (MS_U16)(base_addr & 0xFFFF));
    HAL_DEMURA_Write2ByteMask(L_BK_DEMURA_DMA(0x07), (base_addr >> 16), 0x07FF);
#endif
}


void HAL_DEMURA_SetDL_SramIni_Addr(MS_U16 offset)
{
#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2Byte(REG_0040_TCON_ADL_BKA367, offset,
        HAL_DEMURA_Fld2Mask(REG_0040_TCON_ADL_BKA367_REG_CLIENT3_INI_ADDR));
#else
    HAL_DEMURA_Write2Byte(L_BK_DEMURA_DMA(0x10), offset);
#endif
}


void HAL_DEMURA_SetDL_Depth(MS_U16 HNodeNum)
{
#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2Byte(REG_0028_TCON_ADL_BKA367, HNodeNum,
        (REG_0028_TCON_ADL_BKA367_REG_CLIENT3_DEPTH));
#else
    HAL_DEMURA_Write2Byte(L_BK_DEMURA_DMA(0x0A), HNodeNum);
#endif
}


void HAL_DEMURA_SetDL_ReqLenth(MS_U16 ReqLenth)
{
#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2Byte(REG_0034_TCON_ADL_BKA367, ReqLenth,
        (REG_0034_TCON_ADL_BKA367_REG_CLIENT3_REQ_LEN));
#else
    HAL_DEMURA_Write2Byte(L_BK_DEMURA_DMA(0x0D), ReqLenth);
#endif
}

void HAL_DEMURA_SetDL_TriggerMode(AUTODOWNLOAD_TRIG_e eTrigMode)
{
#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2ByteMask(REG_0004_TCON_ADL_BKA367,
        ((eTrigMode == AUTO_DL_TRIG_MODE) ? 0 : 1),
        (REG_0004_TCON_ADL_BKA367_REG_CLIENT3_TRIG_MODE));
#else
    HAL_DEMURA_Write2ByteMask((L_BK_DEMURA_DMA(0x01)), (eTrigMode << 5), BIT5);
#endif
}

void HAL_DEMURA_Enable_DL(MS_BOOL bEnable)
{
#ifdef CONFIG_DEMURA_MT5896
    HAL_DEMURA_Write2ByteMask(REG_0004_TCON_ADL_BKA367, (bEnable ? 1 : 0),
        (REG_0004_TCON_ADL_BKA367_REG_CLIENT3_EN));
#else
    HAL_DEMURA_Write2ByteMask((L_BK_DEMURA_DMA(0x01)), (bEnable << 2), BIT2);
#endif
}

void HAL_DEMURA_EnableDemuraBypass(MS_BOOL bEnable)
{
    DEBUG_DEMURA("EnableDemuraBypass %d\n", bEnable);
    if (bEnable == TRUE)
    {
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 1, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS);
    }
    else
    {
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS);
    }
}

void HAL_DEMURA_EnableDemura(MS_BOOL bEnable, DEMURA_PANEL_TYPE ptype)
{
#ifdef CONFIG_DEMURA_MT5896

    if(bEnable)
    {
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 1, REG_00BC_DEMURA_BKA377_REG_DMC_R_EN);
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 1, REG_00BC_DEMURA_BKA377_REG_DMC_G_EN);
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 1, REG_00BC_DEMURA_BKA377_REG_DMC_B_EN);
        // HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS);
    }
    else
    {
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0, REG_00BC_DEMURA_BKA377_REG_DMC_R_EN);
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0, REG_00BC_DEMURA_BKA377_REG_DMC_G_EN);
        HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0, REG_00BC_DEMURA_BKA377_REG_DMC_B_EN);
        // HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 1, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS);
    }

#else
    MS_U16 reg_val  = 0;
    MS_U16 reg_mask = 0;

    reg_mask = (BIT8 | REG_DMC_DGA_EN_BIT | REG_DMC_W_EN_BIT | BIT2|BIT1|BIT0);

    switch (ptype)
    {
        case E_DEMURA_RGB_PANEL:
            reg_val  = BIT2|BIT1|BIT0;
            break;
        case E_DEMURA_RGBW_PANEL:
            reg_val = (REG_DMC_DGA_EN_BIT | REG_DMC_W_EN_BIT | BIT2|BIT1|BIT0);
            break;
        default:
            break;
    }

    if (bEnable == TRUE)
    {
        HAL_DEMURA_Write2ByteMask((L_BK_DEMURA(0x2F)),  reg_val, reg_mask);
    }
    else
    {
        HAL_DEMURA_Write2ByteMask((L_BK_DEMURA(0x2F)),  0x00, reg_mask);
    }

    #if(CONFIG_DEMURA_FCIC_SRAM_SHARE)
        reg_val = HAL_DEMURA_Read2Byte(0x0133A8);
        reg_val |= BIT2;
        HAL_DEMURA_Write2Byte(0x0133A8, reg_val);
    #endif
#endif
}


void HAL_DEMURA_Dump_Layerlevel(void)
{
    MS_U32 i = 0;
    MS_U32 layer_num = 8;
    MS_U32 layer_reg_start = 0x11;

    printf("Layer level(8 bit): ");
    for (i = 0; i < layer_num; i++)
    {
        MS_U32 addr = layer_reg_start + i;
        MS_U32 val  = HAL_DEMURA_Read2bytemask(L_BK_DEMURA(addr), 0x0FFF);
        printf("%d ", (int)(val >> 4));  // 12bit --> 8bit
    }
    printf("\n");
}


/******************** Convert Part ********************/

MS_U32 HAL_DEMURA_Cal_LutSize(MS_U32 levelCnt, MS_U32 hNode, MS_U32 vNode, MS_U32 sepType)
{
    int  LevelCount;
    MS_U32 layer_dat_size;   // unit : byte
    MS_U32 node_num;

    switch (levelCnt)
    {
        case 8:
        case 7:
        case 6:
        case 5:
            LevelCount = _8;
            node_num = _4;
            break;
        case 4:
        case 3:
            LevelCount = _4;
            node_num = _8;
            break;
        case 2:
        case 1:
            LevelCount = _2;
            node_num = _16;
            break;
        default:
            LevelCount = _8;
            node_num = _4;
            break;
    }

    if (demura_get_version() == E_DEMURA_VERSION_S11)
    {
        // 1 : R/G/B sperate ;  0 : R/G/B share. from reg_dmc_rgb_mode;
        layer_dat_size = LevelCount * (hNode/7+1) * vNode * ((sepType==1) ? 3*8 : 1*8);

        //layer_dat_size = (layer_dat_size/7) + 1; //Take Ceiling
        return layer_dat_size;
    }

    if (demura_get_version() == E_DEMURA_VERSION_S7_MONO)
    {

        if (((hNode / node_num) + ((hNode % node_num == 0) ? 0 : 1)) > S7_MONO_reg_dmc_h_lut_num_max)
        {
            DEBUG_DEMURA("reg_dmc_h_lut_num can not exceed 132\n");
        }
        if (sepType == 1)
        {
            sepType = 0;
            DEBUG_DEMURA("demura_get_version: %d, not support rgb mode,sepType = %d, reg_dmc_h_lut_num = %d\n", demura_get_version(), sepType, ((hNode / node_num) + ((hNode % node_num == 0) ? 0 : 1)));
        }
    }

    DEMURA_GETS7_NODE_ALIGN(levelCnt, sepType, LevelCount);
    LevelCount = (hNode + (LevelCount - 1)) / LevelCount;
    layer_dat_size = LevelCount * vNode * DEMURA_ADL_ALGINED_BYTES;
    DEBUG_DEMURA("LevelCount = %d, sepType = %d, layer_dat_size = 0x%x\n", LevelCount, sepType, layer_dat_size);
    return layer_dat_size;
}


MS_U32 HAL_DEMURA_Max_RegSize(void)
{
   return (256 * 5);   // unit : byte
}


MS_U32 HAL_DEMURA_Max_RegCount(void)
{
   return   256;   // unit : byte
}

MS_U32 HAL_DEMURA_Get_PacketType(void)
{
    // 1 : 256bit
    return 1;
}


DEMURA_PANEL_TYPE HAL_DEMURA_Get_PanelType(void)
{
#if CONFIG_DEMURA_MT5896
    return E_DEMURA_RGB_PANEL;
#else
    DEMURA_PANEL_TYPE pret = E_DEMURA_RGB_PANEL;


    if ((HAL_DEMURA_Read2ByteMask(REG_CHPI_CLKEN, BIT6) == BIT6) \
         && (HAL_DEMURA_Read2ByteMask(REG_CHPI_BYPASS, BIT0) == 0))
    {
        // Interface CHPI
        pret = E_DEMURA_RGBW_PANEL;
    }
    else if ((HAL_DEMURA_Read2ByteMask(REG_CEDS_CLKEN, BIT7) == BIT7) \
         && (HAL_DEMURA_Read2ByteMask(REG_CEDS_BYPASS, BIT0) == 0))
    {
        // Interface CEDS
        pret = E_DEMURA_RGBW_PANEL;
    }
    else if ((HAL_DEMURA_Read2ByteMask(REG_EPI_CLKEN, BIT7) == BIT7) \
         && (HAL_DEMURA_Read2ByteMask(REG_EPI_BYPASS, BIT5) == 0))
    {
        // Interface EPI
        pret = E_DEMURA_RGBW_PANEL;
    }
    else
    {
        pret = E_DEMURA_RGB_PANEL;
    }
    return pret;
#endif
}

MS_U8 HAL_DEMURA_Get_HBlockShift(MS_U8 u8Hblock)
{
    if (u8Hblock == 4)
    {
        return 2;
    }
    else if (u8Hblock == 8)
    {
        return 3;
    }
    else if (u8Hblock == 16)
    {
        return 4;
    }
    return 0;
}

MS_U8 HAL_DEMURA_Get_VBlockShift(MS_U8 u8Vblock)
{
    if (u8Vblock == 4)
    {
        return 2;
    }
    else if (u8Vblock == 8)
    {
        return 3;
    }
    else if (u8Vblock == 16)
    {
        return 4;
    }
    return 0;
}

