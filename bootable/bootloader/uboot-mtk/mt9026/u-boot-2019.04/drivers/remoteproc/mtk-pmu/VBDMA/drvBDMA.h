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
/// @brief  MStar BDMA Driver DDI
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _DRV_BDMA_H
#define _DRV_BDMA_H

#include "platform.h"

//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define SLEEP_BANK_0XA0         (0xA0)
#define SLEEP_BANK_LEN          (32)
#define SLEEP_BANK_WIDTH        (16)

#define IMI_CLK_REG_0X1E        (0x1E)
#define IMI_CLK_REG_0X0C        (0x0C)

#define ISOINTB_BANK_0XC0       (0xC0)
#define ISOINTB_BANK_0XC2       (0xC2)
#define ISOINTB_BANK_LEN        (2)
#define ISOINTB_BANK_WIDTH      (16)
#define ISOINTB_ENABLE_ALL      (0xFFFF)
#define ISOINTB_DISABLE_ALL     (0x0000)

//-------------------------------------------------------------------------------------------------
// Type and Structure Declaration
//-------------------------------------------------------------------------------------------------
enum {
	E_PMIMI_BANK0 = 0,
	E_PMIMI_BANK1,
	E_PMIMI_BANK2,
	E_PMIMI_BANK3,
	E_PMIMI_BANK4,
	E_PMIMI_BANK5,
	E_PMIMI_BANK6,
	E_PMIMI_BANK7,
	E_PMIMI_BANK8,
	E_PMIMI_BANK9,
	E_PMIMI_BANK10,
	E_PMIMI_BANK11,
	E_PMIMI_BANK12,
	E_PMIMI_BANK13,
	E_PMIMI_BANK14,
	E_PMIMI_BANK15,
	E_PMIMI_BANK16,
	E_PMIMI_BANK17,
	E_PMIMI_BANK18,
	E_PMIMI_BANK19,
	E_PMIMI_BANK20,
	E_PMIMI_BANK21,
	E_PMIMI_BANK22,
	E_PMIMI_BANK23,
	E_PMIMI_BANK24,
	E_PMIMI_BANK25,
	E_PMIMI_BANK26,
	E_PMIMI_BANK27,
	E_PMIMI_BANK28,
	E_PMIMI_BANK29,
	E_PMIMI_BANK30,
	E_PMIMI_BANK31,
	E_PMIMI_MAX,
};

enum {
	E_BDMA_CP_TO_HK = 0,
	E_BDMA_HK_TO_CP,
	E_BDMA_CP_TO_CP,
	E_BDMA_HK_TO_HK,
};

enum {
	E_BDMA_DEV_MIU = 0,
	E_BDMA_DEV_IMI
};

//-------------------------------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------------------------------
void mdrv_bdma_init(mtk_pmu_info *info);
BOOL mdrv_bdma_copy(U32 src, U32 dst, U32 len, U32 mode);
U32 mdrv_bdma_crc32(U32 src, U32 dst, U32 dev);
BOOL mdrv_bdma_pattern_search(U32 src, U32 len, U32 pattern, U32 dev);

void mdrv_pm_imi_init(mtk_pmu_info *info);
void mdrv_pm_imi_sleep(mtk_pmu_info *info, int bankid);
#endif
