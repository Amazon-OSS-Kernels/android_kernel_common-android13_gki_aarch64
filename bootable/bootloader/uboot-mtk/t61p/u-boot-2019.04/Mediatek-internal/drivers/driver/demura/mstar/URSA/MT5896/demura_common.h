// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DEMURA_COMMON_H_
#define _DEMURA_COMMON_H_

#define DEMURA_CALC_H_NODES(lut, den) (((lut) + (den - 1)) / ((den)? (den) : 1))

typedef enum
{
    E_DEMURA_VERSION_S11,   //M6L, M6LE3
    E_DEMURA_VERSION_S7,    // else
    E_DEMURA_VERSION_S7_MONO,   //MOKA
    E_DEMURA_VERSION_MAX
} E_DEMURA_VERSION;

// Function declare
int demura_layer_distance_div(int pre, int nxt);
int register_shift(int mask, int value);
void part_bit15_00(int input_value, int total_bits, int *Bit15_08, int *Bit07_00);
void part_bit23_00(int input_value, int total_bits, int *Bit23_16, int *Bit15_08, int *Bit07_00);
void demura_table_normalization(int lut_size, double *lut_in, int *lut_out,int *layer_gain_out);
void demura_table_normalization1(int lut_size, double *lut_in, int *lut_out, int *layer_gain_out);
void demura_table_normalization_ROI(int lut_h_size, int lut_v_size, int ROI_h_start_offset, int ROI_h_end_offset, int ROI_v_start_offset, int ROI_v_end_offset, double *lut_in, int *lut_out, int *layer_gain_out);
void demura_table_normalization_inv(int lut_size, int layer_gain,int *lut_in,double *lut_out);
void coeff_and_ks(int diff, int *coeff, int *ks);
void FIND_MIN_AND_MAX(int RGB_channel_idx, int block_width, int block_height, strgb_structInfo *layer_blk_img, double *rgb_max, double *rgb_min);
void LUT_SETTING_SUBFUNCTION(int RGB_idx, int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg, strgb_structInfo *layer_blk_img, strgb_structInfo *layer_blk_img_out);
void LUT_AND_SETTING_PROCESSOR(int plane_idx, int block_width, int block_height, int reg_dmc_plane_level, registers *preg , strgb_structInfo *layer_blk_img_in, strgb_structInfo *layer_blk_img_out, int reg_dmc_rgb_mode);
void write_demuraLUT_to_HW_DRAM_FORMAT(stlayer_info_structInfo *layer_info, strgb_structInfo *layer_blk_img[8], m_vector *pLut_out_vector);

int GETBITS( const int Value , const int MSB_bit, const int LSB_bit);
int register_08bits_inv( _08bits reg_tmp, int total_bits);
int register_16bits_inv( _16bits reg_tmp, int MSB_total_bits, int LSB_total_bits);
int twos_complement_to_integer(const int input_value, const int total_bits);
double DMC_LUT_Fix_To_Double(const int Layer_offset_2s, const int Layer_gain, const int Lut_value);


MS_U8 demura_get_version(void);
void demura_store_version(int version);

// extern function
extern MS_BOOL demura_core_ver10(int base_address, int lut_h_size, int lut_v_size, interface_info *pInfo, m_vector *pRegVector, m_vector *Lut_out);
extern registers* demura_core_get_regs(void);
#endif
