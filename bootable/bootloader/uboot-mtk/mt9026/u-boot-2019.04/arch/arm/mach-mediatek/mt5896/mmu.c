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
#include <mtk_mmu.h>
#include <system_impl.h>

static unsigned long long noncache_start_addr = MIU_CACHE_BUS_BASE1;
static unsigned long long cache1_size = MIU_CACHE_MAP_LEN1;

static struct mm_region mtk_mem_map[MEM_MAP_NUM] = {
    {
        .virt = RIU_PM_BASE,
        .phys = RIU_PM_BASE,
        .size = RIU_PM_MAP_LEN,     // RIU PM(2M)
        .attrs = 0
    }, {
        .virt = RIU_NONPM_BASE,
        .phys = RIU_NONPM_BASE,
        .size = RIU_NONPM_MAP_LEN,  // RIU non-PM(6M)
        .attrs = 0
    }, {
        .virt = RIU_DMD_BASE,
        .phys = RIU_DMD_BASE,
        .size = RIU_DMD_MAP_LEN,    // RIU DMD(2M)
        .attrs = 0
    }, {
        .virt = RIU_FRC_BASE,
        .phys = RIU_FRC_BASE,
        .size = RIU_FRC_MAP_LEN,    // RIU FRC(2M)
        .attrs = 0
    }, {
        .virt = RIU_AU_BASE,
        .phys = RIU_AU_BASE,
        .size = RIU_AU_MAP_LEN,     // RIU AU(2M)
        .attrs = 0
    }, {
        .virt = RIU_SCALAR_BASE,
        .phys = RIU_SCALAR_BASE,
        .size = RIU_SCALAR_MAP_LEN,    // RIU SC(4M)
        .attrs = 0
    }, {
        .virt = RIU_CODEC_BASE,
        .phys = RIU_CODEC_BASE,
        .size = RIU_CODEC_MAP_LEN,    // RIU CODEC(2M)
        .attrs = 0
    }, {
        .virt = RIU_HDMI_BASE,
        .phys = RIU_HDMI_BASE,
        .size = RIU_HDMI_MAP_LEN,    // RIU HDMI(2M)
        .attrs = 0
    }, {
        .virt = 0x16000000UL,
        .phys = 0x16000000UL,
        .size = 0x00100000UL,       // ARM GIC(1M)
        .attrs = 0
    }, {
        .virt = SPI_BASE,
        .phys = SPI_BASE,
        .size = SPI_MAP_LEN,        // XIU SPI flash(16M)
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        .virt = MIU_CACHE_BUS_BASE1,
        .phys = MIU_CACHE_BUS_BASE1,
        .size = MIU_CACHE_MAP_LEN1,       // MIU0 - cached
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        .virt = MIU_CACHE_BUS_BASE1,
        .phys = MIU_CACHE_BUS_BASE1,
        .size = MIU_NC_MAP_LEN,        // MIU0 - noncache (2M)
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        .virt = MIU_CACHE_BUS_BASE2,
        .phys = MIU_CACHE_BUS_BASE2,
        .size = MIU_CACHE_MAP_LEN2,       // MIU0 - cached
        .attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS
    }, {
        0,
    }, {
        /* List terminator */
        0,
    }
};

struct mm_region *mem_map = mtk_mem_map;

void init_mem_map(void)
{
    u64 dram_size = 0;
    u32 i = MEM_MAP_CACHE_BASE;

    dram_size = get_dram_size();
    if(dram_size <= 0)
        return;

    while(i<(MEM_MAP_CACHE_BASE + MEM_MAP_DEF_BLOCK_NUM)){
        if((mtk_mem_map[i].attrs & PTE_BLOCK_MEMTYPE(MT_NORMAL)) != 0 && mtk_mem_map[i].virt == MIU_CACHE_BUS_BASE1){
            mtk_mem_map[i].size = cache1_size;
        } else if((mtk_mem_map[i].attrs & PTE_BLOCK_MEMTYPE(MT_NORMAL_NC)) != 0 && mtk_mem_map[i].virt == MIU_CACHE_BUS_BASE1){
            mtk_mem_map[i].virt = noncache_start_addr;
            mtk_mem_map[i].phys = noncache_start_addr;
        } else if((mtk_mem_map[i].attrs & PTE_BLOCK_MEMTYPE(MT_NORMAL)) != 0 && mtk_mem_map[i].virt == MIU_CACHE_BUS_BASE2){
            if(dram_size > MIU_CACHE_MAP_LEN1 + MIU_CACHE_MAP_LEN2){
                mtk_mem_map[i].virt = noncache_start_addr + MIU_NC_MAP_LEN;
                mtk_mem_map[i].phys = noncache_start_addr + MIU_NC_MAP_LEN;
                mtk_mem_map[i].size = MIU_CACHE_MAP_LEN1 + MIU_CACHE_MAP_LEN2 - cache1_size - MIU_NC_MAP_LEN;
                mtk_mem_map[i+1].virt = MIU_CACHE_64BUS_BASE;
                mtk_mem_map[i+1].phys = MIU_CACHE_64BUS_BASE;
                mtk_mem_map[i+1].size = dram_size - MIU_CACHE_MAP_LEN1 - MIU_CACHE_MAP_LEN2;
                mtk_mem_map[i+1].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS;
            }else{
                mtk_mem_map[i].virt = noncache_start_addr + MIU_NC_MAP_LEN;
                mtk_mem_map[i].phys = noncache_start_addr + MIU_NC_MAP_LEN;
                mtk_mem_map[i].size = dram_size - cache1_size - MIU_NC_MAP_LEN;
                mtk_mem_map[i].attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
                PTE_BLOCK_INNER_SHARE | PTE_BLOCK_NS;
            }
        }
        i++;
    }

}

void set_nc_start_address(unsigned long long addr)
{
    noncache_start_addr = addr;
    cache1_size = noncache_start_addr - MIU_CACHE_BUS_BASE1;
}

u64 get_page_table_size(void)
{
    u64 size = 0;
    u64 nr_pages = 0;

    while(mtk_mem_map[nr_pages].size != 0)
        nr_pages++;

    // Normal + enmergency page table size
    size = MMU_PAGE_SIZE * nr_pages * 2;

    return size;
}
