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


#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

#define _BIT0  0x0001
#define _BIT1  0x0002
#define _BIT2  0x0004
#define _BIT3  0x0008
#define _BIT4  0x0010
#define _BIT5  0x0020
#define _BIT6  0x0040
#define _BIT7  0x0080
#define _BIT8  0x0100
#define _BIT9  0x0200
#define _BIT10 0x0400
#define _BIT11 0x0800
#define _BIT12 0x1000
#define _BIT13 0x2000
#define _BIT14 0x4000
#define _BIT15 0x8000

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

typedef struct
{
    MS_U8           PARAMETER_CRC_H;
    MS_U8           PARAMETER_CRC_L;
    MS_U8           dummy0;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U8           DEMURA_TBL_H_H;
    MS_U8           DEMURA_TBL_H_L;
    MS_U8           DEMURA_TBL_V_H;
    MS_U8           DEMURA_TBL_V_L;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U8           dummy1[10];
    MS_U8           LOWER_BOUND_H;
    MS_U8           LOWER_BOUND_L;
    MS_U8           UPPER_BOUND_H;
    MS_U8           UPPER_BOUND_L;
    MS_U8           dummy2[12];
    MS_U8           PLANE00_LV_H;
    MS_U8           PLANE00_LV_L;
    MS_U8           PLANE01_LV_H;
    MS_U8           PLANE01_LV_L;
    MS_U8           PLANE02_LV_H;
    MS_U8           PLANE02_LV_L;
    MS_U8           PLANE03_LV_H;
    MS_U8           PLANE03_LV_L;
    MS_U8           PLANE04_LV_H;
    MS_U8           PLANE04_LV_L;
    MS_U8           PLANE05_LV_H;
    MS_U8           PLANE05_LV_L;
    MS_U8           PLANE06_LV_H;
    MS_U8           PLANE06_LV_L;
    MS_U8           PLANE07_LV_H;
    MS_U8           PLANE07_LV_L;
    MS_U8           dummy3[14];
} __attribute__((packed)) CHOT_BIN_Header;

typedef struct
{
    //File header
    MS_U8           u32MagicNum[4];
    MS_U8           name[14];
    MS_U8           version[2];
    MS_U8           sectionNum[2];
    MS_U8           u16Reseved_0[2];
    MS_U8           all_file_size[4];
    MS_U8           file_totoal_CRC[2];
    MS_U8           file_header_CRC[2]; //offset ox1e
    //section header1
    MS_U8           type_1[4];
    MS_U8           header_offser[4];
    MS_U8           header_size[4];
    MS_U8           header_section_CRC[2];
    MS_U8           u16Reseved_1[2]; //offset 0x2e
    //section header2
    MS_U8           type_2[4];
    MS_U8           lut_offse[4];
    MS_U8           lut_size[4];
    MS_U8           lut_section_CRC[2];
    MS_U8           u16Reseved_2[2]; //offset 0x3e
    //demura parameter
    MS_U8           enDemuraMode; //offset 0x40
    MS_U8           u8PlaneNum;
    MS_U8           u8HblockSize;
    MS_U8           u8VblockSize;
    MS_U8           u16HLutNum[2];
    MS_U8           u16VLutNum[2];
    MS_U8           u8IntBitWidth;
    MS_U8           u8DecimalsBitWidth;
    MS_U8           u8TwoChipEn;
    MS_U8           u8Reseved_3;
    MS_U8           u16CompValGainR[2];
    MS_U8           u16CompValGainG[2];
    MS_U8           u16CompValGainB[2];
    MS_U8           u8Reseved_4[2];
    MS_U8           u16CompValOffsetR[2];
    MS_U8           u16CompValOffsetG[2];
    MS_U8           u16CompValOffsetB[2];
    MS_U8           u16Reseved_5[2];
    MS_U8           u16BlackLimitR[2];
    MS_U8           u16BlackLimitG[2];
    MS_U8           u16BlackLimitB[2];
    MS_U8           u16Reseved_6[2];
    MS_U8           u16WhiteLimitR[2];
    MS_U8           u16WhiteLimitG[2];
    MS_U8           u16WhiteLimitB[2];
    MS_U8           u16Reseved_7[2]; //offset 0x6a
    MS_U8           u16Plane1LevelR[2]; //offset 0x6b
    MS_U8           u16Plane2LevelR[2];
    MS_U8           u16Plane3LevelR[2];
    MS_U8           u16Plane4LevelR[2];
    MS_U8           u16Plane5LevelR[2];
    MS_U8           u16Plane6LevelR[2];
    MS_U8           u16Plane7LevelR[2];
    MS_U8           u16Plane8LevelR[2];
    MS_U8           u16Plane9LevelR[2];
    MS_U8           u16Plane1LevelG[2];
    MS_U8           u16Plane2LevelG[2];
    MS_U8           u16Plane3LevelG[2];
    MS_U8           u16Plane4LevelG[2];
    MS_U8           u16Plane5LevelG[2];
    MS_U8           u16Plane6LevelG[2];
    MS_U8           u16Plane7LevelG[2];
    MS_U8           u16Plane8LevelG[2];
    MS_U8           u16Plane9LevelG[2];
    MS_U8           u16Plane1LevelB[2];
    MS_U8           u16Plane2LevelB[2];
    MS_U8           u16Plane3LevelB[2];
    MS_U8           u16Plane4LevelB[2];
    MS_U8           u16Plane5LevelB[2];
    MS_U8           u16Plane6LevelB[2];
    MS_U8           u16Plane7LevelB[2];
    MS_U8           u16Plane8LevelB[2];
    MS_U8           u16Plane9LevelB[2];
    MS_U8           u16Reseved_8[18];
    MS_U8           u16PlaneB1SlopeR[2]; //offset 0xb4 //65535/(u16Plane1LevelR - u16BlackLimitR)
    MS_U8           u16Plane12SlopeR[2];
    MS_U8           u16Plane23SlopeR[2];
    MS_U8           u16Plane34SlopeR[2];
    MS_U8           u16Plane45SlopeR[2];
    MS_U8           u16Plane56SlopeR[2];
    MS_U8           u16Plane67SlopeR[2];
    MS_U8           u16Plane78SlopeR[2];
    MS_U8           u16Plane89SlopeR[2];
    MS_U8           u16Plane9WSlopeR[2];
    MS_U8           u16PlaneB1SlopeG[2];
    MS_U8           u16Plane12SlopeG[2];
    MS_U8           u16Plane23SlopeG[2];
    MS_U8           u16Plane34SlopeG[2];
    MS_U8           u16Plane45SlopeG[2];
    MS_U8           u16Plane56SlopeG[2];
    MS_U8           u16Plane67SlopeG[2];
    MS_U8           u16Plane78SlopeG[2];
    MS_U8           u16Plane89SlopeG[2];
    MS_U8           u16Plane9WSlopeG[2];
    MS_U8           u16PlaneB1SlopeB[2];
    MS_U8           u16Plane12SlopeB[2];
    MS_U8           u16Plane23SlopeB[2];
    MS_U8           u16Plane34SlopeB[2];
    MS_U8           u16Plane45SlopeB[2];
    MS_U8           u16Plane56SlopeB[2];
    MS_U8           u16Plane67SlopeB[2];
    MS_U8           u16Plane78SlopeB[2];
    MS_U8           u16Plane89SlopeB[2];
    MS_U8           u16Plane9WSlopeB[2]; //offset 0xee
    MS_U8           u16Reseved_9[32];
    //demura lut offset 0x110 to end
}__attribute__((packed)) CSOT_CSOT_BIN_Header;

typedef struct
{
    //File header
    MS_U8          u32MagicNum[4];
    MS_U8           name[14];
    MS_U16          version;
    MS_U16          sectionNum;
    MS_U8           u16Reseved_0[2];
    MS_U32          all_file_size;
    MS_U16          file_totoal_CRC;
    MS_U16          file_header_CRC; //offset ox1e
}__attribute__((packed)) CSOT_File_Header;

typedef enum
{
    CSOT_SECTION_TYPE_NONE = 0,
    CSOT_SECTION_TYPE_PARAM = 1,
    CSOT_SECTION_TYPE_LUT_SINGLE = 2,
    CSOT_SECTION_TYPE_LUT_LEFT = 3,
    CSOT_SECTION_TYPE_LUT_RIGHT = 4,
    CSOT_SECTION_TYPE_PARAM_COMPRESS = 5,
} CSOT_SECTION_TYPE;

typedef struct
{
    //section header
    MS_U32          type_1;
    MS_U32          header_offser;
    MS_U32          header_size;
    MS_U16          header_section_CRC;
    MS_U8           u16Reseved_1[2]; //offset 0x2e
}__attribute__((packed)) CSOT_Section_Header;


typedef struct
{
    //demura parameter
    MS_U8           enDemuraMode; //offset 0x40
    MS_U8           u8PlaneNum;
    MS_U8           u8HblockSize;
    MS_U8           u8VblockSize;
    MS_U16          u16HLutNum;
    MS_U16          u16VLutNum;
    MS_U8           u8IntBitWidth;
    MS_U8           u8DecimalsBitWidth;
    MS_U8           u8TwoChipEn;
    MS_U8           u8Reseved_3;
    MS_U16          u16CompValGainR;
    MS_U16          u16CompValGainG;
    MS_U16          u16CompValGainB;
    MS_U16          u8Reseved_4;
    MS_U16          u16CompValOffsetR;
    MS_U16          u16CompValOffsetG;
    MS_U16          u16CompValOffsetB;
    MS_U16          u16Reseved_5;
    MS_U16          u16BlackLimitR;
    MS_U16          u16BlackLimitG;
    MS_U16          u16BlackLimitB;
    MS_U16          u16Reseved_6;
    MS_U16          u16WhiteLimitR;
    MS_U16          u16WhiteLimitG;
    MS_U16          u16WhiteLimitB;
    MS_U16          u16Reseved_7; //offset 0x6a
    MS_U16          u16Plane1LevelR; //offset 0x6b
    MS_U16          u16Plane2LevelR;
    MS_U16          u16Plane3LevelR;
    MS_U16          u16Plane4LevelR;
    MS_U16          u16Plane5LevelR;
    MS_U16          u16Plane6LevelR;
    MS_U16          u16Plane7LevelR;
    MS_U16          u16Plane8LevelR;
    MS_U16          u16Plane9LevelR;
    MS_U16          u16Plane1LevelG;
    MS_U16          u16Plane2LevelG;
    MS_U16          u16Plane3LevelG;
    MS_U16          u16Plane4LevelG;
    MS_U16          u16Plane5LevelG;
    MS_U16          u16Plane6LevelG;
    MS_U16          u16Plane7LevelG;
    MS_U16          u16Plane8LevelG;
    MS_U16          u16Plane9LevelG;
    MS_U16          u16Plane1LevelB;
    MS_U16          u16Plane2LevelB;
    MS_U16          u16Plane3LevelB;
    MS_U16          u16Plane4LevelB;
    MS_U16          u16Plane5LevelB;
    MS_U16          u16Plane6LevelB;
    MS_U16          u16Plane7LevelB;
    MS_U16          u16Plane8LevelB;
    MS_U16          u16Plane9LevelB;
    MS_U8           u16Reseved_8[18];
    MS_U16          u16PlaneB1SlopeR; //offset 0xb4 //65535/(u16Plane1LevelR - u16BlackLimitR)
    MS_U16          u16Plane12SlopeR;
    MS_U16          u16Plane23SlopeR;
    MS_U16          u16Plane34SlopeR;
    MS_U16          u16Plane45SlopeR;
    MS_U16          u16Plane56SlopeR;
    MS_U16          u16Plane67SlopeR;
    MS_U16          u16Plane78SlopeR;
    MS_U16          u16Plane89SlopeR;
    MS_U16          u16Plane9WSlopeR;
    MS_U16          u16PlaneB1SlopeG;
    MS_U16          u16Plane12SlopeG;
    MS_U16          u16Plane23SlopeG;
    MS_U16          u16Plane34SlopeG;
    MS_U16          u16Plane45SlopeG;
    MS_U16          u16Plane56SlopeG;
    MS_U16          u16Plane67SlopeG;
    MS_U16          u16Plane78SlopeG;
    MS_U16          u16Plane89SlopeG;
    MS_U16          u16Plane9WSlopeG;
    MS_U16          u16PlaneB1SlopeB;
    MS_U16          u16Plane12SlopeB;
    MS_U16          u16Plane23SlopeB;
    MS_U16          u16Plane34SlopeB;
    MS_U16          u16Plane45SlopeB;
    MS_U16          u16Plane56SlopeB;
    MS_U16          u16Plane67SlopeB;
    MS_U16          u16Plane78SlopeB;
    MS_U16          u16Plane89SlopeB;
    MS_U16          u16Plane9WSlopeB; //offset 0xee
    MS_U8           u16Reseved_9[32];
}__attribute__((packed)) CSOT_Demura_Header;
#endif  // _PARSE_HEADER_H_
