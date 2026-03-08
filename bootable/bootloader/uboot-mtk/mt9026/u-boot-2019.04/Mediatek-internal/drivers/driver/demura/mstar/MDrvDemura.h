/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 * Copyright Statement:
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019-2024 MediaTek Inc.
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
 * Copyright(C) 2019-2024 MediaTek Inc.
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
#ifndef _MDRV_DeMura_H_
#define _MDRV_DeMura_H_

#include "apiDemura.h"
#include <demura_version.h>

#define DBG_DEMURA                    1

#if (DBG_DEMURA == 1)
#define DEBUG_DEMURA(msg...)           UBOOT_TRACE(msg)
#else
#define DEBUG_DEMURA(msg...)
#endif

#define DEMURA_PACKET_SIZE(x)           (1 << (7 + x))


typedef struct
{
    // Offset: 0x00 ~ 0x03
    MS_U32 u32AllBinCRC;

    // Offset: 0x04 ~ 0x07
    MS_U32 u32HeaderCRC;

    // Offset: 0x08 ~ 0x0B
    MS_U32 u32HeaderSize;

    // Offset: 0x0C ~ 0x0F
    MS_U32 u32AllBinSize;

    // Offset: 0x10 ~ 0x1F
    MS_U8 u8DemuraID[16];

    // Offset: 0x20 ~ 0x21
    MS_U16 u16BinVersion;

    // Offset: 0x22
    MS_U8 u8LayerDataFomrat; // 1: IC Dram format
                          // 2: Compresses IC Dram format

    // Offset: 0x23
    MS_U8 bR_ch_Enable : 1;
    MS_U8 bG_ch_Enable : 1;
    MS_U8 bB_ch_Enable : 1;
    MS_U8 Reserved : 5;

    // Offset: 0x24 ~ 0x25
    MS_U16 u16HNodeCount;

    // Offset: 0x26 ~ 0x27
    MS_U16 u16VNodeCount;

    // Offset: 0x28
    MS_U8 u4StartLayer : 4;
    MS_U8 u4EndLayer   : 4;

    // Offset: 0x29
    MS_U8 bSeperateRGB : 1;
    MS_U8 dummy        : 7;

    // Offset: 0x2A ~ 0x2D
    MS_U32 u32LayerDataAddr;

    // Offset: 0x2E ~ 0x31:
    //  Layer data original (uncompressed) size in bytes.
    //  For layer data format 0 and 1, layer data original size is equal to layer data flash size.
    MS_U32 u32LayerDataOriSize;

    // Offset: 0x32 ~ 0x35:
    //  Size (in bytes) of layer data that stored on flash.
    //  If layer data format is 2 (compressed by 'gzip -9'), it keeps the size of compressed layer data.
    MS_U32 u32LayerDataFlashSize;

    // Offset: 0x36 ~ 0x39
    MS_U32 u32RegDataAddr;

    // Offset: 0x3A ~ 0x3D
    MS_U32 u32RegDataCount; // Count of register

    // Offset: 0x3E ~ 0x3F
    MS_U16 u16DLLVersion;

    // Offset: 0x40 ~ 0x43
    MS_U32 u32ProjectId;

    // Offset: 0x44 ~ 0x47 Customer data start address
    MS_U32 u32CustomerDataAddr;

    // Offset: 0x48 ~ 0x4B: Customer data size in bytes
    MS_U32 u32CustomerDataSize;

    MS_U32   nDate;           // 0x4C~0x4F
    MS_U8    nHBlockSize;     // 0x50
    MS_U8    nVBlockSize;     // 0x51
    MS_U8    nPacketSize;     // 0x52
}  __attribute__((__packed__)) DeMuraBinHeader; // DMC format




typedef enum
{
    E_DEMURA_TOOL_FORMAT          = 0,  // tool format
    E_DEMURA_UNCOMPRESS_FORMAT    = 1,  // uncompress format
    E_DEMURA_COMPRESS_FORMAT      = 2,  // compress format
} DEMURA_LAYER_FORMAT;

typedef enum
{
    E_DEMURA_RGB_PANEL            = 0,
    E_DEMURA_RGBW_PANEL           = 1,
} DEMURA_PANEL_TYPE;


typedef enum
{
    AUTO_DL_TRIG_MODE             = 0,  // Download One time
    AUTO_DL_ENABLE_MODE           = 1,  // Repeated download data
}AUTODOWNLOAD_TRIG_e;



#define UFC_DEMURA_U8ARRAY_TO_U32(hh,hl,lh,ll) \
    (((u32) hh << 24) | ((u32) hl << 16) | ((u32)lh << 8) | (u32)ll)

#define UFC_DEMURA_U8ARRAY_TO_U16(h,l) \
    (((u16)h << 8) | (u16)l)

#define UFC_HEADER_GET_VERSION(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U32((pheader)->version[0], \
                            (pheader)->version[1], \
                            (pheader)->version[2], \
                            (pheader)->version[3])

#define UFC_HEADER_GET_HEADER_SIZE(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U32((pheader)->header_size[0], \
                            (pheader)->header_size[1], \
                            (pheader)->header_size[2], \
                            (pheader)->header_size[3])

#define UFC_HEADER_GET_LUT_SIZE(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U32((pheader)->lut_size[0], \
                            (pheader)->lut_size[1], \
                            (pheader)->lut_size[2], \
                            (pheader)->lut_size[3])

#define UFC_HEADER_GET_H_SIZE(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->h_size[0], \
                            (pheader)->h_size[1])

#define UFC_HEADER_GET_V_SIZE(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->v_size[0], \
                            (pheader)->v_size[1])

#define UFC_HEADER_GET_R_OFFSET(pheader, idx) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->offset_r[idx][0], \
                            (pheader)->offset_r[idx][1])

#define UFC_HEADER_GET_G_OFFSET(pheader, idx) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->offset_g[idx][0], \
                            (pheader)->offset_g[idx][1])

#define UFC_HEADER_GET_B_OFFSET(pheader, idx) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->offset_b[idx][0], \
                            (pheader)->offset_b[idx][1])

#define UFC_HEADER_GET_PLANE(pheader, idx) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->plane[idx][0], \
                            (pheader)->plane[idx][1])

#define UFC_HEADER_GET_BLACKLIMIT(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->black_limit[0], \
                            (pheader)->black_limit[1])

#define UFC_HEADER_GET_WHITELIMIT(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U16((pheader)->white_limit[0], (pheader)->white_limit[1])

#define UFC_HEADER_GET_HEADERCRC(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U32((pheader)->header_crc[0], \
                            (pheader)->header_crc[1], \
                            (pheader)->header_crc[2], \
                            (pheader)->header_crc[3])

#define UFC_HEADER_GET_VERSION(pheader) \
    UFC_DEMURA_U8ARRAY_TO_U32((pheader)->version[0], \
                            (pheader)->version[1], \
                            (pheader)->version[2], \
                            (pheader)->version[3])


#define UFC_DEMURA_IS_8K(h_size)  ((h_size) >= 7680)
#define UFC_DEMURA_IS_INVALID8K_H_BLK(h_blk_size) ((h_blk_size) != 4)
#define UFC_DEMURA_IS_INVALID4K_H_BLK(h_blk_size) (((h_blk_size) != 3) && ((h_blk_size) != 4))
#define UFC_DEMURA_IS_INVALID_V_BLK(v_blk_size) ((v_blk_size < 2) || (v_blk_size > 4))

#define DEMURA_GET_UFC_U8ARRAY0(val) (((val) & 0x0000FF00) >> 8)
#define DEMURA_GET_UFC_U8ARRAY1(val) (((val) & 0x000000FF))

#define DEMURA_GET_UFC_U32ARRAY3(val) (((val) & 0x000000FF))
#define DEMURA_GET_UFC_U32ARRAY2(val) (((val) & 0x0000FF00) >> 8)
#define DEMURA_GET_UFC_U32ARRAY1(val) (((val) & 0x00FF0000) >> 16)
#define DEMURA_GET_UFC_U32ARRAY0(val) (((val) & 0xFF000000) >> 24)

#define UFC_HEADER_DEMURA_FORMAT "MTK"
#define UFC_HEADER_VERSION  DEMURA_CONVERTER_VERSION
#define UFC_DEMURA_HEADER_SIZE   (160)
#define UFC_DEMURA_HEADER_CHECKSUM_NUM (UFC_DEMURA_HEADER_SIZE - 4)
#define UFC_HEADER_GAIN_NUM (8)
#define UFC_HEADER_OFFSET_NUM (8)
#define UFC_HEADER_PLANE_NUM (8)
#define UFC_HEADER_NODE_NUM (8)
#define UFC_HEADER_FORMAT_NUM (16)
#define UFC_HEADER_PADDING_NUM (27)
#define UFC_HEADER_U32_BYTES (4)
#define UFC_HEADER_U16_BYTES (2)


typedef struct
{
    MS_U8 format[UFC_HEADER_FORMAT_NUM];
    MS_U8 version[UFC_HEADER_U32_BYTES];
    MS_U8 header_size[UFC_HEADER_U32_BYTES]; /* 20 ~ 23 */
    MS_U8 lut_size[UFC_HEADER_U32_BYTES];  /* 24 ~ 27 */
    MS_U8 h_size[UFC_HEADER_U16_BYTES]; /* 28 29  need to remap */
    MS_U8 v_size[UFC_HEADER_U16_BYTES]; /* 30 31  need to remap */
    MS_U8 mode; /* 32 */
    MS_U8 plane_num; /* 33 */
    MS_U8 gain_r[UFC_HEADER_GAIN_NUM]; /* 34 ~ 41 */
    MS_U8 gain_g[UFC_HEADER_GAIN_NUM]; /* 42 ~ 49 */
    MS_U8 gain_b[UFC_HEADER_GAIN_NUM]; /* 50 ~ 57 */
    MS_U8 offset_r[UFC_HEADER_OFFSET_NUM][UFC_HEADER_U16_BYTES]; /* 58 ~ 73 */
    MS_U8 offset_g[UFC_HEADER_OFFSET_NUM][UFC_HEADER_U16_BYTES]; /* 74 ~ 89 */
    MS_U8 offset_b[UFC_HEADER_OFFSET_NUM][UFC_HEADER_U16_BYTES]; /* 90 ~ 105 */
    MS_U8 h_blk_size; /* 106 */
    MS_U8 v_blk_size; /* 107 */
    MS_U8 black_limit[UFC_HEADER_U16_BYTES]; /* 108 ~ 109 */
    MS_U8 plane[UFC_HEADER_PLANE_NUM][UFC_HEADER_U16_BYTES]; /* 110 ~ 125 */
    MS_U8 white_limit[UFC_HEADER_U16_BYTES]; /* 126 ~ 127 */
    MS_U8 dither_en; /* 128 */
    MS_U8 padding[UFC_HEADER_PADDING_NUM]; /* 129 ~ 155 */
    MS_U8 header_crc[UFC_HEADER_U32_BYTES]; /* offset DMC_BIN_HEADER_CRC_OFFSET : 156 ~ 159 */
}__attribute__((__packed__)) ST_DEMURA_UFC_HEADER;


MS_BOOL MDrv_DEMURA_SetIOMapBase(void);
MS_BOOL MDrv_DEMURA_Check_HeaderCRC(MS_U8 *pBinBuf);
MS_BOOL MDrv_DEMURA_Check_AllBinCRC(MS_U8 *pBinBuf);
MS_BOOL MDrv_DEMURA_On_Resume(MS_U8 *pHeader);

MS_BOOL MDrv_DEMURA_Is_Support(MS_U8 *pHdr, Demura_Panel_Data panel_data);
void MDrv_DEMURA_Dump_BinInfo(MS_U8 *pHdr);
MS_BOOL MDrv_DEMURA_HandleData(DeMuraBinHeader* pHdr, MS_U8 *dst_start, MS_U8 *src_addr);
MS_BOOL MDrv_DEMURA_RegisterTableLoad(MS_U8* pHdr, MS_U32 RegDataBuf);
void MDrv_DEMURA_AutoDownload_Setup(MS_U8* pHdr, MS_U32 pLayerDataAddr, MS_BOOL bEnable);

MS_U32 MDrv_DEMURA_DMA_Addr_Unit(void);
void MDrv_DEMURA_TurnOn(MS_BOOL bOnOff);

extern int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp, int stoponerr, int offset);

#endif
