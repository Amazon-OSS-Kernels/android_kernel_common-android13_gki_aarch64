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
#include <ms_vector.h>
#include <read_text.h>
#include <ms_utils.h>
#include <demura.h>
#include <demura_common.h>
#include <demuraDll.h>

#include <apiPNL.h>
#include <MDrvDemura.h>
#include <halDemura.h>
#include <convert_entry.h>
#include "vendor.h"

#include "crc_libs.h"
#include "demura_config.h"

#define SAMPLE_DAT_START    1024     // Byte
#define SAMPLE_DAT_LEN      4096     // Byte

extern E_COLOR_MODE Demura_Original_Mode;

static MS_BOOL Gen_Sf_Signature(AUO_Demura_Header *phdr);

/*
#define AUO_CUS_DAT_LEN      65536
#define AUO_LUT_START_ADDR   131074


static inline char *copy_chars(char *dest, char *src, int begin, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        dest[i] = src[begin + i];
    }
    dest[len] = 0;
    return dest;
}


MS_BOOL demura_HexInput(char *HexFileCont, m_vector *pAUO_VECTOR)
{
    int  dat_num = 0;
    line_info  linfo;
    MS_BOOL  b_StartAddr = FALSE;
    unsigned char *AUO_IN_DATA = pAUO_VECTOR->pbuf;

    // For AUO_Customer data(size : 65536 byte), default is empty.
    memset(AUO_IN_DATA, 0, AUO_CUS_DAT_LEN);
    dat_num = AUO_CUS_DAT_LEN;

    // read file
    while (get_line(&linfo))
    {
        char temp[4];
        char *start = (char *)linfo.pcont_start;
        char *end   = start + linfo.len - 1;
        *end  = 0;
        //printf("%s   ", start);

        if (linfo.len > 1)
        {
            copy_chars(temp, start, 7, 2);
            //printf("%s\n", temp);

            //check start point
            if (b_StartAddr == FALSE)
            {
                if (strcmp(temp, "02") == 0)
                {
                    const char *loc = strstr(start, "020000021000EC");
                    if (loc == NULL)
                    {
                        continue;
                    }
                    else
                    {
                        b_StartAddr = TRUE;
                        printf("Get Start Point\n");
                    }
                }
            }

            if (b_StartAddr == TRUE)
            {
                if (strcmp(temp, "00") == 0)
                {
                    int idx;
                    copy_chars(temp, start, 1, 2);
                    int max_idx = simple_strtoul(temp, NULL, 16);

                    for (idx = 0 ; idx < max_idx ; idx++)
                    {
                        copy_chars(temp, start, 9+(idx*2), 2);
                        unsigned char data = simple_strtoul(temp, NULL, 16);
                        //sprintf(&data, "%c", );

                        AUO_IN_DATA[dat_num++] = data;
                        //printf("%d\n", data);
                    }
                }
                else if (strcmp(temp, "01") == 0)
                {
                    printf("Get End Point\n");
                    break;
                }
            }
        }
    }
    pAUO_VECTOR->dat_num = dat_num;
    printf("dat_num = 0x%x\n\n", dat_num);

    if (get_file_status() != 0)
    {
        return FALSE;
    }

    return TRUE;
}
*/

#if defined (CONFIG_DEMURA_URSA11)
static MS_BOOL check_dmc_parameter_error(int h_size, int v_size, char* error_msg)
{
    MS_BOOL dmc_parameter_error = FALSE;

    if(h_size==4)
    {
         dmc_parameter_error = TRUE;
         error_msg = "block_h_siz error : 4 pixel\n";
    }

    if(v_size==4)
    {
        dmc_parameter_error = TRUE;
        error_msg = "block_v_siz error : 4 pixel\n";        
    }
    
    return dmc_parameter_error;
}

static MS_BOOL set_u11_interface(AUO_Demura_Header *phdr, dmc_registers *pdmc_reg, void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL dmc_parameter_error_flag = FALSE;
    char *dmc_parameter_error_msg = "";

    interface_info *DataInfo   = (interface_info *)pDataInfo;
    interface_info *R_Info_Out = &(DataInfo[0]);
    interface_info *G_Info_Out = &(DataInfo[1]);
    interface_info *B_Info_Out = &(DataInfo[2]);

    DataInfo[0].bOut_dither_en = TRUE;
    DataInfo[1].bOut_dither_en = TRUE;
    DataInfo[2].bOut_dither_en = TRUE;

    //++++++++++++ LUT H/V Size start +++++++++++//
    int Mstar_lut_h_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_h_lut_num, 7, 4);
    int Mstar_lut_v_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_v_lut_num, 2, 8);
    //++++++++++++ LUT H/V Size end ++++++++++++//

    //++++++++++++ Block H/V Size start ++++++++++++//
    int Mstar_block_h_size       = 1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3));
    int Mstar_block_v_size       = 1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_v_block, 3));
    pbin_info->Blk_h_size        = Mstar_block_h_size;
    pbin_info->Blk_v_size        = Mstar_block_v_size;

    dmc_parameter_error_flag = check_dmc_parameter_error(Mstar_block_h_size, Mstar_block_v_size, dmc_parameter_error_msg);
    
    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    int Mstar_y_mode_en          = (register_08bits_inv( (*pdmc_reg).reg_dmc_rgb_mode, 1)==1) ? 0 : 1;
    pbin_info->Sep_type          = ( Mstar_y_mode_en == 1 ? 0 : 1);
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//

    (*R_Info_Out).iChannel = 0;
    (*G_Info_Out).iChannel = 1;
    (*B_Info_Out).iChannel = 2;

    //++++++++++++ layerX level enable start ++++++++++++//
    int layer_level_num      = register_08bits_inv( (*pdmc_reg).reg_dmc_plane_num, 4);

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=8)) ? layer_level_num : 3;

    int idx;

    //Fix CCN
    (*R_Info_Out).bLayer_level_en[0] = FALSE;
    (*G_Info_Out).bLayer_level_en[0] = FALSE;
    (*B_Info_Out).bLayer_level_en[0] = FALSE;
        
    (*R_Info_Out).bLayer_level_en[1] = FALSE;
    (*G_Info_Out).bLayer_level_en[1] = FALSE;
    (*B_Info_Out).bLayer_level_en[1] = FALSE;

    (*R_Info_Out).bLayer_level_en[2] = FALSE;
    (*G_Info_Out).bLayer_level_en[2] = FALSE;
    (*B_Info_Out).bLayer_level_en[2] = FALSE;

    (*R_Info_Out).bLayer_level_en[3] = FALSE;
    (*G_Info_Out).bLayer_level_en[3] = FALSE;
    (*B_Info_Out).bLayer_level_en[3] = FALSE;

    (*R_Info_Out).bLayer_level_en[4] = FALSE;
    (*G_Info_Out).bLayer_level_en[4] = FALSE;
    (*B_Info_Out).bLayer_level_en[4] = FALSE;

    (*R_Info_Out).bLayer_level_en[5] = FALSE;
    (*G_Info_Out).bLayer_level_en[5] = FALSE;
    (*B_Info_Out).bLayer_level_en[5] = FALSE;


    switch(layer_level_num)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            for (idx = 0; idx < layer_level_num + 2; idx++)
            {
                (*R_Info_Out).bLayer_level_en[idx] = TRUE;
                (*G_Info_Out).bLayer_level_en[idx] = TRUE;
                (*B_Info_Out).bLayer_level_en[idx] = TRUE;
            }
            break;
        default:
            dmc_parameter_error_flag = TRUE;
            dmc_parameter_error_msg = "layer_level_num : over 4 layer level\n";
            break;
    }

    if (dmc_parameter_error_flag == TRUE)
    {
        UBOOT_ERROR("%s", dmc_parameter_error_msg);
        return FALSE;
    }
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    (*R_Info_Out).iLayer_level[0] = 0;
    (*G_Info_Out).iLayer_level[0] = 0;
    (*B_Info_Out).iLayer_level[0] = 0;
            
    (*R_Info_Out).iLayer_level[1] = 1023;
    (*G_Info_Out).iLayer_level[1] = 1023;
    (*B_Info_Out).iLayer_level[1] = 1023;
    (*R_Info_Out).iLayer_level[2] = 1023;
    (*G_Info_Out).iLayer_level[2] = 1023;
    (*B_Info_Out).iLayer_level[2] = 1023;
    (*R_Info_Out).iLayer_level[3] = 1023;
    (*G_Info_Out).iLayer_level[3] = 1023;
    (*B_Info_Out).iLayer_level[3] = 1023;
    (*R_Info_Out).iLayer_level[4] = 1023;
    (*G_Info_Out).iLayer_level[4] = 1023;
    (*B_Info_Out).iLayer_level[4] = 1023;
    (*R_Info_Out).iLayer_level[5] = 1023;
    (*G_Info_Out).iLayer_level[5] = 1023;
    (*B_Info_Out).iLayer_level[5] = 1023;
    (*R_Info_Out).iLayer_level[6] = 1023;
    (*G_Info_Out).iLayer_level[6] = 1023;
    (*B_Info_Out).iLayer_level[6] = 1023;
      


    int black_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_black_limit , 4, 8);    // 12bit (format 10.2)
    int layer_level1   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level1, 4, 8);   // 12bit (format 10.2)
    int layer_level2   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level2, 4, 8);   // 12bit (format 10.2)
    int layer_level3   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level3, 4, 8);   // 12bit (format 10.2)
    int layer_level4   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level4, 4, 8);   // 12bit (format 10.2)
    int white_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_white_limit , 4, 8);    // 12bit (format 10.2)

    black_level  = ((black_level + 2)>>2);   // 12 bit -> 10 bit
    layer_level1 = ((layer_level1 + 2)>>2);  // 12 bit -> 10 bit
    layer_level2 = ((layer_level2 + 2)>>2);  // 12 bit -> 10 bit
    layer_level3 = ((layer_level3 + 2)>>2);  // 12 bit -> 10 bit
    layer_level4 = ((layer_level4 + 2)>>2);  // 12 bit -> 10 bit
    white_level  = ((white_level + 2)>>2);   // 12 bit -> 10 bit
    switch(layer_level_num)
    {
    case 1:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = white_level;
        break;
    case 2:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = white_level;
        break;
    case 3:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = white_level;
        break;
    case 4:
        (*R_Info_Out).iLayer_level[0] = black_level;
        (*R_Info_Out).iLayer_level[1] = layer_level1;
        (*R_Info_Out).iLayer_level[2] = layer_level2;
        (*R_Info_Out).iLayer_level[3] = layer_level3;
        (*R_Info_Out).iLayer_level[4] = layer_level4;
        (*R_Info_Out).iLayer_level[5] = white_level;

        (*G_Info_Out).iLayer_level[0] = black_level;
        (*G_Info_Out).iLayer_level[1] = layer_level1;
        (*G_Info_Out).iLayer_level[2] = layer_level2;
        (*G_Info_Out).iLayer_level[3] = layer_level3;
        (*G_Info_Out).iLayer_level[4] = layer_level4;
        (*G_Info_Out).iLayer_level[5] = white_level;

        (*B_Info_Out).iLayer_level[0] = black_level;
        (*B_Info_Out).iLayer_level[1] = layer_level1;
        (*B_Info_Out).iLayer_level[2] = layer_level2;
        (*B_Info_Out).iLayer_level[3] = layer_level3;
        (*B_Info_Out).iLayer_level[4] = layer_level4;
        (*B_Info_Out).iLayer_level[5] = white_level;
        break;
    }
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ low luma weight start ++++++++++++//
    (*R_Info_Out).bLow_luma_en         = FALSE;
    (*R_Info_Out).iLow_luma_thrd       = 0x00;
    (*R_Info_Out).iLow_luma_slope      = 0x0F;
    (*R_Info_Out).iLow_luma_min_weight = 0x0;

    (*G_Info_Out).bLow_luma_en         = FALSE;
    (*G_Info_Out).iLow_luma_thrd       = 0x00;
    (*G_Info_Out).iLow_luma_slope      = 0x0F;
    (*G_Info_Out).iLow_luma_min_weight = 0x0;

    (*B_Info_Out).bLow_luma_en         = FALSE;
    (*B_Info_Out).iLow_luma_thrd       = 0x00;
    (*B_Info_Out).iLow_luma_slope      = 0x0F;
    (*B_Info_Out).iLow_luma_min_weight = 0x0;
    //++++++++++++ low luma weight end ++++++++++++//

    //++++++++++++ high luma weight start ++++++++++++//
    (*R_Info_Out).bHigh_luma_en         = FALSE;
    (*R_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*R_Info_Out).iHigh_luma_slope      = 0x0F;
    (*R_Info_Out).iHigh_luma_min_weight = 0x0;

    (*G_Info_Out).bHigh_luma_en         = FALSE;
    (*G_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*G_Info_Out).iHigh_luma_slope      = 0x0F;
    (*G_Info_Out).iHigh_luma_min_weight = 0x0;

    (*B_Info_Out).bHigh_luma_en         = FALSE;
    (*B_Info_Out).iHigh_luma_thrd       = 0xFF;
    (*B_Info_Out).iHigh_luma_slope      = 0x0F;
    (*B_Info_Out).iHigh_luma_min_weight = 0x0;
    //++++++++++++ high luma weight end ++++++++++++//
    //---------------------- DMC register to Mstar register end ----------------------//


    //---------------------- DMC LUT Mapping to Mstar LUT start ----------------------//
    unsigned char *AUO_IN_DATA    =  phdr->lut_buffer;
    int dmc_lut_start_addr        =  phdr->lut_offset;

    for (idx = 0; idx < 6; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < (Mstar_lut_h_size * Mstar_lut_v_size); cnt_idx++)
        {
            (*R_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*G_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
            (*B_Info_Out).Lut_in[idx][cnt_idx] = 0.0;
        }
    }

    if((layer_level_num>=1)&&(layer_level_num<=4))
    {
        int layer_level_idx;
        for (layer_level_idx = 1; layer_level_idx <= layer_level_num; layer_level_idx++)
        {

            int r_layer_gain;
            int g_layer_gain;
            int b_layer_gain;

            int r_layer_offset;
            int g_layer_offset;
            int b_layer_offset;

            switch(layer_level_idx)
            {
            case 1:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag1, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag1, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag1, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset1, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset1, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset1, 6, 8);
                break;
            case 2:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag2, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag2, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag2, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset2, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset2, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset2, 6, 8);
                break;
            case 3:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag3, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag3, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag3, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset3, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset3, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset3, 6, 8);
                break;
            case 4:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag4, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag4, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag4, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset4, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset4, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset4, 6, 8);
                break;
            }

            int one_layer_lut_HxV_size = (Mstar_y_mode_en==1) ? ( Mstar_lut_h_size * Mstar_lut_v_size) : ( Mstar_lut_h_size * Mstar_lut_v_size*3);
            int auo_dmc_lut_address_idx_offset = (Mstar_y_mode_en==1) ? 1 : 3;

            int auo_dmc_lut_address_idx;
            for (auo_dmc_lut_address_idx = 0; auo_dmc_lut_address_idx < one_layer_lut_HxV_size; auo_dmc_lut_address_idx = auo_dmc_lut_address_idx + auo_dmc_lut_address_idx_offset)
            {
                int real_dmc_lut_address = (one_layer_lut_HxV_size*(layer_level_idx - 1)) + auo_dmc_lut_address_idx + dmc_lut_start_addr;

                if(Mstar_y_mode_en==1)
                {
                    // Mono mode
                    // DMC parameters only use R-Channel register data of DMC_DATA_R_MAGn and DMC_DATA_R_OFFSETn in Mono mode.
                    int lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];

                    double mstar_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, lut_value);

                    int cnt_idx = auo_dmc_lut_address_idx;
                    (*R_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;
                    (*G_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;
                    (*B_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_lut_in_double;

//printf("real_dmc_lut_address = 0x%x, r_layer_offset = 0x%x, r_layer_gain = 0x%x, ", real_dmc_lut_address, r_layer_offset, r_layer_gain);
//printf("0x%x_0x%x : %lf\n", layer_level_idx, cnt_idx, mstar_lut_in_double);
//printf("lut_value = 0x%x, 0x%x_0x%x : %lf\n", lut_value, layer_level_idx, cnt_idx, mstar_lut_in_double);

                }
                else
                {
                    // RGB mode
                    int r_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    int g_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+1];
                    int b_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+2];

                    double mstar_r_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, r_lut_value);
                    double mstar_g_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, g_lut_value);
                    double mstar_b_lut_in_double = DMC_LUT_Fix_To_Double( b_layer_offset, b_layer_gain, b_lut_value);

                    int cnt_idx = (int)((double)auo_dmc_lut_address_idx/(double)3);

                    (*R_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_r_lut_in_double;
                    (*G_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_g_lut_in_double;
                    (*B_Info_Out).Lut_in[layer_level_idx][cnt_idx]       = mstar_b_lut_in_double;
                }
            }
        }
    }
    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//
    return TRUE;
}

#else

static MS_BOOL set_u13_interface(AUO_Demura_Header *phdr, dmc_registers *pdmc_reg, void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL dmc_parameter_error_flag = FALSE;
    char *dmc_parameter_error_msg = "";
    double mstar_lut_in_double = 0;

    interface_info *pInfo_Out = (interface_info *)pDataInfo;

    //++++++++++++ LUT H/V Size start +++++++++++//
    int Mstar_lut_h_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_h_lut_num, 7, 4);
    int Mstar_lut_v_size         = register_16bits_inv( (*pdmc_reg).reg_dmc_v_lut_num, 2, 8);
    //++++++++++++ LUT H/V Size end ++++++++++++//

    //++++++++++++ Block H/V Size start ++++++++++++//
    (*pInfo_Out).reg_dmc_h_block = register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3);
    (*pInfo_Out).reg_dmc_v_block = register_08bits_inv( (*pdmc_reg).reg_dmc_v_block, 3);
    pbin_info->Blk_h_size        = (*pInfo_Out).reg_dmc_h_block;
    pbin_info->Blk_v_size        = (*pInfo_Out).reg_dmc_v_block;

    (*pInfo_Out).reg_dmc_panel_h_size = (Mstar_lut_h_size - 1) * (1<<(register_08bits_inv( (*pdmc_reg).reg_dmc_h_block, 3)));

    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    (*pInfo_Out).reg_dmc_rgb_mode   = (register_08bits_inv( (*pdmc_reg).reg_dmc_rgb_mode, 1)==1) ? 1 : 0;

    pbin_info->Sep_type          = (*pInfo_Out).reg_dmc_rgb_mode;
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//


    //++++++++++++ layerX level enable start ++++++++++++//
    int layer_level_num      = register_08bits_inv( (*pdmc_reg).reg_dmc_plane_num, 4);

    layer_level_num = ((layer_level_num>=1)&&(layer_level_num<=8)) ? layer_level_num : 3;
    (*pInfo_Out).reg_dmc_plane_num = layer_level_num;

    switch(layer_level_num)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        break;
    default:
        dmc_parameter_error_flag = TRUE;
        dmc_parameter_error_msg = "layer_level_num : over 8 layer level\n";
        break;
    }
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    (*pInfo_Out).reg_dmc_black_limit  = 0x100;       // 12 bit , format 10.2
    (*pInfo_Out).reg_dmc_plane_level1 = 0;
    (*pInfo_Out).reg_dmc_plane_level2 = 0;
    (*pInfo_Out).reg_dmc_plane_level3 = 0;
    (*pInfo_Out).reg_dmc_plane_level4 = 0;
    (*pInfo_Out).reg_dmc_plane_level5 = 0;
    (*pInfo_Out).reg_dmc_plane_level6 = 0;
    (*pInfo_Out).reg_dmc_plane_level7 = 0;
    (*pInfo_Out).reg_dmc_plane_level8 = 0;
    (*pInfo_Out).reg_dmc_white_limit  = 0xEF0;       // 12 bit , format 10.2

    int black_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_black_limit , 4, 8);    // 12bit (format 10.2)
    int layer_level1   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level1, 4, 8);   // 12bit (format 10.2)
    int layer_level2   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level2, 4, 8);   // 12bit (format 10.2)
    int layer_level3   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level3, 4, 8);   // 12bit (format 10.2)
    int layer_level4   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level4, 4, 8);   // 12bit (format 10.2)
    int layer_level5   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level5, 4, 8);   // 12bit (format 10.2)
    int layer_level6   = register_16bits_inv( (*pdmc_reg).reg_dmc_plane_level6, 4, 8);   // 12bit (format 10.2)
    int white_level    = register_16bits_inv( (*pdmc_reg).reg_dmc_white_limit , 4, 8);   // 12bit (format 10.2)

    switch(layer_level_num)
    {
    case 1:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 2:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 3:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 4:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 5:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    case 6:
        (*pInfo_Out).reg_dmc_black_limit  = black_level;       // 12 bit , format 10.2
        (*pInfo_Out).reg_dmc_plane_level1 = layer_level1;
        (*pInfo_Out).reg_dmc_plane_level2 = layer_level2;
        (*pInfo_Out).reg_dmc_plane_level3 = layer_level3;
        (*pInfo_Out).reg_dmc_plane_level4 = layer_level4;
        (*pInfo_Out).reg_dmc_plane_level5 = layer_level5;
        (*pInfo_Out).reg_dmc_plane_level6 = layer_level6;
        (*pInfo_Out).reg_dmc_white_limit  = white_level;       // 12 bit , format 10.2
        break;
    default :
        dmc_parameter_error_flag = TRUE;
        dmc_parameter_error_msg = "layer_level_num : over 6 layer level\n";
        break;
    }

    //++++++++++++ layerX level end ++++++++++++//

    (*pInfo_Out).reg_dmc_dither_en              = 0x1;         //  2 bit , 0 : rounding , 1 : dither , 2~3 : truncate
    (*pInfo_Out).reg_dmc_dither_rst_e_4_frame   = 0x0;         //  1 bit
    (*pInfo_Out).reg_dmc_dither_pse_rst_num     = 0x0;         //  2 bit
    //---------------------- DMC register to Mstar register end ----------------------//


    //---------------------- DMC LUT Mapping to Mstar LUT start ----------------------//
    unsigned char *AUO_IN_DATA    =  phdr->lut_buffer;
    int dmc_lut_start_addr        =  phdr->lut_offset;

    // Init to zero value
    int idx;
    for (idx = 0; idx < layer_level_num; idx++)
    {
        int cnt_idx;
        for (cnt_idx = 0; cnt_idx < Mstar_lut_h_size*Mstar_lut_v_size; cnt_idx++)
        {
            (*pInfo_Out).Lut_in[idx][cnt_idx].dbr = 0.0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].dbg = 0.0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].dbb = 0.0;

            (*pInfo_Out).Lut_in[idx][cnt_idx].r = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].g = 0;
            (*pInfo_Out).Lut_in[idx][cnt_idx].b = 0;
        }
    }

    if((layer_level_num>=1)&&(layer_level_num<=6))
    {
        int layer_level_idx;
        for (layer_level_idx = 1; layer_level_idx <= layer_level_num; layer_level_idx++)
        {
            int r_layer_gain;
            int g_layer_gain;
            int b_layer_gain;

            int r_layer_offset;
            int g_layer_offset;
            int b_layer_offset;

            int layer_level = 0;  // layer level 10bit data path

            switch(layer_level_idx)
            {
            case 1:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag1, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag1, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag1, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset1, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset1, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset1, 6, 8);

                layer_level = layer_level1 >> 2;
                break;
            case 2:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag2, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag2, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag2, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset2, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset2, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset2, 6, 8);

                layer_level = layer_level2 >> 2;
                break;
            case 3:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag3, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag3, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag3, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset3, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset3, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset3, 6, 8);

                layer_level = layer_level3 >> 2;
                break;
            case 4:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag4, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag4, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag4, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset4, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset4, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset4, 6, 8);

                layer_level = layer_level4 >> 2;
                break;
             case 5:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag5, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag5, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag5, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset5, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset5, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset5, 6, 8);

                layer_level = layer_level5 >> 2;
                break;
             case 6:
                r_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_r_mag6, 3);
                g_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_g_mag6, 3);
                b_layer_gain = register_08bits_inv((*pdmc_reg).reg_dmc_data_b_mag6, 3);

                r_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_r_offset6, 6, 8);
                g_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_g_offset6, 6, 8);
                b_layer_offset = register_16bits_inv((*pdmc_reg).reg_dmc_data_b_offset6, 6, 8);

                layer_level = layer_level6 >> 2;
            }

            int one_layer_lut_HxV_size = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? (Mstar_lut_h_size*Mstar_lut_v_size) : (Mstar_lut_h_size*Mstar_lut_v_size*3);
            int auo_dmc_lut_address_idx_offset = ((*pInfo_Out).reg_dmc_rgb_mode == 0) ? 1 : 3;

            int auo_dmc_lut_address_idx;
            for (auo_dmc_lut_address_idx = 0; auo_dmc_lut_address_idx < one_layer_lut_HxV_size; auo_dmc_lut_address_idx = auo_dmc_lut_address_idx + auo_dmc_lut_address_idx_offset)
            {
                int real_dmc_lut_address = (one_layer_lut_HxV_size*(layer_level_idx - 1)) + auo_dmc_lut_address_idx + dmc_lut_start_addr;

                if((*pInfo_Out).reg_dmc_rgb_mode == 0)
                {
                    // Mono mode
                    // DMC parameters only use R-Channel register data of DMC_DATA_R_MAGn and DMC_DATA_R_OFFSETn in Mono mode.
                    int lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    if(Demura_Original_Mode == E_RGB_MODE)
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, lut_value);
                    }
                    else
                    {
                        mstar_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, lut_value);
                    }

                    int cnt_idx = auo_dmc_lut_address_idx;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_lut_in_double  + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_lut_in_double + layer_level;
                }
                else
                {
                    // RGB mode
                    int r_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address];
                    int g_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+1];
                    int b_lut_value = (int)AUO_IN_DATA[real_dmc_lut_address+2];

                    double mstar_r_lut_in_double = DMC_LUT_Fix_To_Double( r_layer_offset, r_layer_gain, r_lut_value);
                    double mstar_g_lut_in_double = DMC_LUT_Fix_To_Double( g_layer_offset, g_layer_gain, g_lut_value);
                    double mstar_b_lut_in_double = DMC_LUT_Fix_To_Double( b_layer_offset, b_layer_gain, b_lut_value);

                    int cnt_idx = (int)((double)auo_dmc_lut_address_idx/(double)3);

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbr     = mstar_r_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbg     = mstar_g_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].dbb     = mstar_b_lut_in_double + layer_level;

                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].r       = (int)mstar_r_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].g       = (int)mstar_g_lut_in_double + layer_level;
                    (*pInfo_Out).Lut_in[layer_level_idx-1][cnt_idx].b       = (int)mstar_b_lut_in_double + layer_level;
                }
            }
        }
    }

    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//
    return TRUE;
}

#endif

MS_BOOL AUO_Decode_To_Mstar_Format(AUO_Demura_Header *phdr, void *pDataInfo, BinOutputInfo *pbin_info)
{
    //---------------------- Capture DMC Parameter (register) start ----------------------//
    MS_BOOL bRet;
    unsigned char *AUO_IN_DATA = phdr->hdr_buffer;

    //int dmc_para_start_addr = 0x10002; // (dec : 65538)
    //int auo_dmc_parameter_end_address  = 0x1006A; // (dec : 65642)
    int dmc_para_start_addr = phdr->hdr_offset; // (dec : 2)

    dmc_registers _dmc_reg = {
        {{dmc_para_start_addr +   0,0x80,0x80,"reg_dmc_rgb_mode"             }},
        {{dmc_para_start_addr +   0,0x30,0x10,"reg_dmc_block_size"           }},
        {{dmc_para_start_addr +   0,0x0F,0x03,"reg_dmc_plane_num"            }},
        {{dmc_para_start_addr +   1,0x70,0x30,"reg_dmc_h_block"              }},
        {{dmc_para_start_addr +   1,0x07,0x03,"reg_dmc_v_block"              }},
        {{dmc_para_start_addr +   3,0xF0,0x10,"reg_dmc_h_lut_num"            }, {dmc_para_start_addr +   2,0x7F,0x1e,"reg_dmc_h_lut_num"            }},
        {{dmc_para_start_addr +   4,0xFF,0x0f,"reg_dmc_v_lut_num"            }, {dmc_para_start_addr +   3,0x03,0x01,"reg_dmc_v_lut_num"            }},
        {{dmc_para_start_addr +   5,0x80,0x00,"reg_dmc_plane_b1_ks22"        }},
        {{dmc_para_start_addr +   5,0x40,0x00,"reg_dmc_plane_12_ks22"        }},
        {{dmc_para_start_addr +   5,0x20,0x00,"reg_dmc_plane_23_ks22"        }},
        {{dmc_para_start_addr +   5,0x10,0x00,"reg_dmc_plane_34_ks22"        }},
        {{dmc_para_start_addr +   5,0x08,0x00,"reg_dmc_plane_45_ks22"        }},
        {{dmc_para_start_addr +   5,0x04,0x00,"reg_dmc_plane_56_ks22"        }},
        {{dmc_para_start_addr +   5,0x02,0x00,"reg_dmc_plane_67_ks22"        }},
        {{dmc_para_start_addr +   5,0x01,0x00,"reg_dmc_plane_78_ks22"        }},
        {{dmc_para_start_addr +   6,0x80,0x00,"reg_dmc_plane_8w_ks22"        }},
        {{dmc_para_start_addr +   8,0xFF,0x00,"reg_dmc_black_limit"          }, {dmc_para_start_addr +   7,0x0F,0x00,"reg_dmc_black_limit"          }},
        {{dmc_para_start_addr +  10,0xFF,0xff,"reg_dmc_plane_level1"         }, {dmc_para_start_addr +   9,0x0F,0x0f,"reg_dmc_plane_level1"         }},
        {{dmc_para_start_addr +  12,0xFF,0xff,"reg_dmc_plane_level2"         }, {dmc_para_start_addr +  11,0x0F,0x0f,"reg_dmc_plane_level2"         }},
        {{dmc_para_start_addr +  14,0xFF,0xff,"reg_dmc_plane_level3"         }, {dmc_para_start_addr +  13,0x0F,0x0f,"reg_dmc_plane_level3"         }},
        {{dmc_para_start_addr +  16,0xFF,0xff,"reg_dmc_plane_level4"         }, {dmc_para_start_addr +  15,0x0F,0x0f,"reg_dmc_plane_level4"         }},
        {{dmc_para_start_addr +  18,0xFF,0xff,"reg_dmc_plane_level5"         }, {dmc_para_start_addr +  17,0x0F,0x0f,"reg_dmc_plane_level5"         }},
        {{dmc_para_start_addr +  20,0xFF,0xff,"reg_dmc_plane_level6"         }, {dmc_para_start_addr +  19,0x0F,0x0f,"reg_dmc_plane_level6"         }},
        {{dmc_para_start_addr +  22,0xFF,0xff,"reg_dmc_plane_level7"         }, {dmc_para_start_addr +  21,0x0F,0x0f,"reg_dmc_plane_level7"         }},
        {{dmc_para_start_addr +  24,0xFF,0xff,"reg_dmc_plane_level8"         }, {dmc_para_start_addr +  23,0x0F,0x0f,"reg_dmc_plane_level8"         }},
        {{dmc_para_start_addr +  26,0xFF,0xff,"reg_dmc_white_limit"          }, {dmc_para_start_addr +  25,0x0F,0x0f,"reg_dmc_white_limit"          }},
        {{dmc_para_start_addr +  28,0xFF,0x00,"reg_dmc_plane_b1_coef"        }, {dmc_para_start_addr +  27,0x3F,0x00,"reg_dmc_plane_b1_coef"        }},
        {{dmc_para_start_addr +  30,0xFF,0x00,"reg_dmc_plane_12_coef"        }, {dmc_para_start_addr +  29,0x3F,0x00,"reg_dmc_plane_12_coef"        }},
        {{dmc_para_start_addr +  32,0xFF,0x00,"reg_dmc_plane_23_coef"        }, {dmc_para_start_addr +  31,0x3F,0x00,"reg_dmc_plane_23_coef"        }},
        {{dmc_para_start_addr +  34,0xFF,0x00,"reg_dmc_plane_34_coef"        }, {dmc_para_start_addr +  33,0x3F,0x00,"reg_dmc_plane_34_coef"        }},
        {{dmc_para_start_addr +  36,0xFF,0x00,"reg_dmc_plane_45_coef"        }, {dmc_para_start_addr +  35,0x3F,0x00,"reg_dmc_plane_45_coef"        }},
        {{dmc_para_start_addr +  38,0xFF,0x00,"reg_dmc_plane_56_coef"        }, {dmc_para_start_addr +  37,0x3F,0x00,"reg_dmc_plane_56_coef"        }},
        {{dmc_para_start_addr +  40,0xFF,0x00,"reg_dmc_plane_67_coef"        }, {dmc_para_start_addr +  39,0x3F,0x00,"reg_dmc_plane_67_coef"        }},
        {{dmc_para_start_addr +  42,0xFF,0x00,"reg_dmc_plane_78_coef"        }, {dmc_para_start_addr +  41,0x3F,0x00,"reg_dmc_plane_78_coef"        }},
        {{dmc_para_start_addr +  44,0xFF,0x00,"reg_dmc_plane_8w_coef"        }, {dmc_para_start_addr +  43,0x3F,0x00,"reg_dmc_plane_8w_coef"        }},
        {{dmc_para_start_addr +  45,0x70,0x00,"reg_dmc_data_r_mag1"          }},
        {{dmc_para_start_addr +  45,0x07,0x00,"reg_dmc_data_r_mag2"          }},
        {{dmc_para_start_addr +  46,0x70,0x00,"reg_dmc_data_r_mag3"          }},
        {{dmc_para_start_addr +  46,0x07,0x00,"reg_dmc_data_r_mag4"          }},
        {{dmc_para_start_addr +  47,0x70,0x00,"reg_dmc_data_r_mag5"          }},
        {{dmc_para_start_addr +  47,0x07,0x00,"reg_dmc_data_r_mag6"          }},
        {{dmc_para_start_addr +  48,0x70,0x00,"reg_dmc_data_r_mag7"          }},
        {{dmc_para_start_addr +  48,0x07,0x00,"reg_dmc_data_r_mag8"          }},
        {{dmc_para_start_addr +  49,0x70,0x00,"reg_dmc_data_g_mag1"          }},
        {{dmc_para_start_addr +  49,0x07,0x00,"reg_dmc_data_g_mag2"          }},
        {{dmc_para_start_addr +  50,0x70,0x00,"reg_dmc_data_g_mag3"          }},
        {{dmc_para_start_addr +  50,0x07,0x00,"reg_dmc_data_g_mag4"          }},
        {{dmc_para_start_addr +  51,0x70,0x00,"reg_dmc_data_g_mag5"          }},
        {{dmc_para_start_addr +  51,0x07,0x00,"reg_dmc_data_g_mag6"          }},
        {{dmc_para_start_addr +  52,0x70,0x00,"reg_dmc_data_g_mag7"          }},
        {{dmc_para_start_addr +  52,0x07,0x00,"reg_dmc_data_g_mag8"          }},
        {{dmc_para_start_addr +  53,0x70,0x00,"reg_dmc_data_b_mag1"          }},
        {{dmc_para_start_addr +  53,0x07,0x00,"reg_dmc_data_b_mag2"          }},
        {{dmc_para_start_addr +  54,0x70,0x00,"reg_dmc_data_b_mag3"          }},
        {{dmc_para_start_addr +  54,0x07,0x00,"reg_dmc_data_b_mag4"          }},
        {{dmc_para_start_addr +  55,0x70,0x00,"reg_dmc_data_b_mag5"          }},
        {{dmc_para_start_addr +  55,0x07,0x00,"reg_dmc_data_b_mag6"          }},
        {{dmc_para_start_addr +  56,0x70,0x00,"reg_dmc_data_b_mag7"          }},
        {{dmc_para_start_addr +  56,0x07,0x00,"reg_dmc_data_b_mag8"          }},
        {{dmc_para_start_addr +  58,0xFF,0x00,"reg_dmc_data_r_offset1"       }, {dmc_para_start_addr +  57,0x3F,0x00,"reg_dmc_data_r_offset1"       }},
        {{dmc_para_start_addr +  60,0xFF,0x00,"reg_dmc_data_r_offset2"       }, {dmc_para_start_addr +  59,0x3F,0x00,"reg_dmc_data_r_offset2"       }},
        {{dmc_para_start_addr +  62,0xFF,0x00,"reg_dmc_data_r_offset3"       }, {dmc_para_start_addr +  61,0x3F,0x00,"reg_dmc_data_r_offset3"       }},
        {{dmc_para_start_addr +  64,0xFF,0x00,"reg_dmc_data_r_offset4"       }, {dmc_para_start_addr +  63,0x3F,0x00,"reg_dmc_data_r_offset4"       }},
        {{dmc_para_start_addr +  66,0xFF,0x00,"reg_dmc_data_r_offset5"       }, {dmc_para_start_addr +  65,0x3F,0x00,"reg_dmc_data_r_offset5"       }},
        {{dmc_para_start_addr +  68,0xFF,0x00,"reg_dmc_data_r_offset6"       }, {dmc_para_start_addr +  67,0x3F,0x00,"reg_dmc_data_r_offset6"       }},
        {{dmc_para_start_addr +  70,0xFF,0x00,"reg_dmc_data_r_offset7"       }, {dmc_para_start_addr +  69,0x3F,0x00,"reg_dmc_data_r_offset7"       }},
        {{dmc_para_start_addr +  72,0xFF,0x00,"reg_dmc_data_r_offset8"       }, {dmc_para_start_addr +  71,0x3F,0x00,"reg_dmc_data_r_offset8"       }},
        {{dmc_para_start_addr +  74,0xFF,0x00,"reg_dmc_data_g_offset1"       }, {dmc_para_start_addr +  73,0x3F,0x00,"reg_dmc_data_g_offset1"       }},
        {{dmc_para_start_addr +  76,0xFF,0x00,"reg_dmc_data_g_offset2"       }, {dmc_para_start_addr +  75,0x3F,0x00,"reg_dmc_data_g_offset2"       }},
        {{dmc_para_start_addr +  78,0xFF,0x00,"reg_dmc_data_g_offset3"       }, {dmc_para_start_addr +  77,0x3F,0x00,"reg_dmc_data_g_offset3"       }},
        {{dmc_para_start_addr +  80,0xFF,0x00,"reg_dmc_data_g_offset4"       }, {dmc_para_start_addr +  79,0x3F,0x00,"reg_dmc_data_g_offset4"       }},
        {{dmc_para_start_addr +  82,0xFF,0x00,"reg_dmc_data_g_offset5"       }, {dmc_para_start_addr +  81,0x3F,0x00,"reg_dmc_data_g_offset5"       }},
        {{dmc_para_start_addr +  84,0xFF,0x00,"reg_dmc_data_g_offset6"       }, {dmc_para_start_addr +  83,0x3F,0x00,"reg_dmc_data_g_offset6"       }},
        {{dmc_para_start_addr +  86,0xFF,0x00,"reg_dmc_data_g_offset7"       }, {dmc_para_start_addr +  85,0x3F,0x00,"reg_dmc_data_g_offset7"       }},
        {{dmc_para_start_addr +  88,0xFF,0x00,"reg_dmc_data_g_offset8"       }, {dmc_para_start_addr +  87,0x3F,0x00,"reg_dmc_data_g_offset8"       }},
        {{dmc_para_start_addr +  90,0xFF,0x00,"reg_dmc_data_b_offset1"       }, {dmc_para_start_addr +  89,0x3F,0x00,"reg_dmc_data_b_offset1"       }},
        {{dmc_para_start_addr +  92,0xFF,0x00,"reg_dmc_data_b_offset2"       }, {dmc_para_start_addr +  91,0x3F,0x00,"reg_dmc_data_b_offset2"       }},
        {{dmc_para_start_addr +  94,0xFF,0x00,"reg_dmc_data_b_offset3"       }, {dmc_para_start_addr +  93,0x3F,0x00,"reg_dmc_data_b_offset3"       }},
        {{dmc_para_start_addr +  96,0xFF,0x00,"reg_dmc_data_b_offset4"       }, {dmc_para_start_addr +  95,0x3F,0x00,"reg_dmc_data_b_offset4"       }},
        {{dmc_para_start_addr +  98,0xFF,0x00,"reg_dmc_data_b_offset5"       }, {dmc_para_start_addr +  97,0x3F,0x00,"reg_dmc_data_b_offset5"       }},
        {{dmc_para_start_addr + 100,0xFF,0x00,"reg_dmc_data_b_offset6"       }, {dmc_para_start_addr +  99,0x3F,0x00,"reg_dmc_data_b_offset6"       }},
        {{dmc_para_start_addr + 102,0xFF,0x00,"reg_dmc_data_b_offset7"       }, {dmc_para_start_addr + 101,0x3F,0x00,"reg_dmc_data_b_offset7"       }},
        {{dmc_para_start_addr + 104,0xFF,0x00,"reg_dmc_data_b_offset8"       }, {dmc_para_start_addr + 103,0x3F,0x00,"reg_dmc_data_b_offset8"       }},
    };


    //++++++++++++ Y mode (Mono mode) start ++++++++++++//
    if(Demura_Original_Mode == E_RGB_MODE)
        _dmc_reg.reg_dmc_rgb_mode.Bit07_00.value = 0;
    else
        _dmc_reg.reg_dmc_rgb_mode.Bit07_00.value              = AUO_IN_DATA[_dmc_reg.reg_dmc_rgb_mode.Bit07_00.address];            // dmc_rgb_mode (0 : mono mode / 1 : rgb mode) <-> mstar_y_mode_en (0 : rgb mode / 1 : mono mode ) , ///// ?ÄË¶ÅÂ???/////
    //++++++++++++ Y mode (Mono mode) end ++++++++++++//

    //++++++++++++ layerX level enable start ++++++++++++//
    _dmc_reg.reg_dmc_plane_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_num.Bit07_00.address];           // dmc_plane_num <-> bLayer_level_en[?+2] , ///// +2 ?üÂ??ÄË¶ÅËÄÉÊÖÆcut off ?Ø‰Ωø?®layer level‰æÜÂ??êÔ??Ä‰ª•Mstar?Äworse case?™ËÉΩ??target 4 Â±?/////
    //++++++++++++ layerX level enable end ++++++++++++//

    //++++++++++++ Block H/V Size start ++++++++++++//
    _dmc_reg.reg_dmc_h_block.Bit07_00.value               = AUO_IN_DATA[_dmc_reg.reg_dmc_h_block.Bit07_00.address];             // dmc_h_block <-> Mstar_block_h_size , ///// AUOÁµ¶Á????ÑÂÜ™Ê¨°ÊñπÔºåMstar ‰ªãÈù¢Áµ¶Á??ØÁõ¥?•block?∏Ô??ÄË¶ÅË??õÔ?Ê≥®Ê?AUO?ÄÂ∞èÊ??ØËÉΩ?? -> block?? /////

    _dmc_reg.reg_dmc_v_block.Bit07_00.value               = AUO_IN_DATA[_dmc_reg.reg_dmc_v_block.Bit07_00.address];             // dmc_v_block <-> Mstar_block_v_size , ///// AUOÁµ¶Á????ÑÂÜ™Ê¨°ÊñπÔºåMstar ‰ªãÈù¢Áµ¶Á??ØÁõ¥?•block?∏Ô??ÄË¶ÅË??õÔ?Ê≥®Ê?AUO?ÄÂ∞èÊ??ØËÉΩ?? -> block?? /////
    //++++++++++++ Block H/V Size end ++++++++++++//

    //++++++++++++ LUT H/V Size start +++++++++++//
    _dmc_reg.reg_dmc_h_lut_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_h_lut_num.Bit07_00.address];           // dmc_h_lut_num <-> Mstar_lut_h_size
    _dmc_reg.reg_dmc_h_lut_num.Bit15_08.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_h_lut_num.Bit15_08.address];

    _dmc_reg.reg_dmc_v_lut_num.Bit07_00.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_v_lut_num.Bit07_00.address];           // dmc_v_lut_num <-> Mstar_lut_v_size
    _dmc_reg.reg_dmc_v_lut_num.Bit15_08.value             = AUO_IN_DATA[_dmc_reg.reg_dmc_v_lut_num.Bit15_08.address];
    //++++++++++++ LUT H/V Size end +++++++++++//

    //++++++++++++ layerX level start ++++++++++++//
    _dmc_reg.reg_dmc_black_limit.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_black_limit.Bit07_00.address];         // reg_dmc_black_limit <-> iLayer_level[0] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_black_limit.Bit15_08.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_black_limit.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level1.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level1.Bit07_00.address];        // reg_dmc_plane_level1 <-> iLayer_level[1] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_plane_level1.Bit15_08.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level1.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level2.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level2.Bit07_00.address];        // reg_dmc_plane_level2 <-> iLayer_level[2] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_plane_level2.Bit15_08.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level2.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level3.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level3.Bit07_00.address];        // reg_dmc_plane_level3 <-> iLayer_level[3] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_plane_level3.Bit15_08.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level3.Bit15_08.address];

    _dmc_reg.reg_dmc_plane_level4.Bit07_00.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level4.Bit07_00.address];        // reg_dmc_plane_level4 <-> iLayer_level[4] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_plane_level4.Bit15_08.value          = AUO_IN_DATA[_dmc_reg.reg_dmc_plane_level4.Bit15_08.address];

    _dmc_reg.reg_dmc_white_limit.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_white_limit.Bit07_00.address];         // reg_dmc_white_limit <-> iLayer_level[5] or iLayer_level[4] Ôº?///// AUO??2bitÔºåMstar??0bit /////
    _dmc_reg.reg_dmc_white_limit.Bit15_08.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_white_limit.Bit15_08.address];
    //++++++++++++ layerX level end ++++++++++++//

    //++++++++++++ layerX gain start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag1.Bit07_00.address];         // reg_dmc_data_r_mag1 <-> r layer gain 1

    _dmc_reg.reg_dmc_data_r_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag2.Bit07_00.address];         // reg_dmc_data_r_mag2 <-> r layer gain 2

    _dmc_reg.reg_dmc_data_r_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag3.Bit07_00.address];         // reg_dmc_data_r_mag3 <-> r layer gain 3

    _dmc_reg.reg_dmc_data_r_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_mag4.Bit07_00.address];         // reg_dmc_data_r_mag4 <-> r layer gain 4

    _dmc_reg.reg_dmc_data_g_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag1.Bit07_00.address];         // reg_dmc_data_g_mag1 <-> g layer gain 1

    _dmc_reg.reg_dmc_data_g_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag2.Bit07_00.address];         // reg_dmc_data_g_mag2 <-> g layer gain 2

    _dmc_reg.reg_dmc_data_g_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag3.Bit07_00.address];         // reg_dmc_data_g_mag3 <-> g layer gain 3

    _dmc_reg.reg_dmc_data_g_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_mag4.Bit07_00.address];         // reg_dmc_data_g_mag4 <-> g layer gain 4

    _dmc_reg.reg_dmc_data_b_mag1.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag1.Bit07_00.address];         // reg_dmc_data_b_mag1 <-> b layer gain 1

    _dmc_reg.reg_dmc_data_b_mag2.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag2.Bit07_00.address];         // reg_dmc_data_b_mag2 <-> b layer gain 2

    _dmc_reg.reg_dmc_data_b_mag3.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag3.Bit07_00.address];         // reg_dmc_data_b_mag3 <-> b layer gain 3

    _dmc_reg.reg_dmc_data_b_mag4.Bit07_00.value           = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_mag4.Bit07_00.address];         // reg_dmc_data_b_mag4 <-> b layer gain 4
    //++++++++++++ layerX gain end ++++++++++++//

    //++++++++++++ layerX level offset start ++++++++++++//
    _dmc_reg.reg_dmc_data_r_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset1.Bit07_00.address];      // reg_dmc_data_r_offset1
    _dmc_reg.reg_dmc_data_r_offset1.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset2.Bit07_00.address];      // reg_dmc_data_r_offset2
    _dmc_reg.reg_dmc_data_r_offset2.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset3.Bit07_00.address];      // reg_dmc_data_r_offset3
    _dmc_reg.reg_dmc_data_r_offset3.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_r_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset4.Bit07_00.address];      // reg_dmc_data_r_offset4
    _dmc_reg.reg_dmc_data_r_offset4.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_r_offset4.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset1.Bit07_00.address];      // reg_dmc_data_g_offset1
    _dmc_reg.reg_dmc_data_g_offset1.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset2.Bit07_00.address];      // reg_dmc_data_g_offset2
    _dmc_reg.reg_dmc_data_g_offset2.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset3.Bit07_00.address];      // reg_dmc_data_g_offset3
    _dmc_reg.reg_dmc_data_g_offset3.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_g_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset4.Bit07_00.address];      // reg_dmc_data_g_offset4
    _dmc_reg.reg_dmc_data_g_offset4.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_g_offset4.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset1.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset1.Bit07_00.address];      // reg_dmc_data_b_offset1
    _dmc_reg.reg_dmc_data_b_offset1.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset1.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset2.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset2.Bit07_00.address];      // reg_dmc_data_b_offset2
    _dmc_reg.reg_dmc_data_b_offset2.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset2.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset3.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset3.Bit07_00.address];      // reg_dmc_data_b_offset3
    _dmc_reg.reg_dmc_data_b_offset3.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset3.Bit15_08.address];

    _dmc_reg.reg_dmc_data_b_offset4.Bit07_00.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset4.Bit07_00.address];      // reg_dmc_data_b_offset4
    _dmc_reg.reg_dmc_data_b_offset4.Bit15_08.value        = AUO_IN_DATA[_dmc_reg.reg_dmc_data_b_offset4.Bit15_08.address];
    //++++++++++++ layerX level offset end ++++++++++++//

    //---------------------- Capture DMC Parameter (register) end ----------------------//

    //---------------------- DMC register to Mstar register start ----------------------//
    #if defined (CONFIG_DEMURA_URSA11)
    bRet = set_u11_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #elif defined (CONFIG_DEMURA_URSA13)
    bRet = set_u13_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #elif (defined (CONFIG_DEMURA_M7622) || defined (CONFIG_DEMURA_M7632))
    bRet = set_u13_interface(phdr, &_dmc_reg, pDataInfo, pbin_info);
    #else
    #error "Unkown DEMURA_URSA_TYPE"
    #endif
    if (bRet == FALSE)
    {
        printf("set_demura_u%d_interface error\n", CONFIG_DEMURA_URSA_STRING);
    }

    // Sign the auo dmc data !
    Gen_Sf_Signature(phdr);

    //---------------------- DMC LUT Mapping to Mstar LUT end ----------------------//

    // Free AUO DMC Buffer
    free(phdr->hdr_buffer);
    phdr->hdr_buffer = NULL;
    phdr->hdr_offset = 0;
    phdr->hdr_length = 0;

    dfree(phdr->lut_buffer);
    phdr->lut_buffer = NULL;
    phdr->lut_offset = 0;
    phdr->lut_length = 0;

    return bRet;
}


MS_BOOL Decode_To_Mstar_Format(void *pDataInfo, BinOutputInfo *pbin_info)
{
    MS_BOOL bRet;
    AUO_Demura_Header header;

    // Parse header information
    memset(&header, 0, sizeof(header));
    if (load_vendor_header(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_header Error\n");
        return FALSE;
    }

    if (parse_vendor_header(&header) != TRUE)
    {
        UBOOT_ERROR("parse_vendor_header Error\n");
        return FALSE;
    }
    dump_vendor_header(&header);

    if (load_vendor_lut(&header) != TRUE)
    {
        UBOOT_ERROR("load_vendor_lut Error\n");
        return FALSE;
    }

    // Alloc space for Lut_in
    pbin_info->HNode       =  header.dmc_h_lut_num;
    pbin_info->VNode       =  header.dmc_v_lut_num;
    pbin_info->LevelCount  =  header.dmc_panel_num;

    bRet = Alloc_LutIn_Space(pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("Alloc_LutIn_Space error\n");
        return FALSE;
    }

    // Decode Vendor data
    bRet = AUO_Decode_To_Mstar_Format(&header, pDataInfo, pbin_info);
    if (bRet == FALSE)
    {
        UBOOT_ERROR("AUO_Decode_To_Mstar_Format error\n");
        return FALSE;
    }

    MS_BOOL demura_enable[3]  = {TRUE, TRUE, TRUE};

    pbin_info->reg_base_addr  =  0x7700;
    pbin_info->project_id     =  ((MSTAR_CHIP_ID << 16) + (1ULL << 15) + ID_CUS_MSTAR);
    pbin_info->data_type      =  IC_DRAM;
    pbin_info->Gain_type      =  10;

    memcpy(pbin_info->channel_enable, demura_enable, sizeof(demura_enable));

    return TRUE;
}


static MS_BOOL Gen_Sf_Signature(AUO_Demura_Header *phdr)
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
    snprintf(strbuf, sizeof(strbuf)-1, "0x%x:0x%x", phdr->lut_checksum, sample_crc32);
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
#if (DEMURA_ONLY_MONO_MODE)
            if(lut_checksum_sf == lut_checksum_bd)
            {
                UBOOT_DEBUG("Data Match, No Need to decode again\n");
                UBOOT_TRACE("OK\n");
                return FALSE;
            }
#else
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
#endif
        }

        UBOOT_TRACE("OK\n");
        return TRUE;
    }

    UBOOT_TRACE("OK\n");
    return FALSE;
}

