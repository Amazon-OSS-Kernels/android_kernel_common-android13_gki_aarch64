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

MS_U16 CRC_CAL(MS_U16 crc, int num, MS_U32 *addr)
{
    int i,j;
    MS_U32 data_int, data_temp;
    MS_U16 data_short;
    MS_U8 data_char;

    /* Loop until num = 0 */
    while(num--)
    {
        /* Fetch one 4-bytes data drom memory */
        data_int = *addr;

        for(j = 4; j > 0; j--)
        {
            /* Fetch one byte each time from [31:24] to [7:0] */
            data_temp = data_int >> 8 * (j-1);
            data_char = data_temp;
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

    bRet = read_spi_flash(buffer, DMC_LUT_CHECKSUM, sizeof(buffer));
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT_Header from spi flash failed !\n");
        return checksum;
    }

    checksum = ((MS_U16)buffer[1] << 8) | buffer[0];
    return checksum;
}

MS_BOOL load_vendor_header(CSOT_Hisilicon_Demura_Header *phdr)
{
    MS_U8 read_len = HEADER_LEN ;
    MS_U8 *buffer = (MS_U8 *)malloc(read_len);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, read_len);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, HEADER_START, read_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CSOT_HISILICON Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(CSOT_Hisilicon_Demura_Header *phdr)
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

void dump_vendor_header(CSOT_Hisilicon_Demura_Header *phdr)
{
    MS_U8 i;

    printf("DEMURA_PLANE_NUM    :   0x%x \n", phdr->DEMURA_PLANE_NUM);
    printf("DEMURA_MODE         :   0x%x \n", phdr->DEMURA_MODE);
    printf("DEMURA_TBL_H        :   0x%x \n", phdr->DEMURA_TBL_H);
    printf("DEMURA_TBL_V        :   0x%x \n", phdr->DEMURA_TBL_V);
    printf("DEMURA_BLK_H        :   0x%x \n", phdr->DEMURA_BLK_H);
    printf("DEMURA_BLK_V        :   0x%x \n", phdr->DEMURA_BLK_V);
    printf("TABLE_CRC           :   0x%x \n", phdr->TABLE_CRC);
    printf("DEMURA_TABLE_LENGTH :   %u \n", (unsigned int)phdr->lut_length);
}

MS_BOOL load_vendor_lut(CSOT_Hisilicon_Demura_Header *phdr)
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
    CHECK_DMALLOC_SPACE(lut_buf, lut_size_32);
    for(i = 0; i < (32 - (lut_size%32)); i++)
        lut_buf[0x9B9D0 + i] = 0;

    MS_BOOL bRet = read_spi_flash(lut_buf, LUT_START, lut_size);
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

