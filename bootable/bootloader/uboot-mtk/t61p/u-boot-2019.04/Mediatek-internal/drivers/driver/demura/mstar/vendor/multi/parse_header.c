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
 * Copyright(C) 2019-2023 MediaTek Inc.
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
 * Copyright(C) 2019-2023 MediaTek Inc.
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

#include <command.h>
#include <common.h>
#include <malloc.h>
#include <stdio.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <MsTypes.h>
#include <debug_impl.h>
#include <dmalloc.h>
#include <ms_utils.h>
#include <demura.h>
//#include <convert_entry.h>

#include "vendor.h"
#include "parse_header.h"
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
E_COLOR_MODE Demura_Original_Mode =0;

//G(x) = X16 + X15 + X2 + 1
//CRC16 G(x): 0x8005
const MS_U16 DEMURA_CRC16_TABLE[256] = {
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

// seed ==> 0xFF  or  0x00
MS_U8 inx_crc8(MS_U8 *data, MS_U8 seed, MS_U32 crcStart, MS_U32 crcLength)
{
    MS_U8 coef[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
    MS_U8 crc_out = seed;

    MS_U8 shift[9];
    MS_U8 crc[9];
    MS_U8 din[9];

    MS_U32 index, i;
    for (index = crcStart; index < crcStart + crcLength; index += 1)
    {

        for (i = 0; i < 8; i += 1)
        {
            if ((crc_out & coef[i]) != 0)
                crc[i] = 1;
            else
                crc[i] = 0;

            if ((data[index] & coef[i]) != 0)
                din[i] = 1;
            else
                din[i] = 0;
        }

        shift[1] = (MS_U8)(din[7] ^ crc[7]);
        shift[2] = (MS_U8)(din[6] ^ crc[6]);
        shift[3] = (MS_U8)(din[5] ^ crc[5]);
        shift[4] = (MS_U8)(din[4] ^ crc[4]);
        shift[5] = (MS_U8)(din[3] ^ crc[3] ^ shift[1]);
        shift[6] = (MS_U8)(din[2] ^ crc[2] ^ shift[1] ^ shift[2]);
        shift[7] = (MS_U8)(din[1] ^ crc[1] ^ shift[1] ^ shift[2] ^ shift[3]);
        shift[8] = (MS_U8)(din[0] ^ crc[0] ^ shift[2] ^ shift[3] ^ shift[4]);

        crc[7] = (MS_U8)(shift[1] ^ shift[3] ^ shift[4] ^ shift[5]);
        crc[6] = (MS_U8)(shift[2] ^ shift[4] ^ shift[5] ^ shift[6]);
        crc[5] = (MS_U8)(shift[3] ^ shift[5] ^ shift[6] ^ shift[7]);
        crc[4] = (MS_U8)(shift[4] ^ shift[6] ^ shift[7] ^ shift[8]);
        crc[3] = (MS_U8)(shift[5] ^ shift[7] ^ shift[8]);
        crc[2] = (MS_U8)(shift[6] ^ shift[8]);
        crc[1] = (MS_U8)(shift[7]);
        crc[0] = (MS_U8)(shift[8]);

        crc_out = 0;

        for (i = 0; i < 8; i += 1)
        {
            crc_out += (MS_U8)(crc[i] * coef[i]);
        }
    }
    return crc_out;
}

static MS_BOOL parse_vendor_header_inx_check_asic(MS_U8 *pch)
{
    if(NULL == pch)
    {
        UBOOT_ERROR("ASIC Error(ASIC_Value == 0x0909090909)\n");
        return FALSE;
    }
    
    if ((pch[0] == 0x09) && (pch[1] == 0x09)&& (pch[2] == 0x09) \
         && (pch[3] == 0x09)&& (pch[4] == 0x09))
    {
        UBOOT_ERROR("ASIC Error(ASIC_Value == 0x0909090909)\n");
        return FALSE;
    }
    
    if ((pch[0] == 0xFF) && (pch[1] == 0xFF)&& (pch[2] == 0xFF) \
         && (pch[3] == 0xFF)&& (pch[4] == 0xFF))
    {
        UBOOT_ERROR("ASIC Error(ASIC_Value == 0xFfFfFfFfFf)\n");
        return FALSE;
    }
    return TRUE;
}

MS_BOOL parse_vendor_header_inx(MS_U8 *buf, INX_Demura_Header *phdr)
{
    unsigned int i = 0;
    MS_U32 sum  = 0;
    MS_U8  *pch = NULL;
    INX_Block_INFO *pblk;

    memcpy(phdr, buf, sizeof(INX_Demura_Header));
    if (phdr->magic_id != INX_INDENTIFICATION)
    {
        UBOOT_ERROR("Header Indentifiction  Error(0x%x != 0x%x)\n", (unsigned int)phdr->magic_id, (unsigned int)INX_INDENTIFICATION);
        return FALSE;
    }
    pch = &(phdr->asic[0]);
    if(!parse_vendor_header_inx_check_asic(pch))
        return FALSE;

    // Adjust Demura Header Byte-Order
    phdr->date = inx32_to_cpu(phdr->date);
    for (i = 0; i < ARRAY_SIZE(phdr->block); i++)
    {
        pblk = &(phdr->block[i]);
        pblk->data_checksum = inx16_to_cpu(pblk->data_checksum) >> 8;  // Low Byte : 0x00
        pblk->data_start    = inx32_to_cpu(pblk->data_start);
        pblk->data_length   = inx32_to_cpu(pblk->data_length);
    }
    phdr->lut_checksum    = inx16_to_cpu(phdr->lut_checksum);
    phdr->header_checksum = inx16_to_cpu(phdr->header_checksum);

    // Header checksum
    sum = 0;
    for (i = 0; i < offsetof(INX_Demura_Header, header_checksum); i++)
    {
        sum += buf[i];
    }
    if (phdr->header_checksum != sum)
    {
        UBOOT_ERROR("Header Checksum Error(0x%x != 0x%x)\n", (uint)phdr->header_checksum, (uint)sum);
        return FALSE;
    }

    if ((phdr->function_define & BIT0) == 0)
    {
        UBOOT_ERROR("Demura Definition Error(0x%x & BIT0 != BIT0)\n", (uint)phdr->function_define);
#if (!INX_DEMURA_DL_FLOW)        
    return FALSE;
#endif
    }


    // Adjust Extend_Block_Header Byte-Order
    if (phdr->extension_block)
    {
        buf = buf + offsetof(INX_Demura_Header, extend);

        memcpy(&(phdr->extend), buf, sizeof(_INX_Extend_Header));
        for (i = 0; i < ARRAY_SIZE(phdr->extend.block); i++)
        {
            pblk = &(phdr->extend.block[i]);
            pblk->data_checksum = inx16_to_cpu(pblk->data_checksum) >> 8;  // Low Byte : 0x00
            pblk->data_start    = inx32_to_cpu(pblk->data_start);
            pblk->data_length   = inx32_to_cpu(pblk->data_length);
        }
        phdr->extend.header_checksum = inx16_to_cpu(phdr->extend.header_checksum);

        sum = 0;
        for (i = 0; i < offsetof(_INX_Extend_Header, header_checksum); i++)
        {
            sum += buf[i];
        }
        if (phdr->extend.header_checksum != sum)
        {
            UBOOT_ERROR("Extend Header Checksum Error(0x%x != 0x%x)\n", (uint)phdr->extend.header_checksum, (uint)sum);
            return FALSE;
        }
    }
    return TRUE;
}


MS_BOOL fetch_decode_info_inx(INX_Demura_Header *phdr, Vendor2Mstar_Info *pv2m)
{
    #if(INX_DEMURA_DL_FLOW)
    pv2m->function_define = phdr->function_define;
    #endif

    // ==> Currently, this value is fixed !
    pv2m->hblock_size = 8;
    pv2m->vblock_size = 8;
    pv2m->hblock_num  = 481;
    pv2m->vblock_num  = 271;
    // ==> End

    pv2m->top_limit    = phdr->top_limit;
    pv2m->bot_limit    = phdr->bot_limit;
    pv2m->build_date = phdr->date;
    if (phdr->date >= 0x20000000)
    {
        pv2m->build_date = (pv2m->build_date << 8) + 0x10;  // Dummy build time : AM.10
    }

    unsigned int i;
    INX_Block_INFO *pblk;
    MS_BOOL found;

    for (i =0; i < ARRAY_SIZE(phdr->gray_level); i++)
    {
        if (phdr->gray_level[i] == phdr->top_limit)
            break;
        pv2m->layer_levels[i] = phdr->gray_level[i];
    }
    pv2m->layer_count = i;

    // Search the valid data block
    found = FALSE;
    for (i = 0; i < ARRAY_SIZE(phdr->block); i++)
    {
        pblk = &(phdr->block[i]);
        //printf("pblk->type[1] = 0x%x\n", pblk->type[1]);
        if ((pblk->type[1] & 0x0F) != 0x01)
        {
            continue;
        }
        found = TRUE;
        pv2m->data_start    = pblk->data_start;
        pv2m->data_length   = pblk->data_length;
        pv2m->data_checksum = pblk->data_checksum;
    }

    if (found == FALSE)
    {
        for (i = 0; i < ARRAY_SIZE(phdr->extend.block); i++)
        {
            pblk = &(phdr->extend.block[i]);
            if ((pblk->type[1] & 0x0F) != 0x01)
            {
                continue;
            }
            found = TRUE;
            pv2m->data_start    = pblk->data_start;
            pv2m->data_length   = pblk->data_length;
            pv2m->data_checksum = pblk->data_checksum;
        }
    }

    if (found == FALSE)
    {
        UBOOT_ERROR("Can not find Data Block for Mstar\n");
    }
    return found;
}

MS_U16 HIMAX_CRC_CAL(MS_U8 *pData, MS_U32 length, MS_BOOL bHeader)
{
    MS_U32 i;
    MS_U16 crc = 0, data = 0;

    for(i = 0; i < length; i++)
    {
        // Exclude CRC address for header sum
        if((!bHeader) || ((i != CSOT_HIMAX_LUT_CHECKSUM_H) && (i != CSOT_HIMAX_LUT_CHECKSUM_L) && (i != CSOT_HIMAX_HEADER_CHECKSUM_H) && (i != CSOT_HIMAX_HEADER_CHECKSUM_L)))
        {
            if((i%2) == 0)
                data = (MS_U16)pData[i] << 8;
            else
                data = pData[i];

            crc += data;
        }
    }

    return crc;
}

MS_U16 get_lut_checksum_csot_himax(void)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, CSOT_HIMAX_LUT_CHECKSUM_H, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[0] << 8) | buffer[1];
    return checksum;
}

MS_BOOL load_vendor_header_csot_himax(CSOT_Himax_Demura_Header *phdr)
{
    MS_U32 read_len = CSOT_HIMAX_HEADER_LEN ;
    MS_U8 *buffer = (MS_U8 *)dmalloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, (unsigned int)read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, CSOT_HIMAX_HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header_csot_himax(CSOT_Himax_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    MS_U16 crc = 0;

    phdr->LOW_BOUND = (((MS_U16)buffer[0x60]&0xFF)<<4) | (buffer[0x61]&0x0F); // 0x60[7:0] LOW_H, 0x61[3:0] LOW_L
    phdr->PLANE0_LV = (((MS_U16)buffer[0x3E]&0x0F)<<8) | (buffer[0x35]&0xFF); // 0x3E[3:0] PLANE0_H, 0x35[7:0] PLANE0_L
    phdr->PLANE1_LV = (((MS_U16)buffer[0x3E]&0xF0)<<4) | (buffer[0x36]&0xFF); // 0x3E[7:4] PLANE1_H, 0x36[7:0] PLANE1_L
    phdr->PLANE2_LV = (((MS_U16)buffer[0x3F]&0x0F)<<8) | (buffer[0x37]&0xFF); // 0x3F[3:0] PLANE2_H, 0x37[7:0] PLANE2_L
    phdr->PLANE3_LV = (((MS_U16)buffer[0x3F]&0xF0)<<4) | (buffer[0x38]&0xFF); // 0x3F[7:4] PLANE3_H, 0x38[7:0] PLANE3_L
    phdr->HIGH_BOUND = (((MS_U16)buffer[0x66]&0xFF)<<4) | (buffer[0x67]&0x0F); // 0x66[7:0] HIGH_H, 0x67[3:0] HIGH_L

    phdr->TABLE_CRC = (((MS_U16)buffer[0x99]&0xFF)<<8) | (buffer[0x9A]&0xFF); // 0x99[7:0] TABLE_CRC_H, 0x9A[7:0] TABLE_CRC_L
    phdr->HEADER_CRC = (((MS_U16)buffer[0x19B]&0xFF)<<8) | (buffer[0x19C]&0xFF); // 0x19B[7:0] TABLE_CRC_H, 0x19C[7:0] TABLE_CRC_L

    phdr->lut_length = 484*271*12*CSOT_HIMAX_USE_PLANE_NUM/8;

    crc = HIMAX_CRC_CAL(buffer, CSOT_HIMAX_HEADER_LEN, TRUE);
    printf("HIMAX HEADER CRC = 0x%x \n", (unsigned int)crc);
    if(crc != phdr->HEADER_CRC)
    {
        printf("=====HIMAX HEADER CRC FAIL=====\n");
        dfree(buffer);
        return FALSE;
    }
    printf("=====HIMAX HEADER CRC OK=====\n");

    return TRUE;
}

MS_BOOL load_vendor_lut_csot_himax(CSOT_Himax_Demura_Header *phdr)
{
    MS_U32 lut_size   = phdr->lut_length;
    MS_U16 crc = 0, lut_checksum = phdr->TABLE_CRC;
    MS_U8  *lut_buf = dmalloc(lut_size);
    CHECK_DMALLOC_SPACE(lut_buf, (unsigned int)lut_size);

    MS_BOOL bRet = read_spi_flash(lut_buf, CSOT_HIMAX_LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    crc = HIMAX_CRC_CAL(lut_buf, lut_size, FALSE);
    printf("HIMAX LUT CRC = 0x%x \n", (unsigned int)crc);
    if(crc != lut_checksum)
    {
        printf("=====HIMAX LUT CRC FAIL=====\n");
        dfree(lut_buf);
        return FALSE;
    }
    printf("=====HIMAX LUT CRC OK=====\n");

    phdr->lut_buffer = lut_buf;
    return TRUE;
}

MS_U16 get_lut_checksum_sdc(void)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, SDC_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[1] << 8) | buffer[0];
    return checksum;
}

MS_BOOL load_vendor_header_sdc(SDC_Demura_Header *phdr)
{
    MS_U8 read_len = SDC_HEADER_LEN + 1; // 1 bytes for checksum
    MS_U8 *buffer = (MS_U8 *)dmalloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, (unsigned int)read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, SDC_HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header_sdc(SDC_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    MS_U8 header_checksum = 0, bit_control;
    MS_U32 checksum = 0, i = 0;

    header_checksum = buffer[SDC_HEADER_CHECKSUM - SDC_HEADER_START];
    phdr->PARAMETER_CRC = header_checksum;
    for(i = 0; i < SDC_HEADER_LEN; i++)
    {
        checksum += buffer[i];
    }
    checksum = 0xFC - (checksum & 0xFF);
    printf("header_checksum = 0x%x, calculate checksum = 0x%x \n", (unsigned int)header_checksum, (unsigned int)checksum);

    if(checksum != header_checksum)
    {
        printf("verify dmc header checksum failed !\n");
        dfree(buffer);
        phdr->hdr_buffer = NULL;
        return FALSE;
    }

    // Bit control #1
    bit_control = ((buffer[0] & 0x01)<<7) | ((buffer[1] & 0xFE)>>1);
    phdr->Value_1[6] = (bit_control & (_BIT7 | _BIT6))>>6; // [7:6]
    phdr->Value_1[5] = (bit_control & (_BIT5 | _BIT4))>>4; // [5:4]
    phdr->Value_1[4] = (bit_control & (_BIT3 | _BIT2))>>2; // [3:2]
    phdr->Value_1[3] = (bit_control & (_BIT1 | _BIT0)); // [1:0]

    // Bit control #2
    bit_control = ((buffer[1] & 0x01)<<7) | ((buffer[2] & 0xFE)>>1);
    phdr->Value_1[2] = (bit_control & (_BIT7 | _BIT6))>>6; // [7:6]
    phdr->Value_1[1] = (bit_control & (_BIT5 | _BIT4))>>4; // [5:4]
    phdr->Value_1[0] = (bit_control & (_BIT3 | _BIT2))>>2; // [3:2]


    MS_U16 get_plane[SDC_MAX_PLANE_NUM];
    for(i = 0; i < SDC_MAX_PLANE_NUM; i++)
    {
        get_plane[i] = ((buffer[2+i] & 0x01)<<7) | ((buffer[3+i] & 0xFE)>>1);
    }

    for(i = 0; i < SDC_MAX_PLANE_NUM; i++)
    {
        if(get_plane[i] > 0)
            phdr->PLANE[i] = get_plane[i];
        else
            phdr->PLANE[i] = 0;

        printf("Value_1[%d] = %d, PLANE[%d] = %d ", (int)i, phdr->Value_1[i],(int)i, phdr->PLANE[i]);
        printf("\n");
    }

    return TRUE;
}

MS_BOOL load_vendor_lut_sdc(SDC_Demura_Header *phdr)
{
    MS_U32 lut_size   = SDC_LUT_LEN + 1; // LUT checksum
    MS_U8 lut_checksum = 0;
    MS_U32 checksum = 0, i = 0;
    MS_U8  *lut_buf   = dmalloc(lut_size);
    CHECK_DMALLOC_SPACE(lut_buf, (unsigned int)lut_size);

    MS_BOOL bRet = read_spi_flash(lut_buf, SDC_LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    lut_checksum   = lut_buf[SDC_LUT_LEN];
    phdr->TABLE_CRC = lut_checksum;

    for(i = 0; i < SDC_LUT_LEN; i++)
    {
        checksum += lut_buf[i];
    }
    checksum = 0xFC - (checksum & 0xFF);
    printf("lut_checksum = 0x%x, calculate checksum = 0x%x \n", (unsigned int)lut_checksum, (unsigned int)checksum);

    if (lut_checksum != checksum)
    {
        printf("In %s, verify lut checksum failed !\n", __FUNCTION__);
        dfree(lut_buf);
        return FALSE;
    }

    phdr->lut_buffer = lut_buf;
    phdr->lut_offset = 0;
    phdr->lut_length = lut_size;
    return TRUE;
}

MS_U16 CRC_CAL(MS_U16 crc, int num, MS_U32 *addr)
{
    int i,j;
    MS_U32 data_int, data_temp;
    MS_U16 data_short;
//    MS_U8 data_char;

    /* Loop until num = 0 */
    while(num--)
    {
        /* Fetch one 4-bytes data drom memory */
        data_int = *addr;

        for(j = 4; j > 0; j--)
        {
            /* Fetch one byte each time from [31:24] to [7:0] */
            data_temp = data_int >> 8 * (j-1);
//            data_char = data_temp;
            data_short = data_temp << 8;    /* 8 bit to 16 bit */
            crc = crc ^ data_short;         /* XOR into CRC */

            /* Prepare to rotate 8 bits */
            for(i = 0; i < 8; i++)
            {
                if(crc & 0x8000)
                {
                    /* rotate and XOR with polynomic */
                    crc = (crc << 1) ^ 0x8005;
                }
                else
                {
                    crc <<= 1;
                }
            }

            /* Ensure CRC remains 16-bit value */
            crc &= 0xFFFF;
        }

        addr++;
    }

    return crc;
}

MS_U16 get_lut_checksum_csot_hisilicon(void)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, CSOT_HISI_DMC_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HISI_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[1] << 8) | buffer[0];
    return checksum;
}

MS_BOOL load_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr)
{
    MS_U8 read_len = CSOT_HISI_HEADER_LEN ;
    MS_U8 *buffer = (MS_U8 *)dmalloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, (unsigned int)read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, CSOT_HISI_HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HISILICON Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    MS_U8 H_block, V_block;

    phdr->DEMURA_MODE = (buffer[0x0C] == 1)?1:0; // 1 : color, 0 : mono
    phdr->DEMURA_PLANE_NUM = (buffer[0x10] == 1)?3:5; // 1 : 3plane, 0 : 5plane
    phdr->DEMURA_BLK_H = (buffer[0x18] == 1)?3:2; // 1 : 8*8, 0 : 4*4
    phdr->DEMURA_BLK_V = phdr->DEMURA_BLK_H;
    phdr->lut_length = (MS_U32)buffer[0x1C] | ((MS_U32)buffer[0x1D]<<8) | ((MS_U32)buffer[0x1E]<<16) | ((MS_U32)buffer[0x1F]<<24);
    phdr->TABLE_CRC = (MS_U32)buffer[0x20] | ((MS_U32)buffer[0x21]<<8) | ((MS_U32)buffer[0x22]<<16) | ((MS_U32)buffer[0x23]<<24);

    H_block = 1<<(phdr->DEMURA_BLK_H);
    V_block = 1<<(phdr->DEMURA_BLK_V);

    phdr->DEMURA_TBL_H = (3840/H_block) + 1;
    phdr->DEMURA_TBL_V = (2160/V_block) + 1;

    if((buffer[0x00] != 0x64)||(buffer[0x01] != 0x65)||(buffer[0x02] != 0x6D)||(buffer[0x03] != 0x75)||(buffer[0x04] != 0x72)||(buffer[0x05] != 0x61))
    {
        printf("Not demura data\n");
        return FALSE;
    }
    else if((phdr->lut_length == 0xffffffff) || (phdr->lut_length == 0))
    {
        printf("Read invalid values from flash \n");
        return FALSE;
    }

    return TRUE;
}

void dump_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr)
{
    //MS_U8 i;

    printf("DEMURA_PLANE_NUM    :   0x%x \n", (unsigned int)phdr->DEMURA_PLANE_NUM);
    printf("DEMURA_MODE         :   0x%x \n", (unsigned int)phdr->DEMURA_MODE);
    printf("DEMURA_TBL_H        :   0x%x \n", (unsigned int)phdr->DEMURA_TBL_H);
    printf("DEMURA_TBL_V        :   0x%x \n", (unsigned int)phdr->DEMURA_TBL_V);
    printf("DEMURA_BLK_H        :   0x%x \n", (unsigned int)phdr->DEMURA_BLK_H);
    printf("DEMURA_BLK_V        :   0x%x \n", (unsigned int)phdr->DEMURA_BLK_V);
    printf("TABLE_CRC           :   0x%x \n", (unsigned int)phdr->TABLE_CRC);
    printf("DEMURA_TABLE_LENGTH :   %u \n", (unsigned int)phdr->lut_length);
}

MS_BOOL load_vendor_lut_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr)
{
    MS_U32 lut_size   = phdr->lut_length; // LUT checksum
    MS_U32 lut_checksum = phdr->TABLE_CRC;
    MS_U32 i = 0, lut_size_32 = 0;
    MS_U16 crc = 0, crc_read = (MS_U16)lut_checksum;
    MS_U8  *lut_buf;//   = dmalloc(lut_size);
    //CHECK_DMALLOC_SPACE(lut_buf, lut_size);

    if(lut_size % 32 != 0) // not multiplier of 32 byte
    {
        lut_size_32 = lut_size + (32 - (lut_size%32));
    }
    else
    {
        lut_size_32 = lut_size;
    }
    lut_buf = dmalloc(lut_size_32);
    CHECK_DMALLOC_SPACE(lut_buf, (unsigned int)lut_size_32);
    for(i = 0; i < (32 - (lut_size%32)); i++)
        lut_buf[0x9B9D0 + i] = 0;

    MS_BOOL bRet = read_spi_flash(lut_buf, CSOT_HISI_LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HISILICON Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    crc = CRC_CAL(0xFFFF, (lut_size_32/4), (MS_U32 *)lut_buf);
    //printf("HISILICON CRC = 0x%x \n", crc);
    if(crc != crc_read)
    {
        printf("=====CRC FAIL=====\n");
        dfree(lut_buf);
        return FALSE;
    }
    else
        printf("=====HISILICON CRC OK=====\n");

    phdr->lut_buffer = lut_buf;
    return TRUE;
}

static int do_chot_checksum(MS_U16 CHOT_CRC, MS_U8 *buf, MS_U32 start, MS_U32 end)
{
    MS_U32 i, j;
    MS_U16 crc = 0x0000;
    MS_U16 crc_temp_1, crc_temp_2, crc_temp_3;
    MS_U8 data_buffer;
    MS_U32 checksum = 0; // Don't care checksum overflow. For "MOD(SUM(d[65538], d[65694]), 1<<16)"

    i = 0; //data index

    for(i = start; i <= end; i++)
    {
        data_buffer = buf[i];
        checksum += data_buffer;
        crc_temp_1 = 0;
        crc_temp_2 = 0;
        crc_temp_3 = 0;

        for(j = 0; j < 8; j++)
        {
            crc_temp_1 = ((crc >> 15) ^ data_buffer) & 0x0001;  // result 1 crc_msb[15]^data bit0
            crc_temp_2 = ((crc >> 1) ^ crc_temp_1) & 0x0001;    // result 2 --> crc[1]^result 1
            crc_temp_3 = ((crc >> 14) ^ crc_temp_1) & 0x0001;   // result 3 --> crc[14]^result 1
            crc = crc << 1;
            crc &= 0x7FFA;  // clear crc[0]/[2]/[15];
            crc |= crc_temp_1;  // Load result 1 to crc[0]
            crc |= (crc_temp_2 << 2);    // Load result 2 to crc[2]
            crc |= (crc_temp_3 << 15);  // Load result 3 to crc[15]
            data_buffer >>= 1;
        }
    }

    printf("BIN_CRC = 0x%x, crc = 0x%x, checksum = %d \n", (unsigned int)CHOT_CRC, (unsigned int)crc, (int)checksum);

    if(crc == CHOT_CRC) // CRC correct
        return 0;
    else
        return -1;
}

static int do_dmc_checksum(MS_U16 checksum, MS_U8 *buf, MS_U32 start, MS_U32 end)
{
    MS_U32 i = 0;
    MS_U32 cal_sum = 0; // Don't care checksum overflow. For "MOD(SUM(d[65538], d[65694]), 1<<16)"

    UBOOT_DEBUG("start = 0x%x, end = 0x%x, buf = 0x%p\n", start, end, buf);
    for (i = start; i < end; i++)
    {        
        cal_sum += buf[i];
    }
    cal_sum &= 0xFFFF;

    if (cal_sum != checksum)
    {
        UBOOT_ERROR("do_dmc_checksum failed! cal_sum(0x%x) != checksum(0x%x)\n", (uint)cal_sum, (uint)checksum);
        return -1;
    }
    return 0;
}

MS_BOOL load_vendor_header_chot(CHOT_Demura_Header *phdr, MS_U32 flash_offset)
{
    MS_U8 *buffer = (MS_U8 *)dmalloc(HEADER_LEN);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, (unsigned int)HEADER_LEN);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, flash_offset + HEADER_START, HEADER_LEN);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}

MS_BOOL parse_vendor_header_chot(CHOT_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    CHOT_BIN_Header *pbhdr = (CHOT_BIN_Header *)buffer;

    phdr->PARAMETER_CRC    =  (buffer[0] << 8) | buffer[1];

    int verify_start  =  CHOT_HEADER_START - HEADER_START;
    int verify_end    =  CHOT_HEADER_END - HEADER_START;
    if (do_chot_checksum(phdr->PARAMETER_CRC, buffer, verify_start, verify_end) != 0)
    {
        printf("verify dmc header checksum failed !\n");
        dfree(buffer);
        phdr->hdr_buffer = NULL;
        return FALSE;
    }
    phdr->hdr_offset = verify_start;
    phdr->hdr_length = verify_end - verify_start + 1;

    if (pbhdr->DEMURA_PLANE_NUM >= MAX_PLANE_NUM)    // MAX 8
        phdr->DEMURA_PLANE_NUM  =  MAX_PLANE_NUM;
    else
        phdr->DEMURA_PLANE_NUM = pbhdr->DEMURA_PLANE_NUM;

    phdr->DEMURA_TBL_H    =  ((MS_U16)pbhdr->DEMURA_TBL_H_H << 8) | pbhdr->DEMURA_TBL_H_L;
    phdr->DEMURA_TBL_V    =  ((MS_U16)pbhdr->DEMURA_TBL_V_H << 8) | pbhdr->DEMURA_TBL_V_L;

    if(pbhdr->DEMURA_BLK_H == 0)
        phdr->DEMURA_BLK_H = 4;
    else if(pbhdr->DEMURA_BLK_H == 1)
        phdr->DEMURA_BLK_H = 8;
    else if(pbhdr->DEMURA_BLK_H == 2)
        phdr->DEMURA_BLK_H = 16;

    if(pbhdr->DEMURA_BLK_V == 0)
        phdr->DEMURA_BLK_V = 4;
    else if(pbhdr->DEMURA_BLK_V == 1)
        phdr->DEMURA_BLK_V = 8;
    else if(pbhdr->DEMURA_BLK_V == 2)
        phdr->DEMURA_BLK_V = 16;

    phdr->LOWER_BOUND = ((MS_U16)pbhdr->LOWER_BOUND_H << 8) | pbhdr->LOWER_BOUND_L;
    phdr->UPPER_BOUND = ((MS_U16)pbhdr->UPPER_BOUND_H << 8) | pbhdr->UPPER_BOUND_L;

    MS_U16 get_plane[MAX_PLANE_NUM];
    get_plane[0] = ((MS_U16)pbhdr->PLANE00_LV_H << 8) | pbhdr->PLANE00_LV_L;
    get_plane[1] = ((MS_U16)pbhdr->PLANE01_LV_H << 8) | pbhdr->PLANE01_LV_L;
    get_plane[2] = ((MS_U16)pbhdr->PLANE02_LV_H << 8) | pbhdr->PLANE02_LV_L;
    get_plane[3] = ((MS_U16)pbhdr->PLANE03_LV_H << 8) | pbhdr->PLANE03_LV_L;
    get_plane[4] = ((MS_U16)pbhdr->PLANE04_LV_H << 8) | pbhdr->PLANE04_LV_L;
    get_plane[5] = ((MS_U16)pbhdr->PLANE05_LV_H << 8) | pbhdr->PLANE05_LV_L;
    get_plane[6] = ((MS_U16)pbhdr->PLANE06_LV_H << 8) | pbhdr->PLANE06_LV_L;
    get_plane[7] = ((MS_U16)pbhdr->PLANE07_LV_H << 8) | pbhdr->PLANE07_LV_L;

    MS_U8 i;
    for(i = 0; i < MAX_PLANE_NUM; i++)
    {
        if(get_plane[i] > 0)
            phdr->PLANE[i] = get_plane[i];
        else
            phdr->PLANE[i] = 0;
    }

    // TBL_LEN = ceil(DEMURA_TBL_H * DEMURA_TBL_V * 12 * DEMURA_PLANE_NUM / 256) * 32
    MS_U32 u32_temp = 0;
    u32_temp = (MS_U32)phdr->DEMURA_TBL_H * phdr->DEMURA_TBL_V * 12 * phdr->DEMURA_PLANE_NUM;
    if((u32_temp & 0xff) != 0x00) // round
        u32_temp = (u32_temp >> 8) + 1;
    else
        u32_temp = (u32_temp >> 8);

    phdr->DEMURA_TABLE_LENGTH = u32_temp * 32;
    // TBL_END = ceil(DEMURA_TBL_H * DEMURA_TBL_V * 12 * DEMURA_PLANE_NUM / 256) * 32 + DEMURA_TBL_START_ADDRESS - 1
    phdr->TBL_END_ADDR = phdr->DEMURA_TABLE_LENGTH + CHOT_LUT_START - 1;

    return TRUE;
}

void dump_vendor_header_chot(CHOT_Demura_Header *phdr)
{
    MS_U8 i;

    printf("PARAMETER_CRC       :   0x%x \n", (unsigned int)phdr->PARAMETER_CRC);
    printf("DEMURA_PLANE_NUM    :   0x%x \n", (unsigned int)phdr->DEMURA_PLANE_NUM);
    printf("DEMURA_TBL_H        :   0x%x \n", (unsigned int)phdr->DEMURA_TBL_H);
    printf("DEMURA_TBL_V        :   0x%x \n", (unsigned int)phdr->DEMURA_TBL_V);
    printf("DEMURA_BLK_H        :   0x%x \n", (unsigned int)phdr->DEMURA_BLK_H);
    printf("DEMURA_BLK_V        :   0x%x \n", (unsigned int)phdr->DEMURA_BLK_V);
    printf("LOWER_BOUND         :   0x%x \n", (unsigned int)phdr->LOWER_BOUND);
    printf("UPPER_BOUND         :   0x%x \n", (unsigned int)phdr->UPPER_BOUND);
    printf("PANEL_LEVEL         :   ");
    for(i = 0; i < phdr->DEMURA_PLANE_NUM; i++)
        printf("PLANE[%d] = 0x%x  ", i, (unsigned int)phdr->PLANE[i]);

    printf("\n");
    printf("TBL_END_ADDR        :   %u \n", (unsigned int)phdr->TBL_END_ADDR);
    printf("TABLE_CRC           :   0x%x \n", (unsigned int)phdr->TABLE_CRC);
    printf("DEMURA_TABLE_LENGTH :   %u \n", (unsigned int)phdr->DEMURA_TABLE_LENGTH);
}

MS_BOOL load_vendor_lut_chot(CHOT_Demura_Header *phdr, MS_U32 flash_offset)
{
    MS_U32 lut_size   = phdr->DEMURA_TABLE_LENGTH; //cal_dmc_lut_size(phdr);
    MS_U32 lut_offset = CHOT_LUT_START - CHOT_LUT_CHECKSUM;
    MS_U32 lut_cs_len = lut_size + lut_offset;
    //MS_U32 lut_end = DMC_LUT_START + lut_cs_len - 1;

    MS_U8  *lut_buf   = dmalloc(lut_cs_len);
    CHECK_DMALLOC_SPACE(lut_buf, (unsigned int)lut_cs_len);

    printf("lut_cs_len = %d, lut_size = %d, lut_offset = %d \n",
    (int)lut_cs_len, (int)lut_size, (int)lut_offset);

    MS_BOOL bRet = read_spi_flash(lut_buf, flash_offset + CHOT_LUT_CHECKSUM, lut_cs_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    MS_U16 checksum   = (lut_buf[0] << 8) | lut_buf[1];
    phdr->TABLE_CRC = checksum;

    int verify_start  =  CHOT_LUT_START - CHOT_LUT_CHECKSUM;
    int verify_end    =  verify_start + lut_size - 1;

    if (do_chot_checksum(checksum, lut_buf, verify_start, verify_end) != 0)
    {
        printf("In %s, verify lut checksum failed !\n", __FUNCTION__);
        dfree(lut_buf);
        return FALSE;
    }

    phdr->lut_buffer = lut_buf;
    phdr->lut_offset = lut_offset;
    phdr->lut_length = lut_size;
    return TRUE;
}

MS_U16 get_lut_checksum_chot(MS_U32 flash_offset)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, flash_offset + CHOT_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[0] << 8) | buffer[1];
    return checksum;
}

MS_BOOL load_vendor_header(AUO_Demura_Header *phdr)
{
    MS_U32 AUO_HEADER_START = DMC_HEADER_CHECKSUM;
    MS_U32 AUO_HEADER_LEN = (DMC_HEADER_END   - DMC_HEADER_CHECKSUM);

    MS_U8 *buffer = (MS_U8 *)dmalloc(AUO_HEADER_LEN);
    if (buffer == NULL)
    {
        UBOOT_ERROR("malloc 0x%x byte failed!\n", (unsigned int)AUO_HEADER_LEN);
        return FALSE;
    }

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        dfree(buffer);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, AUO_HEADER_START, AUO_HEADER_LEN);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read AUO_DMC_Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(AUO_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    AUO_BIN_Header *pbhdr = (AUO_BIN_Header *)buffer;

    UBOOT_DEBUG("IN phdr= 0x%p, buffer[0]=0x%x, buffer[1]=0x%x\n", phdr, buffer[0], buffer[1]);

    phdr->hdr_checksum    =  (buffer[0] << 8) | buffer[1];
    phdr->rgb_mode        =  pbhdr->rgb_mode ? E_RGB_MODE : E_MONO_MODE;
#if (DEMURA_ONLY_MONO_MODE)
    Demura_Original_Mode = phdr->rgb_mode;
    if(Demura_Original_Mode == E_RGB_MODE)
    {
        phdr->rgb_mode = E_MONO_MODE;
    }
#endif
    int verify_start  =  DMC_HEADER_START - DMC_HEADER_CHECKSUM;
    int verify_end    =  DMC_HEADER_END   - DMC_HEADER_CHECKSUM;
    if (do_dmc_checksum(phdr->hdr_checksum, buffer, verify_start, verify_end) != 0)
    {
        UBOOT_ERROR("verify dmc header checksum failed !\n");
        UBOOT_DEBUG("DMC_HEADER_START %d, DMC_HEADER_END %d, DMC_HEADER_CHECKSUM %d\n", DMC_HEADER_START, DMC_HEADER_END, DMC_HEADER_CHECKSUM);
        dfree(buffer);
        phdr->hdr_buffer = NULL;
        return FALSE;
    }
    phdr->hdr_offset = verify_start;
    phdr->hdr_length = verify_end - verify_start;

    if (pbhdr->dmc_block_size == 1)
    {
        phdr->dmc_block_size  =  64;
    }
    else if (pbhdr->dmc_block_size == 2)
    {
        phdr->dmc_block_size  =  128;
    }
    else if (pbhdr->dmc_block_size == 3)
    {
        phdr->dmc_block_size  =  256;
    }

    if ((pbhdr->dmc_panel_num >= 1) && (pbhdr->dmc_panel_num <= 8))
    {
        phdr->dmc_panel_num  =  pbhdr->dmc_panel_num;
    }
    else
    {
        phdr->dmc_panel_num  =  3;
    }

    if ((pbhdr->dmc_h_block >= 2) && (pbhdr->dmc_h_block <= 4))
    {
        phdr->dmc_h_block_size  =  1 << pbhdr->dmc_h_block;
    }
    else
    {
        phdr->dmc_h_block_size  =  2;
    }
    if ((pbhdr->dmc_v_block >= 2) && (pbhdr->dmc_v_block <= 4))
    {
        phdr->dmc_v_block_size  =  1 << pbhdr->dmc_v_block;
    }
    else
    {
        phdr->dmc_v_block_size  =  2;
    }

    phdr->dmc_h_lut_num    =  (pbhdr->dmc_h_lut_num_h << 4) | pbhdr->dmc_h_lut_num_l;
    phdr->dmc_v_lut_num    =  (pbhdr->dmc_v_lut_num_h << 8) | pbhdr->dmc_v_lut_num_l;
    phdr->dmc_black_limit  =  (pbhdr->dmc_black_limit_h << 8) | pbhdr->dmc_black_limit_l;

    unsigned int i, panel_level;
    for (i = 0; i < ARRAY_SIZE(pbhdr->panel_levels); i++)
    {
        panel_level = (pbhdr->panel_levels[i].high << 8) | pbhdr->panel_levels[i].low;
        if (panel_level > 0)
        {
            phdr->panel_levels[i] =  panel_level;
        }
    }

    phdr->dmc_white_limit  =  (pbhdr->dmc_white_limit_h << 8) | pbhdr->dmc_white_limit_l;

    UBOOT_DEBUG("OK\n");
    return TRUE;
}


void dump_vendor_header(AUO_Demura_Header *phdr)
{
    unsigned int i;
    printf("RGB_MODE          :  %s\n",       phdr->rgb_mode ? "RGB_MODE" : "MONO_MODE");
    printf("DMC_BLOCK_SIZE    :  %d bits\n",  (int)phdr->dmc_block_size);
    printf("DMC_PANEL_NUM     :  %d layer\n", (int)phdr->dmc_panel_num);
    printf("DMC_H_BLOCK_SIZE  :  %d pixs\n",  (int)phdr->dmc_h_block_size);
    printf("DMC_V_BLOCK_SIZE  :  %d pixs\n",  (int)phdr->dmc_v_block_size);

    printf("DMC_H_LUT_NUM     :  %d\n",       (int)phdr->dmc_h_lut_num);
    printf("DMC_V_LUT_NUM     :  %d\n",       (int)phdr->dmc_v_lut_num);
    printf("DMC_BLACK_LIMIT   :  0x%04x\n",   (uint)(phdr->dmc_black_limit));

    printf("DMC_PANEL_LEVEL   : ");
    for (i = 0; i < phdr->dmc_panel_num; i++)
    {
        printf(" 0x%x", (unsigned int)phdr->panel_levels[i]);
    }
    printf("\n");
    printf("DMC_WHITE_LIMIT   :  0x%04x\n",   (uint)(phdr->dmc_white_limit));
    printf("\n");
}


static MS_U32 cal_dmc_lut_size(AUO_Demura_Header *phdr)
{
    MS_U32 lut_size = 0;
    if(Demura_Original_Mode == E_RGB_MODE)
    {
        lut_size = phdr->dmc_panel_num * 3 * phdr->dmc_h_lut_num * phdr->dmc_v_lut_num;
    }
    else
    {
        if (phdr->rgb_mode == E_RGB_MODE)
        {
            lut_size = phdr->dmc_panel_num * 3 * phdr->dmc_h_lut_num * phdr->dmc_v_lut_num;
        }
        else if (phdr->rgb_mode == E_MONO_MODE)
        {
            lut_size = phdr->dmc_panel_num * phdr->dmc_h_lut_num * phdr->dmc_v_lut_num;
        }
    }
    return lut_size;
}


MS_U16 get_lut_checksum(void)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  checksum = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        return checksum;
    }

    bRet = read_spi_flash(buffer, DMC_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read AUO_DMC_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[0] << 8) | buffer[1];
    return checksum;
}


MS_BOOL load_vendor_lut(AUO_Demura_Header *phdr)
{
    MS_U32 lut_size   = cal_dmc_lut_size(phdr);
    MS_U32 lut_offset = DMC_LUT_START - DMC_LUT_CHECKSUM;
    MS_U32 lut_cs_len = lut_size + lut_offset;

    MS_U8  *lut_buf   = dmalloc(lut_cs_len);
    CHECK_DMALLOC_SPACE(lut_buf, (uint)lut_cs_len);

    MS_BOOL bRet = read_spi_flash(lut_buf, DMC_LUT_CHECKSUM, lut_cs_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read AUO_DMC_Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }
    MS_U16 checksum   = (lut_buf[0] << 8) | lut_buf[1];

    if (do_dmc_checksum(checksum, lut_buf, lut_offset, lut_cs_len) != 0)
    {
        UBOOT_ERROR("verify lut checksum failed !\n");
        dfree(lut_buf);
        return FALSE;
    }
    if (Demura_Original_Mode == E_RGB_MODE)
    {
        MS_U32 i,n;
        MS_U32 lut_mono_size;
        lut_mono_size = phdr->dmc_panel_num * phdr->dmc_h_lut_num * phdr->dmc_v_lut_num + lut_offset ;
        for (i = _2; i < lut_mono_size; i++)
        {
            n = _3 * (i - 1);
            lut_buf[i] = lut_buf[n];
        }
    }
    phdr->lut_buffer   = lut_buf;
    phdr->lut_offset   = lut_offset;
    phdr->lut_length   = lut_size;
    phdr->lut_checksum = checksum;
    return TRUE;
}

// CSOT_CSOT CRC16
MS_U16 CSOT_CSOT_CalcCRC16(MS_U8 *pData,MS_U32 len)
{
    MS_U8 dat;
    MS_U16 crcData = 0x0000;
    while(len--){
        dat = (MS_U8)(crcData>>8);
        crcData <<= 8;
        crcData ^= DEMURA_CRC16_TABLE[dat^*pData++];
    }
    return crcData;
}

MS_U16 get_CSOT_CSOT_CRC(E_FILE_CRC eCRCType)
{
    MS_U8   buffer[2] = {0};
    MS_BOOL bRet;
    MS_U16  u16Crc = 0xFFFF;

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("Init_spi_flash error!\n");
        return u16Crc;
    }

    switch (eCRCType)
    {
        case E_FILE_TATOL_CRC:
            bRet = read_spi_flash(buffer, 0x1C, sizeof(buffer));
            break;
        case E_FILE_HEADER_CRC:
            bRet = read_spi_flash(buffer, 0x1E, sizeof(buffer));
            break;
        case E_DEMURA_PARA_CRC:
            bRet = read_spi_flash(buffer, 0x2C, sizeof(buffer));
            break;
        case E_DEMURA_LUT_CRC:
            bRet = read_spi_flash(buffer, 0x3C, sizeof(buffer));
            break;
        default:
            bRet = FALSE;
            break;
    }

    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT CRC from spi flash failed !\n");
        return u16Crc;
    }

    u16Crc = ((MS_U16)buffer[1] << 8) | buffer[0];
    return u16Crc;
}


MS_BOOL load_vendor_header_csot_csot(CSOT_CSOT_Demura_Header *phdr)
{
    MS_U16 calc_crc = 0;
    CSOT_File_Header *_file_header = NULL;
    CSOT_Section_Header *_section_header = NULL;

    // read File_Header
    phdr->file_header = (MS_U8 *)dmalloc(sizeof(CSOT_File_Header));
    if (phdr->file_header == NULL)
    {
        UBOOT_ERROR("csot malloc file_header 0x%x byte failed!\n", (MS_U32)sizeof(CSOT_File_Header));
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }
    _file_header = (CSOT_File_Header *)phdr->file_header;

    if (read_spi_flash(phdr->file_header, CSOT_CSOT_HEADER_START, sizeof(CSOT_File_Header)) == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT File_Header from spi flash failed !\n");
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }

    // check CRC
    calc_crc = CSOT_CSOT_CalcCRC16(phdr->file_header, sizeof(CSOT_File_Header)
        - sizeof(_file_header->file_header_CRC));
    if (calc_crc != _file_header->file_header_CRC)
    {
        UBOOT_ERROR("=====CSOT_CSOT demura _file_header crc CRC FAIL %x, %x=====\n", calc_crc
            , _file_header->file_header_CRC);
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }
    UBOOT_DEBUG("CSOT File_Header_CRC OK %x\n", _file_header->file_header_CRC);

    // read Section_Header
    phdr->section_header = (MS_U8 *)dmalloc(sizeof(CSOT_Section_Header) * _file_header->sectionNum);
    if (phdr->section_header == NULL)
    {
        UBOOT_ERROR("CSOT malloc %u section_header 0x%x byte failed!\n", _file_header->sectionNum
            , (MS_U32)(sizeof(CSOT_Section_Header) * _file_header->sectionNum));
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }
    _section_header = (CSOT_Section_Header *)phdr->section_header;

    if (read_spi_flash(phdr->section_header, CSOT_CSOT_HEADER_START + sizeof(CSOT_File_Header)
        , sizeof(CSOT_Section_Header)* _file_header->sectionNum) == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT Section_Header from spi flash failed !\n");
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }

    // read Demura_Header
    phdr->demura_header = (MS_U8 *)dmalloc(sizeof(CSOT_Demura_Header));
    if (phdr->demura_header == NULL)
    {
        UBOOT_ERROR("CSOT malloc demura_header 0x%x byte failed!\n", (MS_U32)sizeof(CSOT_Demura_Header));
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }

    if (read_spi_flash(phdr->demura_header, _section_header[0].header_offser
        , sizeof(CSOT_Demura_Header)) == FALSE)
    {
        UBOOT_ERROR("Read CSOT_CSOT File_Header from spi flash failed !\n");
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }

    // check CRC
    calc_crc = CSOT_CSOT_CalcCRC16(phdr->demura_header, sizeof(CSOT_Demura_Header));
    if (calc_crc != _section_header[0].header_section_CRC)
    {
        UBOOT_ERROR("=====CSOT_CSOT demura para crc CRC FAIL %x, %x=====\n", calc_crc
            , _section_header[0].header_section_CRC);
        goto CSOT_LOAD_HEADER_FAIL_RETURN;
    }

    UBOOT_DEBUG("CSOT demura para crc CRC OK %x\n", _section_header[0].header_section_CRC);

    return TRUE;

CSOT_LOAD_HEADER_FAIL_RETURN:
    if (phdr->file_header)
    {
        dfree(phdr->file_header);
        phdr->file_header = NULL;
    }
    if (phdr->section_header)
    {
        dfree(phdr->section_header);
        phdr->section_header = NULL;
    }
    if (phdr->demura_header)
    {
        dfree(phdr->demura_header);
        phdr->demura_header = NULL;
    }
    return FALSE;
}

MS_BOOL parse_vendor_header_csot_csot(CSOT_CSOT_Demura_Header *phdr)
{
    CSOT_File_Header *file_phdr = (CSOT_File_Header *)phdr->file_header;
    CSOT_Demura_Header *_demura_header = (CSOT_Demura_Header *)phdr->demura_header;
    CSOT_Section_Header *section_hdr = (CSOT_Section_Header *)phdr->section_header;
    int i = 0;;
    UBOOT_TRACE("IN\n");

    phdr->DEMURA_MODE = _demura_header->enDemuraMode;
    if (( _demura_header->u8PlaneNum >= 1) && ( _demura_header->u8PlaneNum <= _8))
    {
         phdr->DEMURA_PLANE_NUM = _demura_header->u8PlaneNum;
    }
    else
    {
        phdr->DEMURA_PLANE_NUM = _3;
    }

    if (_demura_header->u8HblockSize == _4)  // 2:4pixs 3:8pixs 4:16pixs
    {
        phdr->DEMURA_BLK_H = _2;
    }
    else if (_demura_header->u8HblockSize == _8)
    {
        phdr->DEMURA_BLK_H = _3;
    }
    else if (_demura_header->u8HblockSize == _16)
    {
        phdr->DEMURA_BLK_H = _4;
    }

    if (_demura_header->u8VblockSize == _4)  // 2:4pixs 3:8pixs 4:16pixs
    {
        phdr->DEMURA_BLK_V = _2;
    }
    else if (_demura_header->u8VblockSize == _8)
    {
        phdr->DEMURA_BLK_V = _3;
    }
    else if (_demura_header->u8VblockSize == _16)
    {
        phdr->DEMURA_BLK_V = _4;
    }

    phdr->DEMURA_TBL_H = _demura_header->u16HLutNum;
    phdr->DEMURA_TBL_V = _demura_header->u16VLutNum;
    phdr->DEMURA_BLACK_LIMIT = _demura_header->u16BlackLimitR;
    phdr->DEMURA_PLANE_LEVEL1 = _demura_header->u16Plane1LevelR;
    phdr->DEMURA_PLANE_LEVEL2 = _demura_header->u16Plane2LevelR;
    phdr->DEMURA_PLANE_LEVEL3 = _demura_header->u16Plane3LevelR;
    phdr->DEMURA_PLANE_LEVEL4 = _demura_header->u16Plane4LevelR;
    phdr->DEMURA_PLANE_LEVEL5 = _demura_header->u16Plane5LevelR;
    phdr->DEMURA_PLANE_LEVEL6 = _demura_header->u16Plane6LevelR;
    phdr->DEMURA_PLANE_LEVEL7 = _demura_header->u16Plane7LevelR;
    phdr->DEMURA_PLANE_LEVEL8 = _demura_header->u16Plane8LevelR;
    phdr->DEMURA_WHITE_LIMIT = _demura_header->u16WhiteLimitR;
    phdr->DEMURA_GAIN_R = _demura_header->u16CompValGainR;
    phdr->DEMURA_GAIN_G = _demura_header->u16CompValGainG;
    phdr->DEMURA_GAIN_B = _demura_header->u16CompValGainB;
    phdr->DEMURA_OFFSET_R = _demura_header->u16CompValOffsetR;
    phdr->DEMURA_OFFSET_G = _demura_header->u16CompValOffsetG;
    phdr->DEMURA_OFFSET_B = _demura_header->u16CompValOffsetB;
    phdr->DEMURA_INT_BIT_WIDTH = _demura_header->u8IntBitWidth;
    phdr->DEMURA_DEC_BIT_WIDTH = _demura_header->u8DecimalsBitWidth;
    for (i = 0; i < file_phdr->sectionNum; i++)
    {
        section_hdr = (CSOT_Section_Header *)&phdr->section_header[i * sizeof(CSOT_Section_Header)];
        if ((CSOT_SECTION_TYPE_LUT_SINGLE == section_hdr->type_1)
            || (CSOT_SECTION_TYPE_LUT_LEFT == section_hdr->type_1))
        {
            phdr->TABLE_CRC[0] = section_hdr->header_section_CRC;
        }
        else if (CSOT_SECTION_TYPE_LUT_RIGHT == section_hdr->type_1)
        {
            phdr->TABLE_CRC[1] = section_hdr->header_section_CRC;
        }
    }
    UBOOT_TRACE("OK\n");
    return TRUE;
}

MS_BOOL load_vendor_lut_csot_csot(CSOT_CSOT_Demura_Header *phdr)
{
    CSOT_File_Header *_file_header = (CSOT_File_Header *)phdr->file_header;
    CSOT_Section_Header *_section_header = (CSOT_Section_Header *)phdr->section_header;
    MS_U8 i, dfree_i;

    for (i = 1; i < _file_header->sectionNum; i++)     //section 2 and 3 is lut table
    {
        phdr->lut_buffer[i-1] = (MS_U8 *)dmalloc(_section_header[i].header_size);
        if (phdr->lut_buffer[i-1] == NULL)
        {
            UBOOT_ERROR("malloc lut_buffer 0x%x byte failed!\n", _section_header[i].header_size);
            for (dfree_i = 1; dfree_i < i; dfree_i++)
            {
                dfree(phdr->lut_buffer[dfree_i-1]);
                phdr->lut_buffer[dfree_i-1] = NULL;
            }
            return FALSE;
        }
        if (read_spi_flash(phdr->lut_buffer[i-1], _section_header[i].header_offser
            , _section_header[i].header_size) == FALSE)
        {
            UBOOT_ERROR("Read CSOT_CSOT Lut_Header from spi flash failed !\n");
            for (dfree_i = 1; dfree_i <= i; dfree_i++)
            {
                dfree(phdr->lut_buffer[dfree_i-1]);
                phdr->lut_buffer[dfree_i-1] = NULL;
            }
            return FALSE;
        }

        if (CSOT_CSOT_CalcCRC16(phdr->lut_buffer[i-1], _section_header[i].header_size)
            != _section_header[i].header_section_CRC)
        {
            for (dfree_i = 1; dfree_i <= i; dfree_i++)
            {
                dfree(phdr->lut_buffer[dfree_i-1]);
                phdr->lut_buffer[dfree_i-1] = NULL;
            }
            UBOOT_DEBUG("=====CSOT_CSOT demura Lut table%d crc CRC failed  %x != %x =====\n", i
                , CSOT_CSOT_CalcCRC16(phdr->lut_buffer[i-1], _section_header[i].header_size)
                , _section_header[i].header_section_CRC);
            return FALSE;
        }
        UBOOT_DEBUG("=====CSOT_CSOT demura Lut table%d crc CRC OK %x=====\n", i
            , _section_header[i].header_section_CRC);
    }
    return TRUE;
}

MS_BOOL load_vendor_header_boe_eswin(BOE_ESWIN_Demura_Header *phdr)
{
    MS_U8 *buffer = (MS_U8 *)dmalloc(BOE_ESWIN_HEADER_LEN);

    if (buffer == NULL)
    {
        UBOOT_ERROR("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, (unsigned int)BOE_ESWIN_HEADER_LEN);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, BOE_ESWIN_HEADER_START, BOE_ESWIN_HEADER_LEN);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX Header from spi flash failed !\n");
        dfree(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}

MS_U32 BOE_ESWIN_Checksum_CAL(MS_U8 *pData, MS_U32 length, MS_BOOL bBigEdian)
{
    MS_U32 i;
    MS_U32 crc = 0, data = 0;

    if (bBigEdian)
    {
        for (i = 0; i < length; i+=4)
        {
            data = ((MS_U32)pData[i + 3] << 24) | ((MS_U32)pData[i + 2] << 16)
                | ((MS_U32)pData[i + 1] << 8) | pData[i];

            crc += data;
        }
    }
    else
    {
        for (i = 0; i < length; i+=4)
        {
            data = ((MS_U32)pData[i] << 24) | ((MS_U32)pData[i + 1] << 16)
                | ((MS_U32)pData[i + 2] << 8) | pData[i + 3];

            crc += data;
        }
    }
    return crc;
}

static void BOE_ESWIN_GetSegmentInfo(MS_U8 rgb, MS_U8 layernum, MS_U16 hlutnum,
    MS_U16 vlutnum, MS_U32 *segnum, MS_U32 *segSum, MS_U32 *alignsize)
{
    MS_U32 rgbAlign = (rgb == 0)? 1 : 3;
    MS_U32 totalLayer = (MS_U32)(layernum?layernum:1) * rgbAlign;
    MS_U32 blockNumPerline = totalLayer * hlutnum;

    MS_U8 decPnum = (totalLayer > 4)?4:totalLayer;
    MS_U32 segMinDen = ((((1 << 13) - 1) / ((vlutnum)?(vlutnum):1)) - 1);
    MS_U32 segMinSize = ((MS_U32)hlutnum * totalLayer - 1)
        / (segMinDen?segMinDen:1);
    MS_U32 segSizeMulti = (segMinSize + (totalLayer * decPnum - 1))
        / (totalLayer * decPnum);

    *segnum = totalLayer * decPnum * segSizeMulti;
    if ((((*segnum) * 10 + 127) / 128) == 17)
    {
        segSizeMulti++;
        *segnum = totalLayer * decPnum * segSizeMulti;
    }
    *segSum = (blockNumPerline + (*segnum) - 1) / (*segnum);
    *alignsize = (((*segnum) * 10 + 127) >> 7) << (7 - 3);
}

MS_BOOL parse_vendor_header_boe_eswin(BOE_ESWIN_Demura_Header *phdr)
{
    MS_U8  *buffer = phdr->hdr_buffer;
    MS_U8  *header_sec = NULL;
    MS_U32 crc = 0;

    phdr->hdr_buffer = buffer;
    phdr->lut_buffer = NULL;
    phdr->lut_offset = BOE_ESWIN_LUT_START;
    phdr->lut_length = 0;
    BOE_ESWIN_HEADER_U32(phdr->u32MagicNum, buffer, 0);
    memcpy(phdr->name, &buffer[4], sizeof(phdr->name));
    BOE_ESWIN_HEADER_U16(phdr->version, buffer, 18);
    BOE_ESWIN_HEADER_U16(phdr->sectionNum, buffer, 20);
    BOE_ESWIN_HEADER_U16(phdr->sectionCrc, buffer, 22);
    BOE_ESWIN_HEADER_U16(phdr->headerCrc, buffer, 30);
    BOE_ESWIN_HEADER_U32(phdr->header[0].type, buffer, 32);
    BOE_ESWIN_HEADER_U32(phdr->header[0].offset, buffer, 36);
    BOE_ESWIN_HEADER_U32(phdr->header[0].size, buffer, 40);
    BOE_ESWIN_HEADER_U32(phdr->header[0].crc, buffer, 44);
    BOE_ESWIN_HEADER_U32(phdr->header[1].type, buffer, 48);
    BOE_ESWIN_HEADER_U32(phdr->header[1].offset, buffer, 52);
    BOE_ESWIN_HEADER_U32(phdr->header[1].size, buffer, 56);
    BOE_ESWIN_HEADER_U32(phdr->header[1].crc, buffer, 60);
    phdr->enDemuraMod = buffer[64];
    phdr->u8PlaneNum = buffer[65];
    phdr->u8HblockSize = buffer[66];
    phdr->u8VblockSize = buffer[67];
    BOE_ESWIN_HEADER_U16(phdr->u16HLutNum, buffer, 68);
    BOE_ESWIN_HEADER_U16(phdr->u16VLutNum, buffer, 70);
    phdr->u8twoChipEn = buffer[72];
    phdr->u8DataFormate = buffer[73];
    BOE_ESWIN_HEADER_U16(phdr->u16BlackLimit, buffer, 76);
    BOE_ESWIN_HEADER_U16(phdr->u16WhiteLimit, buffer, 78);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[0], buffer, 80);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[1], buffer, 82);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[2], buffer, 84);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[3], buffer, 86);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[4], buffer, 88);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[5], buffer, 90);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[6], buffer, 92);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[7], buffer, 94);
    BOE_ESWIN_HEADER_U16(phdr->u16PlaneLevel[8], buffer, 96);
#if DGB_DEMURA_VENDOR_LOG
    UBOOT_TRACE("=====BOE ESWIN HEADER CRC OK=====\n");
    UBOOT_TRACE("magic num = %x\n", phdr->u32MagicNum);
    UBOOT_TRACE("name = %s\n", phdr->name);
    UBOOT_TRACE("version = %x\n", phdr->version);
    UBOOT_TRACE("sectionNum = %x\n", phdr->sectionNum);
    UBOOT_TRACE("sectionCrc = %x\n", phdr->sectionCrc);
    UBOOT_TRACE("headerCrc = %x\n", phdr->headerCrc);
    UBOOT_TRACE("header[0].type = %x\n", phdr->header[0].type);
    UBOOT_TRACE("header[0].offset = %x\n", phdr->header[0].offset);
    UBOOT_TRACE("header[0].size = %x\n", phdr->header[0].size);
    UBOOT_TRACE("header[0].crc = %x\n", phdr->header[0].crc);
    UBOOT_TRACE("header[1].type = %x\n", phdr->header[1].type);
    UBOOT_TRACE("header[1].offset = %x\n", phdr->header[1].offset);
    UBOOT_TRACE("header[1].size = %x\n", phdr->header[1].size);
    UBOOT_TRACE("header[1].crc = %x\n", phdr->header[1].crc);
    UBOOT_TRACE("enDemuraMod = %x\n", phdr->enDemuraMod);
    UBOOT_TRACE("u8PlaneNum = %x\n", phdr->u8PlaneNum);
    UBOOT_TRACE("u8HblockSize = %x\n", phdr->u8HblockSize);
    UBOOT_TRACE("u8VblockSize = %x\n", phdr->u8VblockSize);
    UBOOT_TRACE("HLutNum = %x\n", phdr->u16HLutNum);
    UBOOT_TRACE("VLutNum = %x\n", phdr->u16VLutNum);
    UBOOT_TRACE("u8twoChipEn = %x\n", phdr->u8twoChipEn);
    UBOOT_TRACE("u8DataFormate = %x\n", phdr->u8DataFormate);
    UBOOT_TRACE("u16BlackLimit = %x\n", phdr->u16BlackLimit);
    UBOOT_TRACE("u16WhiteLimit= %x\n", phdr->u16WhiteLimit);
    UBOOT_TRACE("u16PlaneLevel[0]= %x\n", phdr->u16PlaneLevel[0]);
    UBOOT_TRACE("u16PlaneLevel[1]= %x\n", phdr->u16PlaneLevel[1]);
    UBOOT_TRACE("u16PlaneLevel[2]= %x\n", phdr->u16PlaneLevel[2]);
    UBOOT_TRACE("u16PlaneLevel[3]= %x\n", phdr->u16PlaneLevel[3]);
    UBOOT_TRACE("u16PlaneLevel[4]= %x\n", phdr->u16PlaneLevel[4]);
    UBOOT_TRACE("u16PlaneLevel[5]= %x\n", phdr->u16PlaneLevel[5]);
    UBOOT_TRACE("u16PlaneLevel[6]= %x\n", phdr->u16PlaneLevel[6]);
    UBOOT_TRACE("u16PlaneLevel[7]= %x\n", phdr->u16PlaneLevel[7]);
    UBOOT_TRACE("u16PlaneLevel[8]= %x\n", phdr->u16PlaneLevel[8]);
#endif
    // check header crc
    crc = CSOT_CSOT_CalcCRC16(buffer, 30);
    if (crc != phdr->headerCrc)
    {
        UBOOT_ERROR("BOE ESWIN HEADER CRC Error= 0x%x 0x%x\n", (unsigned int)crc, phdr->headerCrc);
        dfree(buffer);
        return FALSE;
    }

    // check section header1 checksum
    if (phdr->header[0].type == 1)
    {
        if ((phdr->header[0].offset >= BOE_ESWIN_HEADER_LEN)
            || (phdr->header[0].size >= BOE_ESWIN_HEADER_LEN))
        {
            UBOOT_ERROR("BOE ESWIN HEADER1 Invalid Range %u %u\n", phdr->header[0].offset,
                phdr->header[0].size);
            dfree(buffer);
            return FALSE;
        }
        header_sec = &buffer[phdr->header[0].offset];
        phdr->lut_length = phdr->header[0].size;
        crc = BOE_ESWIN_Checksum_CAL(header_sec, phdr->header[0].size, FALSE);
        if (phdr->header[0].crc != crc)
        {
            UBOOT_ERROR("BOE ESWIN HEADER1 Checksum FAIL 0x%x 0x%x\n", crc, phdr->header[0].crc);
            dfree(buffer);
            return FALSE;
        }
    }

    BOE_ESWIN_GetSegmentInfo(phdr->enDemuraMod, phdr->u8PlaneNum, phdr->u16HLutNum,
        phdr->u16VLutNum, &phdr->u32SegNum, &phdr->u32SegsPerLine, &phdr->u32SegAlignedBytes);
    phdr->u32BytesPerLine = phdr->u32SegsPerLine * phdr->u32SegAlignedBytes;
#if DGB_DEMURA_VENDOR_LOG
    UBOOT_TRACE("BytesPerLinep = %u\n", phdr->u32BytesPerLine);
    UBOOT_TRACE("SegNum = %u\n", phdr->u32SegNum);
    UBOOT_TRACE("SegsPerLine = %u\n", phdr->u32SegsPerLine);
    UBOOT_TRACE("u32SegAlignedBytes = %u\n", phdr->u32SegAlignedBytes);
#endif
    return TRUE;
}

MS_BOOL load_vendor_lut_boe_eswin(BOE_ESWIN_Demura_Header *phdr)
{
    MS_U32 lut_size;
    MS_U32 crc = 0;
    MS_U8  *lut_buf = NULL;
    if (phdr->header[1].type == 2)
    {
        lut_size = phdr->header[1].size;
        phdr->lut_length = phdr->header[1].size;
        lut_buf = dmalloc(lut_size);
        CHECK_DMALLOC_SPACE(lut_buf, (unsigned int)lut_size);
        if (read_spi_flash(lut_buf, BOE_ESWIN_LUT_START, lut_size) == FALSE)
        {
            UBOOT_ERROR("Read BOE ESWIN Lut from spi flash failed !\n");
            dfree(lut_buf);
            return FALSE;
        }

        crc = BOE_ESWIN_Checksum_CAL(lut_buf, lut_size , FALSE);
        if(crc != phdr->header[1].crc)
        {
            UBOOT_ERROR("BOE ESWIN LUT CRC FAIL %x %x\n", crc, phdr->header[1].crc);
            dfree(lut_buf);
            return FALSE;
        }
        UBOOT_TRACE("=====ESWIN LUT CRC OK=====\n");
        phdr->lut_buffer = lut_buf;
        return TRUE;
    }
    return FALSE;
}
