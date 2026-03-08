// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef __EMMC_HAL_H__
#define __EMMC_HAL_H__

#include "eMMC.h"

#define U32BEND2LEND(X) ( ((X&0x000000FF)<<24) + ((X&0x0000FF00)<<8) + ((X&0x00FF0000)>>8) + ((X&0xFF000000)>>24) )
#define U16BEND2LEND(X) ( ((X&0x00FF)<<8) + ((X&0xFF00)>>8) )

#define _START_TXMIT                0x40 // transmission bit

#define eMMC_GO_IDLE_STATE		(_START_TXMIT+0)
#define eMMC_SEND_OP_COND		(_START_TXMIT+1)
#define eMMC_ALL_SEND_CID		(_START_TXMIT+2)
#define eMMC_SET_RLT_ADDR		(_START_TXMIT+3)
#define eMMC_SEL_DESEL_CARD		(_START_TXMIT+7)
#define eMMC_SEND_EXT_CSD		(_START_TXMIT+8)
#define eMMC_SEND_CSD			(_START_TXMIT+9)
#define eMMC_SWITCH				(_START_TXMIT+6)
#define eMMC_ERASE_GROUP_S		(_START_TXMIT+35)
#define eMMC_ERASE_GROUP_E		(_START_TXMIT+36)
#define eMMC_ERASE				(_START_TXMIT+38)
#define eMMC_SEND_STATUS		(_START_TXMIT+13)
#define eMMC_R_SINGLE_BLOCK		(_START_TXMIT+17)
#define eMMC_R_MULTIP_BLOCK		(_START_TXMIT+18)
#define eMMC_STOP_TRANSMIT		(_START_TXMIT+12)
#define eMMC_W_SINGLE_BLOCK		(_START_TXMIT+24)
#define eMMC_W_MULTIP_BLOCK		(_START_TXMIT+25)
#define eMMC_SEND_TUNING_BLK	(_START_TXMIT+21)


#define EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT       5
#define EMMC_CMD_API_ERR_RETRY_CNT            5
#define EMMC_SDR_INIT_RETRY_CNT               0

U32 emmc_fcie_wait_events(unsigned long ulong_reg_addr, U16 u16_events, U32 u32_micro_sec);
U32 emmc_fcie_polling_events(unsigned long ulong_reg_addr, U16 u16_events, U32 u32_micro_sec);
void emmc_fcie_dump_debug_bus(void);
void emmc_fcie_dump_registers(void);
void emmc_fcie_check_reset_done(void);
U32 emmc_fcie_reset(void);
U32 emmc_fcie_init(void);
void emmc_fcie_err_handler_stop(void);
U32 emmc_fcie_err_handler_retry(void);
void emmc_fcie_err_handler_restore_clk(void);
void emmc_fcie_err_handler_reinit(void);
U32 emmc_fcie_err_handler_reinit_ex(void);
U32 emmc_fcie_send_cmd(U16 u16_mode, U16 u16_ctrl, U32 u32_arg, U8 u8_cmd_idx, U8 u8_rsp_byte_cnt);
void emmc_fcie_clear_events(void);
void emmc_fcie_clear_events_reg0(void);
U32 emmc_fcie_wait_d0_high_ex(U32 u32_us);
U32 emmc_fcie_wait_d0_high(U32 u32_us);
U8 emmc_fcie_cmd_rsp_buf_get(U8 u8addr);


U32 emmc_fcie_choose_speed_mode(void);
void emmc_fcie_apply_timing_set(U8 u8_idx);
void emmc_fcie_set_atop_timing_reg(U8 u8_set_idx);
void emmc_fcie_apply_reg(U8 u8_set_idx);
U32 emmc_fcie_enable_fastmode_ex(U8 pad_type);
U32 emmc_fcie_enable_fast_mode(U8 pad_type);
U32 emmc_fcie_enable_sdr_mode(void);
U32 emmc_fcie_detect_ddr_timing(void);
void emmc_dump_timing_table(void);
U32 emmc_load_timing_table(U8 pad_type);
void emmc_fcie_set_skew4_rsp_reg(void);
void emmc_fcie_set_skew4_data_reg(void);
void emmc_fcie_set_tx_reg(void);
void emmc_skew4_rsp_reg(void);
void emmc_skew4_read_data_reg(U16 *u16_status_err_flag);
void emmc_skew4_write_data_reg(void);
void emmc_skew4_write_data_restored_reg(U16 u16_skew, U8 u8_reg2ch);
void emmc_skew4_write_data_stored_original_reg(U16 *u16_skew, U8 *u8_reg2ch);
#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
void emmc_keep_rsp(U8 *pu8_one_rsp_buf, U8 u8_cmd_idx);
U32  emmc_return_rsp(U8 *pu8_one_rsp_buf, U8 u8_cmd_idx);
U32  emmc_save_context_rsp(void);
U32  emmc_load_context_rsp(void);
U32 emmc_fcie_check_fast_mode(void);
U32 emmc_fcie_check_cid(void);
#endif


//----------------------------------------
U32 emmc_identify(void);
U32 emmc_cmd0(U32 u32_arg);
U32 emmc_rom_boot_cmd0(U32 u32_arg, U32 u32_addr, U16 u16_blk_cnt);
U32 emmc_cmd1(void);
U32 emmc_cmd2(void);
U32 emmc_cmd3_cmd7(U16 u16_RCA, U8 u8_cmd_idx);
U32 emmc_cmd9(U16 u16_rca);
U32 emmc_csd_config(void);
U32  emmc_ext_csd_init(void);
U32 emmc_ext_csd_config(void);
U32 emmc_cmd8(U8 *pu8_data_buf);
U32 emmc_cmd8_miu(U8 *pu8_data_buf);
U32 emmc_cmd8_cifd(U8 *pu8_data_buf);
U32 emmc_set_pwr_off_notification(U8 u8_set_ecsd_pon);
U32 emmc_set_bus_speed(U8 u8_bus_speed);
U32 emmc_sanitize(U8 u8_sanitize_val);
U32 emmc_set_driving_strength(U8 u8_driving);
U32 emmc_set_bus_width(U8 u8_bus_width, U8 u8_if_ddr);
U32 emmc_modify_ext_csd(U8 u8_access_mode, U8 u8_byte_idx, U8 u8_value);
U32 emmc_cmd6(U32 u32_arg);
U32 emmc_erase_cmd_seq(U32 u32_emmc_blk_addr_start, U32 u32_emmc_blk_addr_end);
U32 emmc_cmd35_cmd36(U32 u32_emmc_blk_addr, U8 u8_cmd_idx);
U32 emmc_cmd38(void);
U32 emmc_dump_emmc_status(void);
U32 emmc_cmd13(U16 u16_rca);
U32 emmc_cmd16(U32 u32_blk_length);
U32 emmc_cmd17(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd17_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd17_cifd(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd24(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd24_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd24_cifd(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd12_no_check(U16 u16_rca);
U32 emmc_cmd12(U16 u16_rca);
U32 emmc_cmd18(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd18_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd18_adma(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 len);
U32 emmc_cmd23(U16 u16_blk_cnt);
U32 emmc_cmd25(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd25_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd25_adma(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U32 len);
U32 emmc_cmd21(void);
U32 emmc_get_r1(void);
U32 emmc_check_r1_error(void);
U32 emmc_cmd10(U16 u16_rca, U8 *pu8_res_buf);
U32 emmc_cmd6_skew4(U32 u32_arg);
U32 emmc_cmd17_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd24_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf);
U32 emmc_cmd23_skew4(U16 u16_blk_cnt);
U32 emmc_cmd18_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd25_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt);
U32 emmc_cmd13_skew4(U16 u16_rca);

//----------------------------------------
void emmc_dump_atop_table(void);
void emmc_fcie_symme_try_skew4(void);
#endif // __EMMC_HAL_H__
