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

#ifndef _HAL_CM4_H
#define _HAL_CM4_H

//=============================================================================
// Includs
//=============================================================================
#include "platform.h"

//=============================================================================
// Defines & Macros
//=============================================================================

//TODO: get chip family ID from system API instead of riu register
#ifdef CHIP_MT5896
#undef CHIP_MT5896
#define CHIP_MT5896 0x101
#else
#define CHIP_MT5896 0x101
#endif

#ifdef CHIP_MT5897
#undef CHIP_MT5897
#define CHIP_MT5897 0x109
#else
#define CHIP_MT5897 0x109
#endif

#ifdef CHIP_MT5879
#undef CHIP_MT5879
#define CHIP_MT5879 0x10D
#else
#define CHIP_MT5879 0x10D
#endif

#define REG_PAGA_PLL_DIV_48 0xe1
#define REG_PAGA_0X22       0x22

#define REG_PM_MISC_0X4E    0x4E

#define REG_COIN_CPU_0X00   0x00

#define REG_CKGEN1_PM_0X1E  0x1E
#define REG_CKGEN1_PM_0X0C  0x0C
#define REG_CKGEN1_PM_0X02  0x02
#define REG_CKGEN1_PM_0X06  0x06
#define REG_CKGEN1_PM_0X08  0x08
#define REG_CKGEN1_PM_0X0A  0x0A
//=============================================================================
// Function
//=============================================================================
void mhal_cm4_init(mtk_pmu_info *info);
void mhal_cm4_halt(void);
void mhal_cm4_stop(void);
void mhal_cm4_run(void);
void mhal_cm4_dc_on(void);
void mhal_cm4_dc_off(void);

#endif
