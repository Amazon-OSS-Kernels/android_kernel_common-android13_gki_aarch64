// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

typedef struct
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned high     : 4;
    unsigned reserve  : 4;
#else /* __BIG_ENDIAN */
    unsigned reserve  : 4;
    unsigned high     : 4;
#endif
    unsigned char low;

} __attribute__((packed)) AUO_BIN_Panel_Level;


typedef struct
{
    unsigned char dmc_par_checksum[2];

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned dmc_panel_num     :  4;
    unsigned dmc_block_size    :  2;
    unsigned byte_1_reserve    :  1;
    unsigned rgb_mode          :  1;
#else /* __BIG_ENDIAN */
    unsigned rgb_mode          :  1;
    unsigned byte_1_reserve    :  1;
    unsigned dmc_block_size    :  2;
    unsigned dmc_panel_num     :  4;
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned dmc_v_block       :  3;
    unsigned byte_2_reserve1   :  1;
    unsigned dmc_h_block       :  3;
    unsigned byte_2_reserve    :  1;
#else /* __BIG_ENDIAN */
    unsigned byte_2_reserve    :  1;
    unsigned dmc_h_block       :  3;
    unsigned byte_2_reserve1   :  1;
    unsigned dmc_v_block       :  3;
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char dmc_h_lut_num_h;
    unsigned dmc_v_lut_num_h   :  2;
    unsigned byte_3_reserve    :  2;
    unsigned dmc_h_lut_num_l   :  4;
    unsigned char dmc_v_lut_num_l;
#else /* __BIG_ENDIAN */
    unsigned char dmc_h_lut_num_h;
    unsigned dmc_h_lut_num_l   :  4;
    unsigned byte_3_reserve    :  2;
    unsigned dmc_v_lut_num_h   :  2;
    unsigned char dmc_v_lut_num_l;
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned dmc_panel_78_ks22 :  1;
    unsigned dmc_panel_67_ks22 :  1;
    unsigned dmc_panel_56_ks22 :  1;
    unsigned dmc_panel_45_ks22 :  1;
    unsigned dmc_panel_34_ks22 :  1;
    unsigned dmc_panel_23_ks22 :  1;
    unsigned dmc_panel_12_ks22 :  1;
    unsigned dmc_panel_b1_ks22 :  1;
#else /* __BIG_ENDIAN */
    unsigned dmc_panel_b1_ks22 :  1;
    unsigned dmc_panel_12_ks22 :  1;
    unsigned dmc_panel_23_ks22 :  1;
    unsigned dmc_panel_34_ks22 :  1;
    unsigned dmc_panel_45_ks22 :  1;
    unsigned dmc_panel_56_ks22 :  1;
    unsigned dmc_panel_67_ks22 :  1;
    unsigned dmc_panel_78_ks22 :  1;
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned byte_4_reserve    :  7;
    unsigned dmc_panel_8w_ks22 :  1;
#else /* __BIG_ENDIAN */
    unsigned dmc_panel_8w_ks22 :  1;
    unsigned byte_4_reserve    :  7;
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned dmc_black_limit_h :  4;
    unsigned byte_5_reserve    :  4;
    unsigned char dmc_black_limit_l;
#else /* __BIG_ENDIAN */
    unsigned byte_5_reserve    :  4;
    unsigned dmc_black_limit_h :  4;
    unsigned char dmc_black_limit_l;
#endif

    AUO_BIN_Panel_Level panel_levels[8];

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned dmc_white_limit_h :  4;
    unsigned byte_7_reserve    :  4;
    unsigned char dmc_white_limit_l;
#else /* __BIG_ENDIAN */
    unsigned byte_7_reserve    :  4;
    unsigned dmc_white_limit_h :  4;
    unsigned char dmc_white_limit_l;
#endif
    
    unsigned int reverse[8];

} __attribute__((packed)) AUO_BIN_Header;

#endif  // _PARSE_HEADER_H_
