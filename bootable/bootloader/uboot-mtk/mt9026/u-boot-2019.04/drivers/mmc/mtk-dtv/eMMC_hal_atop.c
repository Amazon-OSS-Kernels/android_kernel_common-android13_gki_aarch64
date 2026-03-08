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

#include "eMMC.h"
#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

U32 gu32_emmc_drv_ext_flag = EMMC_DRV_EXT_INIT_FLAG;

U32 emmc_hw_timer_delay(U32 u32us)
{
	U32 u32_i = u32us;

	while (u32_i > 1000) {
		udelay(1000);
		u32_i -= 1000;
	}
	udelay(u32_i);

	return u32us;
}

U32 emmc_hw_timer_sleep(U32 u32ms)
{
	U32 u32_us = u32ms * 1000;

	while (u32_us > 1000) {
		emmc_hw_timer_delay(1000);
		u32_us -= 1000;
	}

	emmc_hw_timer_delay(u32_us);
	return u32ms;
}

U32 emmc_hw_timer_start(void)
{
	#if 0
	//Reset PIU Timer1
	REG_FCIE_W(TIMER1_MAX_LOW, 0xFFFF);
	REG_FCIE_W(TIMER1_MAX_HIGH, 0xFFFF);
	REG_FCIE_W(TIMER1_ENABLE, 0);

	//Start PIU Timer1
	REG_FCIE_SETBIT(TIMER1_ENABLE, 0x0001);
	#endif
	return 0;
}

U64 emmc_hw_timer_tick(void)
{
	#if 0
	U32 u32_hw_timer = 0;
	U16 u16_timer_low = 0;
	U16 u16_timer_high = 0;

	REG_FCIE_R(TIMER1_CAP_LOW, u16_timer_low);
	REG_FCIE_R(TIMER1_CAP_HIGH, u16_timer_high);

	u32_hw_timer = (u16_timer_high << 16) | u16_timer_low;

	return u32_hw_timer;
	#else
	return get_ticks();
	#endif
}

//------------------------------------
void emmc_set_watch_dog(U8 u8_if_enable)
{
//do nothing
}

void emmc_reset_watch_dog(void)
{
//do nothing
}

//--------------------------------

unsigned long emmc_translate_dma_address_ex(unsigned long ulong_dma_addr, U32 u32_byte_cnt)
{
	flush_cache(ulong_dma_addr, u32_byte_cnt);
	#if defined(CONFIG_MIPS32)

	chip_flush_memory();
	return  (ulong_dma_addr & 0x1fffffff);

	#elif CONFIG_ARM

	//emmc_debug(0,0,"MIU[%Xh], SIZE[%Xh]\n", CONFIG_BUSADDR_START, CONFIG_BUSADDR_SIZE);

	REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_SELECT_MASK);

	#if !(defined(FCIE_NO_MIU_SELECT) && FCIE_NO_MIU_SELECT)
	//Check patched
	if ((ms_os_va2pa(ulong_dma_addr) & CONFIG_SYS_MIU_INTERVAL) == CONFIG_SYS_MIU_INTERVAL) {
		//printf("MIU1 ");

		REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU1_SELECT);

		return (ms_os_va2pa(ulong_dma_addr) - CONFIG_SYS_MIU_INTERVAL);
	}
	#endif
		return virt_to_phys((void *)ulong_dma_addr) - CONFIG_BUSADDR_START;
	//TODO: ask namo how to handle MIU2
	#else
	return virt_to_phys((void *)ulong_dma_addr) - CONFIG_BUSADDR_START;
	#endif
}

void emmc_flush_data_cache_buffer(unsigned long ulong_dma_addr, U32 u32_byte_cnt)
{
	//flush_cache(ulong_dma_addr, u32_byte_cnt);
}

void emmc_invalidate_data_cache_buffer(unsigned long ulong_dma_addr, U32 u32_byte_cnt)
{
	invalidate_dcache_range(ulong_dma_addr, (ulong_dma_addr + (unsigned long)u32_byte_cnt));
}

void emmc_flush_miu_pipe(void)
{
}

void emmc_check_power_cut(void)
{
}

//--------------------------------
#if defined(ENABLE_EMMC_ATOP) && ENABLE_EMMC_ATOP
extern void emmc_fcie_set_atop_timing_reg(U8 u8_set_idx);

U8 gau8_emmc_pll_sel_52[EMMC_FCIE_VALID_CLK_CNT] = {
	EMMC_PLL_CLK_200M,
	EMMC_PLL_CLK_160M,
	EMMC_PLL_CLK_120M
};

U8 gau8_emmc_pll_sel_200[EMMC_FCIE_VALID_CLK_CNT] = {
	EMMC_PLL_CLK_200M,
	EMMC_PLL_CLK_160M,
	EMMC_PLL_CLK_140M
};

static struct emmc_fcie_atop_set g_tab_set_tmp_t;
static struct emmc_fcie_atop_set_skew4 g_tab_set_skew4_tmp_t;

static U32 emmc_fcie_detect_ddr_timing_ex(U8 u8_dqs, U8 u8_delay_sel)
{
	U32 u32_sector_addr;

	u32_sector_addr = EMMC_TEST_BLK_0;
	emmc_drv.t_table.set[0].u8_skew4 = u8_dqs;
	emmc_drv.t_table.set[0].u8_cell = u8_delay_sel;

	emmc_fcie_set_atop_timing_reg(0);
	return emmc_test_blk_wrc_probe_timing(u32_sector_addr);
}

#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200) || \
(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)

void emmc_fcie_detect_timing_not_wrapped_ex(U32 u32_skew4_result, U16 *start_bit, U16 *end_bit)
{
	U16 u16_i;
	U16 u16_start_bit = 0, u16_end_bit = 0;

	u16_start_bit = *start_bit;
	u16_end_bit = *end_bit;

	for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
		if ((u32_skew4_result & (1 << u16_i)) && u16_start_bit ==  START_BIT_DEFAULT)
			u16_start_bit = u16_i;
		else if (u16_start_bit !=  START_BIT_DEFAULT  && (u32_skew4_result & (1 << u16_i)) == 0)
			u16_end_bit = u16_i - 1;

		if (u16_end_bit != END_BIT_DEFAULT) {
			if (u16_end_bit - u16_start_bit + 1 < MIN_OK_SKEW_CNT) {
				u16_start_bit = START_BIT_DEFAULT;
				u16_end_bit = END_BIT_DEFAULT;
			} else {
				break;
			}
		}
	}
	if (u16_end_bit == END_BIT_DEFAULT) {
		if (u32_skew4_result & (1 << (u16_i - 1)))
			u16_end_bit = u16_i - 1;
		else
			u16_end_bit = u16_start_bit;
	}

	*start_bit = u16_start_bit;
	*end_bit = u16_end_bit;
}

static void emmc_fcie_detect_timing_wrapped_ex(U32 u32_skew4_result, U16 *start_bit, U16 *end_bit, U16 *cnt)
{
	U16 u16_i;
	U16 u16_start_bit = 0, u16_end_bit = 0, u16_cnt = 0;

	u16_start_bit = *start_bit;
	u16_end_bit = *end_bit;
	u16_cnt =  *cnt;

	for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
		if (u32_skew4_result & (1 << u16_i)) {
			u16_cnt += 1;
		} else {
			u16_start_bit = u16_i - 1;
			break;
		}
	}
	for (u16_i = TOTAL_PLL_SKEW4_CNT - 1; u16_i > 0; u16_i--) {
		if (u32_skew4_result & (1 << u16_i)) {
			u16_cnt += 1;
		} else {
			u16_end_bit = u16_i + 1;
			break;
		}
	}

	*start_bit = u16_start_bit;
	*end_bit = u16_end_bit;
	*cnt = u16_cnt;
}

U32 emmc_fcie_detect_timing_ex(U32 u32_skew4_result)
{
	U16 u16_i, u16_start_bit = START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT, u16_cnt;

	g_tab_set_tmp_t.u32_scan_result = u32_skew4_result;

	if ((u32_skew4_result & ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) == ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) {
		//emmc_debug(EMMC_DEBUG_LEVEL,0,"  case1: all\n");
		g_tab_set_tmp_t.u8_reg2ch = 1;
		g_tab_set_tmp_t.u8_skew4 = 0;
	} else if ((u32_skew4_result & BIT0) == 0 || (u32_skew4_result & (1 << (TOTAL_PLL_SKEW4_CNT - 1))) == 0) {
			emmc_fcie_detect_timing_not_wrapped_ex(u32_skew4_result, &u16_start_bit, &u16_end_bit);
			if (u16_end_bit - u16_start_bit + 1 < MIN_OK_SKEW_CNT) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: Clk:%uMHz Case2 not enough skew4: %Xh %Xh\n",
					   emmc_drv.u32_clk_khz / EMMC_KHZ,
					   u32_skew4_result, u16_end_bit - u16_start_bit + 1);
				return EMMC_ST_ERR_SKEW4;
			}

			if (((u16_start_bit + u16_end_bit) / 2) < PLL_SKEW4_CNT) {
				g_tab_set_tmp_t.u8_reg2ch = 0;
				g_tab_set_tmp_t.u8_skew4 = (u16_start_bit + u16_end_bit) / 2;
			} else {
				g_tab_set_tmp_t.u8_reg2ch = 1;
				g_tab_set_tmp_t.u8_skew4 = (u16_start_bit + u16_end_bit) / 2 - PLL_SKEW4_CNT;
			}
	} else {
		//to ignore "discontinuous case"
		u16_cnt = 0;
		emmc_fcie_detect_timing_wrapped_ex(u32_skew4_result, &u16_start_bit, &u16_end_bit, &u16_cnt);
		if (u16_cnt < MIN_OK_SKEW_CNT) {
			u32_skew4_result &= ~((1 << (u16_start_bit + 1)) - 1);
			return emmc_fcie_detect_timing_ex(u32_skew4_result);
		}
		for (u16_i = u16_start_bit + 1; u16_i < u16_end_bit; u16_i++)
			u32_skew4_result &= ~(1 << u16_i);

		u16_start_bit = START_BIT_DEFAULT;
		u16_end_bit = END_BIT_DEFAULT;

		//--------------------------------
		//normal judgement
		for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
			if ((u32_skew4_result & (1 << u16_i)) == 0 && u16_start_bit ==  START_BIT_DEFAULT)
				u16_start_bit = u16_i - 1;
			else if (u16_start_bit !=  START_BIT_DEFAULT  && (u32_skew4_result & (1 << u16_i)))
				u16_end_bit = u16_i;

			if (u16_end_bit != END_BIT_DEFAULT)
				break;
		}

		if ((u16_start_bit + 1) > TOTAL_PLL_SKEW4_CNT - u16_end_bit) {
			g_tab_set_tmp_t.u8_reg2ch = 0;
			g_tab_set_tmp_t.u8_skew4 =
				(u16_start_bit - (TOTAL_PLL_SKEW4_CNT - u16_end_bit)) / 2;
		} else {
			g_tab_set_tmp_t.u8_reg2ch = 1;
			g_tab_set_tmp_t.u8_skew4 = u16_end_bit +
				(TOTAL_PLL_SKEW4_CNT - u16_end_bit + u16_start_bit) / 2 -
				PLL_SKEW4_CNT;
		}
	}

	if (g_tab_set_tmp_t.u8_skew4 >= PLL_SKEW4_CNT)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Skew4:%u > %u\n",
			   g_tab_set_tmp_t.u8_skew4, PLL_SKEW4_CNT);

	return EMMC_ST_SUCCESS;
}
#endif

#define FCIE_DELAY_CELL_TS         300 //0.3ns
static struct emmc_fcie_ddr_tab_window sg_ddr_tab_window[2];

U32 emmc_fcie_detect_ddr_timing(void)
{
	U8  u8_dqs, u8_delay_sel, u8_i;
	U8  u8_dqs_prev = 0xFF, u8_delay_sel_prev = 0;
	U8  au8_dqs_reg_val[8] = {0, 1, 2, 3, 4, 5, 6, 7};//0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  au8_dqs_10t[8] = {0, 5, 10, 15, 20, 25, 30, 35};//0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3.5T
	U8  u8_delay_sel_max;
	U32 u32_ts;
	U32 u32_err;
	struct emmc_fcie_ddr_tab_window *p_window = &sg_ddr_tab_window[0];

	emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//just to turn off some log

//--------------------------------------------------
	sg_ddr_tab_window[0].u8_cnt = 0;
	sg_ddr_tab_window[0].aparam[0].u8_dqs = 0;
	sg_ddr_tab_window[0].aparam[1].u8_dqs = 0;
	sg_ddr_tab_window[1].u8_cnt = 0;
	sg_ddr_tab_window[1].aparam[0].u8_dqs = 0;
	sg_ddr_tab_window[1].aparam[1].u8_dqs = 0;
	for (u8_i = 1; u8_i <= BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT; u8_i++) {
		sg_ddr_tab_window[0].au8_dqs_valid_cell_cnt[u8_i] = 0;
		sg_ddr_tab_window[1].au8_dqs_valid_cell_cnt[u8_i] = 0;
	}

	//--------------------------------------------------
	//calculate delay_Sel_max
	u32_ts = (1000 * 1000 * 1000) / emmc_drv.u32_clk_khz;
	u32_ts >>= 2;//for 4X's 1T

	//--------------------------------------------------
	//no need to try DQS of no delay
	for (u8_dqs = 1; u8_dqs <= (BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT); u8_dqs++) {
		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "\n---------------------------\n");

		if (u8_dqs < (BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT))
			u8_delay_sel_max =
				(au8_dqs_10t[u8_dqs + 1] - au8_dqs_10t[u8_dqs])
				* u32_ts / (FCIE_DELAY_CELL_TS * 10);
		else
			u8_delay_sel_max = (BIT_DQS_DELAY_CELL_MASK >> BIT_DQS_DELAY_CELL_SHIFT);

		if (u8_delay_sel_max >
				(BIT_DQS_DELAY_CELL_MASK >> BIT_DQS_DELAY_CELL_SHIFT)) {
			u8_delay_sel_max = (BIT_DQS_DELAY_CELL_MASK >> BIT_DQS_DELAY_CELL_SHIFT);
			emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "max delay cell: %u\n", u8_delay_sel_max);
			//emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "invalid, not try\n\n");
			//continue;
		} else {
			emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "max delay cell: %u\n\n", u8_delay_sel_max);
		}
		sg_ddr_tab_window[0].au8_dqs_try_cell_cnt[u8_dqs] = u8_delay_sel_max;
		sg_ddr_tab_window[1].au8_dqs_try_cell_cnt[u8_dqs] = u8_delay_sel_max;

		for (u8_delay_sel = 0; u8_delay_sel <= u8_delay_sel_max; u8_delay_sel++) {
			u32_err = emmc_fcie_detect_ddr_timing_ex(au8_dqs_reg_val[u8_dqs], u8_delay_sel);
			if (u32_err == EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "  PASS\n\n");
				p_window->u8_cnt++;
				if (p_window->aparam[0].u8_dqs == 0) {
					p_window->aparam[0].u8_dqs = u8_dqs;//dqs uses index
					p_window->aparam[0].u8_cell = u8_delay_sel;
				}
				p_window->au8_dqs_valid_cell_cnt[u8_dqs]++;
				u8_dqs_prev = u8_dqs;
				u8_delay_sel_prev = u8_delay_sel;
			} else {
				if (u8_dqs_prev != 0xFF) {
					p_window->aparam[1].u8_dqs = u8_dqs_prev;//dqs uses index
					p_window->aparam[1].u8_cell = u8_delay_sel_prev;
				}
				u8_dqs_prev = 0xFF;
				//discard & re-use the window having less PASS cnt
				p_window =
					(sg_ddr_tab_window[0].u8_cnt < sg_ddr_tab_window[1].u8_cnt) ?
					&sg_ddr_tab_window[0] : &sg_ddr_tab_window[1];
				p_window->u8_cnt = 0;
				p_window->aparam[0].u8_dqs = 0;
				p_window->aparam[1].u8_dqs = 0;
				for (u8_i = 1; u8_i <= BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT; u8_i++)
					p_window->au8_dqs_valid_cell_cnt[u8_i] = 0;
			}
		}
	}

	//for the case of last try is ok
	if (u8_dqs_prev != 0xFF) {
		p_window->aparam[1].u8_dqs = u8_dqs_prev;//dqs uses index
		p_window->aparam[1].u8_cell = u8_delay_sel_prev;
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "\nW0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		   sg_ddr_tab_window[0].u8_cnt,
		   sg_ddr_tab_window[0].aparam[0].u8_dqs, sg_ddr_tab_window[0].aparam[0].u8_cell,
		   sg_ddr_tab_window[0].aparam[1].u8_dqs, sg_ddr_tab_window[0].aparam[1].u8_cell);
	for (u8_i = 1; u8_i <= BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT; u8_i++)
		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "DQSValidCellCnt[%u]:%u\n",
			   u8_i, sg_ddr_tab_window[0].au8_dqs_valid_cell_cnt[u8_i]);

	emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "\nW1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
		   sg_ddr_tab_window[1].u8_cnt,
		   sg_ddr_tab_window[1].aparam[0].u8_dqs, sg_ddr_tab_window[1].aparam[0].u8_cell,
		   sg_ddr_tab_window[1].aparam[1].u8_dqs, sg_ddr_tab_window[1].aparam[1].u8_cell);
	for (u8_i = 1; u8_i <= BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT; u8_i++)
		emmc_debug(EMMC_DEBUG_LEVEL_HIGH, 0, "DQSValidCellCnt[%u]:%u\n",
			   u8_i, sg_ddr_tab_window[1].au8_dqs_valid_cell_cnt[u8_i]);

	if (sg_ddr_tab_window[0].u8_cnt || sg_ddr_tab_window[1].u8_cnt)
		u32_err = EMMC_ST_SUCCESS;
	else
		u32_err = EMMC_ST_ERR_NO_OK_DDR_PARAM;
	return u32_err;
}

void emmc_fcie_get_max_valid_cell(U8 *au8_dqs_reg_val)
{
	U8 u8_i, u8_set_idx = 0, u8_tmp, u8_dqs_idx = 0, u8_cell_base;
	struct emmc_fcie_ddr_tab_window *p_window = 0;

	//pick up the Window of Cell=0 case
	if (sg_ddr_tab_window[0].aparam[0].u8_dqs != sg_ddr_tab_window[0].aparam[1].u8_dqs &&
	    sg_ddr_tab_window[1].aparam[0].u8_dqs == sg_ddr_tab_window[1].aparam[1].u8_dqs)
		p_window = &sg_ddr_tab_window[0];
	else if (sg_ddr_tab_window[0].aparam[0].u8_dqs == sg_ddr_tab_window[0].aparam[1].u8_dqs &&
		 sg_ddr_tab_window[1].aparam[0].u8_dqs != sg_ddr_tab_window[1].aparam[1].u8_dqs)
		p_window = &sg_ddr_tab_window[1];
	else if (sg_ddr_tab_window[0].aparam[0].u8_dqs != sg_ddr_tab_window[0].aparam[1].u8_dqs &&
		 sg_ddr_tab_window[1].aparam[0].u8_dqs != sg_ddr_tab_window[1].aparam[1].u8_dqs)
		p_window =
			(sg_ddr_tab_window[0].u8_cnt > sg_ddr_tab_window[1].u8_cnt) ?
			&sg_ddr_tab_window[0] : &sg_ddr_tab_window[1];

	//---------------------------
	if (p_window != 0) {
		//pick up the DQS having max valid cell
		u8_tmp = 0;
		for (u8_i = 1; u8_i <= BIT_DQS_MODE_MASK >> BIT_DQS_MDOE_SHIFT; u8_i++) {
			if (u8_tmp <= p_window->au8_dqs_valid_cell_cnt[u8_i]) {
				u8_tmp = p_window->au8_dqs_valid_cell_cnt[u8_i];
				u8_dqs_idx = u8_i;
			}
		}
		if (u8_dqs_idx != 0) {
			if (p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1] >= 7 &&
			    p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx] >= 7) {
				emmc_drv.t_table.set[u8_set_idx].u8_skew4 = au8_dqs_reg_val[u8_dqs_idx];
				emmc_drv.t_table.set[u8_set_idx].u8_cell = 0;
			} else {
				u8_tmp = (p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx] +
				p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1]) / 2;

				if (u8_tmp < p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1]) {
						emmc_drv.t_table.set[u8_set_idx].u8_skew4 =
					au8_dqs_reg_val[u8_dqs_idx - 1];

					u8_cell_base = p_window->au8_dqs_try_cell_cnt[u8_dqs_idx - 1] -
						p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1];
					emmc_drv.t_table.set[u8_set_idx].u8_cell =
						u8_cell_base + p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1] +
						p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx] - u8_tmp;
				} else {
					emmc_drv.t_table.set[u8_set_idx].u8_skew4 =
						au8_dqs_reg_val[u8_dqs_idx];
					emmc_drv.t_table.set[u8_set_idx].u8_cell =
						(p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx - 1] +
						p_window->au8_dqs_valid_cell_cnt[u8_dqs_idx]) / 2;
				}
			}
		}
	} else {
		p_window =
			(sg_ddr_tab_window[0].u8_cnt > sg_ddr_tab_window[1].u8_cnt) ?
			&sg_ddr_tab_window[0] : &sg_ddr_tab_window[1];

		emmc_drv.t_table.set[u8_set_idx].u8_skew4 = au8_dqs_reg_val[p_window->aparam[0].u8_dqs];
		emmc_drv.t_table.set[u8_set_idx].u8_cell =
			(p_window->aparam[0].u8_cell + p_window->aparam[1].u8_cell) / 2;
	}
}

U32 emmc_fcie_build_ddr_timing_table(void)
{
	U8  au8_dqs_reg_val[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	U8  u8_clk_idx, u8_set_idx;
	U32 u32_err, u32_ret;

	emmc_debug(EMMC_DEBUG_LEVEL, 1, "eMMC Info: building DDR table, please wait...\n");
	memset((void *)&emmc_drv.t_table, '\0', sizeof(emmc_drv.t_table));
	u8_set_idx = 0;

	if (!EMMC_IF_NORMAL_SDR()) {
		emmc_drv.u32_drvflag = 0;
		emmc_platform_init();
		u32_err = emmc_fcie_init();
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_fcie_init fail, %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "eMMC Err: emmc_init_device_ex fail: %X\n", u32_err);
			return u32_err;
		}
	}

	//emmc_debug(EMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: SDR mode can't detect DDR timing\n");
	u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_DDR);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(0, 0, "eMMC Err: set DDR IF fail: %X\n", u32_err);
		return u32_err;
	}

	for (u8_clk_idx = 0; u8_clk_idx < EMMC_FCIE_VALID_CLK_CNT; u8_clk_idx++) {
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "=================================\n");
		emmc_clock_setting(gau8_emmc_pll_sel_52[u8_clk_idx]);
		emmc_debug(EMMC_DEBUG_LEVEL_LOW, 0, "=================================\n");

		//---------------------------
		//search and set the Windows
		u32_err = emmc_fcie_detect_ddr_timing();

		//---------------------------
		//set the Table
		if (u32_err == EMMC_ST_SUCCESS) {
			emmc_drv.t_table.u8_set_cnt++;
			emmc_drv.t_table.set[u8_set_idx].u8_clk =
				gau8_emmc_pll_sel_52[u8_clk_idx];
			//---------------------------
			//select Window
			emmc_fcie_get_max_valid_cell(&au8_dqs_reg_val[0]);

			emmc_drv.t_table.u32_ver_no = EMMC_TIMING_TABLE_VERSION;
			emmc_dump_timing_table();
			emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
			emmc_drv.t_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
			memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table));

			emmc_fcie_apply_timing_set(EMMC_TIMING_SET_MAX);

			u32_err = emmc_cmd24(EMMC_DDRTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
			u32_ret = emmc_cmd24(EMMC_DDRTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
			if (u32_err != EMMC_ST_SUCCESS  && u32_ret != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
					   u32_err, u32_ret);
				return EMMC_ST_ERR_SAVE_DDRT_FAIL;
			}
			return EMMC_ST_SUCCESS;
		}
	}

	emmc_dump_timing_table();
	emmc_die();
	return EMMC_ST_ERR_SKEW4;
}

#define SKEW2_CNT         9
#define RXDLL_CNT         16
#define TRIGGER_CNT       8

#define TIGER_LVL_MASK     ((7 << 6) | (7 << 3) | 7)

U32 emmc_fcie_reset_to_legacy(void)
{
	U32 u32_err = 0;

	if (!EMMC_IF_NORMAL_SDR()) {
		emmc_drv.u32_drvflag = 0;
		emmc_platform_init();
		u32_err = emmc_fcie_init();
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_init fail, %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
			return u32_err;
		}
		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "eMMC Err: emmc_init_device_ex fail: %X\n", u32_err);
			emmc_fcie_err_handler_stop();
			return u32_err;
		}
	}
	emmc_fcie_err_handler_stop();

	return u32_err;
}

#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200)
U32 emmc_fcie_reset_to_hs200(U8 u8_clk_param)
{
	U32 u32_err;

	//--------------------------------
	//reset FCIE & eMMC to normal SDR mode
	if (!EMMC_IF_NORMAL_SDR()) {
			emmc_drv.u32_drvflag = 0;
			emmc_platform_init();
			u32_err = emmc_fcie_init();
			if (u32_err) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_init fail, %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "eMMC Err: emmc_init_device_ex fail: %X\n", u32_err);
			return u32_err;
		}
	}

	//--------------------------------
	//set eMMC to HS200 mode
	u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS200);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: set HS200 IF fail: %X\n", u32_err);

		return u32_err;
	}

	emmc_clock_setting(u8_clk_param);
	return u32_err;
}

U32 emmc_fcie_detect_hs200_timing(void)
{
	U32 u32_i, u32_err, u32_skew4_result = 0;
	U16 u16_skew_cnt = 0;

	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; //turn on turning flag

		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
			REG_FCIE_CLRBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
					emmc_drv.host->dev_comp->analog_skew4_reg_bit);
			//REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);

		u32_err = emmc_cmd21();
		if (u32_err == EMMC_ST_SUCCESS  &&
		    emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u16_skew_cnt++;
			u32_skew4_result |= (1 << u32_i);
		} else {
			emmc_debug(0, 1, "failed skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs200((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}

	for (u32_i = PLL_SKEW4_CNT; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
			REG_FCIE_SETBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
					emmc_drv.host->dev_comp->analog_skew4_reg_bit);

		//REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);

		u32_err = emmc_cmd21();
		if (u32_err == EMMC_ST_SUCCESS  &&
		    emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u16_skew_cnt++;
			u32_skew4_result |= (1 << u32_i);
		} else {
			emmc_debug(0, 1, "failed skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs200((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	if (u32_skew4_result == 0 || u16_skew_cnt < MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Clk: %uMHz SkewResult: %Xh SkewCnt: %u\n",
			   emmc_drv.u32_clk_khz / EMMC_KHZ, u32_skew4_result, u16_skew_cnt);
		return EMMC_ST_ERR_SKEW4;
	}

	return emmc_fcie_detect_timing_ex(u32_skew4_result);
}

U32 emmc_fcie_build_hS200_timing_table(void)
{
	U32 u32_err, u32_ret, u32_i;

	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Info: building HS200 table, please wait...\n");
	memset((void *)&emmc_drv.t_table, 0, sizeof(emmc_drv.t_table));
	memset((void *)&emmc_drv.t_table_g, 0, sizeof(emmc_drv.t_table_g));
	//general table setting for HS200
	if (emmc_drv.u32_drvflag & DRV_FLAG_SPEED_HS400_DETECT)
		emmc_drv.t_table_g.u8_speed_mode = FCIE_EMMC_HS400;
	else
		emmc_drv.t_table_g.u8_speed_mode = FCIE_EMMC_HS200;

	//--------------------------------
	//tuning FCIE & macro for HS200
	u32_err = emmc_fcie_reset_to_hs200(gau8_emmc_pll_sel_200[0]);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs200 fail: %Xh\n", u32_err);
		emmc_fcie_reset_to_legacy();
		return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nclk: %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);

	//---------------------------
	//search and set the Windows
	u32_err = emmc_fcie_detect_hs200_timing();
	//---------------------------
	//set the Table
	if (u32_err == EMMC_ST_SUCCESS) {
		//emmc_fcie_init();
		emmc_drv.t_table.u8_set_cnt++;
		emmc_drv.t_table.set[0].u8_clk = gau8_emmc_pll_sel_200[0];
		emmc_drv.t_table.set[0].u8_reg2ch = g_tab_set_tmp_t.u8_reg2ch;
		emmc_drv.t_table.set[0].u8_skew4 = g_tab_set_tmp_t.u8_skew4;
		emmc_drv.t_table.set[0].u32_scan_result = g_tab_set_tmp_t.u32_scan_result;
		emmc_drv.t_table.u32_ver_no = EMMC_TIMING_TABLE_VERSION;

		#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400
		//inherit clock from hs200
		if (emmc_drv.t_table_g.u8_speed_mode == FCIE_EMMC_HS400)
			emmc_drv.t_table_g.u32_clk = gau8_emmc_pll_sel_200[0];
		#endif
		emmc_fcie_apply_timing_set(0);

		emmc_debug(0, 0, "\nscan Skew4\n");

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
			emmc_debug(0, 0, "%02X ", u32_i);

		emmc_debug(0, 0, "\n");

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((emmc_drv.t_table.set[0].u32_scan_result & (1 << u32_i)) == 0)
				emmc_debug(0, 0, " X ");
			else
				emmc_debug(0, 0, " O ");
		}
		emmc_debug(0, 0, "\n");
		emmc_dump_timing_table();
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
		emmc_drv.t_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
		memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table));

		if (!EMMC_CHK_BKG_SCAN_HS200()) {
			u32_err = emmc_cmd24(EMMC_HS200TABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
			u32_ret = emmc_cmd24(EMMC_HS200TABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
			if (u32_err != EMMC_ST_SUCCESS  && u32_ret != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
					   u32_err, u32_ret);
				emmc_fcie_reset_to_legacy();
				return EMMC_ST_ERR_SAVE_DDRT_FAIL;
			}
		}

		memcpy(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1);
		memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table_g, sizeof(emmc_drv.t_table_g));
		u32_err = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
		u32_ret = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);

		if (u32_err != EMMC_ST_SUCCESS  && u32_ret != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
				   u32_err, u32_ret);
			emmc_fcie_reset_to_legacy();
			return EMMC_ST_ERR_SAVE_DDRT_FAIL;
		}

		return EMMC_ST_SUCCESS;
	}

	emmc_fcie_reset_to_legacy();

	return EMMC_ST_ERR_SKEW4;
}

#ifdef IP_FCIE_VERSION_5
U32 emmc_fcie_detect_hs200_timing_skew1_skew4(U8 u8_enable_error_log)
{
	U32 u32_i, u32_err, u32_skew4_result;
	U8  u8_skew1;

	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS200);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = emmc_fcie_build_hS200_timing_table();//eMMC 5.1 do not have hs200 table
	if (u32_err != EMMC_ST_SUCCESS)
		emmc_fcie_err_handler_stop();
	}
	EMMC_DISABLE_LOG(0);
	#endif
	u32_i = 0;
	emmc_debug(0, 0, "Skew4        %02u ", u32_i);
	for (u32_i = 1; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u8_skew1 = 0; u8_skew1 < TOTAL_PLL_SKEW4_CNT; u8_skew1++) {
		u32_skew4_result = 0;
		emmc_drv.t_table.set[0].u8_skew2 = 0;//HS200 SKEW2 is default

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on turning flag
			if (u8_enable_error_log)//disable turning error log
				EMMC_DISABLE_LOG(1);

			if (u32_i < PLL_SKEW4_CNT) {
				emmc_drv.t_table.set[0].u8_reg2ch = 0;
				emmc_drv.t_table.set[0].u8_skew4 = u32_i;
			} else {
				emmc_drv.t_table.set[0].u8_reg2ch = 1;
				emmc_drv.t_table.set[0].u8_skew4 = (u32_i - PLL_SKEW4_CNT);
			}
			emmc_fcie_apply_timing_set(0);
			if (u8_skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1 - PLL_SKEW4_CNT);
			}

			u32_err = emmc_cmd21();
			if (u32_err == EMMC_ST_SUCCESS &&
			    emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew4_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs200((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off turning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: SKEW4: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off turning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew1: %02u |  ", u8_skew1);
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew4_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

U32 gu32_skew4_result = 0x3FFFF;
U32 gu32_skew4_result_lv0 = 0x3FFFF;

U32 emmc_fcie_detect_hs200_timing_triger_level_skew4(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew4_result;
	U8  triger_level;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS200);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC switch to HS200 fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "Skew4              %02u ", u32_i);
	for (u32_i = 1; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");
	//==========================================
	for (triger_level = 0; triger_level < TRIGGER_CNT; triger_level++) {
		if (u8_enable_error_log)//disable tuning error log
			EMMC_DISABLE_LOG(1);

		emmc_set_triger_level(triger_level);
		//--------------------------------------
		u32_skew4_result = 0;

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

			if (u32_i < PLL_SKEW4_CNT) {
					emmc_drv.t_table.set[0].u8_reg2ch = 0;
				emmc_drv.t_table.set[0].u8_skew4 = u32_i;
			} else {
				emmc_drv.t_table.set[0].u8_reg2ch = 1;
				emmc_drv.t_table.set[0].u8_skew4 = (u32_i - PLL_SKEW4_CNT);
			}
			emmc_fcie_apply_timing_set(0);

			u32_err = emmc_cmd21();
			if (u32_err == EMMC_ST_SUCCESS  &&
			    emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew4_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs200((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: SKEW4: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "TrigerLevel: %02u |  ", triger_level);
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew4_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");

		if (triger_level == 0)
			gu32_skew4_result_lv0 &= u32_skew4_result;

		gu32_skew4_result &= u32_skew4_result;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x20, TIGER_LVL_MASK);

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_skew4(void)
{
	U32 u32_i, u32_err, u32_skew4_result = 0;

	EMMC_DISABLE_LOG(1);

	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

	emmc_platform_init();

	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}

	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}

	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS200);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC switch to HS200 fail!!\n");
		emmc_fcie_err_handler_stop();
	}

	EMMC_DISABLE_LOG(0);

	emmc_debug(0, 0, "Skew4:\n");
	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		EMMC_DISABLE_LOG(1);

		if (u32_i < PLL_SKEW4_CNT) {
			#ifdef REG_ANL_SKEW4_INV
			if (emmc_drv.host->dev_comp->analog_skew4)
				REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);
		} else {
			#ifdef REG_ANL_SKEW4_INV
			if (emmc_drv.host->dev_comp->analog_skew4)
				REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
			#endif

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);
		}

		u32_err = emmc_cmd21();
		if (u32_err == EMMC_ST_SUCCESS &&
		    emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_skew4_result |= (1 << u32_i);
		} else {
			u32_err = emmc_fcie_reset_to_hs200((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
				EMMC_DISABLE_LOG(0);
				emmc_debug(0, 1, "eMMC Err: SKEW4: %d fail\n", u32_i);
				break;
			}
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	EMMC_DISABLE_LOG(0);

	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		if ((u32_skew4_result & (1 << u32_i)) == 0)
			printf(" X ");
		else
			printf(" O ");
	}

	printf("\n");

	return EMMC_ST_SUCCESS;
}

#endif
#endif
#if defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400

U32 emmc_fcie_reset_to_hs400(U8 u8_clk_param)
{
	U32 u32_err;

	//emmc_debug(0, 0, "\033[7;31m%s\033[m\n", __FUNCTION__);
	//--------------------------------
	//reset FCIE & eMMC to normal SDR mode
	if (!EMMC_IF_NORMAL_SDR()) {
		//emmc_debug(0,0,"eMMC: re-init to SDR\n");
		emmc_drv.u32_drvflag = 0;
		emmc_platform_init();
		u32_err = emmc_fcie_init();
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_init fail, %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "eMMC Err: emmc_init_device_ex fail: %X\n", u32_err);
			return u32_err;
		}
	}

	//--------------------------------
	//set eMMC to HS200 mode
	//#if 0
	//u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS200);
	//if (u32_err != EMMC_ST_SUCCESS) {
		//emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: set HS200 IF fail: %X\n", u32_err);

		//return u32_err;
	//}
	//#endif

	u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
			   "eMMC Err: set HS400 IF fail: %X\n", u32_err);
		return u32_err;
	}

	emmc_clock_setting(u8_clk_param);

	return u32_err;
}

// 0~15
void emmc_fcie_set_delay_line(U32 u32_value)
{
//printf("\n\nSetDelayLatch(%d)\n", u32Value);

	if (u32_value > RXDLL_CNT) {
		printf("eMMC Err: wrong delay latch value\n");
		return;
	}

	if (u32_value % 2)
		REG_FCIE_SETBIT(reg_emmcpll_0x09, BIT1);
	else
		REG_FCIE_CLRBIT(reg_emmcpll_0x09, BIT1);

	u32_value = (u32_value & 0xFFFE) >> 1;
	REG_FCIE_CLRBIT(reg_emmcpll_0x09, (0x000F << 4));
	REG_FCIE_SETBIT(reg_emmcpll_0x09, (u32_value << 4));
	//printf("emmc_pll[09] = %04X\n", REG_FCIE(reg_emmcpll_0x09));
}

U32 emmc_fcie_detect_hs400_timing_ex(U32 u32_rx_dll_result)
{
	U16 u16_i = 0, u16_start_bit =  START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT, u16_cnt = 0;

	if (emmc_drv.chip_id != MT5896_CHIP_ID)
		emmc_debug(0, 0, "rx_dll_result:0x%x\n", u32_rx_dll_result);

	//==================================================
	//case.1: all bits set
	if ((u32_rx_dll_result & ((1 << RXDLL_CNT) - 1)) == ((1 << RXDLL_CNT) - 1)) {
		if (emmc_drv.chip_id != MT5896_CHIP_ID)
			emmc_debug(EMMC_DEBUG_LEVEL, 0, " all\n");

		g_tab_set_tmp_t.u8_cell = 7;
		u16_cnt = RXDLL_CNT;
	} else {
		do {
			if ((u32_rx_dll_result & (1 << u16_i)) && u16_start_bit == START_BIT_DEFAULT)
				u16_start_bit = u16_i;
			else if (u16_start_bit !=  START_BIT_DEFAULT  && (u32_rx_dll_result & (1 << u16_i)) == 0)
				u16_end_bit = u16_i - 1;

			if (u16_end_bit != END_BIT_DEFAULT) {
				if ((u16_end_bit - u16_start_bit + 1) > u16_cnt) {
					g_tab_set_tmp_t.u8_cell =
						u16_start_bit + ((u16_end_bit - u16_start_bit) / 2);
						u16_cnt = u16_end_bit - u16_start_bit + 1;
				}
				if (u16_cnt >= MIN_OK_DS_CNT)
					break;

				u16_start_bit = START_BIT_DEFAULT;
				u16_end_bit  = END_BIT_DEFAULT;
			}
			u16_i++;
		} while (u16_i < (RXDLL_CNT + 1));
	}
	g_tab_set_tmp_t.u8_cell_cnt = u16_cnt;
	if (emmc_drv.chip_id != MT5896_CHIP_ID)
		emmc_debug(0, 0, "BestDelayLine:0x%x\n", g_tab_set_tmp_t.u8_cell);

	return u16_cnt;
}

U32 emmc_fcie_detect_hs400_timing_skew2_ex(U32 u32_skew2_result)
{
	U16 u16_i = 0, u16_start_bit = START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT, u16_cnt = 0;

	if ((u32_skew2_result & ((1 << SKEW2_CNT) - 1)) == ((1 << SKEW2_CNT) - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " all\n");
		g_tab_set_tmp_t.u8_skew2 = 4;
		u16_cnt = SKEW2_CNT;
	} else {
		do {
			if ((u32_skew2_result & (1 << u16_i)) &&
			    u16_start_bit == START_BIT_DEFAULT) {
				u16_start_bit = u16_i;
			} else if (u16_start_bit !=  START_BIT_DEFAULT  &&
				   (u32_skew2_result & (1 << u16_i)) == 0) {
				u16_end_bit = u16_i - 1;
				break;
			}
			u16_i++;
		} while (u16_i < (SKEW2_CNT + 1));

		if (u16_end_bit !=  END_BIT_DEFAULT) {
			g_tab_set_tmp_t.u8_skew2 = u16_start_bit + ((u16_end_bit - u16_start_bit) >> 1);
			u16_cnt = u16_end_bit - u16_start_bit + 1;
		}
	}

	return u16_cnt;
}

void emmc_fcie_set_skew1_skew4(U32 u32_skew1, U32 u32_skew4)
{
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
	if (u32_skew1 < PLL_SKEW4_CNT) {
		REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_skew1);
	} else {
		REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_skew1 - PLL_SKEW4_CNT);
	}

	if (u32_skew4 >= PLL_SKEW4_CNT) {
		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
		REG_FCIE_SETBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
				emmc_drv.host->dev_comp->analog_skew4_reg_bit);
		#endif
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_skew4 - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);
	} else {
		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
			REG_FCIE_CLRBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
					emmc_drv.host->dev_comp->analog_skew4_reg_bit);
		#endif
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_skew4) << BIT_SKEW4_SHIFT);
	}
}

static void emmc_fcie_detect_hs400_timing_cmd13_ex(U8 *u8_skew_fail_count,
						   S8 *skew4_idx,
						   U8 u8_trg_lvl,
						   U32 u32_skew1)
{
	U32 u32_i = 0, u32_k, u32_err = 0, u32_skew4_result = 0;
	U8 u8_trg_lvl_value[3] = {0, 3, 7};
	S8 s8_skew4_idx;

	s8_skew4_idx = *skew4_idx;
	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		for (u32_k = 0; u32_k < 5; u32_k++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;
			emmc_fcie_set_skew1_skew4(u32_skew1, u32_i);
			emmc_set_triger_level(u8_trg_lvl_value[u8_trg_lvl]);
			u32_err = emmc_cmd13(emmc_drv.u16_rca);
			if (u32_err == EMMC_ST_SUCCESS) {
				//u32_skew4_result |= (1 << u32_i);
				continue;
			} else {
				emmc_set_triger_level(0);
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
					break;
				}
				break;
			}
		}
		if (u32_k == 5)
			u32_skew4_result |= (1 << u32_i);
	}

	if (((u32_skew4_result & ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) !=
	    ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) && u32_skew4_result != 0) {
		//search the fail level
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew4_result & (1 << u32_i)) == 0) {
				s8_skew4_idx = (S32)u32_i - (S32)u32_skew1;
				if (s8_skew4_idx < 0)
					s8_skew4_idx += TOTAL_PLL_SKEW4_CNT;

				u8_skew_fail_count[s8_skew4_idx]++;
			}
		}
	}
	*skew4_idx = s8_skew4_idx;
}

static U32 emmc_fcie_detect_hs400_timing_cmd13(S32 *s32_skew4_ori, U8 *u8_max_count)
{
	U32 u32_i = 0, u32_j, u32_err = 0;
	U8 u8_trg_lvl;
	U8 u8_skew_fail_count[TOTAL_PLL_SKEW4_CNT] = {0};
	U8 u8_max_skew = 0;
	S8 s8_skew4_idx = 0;

	for (u8_trg_lvl = 0; u8_trg_lvl < 3; u8_trg_lvl++) {
		memset(u8_skew_fail_count, 0, sizeof(u8_skew_fail_count));
		*u8_max_count = 0;
		u8_max_skew = 0;
		emmc_set_triger_level(0);
		u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
			emmc_fcie_reset_to_legacy();
			return u32_err;
		}

		//scan skew4 for hs400 rsp using skew1 & skew4
		for (u32_j = 0; u32_j < TOTAL_PLL_SKEW4_CNT; u32_j++)
			emmc_fcie_detect_hs400_timing_cmd13_ex(u8_skew_fail_count, &s8_skew4_idx, u8_trg_lvl, u32_j);

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((*u8_max_count) < u8_skew_fail_count[u32_i]) {
				u8_max_skew = u32_i;
				*u8_max_count = u8_skew_fail_count[u32_i];
			}
		}

		s8_skew4_idx = u8_max_skew - PLL_SKEW4_CNT;
		if (s8_skew4_idx < 0)
			s8_skew4_idx += TOTAL_PLL_SKEW4_CNT;

		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;

		*s32_skew4_ori = s8_skew4_idx;
		s8_skew4_idx = 0;
		if ((*u8_max_count) >= 3)
			break;
	}

	return EMMC_ST_SUCCESS;
}

static U32 emmc_fcie_detect_hs400_timing_rsp(S32 *s32_skew4_ori, U8 *u8_max_count)
{
	U32 u32_err = 0;
	U8 u8_drv_str;

	for (u8_drv_str = 0; u8_drv_str < 2; u8_drv_str++) {
		emmc_drv.u32_last_err_code = 0;
		if (u8_drv_str)
			emmc_drv.u32_last_err_code |= DRV_FLAG_WEAK_STRENGTH;

		u32_err = emmc_fcie_detect_hs400_timing_cmd13(s32_skew4_ori, u8_max_count);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if ((*u8_max_count) >= 3) {
			emmc_drv.u32_last_err_code &= ~DRV_FLAG_WEAK_STRENGTH;
			emmc_set_triger_level(0);
			u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				emmc_fcie_reset_to_legacy();
				return u32_err;
			}
			break;
		}
	}

	return u32_err;
}

static void emmc_fcie_set_hs400_gen_table_rsp(unsigned long t_table_reg_base,
					      S8 s8_skew4_idx,
					      U8 *au8_skew4,
					      U8 *au8_reg_2ch)
{
	//skew4 invesre
	#ifdef REG_ANL_SKEW4_INV
	if (emmc_drv.host->dev_comp->analog_skew4) {
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW4_INV_BIT)].u32_reg_address =
			((unsigned long)(&emmc_drv.host->emmcpllbase->emmcpll_rx[(U8)
			emmc_drv.host->dev_comp->analog_skew4_reg_offset])
			- t_table_reg_base);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW4_INV_BIT)].u16_reg_value =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW4_INV_BIT)].u16_reg_mask =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;
	}
	#endif
	//skew4 value
	emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
		GT_OFFSET_SKEW4_VALUE)].u32_reg_address =
		(reg_emmcpll_0x03 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
		GT_OFFSET_SKEW4_VALUE)].u16_reg_value =
		au8_skew4[(U8)s8_skew4_idx] << 12;
	emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
		GT_OFFSET_SKEW4_VALUE)].u16_reg_mask = BIT_SKEW4_MASK;
	emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
		GT_OFFSET_SKEW4_VALUE)].u16_op_code = REG_OP_SETBIT;

	if (au8_reg_2ch[(U8)s8_skew4_idx] == 1)
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW4_INV_BIT)].u16_op_code = REG_OP_SETBIT;
	else
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW4_INV_BIT)].u16_op_code = REG_OP_CLRBIT;
}

static U32 emmc_fcie_detect_hs400_timing_rx(U32 *au32_rx_dll_result, U8 *au8_cell, U8 *au8_cell_cnt)
{
	U32 u32_i = 0, u32_err = 0;

	if (emmc_drv.chip_id == MT5896_CHIP_ID)
		emmc_debug(0, 0, "\nscan RX\n");
	else
		emmc_debug(0, 0, "\nscan RXDLL\n");

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;
		//using default skew2 to detect RXDLL
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, BIT_DEFAULT_SKEW2);
		emmc_fcie_set_delay_line(u32_i);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			au32_rx_dll_result[0] |= (1 << u32_i);
			//emmc_debug(0, 1, "RXDLL: %d ok\n", u32_i);
		} else {
			emmc_debug(0, 1, "RXDLL: %d fail\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
	au8_cell_cnt[0] = emmc_fcie_detect_hs400_timing_ex(au32_rx_dll_result[0]);

	if (au8_cell_cnt[0] < 7) {
		emmc_debug(0, 1, "RXDLL phase are not enough, only %d!!!\n", au8_cell_cnt[0]);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_NO_OK_DDR_PARAM;
	}

	if (emmc_drv.chip_id == MT5896_CHIP_ID) {
		emmc_drv.u32_fix_skew4_result =
			emmc_fcie_m6e3_fix_skew4_result(au32_rx_dll_result[0]);
		for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++)
			emmc_debug(0, 0, "%02X ", u32_i);

		emmc_debug(0, 0, "\n");
		for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
			if ((emmc_drv.u32_fix_skew4_result & (1 << u32_i)) == 0)
				emmc_debug(0, 0, " X ");
			else
				emmc_debug(0, 0, " O ");
		}
	}	else {
		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++)
			emmc_debug(0, 0, "%02X ", u32_i);

		emmc_debug(0, 0, "\n");

		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			if ((au32_rx_dll_result[0] & (1 << u32_i)) == 0)
				emmc_debug(0, 0, " X ");
			else
				emmc_debug(0, 0, " O ");
		}
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(0, 0, "\nOK\n");

	au8_cell[0] = g_tab_set_tmp_t.u8_cell;
	return u32_err;
}

static void emmc_fcie_set_hs400_gen_table_rx(unsigned long t_table_reg_base,
					     U8 *au8_skew4,
					     U8 *au8_reg_2ch,
					     U8 *au8_cell,
					     U8 *au8_cell_cnt)
{
	S8 s8_skew4_idx = 0;

	g_tab_set_tmp_t.u8_cell_cnt = 0;

	for (s8_skew4_idx = 0; s8_skew4_idx < 5; s8_skew4_idx++) {
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_RXDLL_VALUE)].u32_reg_address =
			(reg_emmcpll_0x09 - t_table_reg_base);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_RXDLL_VALUE)].u16_reg_value =
			(((au8_cell[(U8)s8_skew4_idx] & 0xFFFE) >> 1) << 4) |
			((au8_cell[(U8)s8_skew4_idx] % 2) ? BIT1 : 0);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_RXDLL_VALUE)].u16_reg_mask =
			((0xF << 4) | BIT1);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_RXDLL_VALUE)].u16_op_code = REG_OP_SETBIT;

		if (s8_skew4_idx == 2)
			continue;

		if (g_tab_set_tmp_t.u8_cell_cnt < au8_cell_cnt[(U8)s8_skew4_idx]) {
			g_tab_set_tmp_t.u8_cell_cnt = au8_cell_cnt[(U8)s8_skew4_idx];
			emmc_drv.t_table_g.u8_cur_set_idx = s8_skew4_idx;
		}
	}

	//backward compatible with old version of kernel
	emmc_drv.t_table.set[0].u8_reg2ch = au8_reg_2ch[(U8)emmc_drv.t_table_g.u8_cur_set_idx];
	emmc_drv.t_table.set[0].u8_skew4 = au8_skew4[(U8)emmc_drv.t_table_g.u8_cur_set_idx];
	emmc_drv.t_table.set[0].u8_cell = au8_cell[(U8)emmc_drv.t_table_g.u8_cur_set_idx];
	emmc_drv.t_table.set[0].u8_cell_cnt = g_tab_set_tmp_t.u8_cell_cnt;
}

static U32 emmc_fcie_detect_hs400_timing_tx(U32 *u32_skew2_result)
{
	U32 u32_i = 0, u32_err = 0;
	U32 skew2_result = 0;

	//================================================
	if (emmc_drv.chip_id == MT5896_CHIP_ID)
		emmc_debug(0, 0, "\nscan TX\n");
	else
		emmc_debug(0, 0, "\nscan Skew2\n");

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			skew2_result |= (1 << u32_i);
			//emmc_debug(0, 1, "Skew2: %d ok\n", u32_i);
		} else {
			if (emmc_drv.chip_id != MT5896_CHIP_ID)
				emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);

			u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
			emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}
	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		if ((skew2_result & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(0, 0, "\nOK\n");
	*u32_skew2_result = skew2_result;

	return u32_err;
}

static void emmc_fcie_set_hs400_gen_table_tx(unsigned long t_table_reg_base)
{
	S8 s8_skew4_idx;

	emmc_drv.t_table.set[0].u8_skew2 = g_tab_set_tmp_t.u8_skew2;
	//setup GEN_TIMING_TABLE
	for (s8_skew4_idx = 0 ; s8_skew4_idx < 5; s8_skew4_idx++) {
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW2_VALUE)].u32_reg_address = (reg_emmcpll_0x03 - t_table_reg_base);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW2_VALUE)].u16_reg_value = g_tab_set_tmp_t.u8_skew2 << 4;
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW2_VALUE)].u16_reg_mask = (0xF << 4);
		emmc_drv.t_table_g.reg_set[(U8)(s8_skew4_idx * emmc_drv.t_table_g.u8_register_cnt +
			GT_OFFSET_SKEW2_VALUE)].u16_op_code = REG_OP_SETBIT;
	}
}

static void emmc_fcie_init_hs400_gen_table(void)
{
	memset((void *)emmc_drv.t_table_g.reg_set, 0, sizeof(emmc_drv.t_table_g.reg_set));

	emmc_drv.t_table_g.u8_set_cnt = 5;//initial set count for apply general register
	emmc_drv.t_table_g.u8_register_cnt = 4;//skew4 & its inverse register, RXDLL & its inverse & skew2
	emmc_drv.t_table_g.u8_cur_set_idx = 0;

	if (emmc_drv.host->id_matched_driving <= 4)
		emmc_drv.t_table_g.u32_device_driving = emmc_drv.host->id_matched_driving;
	else
		emmc_drv.t_table_g.u32_device_driving = 0;
}

static U32 emmc_fcie_store_hs400_timing_table(void)
{
	U32 u32_err = 0, u32_ret = 0;

	emmc_drv.t_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table));

	u32_err = emmc_cmd24(EMMC_HS400TABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400TABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
	if (u32_err != EMMC_ST_SUCCESS && u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	emmc_drv.t_table_g.u32_ver_no = EMMC_TIMING_TABLE_VERSION;
	emmc_drv.t_table_g.u8_speed_mode = FCIE_EMMC_HS400;
	memcpy(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1);
	emmc_dump_timing_table();

	emmc_drv.t_table_g.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table_g.u32_ver_no,
						     sizeof(emmc_drv.t_table_g) - sizeof(U32));
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table_g, sizeof(emmc_drv.t_table_g));

	u32_err = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
	if (u32_err != EMMC_ST_SUCCESS  && u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_timing(void)
{
	U32 u32_err = 0, u32_skew2_result = 0;
	U16 u16_skew2_cnt = 0;
	S8  as8_skew4_shift[5] = {-2, -1, 0, 1, 2}, s8_skew4_idx = 0;
	S32 s32_skew4_ori = 0, s32_skew4_tmp = 0;
	U8 u8_max_count = 0;
	U32 au32_rx_dll_result[5] = {0};
	U8 au8_reg_2ch[5], au8_skew4[5], au8_cell[5], au8_cell_cnt[5];
	unsigned long t_table_reg_base = RIU_BASE;

	if (emmc_drv.host->dev_comp->emmcpllbase_t_tablebase > 0)
		t_table_reg_base = ((unsigned long)emmc_drv.host->emmcpllbase);

	emmc_fcie_init_hs400_gen_table();
	//==========================================
	//add skew4 for hs400 using hs200 + pad setting of HS400 to detect skew4
	u32_err = emmc_fcie_detect_hs400_timing_rsp(&s32_skew4_ori, &u8_max_count);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	if (u8_max_count < 3) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HS400 scan skew4 fail\n");
			emmc_fcie_reset_to_legacy();
			return EMMC_ST_ERR_SKEW4;
	}
	if (emmc_drv.chip_id != MT5896_CHIP_ID)
		emmc_debug(0, 0, "HS400 Skew4 %lXh\n", s32_skew4_ori);

	emmc_drv.u32_last_err_code &= ~DRV_FLAG_WEAK_STRENGTH;
	emmc_set_triger_level(0);
	u32_err =  emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		emmc_fcie_reset_to_legacy();
		return u32_err;
	}
	s8_skew4_idx = 0;

LABEL_SCAN_RXDLL:

	s32_skew4_tmp = s32_skew4_ori + as8_skew4_shift[(U8)s8_skew4_idx];

	if (s32_skew4_tmp < 0)
		au8_skew4[(U8)s8_skew4_idx] = TOTAL_PLL_SKEW4_CNT + s32_skew4_tmp;
	else if (s32_skew4_tmp >= TOTAL_PLL_SKEW4_CNT)
		au8_skew4[(U8)s8_skew4_idx] = s32_skew4_tmp - TOTAL_PLL_SKEW4_CNT;
	else
		au8_skew4[(U8)s8_skew4_idx] = s32_skew4_tmp;

	if (au8_skew4[(U8)s8_skew4_idx] < PLL_SKEW4_CNT) {
		au8_reg_2ch[(U8)s8_skew4_idx] = 0;
	} else {
		au8_reg_2ch[(U8)s8_skew4_idx] = 1;
		au8_skew4[(U8)s8_skew4_idx] -= PLL_SKEW4_CNT;
	}
	emmc_fcie_set_hs400_gen_table_rsp(t_table_reg_base, s8_skew4_idx, &au8_skew4[0], &au8_reg_2ch[0]);
	emmc_fcie_apply_reg(s8_skew4_idx);
	//================================================
	//scan RXDLL for the 5 Skew4: [-2, -1, Skew4, +1, +2]
	//================================================
	if (emmc_drv.chip_id != MT5896_CHIP_ID) {
		emmc_debug(0, 0, "\n  Skew4[%d]: inv: %Xh, Skew4: %Xh\n", as8_skew4_shift[(U8)s8_skew4_idx],
			   au8_reg_2ch[(U8)s8_skew4_idx], au8_skew4[(U8)s8_skew4_idx]);
	}
	//--------------------------------------
	//reduce scan time, only scan index 0
	if (s8_skew4_idx == 0) {
		u32_err = emmc_fcie_detect_hs400_timing_rx(au32_rx_dll_result, au8_cell, au8_cell_cnt);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		//skew4 +1 +2 cases of even OkCnt, shift RXDLL +1 to right side.
		if ((au8_cell_cnt[(U8)s8_skew4_idx] & 1) == 0 && as8_skew4_shift[(U8)s8_skew4_idx] > 0)
			au8_cell[(U8)s8_skew4_idx] += 1;
		if (emmc_drv.chip_id != MT5896_CHIP_ID) {
			emmc_debug(0, 0, "             RXDLL: %Xh, RXDLLResult: %Xh, OkCnt: %u\n",
				   au8_cell[(U8)s8_skew4_idx], au32_rx_dll_result[(U8)s8_skew4_idx],
				   au8_cell_cnt[(U8)s8_skew4_idx]);
		}
	} else {
		au8_cell[(U8)s8_skew4_idx] = au8_cell[0];
		au32_rx_dll_result[(U8)s8_skew4_idx] = au32_rx_dll_result[0];
		au8_cell_cnt[(U8)s8_skew4_idx] = au8_cell_cnt[0];
		if (emmc_drv.chip_id != MT5896_CHIP_ID) {
				emmc_debug(0, 0, "             RXDLL: %Xh, RXDLLResult: %Xh, OkCnt: %u\n",
					   au8_cell[(U8)s8_skew4_idx], au32_rx_dll_result[(U8)s8_skew4_idx],
					   au8_cell_cnt[(U8)s8_skew4_idx]);
		}
	}

	s8_skew4_idx++;
	if (s8_skew4_idx < 5)
		goto LABEL_SCAN_RXDLL;

	//--------------------------------------
	//setup GEN_TIMING_TABLE
	emmc_fcie_set_hs400_gen_table_rx(t_table_reg_base, &au8_skew4[0], &au8_reg_2ch[0], &au8_cell[0], &au8_cell_cnt[0]);
	emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);
	u32_err =  emmc_fcie_detect_hs400_timing_tx(&u32_skew2_result);
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u16_skew2_cnt = emmc_fcie_detect_hs400_timing_skew2_ex(u32_skew2_result);
	if (u16_skew2_cnt == 0) {
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_NO_OK_DDR_PARAM;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, g_tab_set_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT);

	emmc_fcie_set_hs400_gen_table_tx(t_table_reg_base);
	emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);
	//==========================================
	u32_err = emmc_fcie_store_hs400_timing_table();

	return u32_err;
}

U32 emmc_fcie_detect_hs400_timing_skew4_rx(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_rx_dll_result;
	U8  u8_skew4;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}
		//HS400 use DLL setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "RX           %02u ", u32_i);
	for (u32_i = 1; u32_i < RXDLL_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew4 = 0; u8_skew4 < TOTAL_PLL_SKEW4_CNT; u8_skew4++) {
		if (u8_skew4 < PLL_SKEW4_CNT) {
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_CLRBIT;
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value = u8_skew4 << 12;
		} else {
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_SETBIT;
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value = (u8_skew4 - PLL_SKEW4_CNT) << 12;
		}

		emmc_fcie_apply_timing_set(0);

		//--------------------------------------
		u32_rx_dll_result = 0;

		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			if (u8_enable_error_log)//disable tuning error log
				EMMC_DISABLE_LOG(1);

			emmc_fcie_set_delay_line(u32_i);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_rx_dll_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: RXDLL: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew4: %02u |  ", u8_skew4);
		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			if ((u32_rx_dll_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_switch_to_hs400_mode(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		//HS400 use DLL setting for DAT[0-7]
		if (emmc_drv.u8_emmc_pll_skew4)
			u32_err = emmc_fcie_detect_hs400_skew4_timing();
		else
			u32_err = emmc_fcie_detect_hs400_timing();

		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}

		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}
	EMMC_DISABLE_LOG(0);
	return u32_err;
}

static U32 emmc_fcie_detect_hs400_timing_skew4_skew1_cmd13(U8 u8_drv_str, U8 *u8_max_count)
{
	U32 u32_i = 0, u32_j, u32_err = 0;
	U8 u8_trg_lvl_value[3] = {0, 3, 7};
	U8 u8_skew_fail_count[18];
	U8 u8_trg_lvl, u8_max_skew, max_count = 0;
	S8 s8_skew4_idx = 0;

	for (u8_trg_lvl = 0; u8_trg_lvl < 3; u8_trg_lvl++) {
		memset(u8_skew_fail_count, 0, sizeof(u8_skew_fail_count));
		max_count = 0;
		u8_max_skew = 0;
		emmc_set_triger_level(0);
		u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
		if (u32_err != EMMC_ST_SUCCESS) {
			EMMC_DISABLE_LOG(0);
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
			return u32_err;
		}
		//scan skew4 for hs400 rsp using skew1 & skew4
		for (u32_j = 0; u32_j < TOTAL_PLL_SKEW4_CNT; u32_j++)
			emmc_fcie_detect_hs400_timing_cmd13_ex(u8_skew_fail_count, &s8_skew4_idx, u8_trg_lvl, u32_j);

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if (max_count < u8_skew_fail_count[u32_i]) {
				u8_max_skew = u32_i;
				max_count = u8_skew_fail_count[u32_i];
			}
		}

		s8_skew4_idx = u8_max_skew - PLL_SKEW4_CNT;
		if (s8_skew4_idx < 0)
			s8_skew4_idx += TOTAL_PLL_SKEW4_CNT;

		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

		s8_skew4_idx = 0;
		if (max_count >= 3) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC DrvStr: 0x%X, TrgLvl: 0x%X\n",
				   u8_drv_str, u8_trg_lvl_value[u8_trg_lvl]);
			emmc_set_triger_level(u8_trg_lvl_value[u8_trg_lvl]);
			break;
		}
	}
	*u8_max_count = max_count;

	return EMMC_ST_SUCCESS;
}

static U32 emmc_fcie_detect_hs400_timing_skew4_skew1_rsp(U8 *u8_max_count)
{
	U32 u32_err = 0;
	U8 u8_drv_str;

	for (u8_drv_str = 0; u8_drv_str < 2; u8_drv_str++) {
		emmc_drv.u32_last_err_code = 0;
		if (u8_drv_str)
			emmc_drv.u32_last_err_code |= DRV_FLAG_WEAK_STRENGTH;

		u32_err = emmc_fcie_detect_hs400_timing_skew4_skew1_cmd13(u8_drv_str, u8_max_count);
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if ((*u8_max_count) >= 3) {
			u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS) {
				EMMC_DISABLE_LOG(0);
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				return u32_err;
			}
			break;
		}
	}

	return EMMC_ST_SUCCESS;
}

static void emmc_fcie_set_hs400_gen_table_skew4(U8 u8_skew4)
{
	if (u8_skew4 < PLL_SKEW4_CNT) {
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_CLRBIT;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value = u8_skew4 << BIT_SKEW4_SHIFT;
	} else {
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_SETBIT;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value = (u8_skew4 - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT;
	}
}

U32 emmc_fcie_detect_hs400_timing_skew4_skew1(U8 u8_enable_error_log)
{
	U32 u32_err = 0, u32_skew1_result = 0;
	U8 u32_i, u8_skew4, u8_max_count = 0;

	u32_err  = emmc_fcie_switch_to_hs400_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	#if 1
	EMMC_DISABLE_LOG(1);
	//==========================================
	//add skew4 for hs400 using hs200 + pad setting of HS400 to detect skew4
	u32_err = emmc_fcie_detect_hs400_timing_skew4_skew1_rsp(&u8_max_count);
	if (u8_max_count < 3) {
		EMMC_DISABLE_LOG(0);
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: HS400 scan skew4 fail\n");
		emmc_fcie_err_handler_stop();
	}
	EMMC_DISABLE_LOG(0);
	#endif
	//==========================================
	u32_i = 0;
	emmc_debug(0, 0, "Skew1        %02u ", u32_i);
	for (u32_i = 1; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew4 = 0; u8_skew4 < TOTAL_PLL_SKEW4_CNT; u8_skew4++) {
		emmc_fcie_set_hs400_gen_table_skew4(u8_skew4);
		emmc_fcie_apply_timing_set(0);
		//--------------------------------------
		u32_skew1_result = 0;

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; //turn on tuning flag
			if (u8_enable_error_log)//disable tuning error log
				EMMC_DISABLE_LOG(1);
			if (u32_i < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i - PLL_SKEW4_CNT);
			}
			if (emmc_cmd13(emmc_drv.u16_rca) == EMMC_ST_SUCCESS) {
				u32_skew1_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: SKEW1: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew4: %02u |  ", u8_skew4);
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew1_result & (1 << u32_i)) == 0)
				emmc_debug(0, 1, " X ");
			else
				emmc_debug(0, 1, " O ");
		}
		emmc_debug(0, 1, "\n");
	}

	//==========================================
	#if 1
	emmc_drv.u32_last_err_code &= ~DRV_FLAG_WEAK_STRENGTH;
	EMMC_DISABLE_LOG(1);
	emmc_set_triger_level(0);
	EMMC_DISABLE_LOG(0);
	u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		return u32_err;
	}
	#endif
	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_timing_skew1_skew2(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew2_result;
	U8  u8_skew1;

	u32_err  = emmc_fcie_switch_to_hs400_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	u32_i = 0;
	emmc_debug(0, 0, "Skew2           %02u ", u32_i);
	for (u32_i = 1; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew1 = 0; u8_skew1 < TOTAL_PLL_SKEW4_CNT; u8_skew1++) {
	//--------------------------------------
		u32_skew2_result = 0;

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; //turn on tuning flag
		if (u8_enable_error_log)//disable tuning error log
			EMMC_DISABLE_LOG(1);

			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value = u32_i << 4;
			emmc_fcie_apply_timing_set(0);
			if (u8_skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1 - PLL_SKEW4_CNT);
			}

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					emmc_debug(0, 1, "eMMC Err: SKEW2: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);

		emmc_debug(0, 0, "Skew1: %02u |  ", u8_skew1);
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			if ((u32_skew2_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

U32 gu32_rx_dll_result = 0xFFFF;
U32 gu32_rx_dll_result_lv0 = 0xFFFF;

U32 emmc_fcie_detect_hs400_timing_trigger_level_rx(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_rx_dll_result;
	U8  triger_level;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();// DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		//HS400 use DLL setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}

		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "RX                 %02u ", u32_i);
	for (u32_i = 1; u32_i < RXDLL_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (triger_level = 0; triger_level < TRIGGER_CNT; triger_level++) {
		if (u8_enable_error_log)//disable tuning error log
			EMMC_DISABLE_LOG(1);

		emmc_set_triger_level(triger_level);
		//--------------------------------------
		u32_rx_dll_result = 0;

		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

			emmc_fcie_set_delay_line(u32_i);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_rx_dll_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: RXDLL: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "TrigerLevel: %02u |  ", triger_level);
		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			if ((u32_rx_dll_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");

		if (triger_level == 0)
			gu32_rx_dll_result_lv0 &= u32_rx_dll_result;

		gu32_rx_dll_result &= u32_rx_dll_result;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x20, TIGER_LVL_MASK);

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_rx_dll(void)
{
	U32 u32_i = 0, u32_err = 0, u32_rx_dll_result = 0;

	EMMC_DISABLE_LOG(1);

	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;

	emmc_platform_init();

	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}

	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		//HS400 use DLL setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}

		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}

	EMMC_DISABLE_LOG(0);

	emmc_debug(0, 0, "RXDLL:\n");
	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);
	emmc_debug(0, 0, "\n");

	emmc_fcie_apply_timing_set(0);

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;

		EMMC_DISABLE_LOG(1);
		emmc_fcie_set_delay_line(u32_i);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_rx_dll_result |= (1 << u32_i);
		} else {
			u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
				EMMC_DISABLE_LOG(0);
				emmc_debug(0, 1, "eMMC Err: RXDLL: %d fail\n", u32_i);
				break;
			}
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
	EMMC_DISABLE_LOG(0);

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		if ((u32_rx_dll_result & (1 << u32_i)) == 0)
			printf(" X ");
		else
			printf(" O ");
	}
	printf("\n");

	return EMMC_ST_SUCCESS;
}

#endif

#if defined(ENABLE_EMMC_HS400_5_1) && ENABLE_EMMC_HS400_5_1

U32 emmc_fcie_reset_to_hs400_5_1(U8 u8_clk_param)
{
	U32 u32_err;

	if (!EMMC_IF_NORMAL_SDR()) {
		emmc_drv.u32_drvflag = 0;
		emmc_platform_init();
		u32_err = emmc_fcie_init();
		if (u32_err) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_init fail, %Xh\n", u32_err);
			return u32_err;
		}
		u32_err = emmc_init_device_ex();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0,
				   "eMMC Err: emmc_init_device_ex fail: %X\n", u32_err);
			return u32_err;
		}
	}

	//--------------------------------
	u32_err = emmc_fcie_enable_fastmode_ex(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: set HS400 5.1 IF fail: %X\n", u32_err);

		return u32_err;
	}

	emmc_clock_setting(u8_clk_param);

	return u32_err;
}

static void emmc_fcie_init_hs400_5_1_gen_table(unsigned long t_table_reg_base)
{
	memset((void *)&emmc_drv.t_table, 0, sizeof(emmc_drv.t_table));
	memset((void *)&emmc_drv.t_table_g, 0, sizeof(emmc_drv.t_table_g));

	emmc_drv.t_table_g.u8_set_cnt = 1;
	emmc_drv.t_table_g.u8_register_cnt = 4;
	emmc_drv.t_table_g.u8_cur_set_idx = 0;
	emmc_drv.t_table_g.u32_clk = gau8_emmc_pll_sel_200[0];

	if (emmc_drv.host->id_matched_driving <= 4)
		emmc_drv.t_table_g.u32_device_driving = emmc_drv.host->id_matched_driving;
	else
		emmc_drv.t_table_g.u32_device_driving = 0;

	//setup GEN_TIMING_TABLE
	//skew4 invesre
	#ifdef REG_ANL_SKEW4_INV
	if (emmc_drv.host->dev_comp->analog_skew4) {
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u32_reg_address =
			((unsigned long)(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset])
			- t_table_reg_base);
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_reg_value =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_reg_mask =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_CLRBIT;
	}
	#endif
	//skew4 value
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u32_reg_address = (reg_emmcpll_0x03 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value   = BIT_SKEW4_MASK;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_mask    = BIT_SKEW4_MASK;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_op_code     = REG_OP_CLRBIT;
}

static void emmc_fcie_set_hs400_5_1_gen_table_rx(U32 au32_rx_dll_result)
{
	U8 au8_cell;
	U32 u32_i;
	unsigned long t_table_reg_base = RIU_BASE;

	if (emmc_drv.host->dev_comp->emmcpllbase_t_tablebase > 0)
		t_table_reg_base = ((unsigned long)emmc_drv.host->emmcpllbase);

	if (emmc_drv.chip_id == MT5896_CHIP_ID) {
		emmc_drv.u32_fix_skew4_result = emmc_fcie_m6e3_fix_skew4_result(au32_rx_dll_result);
		for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++)
			emmc_debug(0, 0, "%02X ", u32_i);

		emmc_debug(0, 0, "\n");
		for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
			if ((emmc_drv.u32_fix_skew4_result & (1 << u32_i)) == 0)
				emmc_debug(0, 0, " X ");
			else
				emmc_debug(0, 0, " O ");
		}
	} else {
		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++)
			emmc_debug(0, 0, "%02X ", u32_i);

		emmc_debug(0, 0, "\n");

		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			if ((au32_rx_dll_result & (1 << u32_i)) == 0)
				emmc_debug(0, 0, " X ");
			else
				emmc_debug(0, 0, " O ");
		}
		emmc_debug(0, 0, "\n");
		emmc_debug(0, 0, "\nOK\n");
	}

	au8_cell = g_tab_set_tmp_t.u8_cell;
	//--------------------------------------
	//setup GEN_TIMING_TABLE
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u32_reg_address =
		(reg_emmcpll_0x09 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_reg_value =
		(((au8_cell & 0xFFFE) >> 1) << 4) | ((au8_cell % 2) ? BIT1 : 0);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_reg_mask =
		((RSG_OP_MASK << 4) | BIT1);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_op_code = REG_OP_SETBIT;

	//backward compatible with old version of kernel
	emmc_drv.t_table.u8_set_cnt++;
	emmc_drv.t_table.set[0].u8_clk = gau8_emmc_pll_sel_200[0];
	emmc_drv.t_table.set[0].u8_reg2ch = 0;
	emmc_drv.t_table.set[0].u8_skew4 = 0;
	emmc_drv.t_table.set[0].u32_scan_result = 0;
	emmc_drv.t_table.set[0].u8_cell = au8_cell;
	emmc_drv.t_table.set[0].u8_cell_cnt = g_tab_set_tmp_t.u8_cell_cnt;
	emmc_drv.t_table.u32_ver_no = EMMC_TIMING_TABLE_VERSION;
}

static U32 emmc_fcie_store_hs400_5_1_timing_table(void)
{
	U32 u32_err = 0, u32_ret = 0;

	emmc_drv.t_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table) - 8);
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table));

	u32_err = emmc_cmd24(EMMC_HS400TABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400TABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
	if (u32_err != EMMC_ST_SUCCESS  && u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	emmc_drv.t_table_g.u32_ver_no = EMMC_TIMING_TABLE_VERSION;
	emmc_drv.t_table_g.u8_speed_mode = FCIE_EMMC_HS400_5_1;
	memcpy(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1);
	emmc_dump_timing_table();

	emmc_drv.t_table_g.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table_g.u32_ver_no,
						     sizeof(emmc_drv.t_table_g) - sizeof(U32));
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table_g, sizeof(emmc_drv.t_table_g));

	u32_err = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
	if (u32_err != EMMC_ST_SUCCESS && u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_5_1_timing(void)
{
	U32 u32_i = 0, u32_err = 0, u32_skew2_result = 0;
	U16 u16_skew2_cnt = 0;
	U32 au32_rx_dll_result = 0;
	U8 au8_cell_cnt;
	unsigned long t_table_reg_base = RIU_BASE;

	if (emmc_drv.host->dev_comp->emmcpllbase_t_tablebase > 0)
		t_table_reg_base = ((unsigned long)emmc_drv.host->emmcpllbase);

	emmc_fcie_init_hs400_5_1_gen_table(t_table_reg_base);
	//================================================
	//scan RXDLL
	//================================================

	u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs400 5.1 fail: %Xh\n", u32_err);
		return u32_err;
	}
	if (emmc_drv.chip_id == MT5896_CHIP_ID)
		emmc_debug(0, 0, "\nscan RX\n");
	else
		emmc_debug(0, 0, "\nscan RXDLL\n");

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;
		//using default skew2 to detect RXDLL
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, BIT_DEFAULT_SKEW2);

		emmc_fcie_set_delay_line(u32_i);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			au32_rx_dll_result |= (1 << u32_i);
			//emmc_debug(0, 1, "RXDLL: %d ok\n", u32_i);
		} else {
			emmc_debug(0, 1, "RXDLL: %d fail\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
	au8_cell_cnt = emmc_fcie_detect_hs400_timing_ex(au32_rx_dll_result);

	if (au8_cell_cnt < 7) {
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_NO_OK_DDR_PARAM;
	}

	emmc_fcie_set_hs400_5_1_gen_table_rx(au32_rx_dll_result);
	emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);
	//================================================
	if (emmc_drv.chip_id == MT5896_CHIP_ID)
		emmc_debug(0, 0, "\nscan TX\n");
	else
		emmc_debug(0, 0, "\nscan Skew2\n");
	//================================================
	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_skew2_result |= (1 << u32_i);
		} else {
			emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
			emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	#if 1
	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		if ((u32_skew2_result & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(0, 0, "\nOK\n");
	#endif

	u16_skew2_cnt = emmc_fcie_detect_hs400_timing_skew2_ex(u32_skew2_result);
	if (u16_skew2_cnt == 0) {
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_NO_OK_DDR_PARAM;
	}

	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, g_tab_set_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT);
	emmc_drv.t_table.set[0].u8_skew2 = g_tab_set_tmp_t.u8_skew2;

	//setup GEN_TIMING_TABLE
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u32_reg_address =
		(reg_emmcpll_0x03 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value =
		g_tab_set_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_mask = (0xF << BIT_SKEW2_SHIFT);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_op_code = REG_OP_SETBIT;

	emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);
	//==========================================
	u32_err = emmc_fcie_store_hs400_5_1_timing_table();

	return u32_err;
}

U32 emmc_fcie_detect_hs400_5_1_timing_rx(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_rx_dll_result;

	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = emmc_fcie_detect_hs400_5_1_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}

	EMMC_DISABLE_LOG(0);
	#endif
	emmc_debug(0, 0, "RX:\n");
	u32_i = 0;
	emmc_debug(0, 0, " %02u ", u32_i);
	for (u32_i = 1; u32_i < RXDLL_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	u32_rx_dll_result = 0;

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		if (u8_enable_error_log)//disable tuning error log
			EMMC_DISABLE_LOG(1);

		emmc_fcie_set_delay_line(u32_i);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_rx_dll_result |= (1 << u32_i);
		} else {
			u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
				EMMC_DISABLE_LOG(0);
				emmc_debug(0, 1, "eMMC Err: RXDLL: %d fail\n", u32_i);
				break;
			}
		}
	}
	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE; // turn off tuning flag
	EMMC_DISABLE_LOG(0);

	for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
		if ((u32_rx_dll_result & (1 << u32_i)) == 0)
			printf(" X ");
		else
			printf(" O ");
	}
	printf("\n");

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_5_1_timing_skew1_skew2(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew2_result;
	U8  u8_skew1;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = emmc_fcie_detect_hs400_5_1_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "Skew2           %02u ", u32_i);
	for (u32_i = 1; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew1 = 0; u8_skew1 < TOTAL_PLL_SKEW4_CNT; u8_skew1++) {
		//--------------------------------------
		u32_skew2_result = 0;

		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;
			if (u8_enable_error_log)
				EMMC_DISABLE_LOG(1);

			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value = u32_i << 4;
			emmc_fcie_apply_timing_set(0);
			if (u8_skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1 - PLL_SKEW4_CNT);
			}

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
					emmc_debug(0, 1, "eMMC Err: SKEW2: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;
		EMMC_DISABLE_LOG(0);

		emmc_debug(0, 0, "Skew1: %02u |  ", u8_skew1);
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			if ((u32_skew2_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_5_1_timing_trigerlevel_rx(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_rx_dll_result;
	U8  triger_level;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = emmc_fcie_detect_hs400_5_1_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "RX                 %02u ", u32_i);
	for (u32_i = 1; u32_i < RXDLL_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (triger_level = 0; triger_level < TRIGGER_CNT; triger_level++) {
		if (u8_enable_error_log)//disable tuning error log
			EMMC_DISABLE_LOG(1);

		emmc_set_triger_level(triger_level);
		//--------------------------------------
		u32_rx_dll_result = 0;

		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			emmc_fcie_set_delay_line(u32_i);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_rx_dll_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: RXDLL: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "TrigerLevel: %02u |  ", triger_level);
		for (u32_i = 0; u32_i < RXDLL_CNT; u32_i++) {
			if ((u32_rx_dll_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");

		if (triger_level == 0)
			gu32_rx_dll_result_lv0 &= u32_rx_dll_result;

		gu32_rx_dll_result &= u32_rx_dll_result;
	}
	REG_FCIE_CLRBIT(reg_emmcpll_0x20, TIGER_LVL_MASK);

	return EMMC_ST_SUCCESS;
}
#endif

U32 emmc_fcie_m6e3_detect_hs400_timing_skew1_skew2(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew2_result;
	U8  u8_skew1;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		//HS400 use SKEW4 setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_skew4_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "Skew2           %02u ", u32_i);
	for (u32_i = 1; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew1 = 0; u8_skew1 < TOTAL_PLL_SKEW4_CNT; u8_skew1++) {
		//--------------------------------------
		u32_skew2_result = 0;

		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			if (u8_enable_error_log)//disable tuning error log
				EMMC_DISABLE_LOG(1);

			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value = u32_i << BIT_SKEW2_SHIFT;
			emmc_fcie_apply_timing_set(0);
			if (u8_skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1 - PLL_SKEW4_CNT);
			}

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					emmc_debug(0, 1, "eMMC Err: SKEW2: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew1: %02u |  ", u8_skew1);
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			if ((u32_skew2_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

static U32 emmc_fcie_m6e3_choose_hs400_mode(void)
{
	U32 u32_err = 0;

	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_drv.u32_drvflag |= DRV_FLAG_SPEED_HS400_DETECT;
		u32_err = emmc_fcie_build_hS200_timing_table();//DS mode uses HS200 timing table
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
			return u32_err;
		}

		//HS400 use SKEW4 setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_skew4_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
				emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}

		emmc_drv.u32_drvflag &= ~DRV_FLAG_SPEED_HS400_DETECT;
	}
	EMMC_DISABLE_LOG(0);

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_m6e3_detect_hs400_timing_skew4_skew1(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew1_result = 0;
	U8  u8_skew4;

	#if 1
	u32_err = emmc_fcie_m6e3_choose_hs400_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;
	#endif

	//==========================================

	u32_i = 0;
	emmc_debug(0, 0, "Skew1        %02u ", u32_i);
	for (u32_i = 1; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew4 = 0; u8_skew4 < TOTAL_PLL_SKEW4_CNT; u8_skew4++) {
		emmc_fcie_set_hs400_gen_table_skew4(u8_skew4);
		emmc_fcie_apply_timing_set(0);
		//--------------------------------------
		u32_skew1_result = 0;

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			if (u8_enable_error_log)//disable tuning error log
				EMMC_DISABLE_LOG(1);
			if (u32_i < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i - PLL_SKEW4_CNT);
			}
			if (emmc_cmd13_skew4(emmc_drv.u16_rca) == EMMC_ST_SUCCESS) {
				u32_skew1_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: SKEW1: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew4: %02u |  ", u8_skew4);
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew1_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	//==========================================
	#if 1
	emmc_drv.u32_last_err_code &= ~DRV_FLAG_WEAK_STRENGTH;
	EMMC_DISABLE_LOG(1);
	emmc_set_triger_level(0);
	EMMC_DISABLE_LOG(0);
	u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		return u32_err;
	}
	#endif

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_m6e3_detect_hs400_5_1_timing_skew1_skew2(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew2_result;
	U8  u8_skew1;
	#if 1
	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		u32_err = emmc_fcie_detect_hs400_5_1_skew4_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: eMMC_FCIE_DetectHS400 5.1 Timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
	EMMC_DISABLE_LOG(0);
	#endif

	u32_i = 0;
	emmc_debug(0, 0, "Skew2           %02u ", u32_i);
	for (u32_i = 1; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");

	//==========================================
	for (u8_skew1 = 0; u8_skew1 < TOTAL_PLL_SKEW4_CNT; u8_skew1++) {
		//--------------------------------------
		u32_skew2_result = 0;

		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			if (u8_enable_error_log)// disable tuning error log
				EMMC_DISABLE_LOG(1);

			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value = u32_i << BIT_SKEW2_SHIFT;
			emmc_fcie_apply_timing_set(0);
			if (u8_skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_skew1 - PLL_SKEW4_CNT);
			}

			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					emmc_debug(0, 1, "eMMC Err: SKEW2: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		EMMC_DISABLE_LOG(0);

		emmc_debug(0, 0, "Skew1: %02u |  ", u8_skew1);
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			if ((u32_skew2_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	return EMMC_ST_SUCCESS;
}

static U32 emmc_fcie_m6e3_switch_to_hs400_5_1_mode(void)
{
	U32 u32_err = EMMC_ST_SUCCESS;

	EMMC_DISABLE_LOG(1);
	emmc_drv.u16_mmc_mode = BIT_SD_DEFAULT_MODE_REG;
	emmc_platform_init();
	u32_err = emmc_fcie_init();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "FCIE Reset Fail!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_init_device_ex();
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Identify Error!!\n");
		emmc_fcie_err_handler_stop();
	}
	u32_err = emmc_fcie_enable_fast_mode(FCIE_EMMC_HS400_5_1);
	if (u32_err != EMMC_ST_SUCCESS) {
		//HS400 use SKEW4 setting for DAT[0-7]
		u32_err = emmc_fcie_detect_hs400_5_1_skew4_timing();
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: emmc_fcie_detect_hs400_timing fail: %Xh\n", u32_err);
			emmc_fcie_err_handler_stop();
		}
	}
	EMMC_DISABLE_LOG(0);
	return u32_err;
}

U32 emmc_fcie_m6e3_detect_hs400_5_1_timing_skew4_skew1(U8 u8_enable_error_log)
{
	U32 u32_i = 0, u32_err = 0, u32_skew1_result = 0;
	U8  u8_skew4;

	#if 1
	u32_err = emmc_fcie_m6e3_switch_to_hs400_5_1_mode();
	if (u32_err != EMMC_ST_SUCCESS)
		return u32_err;

	#endif
	//==========================================

	u32_i = 0;
	emmc_debug(0, 0, "Skew1        %02u ", u32_i);
	for (u32_i = 1; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02u ", u32_i);

	emmc_debug(0, 0, "\n");
	//==========================================
	for (u8_skew4 = 0; u8_skew4 < TOTAL_PLL_SKEW4_CNT; u8_skew4++) {
		emmc_fcie_set_hs400_gen_table_skew4(u8_skew4);
		emmc_fcie_apply_timing_set(0);
		//--------------------------------------
		u32_skew1_result = 0;

		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			if (u8_enable_error_log)//disable tuning error log
				EMMC_DISABLE_LOG(1);
			if (u32_i < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK);
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i - PLL_SKEW4_CNT);
			}
			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew1_result |= (1 << u32_i);
			} else {
				u32_err = emmc_fcie_reset_to_hs400_5_1((U8)emmc_drv.u16_clk_reg_val);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
					EMMC_DISABLE_LOG(0);
					emmc_debug(0, 1, "eMMC Err: SKEW1: %d fail\n", u32_i);
					break;
				}
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE; // turn off tuning flag
		EMMC_DISABLE_LOG(0);
		emmc_debug(0, 0, "Skew4: %02u |  ", u8_skew4);
		for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
			if ((u32_skew1_result & (1 << u32_i)) == 0)
				printf(" X ");
			else
				printf(" O ");
		}
		printf("\n");
	}

	//==========================================
	#if 1
	u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		return u32_err;
	}
	#endif

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_m6e3_fix_skew4_result(U32 u32_skew4_result)
{
	U16 u16_i;
	U16 u16_skew4_cnt = 0, u16_skew4_left_fail_cnt = 0, u16_start_bit = 0, u16_end_bit = 0;
	U32 u32_fix_skew4_result = 0;

	if (emmc_drv.revision == MT5896_CHIP_VER) {
		for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
			if (u32_skew4_result & (1 << u16_i))
				u16_skew4_cnt++;
		}
		for (u16_i = 0; u16_i < PLL_SKEW4_CNT; u16_i++) {
			if ((u32_skew4_result & (1 << u16_i)) == 0)
				u16_skew4_left_fail_cnt++;
		}
	} else {
		for (u16_i = 0; u16_i < RXDLL_CNT; u16_i++) {
			if (u32_skew4_result & (1 << u16_i))
				u16_skew4_cnt++;
		}
		for (u16_i = 0; u16_i < (RXDLL_CNT >> 1); u16_i++) {
			if ((u32_skew4_result & (1 << u16_i)) == 0)
				u16_skew4_left_fail_cnt++;
		}
		u16_skew4_cnt = (u16_skew4_cnt * 3) / 4;
		if (u16_skew4_cnt >= PLL_SKEW4_CNT)
			u16_skew4_cnt -= 1;
	}

	if (u16_skew4_cnt >= PLL_SKEW4_CNT) {
		u32_fix_skew4_result = 0x1FE;
	} else {
		u16_skew4_left_fail_cnt = u16_skew4_left_fail_cnt >> 1;
		if (u16_skew4_left_fail_cnt > (PLL_SKEW4_CNT - u16_skew4_cnt)) {
			u16_start_bit = (PLL_SKEW4_CNT - u16_skew4_cnt);
			u16_end_bit = PLL_SKEW4_CNT;
		} else {
			u16_start_bit = u16_skew4_left_fail_cnt;
			u16_end_bit = u16_start_bit + u16_skew4_cnt;
		}
		for (u16_i = u16_start_bit; u16_i < u16_end_bit; u16_i++)
			u32_fix_skew4_result |= (1 << u16_i);
	}

	return u32_fix_skew4_result;
}

static U32 emmc_fcie_detect_timing_skew4_not_wropped_ex(U32 u32_skew4_result,
							U8 *u8_skew4_idx,
							U8 *u8_reg2ch)
{
	U16 u16_i, u16_start_bit = START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT;

	for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
		if ((u32_skew4_result & 1 << u16_i) &&  u16_start_bit ==  START_BIT_DEFAULT)
			u16_start_bit = u16_i;
		else if (u16_start_bit !=  START_BIT_DEFAULT  && (u32_skew4_result & (1 << u16_i)) == 0)
			u16_end_bit = u16_i - 1;

		if (u16_end_bit != END_BIT_DEFAULT) {
			if ((u16_end_bit - u16_start_bit + 1) < M6E3_MIN_OK_SKEW_CNT) {
				//to ignore "discontinuous case"
				u16_start_bit =  START_BIT_DEFAULT;
				u16_end_bit = END_BIT_DEFAULT;
			} else {
				break;
			}
		}
	}
	if (u16_end_bit == END_BIT_DEFAULT) {
		if (u32_skew4_result & (1 << (u16_i - 1)))
			u16_end_bit = u16_i - 1;
		else
			u16_end_bit = u16_start_bit;
	}

	if ((u16_end_bit - u16_start_bit + 1) < M6E3_MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: Clk:%uMHz Case2 not enough skew4: %Xh %Xh\n",
			   emmc_drv.u32_clk_khz / EMMC_KHZ, u32_skew4_result, u16_end_bit - u16_start_bit + 1);
		return EMMC_ST_ERR_SKEW4;
	}

	if (((u16_start_bit + u16_end_bit) / 2) < PLL_SKEW4_CNT) {
		*u8_reg2ch = 0;
		*u8_skew4_idx = (u16_start_bit + u16_end_bit) / 2;
	} else {
		*u8_reg2ch = 1;
		*u8_skew4_idx = ((u16_start_bit + u16_end_bit) / 2) - PLL_SKEW4_CNT;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_timing_skew4_ex(U32 u32_skew4_result, U8 *u8_skew4_idx, U8 *u8_reg2ch)
{
	U16 u16_i, u16_start_bit =  START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT, u16_cnt = 0;
	U32 u32_err = 0;

LABEL_START:

	if ((u32_skew4_result & ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) == ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) {
		*u8_reg2ch = 1;
		*u8_skew4_idx = 0;
	} else if ((u32_skew4_result & BIT0) == 0 || (u32_skew4_result & (1 << (TOTAL_PLL_SKEW4_CNT - 1))) == 0) {
		u32_err = emmc_fcie_detect_timing_skew4_not_wropped_ex(u32_skew4_result, u8_skew4_idx, u8_reg2ch);

		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;
	} else {
		u16_cnt = 0;
		for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
			if (u32_skew4_result & (1 << u16_i)) {
				u16_cnt++;
			} else {
				u16_start_bit = u16_i - 1;
				break;
			}
		}
		for (u16_i = TOTAL_PLL_SKEW4_CNT - 1; u16_i > 0; u16_i--) {
			if (u32_skew4_result & (1 << u16_i)) {
				u16_cnt++;
			} else {
				u16_end_bit = u16_i + 1;
				break;
			}
		}

		if (u16_cnt < M6E3_MIN_OK_SKEW_CNT) {
			u32_skew4_result &= ~((1 << (u16_start_bit + 1)) - 1);
			u16_start_bit =  START_BIT_DEFAULT;
			u16_end_bit = END_BIT_DEFAULT;
			goto LABEL_START;
		} else {
			for (u16_i = u16_start_bit + 1; u16_i < u16_end_bit; u16_i++)
				u32_skew4_result &= ~(1 << u16_i);

			u16_start_bit =  START_BIT_DEFAULT;
			u16_end_bit = END_BIT_DEFAULT;
		}

		for (u16_i = 0; u16_i < TOTAL_PLL_SKEW4_CNT; u16_i++) {
			if ((u32_skew4_result & (1 << u16_i)) == 0 && u16_start_bit ==  START_BIT_DEFAULT)
				u16_start_bit = u16_i - 1;
			else if (u16_start_bit !=  START_BIT_DEFAULT && (u32_skew4_result & (1 << u16_i)))
				u16_end_bit = u16_i;

			if (u16_end_bit != END_BIT_DEFAULT)
				break;
		}

		if ((u16_start_bit + 1) > (TOTAL_PLL_SKEW4_CNT - u16_end_bit)) {
			*u8_reg2ch = 0;
			*u8_skew4_idx = (u16_start_bit - (TOTAL_PLL_SKEW4_CNT - u16_end_bit)) >> 1;
		} else {
			*u8_reg2ch = 1;
			*u8_skew4_idx = u16_end_bit +
				(((TOTAL_PLL_SKEW4_CNT - u16_end_bit) + u16_start_bit) >> 1) - PLL_SKEW4_CNT;
		}
	}

	if ((*u8_skew4_idx) >= PLL_SKEW4_CNT)
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Skew4:%u > %u\n", (*u8_skew4_idx), PLL_SKEW4_CNT);

	return EMMC_ST_SUCCESS;
}

static U32 emmc_fcie_detect_hs400_cmd13_rsp_skew4_ex(U8 u8_trg_lvl,
						     S8 *skew4_idx,
						     U8 skew1,
						     U32 *skew4_fail_result)
{
	U32 u32_i, u32_k, u32_err = 0, u32_skew4_fail_result;
	S8 s8_skew4_idx;
	U8 u8_trg_lvl_value[3] = {0, 3, 7};

	s8_skew4_idx = *skew4_idx;
	u32_skew4_fail_result = *skew4_fail_result;

	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		for (u32_k = 0; u32_k < M6E3_MIN_OK_SKEW_CNT; u32_k++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW1_MASK | BIT_SKEW4_MASK);
			if (skew1 < PLL_SKEW4_CNT) {
				REG_FCIE_CLRBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_SETBIT(reg_emmcpll_0x03, skew1);
			} else {
				REG_FCIE_SETBIT(reg_emmcpll_0x69, BIT3);//reg_clk_dig_inv
				REG_FCIE_SETBIT(reg_emmcpll_0x03, skew1 - PLL_SKEW4_CNT);
			}

			if (u32_i >= PLL_SKEW4_CNT) {
				#ifdef REG_ANL_SKEW4_INV
				REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
				#endif
				REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);
			} else {
				#ifdef REG_ANL_SKEW4_INV
				REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
				#endif
				REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);
			}
			emmc_set_triger_level(u8_trg_lvl_value[u8_trg_lvl]);
			u32_err = emmc_cmd13_skew4(emmc_drv.u16_rca);
			if (u32_err != EMMC_ST_SUCCESS) {
				s8_skew4_idx = (S32)u32_i - (S32)skew1;
				if (s8_skew4_idx < 0)
					s8_skew4_idx += TOTAL_PLL_SKEW4_CNT;

				u32_skew4_fail_result |= (1 << s8_skew4_idx);
				goto LABEL_END;
			}
		}
	}

LABEL_END:

	*skew4_idx = s8_skew4_idx;
	*skew4_fail_result = u32_skew4_fail_result;
	return u32_err;
}

static U32 emmc_fcie_detect_hs400_rsp_skew4_ex(U8 *u8_skew4_idx, U8 *u8_reg2ch, U16 *u16_skew4_good_cnt)
{
	U32 u32_j, u32_err = 0, u32_skew4_fail_result = 0;
	S8 s8_skew4_idx = 0, s8_skew4_ori = 0;
	U8 u8_trg_lvl, u8_drv_str;

	//==========================================
	//add skew4 for hs400 using hs200 + pad setting of HS400 to detect skew4
	for (u8_drv_str = 0; u8_drv_str < 2; u8_drv_str++) {
		emmc_drv.u32_last_err_code = 0;
		if (u8_drv_str)
			emmc_drv.u32_last_err_code |= DRV_FLAG_WEAK_STRENGTH;

		for (u8_trg_lvl = 0; u8_trg_lvl < 3; u8_trg_lvl++) {
			//scan skew4 for hs400 rsp using skew1 & skew4
			//skew1
			for (u32_j = 0; u32_j < TOTAL_PLL_SKEW4_CNT; u32_j++) {
				u32_err = emmc_fcie_detect_hs400_cmd13_rsp_skew4_ex(u8_trg_lvl,
										    &s8_skew4_idx,
										    u32_j,
										    &u32_skew4_fail_result);
				if (u32_err)
					goto LABEL_END;
			}
		}
	}

LABEL_END:

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	if (u32_skew4_fail_result == 0) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Warn: The result of scanned response in HS400 is all pass\n");
		s8_skew4_ori = PLL_SKEW4_CNT;
	} else {
		g_tab_set_skew4_tmp_t.u32_skew4_result_cmd &= ~(1 << s8_skew4_idx);
		s8_skew4_idx -= PLL_SKEW4_CNT;
		if (s8_skew4_idx < 0)
			s8_skew4_idx += TOTAL_PLL_SKEW4_CNT;

		s8_skew4_ori = (U8)s8_skew4_idx;
		(*u16_skew4_good_cnt) -= 1;
	}

	if (s8_skew4_ori < PLL_SKEW4_CNT) {
		*u8_reg2ch = 0;
		*u8_skew4_idx = (U8)s8_skew4_ori;
	} else {
		*u8_reg2ch = 1;
		*u8_skew4_idx = (U8)(s8_skew4_ori - PLL_SKEW4_CNT);
	}
	emmc_drv.u32_last_err_code &= ~DRV_FLAG_WEAK_STRENGTH;
	emmc_set_triger_level(0);
	u32_err = emmc_fcie_reset_to_hs400((U8)emmc_drv.u16_clk_reg_val);
	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		return u32_err;
	}

	return u32_err;
}

static U32 emmc_fcie_detect_hs400_rsp_skew4(u16 *u16_rsp_ok_cnt)
{
	U32 u32_i, u32_j, u32_err = 0, u32_skew4_result = 0;
	U16 u16_skew4_good_cnt = 0;

	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);

		for (u32_j = 0; u32_j < M6E3_MIN_OK_SKEW_CNT; u32_j++) {
			u32_err = emmc_cmd13_skew4(emmc_drv.u16_rca);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
			if (u32_err == EMMC_ST_SUCCESS) {
				u32_skew4_result |= (1 << u32_i);
				u16_skew4_good_cnt++;
			} else {
				emmc_debug(0, 1, "failed data skew4: %u\n", u32_i);
				u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
				if (u32_err != EMMC_ST_SUCCESS) {
					emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
						   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				return u32_err;
			}
		}
	}

	for (u32_i = PLL_SKEW4_CNT; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);

		for (u32_j = 0; u32_j < M6E3_MIN_OK_SKEW_CNT; u32_j++) {
			u32_err = emmc_cmd13_skew4(emmc_drv.u16_rca);
			if (u32_err != EMMC_ST_SUCCESS)
				break;
		}
		if (u32_err == EMMC_ST_SUCCESS) {
			u32_skew4_result |= (1 << u32_i);
			u16_skew4_good_cnt++;
		} else {
			emmc_debug(0, 1, "failed data skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				return u32_err;
			}
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	if (u32_skew4_result == 0 || u16_skew4_good_cnt < M6E3_MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: Clk: %uMHz, CMD Skew4Result: %Xh, CMD Skew4Cnt: %u\n",
		emmc_drv.u32_clk_khz / EMMC_KHZ,
		u32_skew4_result,
		u16_skew4_good_cnt);
		return EMMC_ST_ERR_SKEW4;
	}
	g_tab_set_skew4_tmp_t.u32_skew4_result_cmd = u32_skew4_result;

	if ((u32_skew4_result & ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) == ((1 << TOTAL_PLL_SKEW4_CNT) - 1)) {
		u32_err = emmc_fcie_detect_hs400_rsp_skew4_ex(&g_tab_set_skew4_tmp_t.u8_skew4_rsp,
							      &g_tab_set_skew4_tmp_t.u8_reg_2ch_rsp,
							      &u16_skew4_good_cnt);
	} else {
		u32_err = emmc_fcie_detect_timing_skew4_ex(u32_skew4_result,
							   &g_tab_set_skew4_tmp_t.u8_skew4_rsp,
							   &g_tab_set_skew4_tmp_t.u8_reg_2ch_rsp);
	}
	*u16_rsp_ok_cnt = u16_skew4_good_cnt;

	return u32_err;
}

static U32 emmc_fcie_detect_hs400_data_skew4(void)
{
	U32 u32_i, u32_err = 0, u32_skew4_result = 0;
	U16 u16_skew4_good_cnt = 0;

	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_skew4_result |= (1 << u32_i);
			u16_skew4_good_cnt++;
		} else {
			emmc_debug(0, 1, "failed data skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				return u32_err;
			}
		}
	}

	for (u32_i = PLL_SKEW4_CNT; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		#ifdef REG_ANL_SKEW4_INV
		REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
		#endif

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u32_skew4_result |= (1 << u32_i);
			u16_skew4_good_cnt++;
		} else {
			emmc_debug(0, 1, "failed data skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);

			if (u32_err != EMMC_ST_SUCCESS) {
				emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
					   "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
				return u32_err;
			}
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE; //turn off tuning flag

	if (u32_skew4_result == 0 || u16_skew4_good_cnt < M6E3_MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
			   "eMMC Err: Clk: %uMHz, DATA Skew4Result: %Xh, DATA Skew4Cnt: %u\n",
		emmc_drv.u32_clk_khz / EMMC_KHZ,
		u32_skew4_result,
		u16_skew4_good_cnt);
		return EMMC_ST_ERR_SKEW4;
	}

	g_tab_set_skew4_tmp_t.u32_skew4_result_data = u32_skew4_result;
		return emmc_fcie_detect_timing_skew4_ex(u32_skew4_result,
							&g_tab_set_skew4_tmp_t.u8_skew4_data,
							&g_tab_set_skew4_tmp_t.u8_reg_2ch_data);
}

U32 emmc_fcie_detect_hs400_5_1_skew4(void)
{
	U32 u32_i, u32_err, u32_skew4_result = 0;
	U16 u16_skew4_cnt = 0;

	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; //turn on tuning flag

		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
			REG_FCIE_CLRBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
					emmc_drv.host->dev_comp->analog_skew4_reg_bit);
		#endif

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW4_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u16_skew4_cnt++;
			u32_skew4_result |= (1 << u32_i);
		} else {
			emmc_debug(0, 1, "failed skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);

			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
	}

	for (u32_i = PLL_SKEW4_CNT; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		//make sure a complete outside clock cycle
		REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

		#ifdef REG_ANL_SKEW4_INV
		if (emmc_drv.host->dev_comp->analog_skew4)
			REG_FCIE_SETBIT(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset],
					emmc_drv.host->dev_comp->analog_skew4_reg_bit);
		#endif
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u32_i - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);

		if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
			u16_skew4_cnt++;
			u32_skew4_result |= (1 << u32_i);
		} else {
			emmc_debug(0, 1, "failed skew4: %u\n", u32_i);
			u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
			if (u32_err != EMMC_ST_SUCCESS)
				return u32_err;
		}
	}

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

	if (u32_skew4_result == 0 || u16_skew4_cnt < M6E3_MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Clk: %uMHz SkewResult: %Xh SkewCnt: %u\n",
			   emmc_drv.u32_clk_khz / EMMC_KHZ, u32_skew4_result, u16_skew4_cnt);
		return EMMC_ST_ERR_SKEW4;
	}
	g_tab_set_tmp_t.u32_scan_result = u32_skew4_result;
	return emmc_fcie_detect_timing_skew4_ex(u32_skew4_result, &g_tab_set_tmp_t.u8_skew4, &g_tab_set_tmp_t.u8_reg2ch);
}

static void emmc_fcie_set_m6e3_hs400_table_rsp(U16 u16_rsp_ok_cnt)
{
	U32 u32_i;

	emmc_drv.t_hs400_table.u16_rsp_valid_cnt = u16_rsp_ok_cnt;
	emmc_drv.t_hs400_table.set_skew4.u32_skew4_result_cmd =
		g_tab_set_skew4_tmp_t.u32_skew4_result_cmd;
	emmc_drv.t_hs400_table.set_skew4.u8_skew4_rsp =
		g_tab_set_skew4_tmp_t.u8_skew4_rsp;
	emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_rsp =
		g_tab_set_skew4_tmp_t.u8_reg_2ch_rsp;

	emmc_debug(0, 0, "\nscan RSP\n");
	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		if ((g_tab_set_skew4_tmp_t.u32_skew4_result_cmd & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}

	emmc_debug(0, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
}

static void emmc_fcie_set_m6e3_hs400_table_rx(void)
{
	U32 u32_i;

	emmc_drv.t_hs400_table.set_skew4.u32_skew4_result_data =
		g_tab_set_skew4_tmp_t.u32_skew4_result_data;

	emmc_drv.t_hs400_table.set_skew4.u8_skew4_data =
		g_tab_set_skew4_tmp_t.u8_skew4_data;
	emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_data =
		g_tab_set_skew4_tmp_t.u8_reg_2ch_data;
	emmc_drv.t_hs400_table.set_skew4.u8_skew2 =
		g_tab_set_skew4_tmp_t.u8_skew2;

	emmc_debug(0, 0, "\nscan RX\n");
	#if 1
	emmc_drv.u32_fix_skew4_result =
		emmc_fcie_m6e3_fix_skew4_result(g_tab_set_skew4_tmp_t.u32_skew4_result_data);
	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");
	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		if ((emmc_drv.u32_fix_skew4_result & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	#else
	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < TOTAL_PLL_SKEW4_CNT; u32_i++) {
		if ((g_tab_set_skew4_tmp_t.u32_skew4_result_data & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	#endif
	emmc_debug(0, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
}

static void emmc_fcie_set_m6e3_hs400_table_tx(U32 u32_skew2_result)
{
	U32 u32_i;

	emmc_drv.t_hs400_table.set_skew4.u8_skew2 = g_tab_set_skew4_tmp_t.u8_skew2;
	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
	if ((u32_skew2_result & (1 << u32_i)) == 0)
		emmc_debug(0, 0, " X ");
	else
		emmc_debug(0, 0, " O ");
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");

	emmc_drv.t_hs400_table.u32_ver_no = EMMC_M6E3_TIMING_TABLE_VERSION;
	memcpy(emmc_drv.t_hs400_table.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1);
	emmc_drv.t_hs400_table.u8_set_cnt++;
}

U32 emmc_fcie_detect_hs400_skew4_timing(void)
{
	U32 u32_err = 0, u32_ret = 0, u32_i, u32_skew2_result = 0;
	U16 u16_skew2_cnt = 0, u16_rsp_ok_cnt = 0;

	emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Info: building M6E3 HS400 table, please wait...\n");
	memset((void *)&emmc_drv.t_hs400_table, 0, sizeof(emmc_drv.t_hs400_table));

	if (emmc_drv.host->id_matched_driving <= 4)
		emmc_drv.t_hs400_table.u32_device_driving = emmc_drv.host->id_matched_driving;
	else
		emmc_drv.t_hs400_table.u32_device_driving = 0;

	emmc_drv.t_hs400_table.set_skew4.u16_clk = gau8_emmc_pll_sel_200[0];

	//--------------------------------
	u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		emmc_fcie_reset_to_legacy();
		return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nclk: %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);
	u32_err = emmc_fcie_detect_hs400_rsp_skew4(&u16_rsp_ok_cnt);

	if (u32_err == EMMC_ST_SUCCESS) {
		emmc_fcie_set_m6e3_hs400_table_rsp(u16_rsp_ok_cnt);
	} else {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Scan the response of skew4 fail in HS400\n");
		goto LBEL_END;
	}

	//using default skew2 to detect SKEW4
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, BIT_DEFAULT_SKEW2);
	u32_err = emmc_fcie_detect_hs400_data_skew4();

	if (u32_err == EMMC_ST_SUCCESS) {
		emmc_fcie_set_m6e3_hs400_table_rx();
		emmc_fcie_set_skew4_data_reg();
		//================================================
		emmc_debug(0, 0, "\nscan TX\n");
		//================================================
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);
				u32_err = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);

				if (u32_err != EMMC_ST_SUCCESS)
					break;
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag

		u16_skew2_cnt = emmc_fcie_m6e3_detect_hs400_timing_skew2_ex(u32_skew2_result);
		if (u16_skew2_cnt == 0) {
			emmc_fcie_reset_to_legacy();
			return EMMC_ST_ERR_NO_OK_DDR_PARAM;
		}

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, g_tab_set_skew4_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT);
		emmc_fcie_set_m6e3_hs400_table_tx(u32_skew2_result);
		emmc_fcie_set_tx_reg();
		emmc_dump_timing_table();

		emmc_drv.t_hs400_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_hs400_table,
								 sizeof(emmc_drv.t_hs400_table) - 8);
		memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_hs400_table, sizeof(emmc_drv.t_hs400_table));

		u32_err = emmc_cmd24(EMMC_HS400SKEW4TABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
		u32_ret = emmc_cmd24(EMMC_HS400SKEW4TABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);

		if (u32_err != EMMC_ST_SUCCESS && u32_ret !=  EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: Save TTable fail: %Xh %Xh\n", u32_err, u32_ret);
			goto LBEL_END;
		}

		u32_err = emmc_stored_crazy_pattern(EMMC_CRAZY_PATTERN_BLK);
		if (u32_err != EMMC_ST_SUCCESS) {
			emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1,
				   "eMMC Err: Save crazy pattern fail: %Xh\n", u32_err);
			goto LBEL_END;
		}
	} else {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Scan RX/TX fail in HS400\n");
		goto LBEL_END;
	}

	return EMMC_ST_SUCCESS;
LBEL_END:

	emmc_fcie_reset_to_legacy();

	return EMMC_ST_ERR_SKEW4;
}

static void emmc_fcie_init_m6e3_hs400_5_1_gen_table(unsigned long t_table_reg_base)
{
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Info: building M6E3 HS400-ES table, please wait...\n");

	memset((void *)&emmc_drv.t_table, 0, sizeof(emmc_drv.t_table));
	memset((void *)&emmc_drv.t_table_g, 0, sizeof(emmc_drv.t_table_g));
	emmc_drv.t_table_g.u8_set_cnt = 1;//initial set count for apply general register
	emmc_drv.t_table_g.u8_register_cnt = 4;//skew4 & its inverse register, skew2
	emmc_drv.t_table_g.u8_cur_set_idx = 0;
	emmc_drv.t_table_g.u32_clk = gau8_emmc_pll_sel_200[0];

	if (emmc_drv.host->id_matched_driving <= 4)
		emmc_drv.t_table_g.u32_device_driving = emmc_drv.host->id_matched_driving;
	else
		emmc_drv.t_table_g.u32_device_driving = 0;

	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u32_reg_address = (reg_emmcpll_0x09 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_reg_value   = BIT_RXDLL_MASK;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_reg_mask    = BIT_RXDLL_MASK;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_RXDLL_VALUE].u16_op_code     = REG_OP_CLRBIT;
}

static void emmc_fcie_m6e3_hs400_5_1_gen_table_rx(unsigned long t_table_reg_base)
{
	U32 u32_i;

	emmc_drv.t_table.u8_set_cnt++;
	emmc_drv.t_table.set[0].u8_clk = gau8_emmc_pll_sel_200[0];
	emmc_drv.t_table.set[0].u8_reg2ch = g_tab_set_tmp_t.u8_reg2ch;
	emmc_drv.t_table.set[0].u8_skew4 = g_tab_set_tmp_t.u8_skew4;
	emmc_drv.t_table.set[0].u32_scan_result = g_tab_set_tmp_t.u32_scan_result;
	emmc_drv.t_table.u32_ver_no = EMMC_M6E3_TIMING_TABLE_VERSION;

	//setup GEN_TIMING_TABLE
	//skew4 invesre
	#ifdef REG_ANL_SKEW4_INV
	if (emmc_drv.host->dev_comp->analog_skew4) {
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u32_reg_address = ((unsigned long)
			(&emmc_drv.host->emmcpllbase->emmcpll_rx[emmc_drv.host->dev_comp->analog_skew4_reg_offset]) -
			t_table_reg_base);
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_reg_value =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_reg_mask =
			emmc_drv.host->dev_comp->analog_skew4_reg_bit;

		if (!g_tab_set_tmp_t.u8_reg2ch)
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_CLRBIT;
		else
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_SETBIT;
	}
	#endif
	//skew4 value
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u32_reg_address = (reg_emmcpll_0x03 - t_table_reg_base);
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value = g_tab_set_tmp_t.u8_skew4 << BIT_SKEW4_SHIFT;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_mask = BIT_SKEW4_MASK;
	emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_op_code = REG_OP_SETBIT;

	emmc_debug(0, 0, "\nscan RX\n");
	#if 1
	emmc_drv.u32_fix_skew4_result =
		emmc_fcie_m6e3_fix_skew4_result(emmc_drv.t_table.set[0].u32_scan_result);
	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < PLL_SKEW4_CNT; u32_i++) {
		if ((emmc_drv.u32_fix_skew4_result & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
	#endif
}

static void emmc_fcie_m6e3_hs400_5_1_print_tx_result(U32 u32_skew2_result)
{
	u32 u32_i;

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++)
		emmc_debug(0, 0, "%02X ", u32_i);

	emmc_debug(0, 0, "\n");

	for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
		if ((u32_skew2_result & (1 << u32_i)) == 0)
			emmc_debug(0, 0, " X ");
		else
			emmc_debug(0, 0, " O ");
	}
	emmc_debug(0, 0, "\n");
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
}

static U32 emmc_fcie_m6e3_store_hs400_5_1_table(void)
{
	U32 u32_ret = 0, u32_err = 0;

	emmc_drv.t_table.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table,
						   sizeof(emmc_drv.t_table) - 8);
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table, sizeof(emmc_drv.t_table));
	u32_err = emmc_cmd24(EMMC_HS400TABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400TABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);
	if (u32_err != EMMC_ST_SUCCESS && u32_ret !=  EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	emmc_drv.t_table_g.u32_ver_no = EMMC_M6E3_TIMING_TABLE_VERSION;
	emmc_drv.t_table_g.u8_speed_mode = FCIE_EMMC_HS400_5_1;
	memcpy(emmc_drv.t_table_g.au8_cid, emmc_drv.au8_cid, EMMC_MAX_RSP_BYTE_CNT - 1);
	emmc_dump_timing_table();

	emmc_drv.t_table_g.u32_chksum = emmc_chk_sum((U8 *)&emmc_drv.t_table_g.u32_ver_no,
						     sizeof(emmc_drv.t_table_g) - sizeof(U32));
	memcpy(GLOBAL_EMMC_SECTORBUF, (U8 *)&emmc_drv.t_table_g, sizeof(emmc_drv.t_table_g));

	u32_err = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_0, GLOBAL_EMMC_SECTORBUF);
	u32_ret = emmc_cmd24(EMMC_HS400EXTTABLE_BLK_1, GLOBAL_EMMC_SECTORBUF);

	if (u32_err != EMMC_ST_SUCCESS && u32_ret != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save TTable fail: %Xh %Xh\n",
			   u32_err, u32_ret);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	u32_err = emmc_stored_crazy_pattern(EMMC_CRAZY_PATTERN_BLK);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Save crazy pattern fail: %Xh\n", u32_err);
		emmc_fcie_reset_to_legacy();
		return EMMC_ST_ERR_SAVE_DDRT_FAIL;
	}

	return EMMC_ST_SUCCESS;
}

U32 emmc_fcie_detect_hs400_5_1_skew4_timing(void)
{
	U32 u32_err = 0, u32_i, u32_skew2_result = 0;
	U16 u16_skew2_cnt = 0;
	unsigned long t_table_reg_base = RIU_BASE;

	if (emmc_drv.host->dev_comp->emmcpllbase_t_tablebase  > 0)
		t_table_reg_base = ((unsigned long)emmc_drv.host->emmcpllbase);

	emmc_fcie_init_m6e3_hs400_5_1_gen_table(t_table_reg_base);
	//--------------------------------
	u32_err = emmc_fcie_reset_to_hs400_5_1(gau8_emmc_pll_sel_200[0]);

	if (u32_err != EMMC_ST_SUCCESS) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: emmc_fcie_reset_to_hs400 fail: %Xh\n", u32_err);
		emmc_fcie_reset_to_legacy();
		return u32_err;
	}
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nclk: %uMHz\n", emmc_drv.u32_clk_khz / EMMC_KHZ);

	//using default skew2 to detect SKEW4
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x03, BIT_DEFAULT_SKEW2);

	//---------------------------
	//search and set the Windows
	u32_err = emmc_fcie_detect_hs400_5_1_skew4();
	//---------------------------
	//set the Table
	if (u32_err == EMMC_ST_SUCCESS) {
		emmc_fcie_m6e3_hs400_5_1_gen_table_rx(t_table_reg_base);
		emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

		//================================================
		emmc_debug(0, 0, "\nscan TX\n");
		//================================================
		for (u32_i = 0; u32_i < SKEW2_CNT; u32_i++) {
			emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
			REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u32_i << BIT_SKEW2_SHIFT);

			if (emmc_test_blk_wrc_probe_timing(EMMC_TEST_BLK_0) == EMMC_ST_SUCCESS) {
				u32_skew2_result |= (1 << u32_i);
			} else {
				emmc_debug(0, 1, "Skew2: %d fail\n", u32_i);
				u32_err = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
				emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

				if (u32_err != EMMC_ST_SUCCESS)
					break;
			}
		}
		emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
		emmc_fcie_m6e3_hs400_5_1_print_tx_result(u32_skew2_result);
		u16_skew2_cnt = emmc_fcie_detect_hs400_timing_skew2_ex(u32_skew2_result);
		if (u16_skew2_cnt == 0) {
			emmc_fcie_reset_to_legacy();
			return EMMC_ST_ERR_NO_OK_DDR_PARAM;
		}

		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, g_tab_set_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT);

		emmc_drv.t_table.set[0].u8_skew2 = g_tab_set_tmp_t.u8_skew2;
		//setup GEN_TIMING_TABLE
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u32_reg_address =
			(reg_emmcpll_0x03 - t_table_reg_base);
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_value =
			g_tab_set_tmp_t.u8_skew2 << BIT_SKEW2_SHIFT;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_reg_mask = BIT_SKEW2_MASK;
		emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW2_VALUE].u16_op_code = REG_OP_SETBIT;

		emmc_fcie_apply_reg(emmc_drv.t_table_g.u8_cur_set_idx);

		u32_err = emmc_fcie_m6e3_store_hs400_5_1_table();
		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		return EMMC_ST_SUCCESS;
	}

	emmc_fcie_reset_to_legacy();
	return EMMC_ST_ERR_SKEW4;
}

U32 emmc_fcie_m6e3_detect_hs400_timing_skew2_ex(U32 u32_skew2_result)
{
	U16 u16_i = 0, u16_start_bit =  START_BIT_DEFAULT, u16_end_bit = END_BIT_DEFAULT, u16_cnt = 0;

	if ((u32_skew2_result & ((1 << SKEW2_CNT) - 1)) == ((1 << SKEW2_CNT) - 1)) {
		emmc_debug(EMMC_DEBUG_LEVEL, 0, " all\n");
		g_tab_set_skew4_tmp_t.u8_skew2 = 4;
		u16_cnt = SKEW2_CNT;
	} else {
		do {
			if ((u32_skew2_result & (1 << u16_i)) &&
			    u16_start_bit == START_BIT_DEFAULT) {
				u16_start_bit = u16_i;
			} else if (u16_start_bit != START_BIT_DEFAULT &&
				   (u32_skew2_result & (1 << u16_i)) == 0) {
				u16_end_bit = u16_i - 1;
				break;
			}
			u16_i++;
		} while (u16_i < (SKEW2_CNT + 1));

		if (u16_end_bit != END_BIT_DEFAULT) {
			g_tab_set_skew4_tmp_t.u8_skew2 = u16_start_bit + ((u16_end_bit - u16_start_bit) >> 1);
				u16_cnt = u16_end_bit - u16_start_bit + 1;
		}
	}

	return u16_cnt;
}

static U32 emmc_fcie_m6e3_probe_timing(U16 u16_skew4)
{
	U32 u32_err = 0, u32_arg = 0;
	U8 u8_partition_config = 0, u8_switch_to_user = 0;

	//make sure a complete outside clock cycle
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
	emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag

	if (emmc_drv.u8_partition_config & PART_ACCESS_MASK)
		u8_switch_to_user = 1;

	#ifdef REG_ANL_SKEW4_INV
	if (u16_skew4 < PLL_SKEW4_CNT)
		REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
	else
		REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
	#endif
	REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
	if (u16_skew4 < PLL_SKEW4_CNT)
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u16_skew4 << BIT_SKEW4_SHIFT);
	else
		REG_FCIE_SETBIT(reg_emmcpll_0x03, (u16_skew4 - PLL_SKEW4_CNT) << BIT_SKEW4_SHIFT);

	if (u8_switch_to_user) {
		u8_partition_config = emmc_drv.u8_partition_config;
		u8_partition_config &= ~PART_ACCESS_MASK;
		u32_arg = (3 << 24) | (EXT_CSD_PART_CONF << 16) | (u8_partition_config << 8);

		u32_err = emmc_cmd6_skew4(u32_arg);
		if (u32_err != EMMC_ST_SUCCESS)
			goto LABEL_END;
	}

	u32_err = emmc_read_crazy_pattern(EMMC_CRAZY_PATTERN_BLK);

LABEL_END:
	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
	return u32_err;
}

static U16 emmc_fcie_m6e3_skew4_start_bit(int start_bit)
{
	U16 u16_start_bit = 0;

	if (start_bit < 0)
		u16_start_bit = (U16)(start_bit + TOTAL_PLL_SKEW4_CNT);
	else if (start_bit > (TOTAL_PLL_SKEW4_CNT - 1))
		u16_start_bit = (U16)(TOTAL_PLL_SKEW4_CNT - start_bit);
	else
		u16_start_bit = (U16)start_bit;

	return u16_start_bit;
}

static U32 emmc_fcie_m6e3_skew4_need_to_reset(U32 u32_err)
{
	U32 ret = 0;

	if (u32_err == EMMC_ST_ERR_CMD17_RSP ||
	    u32_err == EMMC_ST_ERR_CMD17_DATA_CRC ||
	    u32_err == EMMC_ST_ERR_CMD6_RSP) {
		ret = 1;
	} else {
		ret = 0;
	}

	return ret;
}

static U32 emmc_fcie_m6e3_skew4_right_result(U16 u16_start_bit,
					     U16 *u16_next_bit,
					     U16 *u16_skew4_cnt,
					     U32 *u32_skew4_result,
					     U32 *u32_scanned_skew4)
{
	U32 u32_err = 0, u32_ret = 0;
	int i, cur_start_bit = 0;

	for (i = u16_start_bit; i < TOTAL_PLL_SKEW4_CNT; i++) {
		u32_err = emmc_fcie_m6e3_probe_timing((U16)i);
		cur_start_bit = i;
		(*u32_scanned_skew4) |= (1 << i);
		if (!u32_err) {
			(*u16_skew4_cnt) += 1;
			(*u32_skew4_result) |= (1 << i);

			if ((*u16_skew4_cnt) >= M6E3_MIN_OK_SKEW_CNT)
				goto LABEL_END;
		} else {
			if (emmc_fcie_m6e3_skew4_need_to_reset(u32_err)) {
				u32_ret = emmc_fcie_reset();
			} else {
				if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
					u32_ret = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				} else {
					u32_ret = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				}
			}
			if (u32_ret)
				goto LABEL_END;

			if ((*u16_skew4_cnt) < M6E3_MIN_OK_SKEW_CNT) {
				//to ignore "discontinuous case"
				*u16_skew4_cnt = 0;
			}
			goto LABEL_END;
		}
	}

	for (i = 0; i < u16_start_bit; i++) {
		u32_err = emmc_fcie_m6e3_probe_timing((U16)i);
		cur_start_bit = i;
		(*u32_scanned_skew4) |= (1 << i);
		if (!u32_err) {
			(*u16_skew4_cnt) += 1;
			(*u32_skew4_result) |= (1 << i);
			if ((*u16_skew4_cnt) >= M6E3_MIN_OK_SKEW_CNT)
				break;
		} else {
			if (emmc_fcie_m6e3_skew4_need_to_reset(u32_err)) {
				u32_ret = emmc_fcie_reset();
			} else {
				if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
					u32_ret = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				} else {
					u32_ret = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				}
			}
			if (u32_ret)
				break;

			if ((*u16_skew4_cnt) < M6E3_MIN_OK_SKEW_CNT) {
				//to ignore "discontinuous case"
				*u16_skew4_cnt = 0;
			}
			break;
		}
	}

LABEL_END:

	cur_start_bit += 1;
	*u16_next_bit = emmc_fcie_m6e3_skew4_start_bit(cur_start_bit);
	return u32_ret;
}

static U32 emmc_fcie_m6e3_skew4_left_result(U16 u16_start_bit,
					    U16 *u16_next_bit,
					    U16 *u16_skew4_cnt,
					    U32 *u32_skew4_result,
					    U32 *u32_scanned_skew4)
{
	U32 u32_err = 0, u32_ret = 0;
	int i, cur_start_bit = 0;

	for (i = u16_start_bit; i >= 0; i--) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE;//turn on tuning flag
		u32_err = emmc_fcie_m6e3_probe_timing((U16)i);
		cur_start_bit = i;
		(*u32_scanned_skew4) |= (1 << i);
		if (!u32_err) {
			(*u16_skew4_cnt) += 1;
			(*u32_skew4_result) |= (1 << i);
			if ((*u16_skew4_cnt) >= M6E3_MIN_OK_SKEW_CNT)
				goto LABEL_END;
		} else {
			if (emmc_fcie_m6e3_skew4_need_to_reset(u32_err)) {
				u32_ret = emmc_fcie_reset();
			} else {
				if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
					u32_ret = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret != EMMC_ST_SUCCESS)
						return u32_err;
				} else {
					u32_ret = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret != EMMC_ST_SUCCESS)
						return u32_err;
				}
			}
			if (u32_ret)
				goto LABEL_END;

			if ((*u16_skew4_cnt) < M6E3_MIN_OK_SKEW_CNT) {
				//to ignore "discontinuous case"
				*u16_skew4_cnt = 0;
			}
			goto LABEL_END;
		}
	}

	for (i = (TOTAL_PLL_SKEW4_CNT - 1); i > u16_start_bit; i--) {
		emmc_drv.u32_drvflag |= DRV_FLAG_TUNING_TTABLE; //turn on tuning flag

		u32_err = emmc_fcie_m6e3_probe_timing((U16)i);
		cur_start_bit = i;
		(*u32_scanned_skew4) |= (1 << i);
		if (!u32_err) {
			(*u16_skew4_cnt) += 1;
			(*u32_skew4_result) |= (1 << i);
			if ((*u16_skew4_cnt) >= M6E3_MIN_OK_SKEW_CNT)
				break;
		} else {
			if (emmc_fcie_m6e3_skew4_need_to_reset(u32_err)) {
				u32_ret = emmc_fcie_reset();
			} else {
				if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
					u32_ret = emmc_fcie_reset_to_hs400((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				} else {
					u32_ret = emmc_fcie_reset_to_hs400_5_1((U8)gau8_emmc_pll_sel_200[0]);
					if (u32_ret !=  EMMC_ST_SUCCESS)
						return u32_err;
				}
			}
			if (u32_ret)
				break;

			if ((*u16_skew4_cnt) < M6E3_MIN_OK_SKEW_CNT) {
				//to ignore "discontinuous case"
				*u16_skew4_cnt = 0;
			}
			break;
		}
	}

LABEL_END:

	emmc_drv.u32_drvflag &= ~DRV_FLAG_TUNING_TTABLE;//turn off tuning flag
	cur_start_bit -= 1;
	*u16_next_bit = emmc_fcie_m6e3_skew4_start_bit(cur_start_bit);
	return u32_ret;
}


static U32 emmc_fcie_m6e3_chk_scanned_skew4(U32 *u32_scanned_skew4)
{
	U32 u32_err = 0;
	U16 u16_skew4_cnt = 0;
	int i;

	for (i = 0; i < TOTAL_PLL_SKEW4_CNT; i++) {
		if (((*u32_scanned_skew4) & (1 << i)) == 0)
			u16_skew4_cnt++;
	}
	//The remainder of unscanned pahses are less than M6E3_MIN_OK_SKEW_CNT
	if (u16_skew4_cnt < M6E3_MIN_OK_SKEW_CNT)
		u32_err = EMMC_ST_ERR_SKEW4_NOT_ENOUGH;

	return u32_err;
}


static U32 emmc_fcie_m6e3_scan_skew4(void)
{
	U32 u32_skew4_result = 0, u32_err = 0, u32_scanned_skew4 = 0;
	U16 u16_skew4_cnt = 0;
	U16 u16_next_right = 0, u16_next_left = 0;
	int right_start = 0, left_start = 0;


	if (emmc_drv.host->no_fde == 0 && emmc_drv.host->fde_enc)
		fde_aes_func_close();

	emmc_fcie_reset();

	right_start =
		emmc_fcie_m6e3_skew4_start_bit(emmc_drv.cur_skew4 + 1);
	left_start =
		emmc_fcie_m6e3_skew4_start_bit(emmc_drv.cur_skew4 - 1);

	u32_scanned_skew4 |= (U32)(1 << emmc_drv.cur_skew4);

	while (true) {
		u32_err = emmc_fcie_m6e3_chk_scanned_skew4(&u32_scanned_skew4);
		if (u32_err != EMMC_ST_SUCCESS)
			break;

		u32_err = emmc_fcie_m6e3_skew4_right_result((u16)right_start,
							    &u16_next_right,
							    &u16_skew4_cnt,
							    &u32_skew4_result,
							    &u32_scanned_skew4);

		right_start = u16_next_right;

		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if (u16_skew4_cnt >= M6E3_MIN_OK_SKEW_CNT)
			break;

		u32_err = emmc_fcie_m6e3_chk_scanned_skew4(&u32_scanned_skew4);
		if (u32_err != EMMC_ST_SUCCESS)
			break;

		u32_err = emmc_fcie_m6e3_skew4_left_result((u16)left_start,
							   &u16_next_left,
							   &u16_skew4_cnt,
							   &u32_skew4_result,
							   &u32_scanned_skew4);

		left_start = u16_next_left;

		if (u32_err != EMMC_ST_SUCCESS)
			return u32_err;

		if (u16_skew4_cnt >= M6E3_MIN_OK_SKEW_CNT)
			break;
	};

	if (u32_skew4_result == 0 || u16_skew4_cnt < M6E3_MIN_OK_SKEW_CNT) {
		emmc_debug(EMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Clk: %uMHz SkewResult: %Xh SkewCnt: %u\n",
			   emmc_drv.u32_clk_khz / EMMC_KHZ, u32_skew4_result, u16_skew4_cnt);
		return EMMC_ST_ERR_SKEW4;
	}

	if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
		g_tab_set_skew4_tmp_t.u32_skew4_result_data = u32_skew4_result;
		u32_err = emmc_fcie_detect_timing_skew4_ex(u32_skew4_result,
							   &g_tab_set_skew4_tmp_t.u8_skew4_data,
							   &g_tab_set_skew4_tmp_t.u8_reg_2ch_data);
	} else {
		g_tab_set_tmp_t.u32_scan_result = u32_skew4_result;
		u32_err = emmc_fcie_detect_timing_skew4_ex(u32_skew4_result,
							   &g_tab_set_tmp_t.u8_skew4,
							   &g_tab_set_tmp_t.u8_reg2ch);
	}

	return u32_err;
}

U32 emmc_m6e3_rescan_timing(void)
{
	U32 u32_err = 0, u32_arg = 0;
	u8 u8_switch_to_original_area = 0;

	//---------------------------
	//search and set the Windows
	emmc_debug(EMMC_DEBUG_LEVEL, 0, "eMMC Warn: scan M6E3 skew4, please wait...\n");

	if (emmc_drv.u8_partition_config & PART_ACCESS_MASK)
		u8_switch_to_original_area = 1;

	u32_err = emmc_fcie_m6e3_scan_skew4();

	//---------------------------
	//set the Table
	if (u32_err == EMMC_ST_SUCCESS) {
		if (emmc_drv.pad_type == FCIE_EMMC_HS400) {
			emmc_debug(EMMC_DEBUG_LEVEL, 0,
				   "eMMC Warn: original Hs400 skew4 result:%Xh, new HS400 skew4 result:%Xh\n",
				   emmc_drv.t_hs400_table.set_skew4.u32_skew4_result_data,
				   g_tab_set_skew4_tmp_t.u32_skew4_result_data);
			emmc_drv.t_hs400_table.set_skew4.u8_reg_2ch_data =
				g_tab_set_skew4_tmp_t.u8_reg_2ch_data;
			emmc_drv.t_hs400_table.set_skew4.u8_skew4_data =
				g_tab_set_skew4_tmp_t.u8_skew4_data;

			if (g_tab_set_skew4_tmp_t.u8_reg_2ch_data)
				emmc_drv.cur_skew4 = g_tab_set_skew4_tmp_t.u8_skew4_data + PLL_SKEW4_CNT;
			else
				emmc_drv.cur_skew4 = g_tab_set_skew4_tmp_t.u8_skew4_data;
		} else {
			emmc_debug(EMMC_DEBUG_LEVEL, 0,
				   "eMMC Warn: original HS400-ES skew4 result:%Xh, new HS400-ES skew4 result:%Xh\n",
				   emmc_drv.t_table.set[0].u32_scan_result,
				   g_tab_set_tmp_t.u32_scan_result);

			emmc_drv.t_table.set[EMMC_TIMING_SET_MAX].u8_reg2ch = g_tab_set_tmp_t.u8_reg2ch;
			emmc_drv.t_table.set[EMMC_TIMING_SET_MAX].u8_skew4 = g_tab_set_tmp_t.u8_skew4;
			emmc_drv.t_table.set[EMMC_TIMING_SET_MAX].u32_scan_result = g_tab_set_tmp_t.u32_scan_result;

			if (g_tab_set_tmp_t.u8_reg2ch) {
				emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_SETBIT;
				emmc_drv.cur_skew4 = g_tab_set_tmp_t.u8_skew4 + PLL_SKEW4_CNT;
			} else {
				emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_INV_BIT].u16_op_code = REG_OP_CLRBIT;
				emmc_drv.cur_skew4 = g_tab_set_tmp_t.u8_skew4;
			}
			//skew4 value
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_op_code = REG_OP_SETBIT;
			emmc_drv.t_table_g.reg_set[GT_OFFSET_SKEW4_VALUE].u16_reg_value =
				g_tab_set_tmp_t.u8_skew4 << BIT_SKEW4_SHIFT;

			emmc_fcie_apply_timing_set(emmc_drv.t_table_g.u8_cur_set_idx);
		}
		emmc_debug(EMMC_DEBUG_LEVEL, 0, "\nOK\n");
	}

	if (u8_switch_to_original_area) {
		u32_arg = (3 << 24) | (EXT_CSD_PART_CONF << 16) | (emmc_drv.u8_partition_config << 8);
		u32_err = emmc_cmd6_skew4(u32_arg);
	}

	return u32_err;
}

#endif // ENABLE_EMMC_ATOP

void emmc_check_sar5_on(void)
{
	#if defined(ENABLE_EMMC_POWER_SAVING_MODE) && ENABLE_EMMC_POWER_SAVING_MODE
	U16 i, len;
	char *res;

	res = env_get("bootargs");
	if (res) {
	len = strlen(res);
	for (i = 0; i < len; i++) {
		if (res[i] == 'S' && res[i + 1] == 'A' &&
		    res[i + 2] == 'R' && res[i + 3] == '5' &&
		    res[i + 4] == '=' && res[i + 5] == 'O' &&
		    res[i + 6] == 'F' && res[i + 7] == 'F')
			break;
	}

		if (i < len) {
			//emmc_debug(0, 0, "SAR5=OFF\n");
		} else {
			emmc_debug(0, 0, "SAR5=ON\n");
			emmc_prepare_power_saving_mode_queue();
		}
	}

#endif
}

static U32 sg_u32_mem_guard0 = 0xA55A;
struct emmc_driver emmc_drv EMMC_ALIGN1;
static U32 sg_u32_mem_guard1 = 0x1289;

U8 GLOBAL_EMMC_SECTORBUF[EMMC_SECTOR_BUF_16KB] EMMC_ALIGN1;
U8 au8_ext_csd[EMMC_SECTOR_512BYTE] EMMC_ALIGN1;

U32 emmc_check_if_mem_corrupt(void)
{
	if (sg_u32_mem_guard0 != 0xA55A || sg_u32_mem_guard1 != 0x1289)
		return EMMC_ST_ERR_MEM_CORRUPT;

	return EMMC_ST_SUCCESS;
}


#if (defined(ENABLE_EMMC_HS200) && ENABLE_EMMC_HS200) || \
(defined(ENABLE_EMMC_HS400) && ENABLE_EMMC_HS400)

// ======================================================
// 0~7, TRIGGER_CNT steps

void emmc_set_triger_level(U8 u8_level)
{
	const U16 trig_level[TRIGGER_CNT] = {0x0000, 0x0007, 0x0038, 0x003f, 0x01c0, 0x01c7, 0x01f8, 0x01ff};

	if (u8_level > 7) {
		if (emmc_drv.chip_id != MT5896_CHIP_ID)
			emmc_debug(1, 0, "wrong parameter for %s()\n", __func__);

		return;
	}
	if (emmc_drv.chip_id != MT5896_CHIP_ID)
		emmc_debug(1, 0, "set trigger level to %d, ", u8_level);

	REG_FCIE_CLRBIT(reg_emmcpll_0x20, TIGER_LVL_MASK);
	REG_FCIE_SETBIT(reg_emmcpll_0x20, trig_level[u8_level]);
	if (emmc_drv.chip_id != MT5896_CHIP_ID)
		emmc_debug(1, 0, "emmc_pll[0x20] = %04Xh\n\n", REG_FCIE(reg_emmcpll_0x20));

}

// 0~TRIGGER_CNT, 9 steps
void emmc_set_write_phase(U8 u8_cmd_data, U8 u8_phase)
{
	const U8 SKEW1 = 1;
	const U8 SKEW2 = 2; //data DQ
	const U8 SKEW3 = 3; //CMD
	const U8 SKEW4 = 4;

	if (u8_phase > TRIGGER_CNT)
		emmc_debug(1, 0, "wrong parameter for %s() %d %d\n", __func__, u8_cmd_data, u8_phase);

	if (u8_cmd_data == SKEW3) {
		emmc_debug(1, 0, "\nset skew3 cmd phase to %d, ", u8_phase);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT11 | BIT10 | BIT9 | BIT8);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_phase << TRIGGER_CNT);
		emmc_debug(1, 0, "emmc_pll[0x03] = %04Xh\n", REG_FCIE(reg_emmcpll_0x03));
	} else if (u8_cmd_data == SKEW2) {
		emmc_debug(1, 0, "\nset skew2 data phase to %d, ", u8_phase);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT7 | BIT6 | BIT5 | BIT4);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_phase << 4);
		emmc_debug(1, 0, "emmc_pll[0x03] = %04Xh\n", REG_FCIE(reg_emmcpll_0x03));
	} else if (u8_cmd_data == SKEW1) {
		emmc_debug(1, 0, "\nset skew1 phase to %d, ", u8_phase);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT3 | BIT2 | BIT1 | BIT0);
		REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_phase);
		emmc_debug(1, 0, "emmc_pll[0x03] = %04Xh\n", REG_FCIE(reg_emmcpll_0x03));
	} else if (u8_cmd_data == SKEW4) {
		emmc_debug(1, 0, "\nset skew4 phase to %d, ", u8_phase);
		REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT15 | BIT14 | BIT13 | BIT12);
		if (u8_phase < PLL_SKEW4_CNT) {
			REG_FCIE_CLRBIT(reg_emmcpll_0x02, BIT14);
			REG_FCIE_CLRBIT(reg_emmcpll_0x6c, BIT7);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, u8_phase << 12);
		} else {
			REG_FCIE_SETBIT(reg_emmcpll_0x02, BIT14);
			REG_FCIE_SETBIT(reg_emmcpll_0x6c, BIT7);
			REG_FCIE_SETBIT(reg_emmcpll_0x03, (u8_phase - PLL_SKEW4_CNT) << 12);
		}

		emmc_debug(1, 0, "emmc_pll[0x02] = %04Xh\n", REG_FCIE(reg_emmcpll_0x02));
		emmc_debug(1, 0, "emmc_pll[0x03] = %04Xh\n", REG_FCIE(reg_emmcpll_0x03));
	}
}

#endif

void emmc_lock_fcie(U8 *pu8_str)
{
	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);//output clock
	REG_FCIE_SETBIT(FCIE_MIE_FUNC_CTL, BIT_EMMC_ACTIVE);
}

void emmc_unlock_fcie(U8 *pu8_str)
{
	//emmc_debug(0, 1,"%s 1\n", pu8_str);
	REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);//not output clock
	REG_FCIE_CLRBIT(FCIE_MIE_FUNC_CTL, BIT_EMMC_ACTIVE);
	//emmc_debug(0, 1,"%s 2\n", pu8_str);
}

#endif
