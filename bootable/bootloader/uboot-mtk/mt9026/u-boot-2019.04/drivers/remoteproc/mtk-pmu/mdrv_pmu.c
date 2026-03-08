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

#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include "platform.h"
#include "mdrv_pmu.h"
#include "halCM4.h"
#include "drvBDMA.h"

#define REG_PM_SLEEP_0XEC 0xEC

mtk_pmu_info g_pmu_info;
//=============================================
// CM4 Halt out
//=============================================
void mdrv_pmu_enable_uart(int enable)
{
	if (enable) {
		PMU_log(PMU_DEBUG, "pmu Uart enable\n");
		SETREG16(g_pmu_info.pm_sleep | OFFSET(REG_PM_SLEEP_0XEC), BIT6);
	} else {
		PMU_log(PMU_DEBUG, "pmu Uart disable\n");
		CLRREG16(g_pmu_info.pm_sleep | OFFSET(REG_PM_SLEEP_0XEC), BIT6);
	}
}

void mdrv_pmu_stop(void)
{
	mhal_cm4_stop();
}

void mdrv_pmu_halt(void)
{
	mhal_cm4_halt();
}
//=============================================
//  paganini_ac_on_clk
//  paganini_dc_on_clk
//=============================================
void mdrv_pmu_init_info(mtk_pmu_info *info)
{
    memset(&g_pmu_info,0,sizeof(mtk_pmu_info));
    memcpy(&g_pmu_info,info,sizeof(mtk_pmu_info));
}

void mdrv_pmu_init(void)
{
	mhal_cm4_init(&g_pmu_info);
	mdrv_bdma_init(&g_pmu_info);
	mhal_cm4_dc_on();
}

void mdrv_pmu_dc_on(void)
{
	mhal_cm4_dc_on();
	mdrv_bdma_init(&g_pmu_info);
}

void mdrv_pmu_dc_off(void)
{
	mhal_cm4_dc_off();
}
