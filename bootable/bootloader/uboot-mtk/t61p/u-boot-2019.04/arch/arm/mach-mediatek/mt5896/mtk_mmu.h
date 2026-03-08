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
#ifndef _MTK_MMU_
#define _MTK_MMU_

#define MEM_MAP_NUM             15
#define MEM_MAP_CACHE_BASE      10
#define MEM_MAP_DEF_BLOCK_NUM   3
#define MMU_PAGE_SIZE           4096
#define RIU_PM_BASE             0x1C000000UL
#define RIU_PM_MAP_LEN          0x200000UL
#define RIU_NONPM_BASE          0x1C200000UL
#define RIU_NONPM_MAP_LEN       0x600000UL
#define RIU_DMD_BASE            0x1CE00000UL
#define RIU_DMD_MAP_LEN         0x200000UL
#define RIU_FRC_BASE            0x1D000000UL
#define RIU_FRC_MAP_LEN         0x200000UL
#define RIU_AU_BASE             0x1D200000UL
#define RIU_AU_MAP_LEN          0x200000UL
#define RIU_SCALAR_BASE         0x1D400000UL
#define RIU_SCALAR_MAP_LEN      0x400000UL
#define RIU_CODEC_BASE         	0x1D800000UL
#define RIU_CODEC_MAP_LEN      	0x200000UL
#define RIU_HDMI_BASE          	0x1DC00000UL
#define RIU_HDMI_MAP_LEN       	0x200000UL
#define SPI_BASE                0x14000000UL
#define SPI_MAP_LEN             0x01000000UL
#define MIU_CACHE_BUS_BASE1     0x20000000UL
#define MIU_CACHE_MAP_LEN1      0x40000000UL
#define MIU_CACHE_BUS_BASE2     (MIU_CACHE_BUS_BASE1 + MIU_CACHE_MAP_LEN1)
#define MIU_CACHE_MAP_LEN2      0xA0000000UL
#define MIU_CACHE_64BUS_BASE    0x100000000UL
#define MIU_NC_MAP_LEN          0x200000UL
#define MIU0_BUS_BASE           MIU_CACHE_BUS_BASE1

void init_mem_map(void);
void set_nc_start_address(unsigned long long addr);
#endif