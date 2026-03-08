// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <common.h>
#include <stdio.h>
#include <debug_impl.h>
#include <MsTypes.h>
//#include <ShareType.h>
#include <malloc.h>

#include <CommonDataType.h>
#include <dmalloc.h>
#include <read_text.h>
#include <demura.h>
#include <ms_utils.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include <convert_entry.h>
#include "vendor.h"
#include "crc_libs.h"
#include "demura_config.h"

#define SAMPLE_DAT_START    1024     // Byte
#define SAMPLE_DAT_LEN      4096     // Byte
#define FLASH_MAX_SIZE   (1024 * 1024)

typedef union
{
    MS_U32 block_data;
    unsigned char data[4];

    struct
    {
        int nReserved:8;
        int b3:6;
        int b2:6;
        int b1:6;
        int b0:6;
    };
}DATA;

static MS_BOOL Gen_Sf_Signature(CHOT_Demura_Header *phdr)
{
    char   strbuf[1024];
    MS_U32 sample_crc32;
    MS_U8  *buffer;

    buffer = phdr->lut_buffer + phdr->lut_offset;

    sample_crc32 = MDrv_CRC32_Cal_DeMura(buffer + SAMPLE_DAT_START, SAMPLE_DAT_LEN);
    if (sample_crc32 == 0xffffFFFF)
    {
        UBOOT_ERROR("MDrv_CRC32_Cal_DeMura error\n");
        return FALSE;
    }
    memset(strbuf, 0, sizeof(strbuf));
    snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", phdr->TABLE_CRC, sample_crc32);
    env_set(ENV_DEMURA_SIG, strbuf);
    env_save();
    return TRUE;
}


MS_BOOL If_Need_Decode(void)
{
    UBOOT_TRACE("IN\n");

    char *sig_str = env_get(ENV_DEMURA_SIG);
    if (sig_str == NULL)
    {
        UBOOT_DEBUG("Empty Board, should decoding data\n");
        UBOOT_TRACE("OK\n");
        return TRUE;
    }
    else
    {
        MS_U16 lut_checksum_sf;
        MS_U32 lut_checksum_bd;
        MS_U32 sample_crc32, cal_crc32;
        MS_U8  buf[SAMPLE_DAT_LEN];

        lut_checksum_sf = get_lut_checksum();

        UBOOT_DEBUG("sig_str = %s\n", sig_str);
        if(2 == sscanf(sig_str, "0x%x:0x%x", &lut_checksum_bd, &sample_crc32))
        {
            UBOOT_DEBUG("lut_checksum_sf = 0x%x\n", (uint)lut_checksum_sf);
            UBOOT_DEBUG("lut_checksum_bd = 0x%x\n", (uint)lut_checksum_bd);
            UBOOT_DEBUG("sample_crc32  = 0x%x\n",   (uint)sample_crc32);

            if (read_spi_flash(buf, (DMC_LUT_START + SAMPLE_DAT_START), SAMPLE_DAT_LEN) == TRUE)
            {
                cal_crc32 = MDrv_CRC32_Cal_DeMura(buf, SAMPLE_DAT_LEN);
                UBOOT_DEBUG("cal_crc32     = 0x%x\n", (uint)cal_crc32);
                if ((sample_crc32 == cal_crc32) && (lut_checksum_sf == lut_checksum_bd))
                {
                    UBOOT_DEBUG("Data Match, No Need to decode again\n");
                    UBOOT_TRACE("OK\n");
                    return FALSE;
                }
            }
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

static void dump_interface_info(interface_info *pDataInfo)
{
    printf("PLANE_NUM = %d  \n", pDataInfo->reg_dmc_plane_num);
    printf("H_BLOCK = %d, V_BLOCK = %d  \n", pDataInfo->reg_dmc_h_block, pDataInfo->reg_dmc_v_block);
    printf("RGB_MODE = %d   \n", pDataInfo->reg_dmc_rgb_mode);
    printf("PANEL_H_SIZE = %d   \n", pDataInfo->reg_dmc_panel_h_size);
    printf("BLACK_LIMIT = 0x%x, WHITE_LIMIT = 0x%x  \n", pDataInfo->reg_dmc_black_limit, pDataInfo->reg_dmc_white_limit);

    printf("PLANE_LEVEL 1 = 0x%x   \n", pDataInfo->reg_dmc_plane_level1);
    printf("PLANE_LEVEL 2 = 0x%x   \n", pDataInfo->reg_dmc_plane_level2);
    printf("PLANE_LEVEL 3 = 0x%x   \n", pDataInfo->reg_dmc_plane_level3);
    printf("PLANE_LEVEL 4 = 0x%x   \n", pDataInfo->reg_dmc_plane_level4);
    printf("PLANE_LEVEL 5 = 0x%x   \n", pDataInfo->reg_dmc_plane_level5);
    printf("PLANE_LEVEL 6 = 0x%x   \n", pDataInfo->reg_dmc_plane_level6);
    printf("PLANE_LEVEL 7 = 0x%x   \n", pDataInfo->reg_dmc_plane_level7);
    printf("PLANE_LEVEL 8 = 0x%x   \n", pDataInfo->reg_dmc_plane_level8);

    printf("DITHER_4_FRAME = %d, DITHER_RST_NUM = %d, DITHER_EN = %d    \n",
        pDataInfo->reg_dmc_dither_rst_e_4_frame, pDataInfo->reg_dmc_dither_pse_rst_num, pDataInfo->reg_dmc_dither_en);

    printf("ROI_EN = %d, ROI_H_START = %d, ROI_H_END = %d, ROI_V_START = %d, ROI_V_END = %d \n",
        pDataInfo->bROI_en, pDataInfo->iROI_hor_start_offset, pDataInfo->iROI_hor_end_offset,
        pDataInfo->iROI_ver_start_offset, pDataInfo->iROI_ver_end_offset);
}

static void init_morton_data(CHOT_Demura_Header *phdr, interface_info *pDataInfo)
{
    printf("============================================\n");
    pDataInfo->reg_dmc_rgb_mode               = 0;           //  1 bit
    pDataInfo->reg_dmc_panel_h_size           = 0xF00;                     // 13 bit
    //pDataInfo->reg_dmc_black_limit            = 0x100;                     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level1           = phdr->PLANE[0] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level2           = phdr->PLANE[1] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level3           = phdr->PLANE[2] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level4           = phdr->PLANE[3] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level5           = phdr->PLANE[4] << 4;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level6           = 0;     // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level7           = 0;           // 12 bit , format 10.2
    //pDataInfo->reg_dmc_plane_level8           = 0;           // 12 bit , format 10.2
    //pDataInfo->reg_dmc_white_limit            = 0xEF0;       // 12 bit , format 10.2
    pDataInfo->reg_dmc_dither_en              = 0x1;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    pDataInfo->reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    pDataInfo->reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    pDataInfo->bROI_en                        = 0x0;
    pDataInfo->iROI_hor_end_offset            = 0x0;
    pDataInfo->iROI_hor_start_offset          = 0x0;
    pDataInfo->iROI_ver_end_offset            = 0x0;
    pDataInfo->iROI_ver_start_offset          = 0x0;

    // Update CHOT demura header to mstar interface info
    pDataInfo->reg_dmc_plane_num = phdr->DEMURA_PLANE_NUM;
    pDataInfo->reg_dmc_black_limit = phdr->LOWER_BOUND<<2;
    pDataInfo->reg_dmc_plane_level1 = phdr->PLANE[0]<<2;
    pDataInfo->reg_dmc_plane_level2 = phdr->PLANE[1]<<2;
    pDataInfo->reg_dmc_plane_level3 = phdr->PLANE[2]<<2;
    pDataInfo->reg_dmc_plane_level4 = phdr->PLANE[3]<<2;
    pDataInfo->reg_dmc_plane_level5 = phdr->PLANE[4]<<2;
    pDataInfo->reg_dmc_plane_level6 = phdr->PLANE[5]<<2;
    pDataInfo->reg_dmc_plane_level7 = phdr->PLANE[6]<<2;
    pDataInfo->reg_dmc_plane_level8 = phdr->PLANE[7]<<2;
    pDataInfo->reg_dmc_white_limit = phdr->UPPER_BOUND<<2;

    switch(phdr->DEMURA_BLK_H)  // 0: 4x, 1: 8x, 2: 16x customer N -> mstar 2^n
    {
        case 0:
            pDataInfo->reg_dmc_h_block = 2;
            break;
        case 1:
            pDataInfo->reg_dmc_h_block = 3;
            break;
        case 2:
            pDataInfo->reg_dmc_h_block = 4;
            break;
        default:
            pDataInfo->reg_dmc_h_block = 3;
            break;
    }

    switch(phdr->DEMURA_BLK_V)  // 0: 4x, 1: 8x, 2: 16x customer N -> mstar 2^n
    {
        case 0:
            pDataInfo->reg_dmc_v_block = 2;
            break;
        case 1:
            pDataInfo->reg_dmc_v_block = 3;
            break;
        case 2:
            pDataInfo->reg_dmc_v_block = 4;
            break;
        default:
            pDataInfo->reg_dmc_v_block = 3;
            break;
    }
//=============================== End Demura Data Set //===============================
}

int Get_INDEX(int *array, MS_U8 array_num)
{
    int i = 0;
    int min = array[0];
    for(i = 0; i < array_num; i++)
    {
        if(array[i] < min)
            min = i;
    }
    return min;
}

static MS_BOOL CHOT_Decode_To_Mstar_Format(CHOT_Demura_Header *phdr, interface_info *pinfo)
{
    // init morton interface data
    init_morton_data(phdr, pinfo);

    //dump_interface_info(pinfo);

    int Lut_buffer[pinfo->reg_dmc_plane_num][phdr->DEMURA_TBL_V][phdr->DEMURA_TBL_H];
    int idx, i, j, k, data = 0;
    int table_addr = DMC_LUT_START - DMC_LUT_CHECKSUM; // Skip CRC
    MS_BOOL pol = 0;

    // assign LUT 12 bit format
    for (i = 0; i < phdr->DEMURA_TBL_V; i++)
    {
        for (j = 0; j < phdr->DEMURA_TBL_H ; j++)
        {
            for(k = 0; k < pinfo->reg_dmc_plane_num; k++)
            {
                if(pol == 0) //8bit + 4bit
                {
                    data = ((MS_U16)phdr->lut_buffer[table_addr]<<4) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xf0)>>4);
                    pol = 1;
                    table_addr+=1;
                }
                else
                {
                    data = (((MS_U16)phdr->lut_buffer[table_addr] & 0x0f)<<8) | (((MS_U16)phdr->lut_buffer[table_addr + 1]&0xff));
                    pol = 0;
                    table_addr+=2;
                }
                // Update 12bit buffer
                if(data >= 0x800)
                    data = data - 0x1000; // signed bit
                Lut_buffer[k][i][j] = data;
            }
        }
    }

    for (idx = 0; idx < pinfo->reg_dmc_plane_num; idx++)
    {
        int nLayer = 0;
        switch (idx)
        {
            case 0:
                nLayer = pinfo->reg_dmc_plane_level1;
                break;
            case 1:
                nLayer = pinfo->reg_dmc_plane_level2;
                break;
            case 2:
                nLayer = pinfo->reg_dmc_plane_level3;
                break;
            case 3:
                nLayer = pinfo->reg_dmc_plane_level4;
                break;
            case 4:
                nLayer = pinfo->reg_dmc_plane_level5;
                break;
            case 5:
                nLayer = pinfo->reg_dmc_plane_level6;
                break;
            case 6:
                nLayer = pinfo->reg_dmc_plane_level7;
                break;
            case 7:
                nLayer = pinfo->reg_dmc_plane_level8;
                break;
            default:
                break;
        }
        //int **layer_value_lut = Lut_buffer[idx];

        int n_Hnode     = phdr->DEMURA_TBL_H;       // 481;
        int n_Vnode     = phdr->DEMURA_TBL_V;       // 271;

        int idx_image_size = 0;
        double LUT_data = 0;

        //int cnt = 0;

        for (i = 0; i < n_Vnode; i++)
        {
            for (j = 0; j < n_Hnode ; j++)
            {
                LUT_data = (double)(((((Lut_buffer[idx][i][j]))/4) + (nLayer/4)));//(double)(((((Lut_buffer[idx][i][j])&0xFFF)>>2) + (nLayer>>2))&0x3FF); // 12bit to 10bit

                pinfo->Lut_in[idx][idx_image_size].dbr   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].dbg   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].dbb   = LUT_data;//(double)(Lut_buffer[idx][i][j] + nLayer);
        #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[idx][idx_image_size].dbw   = LUT_data;
        #endif

                //(pinfo->Lut_in[idx]+idx_image_size)->r   = (Lut_buffer[idx][i][j] + nLayer);//(pinfo->Lut_in[idx])->r   = (Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].r   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].g   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
                pinfo->Lut_in[idx][idx_image_size].b   = (int)LUT_data;//(Lut_buffer[idx][i][j] + nLayer);
        #if (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
                pinfo->Lut_in[idx][idx_image_size].w   = (int)LUT_data;
        #endif
                idx_image_size += 1;
            }
        }
    }

    // Free buffer
    free(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;

    return TRUE;
}




MS_BOOL Decode_To_Mstar_Format(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;
    CHOT_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header(&header) != TRUE)
    {
        printf("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header(&header) != TRUE)
    {
        printf("parse_vendor_header Error\n");
        return FALSE;
    }
    //dump_vendor_header(&header);

    if (load_vendor_lut(&header) != TRUE)
    {
        printf("load_vendor_lut Error\n");
        return FALSE;
    }

    dump_vendor_header(&header);
    Gen_Sf_Signature(&header);

    // Alloc space for Lut_in
    pbin_info->HNode       =  header.DEMURA_TBL_H;
    pbin_info->VNode       =  header.DEMURA_TBL_V;
    pbin_info->LevelCount  =  header.DEMURA_PLANE_NUM;

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        printf("Alloc_LutIn_Space error\n");
        return FALSE;
    }

    // Decode Vendor data
    bRet = CHOT_Decode_To_Mstar_Format(&header, (interface_info *)pDataInfo);
    if (bRet == FALSE)
    {
        printf("CHOT_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    #if(CONFIG_DEMURA_URSA_TYPE == 13)
    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};
    #else
    MS_BOOL demura_enable[4]  = {TRUE, TRUE, TRUE, TRUE};
    #endif

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + ID_CUS_MSTAR);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;
    pbin_info->Sep_type       =  0;
    pbin_info->Build_Date     =  0x19030614;

    memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}


