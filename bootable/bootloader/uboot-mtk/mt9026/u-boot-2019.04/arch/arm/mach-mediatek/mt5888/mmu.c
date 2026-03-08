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
#include <asm/system.h>
#include <asm/armv8/mmu.h>

#define MMU_PAGE_SIZE       4096
#define RIU_PM_BASE         0x1F000000UL
#define RIU_PM_MAP_LEN      0x200000UL
#define RIU_NONPM_BASE      0x1F200000UL
#define RIU_NONPM_MAP_LEN   0x200000UL
#define RIU_EXT_BASE        0x1F600000UL
#define RIU_EXT_MAP_LEN     0x200000UL
#define RIU_FRC_BASE        0x1F800000UL
#define RIU_FRC_MAP_LEN     0x200000UL
#define SPI_BASE            0x14000000UL
#define SPI_MAP_LEN         0x01000000UL
#define MIU0_BUS_BASE       0x20000000UL
#define MIU0_MAP_LEN        0x40000000UL

static struct mm_region mtk_mem_map[] = {
    {
        .virt = RIU_PM_BASE,
        .phys = RIU_PM_BASE,
        .size = RIU_PM_MAP_LEN,     // RIU PM(2M)
        .attrs = 0
    }, {
        .virt = RIU_NONPM_BASE,
        .phys = RIU_NONPM_BASE,
        .size = RIU_NONPM_MAP_LEN,  // RIU non-PM(2M)
        .attrs = 0
    }, {
        .virt = RIU_EXT_BASE,
        .phys = RIU_EXT_BASE,
        .size = RIU_EXT_MAP_LEN,    // RIU EXT(2M)
        .attrs = 0
    }, {
        .virt = RIU_FRC_BASE,
        .phys = RIU_FRC_BASE,
        .size = RIU_FRC_MAP_LEN,    // RIU FRC(2M)
        .attrs = 0
    }, {
        .virt = 0x16000000UL,
        .phys = 0x16000000UL,
        .size = 0x00008000UL,       // ARM GIC(32K)
        .attrs = 0
    }, {
        .virt = SPI_BASE,
        .phys = SPI_BASE,
        .size = SPI_MAP_LEN,        // XIU SPI flash(16M)
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        .virt = MIU0_BUS_BASE,
        .phys = MIU0_BUS_BASE,
        .size = MIU0_MAP_LEN,       // MIU0 - cached
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        .virt = MIU0_BUS_BASE + MIU0_MAP_LEN,
        .phys = MIU0_BUS_BASE,
        .size = MIU0_MAP_LEN,       // MIU0 - uncached
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        /* List terminator */
        0,
    }
};

struct mm_region *mem_map = mtk_mem_map;

u64 get_page_table_size(void)
{
    u64 size = 0;
    u64 nr_pages = 0;

    nr_pages = sizeof(mtk_mem_map) / sizeof(struct mm_region);
    // Normal + enmergency page table size
    size = MMU_PAGE_SIZE * nr_pages * 2;

    return size;
}
