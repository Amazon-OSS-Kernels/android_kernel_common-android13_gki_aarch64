// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include "eMMC.h"

#ifdef IP_FCIE_VERSION_5

#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

//========================================================
// HAL pre-processors
//========================================================
#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
	#define EMMC_KEEP_RSP(pu8_one_rsp_buf, u8_cmd_idx) \
		if (EMMC_IF_SKIP_IDENTIFY_SAVE()) \
			emmc_keep_rsp(pu8_one_rsp_buf, u8_cmd_idx)
#else
	#define EMMC_KEEP_RSP(pu8_one_rsp_buf, u8_cmd_idx)// NULL to save CPU a JMP/RET time
#endif


#define EMMC_CMD1_RETRY_CNT    0x8000


//========================================================
// HAL APIs
//========================================================
U32 emmc_fcie_wait_events(unsigned long ulong_reg_addr, U16 u16_events, U32 u32_micro_sec)
{
	//emmc_debug(0, 0, "\033[32m%s\033[m\n", __FUNCTION__);
	return emmc_fcie_polling_events(ulong_reg_addr, u16_events, u32_micro_sec);
}

U32 emmc_fcie_polling_events(unsigned long ulong_reg_addr, U16 u16_events, U32 u32_micro_sec)
{
	U32 u32_i, u32_delay_x;
	U16 u16_val = 0;

	if (u32_micro_sec > HW_TIMER_DELAY_100us) {
		u32_delay_x = HW_TIMER_DELAY_100us / HW_TIMER_DELAY_1us;
		u32_micro_sec /= u32_delay_x;
	} else {
		u32_delay_x = 1;
	}
	for (u32_i = 0; u32_i < u32_micro_sec; u32_i++) {
		emmc_check_power_cut();
		emmc_hw_timer_delay(HW_TIMER_DELAY_1us * u32_delay_x);
		REG_FCIE_R(ulong_reg_addr, u16_val);

		if (ulong_reg_addr == FCIE_MIE_EVENT && ((u16_val & BIT_ERR_STS) ==
		    BIT_ERR_STS)) {
			#if 1
			REG_FCIE_R(FCIE_SD_STATUS, u16_val);
			#else
			do {
				REG_FCIE_R(FCIE_SD_STATUS, u16_val);
				++u32_count;
			} while ((u16_val & 0x0040) == 0x0040);
			#endif

			if (u16_val & BIT_DAT_RD_CERR) {
				emmc_debug(0, 0, "\033[7;35mData read CRC error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_DAT_WR_CERR) {
				emmc_debug(0, 0, "\033[7;35mData write CRC error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_DAT_WR_TOUT) {
				emmc_debug(0, 0, "\033[7;35mData write timeout error\033[m\n");
				//emmc_debug(0, 0, "u32_count=%d\n", u32_count);
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_CMD_NO_RSP) {
				emmc_debug(0, 0, "\033[7;35mCommand no response error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_CMD_RSP_CERR) {
				emmc_debug(0, 0, "\033[7;35mCommand response CRC error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_DAT_RD_TOUT) {
				emmc_debug(0, 0, "\033[7;35mData read timeout error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			} else if (u16_val & BIT_SD_CARD_BUSY) {
				emmc_debug(0, 0, "\033[7;35meMMC busy error\033[m\n");
				return EMMC_ST_ERR_DATA_MISMATCH;
			}
			emmc_debug(0, 0, "Unknown type error!!!\n");
			emmc_debug(0, 0, "Ask Brian for debugging......!!!\n");
			emmc_dump_pad_clk();
			emmc_fcie_dump_registers();
		}

		if (u16_events == (u16_val & u16_events))
			if ((REG_FCIE(FCIE_SD_CTRL) & BIT_ERR_DET_ON) == 0)
				break;
	}

	if (u32_i == u32_micro_sec) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: %u us, Reg.%04lXh: %04Xh, but wanted: %04Xh\n",
			   u32_micro_sec * u32_delay_x,
			   (ulong_reg_addr - (U32)FCIE_MIE_EVENT) >> REG_OFFSET_SHIFT_BITS,
			   u16_val, u16_events);

		return EMMC_ST_ERR_TIMEOUT_WAIT_REG0;
	}

	return EMMC_ST_SUCCESS;
}


static U32 gu32_fcie5_debug_bus[4];

static char *gstr_debug_mode[] = {"CMD state",
                                "FIFO status",
                                "ADMA Address",
                                "ADMA blk cnt"};


void emmc_fcie_dump_debug_bus(void)
{
	U32 u32_reg;
	U16 u16_i;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "\n");

	REG_FCIE_CLRBIT(FCIE_EMMC_DEBUG_BUS1, BIT_DEBUG1_MODE_MSK);
	REG_FCIE_SETBIT(FCIE_EMMC_DEBUG_BUS1, BIT_DEBUG1_MODE_SET);

	for (u16_i = 0; u16_i < 4; u16_i++) {
		REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_DEBUG_MODE_MASK);
		REG_FCIE_SETBIT(FCIE_TEST_MODE, (u16_i + 1) << BIT_DEBUG_MODE_SHIFT);

		emmc_debug(1, 0, "0x15 = %Xh, ", REG_FCIE(FCIE_TEST_MODE));
		REG_FCIE_R(FCIE_EMMC_DEBUG_BUS0, u32_reg);
		emmc_debug(1, 0, "0x38 = %Xh, ", u32_reg);
		gu32_fcie5_debug_bus[u16_i] = u32_reg;

		REG_FCIE_R(FCIE_EMMC_DEBUG_BUS1, u32_reg);
		emmc_debug(1, 0, "0x39 = %Xh\n", u32_reg);
		gu32_fcie5_debug_bus[u16_i] |= (u32_reg & 0x00FF) << 16;
	}

	for (u16_i = 0; u16_i < 4; u16_i++)
		emmc_debug(1, 0, "%s:\t %06Xh\n", gstr_debug_mode[u16_i],
			   gu32_fcie5_debug_bus[u16_i]);

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
}


//static U16 sgau16_eMMCDebugReg[100];


void emmc_fcie_dump_register_bank(unsigned long ulong_bank, U16 u16_word_count)
{
	U16 u16_i;
	U16 u16_reg;

	for (u16_i = 0 ; u16_i < u16_word_count; u16_i++) {
		if (u16_i % 8 == 0)
			emmc_debug(1, 0, "\n%02Xh:| ", u16_i);

			REG_FCIE_R(GET_REG_ADDR(ulong_bank, u16_i), u16_reg);
			emmc_debug(1, 0, "%04Xh ", u16_reg);
	}
	emmc_debug(1, 0, "\n");
}

void emmc_fcie_dump_registers(void)
{
	emmc_debug(1, 0, "\nfcie reg:");
	emmc_fcie_dump_register_bank(FCIE_REG_BASE_ADDR, 0x40);

	emmc_debug(1, 0, "\nemmc_pll reg:");
	emmc_fcie_dump_register_bank(EMMC_PLL_BASE, 0x80);
	emmc_debug(1, 0, "\n");
}

void emmc_fcie_err_handler_stop(void)
{
	if (EMMC_IF_DISABLE_RETRY() == 0) {
		emmc_dump_driver_status();
		emmc_dump_pad_clk();
		emmc_fcie_dump_registers();
		emmc_fcie_dump_debug_bus();
		emmc_dump_emmc_status();
		emmc_die("\n");
	} else {
		emmc_fcie_init();
	}
}

void emmc_fcie_symme_try_skew4(void)
{
	emmc_drv.t_table_g.u8_cur_set_idx = 4 - emmc_drv.t_table_g.u8_cur_set_idx;
	emmc_fcie_apply_timing_set(emmc_drv.t_table_g.u8_cur_set_idx);
}

#define MAX_SDR_RETRY_CNT  2
static U8 sgu8_if_need_restore_pad_type = 0xFF;

U32 emmc_fcie_err_handler_retry(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	static U8 u8_sdr_retry_count = EMMC_SDR_INIT_RETRY_CNT;

	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

// -------------------------------------------------
	if ((emmc_drv.u32_drvflag & DRV_FLAG_INIT_DONE) == 0)
		return EMMC_ST_SUCCESS;

// -------------------------------------------------
	if (u8_sdr_retry_count < MAX_SDR_RETRY_CNT &&
	    emmc_drv.pad_type == FCIE_EMMC_HS400 &&
	    emmc_drv.u8_emmc_pll_skew4 == 0 &&
	    emmc_drv.t_table_g.u8_set_cnt) {
	    emmc_fcie_symme_try_skew4();
		++u8_sdr_retry_count;
	} else {
		if (!EMMC_IF_NORMAL_SDR()) {
			sgu8_if_need_restore_pad_type = emmc_drv.pad_type;
			u32_err = emmc_fcie_enable_sdr_mode();
			if (u32_err != EMMC_ST_SUCCESS)
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: EnableSDRModefail: %Xh\n", u32_err);

			u8_sdr_retry_count = 0;
		}
	}

	emmc_debug(EMMC_DEBUG_LEVEL_WARNING, 0,
		   "eMMC Warn: slow clk to %u.%uMHz, %Xh\n",
		   emmc_drv.u32_clk_khz / EMMC_KHZ, (emmc_drv.u32_clk_khz % EMMC_KHZ) / 100,
		   emmc_drv.u16_clk_reg_val);

	return u32_err;
}

void emmc_fcie_err_handler_restore_clk(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	if (sgu8_if_need_restore_pad_type != 0xFF) {
		u32_err = emmc_fcie_enable_fastmode_ex(sgu8_if_need_restore_pad_type);
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: EnableFastMode_Ex fail, %Xh\n", u32_err);
			return;
		}
		sgu8_if_need_restore_pad_type = 0xFF;
	}
}

U32 emmc_fcie_err_handler_reinit_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	char *str_mmc_partition[8] = {
				"eMMC: enable R/W user partition success",
				"eMMC: enable R/W boot1 partition success",
				"eMMC: enable R/W boot2 partition success",
				"eMMC: enable R/W RPMB success",
				"eMMC: enable Gernel Purpose partition success",
				"eMMC: enable Gerne2 Purpose partition success",
				"eMMC: enable Gerne3 Purpose partition success",
				"eMMC: enable Gerne4 Purpose partition success"};

	char *str_err_mmc_partition[8] = {
				"eMMC Err: enable R/W user partition fail",
				"eMMC Err: enable R/W boot1 partition fail",
				"eMMC Err: enable R/W boot2 partition fail",
				"eMMC Err: enable R/W RPMB fail",
				"eMMC Err: enable Gernel Purpose partition fail",
				"eMMC Err: enable Gerne2 Purpose partition fail",
				"eMMC Err: enable Gerne3 Purpose partition fail",
				"eMMC Err: enable Gerne4 Purpose partition fail"};

	u32_err = emmc_fcie_init();
	if (u32_err) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: FCIE_Init fail, %Xh\n", u32_err);
		return u32_err;
	}

	emmc_drv.u32_drvflag = 0;
	emmc_platform_init();
	u32_err = emmc_identify();
	if (u32_err) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Identify fail, %Xh\n", u32_err);
		return u32_err;
	}

	emmc_clock_setting(FCIE_SLOW_CLK);

	u32_err = emmc_cmd3_cmd7(emmc_drv.u16_rca, MMC_CMD_SELECT_CARD);
	if (u32_err) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD7 fail, %Xh\n", u32_err);
		return u32_err;
	}

	u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: enable High speed fail: %Xh\n", u32_err);
		return u32_err;
	}

	if (emmc_drv.u8_partition_config & PART_ACCESS_MASK) {
		u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE,
					      EXT_CSD_PART_CONF, emmc_drv.u8_partition_config);

		if (u32_err)
			emmc_debug(0, 0, "%s, %Xh\n",
				   str_err_mmc_partition[(U8)(emmc_drv.u8_partition_config &
				   PART_ACCESS_MASK)], u32_err);
		else
			emmc_debug(0, 0, "%s\n",
				   str_mmc_partition[(U8)(emmc_drv.u8_partition_config &
				   PART_ACCESS_MASK)]);
	}

	return u32_err;
}

void emmc_fcie_err_handler_reinit(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_drvflag = emmc_drv.u32_drvflag;
	U16 u16_reg10 = emmc_drv.u16_mmc_mode;
	U8  u8_ori_pad_type = emmc_drv.pad_type;

	u32_err = emmc_fcie_err_handler_reinit_ex();
	if (u32_err)
		goto LABEL_REINIT_END;

// ---------------------------------
	emmc_drv.u32_drvflag = u32_drvflag;
	if (EMMC_IF_NORMAL_SDR()) {
		u32_err = emmc_fcie_enable_sdr_mode();
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EnableSDRMode fail, %Xh\n", u32_err);
			goto LABEL_REINIT_END;
		}
	} else {
		u32_err = emmc_fcie_enable_fastmode_ex(u8_ori_pad_type);
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EnableFastMode_Ex fail, %Xh\n", u32_err);
			goto LABEL_REINIT_END;
		}
	}
	emmc_drv.u16_mmc_mode = u16_reg10;
	REG_FCIE_W(FCIE_SD_MODE, emmc_drv.u16_mmc_mode);

LABEL_REINIT_END:
	if (u32_err)
		emmc_die("\n");
}

U32 emmc_fcie_reset(void)
{
	U16 u16_clk = emmc_drv.u16_clk_reg_val;
	U16 u16_cnt;

	REG_FCIE_W(FCIE_RESERVED_FOR_SW, EMMC_DRIVER_VERSION);
	//emmc_debug(1, 1, "1: %Xh\n", REG_FCIE(FCIE_RESERVED_FOR_SW));

	REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_JOB_START);//clear for safe
	emmc_clock_setting(gau8_fcie_clk_sel[0]);//speed up FCIE reset done

	REG_FCIE_CLRBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n);/* active low */
	u16_cnt = 0;
	while (1) {
		if ((REG_FCIE(FCIE_RST) & BIT_RST_STS_MASK) == BIT_RST_STS_MASK)//reset success
			break;
		emmc_hw_timer_delay(HW_TIMER_DELAY_1us);
		if (u16_cnt++ >= 1000) {
			emmc_debug(1, 0, "eMMC Err: FCIE reset fail!\n");
			break;
		}
	}

	REG_FCIE_SETBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n);
	u16_cnt = 0;
	while (1) {
		if ((REG_FCIE(FCIE_RST) & BIT_RST_STS_MASK) == 0)//reset success
			break;
		emmc_hw_timer_delay(HW_TIMER_DELAY_1us);
		if (u16_cnt++ >= 1000) {
			emmc_debug(1, 0, "eMMC Err: FCIE reset fail2!\n");
			break;
		}
	}

	emmc_clock_setting(u16_clk);
	//emmc_debug(1, 1, "2: %Xh\n", REG_FCIE(FCIE_RESERVED_FOR_SW));

	return 0;
}

U32 emmc_fcie_init(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_platform_reset_pre();

	//#if 0
	//U16 u16_i, u16_reg;
	//check timer clock
	//emmc_debug(EMMC_DEBUG_LEVEL, 1, "Timer test, for 6 sec: ");
	//for (u16_i = 6; u16_i > 0; u16_i--) {
		//emmc_debug(EMMC_DEBUG_LEVEL, 1, "%u ", u16_i);
		//emmc_hw_timer_delay(HW_TIMER_DELAY_1s);
	//}
	//emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");

	//check FCIE reg.30h
	//REG_FCIE_R(FCIE_TEST_MODE, u16_reg);
	//if (0) {
		//emmc_debug(0, 1, "eMMC Err: Reg0x30h BIST fail: %04Xh \r\n",u16_reg);
		//return EMMC_ST_ERR_BIST_FAIL;
	//}
	//if (u16_reg & BIT_FCIE_DEBUG_MODE_MASK) {
		//emmc_debug(0, 1, "eMMC Err: Reg0x30h Debug Mode: %04Xh \r\n", u16_reg);
		//return EMMC_ST_ERR_DEBUG_MODE;
	//}

	//u32_err = emmc_fcie_reset();
	//if (u32_err != EMMC_ST_SUCCESS){
		//emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: reset fail\n");
		//emmc_fcie_err_handler_stop();
		//return u32_err;
	//}
	//#endif

	REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN);

// ------------------------------------------
	u32_err = emmc_fcie_reset();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: reset fail: %Xh\n", u32_err);
		emmc_fcie_err_handler_stop();
		return u32_err;
	}

	REG_FCIE_W(FCIE_MIE_INT_EN, 0);
	REG_FCIE_W(FCIE_MIE_FUNC_CTL, BIT_EMMC_EN);
	REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_CMD_SIZE_MASK);
	REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, (EMMC_CMD_BYTE_CNT) << BIT_CMD_SIZE_SHIFT);
	REG_FCIE_W(FCIE_SD_CTRL, 0);
	REG_FCIE_W(FCIE_SD_MODE, emmc_drv.u16_mmc_mode);
	//default sector size: 0x200
	REG_FCIE_W(FCIE_BLK_SIZE,  EMMC_SECTOR_512BYTE);
	REG_FCIE_W(FCIE_RSP_SHIFT_CNT, 0);
	REG_FCIE_W(FCIE_RX_SHIFT_CNT, 0);
	REG_FCIE_W(FCIE_WR_SBIT_TIMER, 0);
	REG_FCIE_W(FCIE_RD_SBIT_TIMER, 0);

	emmc_fcie_clear_events();
	emmc_platform_reset_post();

	return EMMC_ST_SUCCESS;
}

void emmc_fcie_clear_events(void)
{
	U16 u16_reg;

	REG_FCIE_W(FCIE_MIE_EVENT, BIT_ALL_CARD_INT_EVENTS);
	REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
	if (u16_reg & BIT_ALL_CARD_INT_EVENTS)
		emmc_fcie_reset();

	REG_FCIE_W1C(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);
}

void emmc_fcie_clear_events_reg0(void)
{
	U16 u16_reg;

	REG_FCIE_W(FCIE_MIE_EVENT, BIT_ALL_CARD_INT_EVENTS);
	REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
	if (u16_reg & BIT_ALL_CARD_INT_EVENTS)
		emmc_fcie_reset();
}

U32 emmc_fcie_wait_d0_high_ex(U32 u32_us)
{
	U32 u32_cnt;
	U16 u16_read0 = 0, u16_read1 = 0;

	for (u32_cnt = 0; u32_cnt < u32_us; u32_cnt++) {
		REG_FCIE_R(FCIE_SD_STATUS, u16_read0);
		emmc_hw_timer_delay(HW_TIMER_DELAY_1us);
		REG_FCIE_R(FCIE_SD_STATUS, u16_read1);

		if ((u16_read0 & BIT_SD_CARD_BUSY) == 0 &&
		    (u16_read1 & BIT_SD_CARD_BUSY) == 0)
			break;

		if (u32_cnt > 500 && u32_us - u32_cnt > 1000) {
			emmc_hw_timer_sleep(1);
			u32_cnt += (1000 - 2);
		}
	}

	return u32_cnt;
}

U32 emmc_fcie_wait_d0_high(U32 u32_us)
{
	U32 u32_cnt;

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	u32_cnt = emmc_fcie_wait_d0_high_ex(u32_us);

	if (u32_us == u32_cnt) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: wait D0 H timeout %u us\n", u32_cnt);
		return EMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
	}
	//emmc_hw_timer_delay(HW_TIMER_DELAY_1s  * 2);

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_send_cmd(U16 u16_mode, U16 u16_ctrl, U32 u32_arg, U8 u8_cmd_idx, U8 u8_rsp_byte_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_timeout = TIME_WAIT_DAT0_HIGH;
	U16 au16_tmp[3];

	if (u8_cmd_idx == MMC_CMD_ERASE)
		u32_timeout = TIME_WAIT_ERASE_DAT0_HIGH;

	//#if 0
	//EMMC_DISABLE_LOG(0);
	//emmc_debug(0, 1, "\n");
	//emmc_debug(0, 1, "cmd:%u, arg:%Xh, rspb:%Xh, mode:%Xh, ctrl:%Xh\n",
		   //u8_cmd_idx, u32_arg, u8_rsp_byte_cnt, u16_mode, u16_ctrl);
	//#endif

	REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);
	REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, u8_rsp_byte_cnt & BIT_RSP_SIZE_MASK);
	REG_FCIE_W(FCIE_SD_MODE, u16_mode);

	//set cmd
	//CIFC(0) = 39:32 | 47:40
	//CIFC(1) = 23:16 | 31:24
	//CIFC(2) = (CIFC(2) & 0xFF00) | 15:8, ignore (CRC7 | end_bit).
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_CHK_CMD);
	while (1) {
		REG_FCIE_W(FCIE_CMDFIFO_ADDR(0), ((u32_arg >> 24) << 8) | (0x40 | u8_cmd_idx));
		REG_FCIE_W(FCIE_CMDFIFO_ADDR(1), (u32_arg & 0xFF00) | ((u32_arg >> 16) & 0xFF));
		REG_FCIE_W(FCIE_CMDFIFO_ADDR(2), u32_arg & 0xFF);

		REG_FCIE_R(FCIE_CMDFIFO_ADDR(0), au16_tmp[0]);
		REG_FCIE_R(FCIE_CMDFIFO_ADDR(1), au16_tmp[1]);
		REG_FCIE_R(FCIE_CMDFIFO_ADDR(2), au16_tmp[2]);

		if (au16_tmp[0] == (((u32_arg >> 24) << 8) | (0x40 | u8_cmd_idx)) &&
		    au16_tmp[1] == ((u32_arg & 0xFF00) | ((u32_arg >> 16) & 0xFF)) &&
		    au16_tmp[2] == (u32_arg & 0xFF))
			break;
	}
	REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_CHK_CMD);

	if (u8_cmd_idx != MMC_CMD_STOP_TRANSMISSION) {
		u32_err = emmc_fcie_wait_d0_high(u32_timeout);
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_SEND_CMD_ERROR;
	}

	if (u8_cmd_idx == MMC_CMD_STOP_TRANSMISSION ||
	    u8_cmd_idx == MMC_CMD_WRITE_SINGLE_BLOCK ||
	    u8_cmd_idx == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
			emmc_check_power_cut();
	}

	REG_FCIE_W(FCIE_SD_CTRL, u16_ctrl);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	if (emmc_drv.u32_drvflag & DRV_FLAG_RSP_WAIT_D0H) {
		u32_err = emmc_fcie_wait_d0_high(u32_timeout);
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_SEND_CMD_ERROR;
	}

	//wait event
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_CMD_END, TIME_WAIT_CMDRSP_END);

LABEL_SEND_CMD_ERROR:
	return u32_err;
}

void emmc_fcie_get_cmd_fifo(U16 u16_word_pos, U16 u16_word_cnt, U16 *pu16_buf)
{
	U16 u16_i;

	if (u16_word_pos == 0x55)
		emmc_debug(1, 1, "\n");//remove warning

	for (u16_i = 0; u16_i < u16_word_cnt; u16_i++)
		REG_FCIE_R(FCIE_CMDFIFO_ADDR(u16_i), pu16_buf[u16_i]);

}

U32 emmc_wait_cifd_event(U16 u16_wait_event, U32  u32_micro_sec)
{
	U32 u32_count;
	U16 u16_reg;

	for (u32_count = 0; u32_count < u32_micro_sec; u32_count++) {
		REG_FCIE_R(NC_CIFD_EVENT, u16_reg);
		if ((u16_reg & u16_wait_event) == u16_wait_event)
			break;

		emmc_hw_timer_delay(HW_TIMER_DELAY_1us);
	}
	if (u32_count >= u32_micro_sec) {
		REG_FCIE_R(NC_CIFD_EVENT, u16_reg);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "CIFD Event Timeout %X\n", u16_reg);
	}

	return u32_count;
}

U32 emmc_wait_set_cifd(U8 *pu8_data_buf, U32 u32_byte_cnt)
{
	U16 u16_i, *pu16_data = (U16 *)pu8_data_buf;

	if (u32_byte_cnt > FCIE_CIFD_BYTE_CNT)
		return EMMC_ST_ERR_INVALID_PARAM;

	for (u16_i = 0; u16_i < (u32_byte_cnt >> 1); u16_i++)
		REG_FCIE_W(NC_RBUF_CIFD_ADDR(u16_i), pu16_data[u16_i]);

	REG_FCIE_SETBIT(NC_CIFD_EVENT, BIT_RBUF_FULL_TRI);

	if (emmc_wait_cifd_event(BIT_RBUF_EMPTY, HW_TIMER_DELAY_500ms) == HW_TIMER_DELAY_500ms) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: set CIFD timeout 0, ErrCode:%Xh\r\n",
			   EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT);
		return EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT;
	}

	if (emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_R2N_RDY_INT, HW_TIMER_DELAY_500ms)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: set CIFD timeout 1, ErrCode:%Xh\r\n",
			   EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT);

	return EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT;
}

	//emmc_debug(0, 1, "MIE_EVENT 0x%X\n", REG_FCIE(FCIE_MIE_EVENT));
	REG_FCIE_W(FCIE_MIE_EVENT, BIT_R2N_RDY_INT);
	//emmc_debug(0, 1, "MIE_EVENT 0x%X\n", REG_FCIE(FCIE_MIE_EVENT));
	REG_FCIE_SETBIT(NC_CIFD_EVENT, BIT_RBUF_EMPTY); // W1C

	return EMMC_ST_SUCCESS;
}

U32 emmc_wait_get_cifd(U8 *pu8_data_buf, U32 u32_byte_cnt)
{
	U16 u16_i, *pu16_data = (U16 *)pu8_data_buf;

	//emmc_debug(1, 1, "wait 64 bytes CIFD and copy to %08X\n", (U32)pu32_DataBuf);

	if (u32_byte_cnt > FCIE_CIFD_BYTE_CNT)
		return EMMC_ST_ERR_INVALID_PARAM;

	if (emmc_wait_cifd_event(BIT_WBUF_FULL, HW_TIMER_DELAY_500ms) == HW_TIMER_DELAY_500ms) {
		emmc_debug(1, 1, "eMMC Err: get CIFD timeout 0, ErrCode:%Xh\r\n",
			   EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT);
		return EMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT; // timeout
	}

	for (u16_i = 0; u16_i < (u32_byte_cnt >> 1); u16_i++)
		REG_FCIE_R(NC_WBUF_CIFD_ADDR(u16_i), pu16_data[u16_i]);

	REG_FCIE_W(NC_CIFD_EVENT, BIT_WBUF_FULL);
	REG_FCIE_W(NC_CIFD_EVENT, BIT_WBUF_EMPTY_TRI);

	return EMMC_ST_SUCCESS;
}

//===================================================
#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY

U8 au8_all_rsp[EMMC_SECTOR_512BYTE];//last 4 bytes are CRC

void emmc_keep_rsp(U8 *pu8_one_rsp_buf, U8 u8_cmd_idx)
{
	U16 u16_idx;
	U8  u8_byte_cnt;

	u16_idx = u8_cmd_idx * EMMC_CMD_BYTE_CNT;
	u8_byte_cnt = EMMC_CMD_BYTE_CNT;

	if (u8_cmd_idx > 10)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 3;
	else if (u8_cmd_idx > 9)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 2;
	else if (u8_cmd_idx > 2)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 1;

	if (u8_cmd_idx == 10 || u8_cmd_idx == 9 || u8_cmd_idx == 2)
		u8_byte_cnt = EMMC_R2_BYTE_CNT;

	if (u16_idx + u8_byte_cnt > EMMC_SECTOR_512BYTE - 4) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: KeepRsp over 512B, %Xh, %Xh, %Xh\n",
			   u8_cmd_idx, u16_idx, u8_byte_cnt);
		emmc_die("\n");//should be system fatal error, not eMMC driver
	}

	memcpy(au8_all_rsp + u16_idx, pu8_one_rsp_buf, u8_byte_cnt);
}

U32 emmc_return_rsp(U8 *pu8_one_rsp_buf, U8 u8_cmd_idx)
{
	U16 u16_idx;
	U8  u8_byte_cnt;

	u16_idx = u8_cmd_idx * EMMC_CMD_BYTE_CNT;
	u8_byte_cnt = EMMC_CMD_BYTE_CNT;

	if (u8_cmd_idx > 10)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 3;
	else if (u8_cmd_idx > 9)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 2;
	else if (u8_cmd_idx > 2)
		u16_idx += (EMMC_R2_BYTE_CNT - EMMC_CMD_BYTE_CNT) * 1;

	if (u8_cmd_idx == 10 || u8_cmd_idx == 9 || u8_cmd_idx == 2)
		u8_byte_cnt = EMMC_R2_BYTE_CNT;

	if (u16_idx + u8_byte_cnt > EMMC_SECTOR_512BYTE - 4) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: KeepRsp over 512B, %Xh, %Xh, %Xh\n",
			   u8_cmd_idx, u16_idx, u8_byte_cnt);
		return EMMC_ST_ERR_NO_RSP_IN_RAM;
	}

	if (au8_all_rsp[u16_idx] == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 1,
			   "eMMC Info: no rsp, %u %u\n", u8_cmd_idx, u16_idx);
		return EMMC_ST_ERR_NO_RSP_IN_RAM;
	}

	memcpy(pu8_one_rsp_buf, au8_all_rsp + u16_idx, u8_byte_cnt);

	return EMMC_ST_SUCCESS;
}

U32 emmc_save_context_rsp(void)
{
	emmc_drv.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv + 4, sizeof(emmc_drv) - 4);
	if (emmc_drv.u32_chksum == 0)
		emmc_drv.u32_chksum = 1;
	memcpy(GLOBAL_EMMC_SECTORBUF, &emmc_drv, sizeof(emmc_drv));

	emmc_cmd8(au8_ext_csd);
	memcpy(GLOBAL_EMMC_SECTORBUF + EMMC_SECTOR_512BYTE * CONTEXT_SIZE_BLKCNT,
	       au8_ext_csd, EMMC_SECTOR_512BYTE); //for kernel

	memcpy(GLOBAL_EMMC_SECTORBUF + EMMC_SECTOR_512BYTE * (CONTEXT_SIZE_BLKCNT + 1),
	       au8_all_rsp, EMMC_SECTOR_512BYTE); //for kernel

	emmc_cmd25(EMMC_UB_DRV_CONTX, GLOBAL_EMMC_SECTORBUF, CONTEXT_SIZE_BLKCNT + 2);

//emmc_debug(0, 1,"size: %Xh\n", sizeof(emmc_drv));
//emmc_dump_mem((void *)&emmc_drv, sizeof(emmc_drv));

	EMMC_SKIP_IDENTIFY_SAVE(0);
	return EMMC_ST_SUCCESS;
}


U32 emmc_load_context_rsp(void)
{
	U32 u32_err;
	U8  u8_i = 0;
	char *str_speed_mode[6] = {
			"LOW",
			"SDR",
			"DDR",
			"HS200",
			"HS400",
			"HS400 5.1"};

	u32_err = emmc_cmd18(EMMC_UB_DRV_CONTX, GLOBAL_EMMC_SECTORBUF, CONTEXT_SIZE_BLKCNT + 1);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	memcpy((void *)&emmc_drv, GLOBAL_EMMC_SECTORBUF, 4);
	if (emmc_drv.u32_chksum == 0 ||
	    emmc_drv.u32_chksum !=
	    emmc_chk_sum(GLOBAL_EMMC_SECTORBUF + sizeof(U32), sizeof(emmc_drv) - 4))
		return EMMC_ST_ERR_NO_CIS;

	memcpy((void *)&emmc_drv, GLOBAL_EMMC_SECTORBUF,
	       sizeof(emmc_drv) - sizeof(struct mtk_fcie_host *));

//emmc_debug(0, 1,"size: %Xh\n", sizeof(emmc_drv));
//emmc_dump_mem((void *)&emmc_drv, sizeof(emmc_drv));

	u32_err = emmc_fcie_check_cid();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

// load ExtCSD
	memcpy((void *)au8_ext_csd, GLOBAL_EMMC_SECTORBUF +
	       EMMC_SECTOR_512BYTE * CONTEXT_SIZE_BLKCNT, EMMC_SECTOR_512BYTE);

// no need to load AllRsp

//EMMC speed mode

	switch (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_MASK) {
	case DRV_FLAG_SPEED_HIGH:
		if (emmc_drv.u32_drvflag & DRV_FLAG_DDR_MODE)
			u8_i = 2;//ddr
		else
			u8_i = 1;//sdr
	break;
	case DRV_FLAG_SPEED_HS200:
			u8_i = 3;//hs200
	break;
	case DRV_FLAG_SPEED_HS400:
		#if defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1
		if (emmc_drv.u8_ecsd184_stroe_support)
			u8_i = 5;//hs400 5.1
		else
		#endif
			u8_i = 4;//hs400
	break;
	default:
	break;
	}

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\neMMC: %s", str_speed_mode[u8_i]);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, " %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);

	return EMMC_ST_SUCCESS;
}

#endif
//========================================================
// Send CMD HAL APIs
//========================================================
U32 emmc_identify(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U16 u16_i, u16_retry = 0;

	emmc_drv.u16_rca = 1;
	emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
	emmc_drv.u16_mmc_mode &= ~BIT_SD_DATA_WIDTH_MASK;
	emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_MASK;

LABEL_IDENTIFY_CMD0:
	EMMC_RST_L();  emmc_hw_timer_sleep(1);
	EMMC_RST_H();  emmc_hw_timer_sleep(1);

	if (emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH) != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: WaitD0High TO\n");
		emmc_fcie_err_handler_stop();
	}

	if (u16_retry > 10) {
		emmc_fcie_err_handler_stop();
		return u32_err;
	}

	if (u16_retry)
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: retry: %u\n", u16_retry);

// CMD0
	u32_err = emmc_cmd0(0); // reset to idle state
	if (u32_err != EMMC_ST_SUCCESS) {
		u16_retry++;
		goto LABEL_IDENTIFY_CMD0;
	}

// CMD1
	for (u16_i = 0; u16_i < EMMC_CMD1_RETRY_CNT; u16_i++) {
		//emmc_debug(EMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh\n", u16_i);
		u32_err = emmc_cmd1();
		if (u32_err == EMMC_ST_SUCCESS)
			break;

		emmc_hw_timer_sleep(2);

		if (u32_err != EMMC_ST_ERR_CMD1_DEV_NOT_RDY) {
			u16_retry++;
			goto LABEL_IDENTIFY_CMD0;
		}
	}

//emmc_debug(EMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh\n", u16_i);
	if (u32_err != EMMC_ST_SUCCESS) {
		u16_retry++;
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "CMD1 wait eMMC device ready timeout\n");
		goto LABEL_IDENTIFY_CMD0;
	}

// CMD2
	u32_err = emmc_cmd2();
	if (u32_err != EMMC_ST_SUCCESS) {
		u16_retry++;
		goto LABEL_IDENTIFY_CMD0;
	}

// CMD3
	u32_err = emmc_cmd3_cmd7(emmc_drv.u16_rca, MMC_CMD_SET_RELATIVE_ADDR);
	if (u32_err != EMMC_ST_SUCCESS) {
		u16_retry++;
		emmc_drv.u16_rca++;
		goto LABEL_IDENTIFY_CMD0;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_rom_boot_cmd0(U32 u32_arg, U32 u32_addr, U16 u16_blk_cnt)
{
	U32 u32_err;
	unsigned long dma_addr;
	U16 u16_mode, u16_ctrl, u16_val;

	u16_mode = BIT_SD_DEFAULT_MODE_REG | BIT_SD_DATA_WIDTH_8 | BIT_SD_DMA_R_CLK_STOP;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_DTRX_EN;

	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	dma_addr =
		emmc_translate_dma_address_ex((unsigned long)u32_addr,
					      EMMC_SECTOR_512BYTE * u16_blk_cnt);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 0, 0);

	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	REG_FCIE_R(TIMER1_CAP_LOW, u16_val);
	REG_FCIE_W(FCIE_0X5E, u16_val);
	REG_FCIE_R(TIMER1_CAP_HIGH, u16_val);
	REG_FCIE_W(FCIE_0X5F, u16_val);

	// wait event
	while (1) {
		if (((REG_FCIE(FCIE_MIE_EVENT) & BIT_DMA_END) == BIT_DMA_END) ||
		    (((REG_FCIE(TIMER1_CAP_HIGH) << 16) | REG_FCIE(TIMER1_CAP_LOW)) -
		    ((REG_FCIE(FCIE_0X5F) << 16) | REG_FCIE(FCIE_0X5E)) >= 12000000))
		break;
	}

	if ((REG_FCIE(FCIE_MIE_EVENT) & BIT_DMA_END) != BIT_DMA_END)
		u32_err = EMMC_ST_ERR_TIMEOUT_WAIT_REG0;

LABEL_END:
	return u32_err;
}

U32 emmc_cmd0(U32 u32_arg)
{
	U32 u32_err;
	U16 u16_ctrl;
	U8  u8_retry_cmd = 0;

	u16_ctrl = BIT_SD_CMD_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 0, 0);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD0 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_init();
			goto LABEL_SEND_CMD;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD0 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	}

//EMMC_FCIE_CLK_DIS();
	return u32_err;
}

// send OCR
U32 emmc_cmd1(void)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;

//(sector mode | byte mode) | (3.0 | 3.1 | 3.2 | 3.3 | 3.4 V)
	u32_arg = BIT30 | (BIT23 | BIT22 | BIT21 | BIT20 | BIT19 | BIT18 | BIT17 | BIT16 | BIT15 | BIT7);
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 1, EMMC_R3_BYTE_CNT);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: CMD1 send CMD fail: %08Xh\n", u32_err);

	return u32_err;
}

// check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
// R3 has no CRC, so does not check BIT_SD_RSP_CRC_ERR
	if (u16_reg & BIT_SD_RSP_TIMEOUT) {
		u32_err = EMMC_ST_ERR_CMD1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: CMD1 no Rsp, Reg.12: %04Xh\n", u16_reg);

		return u32_err;
	}
	emmc_fcie_get_cmd_fifo(0, 3, (U16 *)emmc_drv.au8_rsp);
//emmc_dump_mem(emmc_drv.au8_rsp, EMMC_R3_BYTE_CNT);

	if ((emmc_drv.au8_rsp[1] & 0x80) == 0) {
		u32_err =  EMMC_ST_ERR_CMD1_DEV_NOT_RDY;
	} else if (emmc_drv.au8_rsp[0] != 0x3F) {
		emmc_debug(0, 1, "CMD1 response buffer error\n");
			   emmc_fcie_err_handler_stop();
			   u32_err = EMMC_ST_ERR_CMD1;
	} else {
		emmc_drv.u8_if_sector_mode = (emmc_drv.au8_rsp[1] & BIT6) >> 6;
		EMMC_KEEP_RSP(emmc_drv.au8_rsp, 1);
	}

	return u32_err;
}

U32 emmc_cmd2(void)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;

	u32_arg = 0;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_RSPR2_EN;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, MMC_CMD_ALL_SEND_CID,
				     EMMC_R2_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD2, %Xh\n", u32_err);
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			u32_err = EMMC_ST_ERR_CMD2;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD2 Reg.12: %04Xh\n", u16_reg);
		} else {
			emmc_fcie_get_cmd_fifo(0, EMMC_R2_BYTE_CNT >> 1, (U16 *)emmc_drv.au8_cid);
			//emmc_dump_mem(emmc_drv.u16_rca, EMMC_R2_BYTE_CNT);

			if (emmc_drv.au8_cid[0] != 0x3F) {
				emmc_debug(0, 1, "CMD2 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD2;
			}
			EMMC_KEEP_RSP(emmc_drv.au8_cid, MMC_CMD_ALL_SEND_CID);
		}
	}
//EMMC_FCIE_CLK_DIS();

	return u32_err;
}

U32 emmc_cmd3_cmd7(U16 u16_rca, U8 u8_cmd_idx)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_r1 = 0, u8_retry_fcie = 0, u8_retry_cmd = 0;

	if (u8_cmd_idx == 7)
		emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;
	else
		u8_retry_fcie = 0xF0;//CMD3: not retry

	u32_arg = u16_rca << 16;

	if (u8_cmd_idx == MMC_CMD_SELECT_CARD && u16_rca != emmc_drv.u16_rca)
		u16_ctrl = BIT_SD_CMD_EN;
	else
		u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     u8_cmd_idx, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_cmd_idx == 3) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD%u retry:%u, %Xh\n",
				   u8_cmd_idx, u8_retry_cmd, u32_err);
			return u32_err;
		}
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD%u retry:%u, %Xh\n",
				   u8_cmd_idx, u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD%u retry:%u, %Xh\n",
			   u8_cmd_idx, u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		//check status
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
		//de-select has no rsp
		if (!(u8_cmd_idx == MMC_CMD_SELECT_CARD && u16_rca != emmc_drv.u16_rca)) {
			if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
				if (u8_cmd_idx == 3) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD%u Reg.12: %04Xh, Retry: %u\n",
						   u8_cmd_idx, u16_reg, u8_retry_fcie);
						return u32_err;
				}
				if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
				    EMMC_IF_DISABLE_RETRY() == 0) {
					u8_retry_fcie++;
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD%u Reg.12: %04Xh, Retry: %u\n",
						   u8_cmd_idx, u16_reg, u8_retry_fcie);
					emmc_fcie_err_handler_retry();
					goto LABEL_SEND_CMD;
				}

				u32_err = EMMC_ST_ERR_CMD3_CMD7;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD%u Reg.12: %04Xh, Retry: %u\n",
					   u8_cmd_idx, u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_stop();
			} else {
				U16 u16_tmp;

				emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
				//To prevent rep bit shift
				if ((u16_tmp & 0xFF) != u8_cmd_idx) {
						 emmc_debug(0, 1, "CMD%d response buffer error\n",
							    u8_cmd_idx);
					emmc_fcie_err_handler_stop();
					return EMMC_ST_ERR_CMD3_CMD7;
				}

				//CMD3 ok, do things here
				u32_err = emmc_check_r1_error();
				if (u32_err != EMMC_ST_SUCCESS) {
					if (u8_cmd_idx == 3) {
						emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
							   "eMMC WARN: CMD%u check R1 error: %Xh, retry: %u\n",
							   u8_cmd_idx, u32_err, u8_retry_r1);
						return u32_err;
					}

					if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT &&
					    EMMC_IF_DISABLE_RETRY() == 0) {
						u8_retry_r1++;
						emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
							   "eMMC WARN: CMD%u check R1 error: %Xh, retry: %u\n",
							   u8_cmd_idx, u32_err, u8_retry_r1);
							   emmc_fcie_err_handler_retry();
						goto LABEL_SEND_CMD;
					}
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Err: CMD%u check R1 error: %Xh, retry: %u\n",
						   u8_cmd_idx, u32_err, u8_retry_r1);
					emmc_fcie_err_handler_stop();
				}
				EMMC_KEEP_RSP(emmc_drv.au8_rsp, u8_cmd_idx);
			}
		}
	}

	EMMC_FCIE_CLK_DIS();
	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

//------------------------------------------------
U32 emmc_csd_config(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd9(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

// ------------------------------
	emmc_drv.u8_spec_vers = (emmc_drv.au8_csd[1] & 0x3C) >> 2;
	emmc_drv.u8_r_bl_len = emmc_drv.au8_csd[6] & 0xF;
	emmc_drv.u8_w_bl_len = ((emmc_drv.au8_csd[13] & 0x3) << 2) +
		((emmc_drv.au8_csd[14] & 0xC0) >> 6);

// ------------------------------
	emmc_drv.u16_c_size = (emmc_drv.au8_csd[7] & 3) << 10;
	emmc_drv.u16_c_size += emmc_drv.au8_csd[8] << 2;
	emmc_drv.u16_c_size += (emmc_drv.au8_csd[9] & 0xC0) >> 6;
	if (emmc_drv.u16_c_size == 0xFFF) {
		emmc_drv.u32_sec_count = 0;
	} else {
		emmc_drv.u8_c_size_mult = ((emmc_drv.au8_csd[10] & 3) << 1) +
			((emmc_drv.au8_csd[11] & 0x80) >> 7);

		emmc_drv.u32_sec_count =
			(emmc_drv.u16_c_size + 1) *
			(1 << (emmc_drv.u8_c_size_mult + 2)) *
			((1 << emmc_drv.u8_r_bl_len) >> 9) - 8;
	}

//printf("emmc_drv.u32_sec_count = %X\n", emmc_drv.u32_sec_count);

// ------------------------------
	emmc_drv.u8_erase_grp_size = (emmc_drv.au8_csd[10] & 0x7C) >> 2;
	emmc_drv.u8_erase_grp_mult = ((emmc_drv.au8_csd[10] & 0x03) << 3) +
		((emmc_drv.au8_csd[11] & 0xE0) >> 5);
	emmc_drv.u32_erase_unit_size = (emmc_drv.u8_erase_grp_size + 1) *
		(emmc_drv.u8_erase_grp_mult + 1);

// ------------------------------
// others
	emmc_drv.u8_taac = emmc_drv.au8_csd[2];
	emmc_drv.u8_nsac = emmc_drv.au8_csd[3];
	emmc_drv.u8_tran_speed = emmc_drv.au8_csd[4];
	emmc_drv.u8_r2w_factor = (emmc_drv.au8_csd[13] & 0x1C) >> 2;

	return EMMC_ST_SUCCESS;
}


// send CSD (in R2)
U32 emmc_cmd9(U16 u16_rca)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;

	u32_arg = u16_rca << 16;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_RSPR2_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     MMC_CMD_SEND_CSD, EMMC_R2_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD9 retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD9 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
			    EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD9 Reg.12: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD9;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD9 Reg.12: %04Xh, Retry fail: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			emmc_fcie_get_cmd_fifo(0, EMMC_R2_BYTE_CNT >> 1, (U16 *)emmc_drv.au8_csd);
			//emmc_dump_mem(emmc_drv.au8_csd, EMMC_R2_BYTE_CNT);

			if (emmc_drv.au8_csd[0] != 0x3F) {
				emmc_debug(0, 1, "CMD9 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD9;
			}

			EMMC_KEEP_RSP(emmc_drv.au8_csd, MMC_CMD_SEND_CSD);
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

// send CID
U32 emmc_cmd10(U16 u16_rca, U8 *pu8_res_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;

	emmc_skew4_rsp_reg();
	u32_arg = u16_rca << 16;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_RSPR2_EN;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     MMC_CMD_SEND_CID, EMMC_R2_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD10, %Xh\n", u32_err);
		return u32_err;
	}
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
	    u32_err = EMMC_ST_ERR_CMD10;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD10 Reg.12: %04Xh\n", u16_reg);
		return u32_err;
	}
	emmc_fcie_get_cmd_fifo(0, EMMC_R2_BYTE_CNT >> 1, (U16 *)pu8_res_buf);

	if (pu8_res_buf[0] != 0x3F) {
		emmc_debug(0, 1, "CMD10 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD10;
	}

	EMMC_FCIE_CLK_DIS();

	return u32_err;
}

U32 emmc_ext_csd_init(void)
{
	U32 u32_err;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_SET_BITS, 175, 1);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: set ERASE_GROUP_DEF fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.u8_ecsd224_hc_erase_grp_size) {
		emmc_drv.u32_erase_unit_size = (emmc_drv.u8_ecsd224_hc_erase_grp_size *
			EMMC_SECTOR_512BYTE_BITS * BIT10) >> EMMC_SECTOR_512BYTE_BITS;
	}

	//#if 0
	//set CLASS_6_CTRL to WP
	//u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_CLEAR_BITS, 59, 1);
	//if (u32_err != EMMC_ST_SUCCESS) {
		//emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   //"eMMC Err: set CLASS_6_CTRL to WP fail: %Xh\n", u32_err);
		//return u32_err;
	//}
	//#endif
//--------------------------------
// set HW RST
	if (au8_ext_csd[162] == 0) {
		u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 162, BIT0);//RST_FUNC
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: %Xh, eMMC, set Ext_CSD[162]: %Xh fail\n",
				   u32_err, BIT0);
			return u32_err;
		}
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_ext_csd_config(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd8(au8_ext_csd);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

//emmc_dump_mem(au8_ext_csd, 0x200);

//--------------------------------
	if (emmc_drv.u32_sec_count == 0)
		emmc_drv.u32_sec_count = ((au8_ext_csd[EXT_CSD_SEC_CNT_24_31] << 24) |
			(au8_ext_csd[EXT_CSD_SEC_CNT_16_23] << 16) |
			(au8_ext_csd[EXT_CSD_SEC_CNT_7_15] << 8) |
			(au8_ext_csd[EXT_CSD_SEC_CNT]));
	emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "emmc_drv.u32_sec_count = %X\n", emmc_drv.u32_sec_count);

//-------------------------------
	if (emmc_drv.u32_boot_sec_count == 0)
		emmc_drv.u32_boot_sec_count = (au8_ext_csd[EXT_CSD_BOOT_MULT] * 128) * 2;

	emmc_drv.u8_boot_size_mult = au8_ext_csd[EXT_CSD_BOOT_MULT];

//--------------------------------
	if (!emmc_drv.u8_bus_width) {
		emmc_drv.u8_bus_width = au8_ext_csd[EXT_CSD_BUS_WIDTH];
		switch (emmc_drv.u8_bus_width) {
		case 0:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
		break;
		case 1:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_4;
		break;
		case 2:
			emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_8;
		break;
		default:
			emmc_debug(0, 1, "eMMC Err: eMMC BUS_WIDTH not support\n");
		}
	}
//--------------------------------
	if (au8_ext_csd[EXT_SECURE_FEATURE_SUPPORT] & BIT4)//TRIM
		emmc_drv.u32_emmc_flag |= EMMC_FLAG_TRIM;
	else
		emmc_drv.u32_emmc_flag &= ~EMMC_FLAG_TRIM;

//--------------------------------
	if (au8_ext_csd[EXT_CSD_HPI_FEATURE] & BIT0) {
		if (au8_ext_csd[EXT_CSD_HPI_FEATURE] & BIT1)
			emmc_drv.u32_emmc_flag |= EMMC_FLAG_HPI_CMD12;
		else
			emmc_drv.u32_emmc_flag |= EMMC_FLAG_HPI_CMD13;
	} else {
			emmc_drv.u32_emmc_flag &= ~(EMMC_FLAG_HPI_CMD12 | EMMC_FLAG_HPI_CMD13);
	}
//--------------------------------
	emmc_drv.u32_rpmb_size_sec_cnt = (au8_ext_csd[EXT_CSD_RPMB_MULT] * 128 * 1024) >> 8;
//--------------------------------
	if (au8_ext_csd[EXT_CSD_WR_REL_PARAM] & BIT2) {
		emmc_drv.u16_reliable_w_blk_cnt = BIT_SD_JOB_BLK_CNT_MASK;
	} else {
		//#if 0
		//emmc_drv.u16_reliable_w_blk_cnt = au8_ext_csd[222];
		//#else
		if ((au8_ext_csd[EXT_CSD_HPI_FEATURE] & BIT0) &&
		    au8_ext_csd[EXT_CSD_REL_WR_SEC_C] == 1) {
			emmc_drv.u16_reliable_w_blk_cnt = 1;
		} else if (0 == (au8_ext_csd[EXT_CSD_HPI_FEATURE] & BIT0)) {
			emmc_drv.u16_reliable_w_blk_cnt = au8_ext_csd[EXT_CSD_REL_WR_SEC_C];
		} else {
			//emmc_debug(0,1, "eMMC Warn: not support dynamic  Reliable-W\n");
			emmc_drv.u16_reliable_w_blk_cnt = 0; // can not support Reliable Write
		}
		//#endif
	}

//--------------------------------
	emmc_drv.u8_erased_mem_content = au8_ext_csd[EXT_CSD_ERASEED_MEM_CONTENT];
	if (emmc_drv.u8_erased_mem_content)
		emmc_drv.u8_erased_mem_content = 0xFF;
//--------------------------------
	emmc_drv.u8_ecsd184_stroe_support = au8_ext_csd[EXT_CSD_STROBE_SUPPORT];
	emmc_drv.u8_ecsd185_hs_timing = au8_ext_csd[EXT_CSD_HS_TIMING];
	emmc_drv.u8_ecsd192_ver = au8_ext_csd[EXT_CSD_REV];
	emmc_drv.u8_ecsd196_dev_type = au8_ext_csd[EXT_CSD_CARD_TYPE];
	emmc_drv.u8_ecsd197_driver_strength = au8_ext_csd[EXT_CSD_DRV_STRENGTH];
	emmc_drv.u8_ecsd248_cmd6_to = au8_ext_csd[EXT_CSD_GENERIC_CMD6_TIME];
	emmc_drv.u8_ecsd247_pwr_off_long_to = au8_ext_csd[EXT_CSD_POWER_OFF_LONG_TIME];
	emmc_drv.u8_ecsd34_pwr_off_ctrl = au8_ext_csd[EXT_CSD_POWER_OFF_NOTIFICATION];
	emmc_drv.u8_partition_config = au8_ext_csd[EXT_CSD_PART_CONF];

//for GP Partition
	emmc_drv.u8_ecsd160_part_supfield = au8_ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
	emmc_drv.u8_ecsd224_hc_erase_grp_size = au8_ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
	emmc_drv.u8_ecsd221_hc_wp_grp_size = au8_ext_csd[EXT_CSD_HC_WP_GRP_SIZE];

	emmc_drv.gp_part[0].u32_part_size = ((au8_ext_csd[EXT_CSD_GP_SIZE_MULT_1_2] << 16) |
						(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_1_1] << 8) |
						(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_1_0])) *
						(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
						emmc_drv.u8_ecsd221_hc_wp_grp_size * 0x80000);

	emmc_drv.gp_part[1].u32_part_size = ((au8_ext_csd[EXT_CSD_GP_SIZE_MULT_2_2] << 16) |
					(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_2_1] << 8) |
						(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_2_0])) *
						(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
						emmc_drv.u8_ecsd221_hc_wp_grp_size * 0x80000);

	emmc_drv.gp_part[2].u32_part_size = ((au8_ext_csd[EXT_CSD_GP_SIZE_MULT_3_2] << 16) |
						(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_3_1] << 8) |
						(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_3_0])) *
						(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
						emmc_drv.u8_ecsd221_hc_wp_grp_size * 0x80000);

	emmc_drv.gp_part[3].u32_part_size = ((au8_ext_csd[EXT_CSD_GP_SIZE_MULT_4_2] << 16) |
					(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_4_1] << 8) |
					(au8_ext_csd[EXT_CSD_GP_SIZE_MULT_4_0])) *
					(emmc_drv.u8_ecsd224_hc_erase_grp_size  *
					emmc_drv.u8_ecsd221_hc_wp_grp_size * 0x80000);

//for Max Enhance Size
	emmc_drv.u8_ecsd157_max_enh_size_0 = au8_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];
	emmc_drv.u8_ecsd158_max_enh_size_1 = au8_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_1];
	emmc_drv.u8_ecsd159_max_enh_size_2 = au8_ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT_2];
	emmc_drv.u8_u8_ecsd155_part_set_complete = au8_ext_csd[EXT_CSD_PARTITION_SETTING];
	emmc_drv.u8_ecsd166_wr_rel_param = au8_ext_csd[EXT_CSD_WR_REL_PARAM];

	emmc_ext_csd_init();

	return EMMC_ST_SUCCESS;
}


U32 emmc_cmd8(U8 *pu8_data_buf)
{
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	return emmc_cmd8_cifd(pu8_data_buf);
	#else
	return emmc_cmd8_miu(pu8_data_buf);
	#endif
}

// CMD8: send EXT_CSD
U32 emmc_cmd8_miu(U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

// -------------------------------
	//#if 0
	//if (EMMC_IF_DISABLE_RETRY() == 0)
	   //emmc_fcie_err_handler_restore_clk();
	//#endif
	emmc_skew4_read_data_reg(&u16_status_err_flag);
// -------------------------------
//send cmd
	u32_arg =  0;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;

LABEL_SEND_CMD:
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 8, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD8 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD8 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}


// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS ||
	    (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD8 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD8_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD8 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if (!(emmc_drv.u8_emmc_pll_skew4 && emmc_drv.t_hs400_table.u8_set_cnt &&
	      emmc_drv.pad_type == FCIE_EMMC_HS400)) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_SEND_EXT_CSD) {
			emmc_debug(0, 1, "CMD8 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD8_MIU;
		}
// -------------------------------
// check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD8_MIU;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD8 check R1 error: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
	EMMC_KEEP_RSP(emmc_drv.au8_rsp, MMC_CMD_SEND_EXT_CSD);

LABEL_END:
	emmc_invalidate_data_cache_buffer((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

// CMD8: send EXT_CSD
U32 emmc_cmd8_cifd(U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg, u16_i;
	U8 u8_retry_fcie = 0, u8_retry_r1 = 0, u8_retry_cmd = 0;
	U16 u16_tmp;

// -------------------------------
// send cmd
	u32_arg =  0;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;

LABEL_SEND_CMD:
	u16_mode = BIT_DATA_DEST | emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 8, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD8 CIFD retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD8 CIFD retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}
//read for data
	for (u16_i = 0; u16_i < (EMMC_SECTOR_512BYTE >> 6); u16_i++) {
		u32_err = emmc_wait_get_cifd((U8 *)((unsigned long)pu8_data_buf + (u16_i << 6)), 0x40);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_fcie_err_handler_stop();
			goto LABEL_END;
		}
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & BIT_SD_FCIE_ERR_FLAGS)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD8 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		u32_err = EMMC_ST_ERR_CMD8_CIFD;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD8 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 8) {
		emmc_debug(0, 1, "CMD8 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD8_CIFD;
	}

// -------------------------------
// check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_r1++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD8 CIFD check R1 error: %Xh, Retry: %u\n",
				   u32_err, u8_retry_r1);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		u32_err = EMMC_ST_ERR_CMD8_CIFD;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD8 CIFD check R1 error: %Xh, Retry: %u\n",
			   u32_err, u8_retry_r1);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}
//EMMC_KEEP_RSP(emmc_drv.au8_rsp, 8);

LABEL_END:
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_set_pwr_off_notification(U8 u8_set_ecsd_pon)
{
	U32 u32_err;
	static U8 u8_old_ecsd_pon = EMMC_ST_SUCCESS;

	if (u8_old_ecsd_pon == EMMC_PWR_OFF_NOTIF_SHORT ||
	    u8_old_ecsd_pon == EMMC_PWR_OFF_NOTIF_LONG) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: PwrOffNotif already set: %u, now: %u\n",
			   u8_old_ecsd_pon, u8_set_ecsd_pon);

		return EMMC_ST_SUCCESS;
	}

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE,
				      EXT_CSD_POWER_OFF_NOTIFICATION,
				      u8_set_ecsd_pon);//PWR_OFF_NOTIFICATION
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_drv.u32_drvflag &= ~DRV_FLAG_PWR_OFF_NOTIF_LONG_MASK;
	switch (u8_set_ecsd_pon) {
	case EMMC_PWR_OFF_NOTIF_OFF:
		emmc_drv.u32_drvflag |= DRV_FLAG_PWR_OFF_NOTIF_OFF;
	break;
	case EMMC_PWR_OFF_NOTIF_ON:
		emmc_drv.u32_drvflag |= DRV_FLAG_PWR_OFF_NOTIF_ON;
	break;
	case EMMC_PWR_OFF_NOTIF_SHORT:
		emmc_drv.u32_drvflag |= DRV_FLAG_PWR_OFF_NOTIF_SHORT;
	break;
	case EMMC_PWR_OFF_NOTIF_LONG:
		emmc_drv.u32_drvflag |= DRV_FLAG_PWR_OFF_NOTIF_LONG;
	break;
	}

	return u32_err;
}

U32 emmc_sanitize(U8 u8_sanitize_val)
{
	U32 u32_err;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, EXT_CSD_SANITIZE, u8_sanitize_val);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH * 4);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	return EMMC_ST_SUCCESS;
}

U32 emmc_set_bus_speed(U8 u8_bus_speed)
{
	U32 u32_err;

	if (u8_bus_speed != EMMC_SPEED_HS400)
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;

	emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_TIMING;
	emmc_drv.u8_ecsd185_hs_timing |= u8_bus_speed;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE,
				      EXT_CSD_HS_TIMING, emmc_drv.u8_ecsd185_hs_timing);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_MASK;
	switch (u8_bus_speed) {
	case EMMC_SPEED_HIGH:
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HIGH;
	break;
	case EMMC_SPEED_HS200:
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS200;
	break;
	case EMMC_SPEED_HS400:
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400;
	break;
	default:
	break;
	}

	return u32_err;
}

U32 emmc_set_driving_strength(U8 u8_driving)
{
	U32 u32_err;

	emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
	emmc_drv.u8_ecsd185_hs_timing |= u8_driving << 4;

	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 185, emmc_drv.u8_ecsd185_hs_timing);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: fail: %Xh\n", u32_err);
		return EMMC_ST_ERR_SET_DRV_STRENGTH;
	}

	return EMMC_ST_SUCCESS;
}

// Use CMD6 to set ExtCSD[183] BUS_WIDTH
U32 emmc_set_bus_width(U8 u8_bus_width, U8 u8_if_ddr)
{
	U8  u8_value;
	U32 u32_err;

// -------------------------------
	switch (u8_bus_width) {
	case 1:
		u8_value = 0;
	break;
	case 4:
		u8_value = 1;
	break;
	case 8:
		u8_value = 2;
	break;
	default:
		return EMMC_ST_ERR_PARAMETER;
	}

	if (u8_if_ddr) {
		u8_value |= BIT2;
		emmc_drv.u32_drvflag |= DRV_FLAG_DDR_MODE;
	} else {
		emmc_drv.u32_drvflag &= ~DRV_FLAG_DDR_MODE;
	}
	if (u8_if_ddr == 2 && emmc_drv.u8_ecsd184_stroe_support) {
		//emmc_debug(0,1,"Enhance Strobe\n");
		u8_value |= BIT7; // Enhanced Storbe
	}

// -------------------------------
	u32_err = emmc_modify_ext_csd(MMC_SWITCH_MODE_WRITE_BYTE, 183, u8_value);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

// -------------------------------
	emmc_drv.u16_mmc_mode &= ~BIT_SD_DATA_WIDTH_MASK;
	switch (u8_bus_width) {
	case 1:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_1;
		emmc_drv.u16_mmc_mode |= BIT_SD_DATA_WIDTH_1;
	break;
	case 4:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_4;
		emmc_drv.u16_mmc_mode |= BIT_SD_DATA_WIDTH_4;
	break;
	case 8:
		emmc_drv.u8_bus_width = BIT_SD_DATA_WIDTH_8;
		emmc_drv.u16_mmc_mode |= BIT_SD_DATA_WIDTH_8;
	break;
	}

	//emmc_debug(EMMC_DEBUG_LEVEL,1,"set %u bus width\n", u8_bus_width);
	return u32_err;
}

U32 emmc_modify_ext_csd(U8 u8_access_mode, U8 u8_byte_idx, U8 u8_value)
{
	U32 u32_arg, u32_err;

	//emmc_debug(0, 1, "ByteIdx: %d(%03X), value %02X\n", u8_ByteIdx, u8_ByteIdx, u8_Value);

	u32_arg = ((u8_access_mode & 3) << 24) | (u8_byte_idx << 16) | (u8_value << 8);

	u32_err = emmc_cmd6(u32_arg);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh\n", u32_err);

		return u32_err;
	}

	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh\n", u32_err);
		return u32_err;
	}

	if (u8_byte_idx == EXT_CSD_PART_CONF) {
		switch (u8_access_mode & 3) {
		case MMC_SWITCH_MODE_SET_BITS:
			emmc_drv.u8_partition_config |= u8_value;
		break;
		case MMC_SWITCH_MODE_CLEAR_BITS:
			emmc_drv.u8_partition_config &= ~u8_value;
		break;
		case MMC_SWITCH_MODE_WRITE_BYTE:
			emmc_drv.u8_partition_config = u8_value;
		break;
		}
	}

	return u32_err;
}

// SWITCH cmd
U32 emmc_cmd6(U32 u32_arg)
{
	U32 u32_err;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;

	emmc_skew4_rsp_reg();

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, 6, EMMC_R1B_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD6 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD6 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
			    EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD6 Reg.12: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}
			u32_err = EMMC_ST_ERR_CMD6;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD6 Reg.12: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != 6) {
				emmc_debug(0, 1, "CMD6 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD6;
			}

//CMD6 ok, do things here
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD6 check R1 error: %Xh\n", u32_err);
				emmc_fcie_err_handler_stop();
			}
			EMMC_KEEP_RSP(emmc_drv.au8_rsp, 6);
		}
	}

	EMMC_FCIE_CLK_DIS();
	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

U32 emmc_cmd6_skew4(U32 u32_arg)
{
	U32 u32_err;
	U16 u16_ctrl, u16_reg, u16_tmp, u16_skew = 0;
	U8  u8_reg2ch = 0;

	if (emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_hs400_table.u8_set_cnt) {
		REG_FCIE_R(reg_emmcpll_0x03, u16_skew);
		u8_reg2ch = (REG_FCIE(REG_ANL_SKEW4_INV) & BIT_ANL_SKEW4_INV) ? 1 : 0;
		emmc_fcie_set_skew4_rsp_reg();
	}

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     MMC_CMD_SWITCH, EMMC_R1B_BYTE_CNT);

	if (!u32_err) {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u16_reg & BIT_SD_RSP_CRC_ERR)
				u32_err = EMMC_ST_ERR_CMD6_RSP;
			else
				u32_err = EMMC_ST_ERR_CMD6;
		} else {
			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
// To prevent rep bit shift
			if ((u16_tmp & 0xFF) != MMC_CMD_SWITCH) {
				emmc_debug(0, 1, "CMD6 response buffer error\n");
				emmc_fcie_err_handler_stop();
				u32_err = EMMC_ST_ERR_CMD6;
				goto LABEL_END;
			}
			u32_err = emmc_check_r1_error();
		}
	}

LABEL_END:

	if (emmc_drv.pad_type == FCIE_EMMC_HS400 && emmc_drv.t_hs400_table.u8_set_cnt) {
		REG_FCIE_W(reg_emmcpll_0x03, u16_skew);
		if (u8_reg2ch) {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		} else {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		}
	}

	EMMC_FCIE_CLK_DIS();
	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

U32 emmc_erase_cmd_seq(U32 u32_emmc_blk_addr_start, U32 u32_emmc_blk_addr_end)
{
	U32 u32_err = 0;
	U8 u8_retry_cmd = 0;

LABEL_SEND_CMD:
	u32_err = emmc_cmd35_cmd36(u32_emmc_blk_addr_start, 35);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEN_END;

	u32_err = emmc_cmd35_cmd36(u32_emmc_blk_addr_end, 36);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEN_END;

	u32_err = emmc_cmd38();
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEN_END;

	return EMMC_ST_SUCCESS;

LABEN_END:
	if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
		u8_retry_cmd++;
		emmc_fcie_err_handler_reinit();
		emmc_fcie_err_handler_retry();
		goto LABEL_SEND_CMD;
	}
	emmc_fcie_err_handler_stop();
	return u32_err;
}

U32 emmc_cmd35_cmd36(U32 u32_emmc_blk_addr, U8 u8_cmd_idx)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U16 u16_tmp;

	emmc_skew4_rsp_reg();

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, u8_cmd_idx, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD%u fail, %Xh\n",
			   u8_cmd_idx, u32_err);
		return u32_err;
	}
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC WARN: CMD%u Reg.12: %04Xh\n",
			   u8_cmd_idx, u16_reg);
		return EMMC_ST_ERR_CMD3536_ERR;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != u8_cmd_idx) {
		emmc_debug(0, 1, "CMD%d response buffer error\n", u8_cmd_idx);
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD3536_ERR;
	}

	// CMD35/36 ok, do things here
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD%u check R1 error: %Xh\n",
			   u8_cmd_idx, u32_err);
		return u32_err;
	}
	//EMMC_KEEP_RSP(emmc_drv.au8_rsp, u8_cmd_idx);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd38(void)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg, u16_tmp;

	emmc_skew4_rsp_reg();

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	if (emmc_drv.u32_emmc_flag & EMMC_FLAG_TRIM)
		u32_arg =  0x1;
	else
		u32_arg =  0x0;

	emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, 38, EMMC_R1B_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD38 fail: %Xh\n", u32_err);
		return u32_err;
	}
	//check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		u32_err = EMMC_ST_ERR_CMD38_ERR;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD38 Reg.12: %04Xh\n", u16_reg);
		return u32_err;
	}
	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 38) {
		emmc_debug(0, 1, "CMD38 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD38_ERR;
	}

	//CMD38 ok, do things here
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD38 check R1 error: %Xh\n", u32_err);
		return u32_err;
	}
	//EMMC_KEEP_RSP(emmc_drv.au8_rsp, 38);
	EMMC_FCIE_CLK_DIS();
	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

U32 emmc_dump_emmc_status(void)
{
	emmc_cmd13(emmc_drv.u16_rca);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC St: %Xh %Xh %Xh %Xh\n",
		   emmc_drv.au8_rsp[1], emmc_drv.au8_rsp[2],
		   emmc_drv.au8_rsp[3], emmc_drv.au8_rsp[4]);
	return EMMC_ST_SUCCESS;
}

//CMD13: send Status
U32 emmc_cmd13(U16 u16_rca)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;

	emmc_skew4_rsp_reg();
	u32_arg = (u16_rca << 16);
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, 13, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_cmd++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD13 retry:%u, %Xh\n",
					   u8_retry_cmd, u32_err);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD13 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
			    EMMC_IF_DISABLE_RETRY() == 0) {
					u8_retry_fcie++;
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD13 Reg.12: %04Xh, Retry: %u\n",
						   u16_reg, u8_retry_fcie);
					emmc_fcie_err_handler_reinit();
					emmc_fcie_err_handler_retry();
					goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD6;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD13 Reg.12: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != 13) {
				emmc_debug(0, 1, "CMD13 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD6;
			}

			//CMD13 ok, do things here
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(0, 1,
					   "eMMC: CMD13 check R1 error: %Xh, should not retry\n",
					   u32_err);
			} else {
				EMMC_KEEP_RSP(emmc_drv.au8_rsp, 13);
			}
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd13_skew4(U16 u16_rca)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;

	u32_arg = (u16_rca << 16);
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     MMC_CMD_SEND_STATUS, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD13 retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD13 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD13 Reg.12: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD6;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD13 Reg.12: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != MMC_CMD_SEND_STATUS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "CMD13 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD6;
			}

			//CMD13 ok, do things here
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC: CMD13 check R1 error: %Xh, should not retry\n",
					   u32_err);
			}
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd16(U32 u32_blk_length)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_r1 = 0, u8_retry_fcie = 0, u8_retry_cmd = 0;

	u32_arg = u32_blk_length;
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 16, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD16 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD16 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD16 Reg.12: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD16;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD16 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != 16) {
				emmc_debug(0, 1, "CMD16 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD16;
			}

			//CMD16 ok, do things here
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT &&
				    EMMC_IF_DISABLE_RETRY() == 0) {
					u8_retry_r1++;
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD16 check R1 error:%Xh,retry:%u\n",
						   u32_err, u8_retry_r1);
					emmc_fcie_err_handler_retry();
					goto LABEL_SEND_CMD;
				}
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD16 check R1 error: %Xh, retry: %u\n",
					   u32_err, u8_retry_r1);
				emmc_fcie_err_handler_stop();
			}
			//EMMC_KEEP_RSP(emmc_drv.au8_rsp, 16);
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

//------------------------------------------------
U32 emmc_cmd17(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	return emmc_cmd17_cifd(u32_emmc_blk_addr, pu8_data_buf);
	#else
	return emmc_cmd17_miu(u32_emmc_blk_addr, pu8_data_buf);
	#endif
}

#define IF_TEST_RETRY_FCIE5(x)  (0)

U32 emmc_cmd17_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;
	// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

	emmc_skew4_read_data_reg(&u16_status_err_flag);
// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;

LABEL_SEND_CMD:
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_BLK_SIZE, 0x200);//64 bytes tuning pattern

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 17, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 Reg.12:%04Xh, Err:%Xh, Retry:%u, Arg:%Xh\n",
				    u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD17_RSP;
		else if (u16_reg & BIT_SD_R_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD17_DATA_CRC;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if ((u16_reg & BIT_SD_RSP_CRC_ERR) == 0) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_READ_SINGLE_BLOCK) {
			emmc_debug(0, 1, "CMD17 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD17_MIU;
		}

//-------------------------------
//check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD17_MIU;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD17 check R1 error: %Xh, Arg: %Xh\n",
				   u32_err, u32_arg);
			emmc_fcie_err_handler_stop();
		}
	}

LABEL_END:
	emmc_invalidate_data_cache_buffer((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd17_cifd(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg, u16_i;
	U8  u8_retry_r1 = 0, u8_retry_fcie = 0, u8_retry_cmd = 0;
	U16 u16_reg_fcie_ddr_mode;
	U16 u16_tmp;

	//if(EMMC_IF_DISABLE_RETRY() == 0)
	//  emmc_fcie_err_handler_restore_clk();

	#if 1
	REG_FCIE_R(FCIE_DDR_MODE, u16_reg_fcie_ddr_mode);
	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT1 | BIT2 | BIT3);
	#endif

// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;

LABEL_SEND_CMD:
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = BIT_DATA_DEST | emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 17, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT  &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 CIFD retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 CIFD retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

//read for data
	for (u16_i = 0; u16_i < (EMMC_SECTOR_512BYTE >> 6); u16_i++) {
		u32_err = emmc_wait_get_cifd((U8 *)((unsigned long)pu8_data_buf +
			(u16_i << 6)), 0x40);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_fcie_err_handler_stop();
			goto LABEL_END;
		}
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & BIT_SD_FCIE_ERR_FLAGS)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD17 CIFD Reg.12:%04Xh,Err:%Xh,Retry:%u,Arg:%Xh\n",
					    u16_reg, u32_err, u8_retry_fcie, u32_arg);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD17_CIFD;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 17) {
		emmc_debug(0, 1, "CMD17 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD17_CIFD;
	}

// -------------------------------
// check device
	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_r1++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\n",
				   u32_err, u8_retry_r1, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}

		u32_err = EMMC_ST_ERR_CMD17_CIFD;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\n",
			   u32_err, u8_retry_r1, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:
	EMMC_FCIE_CLK_DIS();

	REG_FCIE_W(FCIE_DDR_MODE, u16_reg_fcie_ddr_mode);

	return u32_err;
}

U32 emmc_cmd17_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err = 0, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;

	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

	u16_status_err_flag &= ~BIT_SD_RSP_CRC_ERR;

//-------------------------------
//send cmd

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;

LABEL_SEND_CMD:

	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_BLK_SIZE, EMMC_SECTOR_512BYTE);

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & BIT_MIU_DMA_ADDR_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> BIT_MIU_DMA_ADDR_31_16_SHIFT);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & BIT_MIU_DMA_LEN_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> BIT_MIU_DMA_LEN_31_16_SHIFT);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg,
				     MMC_CMD_READ_SINGLE_BLOCK, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 retry:%u, %Xh, Arg: %Xh\n",
								 u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD17 Reg.12: %04Xh, Err:%Xh, Retry:%u, Arg:%Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD17_MIU;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD17_RSP;
		else if (u16_reg & BIT_SD_R_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD17_DATA_CRC;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if ((u16_reg & BIT_SD_RSP_CRC_ERR) == 0) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_READ_SINGLE_BLOCK) {
			emmc_debug(0, 1, "CMD17 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD17_MIU;
		}
// -------------------------------
// check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD17_MIU;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD17 check R1 error: %Xh, Arg: %Xh\n",
				   u32_err, u32_arg);
		}
	}

LABEL_END:

	emmc_invalidate_data_cache_buffer((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd12(U16 u16_rca)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_r1 = 0, u8_retry_cmd = 0;

	emmc_skew4_rsp_reg();

	#if 1
	u32_arg = (u16_rca << 16) |
		((emmc_drv.u32_emmc_flag & EMMC_FLAG_HPI_CMD12) ? 1 : 0);
	#else
	u32_arg = (u16_rca << 16);
	#endif

	#if defined(EMMC_UPDATE_FIRMWARE) && (EMMC_UPDATE_FIRMWARE)
	u32_arg = 0;
	#endif

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;

	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, 12, EMMC_R1B_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD12 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			//emmc_fcie_err_handler_retry();
			//goto LABEL_SEND_CMD;
			return u32_err;
		}
		u32_err = EMMC_ST_ERR_CMD12_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD12 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD12 check Reg.12: %Xh, %Xh, retry: %u\n",
					    u16_reg, u32_err, u8_retry_fcie);
				emmc_fcie_err_handler_reinit();
				//emmc_fcie_err_handler_retry();
				//goto LABEL_SEND_CMD;
				return u32_err;
		}

		u32_err = EMMC_ST_ERR_CMD12;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD12_RSP;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD12 check Reg.12: %Xh, %Xh, retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != 12) {
				emmc_debug(0, 1, "CMD12 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD12_RSP;
			}

			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
					u8_retry_r1++;
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD12 check R1 error: %Xh, Retry: %u\n",
						   u32_err, u8_retry_r1);
					//emmc_fcie_err_handler_retry();
					//goto LABEL_SEND_CMD;
					return u32_err;
				}
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD12 check R1 error: %Xh, Retry: %u\n",
					   u32_err, u8_retry_r1);
				emmc_fcie_err_handler_stop();
			}
			//EMMC_KEEP_RSP(emmc_drv.au8_rsp, 12);
		}
	}

	EMMC_FCIE_CLK_DIS();
	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

U32 emmc_cmd12_no_check(U16 u16_rca)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl;

	emmc_skew4_rsp_reg();

	#if 1
	u32_arg = (u16_rca << 16) |
		((emmc_drv.u32_emmc_flag & EMMC_FLAG_HPI_CMD12) ? 1 : 0);
	#else
	u32_arg = (u16_rca << 16);
	#endif

	#if defined(EMMC_UPDATE_FIRMWARE) && (EMMC_UPDATE_FIRMWARE)
	u32_arg = 0;
	#endif

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	emmc_drv.u32_drvflag |= DRV_FLAG_RSP_WAIT_D0H;


	emmc_fcie_clear_events();
	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl,
				     u32_arg, 12, EMMC_R1B_BYTE_CNT);

	emmc_drv.u32_drvflag &= ~DRV_FLAG_RSP_WAIT_D0H;
	return u32_err;
}

U32 emmc_cmd18(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	U16 u16_cnt;
	U32 u32_err;

	for (u16_cnt = 0; u16_cnt < u16_blk_cnt; u16_cnt++) {
		u32_err = emmc_cmd17_cifd(u32_emmc_blk_addr +
					  u16_cnt, pu8_data_buf + (u16_cnt << 9));
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	}
	#else
	return emmc_cmd18_miu(u32_emmc_blk_addr, pu8_data_buf, u16_blk_cnt);
	#endif
}

U32 emmc_cmd18_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

LABEL_SEND_CMD:
	u32_err = emmc_cmd23(u16_blk_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC WARN: CMD23 %Xh\n", u32_err);
		goto LABEL_END;
	}

	emmc_skew4_read_data_reg(&u16_status_err_flag);
// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	REG_FCIE_W(FCIE_BLK_SIZE, 0x200);//64 bytes tuning pattern

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf,
						 EMMC_SECTOR_512BYTE * u16_blk_cnt);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 18, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT &&
		    EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_cmd++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD18 retry:%u, %Xh\n",
					   u8_retry_cmd, u32_err);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD18 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END,
					TIME_WAIT_n_BLK_END * (1 + (u16_blk_cnt >> 11)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);

			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();

			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD18_RSP;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if ((u16_reg & BIT_SD_RSP_CRC_ERR) == 0) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_READ_MULTIPLE_BLOCK) {
			emmc_debug(0, 1, "CMD18 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD18;
		}

		//-------------------------------
		//check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD18;
			emmc_debug(1, 1, "eMMC Err: CMD18 check R1 error: %Xh, Arg: %Xh\n",
				   u32_err, u32_arg);
			emmc_fcie_err_handler_stop();
		}
	}
LABEL_END:

	//if(u32_err == EMMC_ST_SUCCESS )
	//    u32_err = emmc_cmd13(emmc_drv.u16_rca);

	emmc_invalidate_data_cache_buffer((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE * u16_blk_cnt);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd18_adma(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 len)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

	emmc_skew4_read_data_reg(&u16_status_err_flag);
// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN | BIT_ADMA_EN;

LABEL_SEND_CMD:
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, len);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x0010);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 0x0000);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 18, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD18 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD18 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_n_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
				   u16_reg, u32_err, u8_retry_fcie);

			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if ((u16_reg & BIT_SD_RSP_CRC_ERR) == 0) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_READ_MULTIPLE_BLOCK) {
			emmc_debug(0, 1, "CMD18 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD18;
		}

// -------------------------------
// check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD18;
			emmc_debug(1, 1, "eMMC Err: CMD18 check R1 error: %Xh, Arg: %Xh\n",
				   u32_err, u32_arg);
			emmc_fcie_err_handler_stop();
		}
	}
LABEL_END:
	if (emmc_cmd12(emmc_drv.u16_rca) != EMMC_ST_SUCCESS)
		emmc_cmd12_no_check(emmc_drv.u16_rca);

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd18_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;
	U16 u16_status_err_flag = BIT_SD_FCIE_ERR_FLAGS;

// -------------------------------
LABEL_SEND_CMD:
	u32_err = emmc_cmd23_skew4(u16_blk_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 %Xh\n", u32_err);
		goto LABEL_END;
	}

	u16_status_err_flag &= ~BIT_SD_RSP_CRC_ERR;//Ignore repsonse crc error in HS400 mode on M6E3

// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	REG_FCIE_W(FCIE_BLK_SIZE, EMMC_SECTOR_512BYTE); // 64 bytes tuning pattern

	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf,
						 EMMC_SECTOR_512BYTE * u16_blk_cnt);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & BIT_MIU_DMA_ADDR_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> BIT_MIU_DMA_ADDR_31_16_SHIFT);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & BIT_MIU_DMA_LEN_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> BIT_MIU_DMA_LEN_31_16_SHIFT);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, MMC_CMD_READ_MULTIPLE_BLOCK, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD18 retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD18 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

// -------------------------------
// check FCIE
	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_n_BLK_END *
					(1 + (u16_blk_cnt >> EMMC_SECTOR_512BYTE_BITS)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & u16_status_err_flag)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
					   u16_reg, u32_err, u8_retry_fcie);

				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD18;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD18_RSP;

		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n",
			   u16_reg, u32_err, u8_retry_fcie);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	if ((u16_reg & BIT_SD_RSP_CRC_ERR) == 0) {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != MMC_CMD_READ_MULTIPLE_BLOCK) {
			emmc_debug(0, 1, "CMD18 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD18;
		}

// -------------------------------
// check device
		u32_err = emmc_check_r1_error();
		if (u32_err != EMMC_ST_SUCCESS) {
			u32_err = EMMC_ST_ERR_CMD18;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD18 check R1 error: %Xh, Arg: %Xh\n",
				   u32_err, u32_arg);
		}
	}

LABEL_END:

	emmc_invalidate_data_cache_buffer((unsigned long)pu8_data_buf,
					  EMMC_SECTOR_512BYTE * u16_blk_cnt);
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd23_skew4(U16 u16_blk_cnt)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0, u8_reg2ch = 0;
	U16 u16_skew = 0, u16_tmp;

	if (emmc_drv.t_hs400_table.u16_rsp_valid_cnt > MIN_OK_SKEW_CNT) {
		REG_FCIE_R(reg_emmcpll_0x03, u16_skew);
		u8_reg2ch = (REG_FCIE(REG_ANL_SKEW4_INV) & BIT_ANL_SKEW4_INV) ? 1 : 0;
		emmc_fcie_set_skew4_rsp_reg();
	}

	u32_arg = u16_blk_cnt & 0xFFFF; // don't set BIT24

	#if EMMC_FEATURE_RELIABLE_WRITE
	u32_arg |= BIT31;//don't set BIT24
	#endif

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();

	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg,
				     MMC_CMD_SET_BLOCK_COUNT, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD23 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD23 retry:%u, %Xh\n", u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD23 SD_STS: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD23;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD23 SD_STS: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != MMC_CMD_SET_BLOCK_COUNT) {
				emmc_debug(0, 1, "CMD23 response buffer error\n");
				emmc_fcie_err_handler_stop();
				u32_err = EMMC_ST_ERR_CMD23;
			} else {
				//CMD23 ok, do things here
				u32_err = emmc_check_r1_error();
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Err: CMD23 check R1 error: %Xh\n", u32_err);
					emmc_fcie_err_handler_stop();
				}
			}
		}
	}

	if (emmc_drv.t_hs400_table.u16_rsp_valid_cnt > MIN_OK_SKEW_CNT) {
		REG_FCIE_W(reg_emmcpll_0x03, u16_skew);
		if (u8_reg2ch) {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		} else {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

// enable Reliable Write
U32 emmc_cmd23(U16 u16_blk_cnt)
{
	U32 u32_err, u32_arg;
	U16 u16_ctrl, u16_reg;
	U8  u8_retry_r1 = 0, u8_retry_fcie = 0, u8_retry_cmd = 0;

	emmc_skew4_rsp_reg();

	u32_arg = u16_blk_cnt & 0xFFFF;//don't set BIT24

	#if EMMC_FEATURE_RELIABLE_WRITE
	u32_arg |= BIT31;//don't set BIT24
	#endif

	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	emmc_fcie_clear_events();

	u32_err = emmc_fcie_send_cmd(emmc_drv.u16_mmc_mode, u16_ctrl, u32_arg, 23, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 retry:%u, %Xh\n",
				   u8_retry_cmd, u32_err);
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD23 retry:%u, %Xh\n",
			   u8_retry_cmd, u32_err);
		emmc_fcie_err_handler_stop();
	} else {
		REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

		if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
			if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT &&
			    EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_fcie++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD23 SD_STS: %04Xh, Retry: %u\n",
					   u16_reg, u8_retry_fcie);
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}

			u32_err = EMMC_ST_ERR_CMD23;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD23 SD_STS: %04Xh, Retry: %u\n",
				   u16_reg, u8_retry_fcie);
			emmc_fcie_err_handler_stop();
		} else {
			U16 u16_tmp;

			emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);
			//To prevent rep bit shift
			if ((u16_tmp & 0xFF) != 23) {
				emmc_debug(0, 1, "CMD23 response buffer error\n");
				emmc_fcie_err_handler_stop();
				return EMMC_ST_ERR_CMD23;
			}

			// CMD23 ok, do things here
			u32_err = emmc_check_r1_error();
			if (u32_err != EMMC_ST_SUCCESS) {
				if (u8_retry_r1 < EMMC_CMD_API_ERR_RETRY_CNT &&
				    EMMC_IF_DISABLE_RETRY() == 0) {
					u8_retry_r1++;
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC WARN: CMD23 check R1 error:%Xh, retry:%u\n",
						   u32_err, u8_retry_r1);
					emmc_fcie_err_handler_retry();
					goto LABEL_SEND_CMD;
				}
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: CMD23 check R1 error: %Xh, retry: %u\n",
					   u32_err, u8_retry_r1);
				emmc_fcie_err_handler_stop();
			}
			//EMMC_KEEP_RSP(emmc_drv.au8_rsp, 23);
		}
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd25(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	U16 u16_cnt;
	#endif

	#if  ENABLE_EMMC_RIU_MODE
	U16 u16_retry_cnt = 0;
LABEL_CMD25:
	#endif

	// ------------------------------RIU mode
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	for (u16_cnt = 0; u16_cnt < u16_blk_cnt; u16_cnt++) {
		u32_err = emmc_cmd24_cifd(u32_emmc_blk_addr + u16_cnt,
					  pu8_data_buf + (u16_cnt << EMMC_SECTOR_BYTECNT_BITS));
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_CMD25_END;
	}
	#else
	// ------------------------------MIU mode
	u32_err = emmc_cmd25_miu(u32_emmc_blk_addr, pu8_data_buf, u16_blk_cnt);
	#endif

	#if ENABLE_EMMC_RIU_MODE
LABEL_CMD25_END:
	if (u32_err != EMMC_ST_SUCCESS && u16_retry_cnt < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT) {
		u16_retry_cnt++;
		goto LABEL_CMD25;
	}
	#endif

	return u32_err;
}

U32 emmc_cmd25_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;

	// -------------------------------
	// restore clock to max
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

LABEL_SEND_CMD:
	u32_err = emmc_cmd23(u16_blk_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 %Xh\n", u32_err);
		goto LABEL_END;
	}

	// -------------------------------
	// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE * u16_blk_cnt);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 25, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	// -------------------------------
	// check device
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN:CMD25 Reg12:%04Xh,Err:%Xh,Retry:%u,Arg:%Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD25_RSP;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 25) {
		emmc_debug(0, 1, "CMD25 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD25;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 check R1 error: %Xh, Arg: %Xh\n",
			   u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	// -------------------------------
	// send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_WAIT_D0H;
		goto LABEL_END;
	}

	emmc_check_power_cut();
	emmc_skew4_write_data_reg();
	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END,
					TIME_WAIT_n_BLK_END * (1 + (u16_blk_cnt >> 9)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:

	if (u32_err == EMMC_ST_SUCCESS)
		u32_err = emmc_cmd13(emmc_drv.u16_rca);

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd25_adma(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U32 len)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;

	// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

	emmc_skew4_rsp_reg();
	// -------------------------------
	// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, len);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x0010);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 0x0000);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 25, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}
	// -------------------------------
	// check device
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_cmd12_no_check(emmc_drv.u16_rca);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 25) {
		emmc_debug(0, 1, "CMD25 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD25;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 check R1 error: %Xh, Arg: %Xh\n",
			   u32_err,  u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_WAIT_D0H;
		goto LABEL_END;
	}

	emmc_check_power_cut();
	emmc_skew4_write_data_reg();
	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W | BIT_ADMA_EN);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT,
					BIT_DMA_END, TIME_WAIT_n_BLK_END * (1 + (len >> 20)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Arg: %Xh\n",
			   u16_reg, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:

	if (u32_err == EMMC_ST_SUCCESS) {
		u32_err = emmc_cmd12(emmc_drv.u16_rca);
		if (u32_err == EMMC_ST_SUCCESS)
			u32_err = emmc_cmd13(emmc_drv.u16_rca);
	} else {
		emmc_cmd12_no_check(emmc_drv.u16_rca);
	}

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd25_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf, U16 u16_blk_cnt)
{
	U32 u32_err = 0, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0, u8_reg2ch = 0;
	unsigned long dma_addr;
	U16  u16_skew = 0, u16_tmp;

LABEL_SEND_CMD:
	u32_err = emmc_cmd23_skew4(u16_blk_cnt);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 %Xh\n", u32_err);
		goto LABEL_END;
	}
	emmc_skew4_write_data_stored_original_reg(&u16_skew, &u8_reg2ch);
	// -------------------------------
	// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, u16_blk_cnt);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE * u16_blk_cnt);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & BIT_MIU_DMA_ADDR_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> BIT_MIU_DMA_ADDR_31_16_SHIFT);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (EMMC_SECTOR_512BYTE * u16_blk_cnt) & BIT_MIU_DMA_LEN_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (EMMC_SECTOR_512BYTE * u16_blk_cnt) >> BIT_MIU_DMA_LEN_31_16_SHIFT);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg,
				     MMC_CMD_WRITE_MULTIPLE_BLOCK, EMMC_R1_BYTE_CNT);

	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_WRITE_BACK_SKEW;
	}

	// -------------------------------
	// check FCIE
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD25_RSP;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_stop();
			goto LABEL_WRITE_BACK_SKEW;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != MMC_CMD_WRITE_MULTIPLE_BLOCK) {
		emmc_debug(0, 1, "CMD25 response buffer error\n");
		emmc_fcie_err_handler_stop();
		u32_err = EMMC_ST_ERR_CMD25;
		goto LABEL_WRITE_BACK_SKEW;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD25_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 check R1 error: %Xh, Arg: %Xh\n",
			   u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_WRITE_BACK_SKEW;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		u32_err = EMMC_ST_ERR_CMD25_WAIT_D0H;

LABEL_WRITE_BACK_SKEW:

	emmc_skew4_write_data_restored_reg(u16_skew, u8_reg2ch);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT,
					BIT_DMA_END, TIME_WAIT_n_BLK_END * (1 + (u16_blk_cnt >> 9)));

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
	}

LABEL_END:

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd24(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	#if defined(ENABLE_EMMC_RIU_MODE) && ENABLE_EMMC_RIU_MODE
	return emmc_cmd24_cifd(u32_emmc_blk_addr, pu8_data_buf);
	#else
	return emmc_cmd24_miu(u32_emmc_blk_addr, pu8_data_buf);
	#endif
}

U32 emmc_cmd24_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	unsigned long dma_addr;
	U16 u16_tmp;

// -------------------------------
	if (EMMC_IF_DISABLE_RETRY() == 0)
		emmc_fcie_err_handler_restore_clk();

	emmc_skew4_rsp_reg();
	//-------------------------------
	//send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 24, EMMC_R1_BYTE_CNT);
		if (u32_err != EMMC_ST_SUCCESS) {
			if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
				u8_retry_cmd++;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: CMD24 retry:%u, %Xh, Arg: %Xh\n",
					   u8_retry_cmd, u32_err, u32_arg);
				emmc_fcie_err_handler_reinit();
				emmc_fcie_err_handler_retry();
				goto LABEL_SEND_CMD;
			}
			u32_err = EMMC_ST_ERR_CMD24_RSP;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD24 retry:%u, %Xh, Arg: %Xh\n",
				   u8_retry_cmd, u32_err, u32_arg);
			emmc_fcie_err_handler_stop();
			goto LABEL_END;
		}

	// -------------------------------
	// check device
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
		u8_retry_fcie++;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC WARN: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_reinit();
		emmc_fcie_err_handler_retry();
		goto LABEL_SEND_CMD;
	}
	u32_err = EMMC_ST_ERR_CMD24_MIU;
	if (u16_reg & BIT_SD_RSP_CRC_ERR)
		u32_err = EMMC_ST_ERR_CMD24_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			    u16_reg, u32_err, u8_retry_fcie, u32_arg);
			    emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 24) {
		emmc_debug(0, 1, "CMD24 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD24_MIU;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_MIU_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_MIU_WAIT_D0H;
		goto LABEL_END;
	}

	emmc_check_power_cut();
	emmc_skew4_write_data_reg();
	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT,
					BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Warn: CMD24 %sReg.12: %04Xh, Err:%Xh, Retry:%u, Arg:%Xh\n",
				   (u32_err != EMMC_ST_SUCCESS) ? "TO " : "",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD24_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 %sReg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   (u32_err != EMMC_ST_SUCCESS) ? "TO " : "",
			   u16_reg, u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:
	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

U32 emmc_cmd24_cifd(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg, u16_i;
	U16 u16_reg_fcie_ddr_mode = 0;
	U16 u16_tmp;

// -------------------------------
	#if 1
	REG_FCIE_R(FCIE_DDR_MODE, u16_reg_fcie_ddr_mode);
	REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT1 | BIT2 | BIT3);
	#endif

	//-------------------------------
	//send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = BIT_DATA_DEST | emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> 16);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg, 24, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 CIFD, %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	//-------------------------------
	//check device
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		u32_err = EMMC_ST_ERR_CMD25;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Arg: %Xh\n",
			   u16_reg, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != 24) {
		emmc_debug(0, 1, "CMD24 response buffer error\n");
		emmc_fcie_err_handler_stop();
		return EMMC_ST_ERR_CMD24_CIFD;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_CIFD_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 CIFD check R1 error: %Xh, Arg: %Xh\n",
			   u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

	//-------------------------------
	//send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_CIFD_WAIT_D0H;
		goto LABEL_END;
	}

	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	for (u16_i = 0; u16_i < (EMMC_SECTOR_512BYTE >> 6); u16_i++) {
		u32_err = emmc_wait_set_cifd((U8 *)((unsigned long)pu8_data_buf + (u16_i << 6)), 0x40);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_fcie_err_handler_stop();
			goto LABEL_END;
		}
	}

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);

	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		u32_err = EMMC_ST_ERR_CMD24_CIFD;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 CIFD Reg.12: %04Xh, Err: %Xh, Arg: %Xh\n",
			   u16_reg, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:
	EMMC_FCIE_CLK_DIS();

	#if 1
	REG_FCIE_W(FCIE_DDR_MODE, u16_reg_fcie_ddr_mode);
	#endif

	return u32_err;
}

U32 emmc_cmd24_skew4_miu(U32 u32_emmc_blk_addr, U8 *pu8_data_buf)
{
	U32 u32_err = 0, u32_arg;
	U16 u16_mode, u16_ctrl, u16_reg;
	U8  u8_retry_fcie = 0, u8_retry_cmd = 0;
	U8 u8_reg2ch = 0;
	unsigned long dma_addr;
	U16 u16_skew = 0, u16_tmp;

	emmc_skew4_write_data_stored_original_reg(&u16_skew, &u8_reg2ch);

// -------------------------------
// send cmd
	u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

LABEL_SEND_CMD:

	u32_arg =  u32_emmc_blk_addr << (emmc_drv.u8_if_sector_mode ? 0 : EMMC_SECTOR_512BYTE_BITS);
	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;

	emmc_fcie_clear_events();
	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)pu8_data_buf, EMMC_SECTOR_512BYTE);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & BIT_MIU_DMA_ADDR_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> BIT_MIU_DMA_ADDR_31_16_SHIFT);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> BIT_MIU_DMA_ADDR_47_32_SHIFT);

	REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, EMMC_SECTOR_512BYTE & BIT_MIU_DMA_LEN_15_0_MASK);
	REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, EMMC_SECTOR_512BYTE >> BIT_MIU_DMA_LEN_31_16_SHIFT);

	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, u32_arg,
				     MMC_CMD_WRITE_SINGLE_BLOCK, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		if (u8_retry_cmd < EMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_cmd++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD24_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 retry:%u, %Xh, Arg: %Xh\n",
			   u8_retry_cmd, u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_WRITE_BACK_SKEW;
	}

// -------------------------------
// check FCIE
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD24_MIU;
		if (u16_reg & BIT_SD_RSP_CRC_ERR)
			u32_err = EMMC_ST_ERR_CMD24_RSP;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
				   u16_reg, u32_err, u8_retry_fcie, u32_arg);
			emmc_fcie_err_handler_stop();
			goto LABEL_WRITE_BACK_SKEW;
	}

	emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

	//To prevent rep bit shift
	if ((u16_tmp & 0xFF) != MMC_CMD_WRITE_SINGLE_BLOCK) {
		emmc_debug(0, 1, "CMD24 response buffer error\n");
		emmc_fcie_err_handler_stop();
		u32_err = EMMC_ST_ERR_CMD24_MIU;
		goto LABEL_WRITE_BACK_SKEW;
	}

	u32_err = emmc_check_r1_error();
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD24_MIU_CHK_R1;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_WRITE_BACK_SKEW;
	}
	// -------------------------------
	// send data
	u32_err = emmc_fcie_wait_d0_high(TIME_WAIT_DAT0_HIGH);
	if (u32_err != EMMC_ST_SUCCESS)
		u32_err = EMMC_ST_ERR_CMD24_MIU_WAIT_D0H;

LABEL_WRITE_BACK_SKEW:

	emmc_skew4_write_data_restored_reg(u16_skew, u8_reg2ch);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END;

	REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DTRX_EN | BIT_SD_DAT_DIR_W);
	REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_JOB_START);

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT,
					BIT_DMA_END, TIME_WAIT_1_BLK_END);

	// -------------------------------
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

	if (u32_err != EMMC_ST_SUCCESS || (u16_reg & (BIT_SD_W_FAIL | BIT_SD_W_CRC_ERR))) {
		if (u8_retry_fcie < EMMC_CMD_API_ERR_RETRY_CNT && EMMC_IF_DISABLE_RETRY() == 0) {
			u8_retry_fcie++;
			emmc_fcie_err_handler_reinit();
			emmc_fcie_err_handler_retry();
			goto LABEL_SEND_CMD;
		}
		u32_err = EMMC_ST_ERR_CMD24_MIU;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD24 %sReg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
			   (u32_err != EMMC_ST_SUCCESS) ? "TO " : "", u16_reg,
			   u32_err, u8_retry_fcie, u32_arg);
		emmc_fcie_err_handler_stop();
		goto LABEL_END;
	}

LABEL_END:

	EMMC_FCIE_CLK_DIS();
	return u32_err;
}

static const U8 hs200_tuning_pattern_128[128] = {
0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC, 0xCC,
0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF,
0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF, 0xBB,
0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE, 0xFF,
0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC,
0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE,
0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF,
0xBB, 0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE
};

static const U8 hs200_tuning_pattern_64[64] = {
0xFF, 0x0F, 0xFF, 0x00, 0xFF, 0xCC, 0xC3, 0xCC, 0xC3, 0x3C, 0xCC, 0xFF, 0xFE, 0xFF, 0xFE, 0xEF,
0xFF, 0xDF, 0xFF, 0xDD, 0xFF, 0xFB, 0xFF, 0xFB, 0xBF, 0xFF, 0x7F, 0xFF, 0x77, 0xF7, 0xBD, 0xEF,
0xFF, 0xF0, 0xFF, 0xF0, 0x0F, 0xFC, 0xCC, 0x3C, 0xCC, 0x33, 0xCC, 0xCF, 0xFF, 0xEF, 0xFF, 0xEE,
0xFF, 0xFD, 0xFF, 0xFD, 0xDF, 0xFF, 0xBF, 0xFF, 0xBB, 0xFF, 0xF7, 0xFF, 0xF7, 0x7F, 0x7B, 0xDE
};

// use memcmp to confirm tuning pattern
U32 emmc_cmd21_miu(void)
{
	U32 u32_err = 0;
	U16 u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DTRX_EN, u16_reg;
	U16 u16_mode, u16_tmp;
	U8 i;
	unsigned long dma_addr;

	emmc_fcie_clear_events();

	if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
		REG_FCIE_W(FCIE_BLK_SIZE, 64);//64 bytes tuning pattern
		REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 64 & 0xFFFF);
		REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 64 >> 16);
	} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
		REG_FCIE_W(FCIE_BLK_SIZE, 128);//128 bytes tuning pattern
		REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 128 & 0xFFFF);
		REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 128 >> 16);
	} else {
		u32_err = EMMC_ST_ERR_CMD21_ONE_BIT;
		emmc_debug(0, 0, "eMMC Warn: emmc_drv.u8_bus_width = %02Xh\n", emmc_drv.u8_bus_width);
		goto ERROR_HANDLE;
	}
	if (!(emmc_drv.u8_ecsd196_dev_type & BIT4)) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: eMMC not support HS200 1.8V\n");
		u32_err = EMMC_ST_ERR_CMD21_NO_HS200_1_8V;
		goto ERROR_HANDLE;
	}

	REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
	dma_addr = emmc_translate_dma_address_ex((unsigned long)GLOBAL_EMMC_SECTORBUF, EMMC_SECTOR_512BYTE);

	REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, dma_addr & 0xFFFF);
	REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, dma_addr >> 16);
	if (sizeof(unsigned long) == 8)
		REG_FCIE_W(FCIE_MIU_DMA_ADDR_47_32, dma_addr >> 32);

	u16_mode = emmc_drv.u16_mmc_mode | emmc_drv.u8_bus_width;
	u32_err = emmc_fcie_send_cmd(u16_mode, u16_ctrl, 0, 21, EMMC_R1_BYTE_CNT);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = EMMC_ST_ERR_CMD21_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD21 send CMD fail: %08Xh\n", u32_err);
		goto ERROR_HANDLE;
	}

	u32_err = emmc_fcie_wait_events(FCIE_MIE_EVENT, BIT_DMA_END, HW_TIMER_DELAY_100ms);
	if (u32_err)
		goto ERROR_HANDLE;

	emmc_invalidate_data_cache_buffer((unsigned long)GLOBAL_EMMC_SECTORBUF, EMMC_SECTOR_512BYTE);
	//check status
	REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
	if (u16_reg & (BIT_SD_RSP_TIMEOUT | BIT_SD_RSP_CRC_ERR)) {
		u32_err = EMMC_ST_ERR_CMD21_RSP;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err:CMD21 no Rsp/CRC err, SD_STS: %04Xh\n", u16_reg);
		goto ERROR_HANDLE;
	} else if (u16_reg & BIT_SD_R_CRC_ERR) {
		u32_err = EMMC_ST_ERR_CMD21_DATA_CRC;
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: CMD21 data CRC err, Reg.12: %04Xh\n", u16_reg);
		goto ERROR_HANDLE;
	} else {
		emmc_fcie_get_cmd_fifo(0, 1, &u16_tmp);

		//To prevent rep bit shift
		if ((u16_tmp & 0xFF) != 21) {
			emmc_debug(0, 1, "CMD21 response buffer error\n");
			emmc_fcie_err_handler_stop();
			return EMMC_ST_ERR_CMD21_RSP;
		}

		if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_4) {
			if (memcmp((void *)hs200_tuning_pattern_64, (void *)GLOBAL_EMMC_SECTORBUF, 64)) {
				printf("tuning pattern 4bit width:");
				for (i = 0; i < 64; i++) {
					if (i % 16 == 0)
						printf("\n\t");
					printf("%02X ", GLOBAL_EMMC_SECTORBUF[i]);
				}
				printf("\n");
				u32_err = EMMC_ST_ERR_CMD21_DATA_CMP;
				goto ERROR_HANDLE;
			}
		} else if (emmc_drv.u8_bus_width == BIT_SD_DATA_WIDTH_8) {
			if (memcmp((void *)hs200_tuning_pattern_128, (void *)GLOBAL_EMMC_SECTORBUF, 128)) {
				printf("tuning pattern 8bit width:");
				for (i = 0; i < 128; i++) {
					if (i % 16 == 0)
						printf("\n\t");
					printf("%02X ", GLOBAL_EMMC_SECTORBUF[i]);
				}
				printf("\n");
				u32_err = EMMC_ST_ERR_CMD21_DATA_CMP;
				goto ERROR_HANDLE;
			}
		}
	}

ERROR_HANDLE:
	REG_FCIE_W(FCIE_BLK_SIZE, EMMC_SECTOR_512BYTE); //restore anyway...
	if (u32_err)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %08Xh\n", u32_err);

	return u32_err;
}

//eMMC CMD21 adtc, R1, fix 128 clock, for HS200 only
//64 bytes in 4 bits mode
//128 bytes in 8 bits mode
U32 emmc_cmd21(void)//send tuning block
{
	return emmc_cmd21_miu();
}

U32 emmc_get_r1(void)
{
	return (emmc_drv.au8_rsp[1] << 24) | (emmc_drv.au8_rsp[2] << 16) |
		(emmc_drv.au8_rsp[3] << 8) | emmc_drv.au8_rsp[4];
}

static void emmc_print_r1_31_24_error(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_ST_ERR_R1_31_24\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_ADDRESS_OUT_OF_RANGE >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_R1_ADDRESS_OUT_OF_RANGE\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_ADDRESS_MISALIGN >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_R1_ADDRESS_MISALIGN\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_BLOCK_LEN_ERROR >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_BLOCK_LEN_ERROR\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_ERASE_SEQ_ERROR >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_ERASE_SEQ_ERROR\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_ERASE_PARAM >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_ERASE_PARAM\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_WP_VIOLATION >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_WP_VIOLATION\n");
	if (emmc_drv.au8_rsp[1] & (EMMC_R1_LOCK_UNLOCK_FAILED >> 24))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_LOCK_UNLOCK_FAILED\n");
}

static void emmc_print_r1_23_16_error(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_ST_ERR_R1_23_16\n");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_COM_CRC_ERROR >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_COM_CRC_ERROR\n");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_ILLEGAL_COMMAND >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_ILLEGAL_COMMAND\n");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_DEVICE_ECC_FAILED >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_DEVICE_ECC_FAILED\n");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_CC_ERROR >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_CC_ERROR\n");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_ERROR >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_ERROR\n ");
	if (emmc_drv.au8_rsp[2] & (EMMC_R1_CID_CSD_OVERWRITE >> 16))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_CID_CSD_OVERWRITE\n");
}

static void emmc_print_r1_15_8_error(void)
{
	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_ST_ERR_R1_15_8\n");

	if (emmc_drv.au8_rsp[3] & (EMMC_R1_WP_ERASE_SKIP >> 8))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_WP_ERASE_SKIP\n");
	if (emmc_drv.au8_rsp[3] & (EMMC_R1_ERASE_RESET >> 8))
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_ERASE_RESET\n");
}

U32 emmc_check_r1_error(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	emmc_fcie_get_cmd_fifo(0, 3, (U16 *)emmc_drv.au8_rsp);

	if (emmc_drv.au8_rsp[1] & (EMMC_ERR_R1_31_24 >> 24)) {
		emmc_print_r1_31_24_error();
		u32_err = EMMC_ST_ERR_R1_31_24;
		goto LABEL_CHECK_R1_END;
	}

	if (emmc_drv.au8_rsp[2] & (EMMC_ERR_R1_23_16 >> EMMC_ERR_R1_23_16_SHIFT)) {
		emmc_print_r1_23_16_error();
		u32_err = EMMC_ST_ERR_R1_23_16;
		goto LABEL_CHECK_R1_END;
	}

	if (emmc_drv.au8_rsp[3] & (EMMC_ERR_R1_15_8 >> EMMC_ERR_R1_15_8_SHIFT)) {
		emmc_print_r1_15_8_error();
		u32_err = EMMC_ST_ERR_R1_15_8;
		goto LABEL_CHECK_R1_END;
	}

	if (emmc_drv.au8_rsp[4] & EMMC_ERR_R1_7_0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EMMC_ST_ERR_R1_7_0\n");
		if (emmc_drv.au8_rsp[4] & EMMC_R1_SWITCH_ERROR)
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC_R1_SWITCH_ERROR\n");

		u32_err = EMMC_ST_ERR_R1_7_0;
		goto LABEL_CHECK_R1_END;
	}

LABEL_CHECK_R1_END:

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_dump_mem(emmc_drv.au8_rsp, EMMC_R1_BYTE_CNT);

		if (emmc_drv.au8_rsp[0] == MMC_CMD_WRITE_SINGLE_BLOCK ||
		    emmc_drv.au8_rsp[0] == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
		    emmc_drv.au8_rsp[0] == MMC_CMD_READ_MULTIPLE_BLOCK ||
		    emmc_drv.au8_rsp[0] == MMC_CMD_STOP_TRANSMISSION)
			emmc_fcie_err_handler_reinit();
	}

	return u32_err;
}

// ====================================================

#endif
#endif

