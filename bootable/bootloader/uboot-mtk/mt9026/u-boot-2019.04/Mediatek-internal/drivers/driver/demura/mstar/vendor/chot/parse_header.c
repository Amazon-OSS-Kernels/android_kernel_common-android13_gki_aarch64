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


static int do_dmc_checksum(MS_U16 CHOT_CRC, MS_U8 *buf, MS_U32 start, MS_U32 end)
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

    printf("CHOT_CRC = 0x%x, crc = 0x%x, checksum = %d \n", CHOT_CRC, crc, (int)checksum);

    if(crc == CHOT_CRC) // CRC correct
        return 0;
    else
        return -1;
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

    checksum = ((MS_U16)buffer[0] << 8) | buffer[1];
    return checksum;
}

MS_BOOL load_vendor_header(CHOT_Demura_Header *phdr)
{
    MS_U8 *buffer = (MS_U8 *)malloc(HEADER_LEN);
    if (buffer == NULL)
    {
        printf("In %s, malloc 0x%x byte failed!\n", __FUNCTION__, HEADER_LEN);
        return FALSE;
    }

/*
    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        free(buffer);
        return FALSE;
    }
*/

    MS_BOOL bRet = read_spi_flash(buffer, HEADER_START, HEADER_LEN);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    return TRUE;
}


MS_BOOL parse_vendor_header(CHOT_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    CHOT_BIN_Header *pbhdr = (CHOT_BIN_Header *)buffer;

    phdr->PARAMETER_CRC    =  (buffer[0] << 8) | buffer[1];

    int verify_start  =  DMC_HEADER_START - HEADER_START;
    int verify_end    =  DMC_HEADER_END - HEADER_START;
    if (do_dmc_checksum(phdr->PARAMETER_CRC, buffer, verify_start, verify_end) != 0)
    {
        printf("verify dmc header checksum failed !\n");
        free(buffer);
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
    phdr->TBL_END_ADDR = phdr->DEMURA_TABLE_LENGTH + DMC_LUT_START - 1;

    return TRUE;
}


void dump_vendor_header(CHOT_Demura_Header *phdr)
{
    MS_U8 i;

    printf("PARAMETER_CRC       :   0x%x \n", phdr->PARAMETER_CRC);
    printf("DEMURA_PLANE_NUM    :   0x%x \n", phdr->DEMURA_PLANE_NUM);
    printf("DEMURA_TBL_H        :   0x%x \n", phdr->DEMURA_TBL_H);
    printf("DEMURA_TBL_V        :   0x%x \n", phdr->DEMURA_TBL_V);
    printf("DEMURA_BLK_H        :   0x%x \n", phdr->DEMURA_BLK_H);
    printf("DEMURA_BLK_V        :   0x%x \n", phdr->DEMURA_BLK_V);
    printf("LOWER_BOUND         :   0x%x \n", phdr->LOWER_BOUND);
    printf("UPPER_BOUND         :   0x%x \n", phdr->UPPER_BOUND);
    printf("PANEL_LEVEL         :   ");
    for(i = 0; i < phdr->DEMURA_PLANE_NUM; i++)
        printf("PLANE[%d] = 0x%x  ", i, phdr->PLANE[i]);

    printf("\n");
    printf("TBL_END_ADDR        :   %u \n", (unsigned int)phdr->TBL_END_ADDR);
    printf("TABLE_CRC           :   0x%x \n", phdr->TABLE_CRC);
    printf("DEMURA_TABLE_LENGTH :   %u \n", (unsigned int)phdr->DEMURA_TABLE_LENGTH);
}


MS_U32 cal_dmc_lut_size(CHOT_Demura_Header *phdr)
{
    // TBL_LEN = ceil(DEMURA_TBL_H * DEMURA_TBL_V * 12 * DEMURA_PLANE_NUM / 256) * 32
     MS_U32 u32_temp = 0, lut_size;
     u32_temp = phdr->DEMURA_TBL_H * phdr->DEMURA_TBL_V * 12 * phdr->DEMURA_PLANE_NUM;
     if((u32_temp & 0xff) != 0x00) // round
         lut_size = (u32_temp >> 8) + 1;
     else
         lut_size = (u32_temp >> 8);

    return lut_size;
}


MS_BOOL load_vendor_lut(CHOT_Demura_Header *phdr)
{
    MS_U32 lut_size   = phdr->DEMURA_TABLE_LENGTH; //cal_dmc_lut_size(phdr);
    MS_U32 lut_offset = DMC_LUT_START - DMC_LUT_CHECKSUM;
    MS_U32 lut_cs_len = lut_size + lut_offset;
    //MS_U32 lut_end = DMC_LUT_START + lut_cs_len - 1;

    MS_U8  *lut_buf   = dmalloc(lut_cs_len);
    CHECK_DMALLOC_SPACE(lut_buf, lut_cs_len);

    printf("lut_cs_len = %d, lut_size = %d, lut_offset = %d \n",
    lut_cs_len, lut_size, lut_offset);

    MS_BOOL bRet = read_spi_flash(lut_buf, DMC_LUT_CHECKSUM, lut_cs_len);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read CHOT Lut from spi flash failed !\n");
        dfree(lut_buf);
        return FALSE;
    }

    MS_U16 checksum   = (lut_buf[0] << 8) | lut_buf[1];
    phdr->TABLE_CRC = checksum;

    int verify_start  =  DMC_LUT_START - DMC_LUT_CHECKSUM;
    int verify_end    =  verify_start + lut_size - 1;

    printf("verify_start = %d, verify_end = %d \n", verify_start, verify_end);

#if(0)
    int i = 0;
    for(i = 0; i < 160; i++)
    {
    printf("lut_buf[%d] = 0x%x ", i, lut_buf[verify_start+14+i]);
        if((i%4==0)&& (i!=0))
            printf("\n");
    }
    printf("====================\n");

    for(i = 0; i<16;i++)
    {
    printf("lut_buf[%d] = 0x%x ", i, lut_buf[verify_end-18-i]);
        if((i==4)||(i==8)||(i==12))
            printf("\n");
    }
    printf("\n");
#endif

    if (do_dmc_checksum(checksum, lut_buf, verify_start, verify_end) != 0)
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

