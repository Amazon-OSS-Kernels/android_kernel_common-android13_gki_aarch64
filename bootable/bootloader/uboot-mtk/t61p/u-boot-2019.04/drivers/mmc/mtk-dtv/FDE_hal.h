/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef __FDE_HAL_H__
#define __FDE_HAL_H__

#include "FDE_reg.h"

void fde_aes_sw_reset(void);
void fde_aes_set_key(U8 *pu8_key, U32 u32_key_size);
void fde_set_set_iv(U8 *pu8_iv);
void fde_aes_set_ctr(U8 *pu8_ctr);
void fde_aes_set_tcalc_key(U8 *pu8_tkey);
int fde_aes_set_mode(U8 u8_mode);
int fde_aes_set_codec_dir(U8 u8_dir);
void fde_aes_set_xts_lba(U32 u32_lba);
void fde_aes_set_xex_hw_t_calc_kick(int val);
int fde_aes_init(void);
void fde_aes_open(void);
void fde_aes_close(void);
void FDE_AES_CQE_Open(void);
void FDE_AES_CQE_Close(void);
void fde_aes_func_open(U8 u8_dir, U32 lba);
void fde_aes_func_close(void);
int fde_aes_switch_state(void);


#endif // __eMMC_HAL_H__
