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
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
E_COLOR_MODE Demura_Original_Mode =0;

static int do_dmc_checksum(MS_U16 checksum, MS_U8 *buf, MS_U32 start, MS_U32 end)
{
    MS_U32 i = 0;
    MS_U32 cal_sum = 0; // Don't care checksum overflow. For "MOD(SUM(d[65538], d[65694]), 1<<16)"

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


MS_BOOL load_vendor_header(AUO_Demura_Header *phdr)
{
    #define HEADER_START   (DMC_HEADER_CHECKSUM)
    #define HEADER_LEN     (DMC_HEADER_END   - DMC_HEADER_CHECKSUM)

    MS_U8 *buffer = (MS_U8 *)malloc(HEADER_LEN);

    UBOOT_DEBUG("IN\n");
    if (buffer == NULL)
    {
        UBOOT_ERROR("malloc 0x%x byte failed!\n", HEADER_LEN);
        return FALSE;
    }

    if (init_spi_flash() != TRUE)
    {
        UBOOT_DEBUG("init_spi_flash error!\n");
        free(buffer);
        return FALSE;
    }

    MS_BOOL bRet = read_spi_flash(buffer, HEADER_START, HEADER_LEN);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Read AUO_DMC_Header from spi flash failed !\n");
        free(buffer);
        return FALSE;
    }

    phdr->hdr_buffer = buffer;
    UBOOT_DEBUG("OK\n");
    return TRUE;
}


MS_BOOL parse_vendor_header(AUO_Demura_Header *phdr)
{
    MS_U8  *buffer        = phdr->hdr_buffer;
    AUO_BIN_Header *pbhdr = (AUO_BIN_Header *)buffer;

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
        free(buffer);
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
        printf(" 0x%x", phdr->panel_levels[i]);
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

    if(Demura_Original_Mode == E_RGB_MODE)
    {
        MS_U32 i,n;
        MS_U32 lut_mono_size;
        lut_mono_size = phdr->dmc_panel_num * phdr->dmc_h_lut_num * phdr->dmc_v_lut_num + lut_offset ;
        for(i = 2; i < lut_mono_size; i++)
        {
            n = 3 * ( i - 1 );
            lut_buf[i] = lut_buf[n];
        }
    }

    phdr->lut_buffer   = lut_buf;
    phdr->lut_offset   = lut_offset;
    phdr->lut_length   = lut_size;
    phdr->lut_checksum = checksum;
    return TRUE;
}

