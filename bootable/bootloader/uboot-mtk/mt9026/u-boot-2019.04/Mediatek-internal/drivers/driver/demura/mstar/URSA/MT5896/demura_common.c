// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <MsTypes.h>
#include <stdio.h>
#include <linux/string.h>
#include "CommonDataType.h"
#include <dmalloc.h>
#include <ms_vector.h>
#include <demura.h>
#include <command.h>
#include <debug_impl.h>
#include "ms_array.h"
#include "demura_common.h"

#define DEMURA_TBL_BUF_RGB 7
#define DEMURA_TBL_BUF_MONO 21
#define DEMURA_TBL_BUF 21

#define DMC_S11_MAX 0x7FF
#define DMC_S11_MIN 0x800
#define DMC_S11_OFS 0x1000
#define DMC_LAYER_IMG_S11(x, ret) \
    if (x >= 0) \
    { \
        ret =  MIN(x, DMC_S11_MAX);\
    } \
    else \
    { \
        if (x < -2048) \
        { \
            ret = DMC_S11_MIN; \
        } \
        else \
        { \
            ret = x + DMC_S11_OFS; \
        } \
    }

#define _0 0
#define _1 1
#define _2 2
#define _3 3
#define _4 4
#define _5 5
#define _6 6
#define _7 7
#define _8 8

typedef enum
{
    DEMURA_MODE_MONO = 0,
    DEMURA_MODE_RGB = 1,
} DEMURA_MODE;

typedef struct
{
    MS_U32 d_in_00:12;
    MS_U32 d_in_01:12;
    MS_U32 d_in_02_l:8; //--->1
    MS_U32 d_in_02_h:4;
    MS_U32 d_in_03:12;
    MS_U32 d_in_04:12;
    MS_U32 d_in_05_l:4; //--->2
    MS_U32 d_in_05_h:8;
    MS_U32 d_in_06:12;
    MS_U32 d_in_07:12;//--->3
    MS_U32 d_in_08:12;
    MS_U32 d_in_09:12;
    MS_U32 d_in_10_l:8; //--->4
    MS_U32 d_in_10_h:4;
    MS_U32 d_in_11:12;
    MS_U32 d_in_12:12;
    MS_U32 d_in_13_l:4; //--->5
    MS_U32 d_in_13_h:8;
    MS_U32 d_in_14:12;
    MS_U32 d_in_15:12;//--->6
    MS_U32 d_in_16:12;
    MS_U32 d_in_17:12;
    MS_U32 d_in_18_l:8; //--->7
    MS_U32 d_in_18_h:4;
    MS_U32 d_in_19:12;
    MS_U32 d_in_20:12;
    MS_U32 layer_num:4;
}  __attribute__((packed)) ADL_FORMAT_S11_MONO;

typedef struct
{
    MS_U32 r_in_00:12;
    MS_U32 g_in_00:12;
    MS_U32 b_in_00_l:8; //--->1
    MS_U32 b_in_00_h:4;
    MS_U32 r_in_01:12;
    MS_U32 g_in_01:12;
    MS_U32 b_in_01_l:4; //--->2
    MS_U32 b_in_01_h:8;
    MS_U32 r_in_02:12;
    MS_U32 g_in_02:12;//--->3
    MS_U32 b_in_02:12;
    MS_U32 r_in_03:12;
    MS_U32 g_in_03_l:8; //--->4
    MS_U32 g_in_03_h:4;
    MS_U32 b_in_03:12;
    MS_U32 r_in_04:12;
    MS_U32 g_in_04_l:4; //--->5
    MS_U32 g_in_04_h:8;
    MS_U32 b_in_04:12;
    MS_U32 r_in_05:12;//--->6
    MS_U32 g_in_05:12;
    MS_U32 b_in_05:12;
    MS_U32 r_in_06_l:8; //--->7
    MS_U32 r_in_06_h:4;
    MS_U32 g_in_06:12;
    MS_U32 b_in_06:12;
    MS_U32 layer_num:4;
}  __attribute__((packed)) ADL_FORMAT_S11_RGB;

#ifdef CONFIG_DEMURA_VENDOR_LGD
MS_BOOL bit_mode_LPF(int plane_idx, int block_width, int block_height, strgb_structInfo *layer_blk_img_out);
#endif


static MS_U8 g_u8DemuraVersion = E_DEMURA_VERSION_S7;

void demura_store_version(int version)
{
    UBOOT_TRACE("ver:%d\n", version);

    if ((version == 0x02) || (version == 0x03))
    {
        UBOOT_TRACE("s11\n");
        g_u8DemuraVersion = E_DEMURA_VERSION_S11;
    }
    else if (version == 0x06)
    {
        UBOOT_TRACE("s7_MONO\n");
        g_u8DemuraVersion = E_DEMURA_VERSION_S7_MONO;
    }
    else
    {
        UBOOT_TRACE("s7\n");
        g_u8DemuraVersion = E_DEMURA_VERSION_S7;
    }
}

MS_U8 demura_get_version(void)
{
    return g_u8DemuraVersion;
}


int register_shift(int mask, int value)
{
    int shift_bit = 0;
    int shift_bit_tmp = 0;
    int mask_tmp = mask;
    int count_i;
    for (count_i = 1; count_i < 8; count_i++)
    {
        shift_bit_tmp = (mask_tmp&0x1);
        mask_tmp = (mask_tmp>>1);

        if(shift_bit_tmp==0)
            shift_bit = count_i;
        else
            break;

    }

    int adjust_value = ((value<<shift_bit)&mask);

    return adjust_value;
}

void part_bit15_00(int input_value, int total_bits, int *Bit15_08, int *Bit07_00)
{
    int msb_mask = 0;
    int msb_bit = total_bits - 8;

    int mask_count;
    for (mask_count = 0; mask_count < msb_bit; mask_count++)
    {
        msb_mask = (msb_mask<<1) + 1;
    }

    *Bit07_00 = (input_value&0xFF);
    *Bit15_08 = ((input_value>>8)&msb_mask);
}

void part_bit23_00(int input_value, int total_bits, int *Bit23_16, int *Bit15_08, int *Bit07_00)
{
    int msb_mask = 0;
    int msb_bit = total_bits - 16;

    int mask_count;
    for (mask_count = 0; mask_count < msb_bit; mask_count++)
    {
        msb_mask = (msb_mask<<1) + 1;
    }

    *Bit07_00 = (input_value&0xFF);
    *Bit15_08 = ((input_value>>8)&0xFF);
    *Bit23_16 = ((input_value>>16)&msb_mask);
}


void coeff_and_ks(int diff, int *coeff, int *ks)
{
    double target = (double)(1.0/(double)diff);

    int inv_20 = (int)((double)(1<<20)/(double)diff + 0.5);
    int inv_22 = (int)((double)(1<<22)/(double)diff + 0.5);

    inv_20 = minmax_( inv_20, 0x0, 0x3FFF);
    inv_22 = minmax_( inv_22, 0x0, 0x3FFF);

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


void FIND_MIN_AND_MAX(int RGB_channel_idx, int block_width, int block_height, strgb_structInfo *layer_blk_img, double *rgb_max, double *rgb_min)
{

    // input lut value is base on 10bit data path
    double rgb_max_tmp = 0.0;
    double rgb_min_tmp = 4095.0;

    int CurV, CurH;
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {

            double val = 0;
            switch (RGB_channel_idx)
            {
            case 0:  // R channel
                val = layer_blk_img[posi_blk(CurV, CurH)].r * 4;
                break;
            case 1:  // G channel
                val = layer_blk_img[posi_blk(CurV, CurH)].g * 4;
                break;
            case 2:  // B channel
                val = layer_blk_img[posi_blk(CurV, CurH)].b * 4;
                break;
            }

            rgb_max_tmp = max_(rgb_max_tmp, val);
            rgb_min_tmp = min_(rgb_min_tmp, val);

        }
    }

    *rgb_max = rgb_max_tmp;
    *rgb_min = rgb_min_tmp;
}

void cal_gain_mag(double rgb_max_diff_min_half, int *rgb_gain, int *rgb_dmc_data_mag)
{
	if(rgb_max_diff_min_half<128.0)      // base on 12bit data path , lut value format T7.4 bit => 10bit data path, T5.6
    {
        *rgb_gain = 0;
        *rgb_dmc_data_mag = 0;
    }
    else if(rgb_max_diff_min_half<256.0)  // base on 12bit data path , lut value format T8.3 bit => 10bit data path, T6.5
    {
        *rgb_gain = 1;
        *rgb_dmc_data_mag = 1;
    }
    else if(rgb_max_diff_min_half<512.0)   // base on 12bit data path , lut value format T9.2 bit => 10bit data path, T7.4
    {
        *rgb_gain = 2;
        *rgb_dmc_data_mag = 2;
    }
    else if(rgb_max_diff_min_half<1024.0)   // base on 12bit data path , lut value format T10.1 bit => 10bit data path, T8.3
    {
        *rgb_gain = 3;
        *rgb_dmc_data_mag = 3;
    }
    else if(rgb_max_diff_min_half<2048.0)   // base on 12bit data path , lut value format T11.0 bit => 10bit data path, T9.2
    {
        *rgb_gain = 4;
        *rgb_dmc_data_mag = 4;
    }
    else
    {
        *rgb_gain = 4;
        *rgb_dmc_data_mag = 4;
    }
}

void cal_gain_mag_INT(int rgb_max_diff_min_half, int *rgb_gain, int *rgb_dmc_data_mag)
{

    if(rgb_max_diff_min_half < 128) // base on 10bit data path , lut value format T3.4 bit
    {
        *rgb_gain = 0;
        *rgb_dmc_data_mag = 6;
    }
    else if(rgb_max_diff_min_half < 255) // base on 10bit data path , lut value format T4.3 bit
    {
        *rgb_gain = 1;
        *rgb_dmc_data_mag = 7;
    }
    else if(rgb_max_diff_min_half < 513) // base on 10bit data path , lut value format T5.2 bit
    {
        *rgb_gain = 2;
        *rgb_dmc_data_mag = 0;
    }
    else if(rgb_max_diff_min_half < 1025)   // base on 10bit data path , lut value format T6.1 bit
    {
        *rgb_gain = 3;
        *rgb_dmc_data_mag = 1;
    }
    else if(rgb_max_diff_min_half<=2048/*512/4.0*/)   // base on 10bit data path , lut value format T7.0 bit
    {
        *rgb_gain = 4;
        *rgb_dmc_data_mag = 2;
    }
    else if(rgb_max_diff_min_half < 4097)  // base on 10bit data path , lut value format T8.0 bit
    {
        *rgb_gain = 5;
        *rgb_dmc_data_mag = 3;
    }
    else //if(rgb_max_diff_min_half<=1016)    // base on 10bit data path , lut value format T9.0 bit
    {
        *rgb_gain = 6;
        *rgb_dmc_data_mag = 4;
    }

}

void FIND_MIN_AND_MAX_INT(int RGB_channel_idx, int block_width, int block_height, strgb_structInfo *layer_blk_img, int *rgb_max, int *rgb_min)
{
    // input lut value is base on 10bit data path
    int rgb_max_tmp = 0;
    int rgb_min_tmp = 1024*16-1;//test 1023 -> 14bit;

    int CurV, CurH;
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {

            int val = 0;
            switch (RGB_channel_idx)
            {
            case 0:  // R channel
                val = layer_blk_img[posi_blk(CurV, CurH)].r;
                break;
            case 1:  // G channel
                val = layer_blk_img[posi_blk(CurV, CurH)].g;
#if 0
                if ((CurH < 10) || (CurH > (block_width - 10)))
                {
                    printf("%s@%d h:%d v:%d val = 0x%x\n", __FUNCTION__,__LINE__,CurV, CurH, val);
                }
#endif
                break;
            case 2:  // B channel
                val = layer_blk_img[posi_blk(CurV, CurH)].b;
                break;
#if defined CONFIG_DEMURA_RGBW
            case 3:  // W channel
                val = layer_blk_img[posi_blk(CurV, CurH)].w;
                break;
#endif
            }

            rgb_max_tmp = max_(rgb_max_tmp, val);
            rgb_min_tmp = min_(rgb_min_tmp, val);

        }
    }

    *rgb_max = rgb_max_tmp;
    *rgb_min = rgb_min_tmp;
}

void LUT_SETTING_SUBFUNCTION_S7(int RGB_idx, int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg, strgb_structInfo *layer_blk_img, strgb_structInfo *layer_blk_img_out)
{
    // input lut value is base on 10bit data path
    int rgb_max = 0;
    int rgb_min = 1023;
    FIND_MIN_AND_MAX_INT( RGB_idx, block_width, block_height, layer_blk_img, &rgb_max, &rgb_min);

    int rgb_max_diff_min_half = (rgb_max - rgb_min)* 8; /* (rgb_max - rgb_min)/2*/

    int rgb_max_min_avg = (rgb_max + rgb_min)/2;

    int rgb_gain = 0; // x1
    int rgb_dmc_data_mag = 6;

    cal_gain_mag_INT(rgb_max_diff_min_half, &rgb_gain, &rgb_dmc_data_mag);

    // input data is 10 bit , but reg_dmc_plane_level is 12 bit , so shift 2 bit to 10 bit
    int reg_dmc_data_rgb_offset = rgb_max_min_avg - reg_dmc_plane_level/4;
    int reg_dmc_data_rgb_offset_int = (int)((reg_dmc_data_rgb_offset*4)*4); // 10 bit -> 12 bit -> 12.2bit

    reg_dmc_data_rgb_offset_int = minmax_(reg_dmc_data_rgb_offset_int, -8192, 8191);

    int reg_dmc_data_rgb_offset_int_2s = (reg_dmc_data_rgb_offset_int < 0) ? (16384 + reg_dmc_data_rgb_offset_int) : reg_dmc_data_rgb_offset_int; //T11.2


    //{------------------------ mag and offset register start ------------------------//
    //int Bit15_08, Bit07_00;

    switch(plane_idx)
    {
    case 0:
        switch (RGB_idx)
        {
        case 0: // plane 0 , R channel
            (*preg).reg_dmc_data_r_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset1.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 0 , G channel
            (*preg).reg_dmc_data_g_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset1.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 0 , B channel
            (*preg).reg_dmc_data_b_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset1.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 1:
        switch (RGB_idx)
        {
        case 0: // plane 1 , R channel
            (*preg).reg_dmc_data_r_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset2.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 1 , G channel
            (*preg).reg_dmc_data_g_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset2.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 1 , B channel
            (*preg).reg_dmc_data_b_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset2.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 2:
        switch (RGB_idx)
        {
        case 0: // plane 2 , R channel
            (*preg).reg_dmc_data_r_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset3.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 2 , G channel
            (*preg).reg_dmc_data_g_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset3.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 2 , B channel
            (*preg).reg_dmc_data_b_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset3.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 3:
        switch (RGB_idx)
        {
        case 0: // plane 3 , R channel
            (*preg).reg_dmc_data_r_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset4.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 3 , G channel
            (*preg).reg_dmc_data_g_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset4.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 3 , B channel
            (*preg).reg_dmc_data_b_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset4.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 4:
        switch (RGB_idx)
        {
        case 0: // plane 4 , R channel
            (*preg).reg_dmc_data_r_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset5.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 4 , G channel
            (*preg).reg_dmc_data_g_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset5.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 4 , B channel
            (*preg).reg_dmc_data_b_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset5.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 5:
        switch (RGB_idx)
        {
        case 0: // plane 5 , R channel
            (*preg).reg_dmc_data_r_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset6.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 5 , G channel
            (*preg).reg_dmc_data_g_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset6.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 5 , B channel
            (*preg).reg_dmc_data_b_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset6.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 6:
        switch (RGB_idx)
        {
        case 0: // plane 6, R channel
            (*preg).reg_dmc_data_r_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset7.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 6, G channel
            (*preg).reg_dmc_data_g_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset7.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 6, B channel
            (*preg).reg_dmc_data_b_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset7.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    case 7:
        switch (RGB_idx)
        {
        case 0: // plane 8, R channel
            (*preg).reg_dmc_data_r_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset8.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 1: // plane 8, G channel
            (*preg).reg_dmc_data_g_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset8.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        case 2: // plane 8, B channel
            (*preg).reg_dmc_data_b_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset8.val = reg_dmc_data_rgb_offset_int_2s;
            break;
        }
        break;
    }
    //}------------------------ mag and offset register end ------------------------//


    //{----------------------- LUT start -----------------------//
    int CurV, CurH;
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {
            double rgb_tmp = 0;

            switch (RGB_idx)
            {
            case 0:  // R channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].r;
                break;
            case 1:  // G channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].g;
                break;
            case 2:  // B channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].b;
                break;
            }
            rgb_tmp = rgb_tmp - rgb_max_min_avg;  // double 10bit - double 10bit

            switch(rgb_gain)
            {
            case 0: // format T3.4 (T00xxx.xxxx)
                rgb_tmp = (rgb_tmp*16);
                break;
            case 1: // format T4.3 (T0xxxx.xxx)
                rgb_tmp = (rgb_tmp*8);
                break;
            case 2: // format T5.2 (Txxxxx.xx)
                rgb_tmp = (rgb_tmp*4);
                break;
            case 3: // format T6.1 (Txxxxxx.x)
                rgb_tmp = (rgb_tmp*2);
                break;
            case 4: // format T7.0 (Txxxxxxx.0)
                rgb_tmp = rgb_tmp;
                break;
            case 5: // format T8.0 (Txxxxxxx0.0)
                rgb_tmp = rgb_tmp/2;
                break;
            case 6: // format T9.0 (Txxxxxxx00.0)
                rgb_tmp = rgb_tmp/4;
                break;
            }

            //int rgb_db = (int)(rgb_tmp*4)/4;

            int rgb_int = (int)(rgb_tmp);

            rgb_int = minmax_(rgb_int,-128,127);

            switch (RGB_idx)
            {
            case 0:  // R channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbr = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].r   = rgb_int;
                break;
            case 1:  // G channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbg = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].g   = rgb_int;
                break;
            case 2:  // B channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbb = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].b   = rgb_int;
                break;
            }
        }
    }
    //}----------------------- LUT end -----------------------//
}

void LUT_SETTING_SUBFUNCTION_S11(int RGB_idx, int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg, strgb_structInfo *layer_blk_img, strgb_structInfo *layer_blk_img_out)
{
    // input lut value is base on 10bit data path
    double rgb_max = 0.0;
    double rgb_min = 4095.0;
    FIND_MIN_AND_MAX( RGB_idx, block_width, block_height, layer_blk_img, &rgb_max, &rgb_min);

    double rgb_max_diff_min_half = (double)(rgb_max - rgb_min)/2.0;

    double rgb_max_min_avg = (double)(rgb_max + rgb_min)/2.0;

    int rgb_gain = 0; // x1
    int rgb_dmc_data_mag = 0;

    // convert 10 bit to 12 bit
    for (int CurV = 0; CurV < block_height; CurV++)
    {
        for (int CurH = 0; CurH < block_width; CurH++)
        {
            switch (RGB_idx)
            {
            case 0:  // R channel
                //layer_blk_img[posi_blk(CurV,CurH)].dbr *= 4;
                layer_blk_img[posi_blk(CurV,CurH)].r *= 4;
                break;
            case 1:  // G channel
                //layer_blk_img[posi_blk(CurV,CurH)].dbg *= 4;
                layer_blk_img[posi_blk(CurV,CurH)].g *= 4;
                break;
            case 2:  // B channel
                //layer_blk_img[posi_blk(CurV,CurH)].dbb *= 4;
                layer_blk_img[posi_blk(CurV,CurH)].b *= 4;
                break;
            }
        }
    }

    cal_gain_mag(rgb_max_diff_min_half, &rgb_gain, &rgb_dmc_data_mag);

    // input data is 10 bit , but reg_dmc_plane_level is 12 bit , so shift 2 bit to 10 bit
    double reg_dmc_data_rgb_offset = rgb_max_min_avg - (double)(reg_dmc_plane_level);

    int reg_dmc_data_rgb_offset_int = (int)(reg_dmc_data_rgb_offset*4); // 10 bit -> 12 bit -> 12.2bit

    reg_dmc_data_rgb_offset_int = minmax_(reg_dmc_data_rgb_offset_int, -32768, 32767);//brown test minmax_(reg_dmc_data_rgb_offset_int, -8192, 8191);

    int reg_dmc_data_rgb_offset_int_2s = (reg_dmc_data_rgb_offset_int < 0) ? (65536 + reg_dmc_data_rgb_offset_int) : reg_dmc_data_rgb_offset_int; //brown test (reg_dmc_data_rgb_offset_int < 0) ? (16384 + reg_dmc_data_rgb_offset_int) : reg_dmc_data_rgb_offset_int; //T11.2


    //------------------------ mag and offset register start ------------------------//
    //int Bit15_08, Bit07_00;


    switch(plane_idx)
    {
    case 0:
        switch (RGB_idx)
        {
        case 0: // plane 0 , R channel
            (*preg).reg_dmc_data_r_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset1.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_r_mag1.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag1.Bit07_00.mask, rgb_dmc_data_mag);

            //part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            //(*preg).reg_dmc_data_r_offset1.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset1.Bit07_00.mask, Bit07_00);
            //(*preg).reg_dmc_data_r_offset1.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset1.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 0 , G channel
            (*preg).reg_dmc_data_g_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset1.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_g_mag1.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag1.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset1.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset1.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset1.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset1.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 0 , B channel
            (*preg).reg_dmc_data_b_mag1.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset1.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag1.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag1.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset1.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset1.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset1.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset1.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 1:
        switch (RGB_idx)
        {
        case 0: // plane 1 , R channel
            (*preg).reg_dmc_data_r_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset2.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag2.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag2.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset2.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset2.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset2.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset2.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 1 , G channel
            (*preg).reg_dmc_data_g_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset2.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_g_mag2.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag2.Bit07_00.mask, rgb_dmc_data_mag);

            //part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            //(*preg).reg_dmc_data_g_offset2.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset2.Bit07_00.mask, Bit07_00);
            //(*preg).reg_dmc_data_g_offset2.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset2.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 1 , B channel
            (*preg).reg_dmc_data_b_mag2.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset2.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_b_mag2.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag2.Bit07_00.mask, rgb_dmc_data_mag);

            //part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 14, &Bit15_08, &Bit07_00);
            //(*preg).reg_dmc_data_b_offset2.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset2.Bit07_00.mask, Bit07_00);
            //(*preg).reg_dmc_data_b_offset2.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset2.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 2:
        switch (RGB_idx)
        {
        case 0: // plane 2 , R channel
            (*preg).reg_dmc_data_r_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset3.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_r_mag3.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag3.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset3.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset3.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset3.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset3.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 2 , G channel
            (*preg).reg_dmc_data_g_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset3.val = reg_dmc_data_rgb_offset_int_2s;

            //(*preg).reg_dmc_data_g_mag3.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag3.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset3.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset3.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset3.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset3.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 2 , B channel
            (*preg).reg_dmc_data_b_mag3.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset3.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag3.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag3.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset3.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset3.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset3.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset3.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 3:
        switch (RGB_idx)
        {
        case 0: // plane 3 , R channel
            (*preg).reg_dmc_data_r_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset4.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag4.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag4.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset4.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset4.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset4.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset4.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 3 , G channel
            (*preg).reg_dmc_data_g_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset4.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag4.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag4.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset4.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset4.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset4.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset4.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 3 , B channel
            (*preg).reg_dmc_data_b_mag4.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset4.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag4.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag4.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset4.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset4.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset4.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset4.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 4:
        switch (RGB_idx)
        {
        case 0: // plane 4 , R channel
            (*preg).reg_dmc_data_r_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset5.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag5.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag5.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset5.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset5.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset5.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset5.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 4 , G channel
            (*preg).reg_dmc_data_g_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset5.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag5.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag5.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset5.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset5.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset5.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset5.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 4 , B channel
            (*preg).reg_dmc_data_b_mag5.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset5.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag5.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag5.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset5.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset5.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset5.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset5.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 5:
        switch (RGB_idx)
        {
        case 0: // plane 5 , R channel
            (*preg).reg_dmc_data_r_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset6.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag6.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag6.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset6.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset6.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset6.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset6.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane 5 , G channel
            (*preg).reg_dmc_data_g_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset6.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag6.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag6.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset6.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset6.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset6.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset6.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane 5 , B channel
            (*preg).reg_dmc_data_b_mag6.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset6.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag6.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag6.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset6.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset6.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset6.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset6.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 6:
        switch (RGB_idx)
        {
        case 0: // plane  , R channel
            (*preg).reg_dmc_data_r_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset7.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag7.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag7.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset7.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset7.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset7.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset7.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane  , G channel
            (*preg).reg_dmc_data_g_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset7.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag7.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag7.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset7.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset7.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset7.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset7.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane  , B channel
            (*preg).reg_dmc_data_b_mag7.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset7.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag7.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag7.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset7.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset7.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset7.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset7.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 7:
        switch (RGB_idx)
        {
        case 0: // plane  , R channel
            (*preg).reg_dmc_data_r_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset8.val = reg_dmc_data_rgb_offset_int_2s;
            // (*preg).reg_dmc_data_r_mag8.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag8.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_r_offset8.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset8.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset8.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset8.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane  , G channel
            (*preg).reg_dmc_data_g_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset8.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag8.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag8.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_g_offset8.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset8.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset8.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset8.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane  , B channel
            (*preg).reg_dmc_data_b_mag8.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset8.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag8.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag8.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, &Bit15_08, &Bit07_00);
            // (*preg).reg_dmc_data_b_offset8.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset8.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset8.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset8.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    case 8:
        switch (RGB_idx)
        {
        case 0: // plane  , R channel
            (*preg).reg_dmc_data_r_mag9.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_r_offset9.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_r_mag9.Bit07_00.value               = register_shift((*preg).reg_dmc_data_r_mag9.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, Bit15_08, Bit07_00);
            // (*preg).reg_dmc_data_r_offset9.Bit07_00.value             = register_shift((*preg).reg_dmc_data_r_offset9.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_r_offset9.Bit15_08.value             = register_shift((*preg).reg_dmc_data_r_offset9.Bit15_08.mask, Bit15_08);
            break;
        case 1: // plane  , G channel
            (*preg).reg_dmc_data_g_mag9.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_g_offset9.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_g_mag9.Bit07_00.value               = register_shift((*preg).reg_dmc_data_g_mag9.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, Bit15_08, Bit07_00);
            // (*preg).reg_dmc_data_g_offset9.Bit07_00.value             = register_shift((*preg).reg_dmc_data_g_offset9.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_g_offset9.Bit15_08.value             = register_shift((*preg).reg_dmc_data_g_offset9.Bit15_08.mask, Bit15_08);
            break;
        case 2: // plane  , B channel
            (*preg).reg_dmc_data_b_mag9.val = rgb_dmc_data_mag;
            (*preg).reg_dmc_data_b_offset9.val = reg_dmc_data_rgb_offset_int_2s;

            // (*preg).reg_dmc_data_b_mag9.Bit07_00.value               = register_shift((*preg).reg_dmc_data_b_mag9.Bit07_00.mask, rgb_dmc_data_mag);

            // part_bit15_00( reg_dmc_data_rgb_offset_int_2s, 16, Bit15_08, Bit07_00);
            // (*preg).reg_dmc_data_b_offset9.Bit07_00.value             = register_shift((*preg).reg_dmc_data_b_offset9.Bit07_00.mask, Bit07_00);
            // (*preg).reg_dmc_data_b_offset9.Bit15_08.value             = register_shift((*preg).reg_dmc_data_b_offset9.Bit15_08.mask, Bit15_08);
            break;
        }
        break;
    }
    //------------------------ mag and offset register end ------------------------//


    //----------------------- LUT start -----------------------//
    int CurV, CurH;
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {
            double rgb_tmp = 0;

            switch (RGB_idx)
            {
            case 0:  // R channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].r;
                break;
            case 1:  // G channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].g;
                break;
            case 2:  // B channel
                rgb_tmp = layer_blk_img[posi_blk(CurV, CurH)].b;
                break;
            }
            rgb_tmp = rgb_tmp - rgb_max_min_avg;  // double 10bit - double 10bit

            switch(rgb_gain)
            {
            case 0: // format T3.4 (T00xxx.xxxx)
                rgb_tmp = ((double)rgb_tmp*(double)16);
                break;
            case 1: // format T4.3 (T0xxxx.xxx)
                rgb_tmp = ((double)rgb_tmp*(double)8);
                break;
            case 2: // format T5.2 (Txxxxx.xx)
                rgb_tmp = ((double)rgb_tmp*(double)4);
                break;
            case 3: // format T6.1 (Txxxxxx.x)
                rgb_tmp = ((double)rgb_tmp*(double)2);
                break;
            case 4: default:// format T7.0 (Txxxxxxx.0)
                rgb_tmp = rgb_tmp;
                break;
            }

            //double rgb_db = (double)((double)((int)(rgb_tmp*4))/4.0);

            int rgb_int = (int)(rgb_tmp);

            rgb_int = minmax_(rgb_int,-2048,2047);

            switch (RGB_idx)
            {
            case 0:  // R channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbr = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].r   = rgb_int;
                break;
            case 1:  // G channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbg = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].g   = rgb_int;
                break;
            case 2:  // B channel
                //layer_blk_img_out[posi_blk(CurV,CurH)].dbb = rgb_db;
                layer_blk_img_out[posi_blk(CurV,CurH)].b   = rgb_int;
                break;
            }
        }
    }
    //----------------------- LUT end -----------------------//
}

typedef void (*FUNCP_LUT_SETTING_SUBFUNCTION)(int RGB_idx, int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg, strgb_structInfo *layer_blk_img, strgb_structInfo *layer_blk_img_out);
FUNCP_LUT_SETTING_SUBFUNCTION g_handle_lutsetting[E_DEMURA_VERSION_MAX] ={
    LUT_SETTING_SUBFUNCTION_S11,  //E_DEMURA_VERSION_S11
    LUT_SETTING_SUBFUNCTION_S7,   //E_DEMURA_VERSION_S7
    LUT_SETTING_SUBFUNCTION_S7,   //E_DEMURA_VERSION_S7_MONO
};
void LUT_AND_SETTING_PROCESSOR(int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg , strgb_structInfo *layer_blk_img_in, strgb_structInfo *layer_blk_img_out, int reg_dmc_rgb_mode)
{
    int i;
    if (g_handle_lutsetting[demura_get_version()] != NULL)
    {
        if (demura_get_version() == E_DEMURA_VERSION_S7_MONO && reg_dmc_rgb_mode == 1) // 0 : mono mode , 1 : rgb mode
        {
            g_handle_lutsetting[demura_get_version()] (1, plane_idx, block_width, block_height, reg_dmc_plane_level, preg, layer_blk_img_in, layer_blk_img_out);
            switch (plane_idx)
            {
            case _0:
                (*preg).reg_dmc_data_r_mag1.val = (*preg).reg_dmc_data_g_mag1.val;
                (*preg).reg_dmc_data_r_offset1.val = (*preg).reg_dmc_data_g_offset1.val;
                (*preg).reg_dmc_data_b_mag1.val = (*preg).reg_dmc_data_g_mag1.val;
                (*preg).reg_dmc_data_b_offset1.val = (*preg).reg_dmc_data_g_offset1.val;
                break;
            case _1:
                (*preg).reg_dmc_data_r_mag2.val = (*preg).reg_dmc_data_g_mag2.val;
                (*preg).reg_dmc_data_r_offset2.val = (*preg).reg_dmc_data_g_offset2.val;
                (*preg).reg_dmc_data_b_mag2.val = (*preg).reg_dmc_data_g_mag2.val;
                (*preg).reg_dmc_data_b_offset2.val = (*preg).reg_dmc_data_g_offset2.val;
                break;
            case _2:
                (*preg).reg_dmc_data_r_mag3.val = (*preg).reg_dmc_data_g_mag3.val;
                (*preg).reg_dmc_data_r_offset3.val = (*preg).reg_dmc_data_g_offset3.val;
                (*preg).reg_dmc_data_b_mag3.val = (*preg).reg_dmc_data_g_mag3.val;
                (*preg).reg_dmc_data_b_offset3.val = (*preg).reg_dmc_data_g_offset3.val;
                break;
            case _3:
                (*preg).reg_dmc_data_r_mag4.val = (*preg).reg_dmc_data_g_mag4.val;
                (*preg).reg_dmc_data_r_offset4.val = (*preg).reg_dmc_data_g_offset4.val;
                (*preg).reg_dmc_data_b_mag4.val = (*preg).reg_dmc_data_g_mag4.val;
                (*preg).reg_dmc_data_b_offset4.val = (*preg).reg_dmc_data_g_offset4.val;
                break;
            case _4:
                (*preg).reg_dmc_data_r_mag5.val = (*preg).reg_dmc_data_g_mag5.val;
                (*preg).reg_dmc_data_r_offset5.val = (*preg).reg_dmc_data_g_offset5.val;
                (*preg).reg_dmc_data_b_mag5.val = (*preg).reg_dmc_data_g_mag5.val;
                (*preg).reg_dmc_data_b_offset5.val = (*preg).reg_dmc_data_g_offset5.val;
                break;
            case _5:
                (*preg).reg_dmc_data_r_mag6.val = (*preg).reg_dmc_data_g_mag6.val;
                (*preg).reg_dmc_data_r_offset6.val = (*preg).reg_dmc_data_g_offset6.val;
                (*preg).reg_dmc_data_b_mag6.val = (*preg).reg_dmc_data_g_mag6.val;
                (*preg).reg_dmc_data_b_offset6.val = (*preg).reg_dmc_data_g_offset6.val;
                break;
            case _6:
                (*preg).reg_dmc_data_r_mag7.val = (*preg).reg_dmc_data_g_mag7.val;
                (*preg).reg_dmc_data_r_offset7.val = (*preg).reg_dmc_data_g_offset7.val;
                (*preg).reg_dmc_data_b_mag7.val = (*preg).reg_dmc_data_g_mag7.val;
                (*preg).reg_dmc_data_b_offset7.val = (*preg).reg_dmc_data_g_offset7.val;
                break;
            case _7:
                (*preg).reg_dmc_data_r_mag8.val = (*preg).reg_dmc_data_g_mag8.val;
                (*preg).reg_dmc_data_r_offset8.val = (*preg).reg_dmc_data_g_offset8.val;
                (*preg).reg_dmc_data_b_mag8.val = (*preg).reg_dmc_data_g_mag8.val;
                (*preg).reg_dmc_data_b_offset8.val = (*preg).reg_dmc_data_g_offset8.val;
                break;
            }
        }
        else
        {
            for (i = 0; i < _3; i++)  // R/G/B channel
            {
                g_handle_lutsetting[demura_get_version()] (i, plane_idx, block_width, block_height, reg_dmc_plane_level, preg, layer_blk_img_in, layer_blk_img_out);
            }
        }
    }

/*
    #ifdef CONFIG_DEMURA_VENDOR_LGD
    UBOOT_DEBUG("LGD to Mstar, Call bit_mode_LPF\n");
    bit_mode_LPF(plane_idx, block_width, block_height, layer_blk_img_out);
    #endif
    */
}

//For ccn check
void write_demuraLUT_to_HW_DRAM_FORMAT_S11(stlayer_info_structInfo *layer_info, strgb_structInfo *layer_blk_img[8], m_vector *pLut_out_vector)
{
    int lut_vidx, lut_hidx, plane_idx, idx;
    int out_idx = 0; // For Lut_out arrary
    char *Lut_out = (char *)(pLut_out_vector->pbuf) + pLut_out_vector->dat_num;
    ADL_FORMAT_S11_MONO *adl_out = (ADL_FORMAT_S11_MONO *)Lut_out;
    ADL_FORMAT_S11_RGB *adl_rgbout = (ADL_FORMAT_S11_RGB *)Lut_out;
#if 0
    MS_U8 *pu8DbgData;
#endif
    u16 L[21];
    memset(&L[0], 0, sizeof(L));

#if 0
    printf("%s@%d v:%d h:%d   h:%d v:%d  h:%d v:%d  rgb_mode:%u\n", __FUNCTION__,__LINE__,
        layer_info->real_dram_v_size, layer_info->real_dram_h_size,
        layer_info->block_h_size, layer_info->block_v_size,
        layer_info->dram_h_size, layer_info->dram_v_size, layer_info->dmc_rgb_mode);
#endif

    if(layer_info->dmc_rgb_mode == DEMURA_MODE_RGB)
    {
        for (plane_idx = 0; plane_idx < layer_info->dmc_plane_num; plane_idx++)
        {
            for (lut_vidx = 0; lut_vidx < layer_info->dram_v_size; lut_vidx++)
            {
                for (lut_hidx = 0; lut_hidx < layer_info->dram_h_size; lut_hidx += 7)
                {
                    int lut_idx = MIN(layer_info->dram_h_size * lut_vidx + lut_hidx, layer_info->dram_v_size * layer_info->dram_h_size);
                    for (idx = 0; idx < 7; idx++)
                    {
                        DMC_LAYER_IMG_S11(layer_blk_img[plane_idx][lut_idx + idx].r, layer_blk_img[plane_idx][lut_idx + idx].r);
                        DMC_LAYER_IMG_S11(layer_blk_img[plane_idx][lut_idx + idx].g, layer_blk_img[plane_idx][lut_idx + idx].g);
                        DMC_LAYER_IMG_S11(layer_blk_img[plane_idx][lut_idx + idx].b, layer_blk_img[plane_idx][lut_idx + idx].b);
//                        DMC_LAYER_IMG_S11(layer_blk_img[plane_idx][lut_idx + idx].w, layer_blk_img[plane_idx][lut_idx + idx].w);
                    }
                    adl_rgbout[out_idx].r_in_00 = layer_blk_img[plane_idx][lut_idx + 0].r;
                    adl_rgbout[out_idx].g_in_00 = layer_blk_img[plane_idx][lut_idx + 0].g;
                    adl_rgbout[out_idx].b_in_00_l = layer_blk_img[plane_idx][lut_idx + 0].b & 0xFF;
                    adl_rgbout[out_idx].b_in_00_h = (layer_blk_img[plane_idx][lut_idx + 0].b & 0xF00) >> 8;
                    adl_rgbout[out_idx].r_in_01 = layer_blk_img[plane_idx][lut_idx + 1].r;
                    adl_rgbout[out_idx].g_in_01 = layer_blk_img[plane_idx][lut_idx + 1].g; 
                    adl_rgbout[out_idx].b_in_01_l = layer_blk_img[plane_idx][lut_idx + 1].b & 0xF;
                    adl_rgbout[out_idx].b_in_01_h = (layer_blk_img[plane_idx][lut_idx + 1].b & 0xFF0) >> 4;
                    adl_rgbout[out_idx].r_in_02 = layer_blk_img[plane_idx][lut_idx + 2].r;
                    adl_rgbout[out_idx].g_in_02 = layer_blk_img[plane_idx][lut_idx + 2].g;
                    adl_rgbout[out_idx].b_in_02 = layer_blk_img[plane_idx][lut_idx + 2].b;
                    adl_rgbout[out_idx].r_in_03 = layer_blk_img[plane_idx][lut_idx + 3].r;
                    adl_rgbout[out_idx].g_in_03_l = layer_blk_img[plane_idx][lut_idx + 3].g & 0xFF;
                    adl_rgbout[out_idx].g_in_03_h = (layer_blk_img[plane_idx][lut_idx + 3].g & 0xF00) >> 8;
                    adl_rgbout[out_idx].b_in_03 = layer_blk_img[plane_idx][lut_idx + 3].b;
                    adl_rgbout[out_idx].r_in_04 = layer_blk_img[plane_idx][lut_idx + 4].r;
                    adl_rgbout[out_idx].g_in_04_l = layer_blk_img[plane_idx][lut_idx + 4].g & 0xF;
                    adl_rgbout[out_idx].g_in_04_h = (layer_blk_img[plane_idx][lut_idx + 4].g & 0xFF0) >> 4;
                    adl_rgbout[out_idx].b_in_04 = layer_blk_img[plane_idx][lut_idx + 4].b;
                    adl_rgbout[out_idx].r_in_05 = layer_blk_img[plane_idx][lut_idx + 5].r;
                    adl_rgbout[out_idx].g_in_05 = layer_blk_img[plane_idx][lut_idx + 5].g;
                    adl_rgbout[out_idx].b_in_05 = layer_blk_img[plane_idx][lut_idx + 5].b;
                    adl_rgbout[out_idx].r_in_06_l = layer_blk_img[plane_idx][lut_idx + 6].r & 0xFF;
                    adl_rgbout[out_idx].r_in_06_h = (layer_blk_img[plane_idx][lut_idx + 6].r & 0xF00) >> 8;
                    adl_rgbout[out_idx].g_in_06 = layer_blk_img[plane_idx][lut_idx + 6].g;
                    adl_rgbout[out_idx].b_in_06 = layer_blk_img[plane_idx][lut_idx + 6].b;
                    adl_rgbout[out_idx].layer_num = plane_idx + 1;
                    out_idx++;
                }
            }
        }
        pLut_out_vector->dat_num += out_idx * sizeof(ADL_FORMAT_S11_RGB);
    }
    else
    {
        for (plane_idx = 0; plane_idx < layer_info->dmc_plane_num; plane_idx++)
        {
            for (lut_vidx = 0; lut_vidx < layer_info->dram_v_size; lut_vidx++)
            {
                for (lut_hidx = 0; lut_hidx < layer_info->dram_h_size; lut_hidx += 21)
                {
                    int lut_off = layer_info->dram_h_size * lut_vidx;
                    int lut_idx = MIN(lut_off + lut_hidx,
                        layer_info->dram_v_size * layer_info->dram_h_size);
                    for (idx = 0; idx < 21; idx++)
                    {
                        if ((lut_hidx + idx) < layer_info->dram_h_size)
                        {
                            if (layer_blk_img[plane_idx][lut_idx + idx].g >= 0)
                            {
                                L[idx] = MIN(layer_blk_img[plane_idx][lut_idx + idx].g, 0x7FF);
                            }
                            else
                            {
                                if (layer_blk_img[plane_idx][lut_idx + idx].g < -2048)
                                {
                                    L[idx] = 0x800;
                                }
                                else
                                {
                                    L[idx] = (layer_blk_img[plane_idx][lut_idx + idx].g + 4096);
                                }
                            }
                        }
                        //printf("%s@%d L[%d]= %d %x\n",__FUNCTION__,__LINE__, idx, layer_blk_img[plane_idx][lut_idx + idx].g, L[idx]);
                    }
#if 0
                    pu8DbgData = (MS_U8 *) & adl_out[out_idx];
#endif
                    adl_out[out_idx].d_in_00 = L[0] & 0xFFF;
                    adl_out[out_idx].d_in_01 = L[1] & 0xFFF;
                    adl_out[out_idx].d_in_02_l = L[2] & 0xFF; 
                    adl_out[out_idx].d_in_02_h = (L[2] & 0xF00) >> 8;
                    adl_out[out_idx].d_in_03 = L[3];
                    adl_out[out_idx].d_in_04 = L[4]; 
                    adl_out[out_idx].d_in_05_l = L[5] & 0xF;
                    adl_out[out_idx].d_in_05_h = (L[5] & 0xFF0) >> 4;
                    adl_out[out_idx].d_in_06 = L[6];
                    adl_out[out_idx].d_in_07 = L[7];
                    adl_out[out_idx].d_in_08 = L[8];
                    adl_out[out_idx].d_in_09 = L[9];
                    adl_out[out_idx].d_in_10_l = (L[10] & 0xFF);
                    adl_out[out_idx].d_in_10_h = (L[10] & 0xF00) >> 8;
                    adl_out[out_idx].d_in_11 = L[11];
                    adl_out[out_idx].d_in_12 = L[12];
                    adl_out[out_idx].d_in_13_l = L[13] & 0xF; 
                    adl_out[out_idx].d_in_13_h = (L[13] & 0xFF0) >> 4;
                    adl_out[out_idx].d_in_14 = L[14];
                    adl_out[out_idx].d_in_15 = L[15];
                    adl_out[out_idx].d_in_16 = L[16];
                    adl_out[out_idx].d_in_17 = L[17];
                    adl_out[out_idx].d_in_18_l = L[18] & 0xFF;
                    adl_out[out_idx].d_in_18_h = (L[18] &0xF00) >> 8;
                    adl_out[out_idx].d_in_19 = L[19];
                    adl_out[out_idx].d_in_20 = L[20];
                    adl_out[out_idx].layer_num = plane_idx + 1;
#if 0
                    printf("%s@%d adl_out:%p:%d:%02X %02X %02X %02X ... %02X %02X %02X %02X\n",
                        __FUNCTION__,__LINE__, pu8DbgData, out_idx,
                        pu8DbgData[0],pu8DbgData[1], pu8DbgData[2], pu8DbgData[3],
                        pu8DbgData[28],pu8DbgData[29], pu8DbgData[30], pu8DbgData[31]);
#endif
                    out_idx++;
                }
            }
        }
        pLut_out_vector->dat_num += out_idx * sizeof(ADL_FORMAT_S11_MONO);
    }
    if (pLut_out_vector->dat_num > pLut_out_vector->max_num)
    {
        printf("\n\033[31;1mError !\033[0m\n");
        printf("Lut_out_vector data num  : 0x%x\n", (unsigned int)pLut_out_vector->dat_num);
        printf("Lut_out_vector max  num  : 0x%x\n", (unsigned int)pLut_out_vector->max_num);
    }

}

void write_demuraLUT_to_HW_DRAM_FORMAT_S7(stlayer_info_structInfo *layer_info, strgb_structInfo *layer_blk_img[8], m_vector *pLut_out_vector)
{
    int out_idx = 0; // For Lut_out arrary
    char *Lut_out = (char *)(pLut_out_vector->pbuf) + pLut_out_vector->dat_num;
#if 0
    printf("%s@%d v:%d h:%d   h:%d v:%d  h:%d v:%d  rgb_mode:%u\n", __FUNCTION__,__LINE__,
        layer_info->real_dram_v_size, layer_info->real_dram_h_size,
        layer_info->block_h_size, layer_info->block_v_size,
        layer_info->dram_h_size, layer_info->dram_v_size, layer_info->dmc_rgb_mode);
#endif
    if(layer_info->dmc_rgb_mode == 1)
    {
        int H_offset = 1;
        switch(layer_info->dmc_plane_num)
        {
        case 1:
        case 2:
            H_offset = 4;
            break;
        case 3:
        case 4:
            H_offset = 2;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            H_offset = 1;
            break;
        }


        int lut_vidx, lut_hidx;
        for (lut_vidx = 0; lut_vidx < layer_info->real_dram_v_size; lut_vidx++)
        {
            for (lut_hidx = 0; lut_hidx < layer_info->real_dram_h_size; lut_hidx++)
            {
                strgb_structInfo L[9];
                strgb_structInfo null_val;
                null_val.r = 0;
                null_val.g = 0;
                null_val.b = 0;
                //null_val.dbr = 0;
                //null_val.dbg = 0;
                //null_val.dbb = 0;


                int lut_idx0 = MIN(lut_hidx*H_offset + 0, layer_info->dram_h_size);
                int lut_idx1 = MIN(lut_hidx*H_offset + 1, layer_info->dram_h_size);
                int lut_idx2 = MIN(lut_hidx*H_offset + 2, layer_info->dram_h_size);
                int lut_idx3 = MIN(lut_hidx*H_offset + 3, layer_info->dram_h_size);

                lut_idx0 = (lut_idx0 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx0);
                lut_idx1 = (lut_idx1 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx1);
                lut_idx2 = (lut_idx2 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx2);
                lut_idx3 = (lut_idx3 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx3);

                memset(L, 0, sizeof(L));
                switch(layer_info->dmc_plane_num)
                {
                case 1:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = null_val;
                    L[3] = (lut_idx1 == -1) ? null_val : layer_blk_img[0][lut_idx1];
                    L[4] = null_val;
                    L[5] = (lut_idx2 == -1) ? null_val : layer_blk_img[0][lut_idx2];
                    L[6] = null_val;
                    L[7] = (lut_idx3 == -1) ? null_val : layer_blk_img[0][lut_idx3];
                    L[8] = null_val;
                    break;
                case 2:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx1 == -1) ? null_val : layer_blk_img[0][lut_idx1];
                    L[4] = (lut_idx1 == -1) ? null_val : layer_blk_img[1][lut_idx1];
                    L[5] = (lut_idx2 == -1) ? null_val : layer_blk_img[0][lut_idx2];
                    L[6] = (lut_idx2 == -1) ? null_val : layer_blk_img[1][lut_idx2];
                    L[7] = (lut_idx3 == -1) ? null_val : layer_blk_img[0][lut_idx3];
                    L[8] = (lut_idx3 == -1) ? null_val : layer_blk_img[1][lut_idx3];
                    break;
                case 3:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = null_val;
                    L[5] = (lut_idx1 == -1) ? null_val : layer_blk_img[0][lut_idx1];
                    L[6] = (lut_idx1 == -1) ? null_val : layer_blk_img[1][lut_idx1];
                    L[7] = (lut_idx1 == -1) ? null_val : layer_blk_img[2][lut_idx1];
                    L[8] = null_val;
                    break;
                case 4:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = (lut_idx0 == -1) ? null_val : layer_blk_img[3][lut_idx0];
                    L[5] = (lut_idx1 == -1) ? null_val : layer_blk_img[0][lut_idx1];
                    L[6] = (lut_idx1 == -1) ? null_val : layer_blk_img[1][lut_idx1];
                    L[7] = (lut_idx1 == -1) ? null_val : layer_blk_img[2][lut_idx1];
                    L[8] = (lut_idx1 == -1) ? null_val : layer_blk_img[3][lut_idx1];
                    break;
                case 5:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = (lut_idx0 == -1) ? null_val : layer_blk_img[3][lut_idx0];
                    L[5] = (lut_idx0 == -1) ? null_val : layer_blk_img[4][lut_idx0];
                    L[6] = null_val;
                    L[7] = null_val;
                    L[8] = null_val;
                    break;
                case 6:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = (lut_idx0 == -1) ? null_val : layer_blk_img[3][lut_idx0];
                    L[5] = (lut_idx0 == -1) ? null_val : layer_blk_img[4][lut_idx0];
                    L[6] = (lut_idx0 == -1) ? null_val : layer_blk_img[5][lut_idx0];
                    L[7] = null_val;
                    L[8] = null_val;
                    break;
                case 7:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = (lut_idx0 == -1) ? null_val : layer_blk_img[3][lut_idx0];
                    L[5] = (lut_idx0 == -1) ? null_val : layer_blk_img[4][lut_idx0];
                    L[6] = (lut_idx0 == -1) ? null_val : layer_blk_img[5][lut_idx0];
                    L[7] = (lut_idx0 == -1) ? null_val : layer_blk_img[6][lut_idx0];
                    L[8] = null_val;
                    break;
                case 8:
                    L[1] = (lut_idx0 == -1) ? null_val : layer_blk_img[0][lut_idx0];
                    L[2] = (lut_idx0 == -1) ? null_val : layer_blk_img[1][lut_idx0];
                    L[3] = (lut_idx0 == -1) ? null_val : layer_blk_img[2][lut_idx0];
                    L[4] = (lut_idx0 == -1) ? null_val : layer_blk_img[3][lut_idx0];
                    L[5] = (lut_idx0 == -1) ? null_val : layer_blk_img[4][lut_idx0];
                    L[6] = (lut_idx0 == -1) ? null_val : layer_blk_img[5][lut_idx0];
                    L[7] = (lut_idx0 == -1) ? null_val : layer_blk_img[6][lut_idx0];
                    L[8] = (lut_idx0 == -1) ? null_val : layer_blk_img[7][lut_idx0];
                    break;
                }

                int idx;
                for (idx = 1; idx <= 4; idx++)
                {
                    char rr = (char)((L[idx].r < 0) ? (256 + L[idx].r) : (L[idx].r));
                    char gg = (char)((L[idx].g < 0) ? (256 + L[idx].g) : (L[idx].g));
                    char bb = (char)((L[idx].b < 0) ? (256 + L[idx].b) : (L[idx].b));
                    Lut_out[out_idx++] = bb;
                    Lut_out[out_idx++] = gg;
                    Lut_out[out_idx++] = rr;
                }
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
                for (idx = 5; idx <= 8; idx++)
                {
                    char rr = (char)((L[idx].r < 0) ? (256 + L[idx].r) : (L[idx].r));
                    char gg = (char)((L[idx].g < 0) ? (256 + L[idx].g) : (L[idx].g));
                    char bb = (char)((L[idx].b < 0) ? (256 + L[idx].b) : (L[idx].b));
                    Lut_out[out_idx++] = bb;
                    Lut_out[out_idx++] = gg;
                    Lut_out[out_idx++] = rr;
                }
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
                Lut_out[out_idx++] = 0;
            }
        }

    }
    else
    {
        int H_offset = 8;
        switch(layer_info->dmc_plane_num)
        {
        case 1:
        case 2:
            H_offset = 16;
            break;
        case 3:
        case 4:
            H_offset = 8;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            H_offset = 4;
            break;
        }

        int lut_vidx, lut_hidx;
        for (lut_vidx = 0; lut_vidx < layer_info->real_dram_v_size; lut_vidx++)
        {
            for (lut_hidx = 0; lut_hidx < layer_info->real_dram_h_size; lut_hidx++)
            {
                int L[32]={0};
                int null_val = 0;

                int lut_idx00 = MIN(lut_hidx*H_offset +  0, layer_info->dram_h_size );
                int lut_idx01 = MIN(lut_hidx*H_offset +  1, layer_info->dram_h_size );
                int lut_idx02 = MIN(lut_hidx*H_offset +  2, layer_info->dram_h_size );
                int lut_idx03 = MIN(lut_hidx*H_offset +  3, layer_info->dram_h_size );
                int lut_idx04 = MIN(lut_hidx*H_offset +  4, layer_info->dram_h_size );
                int lut_idx05 = MIN(lut_hidx*H_offset +  5, layer_info->dram_h_size );
                int lut_idx06 = MIN(lut_hidx*H_offset +  6, layer_info->dram_h_size );
                int lut_idx07 = MIN(lut_hidx*H_offset +  7, layer_info->dram_h_size );
                int lut_idx08 = MIN(lut_hidx*H_offset +  8, layer_info->dram_h_size );
                int lut_idx09 = MIN(lut_hidx*H_offset +  9, layer_info->dram_h_size );
                int lut_idx10 = MIN(lut_hidx*H_offset + 10, layer_info->dram_h_size );
                int lut_idx11 = MIN(lut_hidx*H_offset + 11, layer_info->dram_h_size );
                int lut_idx12 = MIN(lut_hidx*H_offset + 12, layer_info->dram_h_size );
                int lut_idx13 = MIN(lut_hidx*H_offset + 13, layer_info->dram_h_size );
                int lut_idx14 = MIN(lut_hidx*H_offset + 14, layer_info->dram_h_size );
                int lut_idx15 = MIN(lut_hidx*H_offset + 15, layer_info->dram_h_size );

                lut_idx00 = (lut_idx00 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx00);
                lut_idx01 = (lut_idx01 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx01);
                lut_idx02 = (lut_idx02 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx02);
                lut_idx03 = (lut_idx03 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx03);
                lut_idx04 = (lut_idx04 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx04);
                lut_idx05 = (lut_idx05 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx05);
                lut_idx06 = (lut_idx06 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx06);
                lut_idx07 = (lut_idx07 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx07);
                lut_idx08 = (lut_idx08 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx08);
                lut_idx09 = (lut_idx09 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx09);
                lut_idx10 = (lut_idx10 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx10);
                lut_idx11 = (lut_idx11 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx11);
                lut_idx12 = (lut_idx12 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx12);
                lut_idx13 = (lut_idx13 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx13);
                lut_idx14 = (lut_idx14 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx14);
                lut_idx15 = (lut_idx15 == layer_info->dram_h_size) ? -1 : (lut_vidx * layer_info->dram_h_size + lut_idx15);


                switch(layer_info->dmc_plane_num)
                {
                case 1:
                    L[ 0] = (lut_idx00 == -1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = null_val;
                    L[ 2] = (lut_idx01 == -1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 3] = null_val;
                    L[ 4] = (lut_idx02 == -1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[ 5] = null_val;
                    L[ 6] = (lut_idx03 == -1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[ 7] = null_val;
                    L[ 8] = (lut_idx04 == -1) ? null_val : layer_blk_img[0][lut_idx04].g;
                    L[ 9] = null_val;
                    L[10] = (lut_idx05 == -1) ? null_val : layer_blk_img[0][lut_idx05].g;
                    L[11] = null_val;
                    L[12] = (lut_idx06 == -1) ? null_val : layer_blk_img[0][lut_idx06].g;
                    L[13] = null_val;
                    L[14] = (lut_idx07 == -1) ? null_val : layer_blk_img[0][lut_idx07].g;
                    L[15] = null_val;
                    L[16] = (lut_idx08 == -1) ? null_val : layer_blk_img[0][lut_idx08].g;
                    L[17] = null_val;
                    L[18] = (lut_idx09 == -1) ? null_val : layer_blk_img[0][lut_idx09].g;
                    L[19] = null_val;
                    L[20] = (lut_idx10 == -1) ? null_val : layer_blk_img[0][lut_idx10].g;
                    L[21] = null_val;
                    L[22] = (lut_idx11 == -1) ? null_val : layer_blk_img[0][lut_idx11].g;
                    L[23] = null_val;
                    L[24] = (lut_idx12 == -1) ? null_val : layer_blk_img[0][lut_idx12].g;
                    L[25] = null_val;
                    L[26] = (lut_idx13 == -1) ? null_val : layer_blk_img[0][lut_idx13].g;
                    L[27] = null_val;
                    L[28] = (lut_idx14 == -1) ? null_val : layer_blk_img[0][lut_idx14].g;
                    L[29] = null_val;
                    L[30] = (lut_idx15 == -1) ? null_val : layer_blk_img[0][lut_idx15].g;
                    L[31] = null_val;
                    break;
                case 2:
                    L[ 0] = (lut_idx00 == -1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00 == -1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx01 == -1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 3] = (lut_idx01 == -1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[ 4] = (lut_idx02 == -1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[ 5] = (lut_idx02 == -1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[ 6] = (lut_idx03 == -1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[ 7] = (lut_idx03 == -1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[ 8] = (lut_idx04 == -1) ? null_val : layer_blk_img[0][lut_idx04].g;
                    L[ 9] = (lut_idx04 == -1) ? null_val : layer_blk_img[1][lut_idx04].g;
                    L[10] = (lut_idx05 == -1) ? null_val : layer_blk_img[0][lut_idx05].g;
                    L[11] = (lut_idx05 == -1) ? null_val : layer_blk_img[1][lut_idx05].g;
                    L[12] = (lut_idx06 == -1) ? null_val : layer_blk_img[0][lut_idx06].g;
                    L[13] = (lut_idx06 == -1) ? null_val : layer_blk_img[1][lut_idx06].g;
                    L[14] = (lut_idx07 == -1) ? null_val : layer_blk_img[0][lut_idx07].g;
                    L[15] = (lut_idx07 == -1) ? null_val : layer_blk_img[1][lut_idx07].g;
                    L[16] = (lut_idx08 == -1) ? null_val : layer_blk_img[0][lut_idx08].g;
                    L[17] = (lut_idx08 == -1) ? null_val : layer_blk_img[1][lut_idx08].g;
                    L[18] = (lut_idx09 == -1) ? null_val : layer_blk_img[0][lut_idx09].g;
                    L[19] = (lut_idx09 == -1) ? null_val : layer_blk_img[1][lut_idx09].g;
                    L[20] = (lut_idx10 == -1) ? null_val : layer_blk_img[0][lut_idx10].g;
                    L[21] = (lut_idx10 == -1) ? null_val : layer_blk_img[1][lut_idx10].g;
                    L[22] = (lut_idx11 == -1) ? null_val : layer_blk_img[0][lut_idx11].g;
                    L[23] = (lut_idx11 == -1) ? null_val : layer_blk_img[1][lut_idx11].g;
                    L[24] = (lut_idx12 == -1) ? null_val : layer_blk_img[0][lut_idx12].g;
                    L[25] = (lut_idx12 == -1) ? null_val : layer_blk_img[1][lut_idx12].g;
                    L[26] = (lut_idx13 == -1) ? null_val : layer_blk_img[0][lut_idx13].g;
                    L[27] = (lut_idx13 == -1) ? null_val : layer_blk_img[1][lut_idx13].g;
                    L[28] = (lut_idx14 == -1) ? null_val : layer_blk_img[0][lut_idx14].g;
                    L[29] = (lut_idx14 == -1) ? null_val : layer_blk_img[1][lut_idx14].g;
                    L[30] = (lut_idx15 == -1) ? null_val : layer_blk_img[0][lut_idx15].g;
                    L[31] = (lut_idx15 == -1) ? null_val : layer_blk_img[1][lut_idx15].g;
                    break;
                case 3:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = null_val;
                    L[ 4] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 5] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[ 6] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[ 7] = null_val;
                    L[ 8] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[ 9] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[10] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[11] = null_val;
                    L[12] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[13] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[14] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[15] = null_val;
                    L[16] = (lut_idx04==-1) ? null_val : layer_blk_img[0][lut_idx04].g;
                    L[17] = (lut_idx04==-1) ? null_val : layer_blk_img[1][lut_idx04].g;
                    L[18] = (lut_idx04==-1) ? null_val : layer_blk_img[2][lut_idx04].g;
                    L[19] = null_val;
                    L[20] = (lut_idx05==-1) ? null_val : layer_blk_img[0][lut_idx05].g;
                    L[21] = (lut_idx05==-1) ? null_val : layer_blk_img[1][lut_idx05].g;
                    L[22] = (lut_idx05==-1) ? null_val : layer_blk_img[2][lut_idx05].g;
                    L[23] = null_val;
                    L[24] = (lut_idx06==-1) ? null_val : layer_blk_img[0][lut_idx06].g;
                    L[25] = (lut_idx06==-1) ? null_val : layer_blk_img[1][lut_idx06].g;
                    L[26] = (lut_idx06==-1) ? null_val : layer_blk_img[2][lut_idx06].g;
                    L[27] = null_val;
                    L[28] = (lut_idx07==-1) ? null_val : layer_blk_img[0][lut_idx07].g;
                    L[29] = (lut_idx07==-1) ? null_val : layer_blk_img[1][lut_idx07].g;
                    L[30] = (lut_idx07==-1) ? null_val : layer_blk_img[2][lut_idx07].g;
                    L[31] = null_val;
                    break;
                case 4:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = (lut_idx00==-1) ? null_val : layer_blk_img[3][lut_idx00].g;
                    L[ 4] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 5] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[ 6] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[ 7] = (lut_idx01==-1) ? null_val : layer_blk_img[3][lut_idx01].g;
                    L[ 8] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[ 9] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[10] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[11] = (lut_idx02==-1) ? null_val : layer_blk_img[3][lut_idx02].g;
                    L[12] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[13] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[14] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[15] = (lut_idx03==-1) ? null_val : layer_blk_img[3][lut_idx03].g;
                    L[16] = (lut_idx04==-1) ? null_val : layer_blk_img[0][lut_idx04].g;
                    L[17] = (lut_idx04==-1) ? null_val : layer_blk_img[1][lut_idx04].g;
                    L[18] = (lut_idx04==-1) ? null_val : layer_blk_img[2][lut_idx04].g;
                    L[19] = (lut_idx04==-1) ? null_val : layer_blk_img[3][lut_idx04].g;
                    L[20] = (lut_idx05==-1) ? null_val : layer_blk_img[0][lut_idx05].g;
                    L[21] = (lut_idx05==-1) ? null_val : layer_blk_img[1][lut_idx05].g;
                    L[22] = (lut_idx05==-1) ? null_val : layer_blk_img[2][lut_idx05].g;
                    L[23] = (lut_idx05==-1) ? null_val : layer_blk_img[3][lut_idx05].g;
                    L[24] = (lut_idx06==-1) ? null_val : layer_blk_img[0][lut_idx06].g;
                    L[25] = (lut_idx06==-1) ? null_val : layer_blk_img[1][lut_idx06].g;
                    L[26] = (lut_idx06==-1) ? null_val : layer_blk_img[2][lut_idx06].g;
                    L[27] = (lut_idx06==-1) ? null_val : layer_blk_img[3][lut_idx06].g;
                    L[28] = (lut_idx07==-1) ? null_val : layer_blk_img[0][lut_idx07].g;
                    L[29] = (lut_idx07==-1) ? null_val : layer_blk_img[1][lut_idx07].g;
                    L[30] = (lut_idx07==-1) ? null_val : layer_blk_img[2][lut_idx07].g;
                    L[31] = (lut_idx07==-1) ? null_val : layer_blk_img[3][lut_idx07].g;
                    break;
                case 5:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = (lut_idx00==-1) ? null_val : layer_blk_img[3][lut_idx00].g;
                    L[ 4] = (lut_idx00==-1) ? null_val : layer_blk_img[4][lut_idx00].g;
                    L[ 5] = null_val;
                    L[ 6] = null_val;
                    L[ 7] = null_val;
                    L[ 8] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 9] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[10] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[11] = (lut_idx01==-1) ? null_val : layer_blk_img[3][lut_idx01].g;
                    L[12] = (lut_idx01==-1) ? null_val : layer_blk_img[4][lut_idx01].g;
                    L[13] = null_val;
                    L[14] = null_val;
                    L[15] = null_val;
                    L[16] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[17] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[18] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[19] = (lut_idx02==-1) ? null_val : layer_blk_img[3][lut_idx02].g;
                    L[20] = (lut_idx02==-1) ? null_val : layer_blk_img[4][lut_idx02].g;
                    L[21] = null_val;
                    L[22] = null_val;
                    L[23] = null_val;
                    L[24] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[25] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[26] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[27] = (lut_idx03==-1) ? null_val : layer_blk_img[3][lut_idx03].g;
                    L[28] = (lut_idx03==-1) ? null_val : layer_blk_img[4][lut_idx03].g;
                    L[29] = null_val;
                    L[30] = null_val;
                    L[31] = null_val;
                    break;
                case 6:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = (lut_idx00==-1) ? null_val : layer_blk_img[3][lut_idx00].g;
                    L[ 4] = (lut_idx00==-1) ? null_val : layer_blk_img[4][lut_idx00].g;
                    L[ 5] = (lut_idx00==-1) ? null_val : layer_blk_img[5][lut_idx00].g;
                    L[ 6] = null_val;
                    L[ 7] = null_val;
                    L[ 8] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 9] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[10] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[11] = (lut_idx01==-1) ? null_val : layer_blk_img[3][lut_idx01].g;
                    L[12] = (lut_idx01==-1) ? null_val : layer_blk_img[4][lut_idx01].g;
                    L[13] = (lut_idx01==-1) ? null_val : layer_blk_img[5][lut_idx01].g;
                    L[14] = null_val;
                    L[15] = null_val;
                    L[16] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[17] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[18] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[19] = (lut_idx02==-1) ? null_val : layer_blk_img[3][lut_idx02].g;
                    L[20] = (lut_idx02==-1) ? null_val : layer_blk_img[4][lut_idx02].g;
                    L[21] = (lut_idx02==-1) ? null_val : layer_blk_img[5][lut_idx02].g;
                    L[22] = null_val;
                    L[23] = null_val;
                    L[24] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[25] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[26] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[27] = (lut_idx03==-1) ? null_val : layer_blk_img[3][lut_idx03].g;
                    L[28] = (lut_idx03==-1) ? null_val : layer_blk_img[4][lut_idx03].g;
                    L[29] = (lut_idx03==-1) ? null_val : layer_blk_img[5][lut_idx03].g;
                    L[30] = null_val;
                    L[31] = null_val;
                    break;
                case 7:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = (lut_idx00==-1) ? null_val : layer_blk_img[3][lut_idx00].g;
                    L[ 4] = (lut_idx00==-1) ? null_val : layer_blk_img[4][lut_idx00].g;
                    L[ 5] = (lut_idx00==-1) ? null_val : layer_blk_img[5][lut_idx00].g;
                    L[ 6] = (lut_idx00==-1) ? null_val : layer_blk_img[6][lut_idx00].g;
                    L[ 7] = null_val;
                    L[ 8] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 9] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[10] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[11] = (lut_idx01==-1) ? null_val : layer_blk_img[3][lut_idx01].g;
                    L[12] = (lut_idx01==-1) ? null_val : layer_blk_img[4][lut_idx01].g;
                    L[13] = (lut_idx01==-1) ? null_val : layer_blk_img[5][lut_idx01].g;
                    L[14] = (lut_idx01==-1) ? null_val : layer_blk_img[6][lut_idx01].g;
                    L[15] = null_val;
                    L[16] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[17] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[18] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[19] = (lut_idx02==-1) ? null_val : layer_blk_img[3][lut_idx02].g;
                    L[20] = (lut_idx02==-1) ? null_val : layer_blk_img[4][lut_idx02].g;
                    L[21] = (lut_idx02==-1) ? null_val : layer_blk_img[5][lut_idx02].g;
                    L[22] = (lut_idx02==-1) ? null_val : layer_blk_img[6][lut_idx02].g;
                    L[23] = null_val;
                    L[24] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[25] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[26] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[27] = (lut_idx03==-1) ? null_val : layer_blk_img[3][lut_idx03].g;
                    L[28] = (lut_idx03==-1) ? null_val : layer_blk_img[4][lut_idx03].g;
                    L[29] = (lut_idx03==-1) ? null_val : layer_blk_img[5][lut_idx03].g;
                    L[30] = (lut_idx03==-1) ? null_val : layer_blk_img[6][lut_idx03].g;
                    L[31] = null_val;
                    break;
                case 8:
                    L[ 0] = (lut_idx00==-1) ? null_val : layer_blk_img[0][lut_idx00].g;
                    L[ 1] = (lut_idx00==-1) ? null_val : layer_blk_img[1][lut_idx00].g;
                    L[ 2] = (lut_idx00==-1) ? null_val : layer_blk_img[2][lut_idx00].g;
                    L[ 3] = (lut_idx00==-1) ? null_val : layer_blk_img[3][lut_idx00].g;
                    L[ 4] = (lut_idx00==-1) ? null_val : layer_blk_img[4][lut_idx00].g;
                    L[ 5] = (lut_idx00==-1) ? null_val : layer_blk_img[5][lut_idx00].g;
                    L[ 6] = (lut_idx00==-1) ? null_val : layer_blk_img[6][lut_idx00].g;
                    L[ 7] = (lut_idx00==-1) ? null_val : layer_blk_img[7][lut_idx00].g;
                    L[ 8] = (lut_idx01==-1) ? null_val : layer_blk_img[0][lut_idx01].g;
                    L[ 9] = (lut_idx01==-1) ? null_val : layer_blk_img[1][lut_idx01].g;
                    L[10] = (lut_idx01==-1) ? null_val : layer_blk_img[2][lut_idx01].g;
                    L[11] = (lut_idx01==-1) ? null_val : layer_blk_img[3][lut_idx01].g;
                    L[12] = (lut_idx01==-1) ? null_val : layer_blk_img[4][lut_idx01].g;
                    L[13] = (lut_idx01==-1) ? null_val : layer_blk_img[5][lut_idx01].g;
                    L[14] = (lut_idx01==-1) ? null_val : layer_blk_img[6][lut_idx01].g;
                    L[15] = (lut_idx01==-1) ? null_val : layer_blk_img[7][lut_idx01].g;
                    L[16] = (lut_idx02==-1) ? null_val : layer_blk_img[0][lut_idx02].g;
                    L[17] = (lut_idx02==-1) ? null_val : layer_blk_img[1][lut_idx02].g;
                    L[18] = (lut_idx02==-1) ? null_val : layer_blk_img[2][lut_idx02].g;
                    L[19] = (lut_idx02==-1) ? null_val : layer_blk_img[3][lut_idx02].g;
                    L[20] = (lut_idx02==-1) ? null_val : layer_blk_img[4][lut_idx02].g;
                    L[21] = (lut_idx02==-1) ? null_val : layer_blk_img[5][lut_idx02].g;
                    L[22] = (lut_idx02==-1) ? null_val : layer_blk_img[6][lut_idx02].g;
                    L[23] = (lut_idx02==-1) ? null_val : layer_blk_img[7][lut_idx02].g;
                    L[24] = (lut_idx03==-1) ? null_val : layer_blk_img[0][lut_idx03].g;
                    L[25] = (lut_idx03==-1) ? null_val : layer_blk_img[1][lut_idx03].g;
                    L[26] = (lut_idx03==-1) ? null_val : layer_blk_img[2][lut_idx03].g;
                    L[27] = (lut_idx03==-1) ? null_val : layer_blk_img[3][lut_idx03].g;
                    L[28] = (lut_idx03==-1) ? null_val : layer_blk_img[4][lut_idx03].g;
                    L[29] = (lut_idx03==-1) ? null_val : layer_blk_img[5][lut_idx03].g;
                    L[30] = (lut_idx03==-1) ? null_val : layer_blk_img[6][lut_idx03].g;
                    L[31] = (lut_idx03==-1) ? null_val : layer_blk_img[7][lut_idx03].g;
                    break;
                }

                int idx;
                for (idx = 0; idx < 32; idx++)
                {
                    char rr = (char)((L[idx] < 0) ? (256 + L[idx]) : (L[idx]));

                    Lut_out[out_idx++]= rr;

                }
            }
        }
    }

    pLut_out_vector->dat_num += out_idx;

    if (pLut_out_vector->dat_num > pLut_out_vector->max_num)
    {
        printf("Error !\n");
        printf("Lut_out_vector data num  : 0x%x\n", (unsigned int)pLut_out_vector->dat_num);
        printf("Lut_out_vector max  num  : 0x%x\n", (unsigned int)pLut_out_vector->max_num);
    }
#if 0
    int i = 0;
    char *Lut_out = (char *)(pLut_out_vector->pbuf) + pLut_out_vector->dat_num;


    int layer_idx = 0, layer = 0, node_idx = 0, Hnode_idx = 0, Vnode_idx = 0, node_idx_start = 0, buf_idx = 0;

    // Nodes per plane = H_node*V_node
    int tbl_plane_node_num = layer_info->real_dram_h_size*layer_info->real_dram_v_size;

    // Buffer per line = round_up(H node/ Nodes_per_buffer)
    int buf_per_line = 0;

    // Total buffer per plane = buffer per line * V_node
    int tbl_plane_buf_num = buf_per_line * layer_info->real_dram_v_size;

    // Last buffer node number
    int last_buf_node = 0;

    //
    int demura_tbl_buf = 0;

    if(layer_info->dmc_rgb_mode==1)
        demura_tbl_buf = DEMURA_TBL_BUF_RGB;
    else
        demura_tbl_buf = DEMURA_TBL_BUF_MONO;


    buf_per_line = (layer_info->real_dram_h_size/demura_tbl_buf) + (((layer_info->real_dram_h_size % demura_tbl_buf) == 0)?0:1);
    last_buf_node = (layer_info->real_dram_h_size % demura_tbl_buf);

    for(layer_idx = 0; layer_idx < layer_info->dmc_plane_num; layer_idx++)
    {
        layer = layer_idx+1; // for output layer index
        for(Vnode_idx = 0; Vnode_idx < layer_info->real_dram_v_size; Vnode_idx++)
        {
            // node idx start at this V line
            node_idx_start = Vnode_idx*layer_info->real_dram_h_size;
            // buffer index
            buf_idx = 0;

            for(buf_idx = 0; buf_idx < buf_per_line; buf_idx++)
            {
                int L[21] = {0};
                char data[32] = {0};

                node_idx = node_idx_start + buf_idx*demura_tbl_buf;

                if((buf_idx == (buf_per_line - 1)) && (last_buf_node != 0))
                {
                    //the last buffer of this line
                    for(i = 0; i < last_buf_node; i++)
                    {
                        if(layer_info->dmc_rgb_mode==1) // RGB mode
                        {
                            L[i*3] = layer_blk_img[layer_idx][node_idx + i].b;
                            L[i*3 + 1] = layer_blk_img[layer_idx][node_idx + i].g;
                            L[i*3 + 2] = layer_blk_img[layer_idx][node_idx + i].r;
                        }
                        else // MONO mode
                        {
                            L[i] = layer_blk_img[layer_idx][node_idx + i].r;
                        }
                    }
                }
                else
                {
                    for(i = 0; i < demura_tbl_buf; i++)
                    {
                        if(layer_info->dmc_rgb_mode==1) // RGB mode
                        {
                            L[i*3] = layer_blk_img[layer_idx][node_idx + i].b;
                            L[i*3 + 1] = layer_blk_img[layer_idx][node_idx + i].g;
                            L[i*3 + 2] = layer_blk_img[layer_idx][node_idx + i].r;
                        }
                        else // MONO mode
                        {
                            L[i] = layer_blk_img[layer_idx][node_idx + i].r;
                        }
                    }
                }

                // s11 format
                for(i = 0; i < 21; i++)
                {
                    L[i] = (L[i] < 0) ? (4096 + L[i]) : (L[i]);
                    L[i] &= 0xFFF;
                }

                // push back 256bits(32 bytes)
                for(i = 0; i < 10; i++)
                {
                    data[3*i] = L[2*i]&0x0FF;
                    data[3*i+1] = ((L[2*i+1]&0x00F) << 4) | ((L[2*i]&0xF00) >> 8);
                    data[3*i+2] = ((L[2*i+1]&0xFF0) >> 4);
                }
                data[30] = L[20]&0x0FF;
                data[31] = ((L[20]&0xF00) >> 8) | ((layer&0x00F) << 4);

                /*for(i = 0; i < 32; i++)
                {
                    //Lut_out.push_back((char)data[i]);

                }*/
                memcpy(Lut_out, data, 32);

            }

        }
    }
#endif
}

typedef void (*FUNCP_WRITE_LUT_TO_HW_DRAM_FORMAT)(stlayer_info_structInfo *layer_info, strgb_structInfo *layer_blk_img[8], m_vector *pLut_out_vector);
FUNCP_WRITE_LUT_TO_HW_DRAM_FORMAT g_handle_luthwformat[E_DEMURA_VERSION_MAX] ={
    write_demuraLUT_to_HW_DRAM_FORMAT_S11,  //E_DEMURA_VERSION_S11
    write_demuraLUT_to_HW_DRAM_FORMAT_S7,   //E_DEMURA_VERSION_S7
    write_demuraLUT_to_HW_DRAM_FORMAT_S7,   //E_DEMURA_VERSION_S7_MONO
};

void write_demuraLUT_to_HW_DRAM_FORMAT(stlayer_info_structInfo *layer_info, strgb_structInfo *layer_blk_img[8], m_vector *pLut_out_vector)
{
    if (g_handle_luthwformat[demura_get_version()] != NULL)
    {
        g_handle_luthwformat[demura_get_version()](layer_info, layer_blk_img, pLut_out_vector);
    }
}

int GETBITS(const int Value , const int MSB_bit, const int LSB_bit)
{
    int Mask = (1<<(MSB_bit - LSB_bit + 1)) - 1;
    int Shift = LSB_bit;

    int step1 = (Value>>Shift);
    int step2 = (step1&Mask);

    return step2;
}


int register_08bits_inv( _08bits reg_tmp, int total_bits)
{
    int shift_bit = 0;
    int shift_bit_tmp = 0;
    int mask_tmp = reg_tmp.Bit07_00.mask;

    int count_i;
    for (count_i = 1; count_i < 8; count_i++)
    {
        shift_bit_tmp = (mask_tmp&0x1);
        mask_tmp = (mask_tmp>>1);

        if(shift_bit_tmp==0)
            shift_bit = count_i;
        else
            break;

    }

    int value = reg_tmp.Bit07_00.value;

    int LSB_bit = shift_bit;
    int MSB_bit = (LSB_bit + total_bits - 1);

    int final_value = GETBITS( value , MSB_bit, LSB_bit);

    return final_value;
}

int register_16bits_inv( _16bits reg_tmp, int MSB_total_bits, int LSB_total_bits)
{
    //------------- LSB process start -------------//
    int shift_bit = 0;
    int shift_bit_tmp = 0;
    int mask_tmp = reg_tmp.Bit07_00.mask;

    int count_i;
    for (count_i = 1; count_i < 8; count_i++)
    {
        shift_bit_tmp = (mask_tmp&0x1);
        mask_tmp = (mask_tmp>>1);

        if(shift_bit_tmp==0)
            shift_bit = count_i;
        else
            break;

    }

    int value = reg_tmp.Bit07_00.value;

    int LSB_bit = shift_bit;
    int MSB_bit = (LSB_bit + LSB_total_bits - 1);

    int LSB_value = GETBITS( value , MSB_bit, LSB_bit);
    //------------- LSB process start -------------//


    //------------- MSB process start -------------//
    shift_bit = 0;
    shift_bit_tmp = 0;
    mask_tmp = reg_tmp.Bit15_08.mask;
    for (count_i = 1; count_i < 8; count_i++)
    {
        shift_bit_tmp = (mask_tmp&0x1);
        mask_tmp = (mask_tmp>>1);

        if(shift_bit_tmp==0)
            shift_bit = count_i;
        else
            break;
    }

    value = reg_tmp.Bit15_08.value;

    LSB_bit = shift_bit;
    MSB_bit = (LSB_bit + MSB_total_bits - 1);

    int MSB_value = GETBITS( value , MSB_bit, LSB_bit);
    //------------- LSB process start -------------//

    int final_value = (MSB_value << LSB_total_bits) + LSB_value;

    return final_value;
}

int twos_complement_to_integer(const int input_value, const int total_bits)
{
    int sign_bit = 1<<(total_bits-1);
    int inverse_value = 1<<total_bits;

    int final_value = (input_value>=sign_bit) ? (input_value - inverse_value) : input_value;

    return final_value;
}

double DMC_LUT_Fix_To_Double(const int Layer_offset_2s, const int Layer_gain, const int Lut_value)
{
    int Layer_offset_2s_to_interger = twos_complement_to_integer( Layer_offset_2s, 14);
    double Layer_offset_double = (double)Layer_offset_2s_to_interger/(double)16;  // format s9.4 base on 10bit data path

    int Lut_value_2s_to_interger = twos_complement_to_integer( Lut_value, 8);

    double Lut_value_double = 0.0;
    switch(Layer_gain)
    {
    case 6:  // s3.4 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger/(double)16;
        break;
    case 7:  // s4.3 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger/(double)8;
        break;
    case 0:  // s5.2 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger/(double)4;
        break;
    case 1:  // s6.1 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger/(double)2;
        break;
    default:
    case 2:  // s7.0 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger/(double)1;
        break;
    case 3:  // s8.0 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger*(double)2;
        break;
    case 4:  // s9.0 base on 10bit data path
        Lut_value_double = (double)Lut_value_2s_to_interger*(double)4;
        break;
    }

    double final_value = Layer_offset_double + Lut_value_double;

    return final_value;

}

#ifdef CONFIG_DEMURA_VENDOR_LGD

#define minmax(v,a,b) (((v)<(a))? (a) : ((v)>(b)) ? (b) : (v))

MS_BOOL bit_mode_LPF(int plane_idx, int block_width, int block_height, strgb_structInfo *layer_blk_img_out)
{
    int LPF[5][5] = {{ 0, 1, 1, 1, 0 },
                     { 1, 3, 7, 3, 1 },
                     { 1, 7, 12, 7, 1 },
                     { 1, 3, 7, 3, 1 },
                     { 0, 1, 1, 1, 0 } }; //sum=64
    struct array_dim2 _tmp;
    if(DEFINE_ARRAY_DIM2(_tmp, sizeof(int), block_height, block_width)!=TRUE)
    {
        return FALSE;
    }
    int ** tmp = (int **)PTR_ARRAY_DIM2(_tmp);

    //! We only use RGB"W" channel (i.e., channel 3)
    //! Shift according to bit_mode
    int CurH, CurV;
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {
            tmp[CurV][CurH] = (int)(layer_blk_img_out[posi_blk(CurV,CurH)].r);
            int loui = tmp[CurV][CurH];
            if ((CurH >= 0 && CurH <=4) && (CurV >=476 && CurV <=480))
                UBOOT_DEBUG("%d, ", loui);
        }
    }

    //! LPF
    for (CurV = 0; CurV < block_height; CurV++)
    {
        for (CurH = 0; CurH < block_width; CurH++)
        {
            if(CurH == 478)
            {
                //int loui = 0;
            }
            int sum = 0;
            int x_sh, y_sh;
            for (x_sh = -2; x_sh < 3; x_sh++)
            {
                for (y_sh = -2; y_sh < 3; y_sh++)
                {
                    sum += tmp[minmax(CurV + y_sh, 0, (block_height-1))][minmax(CurH + x_sh, 0, (block_width-1))] * LPF[y_sh + 2][x_sh + 2];
                }
            }

            //double uuu = (double)minmax((double)sum / (double)64, -128, 127);

            // R channel
            //layer_blk_img_out[posi_blk(CurV,CurH)].dbr = (double)minmax((double)sum / (double)64, -128, 127); // Clamp to s7
            layer_blk_img_out[posi_blk(CurV,CurH)].r   = (int)minmax(sum / 64, -128, 127); // Clamp to s7
            // G channel
            //layer_blk_img_out[posi_blk(CurV,CurH)].dbg = (double)minmax((double)sum / (double)64, -128, 127); // Clamp to s7
            layer_blk_img_out[posi_blk(CurV,CurH)].g   = (int)minmax(sum / 64, -128, 127); // Clamp to s7
            // B channel
            //layer_blk_img_out[posi_blk(CurV,CurH)].dbb = (double)minmax((double)sum / (double)64, -128, 127); // Clamp to s7
            layer_blk_img_out[posi_blk(CurV,CurH)].b   = (int)minmax(sum / 64, -128, 127); // Clamp to s7
        }
    }

    FREE_ARRAY_DIM2(_tmp);
    return TRUE;
}


#endif

