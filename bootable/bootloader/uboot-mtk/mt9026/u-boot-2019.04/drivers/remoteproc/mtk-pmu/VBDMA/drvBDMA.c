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
/// @brief  BDMA DRV Driver DDI
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#define _DRV_BDMA_C
#include <command.h>

#include "halBDMA.h"
#include "drvBDMA.h"
void mdrv_bdma_init(mtk_pmu_info *info)
{
	mhal_bdma_init();
}

BOOL mdrv_bdma_try_wait_done(U32 ch)
{
	#define BDMA_WAIT_US  2000 // 1MBytes should be done within 875us.
	bool ret;
	ret = mhal_bdma_wait_done_us(ch, BDMA_WAIT_US);

	if (ret == FALSE) {
		PMU_log(PMU_ERROR, "wait for vbdma done timeout!\n");
		mhal_bdma_clock_inverse();
	}
	return ret;
}

BOOL mdrv_bdma_copy(U32 src, U32 dst, U32 len, U32 mode)
{
	#define CHANGE_PHASE_COUNT 4
	U8 phase_count = 0;
	U32 ch;

	switch (mode) {
	case E_BDMA_CP_TO_HK:
		ch = E_BDMA_CH0;
		mhal_bdma_imi_to_miu(ch, src, dst, len);
		break;
	case E_BDMA_HK_TO_CP:
		ch = E_BDMA_CH1;
		if (mhal_bdma_get_phase_checking()) {
			do {
				mhal_bdma_miu_to_imi(ch, src, dst, len);
				phase_count++;
				if (phase_count > CHANGE_PHASE_COUNT)
					break;
			} while (mdrv_bdma_try_wait_done(ch) == FALSE);
		} else {
			mhal_bdma_miu_to_imi(ch, src, dst, len);
		}
		break;
	case E_BDMA_CP_TO_CP:
		ch = E_BDMA_CH0;
		if (mhal_bdma_get_phase_checking()) {
			do {
				mhal_bdma_imi_to_imi(ch, src, dst, len);
				phase_count++;
				if (phase_count > CHANGE_PHASE_COUNT)
					break;
			} while (mdrv_bdma_try_wait_done(ch) == FALSE);
		} else {
			mhal_bdma_imi_to_imi(ch, src, dst, len);
		}
		break;
	case E_BDMA_HK_TO_HK:
	default:
		//PMU_log(PMU_ERROR, "mdrv_bdma_copy: mode %d failed\n", mode);
		return FALSE;
	}
	mhal_bdma_polling_done(ch);
	return TRUE;
}


U32 mdrv_bdma_crc32(U32 src, U32 len, U32 dev)
{
	U32 ch = E_BDMA_CH1;

	switch (dev) {
	case E_BDMA_DEV_MIU:
		mhal_bdma_calculate_crc32_from_miu(ch, src, len);
	break;
	case E_BDMA_DEV_IMI:
		mhal_bdma_calculate_crc32_from_imi(ch, src, len);
	break;
	}
	mhal_bdma_polling_done(ch);
	return mhal_bdma_get_crc32(ch);
}

BOOL mdrv_bdma_pattern_search(U32 src, U32 len, U32 pattern, U32 dev)
{
	U32 ch = E_BDMA_CH1;

	switch (dev) {
	case E_BDMA_DEV_IMI:
		return mhal_bdma_pattern_search_from_imi(ch, src, len, pattern);
	default:
		//PMU_log(PMU_ERROR, "mdrv_bdma_pattern_search: Dev %d not support\n", dev);
	return false;
	}
}

void mdrv_pm_imi_init(mtk_pmu_info *info)
{
	int i, offset;
	phys_addr_t pm_imi;
	phys_addr_t ckgen01_pm;
	unsigned int reg_id;
	unsigned int reg_id_end;
	unsigned short val;
	int reset_status;

	if (!info || !info->pm_imi || !info->ckgen01_pm)
		return;

	pm_imi = info->pm_imi;
	ckgen01_pm = info->ckgen01_pm;

	//reg_sw_en_imi2paganini_imi
	INSREG8((ckgen01_pm) | (OFFSET8(IMI_CLK_REG_0X1E)), 0x01, 0x01);
	//reg_sw_en_cm42cm4
	INSREG8((ckgen01_pm) | (OFFSET8(IMI_CLK_REG_0X0C)), 0x01, 0x01);

	//In current, we only init sleep mode
	//1.resume pm imi bank0~bank31
	reg_id_end = SLEEP_BANK_0XA0 + SLEEP_BANK_LEN;
	offset = SLEEP_BANK_LEN / SLEEP_BANK_WIDTH;
	for (reg_id = SLEEP_BANK_0XA0; reg_id < reg_id_end; reg_id += offset) {
		for (i = 0; i < SLEEP_BANK_WIDTH; i++) {
			val = INREG16((pm_imi) | (OFFSET8(reg_id)));
			val |= (1 << i);
			OUTREG16((pm_imi) | (OFFSET(reg_id)), val);
		}
	}

	//delay 1us before isointb setting after sleep bank apply
	udelay(1);

	//disable isointb (bank0~bank31)
	OUTREG16((pm_imi) | (OFFSET(ISOINTB_BANK_0XC0)), ISOINTB_ENABLE_ALL);
	OUTREG16((pm_imi) | (OFFSET(ISOINTB_BANK_0XC2)), ISOINTB_ENABLE_ALL);

	//delay 1us after isointb setting
	udelay(1);

	//reset imi
	reset_status = INREG16((pm_imi) | (OFFSET8(0x0)));
	OUTREG16((pm_imi) | (OFFSET8(0x0)), 0x0);
	udelay(1);
	OUTREG16((pm_imi) | (OFFSET8(0x0)), reset_status);
	udelay(1);
}

void mdrv_pm_imi_sleep(mtk_pmu_info *info, int bankid)
{
	int i, offset;
	phys_addr_t pm_imi;
	unsigned int reg_id;
	unsigned int reg_id_end;
	unsigned short val;

	if (!info || !info->pm_imi)
		return;

	pm_imi = info->pm_imi;
	if (bankid == E_PMIMI_MAX) {
		OUTREG16((pm_imi) | (OFFSET(ISOINTB_BANK_0XC0)), ISOINTB_DISABLE_ALL);
		OUTREG16((pm_imi) | (OFFSET(ISOINTB_BANK_0XC2)), ISOINTB_DISABLE_ALL);

		//delay 1us before sleep bank setting after isointb apply
		udelay(1);

		reg_id_end = SLEEP_BANK_0XA0 + SLEEP_BANK_LEN;
		offset = SLEEP_BANK_LEN / SLEEP_BANK_WIDTH;
		for (reg_id = SLEEP_BANK_0XA0; reg_id < reg_id_end; reg_id += offset) {
			for (i = 0; i < SLEEP_BANK_WIDTH; i++) {
				val = INREG16((pm_imi) | (OFFSET8(reg_id)));
				val &= ~(1 << i);
				OUTREG16((pm_imi) | (OFFSET(reg_id)), val);
			}
		}
	} else {
		offset = (bankid > E_PMIMI_BANK15) ? ISOINTB_BANK_LEN : 0;
		val = INREG16((pm_imi) | (OFFSET8(ISOINTB_BANK_0XC0 + offset)));
		val &= ~(1 << (bankid % ISOINTB_BANK_WIDTH));
		OUTREG16((pm_imi) | (OFFSET(ISOINTB_BANK_0XC0 + offset)), val);

		//delay 1us before sleep bank setting after isointb apply
		udelay(1);

		reg_id = SLEEP_BANK_0XA0 + bankid;
		for (i = 0; i < SLEEP_BANK_WIDTH; i++) {
			val = INREG16((pm_imi) | (OFFSET8(reg_id)));
			val &= ~(1 << i);
			OUTREG16((pm_imi) | (OFFSET(reg_id)), val);
		}
	}
}
