/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
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
 * Copyright(C) 2019 MediaTek Inc.
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
#include <MsTypes.h>
#include <stdio.h>
#include <linux/string.h>
#include "CommonDataType.h"
#include <dmalloc.h>
#include <ms_vector.h>
#include <demura.h>
#include <demura_common.h>
#include "DEMURA_BKA377.h"
#include "TCON_ADL_BKA367.h"
#include "halRegOp.h"
#include <demuraDll.h>
#include <debug_impl.h>

#define _4 4
#define _S7_MONO_RGB_DMC_H_LUT_NUM_MAX 132

static registers _reg = {
        //Align to machli_reg_sc_0406.xls
       {REG_0040_DEMURA_BKA377, 0x0000, REG_0040_DEMURA_BKA377_REG_DMC_BLACK_LIMIT},            //reg_dmc_black_limit
       {REG_0044_DEMURA_BKA377, 0x0FFF, REG_0044_DEMURA_BKA377_REG_DMC_PLANE_LEVEL1},           //reg_dmc_plane_level1
       {REG_0048_DEMURA_BKA377, 0x0FFF, REG_0048_DEMURA_BKA377_REG_DMC_PLANE_LEVEL2},           //reg_dmc_plane_level2
       {REG_004C_DEMURA_BKA377, 0x0FFF, REG_004C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL3},           //reg_dmc_plane_level3
       {REG_0050_DEMURA_BKA377, 0x0FFF, REG_0050_DEMURA_BKA377_REG_DMC_PLANE_LEVEL4},           //reg_dmc_plane_level4
       {REG_0054_DEMURA_BKA377, 0x0FFF, REG_0054_DEMURA_BKA377_REG_DMC_PLANE_LEVEL5},           //reg_dmc_plane_level5
       {REG_0058_DEMURA_BKA377, 0x0FFF, REG_0058_DEMURA_BKA377_REG_DMC_PLANE_LEVEL6},           //reg_dmc_plane_level6
       {REG_005C_DEMURA_BKA377, 0x0FFF, REG_005C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL7},           //reg_dmc_plane_level7
       {REG_0060_DEMURA_BKA377, 0x0FFF, REG_0060_DEMURA_BKA377_REG_DMC_PLANE_LEVEL8},           //reg_dmc_plane_level8
       {REG_0064_DEMURA_BKA377, 0x00FF, REG_0064_DEMURA_BKA377_REG_DMC_WHITE_LIMIT},            //reg_dmc_white_limit
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG4},            //reg_dmc_data_r_mag4
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG3},            //reg_dmc_data_r_mag3
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG2},            //reg_dmc_data_r_mag2
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG1},            //reg_dmc_data_r_mag1
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG8},            //reg_dmc_data_r_mag8
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG7},            //reg_dmc_data_r_mag7
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG6},            //reg_dmc_data_r_mag6
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG5},            //reg_dmc_data_r_mag5
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG4},            //reg_dmc_data_g_mag4
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG3},            //reg_dmc_data_g_mag3
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG2},            //reg_dmc_data_g_mag2
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG1},            //reg_dmc_data_g_mag1
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG8},            //reg_dmc_data_g_mag8
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG7},            //reg_dmc_data_g_mag7
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG6},            //reg_dmc_data_g_mag6
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG5},            //reg_dmc_data_g_mag5
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG4},            //reg_dmc_data_b_mag4
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG3},            //reg_dmc_data_b_mag3
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG2},            //reg_dmc_data_b_mag2
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG1},            //reg_dmc_data_b_mag1
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG8},            //reg_dmc_data_b_mag8
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG7},            //reg_dmc_data_b_mag7
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG6},            //reg_dmc_data_b_mag6
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG5},            //reg_dmc_data_b_mag5
       {REG_0080_DEMURA_BKA377, 0x0000, REG_0080_DEMURA_BKA377_REG_DMC_PLANE_B1_COEF},          //reg_dmc_plane_b1_coef
       {REG_0084_DEMURA_BKA377, 0x0000, REG_0084_DEMURA_BKA377_REG_DMC_PLANE_12_COEF},          //reg_dmc_plane_12_coef
       {REG_0088_DEMURA_BKA377, 0x0000, REG_0088_DEMURA_BKA377_REG_DMC_PLANE_23_COEF},          //reg_dmc_plane_23_coef
       {REG_008C_DEMURA_BKA377, 0x0000, REG_008C_DEMURA_BKA377_REG_DMC_PLANE_34_COEF},          //reg_dmc_plane_34_coef
       {REG_0090_DEMURA_BKA377, 0x0000, REG_0090_DEMURA_BKA377_REG_DMC_PLANE_45_COEF},          //reg_dmc_plane_45_coef
       {REG_0094_DEMURA_BKA377, 0x0000, REG_0094_DEMURA_BKA377_REG_DMC_PLANE_56_COEF},          //reg_dmc_plane_56_coef
       {REG_0098_DEMURA_BKA377, 0x0000, REG_0098_DEMURA_BKA377_REG_DMC_PLANE_67_COEF},          //reg_dmc_plane_67_coef
       {REG_009C_DEMURA_BKA377, 0x0000, REG_009C_DEMURA_BKA377_REG_DMC_PLANE_78_COEF},          //reg_dmc_plane_78_coef
       {REG_00A0_DEMURA_BKA377, 0x0000, REG_00A0_DEMURA_BKA377_REG_DMC_PLANE_9W_COEF},          //reg_dmc_plane_9w_coef
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_9W_KS22},          //reg_dmc_plane_9w_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_89_KS22},          //reg_dmc_plane_89_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_78_KS22},          //reg_dmc_plane_78_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_67_KS22},          //reg_dmc_plane_67_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_56_KS22},          //reg_dmc_plane_56_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_45_KS22},          //reg_dmc_plane_45_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_34_KS22},          //reg_dmc_plane_34_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_23_KS22},          //reg_dmc_plane_23_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_12_KS22},          //reg_dmc_plane_12_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_B1_KS22},          //reg_dmc_plane_b1_ks22
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_H_BLOCK},                //reg_dmc_h_block
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_V_BLOCK},                //reg_dmc_v_block
       {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_RGB_MODE},               //reg_dmc_rgb_mode
       {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_BLOCK_SIZE},             //reg_dmc_block_size
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_PLANE_NUM},              //reg_dmc_plane_num
       {REG_00AC_DEMURA_BKA377, 0x01E1, REG_00AC_DEMURA_BKA377_REG_DMC_H_LUT_NUM},              //reg_dmc_h_lut_num
       {REG_00B0_DEMURA_BKA377, 0x010F, REG_00B0_DEMURA_BKA377_REG_DMC_V_LUT_NUM},              //reg_dmc_v_lut_num
       {REG_00B4_DEMURA_BKA377, 0x0000, REG_00B4_DEMURA_BKA377_REG_DMC_DUMMY1},                 //reg_dmc_dummy1
       {REG_00B8_DEMURA_BKA377, 0x0000, REG_00B8_DEMURA_BKA377_REG_DMC_DUMMY2},                 //reg_dmc_dummy2
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_RST_E_4_FRAME},   //reg_dmc_dither_rst_e_4_frame
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_PSE_RST_NUM},     //reg_dmc_dither_pse_rst_num
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_POS_AF_GAMMA},           //reg_dmc_pos_af_gamma
       {REG_00BC_DEMURA_BKA377, 0x0001, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS},                 //reg_dmc_bypass
       {REG_00BC_DEMURA_BKA377, 0x0001, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_EN},              //reg_dmc_dither_en
       {REG_00BC_DEMURA_BKA377, 0x0001, REG_00BC_DEMURA_BKA377_REG_DMC_DB_EN},                  //reg_dmc_db_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_R_EN},                   //reg_dmc_r_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_G_EN},                   //reg_dmc_g_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_B_EN},                   //reg_dmc_b_en
       {REG_00C0_DEMURA_BKA377, 0x0000, REG_00C0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET1},         //reg_dmc_data_r_offset1
       {REG_00C4_DEMURA_BKA377, 0x0000, REG_00C4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET2},         //reg_dmc_data_r_offset2
       {REG_00C8_DEMURA_BKA377, 0x0000, REG_00C8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET3},         //reg_dmc_data_r_offset3
       {REG_00CC_DEMURA_BKA377, 0x0000, REG_00CC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET4},         //reg_dmc_data_r_offset4
       {REG_00D0_DEMURA_BKA377, 0x0000, REG_00D0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET5},         //reg_dmc_data_r_offset5
       {REG_00D4_DEMURA_BKA377, 0x0000, REG_00D4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET6},         //reg_dmc_data_r_offset6
       {REG_00D8_DEMURA_BKA377, 0x0000, REG_00D8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET7},         //reg_dmc_data_r_offset7
       {REG_00DC_DEMURA_BKA377, 0x0000, REG_00DC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET8},         //reg_dmc_data_r_offset8
       {REG_00E0_DEMURA_BKA377, 0x0000, REG_00E0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET1},         //reg_dmc_data_g_offset1
       {REG_00E4_DEMURA_BKA377, 0x0000, REG_00E4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET2},         //reg_dmc_data_g_offset2
       {REG_00E8_DEMURA_BKA377, 0x0000, REG_00E8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET3},         //reg_dmc_data_g_offset3
       {REG_00EC_DEMURA_BKA377, 0x0000, REG_00EC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET4},         //reg_dmc_data_g_offset4
       {REG_00F0_DEMURA_BKA377, 0x0000, REG_00F0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET5},         //reg_dmc_data_g_offset5
       {REG_00F4_DEMURA_BKA377, 0x0000, REG_00F4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET6},         //reg_dmc_data_g_offset6
       {REG_00F8_DEMURA_BKA377, 0x0000, REG_00F8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET7},         //reg_dmc_data_g_offset7
       {REG_00FC_DEMURA_BKA377, 0x0000, REG_00FC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET8},         //reg_dmc_data_g_offset8
       {REG_0100_DEMURA_BKA377, 0x0000, REG_0100_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET1},         //reg_dmc_data_b_offset1
       {REG_0104_DEMURA_BKA377, 0x0000, REG_0104_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET2},         //reg_dmc_data_b_offset2
       {REG_0108_DEMURA_BKA377, 0x0000, REG_0108_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET3},         //reg_dmc_data_b_offset3
       {REG_010C_DEMURA_BKA377, 0x0000, REG_010C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET4},         //reg_dmc_data_b_offset4
       {REG_0110_DEMURA_BKA377, 0x0000, REG_0110_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET5},         //reg_dmc_data_b_offset5
       {REG_0114_DEMURA_BKA377, 0x0000, REG_0114_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET6},         //reg_dmc_data_b_offset6
       {REG_0118_DEMURA_BKA377, 0x0000, REG_0118_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET7},         //reg_dmc_data_b_offset7
       {REG_011C_DEMURA_BKA377, 0x0000, REG_011C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET8},         //reg_dmc_data_b_offset8
       {REG_0120_DEMURA_BKA377, 0x0000, REG_0120_DEMURA_BKA377_REG_DMC_PANEL_H_SIZE},           //reg_dmc_panel_h_size
       {REG_0124_DEMURA_BKA377, 0x0002, REG_0124_DEMURA_BKA377_REG_DMC_PRELOAD_LINE},           //reg_dmc_preload_line
//     {REG_0128_DEMURA_BKA377, 0x0000, REG_0128_DEMURA_BKA377_REG_DMC_R_H_R},                  //reg_dmc_r_h_r
//     {REG_0128_DEMURA_BKA377, 0x0000, REG_0128_DEMURA_BKA377_REG_DMC_Q_H_R},                  //reg_dmc_q_h_r
//     {REG_012C_DEMURA_BKA377, 0x0000, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_EN},           //reg_dmc_grid_line_en
//     {REG_012C_DEMURA_BKA377, 0x0000, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_SIZE},         //reg_dmc_grid_line_size
//     {REG_012C_DEMURA_BKA377, 0x00FF, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_COLOR},        //reg_dmc_grid_line_color
       {REG_0130_DEMURA_BKA377, 0x0000, REG_0130_DEMURA_BKA377_REG_DMC_FLOW_CTRL_EN},           //reg_dmc_flow_ctrl_en
       {REG_0130_DEMURA_BKA377, 0x0010, REG_0130_DEMURA_BKA377_REG_DMC_FLOW_LEN},               //reg_dmc_flow_len
//     {REG_013C_DEMURA_BKA377, 0x0000, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_XP},            //reg_adl_checksum_xp
//     {REG_013C_DEMURA_BKA377, 0x0000, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_LR},            //reg_adl_checksum_lr
//     {REG_013C_DEMURA_BKA377, 0x0001, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_EN},            //reg_adl_checksum_en
       {REG_013C_DEMURA_BKA377, 0x0008, REG_013C_DEMURA_BKA377_REG_DMC_GUARD_BAND_SIZE},        //reg_adl_checksum_en
       {REG_0180_DEMURA_BKA377, 0x0FFF, REG_0180_DEMURA_BKA377_REG_DMC_PLANE_LEVEL9},           //reg_dmc_plane_level9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_R_MAG9},            //reg_dmc_data_r_mag9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_G_MAG9},            //reg_dmc_data_g_mag9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_B_MAG9},            //reg_dmc_data_b_mag9
       {REG_0188_DEMURA_BKA377, 0x0000, REG_0188_DEMURA_BKA377_REG_DMC_PLANE_89_COEF},          //reg_dmc_plane_89_coef
       {REG_018C_DEMURA_BKA377, 0x0000, REG_018C_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET9},         //reg_dmc_data_r_offset9
       {REG_0190_DEMURA_BKA377, 0x0000, REG_0190_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET9},         //reg_dmc_data_g_offset9
       {REG_0194_DEMURA_BKA377, 0x0000, REG_0194_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET9},         //reg_dmc_data_b_offset9
       {REG_0198_DEMURA_BKA377, 0x8000, REG_0198_DEMURA_BKA377_REG_DMC_DELTA_LOW_LIMIT},        //reg_dmc_delta_low_limit
       {REG_019C_DEMURA_BKA377, 0x7FFF, REG_019C_DEMURA_BKA377_REG_DMC_DELTA_HIGH_LIMIT},       //reg_dmc_delta_high_limit
       {REG_01A0_DEMURA_BKA377, 0x0000, REG_01A0_DEMURA_BKA377_REG_DMC_POS_V_OFFSET},           //reg_dmc_pos_v_offset
       {REG_01A0_DEMURA_BKA377, 0x0000, REG_01A0_DEMURA_BKA377_REG_DMC_POS_H_OFFSET},           //reg_dmc_pos_h_offset
       {REG_01A4_DEMURA_BKA377, 0x0001, REG_01A4_DEMURA_BKA377_REG_DMC_L1_EQ_0_EN},             //reg_dmc_l1_eq_0_en
       {REG_01A4_DEMURA_BKA377, 0x00010, REG_01A4_DEMURA_BKA377_REG_DMC_FLOW_REST}              //reg_dmc_flow_rest
//     {REG_01B4_DEMURA_BKA377, , REG_01B4_DEMURA_BKA377_REG_DMC_BIST_FAIL_0},                  //reg_dmc_bist_fail_0
//     {REG_01B8_DEMURA_BKA377, , REG_01B8_DEMURA_BKA377_REG_DMC_BIST_FAIL_1},                  //reg_dmc_bist_fail_1
//     {REG_01BC_DEMURA_BKA377, , REG_01BC_DEMURA_BKA377_REG_DMC_BIST_FAIL_2},                  //reg_dmc_bist_fail_2
};

void demura_core_ver10_mono_extern(int lut_h_size, interface_info *pInfo)
{
    if ((*pInfo).reg_dmc_rgb_mode == 1) // 0 : mono mode 1 : rgb mode
    {
        _reg.reg_dmc_rgb_mode.val = 0;
        UBOOT_ERROR("demura_get_version: %d, not support rgb mode, change to mono mode\n", demura_get_version());
    }

    if (_reg.reg_dmc_plane_num.val > _4)
    {
        if (((lut_h_size / _4) + ((lut_h_size % _4 == 0) ? 0 : 1)) > _S7_MONO_RGB_DMC_H_LUT_NUM_MAX)
        {
            _reg.reg_dmc_plane_num.val = _4;
            UBOOT_ERROR("demura plane_num only support %d layer\n", _reg.reg_dmc_plane_num.val);
        }
    }
}

MS_BOOL demura_core_ver10(int base_address, int lut_h_size, int lut_v_size, interface_info *pInfo,
                        m_vector *pRegVector, m_vector *pLut_out_vector)
{
    //int reg_bank = base_address;  // 0x11200
    MS_U16 width, height, shift;
    demura_get_panelinfo(&width, &height);
#if 0
    //reg_struct
    registers _reg = {
        //Align to machli_reg_sc_0406.xls
       {REG_0040_DEMURA_BKA377, 0x0000, REG_0040_DEMURA_BKA377_REG_DMC_BLACK_LIMIT},            //reg_dmc_black_limit
       {REG_0044_DEMURA_BKA377, 0x0FFF, REG_0044_DEMURA_BKA377_REG_DMC_PLANE_LEVEL1},           //reg_dmc_plane_level1
       {REG_0048_DEMURA_BKA377, 0x0FFF, REG_0048_DEMURA_BKA377_REG_DMC_PLANE_LEVEL2},           //reg_dmc_plane_level2
       {REG_004C_DEMURA_BKA377, 0x0FFF, REG_004C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL3},           //reg_dmc_plane_level3
       {REG_0050_DEMURA_BKA377, 0x0FFF, REG_0050_DEMURA_BKA377_REG_DMC_PLANE_LEVEL4},           //reg_dmc_plane_level4
       {REG_0054_DEMURA_BKA377, 0x0FFF, REG_0054_DEMURA_BKA377_REG_DMC_PLANE_LEVEL5},           //reg_dmc_plane_level5
       {REG_0058_DEMURA_BKA377, 0x0FFF, REG_0058_DEMURA_BKA377_REG_DMC_PLANE_LEVEL6},           //reg_dmc_plane_level6
       {REG_005C_DEMURA_BKA377, 0x0FFF, REG_005C_DEMURA_BKA377_REG_DMC_PLANE_LEVEL7},           //reg_dmc_plane_level7
       {REG_0060_DEMURA_BKA377, 0x0FFF, REG_0060_DEMURA_BKA377_REG_DMC_PLANE_LEVEL8},           //reg_dmc_plane_level8
       {REG_0064_DEMURA_BKA377, 0x00FF, REG_0064_DEMURA_BKA377_REG_DMC_WHITE_LIMIT},            //reg_dmc_white_limit
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG4},            //reg_dmc_data_r_mag4
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG3},            //reg_dmc_data_r_mag3
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG2},            //reg_dmc_data_r_mag2
       {REG_0068_DEMURA_BKA377, 0x0000, REG_0068_DEMURA_BKA377_REG_DMC_DATA_R_MAG1},            //reg_dmc_data_r_mag1
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG8},            //reg_dmc_data_r_mag8
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG7},            //reg_dmc_data_r_mag7
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG6},            //reg_dmc_data_r_mag6
       {REG_006C_DEMURA_BKA377, 0x0000, REG_006C_DEMURA_BKA377_REG_DMC_DATA_R_MAG5},            //reg_dmc_data_r_mag5
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG4},            //reg_dmc_data_g_mag4
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG3},            //reg_dmc_data_g_mag3
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG2},            //reg_dmc_data_g_mag2
       {REG_0070_DEMURA_BKA377, 0x0000, REG_0070_DEMURA_BKA377_REG_DMC_DATA_G_MAG1},            //reg_dmc_data_g_mag1
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG8},            //reg_dmc_data_g_mag8
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG7},            //reg_dmc_data_g_mag7
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG6},            //reg_dmc_data_g_mag6
       {REG_0074_DEMURA_BKA377, 0x0000, REG_0074_DEMURA_BKA377_REG_DMC_DATA_G_MAG5},            //reg_dmc_data_g_mag5
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG4},            //reg_dmc_data_b_mag4
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG3},            //reg_dmc_data_b_mag3
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG2},            //reg_dmc_data_b_mag2
       {REG_0078_DEMURA_BKA377, 0x0000, REG_0078_DEMURA_BKA377_REG_DMC_DATA_B_MAG1},            //reg_dmc_data_b_mag1
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG8},            //reg_dmc_data_b_mag8
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG7},            //reg_dmc_data_b_mag7
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG6},            //reg_dmc_data_b_mag6
       {REG_007C_DEMURA_BKA377, 0x0000, REG_007C_DEMURA_BKA377_REG_DMC_DATA_B_MAG5},            //reg_dmc_data_b_mag5
       {REG_0080_DEMURA_BKA377, 0x0000, REG_0080_DEMURA_BKA377_REG_DMC_PLANE_B1_COEF},          //reg_dmc_plane_b1_coef
       {REG_0084_DEMURA_BKA377, 0x0000, REG_0084_DEMURA_BKA377_REG_DMC_PLANE_12_COEF},          //reg_dmc_plane_12_coef
       {REG_0088_DEMURA_BKA377, 0x0000, REG_0088_DEMURA_BKA377_REG_DMC_PLANE_23_COEF},          //reg_dmc_plane_23_coef
       {REG_008C_DEMURA_BKA377, 0x0000, REG_008C_DEMURA_BKA377_REG_DMC_PLANE_34_COEF},          //reg_dmc_plane_34_coef
       {REG_0090_DEMURA_BKA377, 0x0000, REG_0090_DEMURA_BKA377_REG_DMC_PLANE_45_COEF},          //reg_dmc_plane_45_coef
       {REG_0094_DEMURA_BKA377, 0x0000, REG_0094_DEMURA_BKA377_REG_DMC_PLANE_56_COEF},          //reg_dmc_plane_56_coef
       {REG_0098_DEMURA_BKA377, 0x0000, REG_0098_DEMURA_BKA377_REG_DMC_PLANE_67_COEF},          //reg_dmc_plane_67_coef
       {REG_009C_DEMURA_BKA377, 0x0000, REG_009C_DEMURA_BKA377_REG_DMC_PLANE_78_COEF},          //reg_dmc_plane_78_coef
       {REG_00A0_DEMURA_BKA377, 0x0000, REG_00A0_DEMURA_BKA377_REG_DMC_PLANE_9W_COEF},          //reg_dmc_plane_9w_coef
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_9W_KS22},          //reg_dmc_plane_9w_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_89_KS22},          //reg_dmc_plane_89_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_78_KS22},          //reg_dmc_plane_78_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_67_KS22},          //reg_dmc_plane_67_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_56_KS22},          //reg_dmc_plane_56_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_45_KS22},          //reg_dmc_plane_45_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_34_KS22},          //reg_dmc_plane_34_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_23_KS22},          //reg_dmc_plane_23_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_12_KS22},          //reg_dmc_plane_12_ks22
       {REG_00A4_DEMURA_BKA377, 0x0000, REG_00A4_DEMURA_BKA377_REG_DMC_PLANE_B1_KS22},          //reg_dmc_plane_b1_ks22
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_H_BLOCK},                //reg_dmc_h_block
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_V_BLOCK},                //reg_dmc_v_block
       {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_RGB_MODE},               //reg_dmc_rgb_mode
       {REG_00A8_DEMURA_BKA377, 0x0001, REG_00A8_DEMURA_BKA377_REG_DMC_BLOCK_SIZE},             //reg_dmc_block_size
       {REG_00A8_DEMURA_BKA377, 0x0003, REG_00A8_DEMURA_BKA377_REG_DMC_PLANE_NUM},              //reg_dmc_plane_num
       {REG_00AC_DEMURA_BKA377, 0x01E1, REG_00AC_DEMURA_BKA377_REG_DMC_H_LUT_NUM},              //reg_dmc_h_lut_num
       {REG_00B0_DEMURA_BKA377, 0x010F, REG_00B0_DEMURA_BKA377_REG_DMC_V_LUT_NUM},              //reg_dmc_v_lut_num
       {REG_00B4_DEMURA_BKA377, 0x0000, REG_00B4_DEMURA_BKA377_REG_DMC_DUMMY1},                 //reg_dmc_dummy1
       {REG_00B8_DEMURA_BKA377, 0x0000, REG_00B8_DEMURA_BKA377_REG_DMC_DUMMY2},                 //reg_dmc_dummy2
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_RST_E_4_FRAME},   //reg_dmc_dither_rst_e_4_frame
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_PSE_RST_NUM},     //reg_dmc_dither_pse_rst_num
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_POS_AF_GAMMA},           //reg_dmc_pos_af_gamma
       {REG_00BC_DEMURA_BKA377, 0x0001, REG_00BC_DEMURA_BKA377_REG_DMC_BYPASS},                 //reg_dmc_bypass
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_DITHER_EN},              //reg_dmc_dither_en
       {REG_00BC_DEMURA_BKA377, 0x0001, REG_00BC_DEMURA_BKA377_REG_DMC_DB_EN},                  //reg_dmc_db_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_R_EN},                   //reg_dmc_r_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_G_EN},                   //reg_dmc_g_en
       {REG_00BC_DEMURA_BKA377, 0x0000, REG_00BC_DEMURA_BKA377_REG_DMC_B_EN},                   //reg_dmc_b_en
       {REG_00C0_DEMURA_BKA377, 0x0000, REG_00C0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET1},         //reg_dmc_data_r_offset1
       {REG_00C4_DEMURA_BKA377, 0x0000, REG_00C4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET2},         //reg_dmc_data_r_offset2
       {REG_00C8_DEMURA_BKA377, 0x0000, REG_00C8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET3},         //reg_dmc_data_r_offset3
       {REG_00CC_DEMURA_BKA377, 0x0000, REG_00CC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET4},         //reg_dmc_data_r_offset4
       {REG_00D0_DEMURA_BKA377, 0x0000, REG_00D0_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET5},         //reg_dmc_data_r_offset5
       {REG_00D4_DEMURA_BKA377, 0x0000, REG_00D4_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET6},         //reg_dmc_data_r_offset6
       {REG_00D8_DEMURA_BKA377, 0x0000, REG_00D8_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET7},         //reg_dmc_data_r_offset7
       {REG_00DC_DEMURA_BKA377, 0x0000, REG_00DC_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET8},         //reg_dmc_data_r_offset8
       {REG_00E0_DEMURA_BKA377, 0x0000, REG_00E0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET1},         //reg_dmc_data_g_offset1
       {REG_00E4_DEMURA_BKA377, 0x0000, REG_00E4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET2},         //reg_dmc_data_g_offset2
       {REG_00E8_DEMURA_BKA377, 0x0000, REG_00E8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET3},         //reg_dmc_data_g_offset3
       {REG_00EC_DEMURA_BKA377, 0x0000, REG_00EC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET4},         //reg_dmc_data_g_offset4
       {REG_00F0_DEMURA_BKA377, 0x0000, REG_00F0_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET5},         //reg_dmc_data_g_offset5
       {REG_00F4_DEMURA_BKA377, 0x0000, REG_00F4_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET6},         //reg_dmc_data_g_offset6
       {REG_00F8_DEMURA_BKA377, 0x0000, REG_00F8_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET7},         //reg_dmc_data_g_offset7
       {REG_00FC_DEMURA_BKA377, 0x0000, REG_00FC_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET8},         //reg_dmc_data_g_offset8
       {REG_0100_DEMURA_BKA377, 0x0000, REG_0100_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET1},         //reg_dmc_data_b_offset1
       {REG_0104_DEMURA_BKA377, 0x0000, REG_0104_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET2},         //reg_dmc_data_b_offset2
       {REG_0108_DEMURA_BKA377, 0x0000, REG_0108_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET3},         //reg_dmc_data_b_offset3
       {REG_010C_DEMURA_BKA377, 0x0000, REG_010C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET4},         //reg_dmc_data_b_offset4
       {REG_0110_DEMURA_BKA377, 0x0000, REG_0110_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET5},         //reg_dmc_data_b_offset5
       {REG_0114_DEMURA_BKA377, 0x0000, REG_0114_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET6},         //reg_dmc_data_b_offset6
       {REG_0118_DEMURA_BKA377, 0x0000, REG_0118_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET7},         //reg_dmc_data_b_offset7
       {REG_011C_DEMURA_BKA377, 0x0000, REG_011C_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET8},         //reg_dmc_data_b_offset8
       {REG_0120_DEMURA_BKA377, 0x0000, REG_0120_DEMURA_BKA377_REG_DMC_PANEL_H_SIZE},           //reg_dmc_panel_h_size
       {REG_0124_DEMURA_BKA377, 0x0002, REG_0124_DEMURA_BKA377_REG_DMC_PRELOAD_LINE},           //reg_dmc_preload_line
//     {REG_0128_DEMURA_BKA377, 0x0000, REG_0128_DEMURA_BKA377_REG_DMC_R_H_R},                  //reg_dmc_r_h_r
//     {REG_0128_DEMURA_BKA377, 0x0000, REG_0128_DEMURA_BKA377_REG_DMC_Q_H_R},                  //reg_dmc_q_h_r
//     {REG_012C_DEMURA_BKA377, 0x0000, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_EN},           //reg_dmc_grid_line_en
//     {REG_012C_DEMURA_BKA377, 0x0000, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_SIZE},         //reg_dmc_grid_line_size
//     {REG_012C_DEMURA_BKA377, 0x00FF, REG_012C_DEMURA_BKA377_REG_DMC_GRID_LINE_COLOR},        //reg_dmc_grid_line_color
       {REG_0130_DEMURA_BKA377, 0x0000, REG_0130_DEMURA_BKA377_REG_DMC_FLOW_CTRL_EN},           //reg_dmc_flow_ctrl_en
       {REG_0130_DEMURA_BKA377, 0x0010, REG_0130_DEMURA_BKA377_REG_DMC_FLOW_LEN},               //reg_dmc_flow_len
//     {REG_013C_DEMURA_BKA377, 0x0000, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_XP},            //reg_adl_checksum_xp
//     {REG_013C_DEMURA_BKA377, 0x0000, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_LR},            //reg_adl_checksum_lr
//     {REG_013C_DEMURA_BKA377, 0x0001, REG_013C_DEMURA_BKA377_REG_ADL_CHECKSUM_EN},            //reg_adl_checksum_en
       {REG_013C_DEMURA_BKA377, 0x0008, REG_013C_DEMURA_BKA377_REG_DMC_GUARD_BAND_SIZE},        //reg_adl_checksum_en
       {REG_0180_DEMURA_BKA377, 0x0FFF, REG_0180_DEMURA_BKA377_REG_DMC_PLANE_LEVEL9},           //reg_dmc_plane_level9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_R_MAG9},            //reg_dmc_data_r_mag9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_G_MAG9},            //reg_dmc_data_g_mag9
       {REG_0184_DEMURA_BKA377, 0x0000, REG_0184_DEMURA_BKA377_REG_DMC_DATA_B_MAG9},            //reg_dmc_data_b_mag9
       {REG_0188_DEMURA_BKA377, 0x0000, REG_0188_DEMURA_BKA377_REG_DMC_PLANE_89_COEF},          //reg_dmc_plane_89_coef
       {REG_018C_DEMURA_BKA377, 0x0000, REG_018C_DEMURA_BKA377_REG_DMC_DATA_R_OFFSET9},         //reg_dmc_data_r_offset9
       {REG_0190_DEMURA_BKA377, 0x0000, REG_0190_DEMURA_BKA377_REG_DMC_DATA_G_OFFSET9},         //reg_dmc_data_g_offset9
       {REG_0194_DEMURA_BKA377, 0x0000, REG_0194_DEMURA_BKA377_REG_DMC_DATA_B_OFFSET9},         //reg_dmc_data_b_offset9
       {REG_0198_DEMURA_BKA377, 0x8000, REG_0198_DEMURA_BKA377_REG_DMC_DELTA_LOW_LIMIT},        //reg_dmc_delta_low_limit
       {REG_019C_DEMURA_BKA377, 0x7FFF, REG_019C_DEMURA_BKA377_REG_DMC_DELTA_HIGH_LIMIT},       //reg_dmc_delta_high_limit
       {REG_01A0_DEMURA_BKA377, 0x0000, REG_01A0_DEMURA_BKA377_REG_DMC_POS_V_OFFSET},           //reg_dmc_pos_v_offset
       {REG_01A0_DEMURA_BKA377, 0x0000, REG_01A0_DEMURA_BKA377_REG_DMC_POS_H_OFFSET},           //reg_dmc_pos_h_offset
       {REG_01A4_DEMURA_BKA377, 0x0001, REG_01A4_DEMURA_BKA377_REG_DMC_L1_EQ_0_EN},             //reg_dmc_l1_eq_0_en
       {REG_01A4_DEMURA_BKA377, 0x00010, REG_01A4_DEMURA_BKA377_REG_DMC_FLOW_REST}              //reg_dmc_flow_rest
//     {REG_01B4_DEMURA_BKA377, , REG_01B4_DEMURA_BKA377_REG_DMC_BIST_FAIL_0},                  //reg_dmc_bist_fail_0
//     {REG_01B8_DEMURA_BKA377, , REG_01B8_DEMURA_BKA377_REG_DMC_BIST_FAIL_1},                  //reg_dmc_bist_fail_1
//     {REG_01BC_DEMURA_BKA377, , REG_01BC_DEMURA_BKA377_REG_DMC_BIST_FAIL_2},                  //reg_dmc_bist_fail_2
    };
#endif



    //int Bit07_00;
    //int Bit15_08;

    DMC_GET_LUT_SHIFT ((*pInfo).reg_dmc_h_block, width, lut_h_size);
    DMC_GET_LUT_SHIFT ((*pInfo).reg_dmc_v_block, height, lut_v_size);

    if (demura_get_version() == E_DEMURA_VERSION_S7_MONO && (*pInfo).reg_dmc_rgb_mode == 1) // 0 : mono mode , 1 : rgb mode
    {
        UBOOT_ERROR("demura_get_version: %d, not support rgb mode\n", demura_get_version());
    }

    //----------------------- interface input start -----------------------//
    _reg.reg_dmc_plane_num.val              = (*pInfo).reg_dmc_plane_num; // 1 ~ 8
    _reg.reg_dmc_h_block.val                = (*pInfo).reg_dmc_h_block; // 2 ~ 4 for U13 , 2 ~ 6 for Odinson , 2 : 4 pixel , 3 : 8 pixel , 4 : 16 pixel , 5 : 32 pixel , 6 : 64 pixel
    _reg.reg_dmc_v_block.val                = (*pInfo).reg_dmc_v_block; // 2 ~ 4 for U13 , 2 ~ 6 for Odinson , 2 : 4 pixel , 3 : 8 pixel , 4 : 16 pixel , 5 : 32 pixel , 6 : 64 pixel
    _reg.reg_dmc_rgb_mode.val               = (*pInfo).reg_dmc_rgb_mode; // 0 : mono mode , 1 : rgb mode
    _reg.reg_dmc_panel_h_size.val           = (*pInfo).reg_dmc_panel_h_size; // panel real horizontal size
    _reg.reg_dmc_r_en.val                   = (*pInfo).reg_dmc_r_en;
    _reg.reg_dmc_g_en.val                   = (*pInfo).reg_dmc_g_en;
    _reg.reg_dmc_b_en.val                   = (*pInfo).reg_dmc_b_en;
    //int reg_dmc_w_en                      = 1;
    //int reg_dmc_dga_en                    = 1;
    _reg.reg_dmc_black_limit.val            = (*pInfo).reg_dmc_black_limit;  // 12 bit , format 10.2
    _reg.reg_dmc_plane_level1.val           = (*pInfo).reg_dmc_plane_level1; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level2.val           = (*pInfo).reg_dmc_plane_level2; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level3.val           = (*pInfo).reg_dmc_plane_level3; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level4.val           = (*pInfo).reg_dmc_plane_level4; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level5.val           = (*pInfo).reg_dmc_plane_level5; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level6.val           = (*pInfo).reg_dmc_plane_level6; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level7.val           = (*pInfo).reg_dmc_plane_level7; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level8.val           = (*pInfo).reg_dmc_plane_level8; // 12 bit , format 10.2
    _reg.reg_dmc_plane_level9.val           = (*pInfo).reg_dmc_plane_level9; // 12 bit , format 10.2
    _reg.reg_dmc_white_limit.val            = (*pInfo).reg_dmc_white_limit; // 12 bit , format 10.2
    _reg.reg_dmc_dither_rst_e_4_frame.val   = (*pInfo).reg_dmc_dither_rst_e_4_frame; // 0 : dither frame reset off , 1 : dither frame reset on
    _reg.reg_dmc_dither_pse_rst_num.val     = (*pInfo).reg_dmc_dither_pse_rst_num; // 0 : 1 frame reset , 1 : 2 frame reset , 2 : 4 frame reset , 3 : 8 frame reset
    _reg.reg_dmc_dither_en.val              = (*pInfo).reg_dmc_dither_en; // 2 bit , 0 : rounding , 1 : dither , 2~3 truncate
    //----------------------- interface input end -----------------------//
    if (demura_get_version() == E_DEMURA_VERSION_S7_MONO) // only mono case
    {
        demura_core_ver10_mono_extern(lut_h_size, pInfo);
    }

    //--------------------- register Layer coeff and ks22 start ---------------------//

    int diff_b1 = MAX(_reg.reg_dmc_plane_level1.val * 4 - _reg.reg_dmc_black_limit.val  * 4, 1); // layer distance is 14 bit , input layer is 12 (10.2) bit , so shift 2 bit keep 14 bit
    int diff_12 = MAX(_reg.reg_dmc_plane_level2.val * 4 - _reg.reg_dmc_plane_level1.val * 4, 1);
    int diff_23 = MAX(_reg.reg_dmc_plane_level3.val * 4 - _reg.reg_dmc_plane_level2.val * 4, 1);
    int diff_34 = MAX(_reg.reg_dmc_plane_level4.val * 4 - _reg.reg_dmc_plane_level3.val * 4, 1);
    int diff_45 = MAX(_reg.reg_dmc_plane_level5.val * 4 - _reg.reg_dmc_plane_level4.val * 4, 1);
    int diff_56 = MAX(_reg.reg_dmc_plane_level6.val * 4 - _reg.reg_dmc_plane_level5.val * 4, 1);
    int diff_67 = MAX(_reg.reg_dmc_plane_level7.val * 4 - _reg.reg_dmc_plane_level6.val * 4, 1);
    int diff_78 = MAX(_reg.reg_dmc_plane_level8.val * 4 - _reg.reg_dmc_plane_level7.val * 4, 1);
    int diff_89 = MAX(_reg.reg_dmc_plane_level9.val * 4 - _reg.reg_dmc_plane_level8.val * 4, 1);
    int diff_9w = MAX(_reg.reg_dmc_white_limit.val  * 4 - _reg.reg_dmc_plane_level9.val * 4, 1);

    switch(_reg.reg_dmc_plane_num.val)
    {
        case 1:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level1.val * 4, 1);
            break;
        case 2:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level2.val * 4, 1);
            break;
        case 3:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level3.val * 4, 1);
            break;
        case 4:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level4.val * 4, 1);
            break;
        case 5:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level5.val * 4, 1);
            break;
        case 6:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level6.val * 4, 1);
            break;
        case 7:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level7.val * 4, 1);
            break;
        case 8:
            diff_9w = MAX(_reg.reg_dmc_white_limit.val * 4  - _reg.reg_dmc_plane_level8.val * 4, 1);
            break;
        default:
            break;
    }


    int coeff = 0;
    int ks = 0;

    coeff_and_ks( diff_b1, &coeff, &ks);
    _reg.reg_dmc_plane_b1_coef.val = coeff;
    _reg.reg_dmc_plane_b1_ks22.val = ks;

    coeff_and_ks( diff_12, &coeff, &ks);
    _reg.reg_dmc_plane_12_coef.val = coeff;
    _reg.reg_dmc_plane_12_ks22.val = ks;

    coeff_and_ks( diff_23, &coeff, &ks);
    _reg.reg_dmc_plane_23_coef.val = coeff;
    _reg.reg_dmc_plane_23_ks22.val = ks;

    coeff_and_ks( diff_34, &coeff, &ks);
    _reg.reg_dmc_plane_34_coef.val = coeff;
    _reg.reg_dmc_plane_34_ks22.val = ks;

    coeff_and_ks( diff_45, &coeff, &ks);
    _reg.reg_dmc_plane_45_coef.val = coeff;
    _reg.reg_dmc_plane_45_ks22.val = ks;

    coeff_and_ks( diff_56, &coeff, &ks);
    _reg.reg_dmc_plane_56_coef.val = coeff;
    _reg.reg_dmc_plane_56_ks22.val = ks;

    coeff_and_ks( diff_67, &coeff, &ks);
    _reg.reg_dmc_plane_67_coef.val = coeff;
    _reg.reg_dmc_plane_67_ks22.val = ks;

    coeff_and_ks( diff_78, &coeff, &ks);
    _reg.reg_dmc_plane_78_coef.val = coeff;
    _reg.reg_dmc_plane_78_ks22.val = ks;

    if (demura_get_version() == E_DEMURA_VERSION_S11)
    {
        coeff_and_ks(diff_89, &coeff, &ks);
    }
    else
    {
        coeff_and_ks(diff_9w, &coeff, &ks);
    }
    _reg.reg_dmc_plane_89_coef.val = coeff;
    _reg.reg_dmc_plane_89_ks22.val = ks;

    coeff_and_ks( diff_9w, &coeff, &ks);
    _reg.reg_dmc_plane_9w_coef.val = coeff;
    _reg.reg_dmc_plane_9w_ks22.val = ks;
    //--------------------- register Layer coeff and ks22 end ---------------------//

    //--------------------- register H_lut_num/V_lut_num start ---------------------//
    int block_height = lut_v_size;
    int reg_dmc_v_lut_num = block_height;

    int block_width = lut_h_size;
    int reg_dmc_h_lut_num = block_width;


    if(_reg.reg_dmc_rgb_mode.val)
    {
        int div_val;
        switch (_reg.reg_dmc_plane_num.val)
        {
        case 1:
        case 2:
            div_val = 4;
            break;
        case 3:
        case 4:
            div_val = 2;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        default:
            div_val = 1;
            break;
        }

        int reg_dmc_h_lut_num_tmp = reg_dmc_h_lut_num;

        reg_dmc_h_lut_num = (reg_dmc_h_lut_num_tmp/div_val) + (((reg_dmc_h_lut_num_tmp%div_val) > 0) ? 1 : 0);

    }
    else
    {
        int div_val;
        switch (_reg.reg_dmc_plane_num.val)
        {
        case 1:
        case 2:
            div_val = 16;
            break;
        case 3:
        case 4:
            div_val = 8;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        default:
            div_val = 4;
            break;
        }

        int reg_dmc_h_lut_num_tmp = reg_dmc_h_lut_num;

        reg_dmc_h_lut_num = (reg_dmc_h_lut_num_tmp/div_val) + (((reg_dmc_h_lut_num_tmp%div_val) > 0) ? 1 : 0);
    }

    _reg.reg_dmc_h_lut_num.val = (MS_U16)reg_dmc_h_lut_num;
    _reg.reg_dmc_v_lut_num.val = (MS_U16)reg_dmc_v_lut_num;
    //--------------------- register H_lut_num/V_lut_num end ---------------------//

    //--------------------- register demura flow control enable start---------------------//
    _reg.reg_dmc_flow_ctrl_en.val = 1;
    _reg.reg_dmc_flow_len.val = (MS_U16)reg_dmc_h_lut_num;
    //part_bit15_00(reg_dmc_h_lut_num, 11, &Bit15_08, &Bit07_00);
    //_reg.reg_dmc_flow_len.Bit07_00.value                          = register_shift(_reg.reg_dmc_flow_len.Bit07_00.mask, Bit07_00);
    //_reg.reg_dmc_flow_len.Bit15_08.value                          = register_shift(_reg.reg_dmc_flow_len.Bit15_08.mask, Bit15_08);

    //_reg.reg_dmc_flow_ctrl_en.Bit07_00.value                      = register_shift(_reg.reg_dmc_flow_ctrl_en.Bit07_00.mask, 1);
    //--------------------- register demura flow control enable end---------------------//


    //--------------------- layer LUT initial start ---------------------//

    strgb_structInfo *layerall_blk_img[8];
    memset(layerall_blk_img, 0, sizeof(layerall_blk_img));

    //--------------------- layer LUT initial end ---------------------//


    //--------------------- calculation layer LUT start ---------------------//
    int plane_idx;
    for (plane_idx = 0; plane_idx < _reg.reg_dmc_plane_num.val; plane_idx++)
    {

        int reg_dmc_plane_level = _reg.reg_dmc_plane_level1.val;
        switch(plane_idx)
        {
        case 0:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level1.val;
            break;
        case 1:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level2.val;
            break;
        case 2:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level3.val;
            break;
        case 3:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level4.val;
            break;
        case 4:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level5.val;
            break;
        case 5:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level6.val;
            break;
        case 6:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level7.val;
            break;
        case 7:
            reg_dmc_plane_level = _reg.reg_dmc_plane_level8.val;
            break;
        }

        {
            // Reduce memory-usage  step-1
            int layer_blk_img_size = sizeof(strgb_structInfo) * block_height * block_width;
            char *pblk_img = dmalloc(layer_blk_img_size);
            CHECK_DMALLOC_SPACE(pblk_img, (unsigned int)layer_blk_img_size);
            layerall_blk_img[plane_idx] = (strgb_structInfo *)pblk_img;

            int CurV, CurH;
            for (CurV = 0; CurV < block_height; CurV++)
            {
                for (CurH = 0; CurH < block_width; CurH++)
                {
                    layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].r    = 0;
                    layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].g    = 0;
                    layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].b    = 0;
                    //layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].w    = 0;
                    //layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].dbr  = 0.0;
                    //layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].dbg  = 0.0;
                    //layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].dbb  = 0.0;
                    //layerall_blk_img[plane_idx][posi_blk(CurV,CurH)].dbw  = 0.0;
                }
            }
        }

        //calculation LUT?Bmag?Moffset register
        LUT_AND_SETTING_PROCESSOR(plane_idx, block_width, block_height, reg_dmc_plane_level, &_reg , (*pInfo).Lut_in[plane_idx], layerall_blk_img[plane_idx], (*pInfo).reg_dmc_rgb_mode);

        // Reduce memory-usage  step-2
        dfree( (*pInfo).Lut_in[plane_idx] );
        (*pInfo).Lut_in[plane_idx] = NULL;
    }
    //--------------------- calculation layer LUT end ---------------------//

    stlayer_info_structInfo layer_info;
    layer_info.dram_h_size = block_width;
    layer_info.dram_v_size = block_height;
    layer_info.dmc_rgb_mode = _reg.reg_dmc_rgb_mode.val;
    layer_info.real_dram_h_size = reg_dmc_h_lut_num;
    layer_info.real_dram_v_size = reg_dmc_v_lut_num;
    layer_info.dmc_plane_num = _reg.reg_dmc_plane_num.val;
    write_demuraLUT_to_HW_DRAM_FORMAT(&layer_info, layerall_blk_img, pLut_out_vector);


    for (plane_idx = 0; plane_idx < _reg.reg_dmc_plane_num.val; plane_idx++)
    {
        dfree(layerall_blk_img[plane_idx]);     // free layerall_blk_img arrary
    }
    /*
    for (plane_idx = reg_dmc_plane_num; plane_idx < 8; plane_idx++)
    {
        dfree( (*pInfo).Lut_in[plane_idx] );
        (*pInfo).Lut_in[plane_idx] = NULL;
    }
    */

    //---------------- dump register to vector start ----------------//
    int out_idx = 0;
    //reg_struct *regList = (reg_struct *)pRegVector->pbuf + pRegVector->dat_num;
    demura_reg *regList = (demura_reg *)pRegVector->pbuf + pRegVector->dat_num;

    printf("dat_num = %d \n", pRegVector->dat_num);
#if 0
    memcpy(regList, &_reg, sizeof(registers));
    pRegVector->dat_num = (int)sizeof(_reg)/sizeof(registers);
#else
    regList[out_idx++] = (_reg.reg_dmc_black_limit);
    regList[out_idx++] = (_reg.reg_dmc_plane_level1);
    regList[out_idx++] = (_reg.reg_dmc_plane_level2);
    regList[out_idx++] = (_reg.reg_dmc_plane_level3);;
    regList[out_idx++] = (_reg.reg_dmc_plane_level4);
    regList[out_idx++] = (_reg.reg_dmc_plane_level5);
    regList[out_idx++] = (_reg.reg_dmc_plane_level6);
    regList[out_idx++] = (_reg.reg_dmc_plane_level7);
    regList[out_idx++] = (_reg.reg_dmc_plane_level8);
    regList[out_idx++] = (_reg.reg_dmc_white_limit);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag4);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag3);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag2);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag1);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag8);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag7);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag6);
    regList[out_idx++] = (_reg.reg_dmc_data_r_mag5);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag4);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag3);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag2);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag1);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag8);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag7);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag6);
    regList[out_idx++] = (_reg.reg_dmc_data_g_mag5);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag4);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag3);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag2);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag1);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag8);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag7);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag6);
    regList[out_idx++] = (_reg.reg_dmc_data_b_mag5);
    /*regList[out_idx++] = (_reg.reg_dmc_data_w_mag4);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag3.);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag2.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag1.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag8.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag7.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag6.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_mag5.Bit07_00);*/
    regList[out_idx++] = (_reg.reg_dmc_plane_b1_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_12_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_23_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_34_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_45_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_56_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_67_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_78_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_89_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_9w_coef);
    regList[out_idx++] = (_reg.reg_dmc_plane_9w_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_89_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_78_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_67_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_56_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_45_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_34_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_23_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_12_ks22);
    regList[out_idx++] = (_reg.reg_dmc_plane_b1_ks22);
    regList[out_idx++] = (_reg.reg_dmc_h_block);
    regList[out_idx++] = (_reg.reg_dmc_v_block);
    regList[out_idx++] = (_reg.reg_dmc_rgb_mode);
    regList[out_idx++] = (_reg.reg_dmc_block_size);
    regList[out_idx++] = (_reg.reg_dmc_plane_num);
    regList[out_idx++] = (_reg.reg_dmc_h_lut_num);
    regList[out_idx++] = (_reg.reg_dmc_h_lut_num);
    regList[out_idx++] = (_reg.reg_dmc_v_lut_num);
    regList[out_idx++] = (_reg.reg_dmc_v_lut_num);
    regList[out_idx++] = (_reg.reg_dmc_flow_ctrl_en);
    regList[out_idx++] = (_reg.reg_dmc_flow_len);
    regList[out_idx++] = (_reg.reg_dmc_flow_len);
    //regList[out_idx++] = (_reg.reg_dmc_grid_line_en);
    //regList[out_idx++] = (_reg.reg_dmc_grid_line_size);
    //regList[out_idx++] = (_reg.reg_dmc_grid_line_color);
    regList[out_idx++] = (_reg.reg_dmc_dither_rst_e_4_frame);
    regList[out_idx++] = (_reg.reg_dmc_dither_pse_rst_num);
    regList[out_idx++] = (_reg.reg_dmc_dither_en);
    //regList[out_idx++] = (_reg.reg_dmc_r_en.Bit07_00);
    //regList[out_idx++] = (_reg.reg_dmc_g_en.Bit07_00);
    //regList[out_idx++] = (_reg.reg_dmc_b_en.Bit07_00);
    //regList.push_back(_reg.reg_dmc_w_en.Bit07_00);
    //regList.push_back(_reg.reg_dmc_dga_en.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset1);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset1);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset2);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset2);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset3);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset3);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset4);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset4);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset5);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset5);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset6);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset6);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset7);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset7);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset8);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset8);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset9);
    regList[out_idx++] = (_reg.reg_dmc_data_r_offset9);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset1);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset1);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset2);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset3);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset4);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset5);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset6);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset7);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset8);
    regList[out_idx++] = (_reg.reg_dmc_data_g_offset9);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset1);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset2);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset3);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset4);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset5);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset6);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset7);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset8);
    regList[out_idx++] = (_reg.reg_dmc_data_b_offset9);
/*    regList[out_idx++] = (_reg.reg_dmc_data_w_offset1.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset1.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset2.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset2.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset3.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset3.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset4.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset4.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset5.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset5.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset6.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset6.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset7.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset7.Bit15_08);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset8.Bit07_00);
    regList[out_idx++] = (_reg.reg_dmc_data_w_offset8.Bit15_08);*/
    regList[out_idx++] = (_reg.reg_dmc_panel_h_size);
    //regList[out_idx++] = (_reg.reg_dmc_preload_line.Bit07_00);
    //regList[out_idx++] = (_reg.reg_dmc_preload_line.Bit15_08);
    //regList[out_idx++] = (_reg.reg_dmc_mono_mode_q_h_r.Bit07_00);
    //regList[out_idx++] = (_reg.reg_dmc_mono_mode_r_h_r.Bit07_00);

    pRegVector->dat_num += out_idx;
#endif
    //---------------- dump register to vector end   ----------------//

    return TRUE;
}


registers* demura_core_get_regs(void)
{
    return &_reg;
}

