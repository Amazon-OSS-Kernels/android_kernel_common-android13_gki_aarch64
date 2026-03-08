// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#define HEADER_START   (DMC_HEADER_CHECKSUM)
#define HEADER_LEN     (DMC_HEADER_END - DMC_HEADER_CHECKSUM + 1)

#define DMC_HEADER_CHECKSUM     0x4
#define DMC_HEADER_START        0x6
#define DMC_HEADER_END          0x3F

#define DMC_LUT_CHECKSUM        0x40
#define DMC_LUT_START           0x42

#define MAX_PLANE_NUM           8

typedef struct
{
    _08bits reg_dmc_rgb_mode;        //  1 bit
    _08bits reg_dmc_block_size;      //  2 bit
    _08bits reg_dmc_plane_num;       //  4 bit
    _08bits reg_dmc_h_block;         //  3 bit
    _08bits reg_dmc_v_block;         //  3 bit
    _16bits reg_dmc_h_lut_num;       // 11 bit
    _16bits reg_dmc_v_lut_num;       // 10 bit
    _08bits reg_dmc_plane_b1_ks22;   //  1 bit
    _08bits reg_dmc_plane_12_ks22;   //  1 bit
    _08bits reg_dmc_plane_23_ks22;   //  1 bit
    _08bits reg_dmc_plane_34_ks22;   //  1 bit
    _08bits reg_dmc_plane_45_ks22;   //  1 bit
    _08bits reg_dmc_plane_56_ks22;   //  1 bit
    _08bits reg_dmc_plane_67_ks22;   //  1 bit
    _08bits reg_dmc_plane_78_ks22;   //  1 bit
    _08bits reg_dmc_plane_8w_ks22;   //  1 bit
    _16bits reg_dmc_black_limit;     // 12 bit
    _16bits reg_dmc_plane_level1;    // 12 bit
    _16bits reg_dmc_plane_level2;    // 12 bit
    _16bits reg_dmc_plane_level3;    // 12 bit
    _16bits reg_dmc_plane_level4;    // 12 bit
    _16bits reg_dmc_plane_level5;    // 12 bit
    _16bits reg_dmc_plane_level6;    // 12 bit
    _16bits reg_dmc_plane_level7;    // 12 bit
    _16bits reg_dmc_plane_level8;    // 12 bit
    _16bits reg_dmc_white_limit;     // 12 bit
    _16bits reg_dmc_plane_b1_coef;   // 14 bit
    _16bits reg_dmc_plane_12_coef;   // 14 bit
    _16bits reg_dmc_plane_23_coef;   // 14 bit
    _16bits reg_dmc_plane_34_coef;   // 14 bit
    _16bits reg_dmc_plane_45_coef;   // 14 bit
    _16bits reg_dmc_plane_56_coef;   // 14 bit
    _16bits reg_dmc_plane_67_coef;   // 14 bit
    _16bits reg_dmc_plane_78_coef;   // 14 bit
    _16bits reg_dmc_plane_8w_coef;   // 14 bit
    _08bits reg_dmc_data_r_mag1;     //  3 bit
    _08bits reg_dmc_data_r_mag2;     //  3 bit
    _08bits reg_dmc_data_r_mag3;     //  3 bit
    _08bits reg_dmc_data_r_mag4;     //  3 bit
    _08bits reg_dmc_data_r_mag5;     //  3 bit
    _08bits reg_dmc_data_r_mag6;     //  3 bit
    _08bits reg_dmc_data_r_mag7;     //  3 bit
    _08bits reg_dmc_data_r_mag8;     //  3 bit
    _08bits reg_dmc_data_g_mag1;     //  3 bit
    _08bits reg_dmc_data_g_mag2;     //  3 bit
    _08bits reg_dmc_data_g_mag3;     //  3 bit
    _08bits reg_dmc_data_g_mag4;     //  3 bit
    _08bits reg_dmc_data_g_mag5;     //  3 bit
    _08bits reg_dmc_data_g_mag6;     //  3 bit
    _08bits reg_dmc_data_g_mag7;     //  3 bit
    _08bits reg_dmc_data_g_mag8;     //  3 bit
    _08bits reg_dmc_data_b_mag1;     //  3 bit
    _08bits reg_dmc_data_b_mag2;     //  3 bit
    _08bits reg_dmc_data_b_mag3;     //  3 bit
    _08bits reg_dmc_data_b_mag4;     //  3 bit
    _08bits reg_dmc_data_b_mag5;     //  3 bit
    _08bits reg_dmc_data_b_mag6;     //  3 bit
    _08bits reg_dmc_data_b_mag7;     //  3 bit
    _08bits reg_dmc_data_b_mag8;     //  3 bit
    _16bits reg_dmc_data_r_offset1;   // 14 bit
    _16bits reg_dmc_data_r_offset2;   // 14 bit
    _16bits reg_dmc_data_r_offset3;   // 14 bit
    _16bits reg_dmc_data_r_offset4;   // 14 bit
    _16bits reg_dmc_data_r_offset5;   // 14 bit
    _16bits reg_dmc_data_r_offset6;   // 14 bit
    _16bits reg_dmc_data_r_offset7;   // 14 bit
    _16bits reg_dmc_data_r_offset8;   // 14 bit
    _16bits reg_dmc_data_g_offset1;   // 14 bit
    _16bits reg_dmc_data_g_offset2;   // 14 bit
    _16bits reg_dmc_data_g_offset3;   // 14 bit
    _16bits reg_dmc_data_g_offset4;   // 14 bit
    _16bits reg_dmc_data_g_offset5;   // 14 bit
    _16bits reg_dmc_data_g_offset6;   // 14 bit
    _16bits reg_dmc_data_g_offset7;   // 14 bit
    _16bits reg_dmc_data_g_offset8;   // 14 bit
    _16bits reg_dmc_data_b_offset1;   // 14 bit
    _16bits reg_dmc_data_b_offset2;   // 14 bit
    _16bits reg_dmc_data_b_offset3;   // 14 bit
    _16bits reg_dmc_data_b_offset4;   // 14 bit
    _16bits reg_dmc_data_b_offset5;   // 14 bit
    _16bits reg_dmc_data_b_offset6;   // 14 bit
    _16bits reg_dmc_data_b_offset7;   // 14 bit
    _16bits reg_dmc_data_b_offset8;   // 14 bit

} dmc_registers;


typedef enum
{
    E_MONO_MODE  = 0,
    E_RGB_MODE   = 1,
} E_COLOR_MODE;

typedef struct
{
    MS_U16          PARAMETER_CRC;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U16          LOWER_BOUND;
    MS_U16          UPPER_BOUND;
    MS_U16          PLANE[MAX_PLANE_NUM];
    MS_U32          TBL_END_ADDR;
    MS_U16          TABLE_CRC;
    MS_U32          DEMURA_TABLE_LENGTH;

    // Header Buffer
    MS_U8         *hdr_buffer;
    MS_U32        hdr_offset;
    MS_U32        hdr_length;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}CHOT_Demura_Header;

MS_BOOL load_vendor_header(CHOT_Demura_Header *phdr);
MS_BOOL parse_vendor_header(CHOT_Demura_Header *phdr);

void dump_vendor_header(CHOT_Demura_Header *phdr);
MS_BOOL load_vendor_lut(CHOT_Demura_Header *phdr);
MS_U16 get_lut_checksum(void);

#endif
