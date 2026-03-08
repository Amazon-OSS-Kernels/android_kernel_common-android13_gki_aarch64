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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    halBDMA.c
/// @brief  BDMA hal DDI
///////////////////////////////////////////////////////////////////////////////////////////////////

//=============================================================================
// Include Files
//=============================================================================
#include "halBDMA.h"

#define BIT(_bit_)                  (1 << (_bit_))

#define VBDMA_REG_CTRL(ch)           OFFSET(OFFSET((0x00+ch*0x10)))
#define VBDMA_REG_STATUS(ch)         OFFSET(OFFSET((0x01+ch*0x10)))
#define VBDMA_REG_DEV_SEL(ch)        OFFSET(OFFSET((0x02+ch*0x10)))
#define VBDMA_REG_MISC(ch)           OFFSET(OFFSET((0x03+ch*0x10)))
#define VBDMA_REG_SRC_ADDR_L(ch)     OFFSET(OFFSET((0x04+ch*0x10)))
#define VBDMA_REG_SRC_ADDR_H(ch)     OFFSET(OFFSET((0x05+ch*0x10)))
#define VBDMA_REG_DST_ADDR_L(ch)     OFFSET(OFFSET((0x06+ch*0x10)))
#define VBDMA_REG_DST_ADDR_H(ch)     OFFSET(OFFSET((0x07+ch*0x10)))
#define VBDMA_REG_SIZE_L(ch)         OFFSET(OFFSET((0x08+ch*0x10)))
#define VBDMA_REG_SIZE_H(ch)         OFFSET(OFFSET((0x09+ch*0x10)))
#define VBDMA_REG_CMD0_L(ch)         OFFSET(OFFSET((0x0A+ch*0x10)))
#define VBDMA_REG_CMD0_H(ch)         OFFSET(OFFSET((0x0B+ch*0x10)))
#define VBDMA_REG_CMD1_L(ch)         OFFSET(OFFSET((0x0C+ch*0x10)))
#define VBDMA_REG_CMD1_H(ch)         OFFSET(OFFSET((0x0D+ch*0x10)))
#define VBDMA_REG_CMD2_L(ch)         OFFSET(OFFSET((0x0E+ch*0x10)))
#define VBDMA_REG_CMD2_H(ch)         OFFSET(OFFSET((0x0F+ch*0x10)))
//---------------------------------------------
// definition for BDMA_REG_CH0_CTRL/BDMA_REG_CH1_CTRL
//---------------------------------------------
#define BDMA_CH_TRIGGER             BIT(0)
#define BDMA_CH_STOP                BIT(4)
//---------------------------------------------
// definition for REG_BDMA_CH0_STATUS/REG_BDMA_CH1_STATUS
//---------------------------------------------
#define BDMA_CH_QUEUED              BIT(0)
#define BDMA_CH_BUSY                BIT(1)
#define BDMA_CH_INT                 BIT(2)
#define BDMA_CH_DONE                BIT(3)
#define BDMA_CH_RESULT              BIT(4)
#define BDMA_CH_CLEAR_STATUS        (BDMA_CH_INT|BDMA_CH_DONE|BDMA_CH_RESULT)

//---------------------------------------------
// definition for REG_BDMA_CH0_MISC/REG_BDMA_CH1_MISC
//---------------------------------------------

#define BDMA_CH_ADDR_DECDIR         BIT(0)
#define BDMA_CH_DONE_INT_EN         BIT(1)
#define BDMA_CH_CRC_REFLECTION      BIT(4)
#define BDMA_CH_MOBF_EN             BIT(5)

#define BDMA_FLASH_BUSY_TIMEOUT     (5000)
#define BDMA_OFFSET_SHIFT           (16)

#define BDMA_CRC32_CMD0_L (0x1db7)
#define BDMA_CRC32_CMD0_H (0x04c1)

#define VBDMA_INVALID_MASK  (0xF)
#define VBDMA_MASK_0XFFFF   (0xFFFF)

#define VBDMA_VAL_0XFFFF (0xFFFF)
#define VBDMA_VAL_0X0240 (0x0240)
#define VBDMA_VAL_0X0340 (0x0340)
#define VBDMA_VAL_0X2000 (0x2000)
#define VBDMA_VAL_0X2010 (0x2010)
#define VBDMA_VAL_0X4035 (0x4035)
#define VBDMA_VAL_0X4041 (0x4041)
#define VBDMA_VAL_0X4140 (0x4140)
#define VBDMA_VAL_0X4141 (0x4141)
#define VBDMA_VAL_0X8000 (0x8000)

#define VBDMA_VAL_0X02   (0x02)
#define VBDMA_VAL_0X03   (0x03)
#define VBDMA_VAL_0X04   (0x04)
#define VBDMA_VAL_0X08   (0x08)
#define VBDMA_VAL_0X10   (0x10)
#define VBDMA_VAL_0X0C   (0x0C)

#define VBDMA_REG_0X18   (0x18)
#define VBDMA_REG_0X1E   (0x1E)
#define VBDMA_REG_0X52   (0x52)
#define VBDMA_REG_0X80   (0x80)
#define VBDMA_REG_0XF4   (0xF4)

#define VIVALDI_VAL_0X2    (0x2)
#define VIVALDI_REG_0XA4   (0xA4)

extern mtk_pmu_info g_pmu_info;
BOOL g_bPhaseChecking = 0;

BOOL mhal_bdma_wait_done(U32 channel, U32 timeout_ms)
{
	U32 count = 0;

	for (count = 0; count < timeout_ms; count++) {
		if (INREG16(g_pmu_info.viv_bdma | VBDMA_REG_STATUS(channel)) & BDMA_CH_DONE)
			return TRUE;
		mdelay(1);
	}

	return FALSE;
}

void mhal_bdma_set_phase_checking(BOOL enable)
{
	g_bPhaseChecking = enable;
}

BOOL mhal_bdma_get_phase_checking(void)
{
	return g_bPhaseChecking;
}


BOOL mhal_bdma_wait_done_us(U32 channel, U32 timeout_us)
{
	U32 count = 0;

	for (count = 0; count < timeout_us; count++) {
		if (INREG16(g_pmu_info.viv_bdma | VBDMA_REG_STATUS(channel)) & BDMA_CH_DONE)
			return TRUE;
		udelay(1);
	}

	return FALSE;
}

BOOL mhal_bdma_polling_done(U32 channel)
{
	while (1) {
		if (INREG16(g_pmu_info.viv_bdma | VBDMA_REG_STATUS(channel)) & BDMA_CH_DONE)
			return TRUE;
	}
}

void mhal_bdma_change_phase(void)
{
	static U8 phase = 0;
	u16 value_bit_1 = 0;
	u16 value_bit_3_2 = 0;

	if (phase == 0) {
		phase = 1;
		value_bit_1   = VBDMA_VAL_0X02;
		value_bit_3_2 = VBDMA_VAL_0X04;
	}
	else if (phase == 1) {
		phase = VBDMA_VAL_0X02;
		value_bit_1   = VBDMA_VAL_0X02;
		value_bit_3_2 = VBDMA_VAL_0X08;
	}
	else if (phase == VBDMA_VAL_0X02) {
		phase = VBDMA_VAL_0X03;
		value_bit_1   = 0x00;
		value_bit_3_2 = VBDMA_VAL_0X08;
	}
	else if (phase == VBDMA_VAL_0X03) {
		phase = 0;
		value_bit_1   = 0x00;
		value_bit_3_2 = VBDMA_VAL_0X04;
	}

	INSREG16(g_pmu_info.ckgen00_pm|(OFFSET8(VBDMA_REG_0X18)), VBDMA_VAL_0X02, value_bit_1);
	INSREG16(g_pmu_info.ckgen00_pm|(OFFSET8(VBDMA_REG_0X18)), VBDMA_VAL_0X0C, value_bit_3_2);
}

void mhal_bdma_clock_inverse(void)
{
	U16 clk;

	PMU_log(PMU_WARNING,"IMI clock inverse.\n");
	clk = INREG16(g_pmu_info.ckgen00_pm|(OFFSET8(VBDMA_REG_0X18)));

	INSREG16(g_pmu_info.viv_bdma  |(OFFSET8(VBDMA_REG_0X80)), VBDMA_VAL_0X10, VBDMA_VAL_0X10);
	INSREG16(g_pmu_info.vivaldi4  |(OFFSET8(VBDMA_REG_0X52)), VBDMA_VAL_0X08, VBDMA_VAL_0X08);
	INSREG16(g_pmu_info.ckgen01_pm|(OFFSET8(VBDMA_REG_0X1E)), 0x01, 0x00);
	OUTREG16(g_pmu_info.ckgen01_pm|(OFFSET8(VBDMA_REG_0XF4)), 0x0000);

	mhal_bdma_change_phase();

	INSREG16(g_pmu_info.ckgen01_pm|(OFFSET8(VBDMA_REG_0X1E)), 0x01, 0x01);
	INSREG16(g_pmu_info.vivaldi4  |(OFFSET8(VBDMA_REG_0X52)), VBDMA_VAL_0X08, 0x00);
	INSREG16(g_pmu_info.viv_bdma  |(OFFSET8(VBDMA_REG_0X80)), VBDMA_VAL_0X10, 0x00);

	PMU_log(PMU_WARNING,"0x0C18 from 0x%x ",clk);
	clk = INREG16(g_pmu_info.ckgen00_pm|(OFFSET8(VBDMA_REG_0X18)));
	PMU_log(PMU_WARNING,"to 0x%x\n",clk);
}

BOOL mhal_bdma_is_busy(U32 channel)
{
	U32 status = INREG16(g_pmu_info.viv_bdma | VBDMA_REG_STATUS(channel));

	return (BDMA_CH_BUSY == (status & BDMA_CH_BUSY));
}

BOOL mhal_bdma_flash_to_miu(U32 channel, U32 flash_addr, U32 dram_addr, U32 len)
{
	int i = 0;
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;
	U32 status = 0;

	if ((len & VBDMA_INVALID_MASK) ||
	    (flash_addr & VBDMA_INVALID_MASK) ||
	    (dram_addr & VBDMA_INVALID_MASK))
		return FALSE;

	for (i = 0; i < BDMA_FLASH_BUSY_TIMEOUT; i++) {
		status = INREG16(viv_bdma | VBDMA_REG_STATUS(channel));
		if (0 == (status & BDMA_CH_BUSY))
			break;
		mdelay(1);
	}

	if (i == BDMA_FLASH_BUSY_TIMEOUT)
		return FALSE;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);

	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X4035);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(flash_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((flash_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(dram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((dram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_calculate_crc32_from_miu(U32 channel, U32 dram_addr, U32 len)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);

	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X0340);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(dram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((dram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel), 0);
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel), 0);

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CMD0_L(channel), BDMA_CRC32_CMD0_L);
	OUTREG16(viv_bdma | VBDMA_REG_CMD0_H(channel), BDMA_CRC32_CMD0_H);
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_L(channel), VBDMA_VAL_0XFFFF);
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_H(channel), VBDMA_VAL_0XFFFF);

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_calculate_crc32_from_imi(U32 channel, U32 sram_addr, U32 len)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	//reflection 1,checked with VAL
	OUTREG16(viv_bdma | VBDMA_REG_MISC(channel), VBDMA_VAL_0X2010);
	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X0340);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(sram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((sram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(sram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((sram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CMD0_L(channel), BDMA_CRC32_CMD0_L);
	OUTREG16(viv_bdma | VBDMA_REG_CMD0_H(channel), BDMA_CRC32_CMD0_H);
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_L(channel), VBDMA_VAL_0XFFFF);
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_H(channel), VBDMA_VAL_0XFFFF);

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_pattern_search_from_imi(U32 channel, U32 sram_addr, U32 len, U32 pattern)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;
	U16 u16RegVal;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	//reflection 1,checked with VAL
	OUTREG16(viv_bdma | VBDMA_REG_MISC(channel), VBDMA_VAL_0X2000);
	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X0240);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(sram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((sram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(sram_addr & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((sram_addr >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CMD0_L(channel),
		 (U16)(pattern & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_CMD0_H(channel),
		 (U16)((pattern >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_L(channel), 0);
	OUTREG16(viv_bdma | VBDMA_REG_CMD1_H(channel), 0);
	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	while (1) {
		u16RegVal = INREG16(viv_bdma | VBDMA_REG_STATUS(channel));
		if ((u16RegVal & BDMA_CH_DONE) || (u16RegVal & BDMA_CH_RESULT))
			break;
	}

	if (u16RegVal & BDMA_CH_RESULT)
		return TRUE;
	else
		return FALSE;
}

U32 mhal_bdma_get_crc32(U32 channel)
{
	U16 cmd1_l = INREG16(g_pmu_info.viv_bdma | VBDMA_REG_CMD1_L(channel));
	U16 cmd1_h = INREG16(g_pmu_info.viv_bdma | VBDMA_REG_CMD1_H(channel));

	return (cmd1_l | (cmd1_h << BDMA_OFFSET_SHIFT));
}

BOOL mhal_bdma_imi_to_miu(U32 channel, U32 src, U32 dst, U32 len)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	OUTREG16(viv_bdma | VBDMA_REG_MISC(channel), VBDMA_VAL_0X8000);

	//replace MIU channel0
	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X4041);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(src & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((src >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(dst & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((dst >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_miu_to_imi(U32 channel, U32 src, U32 dst, U32 len)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	OUTREG16(viv_bdma | VBDMA_REG_MISC(channel), VBDMA_VAL_0X8000);

	//replace MIU channel0
	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X4140);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(src & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((src >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(dst & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((dst >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_imi_to_imi(U32 channel, U32 src, U32 dst, U32 len)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	//PMU_log(PMU_INFO, "mhal_bdma_imi_to_imi = 0x%lx -> 0x%lx (0x%lx)\n",
	//	  src, dst, len);

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	OUTREG16(viv_bdma | VBDMA_REG_MISC(channel), VBDMA_VAL_0X8000);

	OUTREG16(viv_bdma | VBDMA_REG_DEV_SEL(channel), VBDMA_VAL_0X4141);
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_L(channel),
		 (U16)(src & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SRC_ADDR_H(channel),
		 (U16)((src >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_L(channel),
		 (U16)(dst & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_DST_ADDR_H(channel),
		 (U16)((dst >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_SIZE_L(channel),
		 (U16)(len & VBDMA_MASK_0XFFFF));
	OUTREG16(viv_bdma | VBDMA_REG_SIZE_H(channel),
		 (U16)((len >> BDMA_OFFSET_SHIFT) & VBDMA_MASK_0XFFFF));

	OUTREG16(viv_bdma | VBDMA_REG_CTRL(channel), BDMA_CH_TRIGGER);
	return TRUE;
}

BOOL mhal_bdma_clear_status(U32 channel)
{
	phys_addr_t viv_bdma = g_pmu_info.viv_bdma;

	OUTREG16(viv_bdma | VBDMA_REG_STATUS(channel), BDMA_CH_CLEAR_STATUS);
	return TRUE;
}

void mhal_bdma_init(void)
{
	// bdma XTAIL Clock
	SETREG16(g_pmu_info.vivaldi2 | OFFSET(VIVALDI_REG_0XA4), VIVALDI_VAL_0X2);
	mdelay(1);
}
