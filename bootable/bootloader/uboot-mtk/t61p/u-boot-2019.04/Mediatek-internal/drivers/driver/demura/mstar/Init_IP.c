/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#include <common.h>
#include <MsTypes.h>
#include <apiPNL.h>
#include <malloc.h>
////#include <ShareType.h>
////#include <MsStr.h>
#include <debug_impl.h>
//#include <MsSystem.h>
#include <drvMIU.h>
#include <MDrvDemura.h>
#include <ms_utils.h>
#include <dmalloc.h>
#include <demura.h>
#include <CommonDataType.h>
#include "convert_entry.h"
#include "apiDemura.h"
#include "demura_config.h"

#undef ALIGN
#define ALIGN(x,a)        __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

static DeMuraBinHeader *pHeader = NULL;

MS_BOOL __attribute__((weak)) If_Need_Decode(void)
{
    return FALSE;
}

MS_BOOL __attribute__((weak)) Check_MuraTable(void)
{
    return TRUE;
}


static MS_BOOL Alloc_Load_Buf(MS_U8 *pData, MS_U8 **bin_buf, MS_U8 **tbuf_addr, MS_U32 id_num)
{

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    MS_U32  filesize;
    ST_DEMURA_UFC_HEADER *pHdr = (ST_DEMURA_UFC_HEADER *)pData;
    filesize = UFC_HEADER_GET_LUT_SIZE(pHdr) + UFC_HEADER_GET_HEADER_SIZE(pHdr);
    *bin_buf = (MS_U8 *)(size_t)add_dbin_buf(DEMURA_MSTAR_IP, filesize, id_num);
    if (*bin_buf == (void *)0)
    {
        UBOOT_ERROR("Get_Dbin_Buf Error!\n");
        return FALSE;
    }

#else
    MS_U32  filesize;
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    filesize   =  pHdr->u32AllBinSize;
    filesize  -=  pHdr->u32LayerDataFlashSize;
    filesize  +=  pHdr->u32LayerDataOriSize;
    *bin_buf = (MS_U8 *)(size_t)add_dbin_buf(DEMURA_MSTAR_IP, filesize, id_num);
    if (*bin_buf == (void *)0)
    {
        UBOOT_ERROR("Get_Dbin_Buf Error!\n");
        return FALSE;
    }

    if (pHdr->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        MS_U32 unzip_len = get_dbuf_length();
        *tbuf_addr = (MS_U8 *)dmalloc(unzip_len);
        CHECK_DMALLOC_SPACE(*tbuf_addr, (uint)unzip_len);
    }
#endif
    return TRUE;
}


static void Free_Load_Buf(MS_U8 *pData, MS_U8 **bin_buf, MS_U8 **tbuf_addr)
{
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    pop_demura_bin();
    *bin_buf   = NULL;
#else
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    if (pHdr->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
       dfree(tbuf_addr);
       //*tbuf_addr = NULL;
    }
    pop_demura_bin();
    *bin_buf   = NULL;
#endif
}

MS_BOOL MsDemura_LoadHeader(MS_U8 *pHdr, Demura_Panel_Data panel_data)
{
    MS_BOOL bRet = FALSE;
    UBOOT_TRACE("IN\n");

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    UBOOT_TRACE("Read mtk-ufc bin\n");
    bRet = read_raw_data(pHdr, 0, sizeof(ST_DEMURA_UFC_HEADER));
#else
    UBOOT_TRACE("Read mtk-demura bin\n");
    bRet = read_raw_data(pHdr, 0, sizeof(DeMuraBinHeader));
#endif
    if (bRet != TRUE)
    {
        UBOOT_ERROR("raw read Demura Header Failed\n");
        return FALSE;
    }

    MDrv_DEMURA_Dump_BinInfo(pHdr);
    UBOOT_TRACE("resolution : %d * %d\n", panel_data.u16PanelWidth, panel_data.u16PanelHeight);
    if (MDrv_DEMURA_Check_HeaderCRC(pHdr) == FALSE)
    {
        UBOOT_ERROR("Calculate Demura Bin header CRC fail\n");
        return FALSE;
    }

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    if (UFC_HEADER_GET_VERSION((ST_DEMURA_UFC_HEADER *)pHdr) != UFC_HEADER_VERSION)
    {
        UBOOT_ERROR("Check Demura Converter Version fail\n");
        return FALSE;
    }
#endif

    if (MDrv_DEMURA_Is_Support(pHdr, panel_data) == FALSE)
    {
        UBOOT_ERROR("This Demura Bin do Not Support this panel\n");
        return FALSE;
    }

    UBOOT_TRACE("OK\n");
    return TRUE;
}


MS_BOOL MsDemura_LoadBin(MS_U8 *pData, MS_U8 *mmap_buf, MS_U8 *unzip_buf)
{
    MS_U8 *buf = NULL;
    MS_U32 u32fileSize;
    MS_BOOL bRet = FALSE;
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    ST_DEMURA_UFC_HEADER *pHdr = (ST_DEMURA_UFC_HEADER *)pData;
    u32fileSize = UFC_HEADER_GET_LUT_SIZE(pHdr) + UFC_HEADER_GET_HEADER_SIZE(pHdr); 
    buf = mmap_buf;
    UBOOT_TRACE("Load data on mmap_buffer : %p\n", mmap_buf);
#else
    MS_U32 data_align;
    MS_BOOL bAdjust = TRUE;
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    UBOOT_TRACE("IN\n");

    u32fileSize = pHdr->u32AllBinSize;
    data_align  = MDrv_DEMURA_DMA_Addr_Unit();

    if (pHdr->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        if (unzip_buf == NULL)
        {
            UBOOT_ERROR("Can not unzip layer data, for unzip_buf = NULL\n");
            return FALSE;
        }
        buf = unzip_buf;
        bAdjust = TRUE;
        DEBUG_DEMURA("Select unzip buffer : %p\n", unzip_buf);
    }
    else if (pHdr->u8LayerDataFomrat == E_DEMURA_UNCOMPRESS_FORMAT)
    {
        int align_mask = data_align - 1;

        // Data unalign, memcpy can not be avoided.
        if ( ((pHdr->u32LayerDataAddr & align_mask) != 0)        \
             || ((pHdr->u32LayerDataOriSize & align_mask) != 0)  \
             || ((pHdr->u32RegDataAddr & align_mask) != 0)       \
             || ((pHdr->u32CustomerDataAddr & align_mask) != 0)  \
            )
        {
            buf = unzip_buf;
            bAdjust = TRUE;
            DEBUG_DEMURA("Load data on temp_buffer : %p\n", unzip_buf);
        }
        else
        {
            buf = mmap_buf;
            bAdjust = FALSE;
            DEBUG_DEMURA("Load data on mmap_buffer : %p\n", mmap_buf);
        }
    }
#endif
    if (buf == NULL)
    {
        UBOOT_ERROR("LoadBin from NULL buffer!\n");
        return FALSE;
    }

//#ifndef CONFIG_DEMURA_LGD_DEMO
#if(!CONFIG_DEMURA_WITHOUT_MMC_PART)
    bRet = read_raw_data(buf, 0, u32fileSize);
    if (bRet != TRUE)
    {
        UBOOT_ERROR("raw read Demura binary Failed\n");
        return FALSE;
    }
#endif
//#endif
    if (MDrv_DEMURA_Check_AllBinCRC(buf) == FALSE)
    {
        UBOOT_ERROR("Calculate All Demura Bin CRC fail\n");
        return FALSE;
    }

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
#else
    // Try to avoid unnecssary memcpy.
    if ((bAdjust == TRUE) && (MDrv_DEMURA_HandleData(pHdr, mmap_buf, unzip_buf) != TRUE))
    {
        UBOOT_ERROR("MDrv_DEMURA_HandleData Failed\n");
        return FALSE;
    }
#endif
    UBOOT_TRACE("OK\n");
    return TRUE;
}

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
static MS_U32 Convert_and_Reload(MS_U8 *pHdr, Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
#if (CONFIG_DEMURA_VENDOR_MSTAR == 0)
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    BinOutputInfo bin_info = {0};

    if(EN_DEMURA_MULTI_MTK == multi_vendor)
    {
        (void)pHdr;  // remove complier warning
        (void)panel_data;
        UBOOT_ERROR("Load Demura Binary Error!\n");
        return 0x00;
    }

    if (do_demura_convert(&bin_info, multi_vendor) != 0)
    {
        UBOOT_ERROR("Convert %s to Mstar Demura Failed!\n", CONFIG_DEMURA_VENDOR_STRING);
        return 0x00;
    }

    if(get_demura_env_bypass_write_fs())
    {
        #ifdef CONFIG_MTK_DEMURA_UFC_BIN
        memcpy(pHdr, bin_info.bin_buf, sizeof(ST_DEMURA_UFC_HEADER));  // Copy Demura Header;
        #else
        memcpy(pHdr, bin_info.bin_buf, sizeof(DeMuraBinHeader));  // Copy Demura Header;
        #endif

        if (MDrv_DEMURA_Check_HeaderCRC(pHdr) == FALSE)
        {
            UBOOT_ERROR("Calculate Demura Bin header CRC fail\n");
            dfree(bin_info.bin_buf);
            return 0x00;
        }
        if (MDrv_DEMURA_Is_Support(pHdr, panel_data) == FALSE)
        {
            UBOOT_ERROR("This Demura Bin do Not Support this panel\n");
            dfree(bin_info.bin_buf);
            return 0x00;
        }
        if (Alloc_Load_Buf(pHdr, &bin_buf, &tbuf_addr, 0) != TRUE)
        {
            UBOOT_ERROR("Alloc_Load_Buf Error!\n");
            dfree(bin_info.bin_buf);
            return 0x00;
        }

        #ifdef CONFIG_MTK_DEMURA_UFC_BIN
        memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
        #else
        if (((DeMuraBinHeader *)pHdr)->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
            memcpy(tbuf_addr, bin_info.bin_buf, bin_info.bin_size);
        else
            memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
        #endif

        if (MDrv_DEMURA_Check_AllBinCRC(bin_buf) == FALSE)
        {
            UBOOT_ERROR("Calculate All Demura Bin CRC fail\n");
            dfree(tbuf_addr);
            dfree(bin_info.bin_buf);
            return 0x00;
        }
        dfree(tbuf_addr);
        dfree(bin_info.bin_buf);
        return (MS_U32)((uintptr_t)bin_buf);
    }

    #if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
    #ifdef CONFIG_MTK_DEMURA_UFC_BIN
    memcpy(pHdr, bin_info.bin_buf, sizeof(ST_DEMURA_UFC_HEADER));  // Copy Demura Header;
    #else
    memcpy(pHdr, bin_info.bin_buf, sizeof(DeMuraBinHeader));  // Copy Demura Header;
    #endif
    #else

    if (write_raw_data(bin_info.bin_buf, 0, bin_info.bin_size) != TRUE)
    {
        UBOOT_ERROR("write_raw_data to Demura(Partition) Failed\n");
        return FALSE;
    }
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);

    if (MsDemura_LoadHeader(pHdr, panel_data) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadHeader Error Again!\n");
        dfree(bin_info.bin_buf);
        return 0x00;
    }
    #endif

    if (MsDemura_LoadBin(pHdr, bin_buf, tbuf_addr) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadBin Error Again!\n");
        Free_Load_Buf(pHdr, &bin_buf, &tbuf_addr);
        dfree(tbuf_addr);
        dfree(bin_info.bin_buf);
        return 0x00;
    }
    dfree(tbuf_addr);
    dfree(bin_info.bin_buf);
    return (MS_U32)((uintptr_t)bin_buf);
#else
    pHdr = pHdr;  // remove complier warning
    panel_data = panel_data;
    UBOOT_ERROR("Load Demura Binary Error!\n");
    return 0x00;
#endif
}

static MS_U32 MsDemura_AC_Init(MS_U8 *pHdr, Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    MS_BOOL ret       = FALSE;
    MS_BOOL decode_ret = TRUE;
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
#else
    DeMuraBinHeader *pHeadr = (DeMuraBinHeader *)pHdr;
#endif
    if(multi_vendor == EN_DEMURA_MULTI_INX)
    {
        if (Check_MuraTable_INX() != TRUE)
        {
            UBOOT_ERROR("Invalid INX MuraTable, Don't enable Demura Function !\n");
            return 0x00;
        }
    }

    #if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_DEBUG("CONFIG_DEMURA_WITHOUT_MMC_PART, do demura convert! \n");
    return Convert_and_Reload(pHdr, panel_data, multi_vendor);
    #else
    ret = MsDemura_LoadHeader(pHdr, panel_data);
    #endif

    if (ret == TRUE)
    {
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
#else
        if(((pHeadr->u32ProjectId & 0xff) != multi_vendor) && (EN_DEMURA_MULTI_MTK != multi_vendor))
       {
            UBOOT_ERROR("Demura Bin and panel is not matched\n");
            ret = FALSE;
       }
#endif
    }

    if (ret != TRUE)//(MsDemura_LoadHeader(pHdr, panel_data) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadHeader Error, do convert\n");
        return Convert_and_Reload(pHdr, panel_data, multi_vendor);
    }

    if (Alloc_Load_Buf(pHdr, &bin_buf, &tbuf_addr, 0) != TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

    if(multi_vendor == EN_DEMURA_MULTI_NOVA)
    {
        decode_ret = If_Need_Decode_NOVA(NOVA_Shift);
    }
    else if(multi_vendor == EN_DEMURA_MULTI_AUO)
    {
        decode_ret = If_Need_Decode_AUO();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_CSOT_HISILICON)
    {
        decode_ret = If_Need_Decode_CSOT_HISILICON();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_CSOT_HIMAX)
    {
        decode_ret = If_Need_Decode_CSOT_HIMAX();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_SDC)
    {
        decode_ret = If_Need_Decode_SDC();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_INX)
    {
        decode_ret = If_Need_Decode_INX();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_CSOT_CSOT)
    {
        decode_ret = If_Need_Decode_CSOT_CSOT();
    }
    else if(multi_vendor == EN_DEMURA_MULTI_HKC_NOVA)
    {
        decode_ret = If_Need_Decode_NOVA(HKC_NOVA_Shift);
    }
    else if(multi_vendor == EN_DEMURA_MULTI_H_K_C_NOVA_120HZ)
    {
        decode_ret = If_Need_Decode_NOVA(H_K_C_NOVA_Shift_120HZ);
    }
    else if(multi_vendor == EN_DEMURA_MULTI_MTK)
    {
        decode_ret = FALSE;
    }
    else if(multi_vendor == EN_DEMURA_MULTI_BOE_ESWIN)
    {
        decode_ret = If_Need_Decode_BOE_ESWIN();
    }

    if ((MsDemura_LoadBin(pHdr, bin_buf, tbuf_addr) != TRUE) || (decode_ret == TRUE))
    {
        Free_Load_Buf(pHdr, &bin_buf, &tbuf_addr);
        dfree(tbuf_addr);
        return Convert_and_Reload(pHdr, panel_data, multi_vendor);
    }

    dfree(tbuf_addr);
    return (MS_U32)((uintptr_t)bin_buf);
}

#else
static MS_U32 Convert_and_Reload(DeMuraBinHeader *pHdr, DemuraPanel_Data panel_data)
{
#if (CONFIG_DEMURA_VENDOR_MSTAR == 0)
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    BinOutputInfo bin_info = {0};

    if (do_demura_convert(&bin_info) != 0)
    {
        UBOOT_ERROR("Convert %s to Mstar Demura Failed!\n", CONFIG_DEMURA_VENDOR_STRING);
        return 0x00;
    }

#if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
    memcpy(pHdr, bin_info.bin_buf, sizeof(DeMuraBinHeader));  // Copy Demura Header;
#else
    if (write_raw_data(bin_info.bin_buf, 0, bin_info.bin_size) != TRUE)
    {
        UBOOT_ERROR("write_raw_data to Demura(Partition) Failed\n");
        return FALSE;
    }
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);

    if (MsDemura_LoadHeader(pHdr, panel_data) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadHeader Error Again!\n");
        return 0x00;
    }
#endif

    if (Alloc_Load_Buf(pHdr, &bin_buf, &tbuf_addr, 0) != TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

#if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    if (pHdr->u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        memcpy(tbuf_addr, bin_info.bin_buf, bin_info.bin_size);
    }
    else
    {
        memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
    }
#endif

    if (MsDemura_LoadBin(pHdr, bin_buf, tbuf_addr) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadBin Error Again!\n");
        Free_Load_Buf(pHdr, &bin_buf, &tbuf_addr);
        return 0x00;
    }
    dfree(tbuf_addr);
    return (MS_U32)((uintptr_t)bin_buf);
#else
    pHdr = pHdr;  // remove complier warning
    panel_data = panel_data;
    UBOOT_ERROR("Load Demura Binary Error!\n");
    return 0x00;
#endif
}

static MS_U32 MsDemura_AC_Init(DeMuraBinHeader *pHdr, Demura_Panel_Data panel_data)
{
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;

    // Init demura heap ==> should be the first one !
    if (init_demura_heap() != TRUE)
    {
        UBOOT_ERROR("init_demura_heap failed!\n");
        return 0x00;
    }

    if (Check_MuraTable() != TRUE)
    {
        UBOOT_ERROR("Invalid MuraTable, Don't enable Demura Function !\n");
        return 0x00;
    }

#if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_DEBUG("CONFIG_DEMURA_WITHOUT_MMC_PART, do demura convert! \n");
    return Convert_and_Reload(pHdr, panel_data);
#else
    if (MsDemura_LoadHeader(pHdr, panel_data) != TRUE)
    {
        UBOOT_ERROR("MsDemura_LoadHeader Error\n");
        return Convert_and_Reload(pHdr, panel_data);
    }
#endif

    if (Alloc_Load_Buf(pHdr, &bin_buf, &tbuf_addr, 0) != TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

    if ((MsDemura_LoadBin(pHdr, bin_buf, tbuf_addr) != TRUE) || (If_Need_Decode() == TRUE))
    {
        Free_Load_Buf(pHdr, &bin_buf, &tbuf_addr);
        return Convert_and_Reload(pHdr, panel_data);
    }

    dfree(tbuf_addr);
    return (MS_U32)bin_buf;
}
#endif

#ifdef CONFIG_DEMURA_LGD_DEMO

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
static MS_U32 LGDemo_Convert_And_Load(MS_BOOL bFCIC, MS_U32 id_num, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    BinOutputInfo bin_info = {0};
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    ST_DEMURA_UFC_HEADER Hdr;
#else
    DeMuraBinHeader Hdr;
#endif

    memset(&bin_info, 0, sizeof(bin_info));
    bin_info.Vendor_ARG1 = bFCIC;

    if (do_demura_convert(&bin_info, multi_vendor) != 0)
    {
        UBOOT_ERROR("Convert FCIC(%s) to Mstar-Demura Failed!\n", (bFCIC == TRUE ? "FCIC" : "simple-FCIC"));
        return 0x00;
    }

    #if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
    #else
    if (write_raw_data(bin_info.bin_buf, 0, bin_info.bin_size) != TRUE)
    {
        UBOOT_ERROR("write_raw_data to Demura(Partition) Failed\n");
        return FALSE;
    }
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
    #endif

    memcpy(&Hdr, bin_info.bin_buf, sizeof(Hdr));  // Copy Demura Header;

    if (Alloc_Load_Buf((MS_U8 *)&Hdr, &bin_buf, &tbuf_addr, id_num)!= TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
#else
    if (Hdr.u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        memcpy(tbuf_addr, bin_info.bin_buf, bin_info.bin_size);
    }
    else
    {
        memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
    }
#endif
    if (MsDemura_LoadBin((MS_U8*)&Hdr, bin_buf, tbuf_addr) != TRUE)
    {
        Free_Load_Buf((MS_U8 *)&Hdr, &bin_buf, &tbuf_addr);
        dfree(tbuf_addr);
        return 0x00;
    }
    dfree(tbuf_addr);

    return (MS_U32)((uintptr_t)bin_buf); // Default return the first mstar-demura binary.
}

MS_U32 SumString(const char *string)
{
    MS_U32 ret = 0;
    const char *pch  = string;

    while(*pch)
    {
        ret += *pch;
        pch++;
    }
    return ret;
}


MS_U32 MsDemura_Demo_AC_Init(MS_U8 *pData, Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_U32 bin_buf_lgd;
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    MS_BOOL ret = FALSE;

    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    // Init demura heap ==> should be the first one !
    if (init_demura_heap() != TRUE)
    {
        UBOOT_ERROR("init_demura_heap failed!\n");
        return 0x00;
    }

    #if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    ret = FALSE;
    #else
    ret = MsDemura_LoadHeader(pData, panel_data);
    #endif

    if((ret == TRUE) && ((pHdr->u32ProjectId & 0xff) != multi_vendor))
    {
        UBOOT_ERROR("Demura Bin and panel is not matched\n");
        ret = FALSE;
    }

    if(ret != TRUE)
    {
        bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, 0, multi_vendor);
        if (bin_buf_lgd == 0)
        {
            UBOOT_ERROR("Load Mstar-Demura-LGD Failed !\n");
        }
        else
        {
            switch(multi_vendor)
            {
                case EN_DEMURA_MULTI_LGD_V18:
                    printf("EN_DEMURA_MULTI_LGD_V18 OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_SQ18_19_COLOR:
                    printf("EN_DEMURA_MULTI_LGD_SQ18_19_COLOR OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_SQ18_19_MONO:
                    printf("EN_DEMURA_MULTI_LGD_SQ18_19_MONO OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_V19_COLOR:
                    printf("EN_DEMURA_MULTI_LGD_V19_COLOR OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_V19_MONO:
                    printf("EN_DEMURA_MULTI_LGD_V19_MONO OK \n");
                    break;
                default:
                    printf("should not be here %d\n", multi_vendor);
                    break;
            }

            memcpy(pHdr, (void *)((uintptr_t)bin_buf_lgd), sizeof(*pHdr));
        }

        return bin_buf_lgd;
    }

    if (Alloc_Load_Buf(pData, &bin_buf, &tbuf_addr, 0) != TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

    if ((MsDemura_LoadBin(pData, bin_buf, tbuf_addr) != TRUE) || (If_Need_Decode_LGD(multi_vendor) == TRUE))
    {
        Free_Load_Buf(pData, &bin_buf, &tbuf_addr);


        bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, 0, multi_vendor);
        if (bin_buf_lgd == 0)
        {
            UBOOT_ERROR("Load Mstar-Demura-LGD Failed !\n");
        }
        else
        {
            switch(multi_vendor)
            {
                case EN_DEMURA_MULTI_LGD_V18:
                    printf("EN_DEMURA_MULTI_LGD_V18 OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_SQ18_19_COLOR:
                    printf("EN_DEMURA_MULTI_LGD_SQ18_19_COLOR OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_SQ18_19_MONO:
                    printf("EN_DEMURA_MULTI_LGD_SQ18_19_MONO OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_V19_COLOR:
                    printf("EN_DEMURA_MULTI_LGD_V19_COLOR OK \n");
                    break;
                case EN_DEMURA_MULTI_LGD_V19_MONO:
                    printf("EN_DEMURA_MULTI_LGD_V19_MONO OK \n");
                    break;
                default:
                    printf("should not be here %d\n", multi_vendor);
                    break;
            }

            memcpy(pHdr, (void *)((uintptr_t)bin_buf_lgd), sizeof(*pHdr));
        }
        dfree(tbuf_addr);
        return (MS_U32)((uintptr_t)bin_buf_lgd);
    }

    dfree(tbuf_addr);
    return (MS_U32)((uintptr_t)bin_buf);
}

#else
static MS_U32 LGDemo_Convert_And_Load(MS_BOOL bFCIC, MS_U32 id_num)
{
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    BinOutputInfo bin_info = {0};
    DeMuraBinHeader Hdr;

    memset(&bin_info, 0, sizeof(bin_info));
    bin_info.Vendor_ARG1 = bFCIC;

    if (do_demura_convert(&bin_info) != 0)
    {
        UBOOT_ERROR("Convert FCIC(%s) to Mstar-Demura Failed!\n", (bFCIC == TRUE ? "FCIC" : "simple-FCIC"));
        return 0x00;
    }

#if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
#else
    if (write_raw_data(bin_info.bin_buf, 0, bin_info.bin_size) != TRUE)
    {
        UBOOT_ERROR("write_raw_data to Demura(Partition) Failed\n");
        return FALSE;
    }
    UBOOT_INFO("Convert %s to Mstar Demura Success!\n", CONFIG_DEMURA_VENDOR_STRING);
#endif

    memcpy(&Hdr, bin_info.bin_buf, sizeof(Hdr));  // Copy Demura Header;

    if (Alloc_Load_Buf(&Hdr, &bin_buf, &tbuf_addr, id_num)!= TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }
    if (Hdr.u8LayerDataFomrat == E_DEMURA_COMPRESS_FORMAT)
    {
        memcpy(tbuf_addr, bin_info.bin_buf, bin_info.bin_size);
    }
    else
    {
        memcpy(bin_buf,   bin_info.bin_buf, bin_info.bin_size);
    }

    if (MsDemura_LoadBin(&Hdr, bin_buf, tbuf_addr) != TRUE)
    {
        Free_Load_Buf(&Hdr, &bin_buf, &tbuf_addr);
        return 0x00;
    }
    dfree(tbuf_addr);

    return (MS_U32)bin_buf;  // Default return the first mstar-demura binary.
}

static MS_U32 SumString(const char *string)
{
    MS_U32 ret = 0;
    const char *pch  = string;

    while(*pch)
    {
        ret += *pch;
        pch++;
    }
    return ret;
}


MS_U32 MsDemura_Demo_AC_Init(MS_U8 *pData, Demura_Panel_Data panel_data)
{
    MS_U32 bin_buf_lgd;
    MS_U8  *bin_buf   = NULL;
    MS_U8  *tbuf_addr = NULL;
    MS_BOOL ret = FALSE;
    DeMuraBinHeader *pHdr = (DeMuraBinHeader *)pData;
    // Init demura heap ==> should be the first one !
    if (init_demura_heap() != TRUE)
    {
        UBOOT_ERROR("init_demura_heap failed!\n");
        return 0x00;
    }

#if(CONFIG_DEMURA_WITHOUT_MMC_PART)
    ret = FALSE;
#else
    ret = MsDemura_LoadHeader(pHdr, panel_data);
#endif

    if(ret != TRUE)
    {
        #if defined (CONFIG_DEMURA_VENDOR_LGD_V19)
            bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, SumString("V19-Simple-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:V19-Simple-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_V19 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_SQ18_19)
            bin_buf_lgd = LGDemo_Convert_And_Load(TRUE, SumString("Full-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:Full-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_SQ18_19 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_V18)
            bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, SumString("Simple-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:Simple-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_V18 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #endif
        return bin_buf_lgd;
    }

    if (Alloc_Load_Buf(pHdr, &bin_buf, &tbuf_addr, 0) != TRUE)
    {
        UBOOT_ERROR("Alloc_Load_Buf Error!\n");
        return 0x00;
    }

    if ((MsDemura_LoadBin(pHdr, bin_buf, tbuf_addr) != TRUE) || (If_Need_Decode() == TRUE))
    {
        Free_Load_Buf(pHdr, &bin_buf, &tbuf_addr);

        #if defined (CONFIG_DEMURA_VENDOR_LGD_V19)
            bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, SumString("V19-Simple-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:V19-Simple-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_V19 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_SQ18_19)
            bin_buf_lgd = LGDemo_Convert_And_Load(TRUE, SumString("Full-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:Full-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_SQ18_19 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #elif defined (CONFIG_DEMURA_VENDOR_LGD_V18)
            bin_buf_lgd = LGDemo_Convert_And_Load(FALSE, SumString("Simple-FCIC"));
            if (bin_buf_lgd == 0)
            {
                UBOOT_ERROR("Load Mstar-Demura(origin:Simple-FCIC) Failed !\n");
            }
            else
            {
                printf("CONFIG_DEMURA_VENDOR_LGD_V18 OK \n");
                memcpy(pHdr, (void *)bin_buf_lgd, sizeof(*pHdr));
            }
        #endif
        return bin_buf_lgd;
    }

    dfree(tbuf_addr);
    return (MS_U32)bin_buf;
}
#endif
#endif


static MS_U64 MsDemura_STR_Init(MS_U8 *pHdr)
{
    MS_U64  bus_addr;
    demura_bin_desc *pbin;

    pbin = get_demura_bin(DEMURA_MSTAR_IP, 0);
    if (pbin == NULL)
    {
        UBOOT_ERROR("On STR resuming, Can no find Mstar demura_bin_desc!\n");
        return 0x00;
    }
    #ifdef CONFIG_MBOOT
    bus_addr = pbin->phy_addr;
    #else
    bus_addr = pbin->virt_addr;
    #endif

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    memcpy(pHdr, (void *)(size_t)bus_addr, sizeof(ST_DEMURA_UFC_HEADER));
#else
    memcpy(pHdr, (void *)(size_t)bus_addr, sizeof(DeMuraBinHeader));
#endif
    if (MDrv_DEMURA_On_Resume(pHdr) == FALSE)
    {
        UBOOT_ERROR("On STR resuming, the Demura bin on DRAM is broken !\n");
        return 0x00;
    }
    return bus_addr;
}

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
MS_BOOL MApi_MsDemura_Init(Demura_Panel_Data panel_data, EN_DEMURA_MULTI_VENDOR multi_vendor)
{
    MS_U64  bus_addr  = 0;
    MS_U32  reg_addr  = 0;
#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    ST_DEMURA_UFC_HEADER BinHeader;
#else
    DeMuraBinHeader  BinHeader;
#endif
    /*if (MDrv_DEMURA_SetIOMapBase() != TRUE)
    {
        UBOOT_ERROR("MDrv_DEMURA_SetIOMapBase Failed\n");
        return FALSE;
    }*/

    if (check_str_resume() == 1)
    {
        bus_addr = MsDemura_STR_Init((MS_U8 *)&BinHeader);
        if (bus_addr == 0)
        {
            UBOOT_ERROR("MsDemura_STR_Init Error\n");
            return FALSE;
        }
    }
    else
    {
        // LGD case
        if ((multi_vendor > EN_DEMURA_MULTI_LGD_START) && (multi_vendor < EN_DEMURA_MULTI_LGD_END)) //#ifdef CONFIG_DEMURA_LGD_DEMO
            bus_addr = MsDemura_Demo_AC_Init((MS_U8 *)&BinHeader, panel_data, multi_vendor);
        else //#else
            bus_addr = MsDemura_AC_Init((MS_U8 *)&BinHeader, panel_data, multi_vendor);

        if (bus_addr == 0)
        {
            UBOOT_ERROR("MsDemura_AC_Init Error\n");
            return FALSE;
        }
    }

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    reg_addr = bus_addr + sizeof(ST_DEMURA_UFC_HEADER);
#else
    reg_addr = (bus_addr + BinHeader.u32RegDataAddr);
#endif
    if (MDrv_DEMURA_RegisterTableLoad((MS_U8 *)&BinHeader, reg_addr) == FALSE)
    {
        UBOOT_ERROR("MDrv_DEMURA_RegisterTableLoad error\n");
        return FALSE;
    }
    pHeader = (DeMuraBinHeader *)((size_t)bus_addr);

    return TRUE;
}
#else
MS_BOOL MApi_MsDemura_Init(Demura_Panel_Data panel_data)
{
    MS_U64  bus_addr  = 0;
    MS_U32  reg_addr  = 0;
    DeMuraBinHeader  BinHeader;

    if (MDrv_DEMURA_SetIOMapBase() != TRUE)
    {
        UBOOT_ERROR("MDrv_DEMURA_SetIOMapBase Failed\n");
        return FALSE;
    }

    if (check_str_resume() == 1)
    {
        bus_addr = MsDemura_STR_Init(&BinHeader);
        if (bus_addr == 0)
        {
            UBOOT_ERROR("MsDemura_STR_Init Error\n");
            return FALSE;
        }
    }
    else
    {
    #ifdef CONFIG_DEMURA_LGD_DEMO
        bus_addr = MsDemura_Demo_AC_Init(&BinHeader, panel_data);
    #else
        bus_addr = MsDemura_AC_Init(&BinHeader, panel_data);
    #endif
        if (bus_addr == 0)
        {
            UBOOT_ERROR("MsDemura_AC_Init Error\n");
            return FALSE;
        }
    }

    reg_addr = (bus_addr + BinHeader.u32RegDataAddr);

    if (MDrv_DEMURA_RegisterTableLoad(&BinHeader, reg_addr) == FALSE)
    {
        UBOOT_ERROR("MDrv_DEMURA_RegisterTableLoad error\n");
        return FALSE;
    }
    pHeader = (DeMuraBinHeader *)((size_t)bus_addr);

    return TRUE;
}
#endif

MS_BOOL MApi_MsDemura_Enable(MS_BOOL bOnOff)
{
    MS_U32 phy_addr;
    MS_U32 lut_addr;

    if (pHeader == NULL)
    {
        UBOOT_ERROR("Mstar_Demura is not initlized !\n");
        return FALSE;
    }

    phy_addr = BA2PA( (MS_PHY)pHeader );

#ifdef CONFIG_MTK_DEMURA_UFC_BIN
    lut_addr = phy_addr + sizeof(ST_DEMURA_UFC_HEADER);
#else
    lut_addr = phy_addr + pHeader->u32LayerDataAddr;
#endif
    if (bOnOff == TRUE)
    {
        UBOOT_TRACE("Lut is loaded on phy_addr : 0x%x\n", (uint)lut_addr);
        MDrv_DEMURA_AutoDownload_Setup((MS_U8 *)pHeader, lut_addr, TRUE);
        MDrv_DEMURA_TurnOn(TRUE);
    }
    else
    {
        MDrv_DEMURA_AutoDownload_Setup((MS_U8 *)pHeader, lut_addr, FALSE);
        MDrv_DEMURA_TurnOn(FALSE);
    }
    return TRUE;
}

void MApi_MsDemura_SetVersion(int version)
{
    set_demura_version(version);
}

void MApi_MsDemura_SetPnlInfo(MS_U16 width, MS_U16 height)
{
    set_demura_pnl(width, height);
}

void MApi_MsDemura_SetBinType(MS_U8 u8type)
{
    set_demura_bin_type(u8type);/* EN_MS_UTIL_BIN_TYPE */
}

void MApi_MsDemura_SetAct(MS_U8 act)
{
    set_demura_act(act);
}

