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
/*
 * $Id: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h#4 $
 * $Header: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h#4 $
 * $Date: 2015/06/10 $
 * $DateTime: 2015/06/10 16:00:37 $
 * $Change: 1251790 $
 * $File: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h $
 * $Revision: #4 $
 */

#ifndef PLATEFORM_H
#define PLATEFORM_H

#include <asm/io.h>
#include <asm/types.h>
#include <linux/delay.h>
#include <stdio.h>

#ifndef PMU_MSG
#define PMU_MSG 1
#endif
/* Define trace levels. */
#define PMU_ERROR           (1)    /* Error condition debug messages. */
#define PMU_WARNING         (2)    /* Warning condition debug messages. */
#define PMU_DEBUG           (3)    /* debug condition debug messages. */
#define PMU_INFO            (4)    /* inforation condition debug messages. */

/* Higer debug level means more verbose */
#ifndef PMU_DEBUG_LEVEL
#define PMU_DEBUG_LEVEL                PMU_WARNING
#endif
#if defined(PMU_MSG) && (PMU_MSG)
#define PMU_printf    printf
#define PMU_log(dbg_lv, str, ...)                       \
        do {                                            \
                if (dbg_lv > PMU_DEBUG_LEVEL){          \
                        break;                          \
                } else {                                \
                        PMU_printf(str, ##__VA_ARGS__); \
                }                                       \
         } while(0)
#else /* PMU_MSG */
#define PMU_printf(...)
#define PMU_log(dbg_lv, str, ...)       do{}while(0)
#endif /* PMU_MSG */

typedef struct {
	phys_addr_t paganini;
	phys_addr_t pm_misc;
	phys_addr_t pm_top;
	phys_addr_t pm_sleep;
	phys_addr_t cpu_int;
	phys_addr_t inturrpt;
	phys_addr_t viv_bdma;
	phys_addr_t MALIBOX;
	phys_addr_t vivaldi4;
	phys_addr_t pm_imi; //block_arbiter_gp1;
	phys_addr_t vivaldi2;
	phys_addr_t ckgen00_pm;
	phys_addr_t ckgen01_pm;
	phys_addr_t vad_0;
	phys_addr_t vrec;
	phys_addr_t coin_cpu;
	phys_addr_t vad_1;
	phys_addr_t efuse_0;
} mtk_pmu_info ;
/*==========================================================================
    //CA4 Physical Address Mapping
===========================================================================*/
#define OFFSET(x)       ((x) << 1)
#define OFFSET8(x)      (x) * 2 - ((x) & 1) // for readb & writeb


/*==========================================================================
    Put the configuration base address of each IP here.
===========================================================================*/

/* macro to get at MMIO space when running virtually */

/* read register by word */
#define ms_readw(a)                 readw(a)
/* write register by word */
#define ms_writew(v,a)              writew(v,a)
 /* read register by byte */
#define ms_readb(a)                 readb(a)
/* write register by byte */
#define ms_writeb(v,a)              writeb(v,a)


#define INREG8(x)               ms_readb(x)
#define OUTREG8(x, y)           ms_writeb((unsigned char)(y), x)
#define SETREG8(x, y)           OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)           OUTREG8(x, INREG8(x)&~(y))
#define INSREG8(addr, mask, val) OUTREG8(addr, ((INREG8(addr)&(~(mask))) | val))

#define INREG16(x)              ms_readw(x)
#define OUTREG16(x, y)          ms_writew((unsigned short)(y), x)
#define SETREG16(x, y)          OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)          OUTREG16(x, INREG16(x)&~(y))
#define INSREG16(addr, mask, val) OUTREG16(addr, ((INREG16(addr)&(~(mask))) | val))


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL unsigned int
#endif
#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef WORD
#define WORD unsigned short
#endif
#ifndef DWORD
#define DWORD unsigned int
#endif
#ifndef U8
#define U8 unsigned char
#endif
#ifndef U16
#define U16 unsigned short
#endif
#ifndef U32
#define U32 unsigned int
#endif
#ifndef S32
#define S32 int
#endif

#ifndef BIT0
#define BIT0  0x1
#endif
#ifndef BIT1
#define BIT1  0x2
#endif
#ifndef BIT2
#define BIT2  0x4
#endif
#ifndef BIT3
#define BIT3  0x8
#endif
#ifndef BIT4
#define BIT4  0x10
#endif
#ifndef BIT5
#define BIT5  0x20
#endif
#ifndef BIT6
#define BIT6  0x40
#endif
#ifndef BIT7
#define BIT7  0x80
#endif
#ifndef BIT8
#define BIT8  0x100
#endif
#ifndef BIT9
#define BIT9  0x200
#endif
#ifndef BIT10
#define BIT10 0x400
#endif
#ifndef BIT11
#define BIT11 0x800
#endif
#ifndef BIT12
#define BIT12 0x1000
#endif
#ifndef BIT13
#define BIT13 0x2000
#endif
#ifndef BIT14
#define BIT14 0x4000
#endif
#ifndef BIT15
#define BIT15 0x8000
#endif

#endif //PLATEFORM_H
