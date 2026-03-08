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

#ifndef EMMC_DRIVER_H
#define EMMC_DRIVER_H

//=====================================================================================
#include <romtblo_impl.h>//CHIP ID

//=====================================================================================
#include "eMMC_config.h" // [CAUTION]: edit eMMC_config.h for your platform
//=====================================================================================
#include "eMMC_err_codes.h"

#define EMMC_DRIVER_VERSION   3 // used to sync with other SW stages (e.g. linux kernel)
//=====================================================================================

#define EMMC_FCIE_CLK_DIS()    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN)

//===========================================================
// debug macro
//===========================================================
extern U32 gu32_emmc_drv_ext_flag;

#define EMMCDRV_EXTFLAG_DISABLE_LOG   BIT0
#define EMMC_DISABLE_LOG(yes)    \
	if (1) {\
		if (yes)\
			gu32_emmc_drv_ext_flag |= EMMCDRV_EXTFLAG_DISABLE_LOG;\
		else \
			gu32_emmc_drv_ext_flag &= ~EMMCDRV_EXTFLAG_DISABLE_LOG;\
	}
#define EMMC_IF_DISABLE_LOG()    \
	(gu32_emmc_drv_ext_flag & EMMCDRV_EXTFLAG_DISABLE_LOG || EMMC_IF_TUNING_TTABLE())

#define EMMCDRV_EXTFLAG_BKG_SCAN      BIT1 // for Irwin
#define EMMC_CHK_BKG_SCAN_HS200()  (gu32_emmc_drv_ext_flag & EMMCDRV_EXTFLAG_BKG_SCAN)

#define EMMCDRV_EXTFLAG_DISABLE_RETRY   BIT2
#define EMMC_DISABLE_RETRY(yes)    \
	if (1) {\
		if (yes)\
			gu32_emmc_drv_ext_flag |= EMMCDRV_EXTFLAG_DISABLE_RETRY; \
		else\
			gu32_emmc_drv_ext_flag &= ~EMMCDRV_EXTFLAG_DISABLE_RETRY;\
	}
#define EMMC_IF_DISABLE_RETRY()    \
	((gu32_emmc_drv_ext_flag & EMMCDRV_EXTFLAG_DISABLE_RETRY) || EMMC_IF_TUNING_TTABLE())

#define EMMC_DRV_EXTFLAG_DISABLE_PAD_CHECK  BIT3
#define EMMC_DISABLE_PAD_CHECK(yes)    \
	if (1) { \
		if (yes) \
			gu32_emmc_drv_ext_flag |= EMMC_DRV_EXTFLAG_DISABLE_PAD_CHECK; \
		else \
			gu32_emmc_drv_ext_flag &= ~EMMC_DRV_EXTFLAG_DISABLE_PAD_CHECK;\
	}
#define EMMC_IF_DISABLE_PAD_CHECK()    \
	(gu32_emmc_drv_ext_flag & EMMC_DRV_EXTFLAG_DISABLE_PAD_CHECK)

#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
#define EMMC_DRV_EXTFLAG_SKIP_IDENTIFY_SAVE  BIT4
#define EMMC_SKIP_IDENTIFY_SAVE(yes)    \
	if (1) { \
		if (yes)\
			gu32_emmc_drv_ext_flag |= EMMC_DRV_EXTFLAG_SKIP_IDENTIFY_SAVE; \
		else \
			gu32_emmc_drv_ext_flag &= ~EMMC_DRV_EXTFLAG_SKIP_IDENTIFY_SAVE;\
	}
#define EMMC_IF_SKIP_IDENTIFY_SAVE()    \
	(gu32_emmc_drv_ext_flag & EMMC_DRV_EXTFLAG_SKIP_IDENTIFY_SAVE)
#endif

#define EMMC_DRV_EXT_INIT_FLAG    0
//===========================================================
// macro for Spec.
//===========================================================
#define ADDRESSING_MODE_BYTE      1 // 1 byte
#define ADDRESSING_MODE_SECTOR    2 // 512 bytes
#define ADDRESSING_MODE_4KB       3 // 4KB bytes

#define EMMC_DEVTYPE_HS400_1_8V   BIT6 // ECSD[196]
#define EMMC_DEVTYPE_HS200_1_8V   BIT4
#define EMMC_DEVTYPE_DDR          BIT2
#define EMMC_DEVTYPE_ALL          (EMMC_DEVTYPE_HS400_1_8V | EMMC_DEVTYPE_HS200_1_8V | EMMC_DEVTYPE_DDR)

#define eMMC_SPEED_OLD            0 // ECSD[185]
#define EMMC_SPEED_HIGH           1
#define EMMC_SPEED_HS200          2
#define EMMC_SPEED_HS400          3

#define EMMC_DRIVING_TYPE0	      0 // x1
#define EMMC_DRIVING_TYPE1	      1 // x1.5
#define EMMC_DRIVING_TYPE2	      2 // x0.75
#define EMMC_DRIVING_TYPE3	      3 // x0.5
#define EMMC_DRIVING_TYPE4	      4 // x1.2

#define EMMC_FLAG_TRIM            BIT0
#define EMMC_FLAG_HPI_CMD12       BIT1
#define EMMC_FLAG_HPI_CMD13       BIT2

#define EMMC_PWR_OFF_NOTIF_OFF      0
#define EMMC_PWR_OFF_NOTIF_ON       1
#define EMMC_PWR_OFF_NOTIF_SHORT    2
#define EMMC_PWR_OFF_NOTIF_LONG     3

//-------------------------------------------------------
// Devices has to be in 512B block length mode by default
// after power-on, or software reset.
//-------------------------------------------------------
#define EMMC_SECTOR_512BYTE       0x200
#define EMMC_SECTOR_512BYTE_BITS  9
#define EMMC_SECTOR_512BYTE_MASK  (EMMC_SECTOR_512BYTE - 1)

#define EMMC_SECTOR_BUF_16KB      (EMMC_SECTOR_512BYTE * 0x20)

#define EMMC_SECTOR_BYTECNT       EMMC_SECTOR_512BYTE
#define EMMC_SECTOR_BYTECNT_BITS  EMMC_SECTOR_512BYTE_BITS
//-------------------------------------------------------

#define EMMC_CMD_BYTE_CNT         5
#define EMMC_R1_BYTE_CNT          5
#define EMMC_R1B_BYTE_CNT         5
#define EMMC_R2_BYTE_CNT          16
#define EMMC_R3_BYTE_CNT          5
#define EMMC_R4_BYTE_CNT          5
#define EMMC_R5_BYTE_CNT          5
#define EMMC_MAX_RSP_BYTE_CNT     EMMC_R2_BYTE_CNT



//===========================================================
// DDR Timing Table
//===========================================================
struct emmc_fcie_ddr_tab_param {

	U8 u8_dqs, u8_cell;

} EMMC_PACK1;


struct emmc_fcie_ddr_tab_window {
	U8 u8_cnt;
	// DQS uses index, not reg value (see code)
	struct emmc_fcie_ddr_tab_param aparam[2];
	U8 au8_dqs_try_cell_cnt[8];
	U8 au8_dqs_valid_cell_cnt[8];

} EMMC_PACK1;


#define EMMC_TIMING_SET_MAX      0
// ----------------------------------------------
struct emmc_timing_table {

	U8 u8_set_cnt, u8_cur_set_idx;

	// ATOP (for  DDR52, HS200, HS400)
	struct emmc_fcie_atop_set set[1];

	U32 u32_chksum; // put in the last
	U32 u32_ver_no; // for auto update

} EMMC_PACK1;

struct emmc_hs400_timing_table {
	struct emmc_fcie_atop_set_skew4 set_skew4;
	U16 u16_rsp_valid_cnt;
	U8 u8_set_cnt;
	U8 au8_cid[EMMC_MAX_RSP_BYTE_CNT];
	U32 u32_device_driving;
	U32 u32_chksum; // put in the last
	U32 u32_ver_no; // for auto update
} EMMC_PACK1;
#define REG_OP_W    1
#define REG_OP_CLRBIT  2
#define REG_OP_SETBIT  3
#define RSG_OP_MASK    0xF
//total 10 bytes
struct emmc_fcie_reg_set {
	U32 u32_reg_address;//(BANK_ADDRESS + REGISTER OFFSET ) << 2
	U16 u16_reg_value;
	U16 u16_reg_mask;
	U16 u16_op_code;
} EMMC_PACK1;

struct emmc_gen_timing_table {
	U32 u32_chksum;
	U32 u32_ver_no;// for auto update
	U32 u32_clk;
	U8 u8_speed_mode;
	U8 u8_cur_set_idx;
	U8 u8_register_cnt;
	U8 u8_set_cnt;
	U8 au8_cid[EMMC_MAX_RSP_BYTE_CNT];
	U32 u32_device_driving;
	U32 u32_dummy[5];      //for extension
	struct emmc_fcie_reg_set reg_set[45]; //at most 45 register set
} EMMC_PACK1;

/*
 * Timing Table Parameter
 */

#if defined(CONFIG_EMMC_FORCE_DDR52)
#define EMMC_TIMING_TABLE_VERSION        2
#else
#define EMMC_TIMING_TABLE_VERSION        6 // for CL > 1047810
#endif
#define EMMC_M6E3_TIMING_TABLE_VERSION   8



#define PLL_SKEW4_CNT                   9
#define MIN_OK_SKEW_CNT                 7
#define MIN_OK_DS_CNT                   8
#define M6E3_MIN_OK_SKEW_CNT            5
#define TOTAL_PLL_SKEW4_CNT             18


#define START_BIT_DEFAULT 0xFFFF
#define END_BIT_DEFAULT   0xFFFF


//===========================================================
// life test
//===========================================================
struct emmc_fcie_life_test_bytes_cnt_t {
	U32 u32_life_time_test_fixed_gbytes;
	U32 u32_life_time_test_filled_gbytes;
	U32 u32_life_time_test_random_gbytes;
	U32 u32_chksum; // put in the last
} EMMC_PACK1;



//===========================================================
// Gernel Purpose Partition
//===========================================================
struct emmc_gp_part {
	U32 u32_part_size;
	U8 u8_en_attr;
	U8 u8_ext_attr;
	U8 u8_relw;
} EMMC_PACK1;

//===========================================================
// driver flag (u32_DrvFlag)
//===========================================================
#define DRV_FLAG_INIT_DONE       BIT0 // include eMMC identify done

#define DRV_FLAG_GET_PART_INFO   BIT1
#define DRV_FLAG_RSP_WAIT_D0H    BIT2 // currently only R1b

#define DRV_FLAG_DDR_MODE        BIT3
#define DRV_FLAG_TUNING_TTABLE   BIT4 // to avoid retry & heavy log
#define DRV_FLAG_SPEED_MASK      (BIT7 | BIT6 | BIT5)
#define DRV_FLAG_SPEED_HIGH      BIT5
#define DRV_FLAG_SPEED_HS200     BIT6
#define DRV_FLAG_SPEED_HS400     BIT7
#define DRV_FLAG_WEAK_STRENGTH   BIT8

#define EMMC_IF_NORMAL_SDR()   (0 == (emmc_drv.u32_drvflag & DRV_FLAG_DDR_MODE) &&\
	DRV_FLAG_SPEED_HIGH == (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_MASK))
#define EMMC_SPEED_MODE()      (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_MASK)

#define DRV_FLAG_PWR_OFF_NOTIF_LONG_MASK   (BIT8 | BIT9)
#define DRV_FLAG_PWR_OFF_NOTIF_OFF    0
#define DRV_FLAG_PWR_OFF_NOTIF_ON     BIT8
#define DRV_FLAG_PWR_OFF_NOTIF_SHORT  BIT9
#define DRV_FLAG_PWR_OFF_NOTIF_LONG   (BIT8 | BIT9)
#define DRV_FLAG_ERROR_RETRY        BIT11
#define DRV_FLAG_GEN_TTABLE         BIT12
#define DRV_FLAG_SPEED_HS400_DETECT BIT13

struct emmc_driver
{
	struct emmc_timing_table t_table;
	struct emmc_gen_timing_table t_table_g;
	struct emmc_hs400_timing_table t_hs400_table;
	int cur_skew4;


	U32 u32_chksum;
	// ----------------------------------------
    // FCIE
    // ----------------------------------------
	U16 u16_rca;
	U32 u32_drvflag, u32_last_err_code;
	U8  au8_rsp[EMMC_MAX_RSP_BYTE_CNT];
	U8  au8_csd[EMMC_MAX_RSP_BYTE_CNT];
	U8  au8_cid[EMMC_MAX_RSP_BYTE_CNT];
	U8  pad_type;
	U16 u16_mmc_mode;
	U32 u32_clk_khz;
	U16 u16_clk_reg_val;

	struct emmc_fcie_life_test_bytes_cnt_t life_bytes_cnt_t;
    // ----------------------------------------
    // eMMC
    // ----------------------------------------
	// CSD
	U8  u8_spec_vers;
	U8  u8_r_bl_len, u8_w_bl_len; // supported max blk len
	U16 u16_c_size;
	U8  u8_taac, u8_nsac, u8_tran_speed;
	U8  u8_c_size_mult;
	U8  u8_erase_grp_size, u8_erase_grp_mult;
	U8  u8_r2w_factor;

	U8  u8_if_sector_mode;
	U32 u32_emmc_flag;
	U32 u32_erase_unit_size;

	// ExtCSD
	U32 u32_sec_count;
	U32 u32_boot_sec_count;

	U8  u8_bus_width, u8_erased_mem_content;
	U32 u32_rpmb_size_sec_cnt;
	U16 u16_reliable_w_blk_cnt;
	U8  u8_ecsd185_hs_timing, u8_ecsd192_ver, u8_ecsd196_dev_type, u8_ecsd197_driver_strength;
	U8  u8_ecsd248_cmd6_to, u8_ecsd247_pwr_off_long_to, u8_ecsd34_pwr_off_ctrl;
	U8	u8_ecsd160_part_supfield, u8_ecsd224_hc_erase_grp_size, u8_ecsd221_hc_wp_grp_size;
	U8	u8_ecsd159_max_enh_size_2, u8_ecsd158_max_enh_size_1, u8_ecsd157_max_enh_size_0;
	U8	u8_u8_ecsd155_part_set_complete, u8_ecsd166_wr_rel_param;
	U8  u8_boot_size_mult;
	U8  u8_ecsd184_stroe_support;

    // ----------------------------------------


	// ----------------------
	struct emmc_gp_part	gp_part[4];

	U32 u32_en_user_start_addr;
	U32 u32_en_user_size;
	U8	u8_en_user_en_attr;
	U8	u8_en_user_relw;

	U8  u8_partition_config;
	U8  u8_emmc_pll_skew4;
	U8  u8_if_switch_hs200_to_high;
	U8  u8_cmd_log_en;
	U32 u32_fix_skew4_result;
	U32 chip_id;
	U32 revision;
	struct mtk_fcie_host *host;
};

/*
 * Device Driving Strength
 */
#define BITS_MSK_DRIVER_STRENGTH    0xF0
#define BITS_MSK_TIMING             0x0F
#define BITS_WEAK_DRIVER_STRENGTH   0x03

extern struct emmc_driver emmc_drv;
extern U8 au8_ext_csd[];

// ADMA Descriptor
struct  adma_descriptor {
	U32	u32_end     : 1;
	U32	u32_miu_sel : 2;
	U32             : 13;
	U32 u32_job_cnt : 16;
	U32 u32_address;
	U32 u32_dma_len;
	U32 u32_address2;
};

//===========================================================
// exposed APIs
//===========================================================
#include "drv_eMMC.h"

//===========================================================
// internal used functions
//===========================================================
#include "eMMC_hal.h"
#include "eMMC_RPMB.h"
#include "FDE_hal.h"

extern U8 *gau8_wbuf, *gau8_rbuf;

U32  emmc_ipverify_main(void);
U32  emmc_crazy_write_only(U16 *u16_pattern_test, U16 u16_pattern_cnt);
U32  emmc_crazy_single_write_only(U16 *u16_pattern_test, U16 u16_pattern_cnt);
U32  emmc_ipverify_main_big(void);
U32  emmc_ipverify_main_ex(U32 u32_data_pattern);
U32  emmc_ipverify_riu_main_ex(U32 u32_data_pattern);
#define eMMC_TEST_READONLY   1
#define eMMC_TEST_WRITEONLY  2
U32  emmc_ipverify_write_only(U16 u16_test_pattern);
U32  emmc_ipverify_read_only(void);
U32  emmc_ipverify_sdr_ddr_all_clk_temp(void);
void emmc_test_down_count(U32 u32_sec);
U32  emmc_ipverify_performance(void);
U32  emmc_test_blk_wrc_probe_timing(U32 u32_emmc_addr);
U32  emmc_stored_crazy_pattern(U32 u32_emmc_addr);
U32  emmc_read_crazy_pattern(U32 u32_emmc_addr);
U32  emmc_test_life_time(U8 u8_test_mode);
U32  emmc_test_pwr_cut_init_data(U8 *u8_data_buf, U32 u32_blk_start_addr);
U32  emmc_test_pwr_cut_test(U8 *u8_data_buf, U32 u32_blk_start_addr);
U32  emmc_test_read_disturbance(void);
U32  emmc_test_aes_simple_test(U32 u32_data_pattern);
U32  emmc_ipverify_performance_fde(void);
U32  emmc_test_single_blk_wrc_riu(U32 u32_emmc_addr, U32 u32_data_pattern);
U32  emmc_test_single_blk_wrc_miu(U32 u32_emmc_addr, U32 u32_data_pattern);
U32  emmc_test_single_blk_wrc_miu_ex(U32 u32_emmc_addr, U8 *pu8_w, U8 *pu8_r);
U32  emmc_test_sg_wrc_miu(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern);
U32  emmc_test_multi_blk_wrc_miu(U32 u32_emmc_addr, U16 u16_blk_cnt, U32 u32_data_pattern);
U32  emmc_ipverify_main_ex(U32 u32_data_pattern);
U32  emmc_ipverify_main_sg_ex(U32 u32_data_pattern);
U32  emmc_ipverify_main_api_ex(U32 u32_data_pattern);
void emmc_test_down_count(U32 u32_sec);
//power cut
void emmc_check_power_cut(void);
#if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
void emmc_prepare_power_saving_mode_queue(void);
#endif

void emmc_lock_fcie(U8 *pu8_str);
void emmc_unlock_fcie(U8 *pu8_str);

#define EMMC_LIFETIME_TEST_FIXED      1
#define EMMC_LIFETIME_TEST_FILLED     2
#define EMMC_LIFETIME_TEST_RANDOM     3
#define EMMC_LIFETIME_TEST_RANDOM_BIG 4

#define EMMC_RESERVED_BLK_FOR_MAP 4096//0~2Mbytes
#define EMMC_UBOOT_SIZE       6144//2~5Mbytes
#define EMMC_MPOOL_SIZE       4096//5~7Mbytes

#define UPGRADE_BUFFER_ADDR 0x30200000
#define PWR_CUT_TEST_START_BLK  0x700000

#define MAX_LOOP_CNT 0xFFFFFFFF
#define UNLIMIT_LOOP_CNT 0x3697

/*
 * EXT_CSD fields
 */
#define EXT_CSD_GP_SIZE_MULT_1_0        143
#define EXT_CSD_GP_SIZE_MULT_1_1        144
#define EXT_CSD_GP_SIZE_MULT_1_2        145

#define EXT_CSD_GP_SIZE_MULT_2_0        146
#define EXT_CSD_GP_SIZE_MULT_2_1        147
#define EXT_CSD_GP_SIZE_MULT_2_2        148

#define EXT_CSD_GP_SIZE_MULT_3_0        149
#define EXT_CSD_GP_SIZE_MULT_3_1        150
#define EXT_CSD_GP_SIZE_MULT_3_2        151

#define EXT_CSD_GP_SIZE_MULT_4_0        152
#define EXT_CSD_GP_SIZE_MULT_4_1        153
#define EXT_CSD_GP_SIZE_MULT_4_2        154

#define EXT_CSD_MAX_ENH_SIZE_MULT_1     158
#define EXT_CSD_MAX_ENH_SIZE_MULT_2     159
#define EXT_CSD_SANITIZE                165
#define EXT_CSD_ERASEED_MEM_CONTENT     181
#define EXT_CSD_DRV_STRENGTH            197

#define EXT_CSD_SEC_CNT_7_15			213
#define EXT_CSD_SEC_CNT_16_23			214
#define EXT_CSD_SEC_CNT_24_31			215

#define EXT_CSD_HC_ERASE_GRP_SIZE       224
#define EXT_CSD_REL_WR_SEC_C			222
#define EXT_CSD_HC_WP_GRP_SIZE          221

#define EXT_CSD_POWER_OFF_LONG_TIME     247
#define EXT_CSD_GENERIC_CMD6_TIME       248

#define EXT_CSD_PRE_EOL_INFO			267
#define EXT_CSD_LIFE_TIME_TYPA			268
#define EXT_CSD_LIFE_TIME_TYPB			269

#define EXT_CSD_HPI_FEATURE			    503

U32 emmc_boot_mode(U32 *pu32_dddr, U32 u32_byte_cnt);
void emmc_fcie_get_cmd_fifo(U16 u16_word_pos, U16 u16_word_cnt, U16 *pu16_buf);
U32 emmc_wait_cifd_event(U16 u16_wait_event, U32 u32_micro_sec);
U32 emmc_wait_set_cifd(U8 *pu8_data_buf, U32 u32_byte_cnt);
U32 emmc_wait_get_cifd(U8 *pu8_data_buf, U32 u32_byte_cnt);
//U32  emmc_check_align_pack(U8 u8_align_byte_cnt);
void  emmc_dump_mem(unsigned char *buf, U32 cnt);
void  emmc_dump_mem_32(U32 *buf, U32 cnt);
U32  emmc_compare_data(U8 *pu8_buf0, U8 *pu8_buf1, U32 u32_byte_cnt);
U32 emmc_chk_sum(U8 *pu8_data, U32 u32_byte_cnt);
U32  emmc_print_device_info(void);
U32  emmc_compare_cis_tag(U8 *tag);

#endif // EMMC_DRIVER_H
