// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (C) 2023 MediaTek Inc.
 */

#include "eMMC.h"

// ==========================================================
U32 emmc_fcie_enable_sdr_mode(void)
{
	U32 u32_err_speed = EMMC_ST_SUCCESS, u32_err_width = EMMC_ST_SUCCESS, u32_err = EMMC_ST_SUCCESS;

	//----------------------------------------

	if (DRV_FLAG_SPEED_HS200 == EMMC_SPEED_MODE() || DRV_FLAG_SPEED_HS400 == EMMC_SPEED_MODE()) {
		U32 u32_drvflag = emmc_drv.u32_drvflag;

		u32_err = emmc_fcie_err_handler_reinit_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh\n", u32_err);
			return u32_err;
		}
		emmc_drv.u32_drvflag = u32_drvflag;
	}

	u32_err_speed = emmc_set_bus_speed(EMMC_SPEED_HIGH);
	u32_err_width = emmc_set_bus_width(8, 0);

	if (u32_err_speed != EMMC_ST_SUCCESS  || u32_err_width != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: %Xh %Xh\n", u32_err_speed, u32_err_width);
		return u32_err_speed ? u32_err_speed : u32_err_width;
	}

	//----------------------------------------
	//set to normal SDR 48MHz
	emmc_pads_switch(FCIE_EMMC_SDR);
	emmc_clock_setting(FCIE_DEFAULT_CLK);

	return EMMC_ST_SUCCESS;
}

//===========================
void emmc_dump_atop_table(void)
{
	U16 u16_skew4_cnt = 0, u16_m6e3_rx_pass_percent = 0;
	U32 u32_i;

	if (emmc_drv.chip_id == MT5896_CHIP_ID) {
		if (emmc_drv.pad_type == FCIE_EMMC_HS400_AIFO_5_1) {
			for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
				if (emmc_drv.u32_fix_skew4_result & (1 << u32_i))
					u16_skew4_cnt++;
			}
			if (u16_skew4_cnt >= PLL_SKEW4_CNT)
				u16_skew4_cnt = (PLL_SKEW4_CNT - 1);

			u16_m6e3_rx_pass_percent = ((u16_skew4_cnt * 10) / PLL_SKEW4_CNT) * 10 +
						    ((u16_skew4_cnt % 10));

			emmc_debug(EMMC_DEBUG_LEVEL, 0, "    Clk: %04Xh, RX %02d%% > 50%%,TX: %02Xh\n",
				   emmc_drv.t_table.set[0].u8_clk,
				   u16_m6e3_rx_pass_percent,
				   emmc_drv.t_table.set[0].u8_skew2);
		} else if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
			for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
				if (emmc_drv.u32_fix_skew4_result & (1 << u32_i))
					u16_skew4_cnt++;
			}
			if (u16_skew4_cnt >= PLL_SKEW4_CNT)
				u16_skew4_cnt = (PLL_SKEW4_CNT - 1);

			u16_m6e3_rx_pass_percent = ((u16_skew4_cnt * 10) / PLL_SKEW4_CNT) * 10 +
				((u16_skew4_cnt % 10));

			emmc_debug(EMMC_DEBUG_LEVEL, 0, "    Clk: %04Xh, RX %02d%% > 50%%, CMD: %02Xh, TX: %02Xh\n",
				   emmc_drv.t_hs400_table.set_skew4.u16_clk,
				   u16_m6e3_rx_pass_percent,
				   emmc_drv.t_hs400_table.set_skew4.u8_skew4_rsp,
				   emmc_drv.t_hs400_table.set_skew4.u8_skew2);
		} else {
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n  eMMC ATop Timing Table: Ver:%Xh Cnt:%u CurIdx:%u\n",
				   emmc_drv.t_table.u32_ver_no,
				   emmc_drv.t_table.u8_set_cnt, emmc_drv.t_table.u8_cur_set_idx);

			emmc_debug(EMMC_DEBUG_LEVEL, 0, "    Clk: %04Xh, Result: %08Xh, Reg2Ch: %02Xh, Skew4: %02Xh\n",
				   emmc_drv.t_table.set[0].u8_clk,
				   emmc_drv.t_table.set[0].u32_scan_result,
				   emmc_drv.t_table.set[0].u8_reg2ch,
				   emmc_drv.t_table.set[0].u8_skew4);
		}
	} else {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\n  eMMC ATop Timing Table: Ver:%Xh Cnt:%u CurIdx:%u\n",
			   emmc_drv.t_table.u32_ver_no,
			   emmc_drv.t_table.u8_set_cnt, emmc_drv.t_table.u8_cur_set_idx);

		emmc_debug(EMMC_DEBUG_LEVEL, 0, "    Clk: %04Xh, Result: %08Xh, Reg2Ch: %02Xh, Skew4: %02Xh\n",
			   emmc_drv.t_table.set[0].u8_clk,
			   emmc_drv.t_table.set[0].u32_scan_result,
			   emmc_drv.t_table.set[0].u8_reg2ch,
			   emmc_drv.t_table.set[0].u8_skew4);
		if (emmc_drv.pad_type == FCIE_EMMC_HS400 || emmc_drv.pad_type == FCIE_EMMC_HS400_AIFO_5_1) {
				emmc_debug(EMMC_DEBUG_LEVEL, 0, "      Skew2: %02Xh, RXDLL: %Xh, RXDLLCnt: %02Xh\n",
					   emmc_drv.t_table.set[0].u8_skew2,
					   emmc_drv.t_table.set[0].u8_cell,
					   emmc_drv.t_table.set[0].u8_cell_cnt);
		}
	}
}

void emmc_fcie_set_atop_timing_reg(U8 u8_set_idx)
{
	if (emmc_drv.pad_type == FCIE_EMMC_DDR) {
		REG_FCIE_CLRBIT(reg_emmcpll_0x6c, BIT_DQS_MODE_MASK | BIT_DQS_DELAY_CELL_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x6c, emmc_drv.t_table.set[u8_set_idx].u8_skew4 << BIT_DQS_MDOE_SHIFT);
		REG_FCIE_SETBIT(reg_emmcpll_0x6c, emmc_drv.t_table.set[u8_set_idx].u8_cell << BIT_DQS_DELAY_CELL_SHIFT);
	} else {
		#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200) || \
		(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
		if (emmc_drv.t_table.set[u8_set_idx].u8_reg2ch) {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		} else {
			#ifdef REG_ANL_SKEW4_INV
			REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif
		}

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, emmc_drv.t_table.set[u8_set_idx].u8_skew4 << BIT_SKEW4_SHIFT);

		#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
		emmc_fcie_set_delay_line(emmc_drv.t_table.set[u8_set_idx].u8_cell);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, emmc_drv.t_table.set[u8_set_idx].u8_skew2 << BIT_SKEW2_SHIFT);
		#endif
		#endif
	}
}

void emmc_fcie_apply_reg(U8 u8_set_idx)
{
	int i;
	unsigned long t_table_reg_base = RIU_BASE;

	if (emmc_drv.host->dev_comp->emmcpllbase_t_tablebase > 0)
		t_table_reg_base = ((unsigned long)emmc_drv.host->emmcpllbase);

	for (i = 0; i < emmc_drv.t_table_g.u8_register_cnt; i++) {
		if (emmc_drv.t_table_g.reg_set[u8_set_idx *
		    emmc_drv.t_table_g.u8_register_cnt + i].u16_op_code == REG_OP_W) {
			REG_FCIE_W(t_table_reg_base +
				emmc_drv.t_table_g.reg_set[u8_set_idx *
				emmc_drv.t_table_g.u8_register_cnt + i].u32_reg_address,
				emmc_drv.t_table_g.reg_set[u8_set_idx *
				emmc_drv.t_table_g.u8_register_cnt + i].u16_reg_value);
		} else if (emmc_drv.t_table_g.reg_set[u8_set_idx *
			   emmc_drv.t_table_g.u8_register_cnt + i].u16_op_code == REG_OP_CLRBIT) {
			REG_FCIE_CLRBIT(t_table_reg_base +
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u32_reg_address,
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u16_reg_value);
		} else if (emmc_drv.t_table_g.reg_set[u8_set_idx *
			   emmc_drv.t_table_g.u8_register_cnt + i].u16_op_code == REG_OP_SETBIT) {
			REG_FCIE_CLRBIT(t_table_reg_base +
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u32_reg_address,
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u16_reg_mask);

			REG_FCIE_SETBIT(t_table_reg_base +
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u32_reg_address,
					emmc_drv.t_table_g.reg_set[u8_set_idx *
					emmc_drv.t_table_g.u8_register_cnt + i].u16_reg_value);
		} else if (emmc_drv.t_table_g.reg_set[u8_set_idx *
			   emmc_drv.t_table_g.u8_register_cnt + i].u16_op_code == 0) {
			//do nothing here
			;
		} else {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "unknown OpCode 0x%X\n",
				   emmc_drv.t_table_g.reg_set[u8_set_idx *
				   emmc_drv.t_table_g.u8_register_cnt + i].u16_op_code);
			emmc_die();
		}
	}
}

void emmc_fcie_set_skew4_rsp_reg(void)
{
	U8 u8_skew4 = 0, u8_reg2ch = 0;

	u8_skew4 = emmc_drv.t_hs400_table.set_skew4.u8_skew4_rsp;
	u8_reg2ch = emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_rsp;

	if (u8_reg2ch > 0) {
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
	} else {
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew4 << BIT_SKEW4_SHIFT);
}

void emmc_fcie_set_skew4_data_reg(void)
{
	U8 u8_skew4 = 0, u8_reg2ch = 0;

	u8_skew4 = emmc_drv.t_hs400_table.set_skew4.u8_skew4_data;
	u8_reg2ch = emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_data;

	if (u8_reg2ch > 0) {
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
	} else {
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew4 << BIT_SKEW4_SHIFT);
}

void emmc_fcie_set_tx_reg(void)
{
	U8 u8_skew2 = 0;

	if (emmc_drv.t_hs400_table.u8_set_cnt)
		u8_skew2 = emmc_drv.t_hs400_table.set_skew4.u8_skew2;

	if (emmc_drv.t_hs400_table.u8_set_cnt) {
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew2 << BIT_SKEW2_SHIFT);
	}
}

void emmc_skew4_rsp_reg(void)
{
	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.t_hs400_table.u8_set_cnt && emmc_drv.pad_type == FCIE_EMMC_HS400)
		emmc_fcie_set_skew4_rsp_reg();

}

void emmc_skew4_read_data_reg(U16 *u16_status_err_flag)
{
	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.t_hs400_table.u8_set_cnt && emmc_drv.pad_type == FCIE_EMMC_HS400) {
		(*u16_status_err_flag) &= ~BIT_SD_RSP_CRC_ERR;//Ignore repsonse crc error in HS400 mode on M6E3
		emmc_fcie_set_skew4_data_reg();
	}
}

void emmc_skew4_write_data_reg(void)
{
	if (emmc_drv.u8_emmc_pll_skew4 && emmc_drv.t_hs400_table.u8_set_cnt &&
	    emmc_drv.pad_type == FCIE_EMMC_HS400) {
		emmc_fcie_set_skew4_data_reg();
	}
}

void emmc_skew4_write_data_stored_original_reg(U16 *u16_skew, U8 *u8_reg2ch)
{
	U16 u16_skew4 = 0;
	U8 u8_skew_invert_reg = 0;

	if (emmc_drv.t_hs400_table.u16_rsp_valid_cnt > MIN_OK_SKEW_CNT) {
		REG_FCIE_R(reg_emmcpll_0x03, u16_skew4);
		u8_skew_invert_reg = (REG_FCIE(REG_ANL_SKEW4_INV) & BIT_ANL_SKEW4_INV) ? 1 : 0;
		emmc_fcie_set_skew4_rsp_reg();
	}
	*u16_skew = u16_skew4;
	*u8_reg2ch = u8_skew_invert_reg;
}

void emmc_skew4_write_data_restored_reg(U16 u16_skew, U8 u8_reg2ch)
{
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
}

void emmc_dump_general_table(void)
{
	int i, j;

	emmc_debug(EMMC_DEBUG_LEVEL, 0,
		   "\n eMMC General Timing Table: Ver:%Xh Cnt:%u CurIdx:%u SpeedMode:%Xh dev_driving:%Xh\n",
		   emmc_drv.t_table_g.u32_ver_no, emmc_drv.t_table_g.u8_set_cnt,
		   emmc_drv.t_table_g.u8_cur_set_idx, emmc_drv.t_table_g.u8_speed_mode,
		   emmc_drv.t_table_g.u32_device_driving);

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "CID in GT Table\n");
	emmc_dump_mem(emmc_drv.t_table_g.au8_cid, EMMC_MAX_RSP_BYTE_CNT);
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "OpCode Address Value Mask\n");

	for (i = 0; i < emmc_drv.t_table_g.u8_set_cnt; i++) {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "==========Set 0x%X==========\n", i);

		for (j = 0; j < emmc_drv.t_table_g.u8_register_cnt; j++) {
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "0x%X 0x%08X 0x%04X 0x%04X\n",
				   emmc_drv.t_table_g.reg_set[i * emmc_drv.t_table_g.u8_register_cnt + j].u16_op_code,
				   emmc_drv.t_table_g.reg_set[i * emmc_drv.t_table_g.u8_register_cnt + j].u32_reg_address,
				   emmc_drv.t_table_g.reg_set[i * emmc_drv.t_table_g.u8_register_cnt + j].u16_reg_value,
				   emmc_drv.t_table_g.reg_set[i * emmc_drv.t_table_g.u8_register_cnt + j].u16_reg_mask);
		}
	}
}

void emmc_fcie_apply_timing_set(U8 u8_idx)
{
	// make sure a complete outside clock cycle
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

	if ((emmc_drv.pad_type != FCIE_EMMC_HS400 && emmc_drv.pad_type != FCIE_EMMC_HS400_5_1)) {
		emmc_clock_setting(emmc_drv.t_table.set[u8_idx].u8_clk);
		emmc_fcie_set_atop_timing_reg(u8_idx);
		emmc_drv.t_table.u8_cur_set_idx = u8_idx;
	} else {
		if (emmc_drv.u8_emmc_pll_skew4) {
			if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
				emmc_clock_setting(emmc_drv.t_hs400_table.set_skew4.u16_clk);
				emmc_fcie_set_tx_reg();
			} else if (emmc_drv.pad_type == FCIE_EMMC_HS400_5_1) {
				//apply ext table
				emmc_clock_setting(emmc_drv.t_table_g.u32_clk);
				emmc_fcie_apply_reg(u8_idx);
				emmc_drv.t_table_g.u8_cur_set_idx = u8_idx;
			}
		} else {
			//apply ext table
			emmc_clock_setting(emmc_drv.t_table_g.u32_clk);
			emmc_fcie_apply_reg(u8_idx);
			emmc_drv.t_table_g.u8_cur_set_idx = u8_idx;
		}
	}
}

void emmc_dump_timing_table(void)
{
	emmc_dump_atop_table();
}

static U32 emmc_load_hs400_timing_table_ex(void)
{
	U32 u32_err = 0, u32_chksum = 0;
	U8  u8_reg2ch = 0, u8_skew4 = 0;

	if (emmc_drv.u8_ecsd184_stroe_support) {
		memcpy((U8 *)&emmc_drv.t_table, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table));

		u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
		if (u32_chksum != emmc_drv.t_table.u32_chksum || u32_chksum == 0) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: ChkSum error, no Table\n");
			u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
			goto LABEL_END_OF_NO_TABLE;
		}
		//check if need auto-update
		if (emmc_drv.t_table.u32_ver_no != EMMC_M6E3_TIMING_TABLE_VERSION) {
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: auto update Timing Table ...%Xh %Xh\n",
				   EMMC_M6E3_TIMING_TABLE_VERSION, emmc_drv.t_table.u32_ver_no);
			u32_err = EMMC_ST_ERR_DDRT_NONA;
			goto LABEL_END_OF_NO_TABLE;
		}
		//read general table
		u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC WARN: load 1st Gen_TTable fail, %Xh\n", u32_err);

			u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF, 1);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Warn: load 2nd Gen_TTables fail, %Xh\n", u32_err);
				goto LABEL_END_OF_NO_TABLE;
			}
		}
		memcpy((U8 *)&emmc_drv.t_table_g, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table_g));
		u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table_g.u32_ver_no,
					  (sizeof(emmc_drv.t_table_g) - sizeof(U32)/*checksum*/));
		if (u32_chksum != emmc_drv.t_table_g.u32_chksum || u32_chksum == 0) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Warn: ChkSum error, no Gen_TTable\n");
			u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
			goto LABEL_END_OF_NO_TABLE;
		}
		//check if need auto-update
		if (emmc_drv.t_table_g.u32_ver_no != EMMC_M6E3_TIMING_TABLE_VERSION) {
			emmc_debug(EMMC_DEBUG_LEVEL, 1,
				   "eMMC Warn: auto update Gen_TTable ...%Xh %Xh\n",
				   EMMC_M6E3_TIMING_TABLE_VERSION, emmc_drv.t_table_g.u32_ver_no);
			u32_err = EMMC_ST_ERR_DDRT_NONA;
			goto LABEL_END_OF_NO_TABLE;
		}

		if (memcmp(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1)) {
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: mismatch CID, need to rebuild TTable\n");
			u32_err = EMMC_ST_ERR_DDRT_NONA;
			goto LABEL_END_OF_NO_TABLE;
		}
		u8_reg2ch = emmc_drv.t_table.set[0].u8_reg2ch;
		u8_skew4 = emmc_drv.t_table.set[0].u8_skew4;
		emmc_drv.host->id_matched_driving = emmc_drv.t_table_g.u32_device_driving;
	} else {
		memcpy((U8 *)&emmc_drv.t_hs400_table, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_hs400_table));
		u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_hs400_table, sizeof(emmc_drv.t_hs400_table) - 8);
		if (u32_chksum != emmc_drv.t_hs400_table.u32_chksum || u32_chksum == 0) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: ChkSum error, no Table\n");
			u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
			goto LABEL_END_OF_NO_TABLE;
		}
		//--------------------------------------
		//check if need auto-update
		if (emmc_drv.t_hs400_table.u32_ver_no != EMMC_M6E3_TIMING_TABLE_VERSION) {
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: auto update Timing Table ...%Xh %Xh\n",
				   EMMC_M6E3_TIMING_TABLE_VERSION, emmc_drv.t_hs400_table.u32_ver_no);
			u32_err = EMMC_ST_ERR_DDRT_NONA;
			goto LABEL_END_OF_NO_TABLE;
		}

		if (memcmp(emmc_drv.t_hs400_table.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1)) {
			emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: mismatch CID, need to rebuild TTable\n");
			u32_err = EMMC_ST_ERR_DDRT_NONA;
			goto LABEL_END_OF_NO_TABLE;
		}
		u8_reg2ch = emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_data;
		u8_skew4 = emmc_drv.t_hs400_table.set_skew4.u8_skew4_data;
		emmc_drv.host->id_matched_driving = emmc_drv.t_hs400_table.u32_device_driving;
	}

	emmc_drv.cur_skew4 = (u8_reg2ch > 0) ? (PLL_SKEW4_CNT + u8_skew4) : u8_skew4;

LABEL_END_OF_NO_TABLE:

	return u32_err;
}

static U32 emmc_load_hs200_timing_table_ex(void)
{
	U32 u32_err = 0, u32_chksum = 0;

	memcpy((U8 *)&emmc_drv.t_table, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table));
	u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
	if (u32_chksum != emmc_drv.t_table.u32_chksum) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: ChkSum error, no Table\n");
		u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
		goto LABEL_END_OF_NO_TABLE;
	}
	if (u32_chksum == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: no Table\n");
		u32_err = EMMC_ST_ERR_DDRT_NONA;
		goto LABEL_END_OF_NO_TABLE;
	}

	u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF, 1);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC WARN: load 1st Gen_TTable fail, %Xh\n", u32_err);

		u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Warn: load 2nd Gen_TTables fail, %Xh\n", u32_err);
			goto LABEL_END_OF_NO_TABLE;
		}
	}

	memcpy((U8 *)&emmc_drv.t_table_g, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table_g));

	if (memcmp(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1,
			   "eMMC Warn: mismatch CID, need to rebuild TTable\n");
		u32_err = EMMC_ST_ERR_DDRT_NONA;
	}

LABEL_END_OF_NO_TABLE:

	return u32_err;
}

static U32 emmc_chk_timing_table_ex(U8 pad_type)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_chksum;

	memcpy((U8 *)&emmc_drv.t_table, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table));
	u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
	if (u32_chksum != emmc_drv.t_table.u32_chksum) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: ChkSum error, no Table\n");
		u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
		goto LABEL_END;
	}
	if (u32_chksum == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: no Table\n");
		u32_err = EMMC_ST_ERR_DDRT_NONA;
		goto LABEL_END;
	}
	//--------------------------------------
	//check if need auto-update
	if (emmc_drv.t_table.u32_ver_no != EMMC_TIMING_TABLE_VERSION) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: auto update Timing Table ...%Xh %Xh\n",
			   EMMC_TIMING_TABLE_VERSION, emmc_drv.t_table.u32_ver_no);
		u32_err = EMMC_ST_ERR_DDRT_NONA;
	}

LABEL_END:

	return u32_err;
}

static U32 emmc_chk_timing_gen_table_ex(U8 pad_type)
{
	U32 u32_err = EMMC_ST_SUCCESS, u32_chksum;

	u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table_g.u32_ver_no,
				  (sizeof(emmc_drv.t_table_g) - sizeof(U32)/*checksum*/));
	if (u32_chksum != emmc_drv.t_table_g.u32_chksum) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: ChkSum error, no Gen_TTable\n");
		u32_err = EMMC_ST_ERR_DDRT_CHKSUM;
		goto LABEL_END;
	}
	if (u32_chksum == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: no Gen_TTable\n");
		u32_err = EMMC_ST_ERR_DDRT_NONA;
		goto LABEL_END;
	}
	//--------------------------------------
	//check if need auto-update
	if (emmc_drv.t_table_g.u32_ver_no != EMMC_TIMING_TABLE_VERSION) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: auto update Gen_TTable ...%Xh %Xh\n",
			   EMMC_TIMING_TABLE_VERSION, emmc_drv.t_table_g.u32_ver_no);
		u32_err = EMMC_ST_ERR_DDRT_NONA;
	}

LABEL_END:

	return u32_err;
}

static U32 emmc_load_gen_timing_table_ex(U8 pad_type)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_chk_timing_table_ex(pad_type);
	if (u32_err != EMMC_ST_SUCCESS)
		goto LABEL_END_OF_NO_TABLE;

	#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) ||\
		(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400) ||\
		(defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)
		//read general table if hs400
		if (pad_type == FCIE_EMMC_HS400_5_1 || pad_type == FCIE_EMMC_HS400 ||
		    pad_type == FCIE_EMMC_HS200) {
			u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF, 1);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC WARN: load 1st Gen_TTable fail, %Xh\n", u32_err);

				u32_err = emmc_cmd18(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF, 1);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Warn: load 2nd Gen_TTables fail, %Xh\n", u32_err);
					goto LABEL_END_OF_NO_TABLE;
				}
			}
			memcpy((U8 *)&emmc_drv.t_table_g, GLOBAL_EMMC_SECTORBUF, sizeof(emmc_drv.t_table_g));
			if (FCIE_EMMC_HS400 == pad_type || FCIE_EMMC_HS400_5_1 == pad_type) {
				u32_err = emmc_chk_timing_gen_table_ex(pad_type);
				if (u32_err != EMMC_ST_SUCCESS)
					goto LABEL_END_OF_NO_TABLE;
			}
			if (memcmp(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1)) {
				emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: mismatch CID, need to rebuild TTable\n");
				u32_err = EMMC_ST_ERR_DDRT_NONA;
				goto LABEL_END_OF_NO_TABLE;
			}
			if (!EMMC_IF_DISABLE_PAD_CHECK()) {
				if (emmc_drv.t_table_g.u8_speed_mode != pad_type) {
					emmc_debug(EMMC_DEBUG_LEVEL, 1,
						   "eMMC Warn: mismatch speed, need to rebuild TTable\n");
					u32_err = EMMC_ST_ERR_DDRT_NONA;
					goto LABEL_END_OF_NO_TABLE;
				}
			}
			if (pad_type == FCIE_EMMC_HS400 || pad_type == FCIE_EMMC_HS400_5_1) {
				if (emmc_drv.host->id_matched_driving <= 4 &&
				    emmc_drv.host->id_matched_driving !=
				    emmc_drv.t_table_g.u32_device_driving) {
					emmc_debug(EMMC_DEBUG_LEVEL, 1,
						   "eMMC Warn: mismatch cust driving, need to rebuild TTable\n");
					u32_err = EMMC_ST_ERR_DDRT_NONA;
					goto LABEL_END_OF_NO_TABLE;
				}
				emmc_drv.host->id_matched_driving = emmc_drv.t_table_g.u32_device_driving;
			}
		}
	#endif
LABEL_END_OF_NO_TABLE:

	return u32_err;
}

U32 emmc_load_timing_table(U8 pad_type)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U32 u32_emmc_blk_addr;

	//--------------------------------------
	switch (pad_type) {
	#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) ||\
		(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
	case FCIE_EMMC_HS400_5_1:
			u32_emmc_blk_addr = EMMC_HS400TABLE_BLK_0;
	break;
	case FCIE_EMMC_HS400:
		u32_emmc_blk_addr = (emmc_drv.u8_emmc_pll_skew4 > 0) ?
			EMMC_HS400SKEW4TABLE_BLK_0 : EMMC_HS400TABLE_BLK_0;
	break;
	#endif
	#if defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200
	case FCIE_EMMC_HS200:
		u32_emmc_blk_addr = EMMC_HS200TABLE_BLK_0;
	break;
	#endif
	case FCIE_EMMC_DDR:
		u32_emmc_blk_addr = EMMC_DDRTABLE_BLK_0;
	break;
	default:
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: unknown pad type %Xh\n", pad_type);
	return EMMC_ST_ERR_DDRT_NONA;
	}

	//--------------------------------------
	u32_err = emmc_cmd18(u32_emmc_blk_addr, GLOBAL_EMMC_SECTORBUF, 1);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: load 1 Table fail, %Xh\n", u32_err);

		u32_err = emmc_cmd18(u32_emmc_blk_addr + 1, GLOBAL_EMMC_SECTORBUF, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: load 2 Tables fail, %Xh\n", u32_err);
			goto LABEL_END_OF_NO_TABLE;
		}
	}
	//--------------------------------------
	if (emmc_drv.u8_emmc_pll_skew4) {
		if (pad_type == FCIE_EMMC_HS400_5_1 || pad_type == FCIE_EMMC_HS400) {
			u32_err = emmc_load_hs400_timing_table_ex();
			if (u32_err != EMMC_ST_SUCCESS)
				goto LABEL_END_OF_NO_TABLE;
		} else if (pad_type == FCIE_EMMC_HS200) {
			u32_err = emmc_load_hs200_timing_table_ex();
			if (u32_err != EMMC_ST_SUCCESS)
				goto LABEL_END_OF_NO_TABLE;
		}
	} else {
		u32_err = emmc_load_gen_timing_table_ex(pad_type);
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_END_OF_NO_TABLE;
	}

	return EMMC_ST_SUCCESS;

LABEL_END_OF_NO_TABLE:
	emmc_drv.t_table.u8_set_cnt = 0;
	#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) || \
		(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
	emmc_drv.t_table_g.u8_set_cnt = 0;
	emmc_drv.t_table_g.u32_ver_no = 0;
	#endif

	if (emmc_drv.u8_emmc_pll_skew4 == 0 && emmc_drv.u8_ecsd184_stroe_support == 0) {
		emmc_drv.t_hs400_table.u8_set_cnt = 0;
		emmc_drv.t_hs400_table.u32_ver_no = 0;
	}

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: load Tables fail, %Xh\n", u32_err);
	return u32_err;
}


//set eMMC device & pad registers (no macro timing registers, since also involved in tuning procedure)
static U32 emmc_fcie_enable_fastmode_hs200_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_set_bus_width(8, 0);//disable DDR
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HS200 disable DDR fail: %Xh\n", u32_err);
		return u32_err;
	}
	u32_err = emmc_set_bus_speed(EMMC_SPEED_HS200);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: enable HS200 fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.t_table.u8_set_cnt == 0)
		emmc_clock_setting(gau8_emmc_pll_sel_200[0]);

	return u32_err;
}

static U32 emmc_fcie_enable_fastmode_hs400_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_set_bus_width(8, 1);//enable DDR
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HS400 enable DDR fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.t_hs400_table.u8_set_cnt) {
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
		emmc_drv.u8_ecsd185_hs_timing |=
			(emmc_drv.t_hs400_table.u32_device_driving &
			BITS_MSK_TIMING) << EXT_CSD_DRV_STR_SHIFT;
	} else if (emmc_drv.t_table_g.u8_set_cnt) {
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
		emmc_drv.u8_ecsd185_hs_timing |=
			(emmc_drv.t_table_g.u32_device_driving & BITS_MSK_TIMING) << EXT_CSD_DRV_STR_SHIFT;
	}
	if (emmc_drv.u32_last_err_code & DRV_FLAG_WEAK_STRENGTH) {
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
		emmc_drv.u8_ecsd185_hs_timing |= BITS_WEAK_DRIVER_STRENGTH << EXT_CSD_DRV_STR_SHIFT;
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 1, "weakest driving strength for tuning as 0x%X\n",
			   emmc_drv.u8_ecsd185_hs_timing);
	}

	u32_err = emmc_set_bus_speed(EMMC_SPEED_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: enable HS400 fail: %Xh\n", u32_err);

		return u32_err;
	}

	if (emmc_drv.t_table.u8_set_cnt == 0 && emmc_drv.t_hs400_table.u8_set_cnt == 0)
		emmc_clock_setting(gau8_emmc_pll_sel_200[0]);

	return u32_err;
}

static U32 emmc_fcie_enable_fastmode_hs400_5_1_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_set_bus_width(8, 2);//enable DDR
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HS400 5.1 enable DDR fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.t_table_g.u8_set_cnt) {
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
		emmc_drv.u8_ecsd185_hs_timing |=
		(emmc_drv.t_table_g.u32_device_driving & BITS_MSK_TIMING) << EXT_CSD_DRV_STR_SHIFT;
	}

	if (emmc_drv.u32_last_err_code & DRV_FLAG_WEAK_STRENGTH) {
		emmc_drv.u8_ecsd185_hs_timing &= ~BITS_MSK_DRIVER_STRENGTH;
		emmc_drv.u8_ecsd185_hs_timing |= BITS_WEAK_DRIVER_STRENGTH << EXT_CSD_DRV_STR_SHIFT;
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 1, "weakest driving strength for tuning as 0x%X\n",
			   emmc_drv.u8_ecsd185_hs_timing);
	}

	u32_err = emmc_set_bus_speed(EMMC_SPEED_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: enable HS400 5.1 fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.t_table.u8_set_cnt == 0)
		emmc_clock_setting(gau8_emmc_pll_sel_200[0]);

	return u32_err;
}

U32 emmc_fcie_enable_fastmode_ex(U8 pad_type)
{
	U32 u32_err = EMMC_ST_SUCCESS;
	U8  u8_cur_set_idx = 0;

	switch (pad_type) {
	case FCIE_EMMC_DDR:
		u32_err = emmc_set_bus_speed(EMMC_SPEED_HIGH);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: enable HighSpeed fail: %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_set_bus_width(8, 1);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: enable DDR fail: %Xh\n", u32_err);
			return u32_err;
		}

		if (emmc_drv.t_table.u8_set_cnt == 0)
			emmc_clock_setting(gau8_emmc_pll_sel_52[0]);

	break;

	#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200) || \
		(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
	case FCIE_EMMC_HS200:
		u32_err = emmc_fcie_enable_fastmode_hs200_ex();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	break;
	#endif

	#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
	case FCIE_EMMC_HS400:
		u32_err = emmc_fcie_enable_fastmode_hs400_ex();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	break;
	#endif
	#if defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1
	case FCIE_EMMC_HS400_5_1:
		u32_err = emmc_fcie_enable_fastmode_hs400_5_1_ex();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	break;
	#endif
	}

	//--------------------------------------
	emmc_pads_switch(pad_type);

	if (emmc_drv.u8_emmc_pll_skew4) {
		#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) || \
			(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)

		if (emmc_drv.t_table_g.u8_set_cnt && pad_type == FCIE_EMMC_HS400_5_1)
			u8_cur_set_idx = emmc_drv.t_table_g.u8_cur_set_idx;
		else
		#endif
			u8_cur_set_idx = EMMC_TIMING_SET_MAX;
	} else {
		#if (defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1) || \
			(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)
			if (emmc_drv.t_table_g.u8_set_cnt &&
			    (pad_type == FCIE_EMMC_HS400 || pad_type == FCIE_EMMC_HS400_5_1))
				u8_cur_set_idx = emmc_drv.t_table_g.u8_cur_set_idx;
			else
			#endif
				u8_cur_set_idx = EMMC_TIMING_SET_MAX;
	}

	emmc_fcie_apply_timing_set(u8_cur_set_idx);

	return u32_err;
}

U32 emmc_fcie_enable_fast_mode(U8 pad_type)
{
	U32 u32_err;

	if (pad_type == emmc_drv.pad_type)
		return EMMC_ST_SUCCESS;

	u32_err = emmc_load_timing_table(pad_type);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: no Timing Table, %Xh\n", u32_err);
		return u32_err;
	}
	u32_err = emmc_fcie_enable_fastmode_ex(pad_type);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: EnableDDRMode_Ex fail, %Xh\n", u32_err);
		emmc_die("");
		return u32_err;
	}

	return EMMC_ST_SUCCESS;
}

//if failed, must be board issue.
U32 emmc_fcie_choose_hs400_5_1_mode_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	if (emmc_get_r1() & EMMC_R1_DEVICE_IS_LOCKED) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\neMMC Warn: HS400 5.1, but locked\n");
		return u32_err;
	}

	if (emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1) != EMMC_ST_SUCCESS) {
		if (emmc_drv.u8_emmc_pll_skew4) {
			u32_err = emmc_fcie_detect_hs400_5_1_skew4_timing();
		} else {
			//HS400 use DLL setting for DAT[0-7]
			u32_err = emmc_fcie_detect_hs400_5_1_timing();
		}
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}

	emmc_debug(0, 0, "\neMMC: HS400 5.1 %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);

	return u32_err;
}

static U32 emmc_fcie_choose_hs400_mode_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;
	if (emmc_get_r1() & EMMC_R1_DEVICE_IS_LOCKED) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\neMMC Warn: HS400, but locked\n");
		return u32_err;
	}

	if (emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400) != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		if (emmc_drv.u8_emmc_pll_skew4) {
			u32_err = emmc_fcie_detect_hs400_skew4_timing();
		} else {
			//HS400 use DLL setting for DAT[0-7]
			u32_err = emmc_fcie_detect_hs400_timing();
		}

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	emmc_debug(0, 0, "\neMMC: HS400 %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);

	return u32_err;
}

static U32 emmc_fcie_choose_hs200_mode_ex(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	u32_err = emmc_cmd13(emmc_drv.u16_rca);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	if (emmc_get_r1() & EMMC_R1_DEVICE_IS_LOCKED) {
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "\neMMC Warn: HS200, but locked\n");
		return u32_err;
	}

	if (emmc_fcie_enable_fast_mode(FCIE_EMMC_HS200) != EMMC_ST_SUCCESS)
		u32_err = emmc_fcie_build_hS200_timing_table();

	emmc_debug(0, 0, "\neMMC: HS200 %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);
	return u32_err;
}

U32 emmc_fcie_choose_speed_mode(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	#if defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1

	if ((emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS400_1_8V) && emmc_drv.u8_ecsd184_stroe_support) {
		u32_err = emmc_fcie_choose_hs400_5_1_mode_ex();
		return u32_err;
	}
	#endif

	#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
	if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS400_1_8V) {
		u32_err = emmc_fcie_choose_hs400_mode_ex();
		return u32_err;
	}
	#endif

	#if defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200
	if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_HS200_1_8V) {
		u32_err = emmc_fcie_choose_hs200_mode_ex();
		return u32_err;
	}
	#endif

	#if (defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP)
	if (emmc_drv.u8_ecsd196_dev_type & EMMC_DEVTYPE_DDR) {
		if (emmc_fcie_enable_fast_mode(FCIE_EMMC_DDR) != EMMC_ST_SUCCESS)
			u32_err = emmc_fcie_build_ddr_timing_table();

		emmc_debug(0, 0, "\neMMC: DDR %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);
		return u32_err;
	}
	#endif

	emmc_debug(0, 0, "\neMMC: SDR %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);
	return u32_err;
}

#if defined(EMMC_SKIP_IDENTIFY) && EMMC_SKIP_IDENTIFY
U32 emmc_fcie_check_fast_mode(void)
{
	U32 u32_ret = 0;

	if (REG_FCIE(FCIE_DDR_MODE) & BIT_32BIT_MACRO_EN) {
		REG_FCIE_W(FCIE_RESERVED_FOR_SW, EMMC_DRIVER_VERSION);
		//emmc_debug(0, 1 ,"fast mode: %Xh\n", sizeof(emmc_drv));
		u32_ret = 1;
	} else {
		u32_ret = 0;
	}

	return u32_ret;
}

U32 emmc_fcie_check_cid(void)
{
	U32 u32_err = 0;
	U8 u8_cid[EMMC_R2_BYTE_CNT];
	U8 u8_retry = 0;

	memset(u8_cid, 0, EMMC_R2_BYTE_CNT);

LABEL_CHK_CID_START:

	emmc_cmd3_cmd7(0, 7);//Disconnect

	if (u8_retry > 10)
		goto LABEL_CHK_CID_END;

	if (u8_retry)
		emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Warn: retry: %u\n", u8_retry);

	u32_err = emmc_cmd10(emmc_drv.u16_rca, u8_cid);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_fcie_err_handler_reinit();
		u8_retry++;
		goto LABEL_CHK_CID_START;
	}

	if (memcmp(emmc_drv.au8_cid, u8_cid, EMMC_R2_BYTE_CNT))
		u32_err = EMMC_ST_ERR_NO_CIS;

LABEL_CHK_CID_END:
	emmc_cmd3_cmd7(emmc_drv.u16_rca, 7);

	return u32_err;
}


#endif

