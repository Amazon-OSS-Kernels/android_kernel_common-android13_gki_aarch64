// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <MsCommon.h>
#include <debug_impl.h>
//#include <apiPNL.h>
#include <linux/string.h>
#include <ms_utils.h>
#include <MDrvDemura.h>
#include <halDemura.h>
//#include <drvMMIO.h>
#include <halRegOp.h>
#include <crc_libs.h>
#include "apiDemura.h"
#include "DEMURA_BKA377.h"
#include "CommonDataType.h"
#include "demura.h"
#include "demuraDll.h"
#include "halDemura.h"

#ifdef ALIGN
#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#endif


MS_BOOL MDrv_DEMURA_SetIOMapBase(void)
{
#ifndef CONFIG_DEMURA_MT5896
    MS_VIRT  NonPM_Addr = 0;
    MS_PHY  NonPM_Size = 0;

    if (FALSE == MDrv_MMIO_GetBASE(&NonPM_Addr, &NonPM_Size, MS_MODULE_BDMA)) {  // Wait for utopia to fix it
        UBOOT_ERROR("Get IOMAP Base faill!\n");
        return FALSE;
    }
    UBOOT_DEBUG("Get IOMAP ID:%u Base:%lx!\n", MS_MODULE_BDMA, (ulong)NonPM_Addr);
    HAL_DEMURA_SetIOMapBase(NonPM_Addr);
#endif
    return TRUE;
}


MS_BOOL MDrv_DEMURA_Check_HeaderCRC(MS_U8 *pBinBuf)
{
    MS_U32 calHeaderCRC = 0;
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    int idx = 0;
    MS_U32 u32HeaderCRC = UFC_HEADER_GET_HEADERCRC((ST_DEMURA_UFC_HEADER *)pBinBuf);
    UBOOT_TRACE("IN\n");

    for (; idx < UFC_DEMURA_HEADER_CHECKSUM_NUM; idx++)
    {
        calHeaderCRC += pBinBuf[idx];
    }

    if(u32HeaderCRC != calHeaderCRC)
    {
        UBOOT_ERROR("Calculate HeaderCRC = 0x%x\n", (uint)calHeaderCRC);
        UBOOT_ERROR("Binary    HeaderCRC = 0x%x\n", (uint)u32HeaderCRC);
        return FALSE;
    }

#else
    DeMuraBinHeader *pHdr;
    UBOOT_TRACE("IN\n");
    pHdr = (DeMuraBinHeader *)pBinBuf;
    calHeaderCRC = MDrv_CRC32_Cal_DeMura(pBinBuf+8, sizeof(DeMuraBinHeader)-8);
    if(calHeaderCRC != pHdr->u32HeaderCRC)
    {
        UBOOT_ERROR("Calculate HeaderCRC = 0x%x\n", (uint)calHeaderCRC);
        UBOOT_ERROR("Binary    HeaderCRC = 0x%x\n", (uint)pHdr->u32HeaderCRC);
        return FALSE;
    }
#endif
    UBOOT_TRACE("OK\n");
    return TRUE;
}


MS_BOOL MDrv_DEMURA_Check_AllBinCRC(MS_U8 *pBinBuf)
{
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
#else

    MS_U32 calAllBinCRC;
    DeMuraBinHeader *pHdr;

    UBOOT_TRACE("IN\n");
    pHdr= (DeMuraBinHeader *)pBinBuf;
    calAllBinCRC = MDrv_CRC32_Cal_DeMura((pBinBuf+4), (pHdr->u32AllBinSize - 4));
    if(calAllBinCRC != pHdr->u32AllBinCRC)
    {
        UBOOT_DEBUG("Calculate AllBinCRC = 0x%x\n", (uint)calAllBinCRC);
        UBOOT_DEBUG("Binary    AllBinCRC = 0x%x\n", (uint)pHdr->u32AllBinCRC);
        return FALSE;
    }
    UBOOT_TRACE("OK\n");
#endif
    return TRUE;
}


MS_BOOL MDrv_DEMURA_On_Resume(MS_U8 *pHeader)
{
    if (MDrv_DEMURA_Check_HeaderCRC(pHeader) == FALSE)
    {
        UBOOT_ERROR("MDrv_DEMURA_Check_HeaderCRC fail\n");
        return FALSE;
    }
    return TRUE;
}


MS_BOOL MDrv_DEMURA_Is_Support(MS_U8 *pData, Demura_Panel_Data panel_data)
{
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    ST_DEMURA_UFC_HEADER *pHdr = (ST_DEMURA_UFC_HEADER *)pData;
    if (UFC_HEADER_GET_H_SIZE(pHdr) != panel_data.u16PanelWidth)
    {
        UBOOT_ERROR("panel_data.u16PanelWidth:%d %d\n", panel_data.u16PanelWidth,
            UFC_HEADER_GET_H_SIZE(pHdr));
        return FALSE;
    }
    if (UFC_HEADER_GET_V_SIZE(pHdr) != panel_data.u16PanelHeight)
    {
        UBOOT_ERROR("panel_data.u16PanelHeight:%d,%d\n", panel_data.u16PanelHeight,
            UFC_HEADER_GET_V_SIZE(pHdr));
        return FALSE;
    }

    if (pHdr->plane_num > DEMURA_MAX_LAYER)
    {
        UBOOT_ERROR("Error: plane number (%d) is invalid\n", pHdr->plane_num);
        return FALSE;
    }

    if (UFC_HEADER_GET_LUT_SIZE(pHdr) == 0)
    {
        UBOOT_ERROR("Error: default bin.\n");
        return FALSE;
    }

#else
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *) pData;
    MS_U16 Demura_LayerCount;

    DEBUG_DEMURA("IN\n");
    if(strncmp((const char *)&(pHdr->u8DemuraID),"mstar demura", 12) != 0)
    {
        UBOOT_ERROR("Error, Demura ID compare fail.\n");
        UBOOT_ERROR("Demura ID != 'mstar demura'\n");
        return FALSE;
    }
    if ( pHdr->u4StartLayer    > DEMURA_MAX_LAYER
         ||(pHdr->u4EndLayer   > DEMURA_MAX_LAYER)
         ||(pHdr->u4StartLayer > pHdr->u4EndLayer))
    {
        UBOOT_ERROR("Error: Layer Counter Start(%d) and End(%d)  is invalid\n", pHdr->u4StartLayer
                     , pHdr->u4EndLayer);
        return FALSE;
    }
    Demura_LayerCount = (pHdr->u4EndLayer - pHdr->u4StartLayer + 1);
    UBOOT_TRACE("Demura_LayerCount = 0x%x\n", Demura_LayerCount);

    if (HAL_DEMURA_Get_PanelWidth(pHdr) != panel_data.u16PanelWidth)
    {
        UBOOT_ERROR("panel_data.u16PanelWidth = %d,%d\n", panel_data.u16PanelWidth,
            HAL_DEMURA_Get_PanelWidth(pHdr));
        return FALSE;
    }
    if (HAL_DEMURA_Get_PanelHeight(pHdr) != panel_data.u16PanelHeight)
    {
        UBOOT_ERROR("panel_data.u16PanelHeight = %d,%d\n", panel_data.u16PanelHeight,
            HAL_DEMURA_Get_PanelHeight(pHdr));
        return FALSE;
    }
#endif
    DEBUG_DEMURA("OK\n");
    return TRUE;
}


void MDrv_DEMURA_Dump_BinInfo(MS_U8 *pData)
{
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    int i;
    ST_DEMURA_UFC_HEADER *pHdr = (ST_DEMURA_UFC_HEADER *)pData;
    DEBUG_DEMURA("version = 0x%x \n", (MS_U32)UFC_HEADER_GET_VERSION(pHdr));
    DEBUG_DEMURA("hdr crc = 0x%x\n",  (uint)UFC_HEADER_GET_HEADERCRC(pHdr));
    DEBUG_DEMURA("hdr size = %u \n", (MS_U32)UFC_HEADER_GET_HEADER_SIZE(pHdr));
    DEBUG_DEMURA("lut size = %u \n", (MS_U32)UFC_HEADER_GET_LUT_SIZE(pHdr));
    DEBUG_DEMURA("h size = %u \n", (MS_U32)UFC_HEADER_GET_H_SIZE(pHdr));
    DEBUG_DEMURA("v size = %u \n", (MS_U32)UFC_HEADER_GET_V_SIZE(pHdr));
    DEBUG_DEMURA("plane black = %u \n", (MS_U32)UFC_HEADER_GET_BLACKLIMIT(pHdr));
    for (i = 0; i < UFC_HEADER_PLANE_NUM; i++)
    {
        DEBUG_DEMURA("plane %d = %u \n", i, (MS_U32)UFC_HEADER_GET_PLANE(pHdr, i));
    }
    DEBUG_DEMURA("plane white = %u \n", (MS_U32)UFC_HEADER_GET_WHITELIMIT(pHdr));

    for (i = 0; i < UFC_HEADER_OFFSET_NUM; i++)
    {
        DEBUG_DEMURA("offset %d = %u %u %u\n", i,
            (MS_U32)UFC_HEADER_GET_R_OFFSET(pHdr, i),
            (MS_U32)UFC_HEADER_GET_G_OFFSET(pHdr, i),
            (MS_U32)UFC_HEADER_GET_B_OFFSET(pHdr, i));
    }

    for (i = 0; i < UFC_HEADER_GAIN_NUM; i++)
    {
        DEBUG_DEMURA("gain %d = %u %u %u\n", i, pHdr->gain_r[i], pHdr->gain_g[i], pHdr->gain_b[i]);
    }
#else
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    char *pdate;
    MS_U8 i;
    extern EN_DEBUG_LEVEL dbgLevel;
    DEBUG_DEMURA("IN\n");

    DEBUG_DEMURA("AllBinCRC   = 0x%x\n",  (uint)pHdr->u32AllBinCRC);
    DEBUG_DEMURA("HeaderCRC   = 0x%x\n",  (uint)pHdr->u32HeaderCRC);
    DEBUG_DEMURA("HeaderSize  = 0x%x\n",  (uint)pHdr->u32HeaderSize);
    DEBUG_DEMURA("AllBinSize  = 0x%x\n",  (uint)pHdr->u32AllBinSize);
    DEBUG_DEMURA("DemuraID    = ");
    for(i = 0; i < sizeof(pHdr->u8DemuraID); i++)
    {
        #if (DBG_DEMURA == 1)
        if (dbgLevel & EN_DEBUG_LEVEL_DEBUG)
            printf("%02x ", pHdr->u8DemuraID[i]);
        #endif
    }
    if (dbgLevel & EN_DEBUG_LEVEL_DEBUG)
        printf("\n");

    DEBUG_DEMURA("BinVersion   = 0x%x\n",     (uint)pHdr->u16BinVersion);
    DEBUG_DEMURA("LayerDataFomrat = 0x%x\n",  (uint)pHdr->u8LayerDataFomrat);
    DEBUG_DEMURA("bR_ch_Enable = 0x%x\n",     (uint)pHdr->bR_ch_Enable);
    DEBUG_DEMURA("bG_ch_Enable = 0x%x\n",     (uint)pHdr->bG_ch_Enable);
    DEBUG_DEMURA("bB_ch_Enable = 0x%x\n",     (uint)pHdr->bB_ch_Enable);

    DEBUG_DEMURA("HNodeCount  = 0x%x\n",      (uint)pHdr->u16HNodeCount);
    DEBUG_DEMURA("VNodeCount  = 0x%x\n",      (uint)pHdr->u16VNodeCount);
    DEBUG_DEMURA("StartLayer  = 0x%x\n",      (uint)pHdr->u4StartLayer);
    DEBUG_DEMURA("EndLayer    = 0x%x\n",      (uint)pHdr->u4EndLayer);
    DEBUG_DEMURA("SeperateRGB = 0x%x\n",      (uint)pHdr->bSeperateRGB);

    DEBUG_DEMURA("LayerDataAddr    = 0x%x\n",    (uint)pHdr->u32LayerDataAddr);
    DEBUG_DEMURA("LayerDataOriSize = 0x%x\n",    (uint)pHdr->u32LayerDataOriSize);
    DEBUG_DEMURA("LayerDataFlashSize = 0x%x\n",  (uint)pHdr->u32LayerDataFlashSize);
    DEBUG_DEMURA("RegDataAddr      = 0x%x\n",    (uint)pHdr->u32RegDataAddr);
    DEBUG_DEMURA("RegDataCount     = 0x%x\n",    (uint)pHdr->u32RegDataCount);

    DEBUG_DEMURA("DLLVersion  = 0x%x\n",         (uint)pHdr->u16DLLVersion);
    DEBUG_DEMURA("ProjectId   = 0x%x\n",         (uint)pHdr->u32ProjectId);
    DEBUG_DEMURA("CustomerDataAddr = 0x%x\n",    (uint)pHdr->u32CustomerDataAddr);
    DEBUG_DEMURA("CustomerDataSize = 0x%x\n",    (uint)pHdr->u32CustomerDataSize);
    #if (DBG_DEMURA == 1)
    DEBUG_DEMURA("Demura Bin Build date : ");
    pdate = (char *)&(pHdr->nDate);
    if (dbgLevel & EN_DEBUG_LEVEL_DEBUG)
        printf("20%02x/%02x/%02x %02x:00\n", (uint)pdate[0], (uint)pdate[1], (uint)pdate[2], (uint)pdate[3]);
    #endif

    DEBUG_DEMURA("nHBlockSize = 0x%x\n", (uint) pHdr->nHBlockSize);
    DEBUG_DEMURA("nVBlockSize = 0x%x\n", (uint)pHdr->nVBlockSize);
    DEBUG_DEMURA("nPacketSize = 0x%x\n", (uint)pHdr->nPacketSize);
#endif
    DEBUG_DEMURA("OK\n\n");
}


MS_BOOL MDrv_DEMURA_HandleData(DeMuraBinHeader* pHdr, MS_U8 *dst_start, MS_U8 *src_addr)
{
    MS_U8  *src, *dst;
    MS_U32 dstlen;
    unsigned long lenp = (~0UL);
    MS_U32 ret, org_len;
    MS_U32 u32CalCRC, pad_num;
    DeMuraBinHeader *pNewHdr;

    DEBUG_DEMURA("IN\n");

    // For STR resuming
    //   Copy the header
    dst = dst_start;
    pNewHdr = (DeMuraBinHeader *)dst;
    memcpy(dst, src_addr, sizeof(DeMuraBinHeader));
    dst = dst + sizeof(DeMuraBinHeader);
    pad_num = ALIGN((size_t)dst, DEMURA_DMA_ADDR_UNIT) - (size_t)dst;
    if (pad_num != 0)
        memset(dst, 0, pad_num);
    dst = dst + pad_num;

    // Load layer data
    pNewHdr->u8LayerDataFomrat = E_DEMURA_UNCOMPRESS_FORMAT;
    pNewHdr->u32LayerDataAddr  = (size_t)dst - (size_t)dst_start;

    if (pHdr->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        MS_U32 distance = (dst_start >= src_addr) ? (dst_start - src_addr) : (src_addr - dst_start);

        if (distance < pHdr->u32AllBinSize)
        {
            DEBUG_DEMURA("For Compress data, dest(0x%lx) - src(0x%lx) < AllBinSize(0x%lx)\n",
                           (unsigned long)dst_start, (unsigned long)src_addr, (unsigned long)pHdr->u32AllBinSize);
            return FALSE;
        }

        dstlen = pHdr->u32LayerDataFlashSize - 6;    // 4byte  original filesize, 2byte magic value('0xBE', 0x'EF')
        src  = (MS_U8 *)(src_addr + pHdr->u32LayerDataAddr);
        org_len = *(MS_U32 *)(src + dstlen);

        DEBUG_DEMURA("dst = 0x%p, dstlen = 0x%x, src = 0x%p\n", dst, dstlen, src);
        ret = zunzip((void *)dst, dstlen, src, &lenp, 1, 0);
        if (ret != 0)
        {
            DEBUG_DEMURA("zunzip failed!\n");
            return FALSE;
        }
        if ((org_len != lenp) || (lenp != pHdr->u32LayerDataOriSize))
        {
            DEBUG_DEMURA("origal_len = 0x%lx, lenp = 0x%lx\n", (unsigned long)org_len, (unsigned long)lenp);
            DEBUG_DEMURA("zunzip data abort\n");
            return FALSE;
        }
        DEBUG_DEMURA("zunzip done, lenp = 0x%lx\n", (unsigned long)lenp);
    }
    else if (pHdr->u8LayerDataFomrat == E_DEMURA_UNCOMPRESS_FORMAT)
    {
        memcpy(dst, (src_addr + pHdr->u32LayerDataAddr), pHdr->u32LayerDataOriSize);
    }
    dst = dst +  pHdr->u32LayerDataOriSize;
    pad_num = ALIGN((size_t)dst, DEMURA_DMA_ADDR_UNIT) - (size_t)dst;
    if (pad_num != 0)
        memset(dst, 0, pad_num);
    dst = dst + pad_num;

    //   Copy the register data
    memcpy(dst, (src_addr + pHdr->u32RegDataAddr), (pHdr->u32RegDataCount * 5));
    pNewHdr->u32RegDataAddr = (MS_U32)(dst - dst_start);
    dst = dst + pHdr->u32RegDataCount * 5;
    pad_num = ALIGN((size_t)dst, DEMURA_DMA_ADDR_UNIT) - (size_t)dst;
    if (pad_num != 0)
        memset(dst, 0, pad_num);
    dst = dst + pad_num;

    //   Copy the Customer data
    if (pHdr->u32CustomerDataAddr != 0x00)
    {
        memcpy(dst, (src_addr + pHdr->u32CustomerDataAddr), pHdr->u32CustomerDataSize);
        pNewHdr->u32CustomerDataAddr  = (MS_U32)(dst - dst_start);
        dst = dst + pHdr->u32CustomerDataSize;
    }
    pNewHdr->u32AllBinSize = (MS_U32)(dst - dst_start);

    //   Re generate the CRC of header
    u32CalCRC = MDrv_CRC32_Cal_DeMura((MS_U8*)pNewHdr + 8, sizeof(DeMuraBinHeader)-8);
    pNewHdr->u32HeaderCRC = u32CalCRC;
    memcpy((void *)pHdr, (void *)pNewHdr, sizeof(DeMuraBinHeader));  // Copy back the New Header

    MDrv_DEMURA_Dump_BinInfo((MS_U8 *)pHdr);
    DEBUG_DEMURA("OK\n");
    return TRUE;
}

static void _MDrv_PQReg_Demura_coeff_and_ks(int diff, int *coeff, int *ks)
{
    double target = (double)(1.0/(double)diff);

    int inv_20 = (int)((double)(1<<20)/(double)diff + 0.5);
    int inv_22 = (int)((double)(1<<22)/(double)diff + 0.5);

    inv_20 = minmax_(inv_20, 0x0, 0x3FFF);
    inv_22 = minmax_(inv_22, 0x0, 0x3FFF);

    double inv_20_value = (double)((double)inv_20/(double)(1<<20));
    double inv_22_value = (double)((double)inv_22/(double)(1<<22));

    double target1 = (target > inv_20_value) ? (target - inv_20_value) : (inv_20_value - target);
    double target2 = (target > inv_22_value) ? (target - inv_22_value) : (inv_22_value - target);

    if(target1<target2)
    {
        *ks = 0;
        *coeff = inv_20;
    }
    else
    {
        *ks = 1;
        *coeff = inv_22;
    }
}

MS_BOOL MDrv_DEMURA_RegisterTableLoad(MS_U8* pHdrBuf, MS_U32 RegDataBuf)
{
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    int i, plane_max_idx;
    int diff[9] = {0};
    int coeff[9], ks[9];
    ST_DEMURA_UFC_HEADER *pHdr = (ST_DEMURA_UFC_HEADER *)pHdrBuf;
    //MS_U32 lut_size;
    MS_U16 h_size, v_size;
    MS_U8  gain_r[UFC_HEADER_GAIN_NUM];
    MS_U8  gain_g[UFC_HEADER_GAIN_NUM];
    MS_U8  gain_b[UFC_HEADER_GAIN_NUM];
    MS_U16 offset_r[UFC_HEADER_OFFSET_NUM];
    MS_U16 offset_g[UFC_HEADER_OFFSET_NUM];
    MS_U16 offset_b[UFC_HEADER_OFFSET_NUM];
    MS_U16 black_limit, white_limit;
    MS_U16 plane_level[UFC_HEADER_PLANE_NUM];

    //lut_size    = UFC_HEADER_GET_LUT_SIZE(pHdr);
    h_size      = UFC_HEADER_GET_H_SIZE(pHdr);
    v_size      = UFC_HEADER_GET_V_SIZE(pHdr);

    for (i = 0; i < UFC_HEADER_GAIN_NUM; i++)
    {
        gain_r[i] = pHdr->gain_r[i];
        gain_g[i] = pHdr->gain_g[i];
        gain_b[i] = pHdr->gain_b[i];
        offset_r[i] = UFC_HEADER_GET_R_OFFSET(pHdr, i);
        offset_g[i] = UFC_HEADER_GET_G_OFFSET(pHdr, i);
        offset_b[i] = UFC_HEADER_GET_B_OFFSET(pHdr, i);
        plane_level[i] = UFC_HEADER_GET_PLANE(pHdr, i);
    }
    black_limit = UFC_HEADER_GET_BLACKLIMIT(pHdr);
    white_limit = UFC_HEADER_GET_WHITELIMIT(pHdr);

    HAL_DEMURA_Write2ByteMask(REG_00A8_DEMURA_BKA377, pHdr->h_blk_size,
        REG_00A8_DEMURA_BKA377_REG_DMC_H_BLOCK);
    HAL_DEMURA_Write2ByteMask(REG_00A8_DEMURA_BKA377, pHdr->v_blk_size,
        REG_00A8_DEMURA_BKA377_REG_DMC_V_BLOCK);

#if 0 //remove
    HAL_DEMURA_Write2ByteMask(REG_00A8_DEMURA_BKA377, blk_size,
        REG_00A8_DEMURA_BKA377_REG_DMC_BLOCK_SIZE);
#endif

    HAL_DEMURA_Write2ByteMask(REG_00A8_DEMURA_BKA377, pHdr->plane_num,
        REG_00A8_DEMURA_BKA377_REG_DMC_PLANE_NUM);
    HAL_DEMURA_Write2ByteMask(REG_00A8_DEMURA_BKA377, pHdr->mode,
        REG_00A8_DEMURA_BKA377_REG_DMC_RGB_MODE);

    // H/V LUT num : H_size/H_block + 1
    h_size = demura_get_h_nodes(h_size, pHdr->h_blk_size, pHdr->plane_num, pHdr->mode);
    HAL_DEMURA_Write2ByteMask(REG_00AC_DEMURA_BKA377,
        (u16)h_size,
        REG_00AC_DEMURA_BKA377_REG_DMC_H_LUT_NUM);

    HAL_DEMURA_Write2ByteMask(REG_00B0_DEMURA_BKA377,
        (u16)(v_size >> pHdr->v_blk_size) + 1,
        REG_00B0_DEMURA_BKA377_REG_DMC_V_LUT_NUM);

    // black & white
    HAL_DEMURA_Write2ByteMask(REG_0040_DEMURA_BKA377, black_limit,
        REG_0040_DEMURA_BKA377_REG_DMC_BLACK_LIMIT);
    HAL_DEMURA_Write2ByteMask(REG_0064_DEMURA_BKA377, white_limit,
        REG_0064_DEMURA_BKA377_REG_DMC_WHITE_LIMIT);

    // plane limit @12bit
    HAL_DEMURA_Write2ByteMask(REG_0044_DEMURA_BKA377, plane_level[0],
        REG_0044_DEMURA_BKA377_REG_DMC_PLANE_LEVEL1);
    HAL_DEMURA_Write2ByteMask(REG_0048_DEMURA_BKA377, plane_level[1],
        REG_0048_DEMURA_BKA377_REG_DMC_PLANE_LEVEL2);
    HAL_DEMURA_Write2ByteMask(REG_004C_DEMURA_BKA377, plane_level[2],
        REG_004C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL3);
    HAL_DEMURA_Write2ByteMask(REG_0050_DEMURA_BKA377, plane_level[3],
        REG_0050_DEMURA_BKA377_REG_DMC_PLANE_LEVEL4);
    HAL_DEMURA_Write2ByteMask(REG_0054_DEMURA_BKA377, plane_level[4],
        REG_0054_DEMURA_BKA377_REG_DMC_PLANE_LEVEL5);
    HAL_DEMURA_Write2ByteMask(REG_0058_DEMURA_BKA377, plane_level[5],
        REG_0058_DEMURA_BKA377_REG_DMC_PLANE_LEVEL6);
    HAL_DEMURA_Write2ByteMask(REG_005C_DEMURA_BKA377, plane_level[6],
        REG_005C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL7);
    HAL_DEMURA_Write2ByteMask(REG_0060_DEMURA_BKA377, plane_level[7],
        REG_0060_DEMURA_BKA377_REG_DMC_PLANE_LEVEL8);


    // Gain and offset
    HAL_DEMURA_Write2ByteMask(REG_0068_DEMURA_BKA377, gain_r[0],
        REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG1);
    HAL_DEMURA_Write2ByteMask(REG_0068_DEMURA_BKA377, gain_r[1],
        REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG2);
    HAL_DEMURA_Write2ByteMask(REG_0068_DEMURA_BKA377, gain_r[2],
        REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG3);
    HAL_DEMURA_Write2ByteMask(REG_0068_DEMURA_BKA377, gain_r[3],
        REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG4);
    HAL_DEMURA_Write2ByteMask(REG_006C_DEMURA_BKA377, gain_r[4],
        REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG5);
    HAL_DEMURA_Write2ByteMask(REG_006C_DEMURA_BKA377, gain_r[5],
        REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG6);
    HAL_DEMURA_Write2ByteMask(REG_006C_DEMURA_BKA377, gain_r[6],
        REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG7);
    HAL_DEMURA_Write2ByteMask(REG_006C_DEMURA_BKA377, gain_r[7],
        REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG8);


    HAL_DEMURA_Write2ByteMask(REG_0070_DEMURA_BKA377, gain_g[0],
        REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG1);
    HAL_DEMURA_Write2ByteMask(REG_0070_DEMURA_BKA377, gain_g[1],
        REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG2);
    HAL_DEMURA_Write2ByteMask(REG_0070_DEMURA_BKA377, gain_g[2],
        REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG3);
    HAL_DEMURA_Write2ByteMask(REG_0070_DEMURA_BKA377, gain_g[3],
        REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG4);
    HAL_DEMURA_Write2ByteMask(REG_0074_DEMURA_BKA377, gain_g[4],
        REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG5);
    HAL_DEMURA_Write2ByteMask(REG_0074_DEMURA_BKA377, gain_g[5],
        REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG6);
    HAL_DEMURA_Write2ByteMask(REG_0074_DEMURA_BKA377, gain_g[6],
        REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG7);
    HAL_DEMURA_Write2ByteMask(REG_0074_DEMURA_BKA377, gain_g[7],
        REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG8);

    HAL_DEMURA_Write2ByteMask(REG_0078_DEMURA_BKA377, gain_b[0],
        REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG1);
    HAL_DEMURA_Write2ByteMask(REG_0078_DEMURA_BKA377, gain_b[1],
        REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG2);
    HAL_DEMURA_Write2ByteMask(REG_0078_DEMURA_BKA377, gain_b[2],
        REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG3);
    HAL_DEMURA_Write2ByteMask(REG_0078_DEMURA_BKA377, gain_b[3],
        REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG4);
    HAL_DEMURA_Write2ByteMask(REG_007C_DEMURA_BKA377, gain_b[4],
        REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG5);
    HAL_DEMURA_Write2ByteMask(REG_007C_DEMURA_BKA377, gain_b[5],
        REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG6);
    HAL_DEMURA_Write2ByteMask(REG_007C_DEMURA_BKA377, gain_b[6],
        REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG7);
    HAL_DEMURA_Write2ByteMask(REG_007C_DEMURA_BKA377, gain_b[7],
        REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG8);

    HAL_DEMURA_Write2ByteMask(REG_00C0_DEMURA_BKA377, offset_r[0],
        REG_00C0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET1);
    HAL_DEMURA_Write2ByteMask(REG_00C4_DEMURA_BKA377, offset_r[1],
        REG_00C4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET2);
    HAL_DEMURA_Write2ByteMask(REG_00C8_DEMURA_BKA377, offset_r[2],
        REG_00C8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET3);
    HAL_DEMURA_Write2ByteMask(REG_00CC_DEMURA_BKA377, offset_r[3],
        REG_00CC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET4);
    HAL_DEMURA_Write2ByteMask(REG_00D0_DEMURA_BKA377, offset_r[4],
        REG_00D0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET5);
    HAL_DEMURA_Write2ByteMask(REG_00D4_DEMURA_BKA377, offset_r[5],
        REG_00D4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET6);
    HAL_DEMURA_Write2ByteMask(REG_00D8_DEMURA_BKA377, offset_r[6],
        REG_00D8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET7);
    HAL_DEMURA_Write2ByteMask(REG_00DC_DEMURA_BKA377, offset_r[7],
        REG_00DC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET8);

    HAL_DEMURA_Write2ByteMask(REG_00E0_DEMURA_BKA377, offset_g[0],
        REG_00E0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET1);
    HAL_DEMURA_Write2ByteMask(REG_00E4_DEMURA_BKA377, offset_g[1],
        REG_00E4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET2);
    HAL_DEMURA_Write2ByteMask(REG_00E8_DEMURA_BKA377, offset_g[2],
        REG_00E8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET3);
    HAL_DEMURA_Write2ByteMask(REG_00EC_DEMURA_BKA377, offset_g[3],
        REG_00EC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET4);
    HAL_DEMURA_Write2ByteMask(REG_00F0_DEMURA_BKA377, offset_g[4],
        REG_00F0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET5);
    HAL_DEMURA_Write2ByteMask(REG_00F4_DEMURA_BKA377, offset_g[5],
        REG_00F4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET6);
    HAL_DEMURA_Write2ByteMask(REG_00F8_DEMURA_BKA377, offset_g[6],
        REG_00F8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET7);
    HAL_DEMURA_Write2ByteMask(REG_00FC_DEMURA_BKA377, offset_g[7],
        REG_00FC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET8);

    HAL_DEMURA_Write2ByteMask(REG_0100_DEMURA_BKA377, offset_b[0],
        REG_0100_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET1);
    HAL_DEMURA_Write2ByteMask(REG_0104_DEMURA_BKA377, offset_b[1],
        REG_0104_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET2);
    HAL_DEMURA_Write2ByteMask(REG_0108_DEMURA_BKA377, offset_b[2],
        REG_0108_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET3);
    HAL_DEMURA_Write2ByteMask(REG_010C_DEMURA_BKA377, offset_b[3],
        REG_010C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET4);
    HAL_DEMURA_Write2ByteMask(REG_0110_DEMURA_BKA377, offset_b[4],
        REG_0110_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET5);
    HAL_DEMURA_Write2ByteMask(REG_0114_DEMURA_BKA377, offset_b[5],
        REG_0114_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET6);
    HAL_DEMURA_Write2ByteMask(REG_0118_DEMURA_BKA377, offset_b[6],
        REG_0118_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET7);
    HAL_DEMURA_Write2ByteMask(REG_011C_DEMURA_BKA377, offset_b[7],
        REG_011C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET8);

    // layer distance is 14 bit , input layer is 12 bit , so shift 2 bit keep 14 bit
    plane_max_idx = pHdr->plane_num - 1;
    diff[0] = MAX((plane_level[0] - black_limit) << 2, 1);
    _MDrv_PQReg_Demura_coeff_and_ks(diff[0], &coeff[0], &ks[0]);
    for ( i = 1; i < UFC_HEADER_PLANE_NUM; i++)
    {
        if (plane_max_idx >= i)
        {
            diff[i] = MAX((plane_level[i] - plane_level[(i - 1)]) << 2, 1);
        }
        else
        {
            diff[i] = MAX((white_limit - plane_level[plane_max_idx]) << 2, 1);
        }
        _MDrv_PQReg_Demura_coeff_and_ks(diff[i], &coeff[i], &ks[i]);
    }
    diff[8] = MAX((white_limit - plane_level[plane_max_idx]) << 2, 1);
    _MDrv_PQReg_Demura_coeff_and_ks(diff[8], &coeff[8], &ks[8]);

    HAL_DEMURA_Write2ByteMask(REG_0080_DEMURA_BKA377, coeff[0],
        REG_0080_DEMURA_BKA377_REG_DMC_PLANE_B1_COEF);
    HAL_DEMURA_Write2ByteMask(REG_0084_DEMURA_BKA377, coeff[1],
        REG_0084_DEMURA_BKA377_REG_DMC_PLANE_12_COEF);
    HAL_DEMURA_Write2ByteMask(REG_0088_DEMURA_BKA377, coeff[2],
        REG_0088_DEMURA_BKA377_REG_DMC_PLANE_23_COEF);
    HAL_DEMURA_Write2ByteMask(REG_008C_DEMURA_BKA377, coeff[3],
        REG_008C_DEMURA_BKA377_REG_DMC_PLANE_34_COEF);
    HAL_DEMURA_Write2ByteMask(REG_0090_DEMURA_BKA377, coeff[4],
        REG_0090_DEMURA_BKA377_REG_DMC_PLANE_45_COEF);
    HAL_DEMURA_Write2ByteMask(REG_0094_DEMURA_BKA377, coeff[5],
        REG_0094_DEMURA_BKA377_REG_DMC_PLANE_56_COEF);
    HAL_DEMURA_Write2ByteMask(REG_0098_DEMURA_BKA377, coeff[6],
        REG_0098_DEMURA_BKA377_REG_DMC_PLANE_67_COEF);
    HAL_DEMURA_Write2ByteMask(REG_009C_DEMURA_BKA377, coeff[7],
        REG_009C_DEMURA_BKA377_REG_DMC_PLANE_78_COEF);
    HAL_DEMURA_Write2ByteMask(REG_00A0_DEMURA_BKA377, coeff[8],
        REG_00A0_DEMURA_BKA377_REG_DMC_PLANE_9W_COEF);

    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[0],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_B1_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[1],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_12_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[2],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_23_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[3],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_34_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[4],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_45_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[5],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_56_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[6],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_67_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[7],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_78_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[8],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_89_KS22);
    HAL_DEMURA_Write2ByteMask(REG_00A4_DEMURA_BKA377, ks[8],
        REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_9W_KS22);

    // flow control
    HAL_DEMURA_Write2ByteMask(REG_0130_DEMURA_BKA377, REG_0130_DEMURA_BKA377_REG_DMC_FLOW_CTRL_EN,
        REG_0130_DEMURA_BKA377_REG_DMC_FLOW_CTRL_EN);
    HAL_DEMURA_Write2ByteMask(REG_0130_DEMURA_BKA377, h_size,
        REG_0130_DEMURA_BKA377_REG_DMC_FLOW_LEN);

    // 77_2f[3] = 0 : turn off db or fmt_convert
    HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, 0,
        Fld(1,3,AC_MSKB0));

    // 77_2f[5:4] = 1 : enable dither
    HAL_DEMURA_Write2ByteMask(REG_00BC_DEMURA_BKA377, pHdr->dither_en,
        REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_EN);
#else
    DeMuraBinHeader* pHdr = (DeMuraBinHeader *) pHdrBuf;
    MS_U32 u32Counter;
    MS_U32 u32RegisterAddress;
    MS_U8  u8Value;
    MS_U8  u8Mask;
#if (!CONFIG_TARGET_MT5896_DEMURA) //bank*0x200+addr*4
    MS_U32 base_addr = 0;
#endif
    MS_U8 *pu8RegDataBuf = (MS_U8 *)((uintptr_t)RegDataBuf);
    DEBUG_DEMURA("IN\n");
    if((pHdr->u32RegDataCount == 0) || (pHdr->u32RegDataCount > 256))
    {
        DEBUG_DEMURA("Error: Demura Register Counter = %d, No register value needs to be loaded.\n", (int)pHdr->u32RegDataCount);
        return FALSE;
    }

    //printf("[%s:%d]Register Count = %d  %p\n", __FUNCTION__, __LINE__, (int)pHdr->u32RegDataCount, pu8RegDataBuf);
//    UBOOT_DUMP((unsigned int)pu8RegDataBuf, (pHdr->u32RegDataCount * 5));
    for (u32Counter = 0; u32Counter < pHdr->u32RegDataCount; u32Counter++)
    {
        // Littele Endian
#if CONFIG_TARGET_MT5896_DEMURA //bank*0x200+addr*4
        u32RegisterAddress = (((MS_U32)pu8RegDataBuf[2] << 16)
            |((MS_U32)pu8RegDataBuf[1] << 8) | pu8RegDataBuf[0]) + DEMURA_BKA377;
        u8Value = (pu8RegDataBuf[3]);
        u8Mask = (pu8RegDataBuf[4]);

#if 0
        printf("%s@%d [%u] 0x%x = 0x%x & 0x%x u32RegisterAddress=%x\n",
            __FUNCTION__, __LINE__, u32Counter, u32RegisterAddress, u8Value, u8Mask, u32RegisterAddress);
#endif
        HAL_DEMURA_WriteByteMask(u32RegisterAddress , (MS_U16)u8Value, (MS_U16)u8Mask);
#else
        u32RegisterAddress = (((MS_U32)pu8RegDataBuf[2] << 16)
            |((MS_U32)pu8RegDataBuf[1] << 8) | pu8RegDataBuf[0]) + DEMURA_BKA377;

        u8Value = (pu8RegDataBuf[3]);
        u8Mask = (pu8RegDataBuf[4]);

        HAL_DEMURA_WriteByteMask(u32RegisterAddress, u8Value, u8Mask);
#endif
        pu8RegDataBuf += 5;
    }
    //HAL_DEMURA_Dump_Layerlevel();
#endif
    DEBUG_DEMURA("OK\n");
    return TRUE;
}


void MDrv_DEMURA_AutoDownload_Setup(MS_U8* pHdr, MS_U32 pLayerDataAddr, MS_BOOL bEnable)
{
    DEBUG_DEMURA("IN\n");

    if (bEnable == TRUE)
    {
        HAL_DEMURA_SetDL_BaseAddr(pLayerDataAddr);
        HAL_DEMURA_SetDL_SramIni_Addr(0);

#if 0 // useless, it's set by HW.
        HAL_DEMURA_SetDL_Depth(pHdr->u16HNodeCount);
#endif
        HAL_DEMURA_SetDL_ReqLenth(DEMURA_DMA_REQ_LENGTH);
        HAL_DEMURA_SetDL_TriggerMode(AUTO_DL_ENABLE_MODE);
        HAL_DEMURA_Enable_DL(TRUE);
    }
    else
    {
        HAL_DEMURA_SetDL_TriggerMode(AUTO_DL_TRIG_MODE);
        HAL_DEMURA_Enable_DL(FALSE);
    }
    DEBUG_DEMURA("OK\n");
}

MS_U32 MDrv_DEMURA_DMA_Addr_Unit(void)
{
    return DEMURA_DMA_ADDR_UNIT;
}

void MDrv_DEMURA_TurnOn(MS_BOOL bOnOff)
{
    DEMURA_PANEL_TYPE ptype;
    DEBUG_DEMURA("IN\n");

    ptype = HAL_DEMURA_Get_PanelType();
    if (ptype == E_DEMURA_RGBW_PANEL)
    {
        UBOOT_INFO("Detected RGBW Panel\n");
    }

    HAL_DEMURA_EnableDemura(bOnOff, ptype);
    DEBUG_DEMURA("OK\n");
}

void MDrv_DEMURA_BYPASS(MS_BOOL bOnOff)
{
#if defined(CONFIG_DEMURA_VENDOR_MULTI)
#else
    if (MDrv_DEMURA_SetIOMapBase() == FALSE)
    {
        DEBUG_DEMURA("MDrv_DEMURA_SetIOMapBase init fail\n");
    }
#endif
    HAL_DEMURA_EnableDemuraBypass(bOnOff);
}
