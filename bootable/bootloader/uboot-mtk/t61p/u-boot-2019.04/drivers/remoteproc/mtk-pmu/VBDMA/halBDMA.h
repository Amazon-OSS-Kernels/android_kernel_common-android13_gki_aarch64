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
/// @brief  BDMA HAL DDI
/// @attention
/// <b><em></em></b>
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _HAL_BDMA_H
#define _HAL_BDMA_H

#include "platform.h"
//=============================================================================
// Includs
//=============================================================================
enum {
	E_BDMA_CH0,
	E_BDMA_CH1
};

BOOL mhal_bdma_wait_done(U32 channel, U32 timeout_ms);
BOOL mhal_bdma_wait_done_us(U32 channel, U32 timeout_us);
void mhal_bdma_set_phase_checking(BOOL enable);
BOOL mhal_bdma_get_phase_checking(void);
void mhal_bdma_clock_inverse(void);
BOOL mhal_bdma_polling_done(U32 channel);
BOOL mhal_bdma_flash_to_mem(U32 channel, U32 flash_addr, U32 dram_addr, U32 len);
BOOL mhal_bdma_calculate_crc32(U32 channel, U32 dram_addr, U32 len);
U32 mhal_bdma_get_crc32(U32 channel);
BOOL mhal_bdma_mem_to_mem(U32 channel, U32 src, U32 dst, U32 len);
BOOL mhal_bdma_imi_to_imi(U32 channel, U32 src, U32 dst, U32 len);
BOOL mhal_bdma_imi_to_miu(U32 channel, U32 src, U32 dst, U32 len);
BOOL mhal_bdma_miu_to_imi(U32 channel, U32 src, U32 dst, U32 len);
BOOL mhal_bdma_calculate_crc32_from_miu(U32 channel, U32 dram_addr, U32 len);
BOOL mhal_bdma_calculate_crc32_from_imi(U32 channel, U32 sram_addr, U32 len);
BOOL mhal_bdma_pattern_search_from_imi(U32 channel, U32 sram_addr, U32 len, U32 pattern);
BOOL mhal_bdma_is_busy(U32 channel);
BOOL mhal_bdma_clear_status(U32 channel);
void mhal_bdma_init(void);
#endif
