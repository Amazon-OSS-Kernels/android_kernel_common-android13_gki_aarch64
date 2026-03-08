// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

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

MS_U16 get_lut_checksum(void)
{
    MS_U8   buffer[2];
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

MS_BOOL load_vendor_header(CSOT_Himax_Demura_Header *phdr)
{
    MS_U8 read_len = CSOT_HIMAX_HEADER_LEN ;
    MS_U8 *buffer = (MS_U8 *)malloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, CSOT_HIMAX_HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(CSOT_Himax_Demura_Header *phdr)
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
    printf("HIMAX HEADER CRC = 0x%x \n", crc);
    if(crc != phdr->HEADER_CRC)
    {
        printf("=====HIMAX HEADER CRC FAIL=====\n");
        free(buffer);
        return FALSE;
    }
    else
        printf("=====HIMAX HEADER CRC OK=====\n");
    
    return TRUE;
}

MS_BOOL load_vendor_lut(CSOT_Himax_Demura_Header *phdr)
{
    MS_U32 lut_size   = phdr->lut_length;
    MS_U16 crc = 0, lut_checksum = phdr->TABLE_CRC;
    MS_U8  *lut_buf = dmalloc(lut_size);
    CHECK_DMALLOC_SPACE(lut_buf, lut_size);

    MS_BOOL bRet = read_spi_flash(lut_buf, CSOT_HIMAX_LUT_START, lut_size);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HIMAX Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    crc = HIMAX_CRC_CAL(lut_buf, lut_size, FALSE);
    printf("HIMAX LUT CRC = 0x%x \n", crc);
    if(crc != lut_checksum)
    {
        printf("=====HIMAX LUT CRC FAIL=====\n");
        dfree(lut_buf);
        return FALSE;
    }
    else
        printf("=====HIMAX LUT CRC OK=====\n");

    phdr->lut_buffer = lut_buf;
    return TRUE;
}

