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
#ifndef _HAL_REGOP_H_
#define _HAL_REGOP_H_

#define BIT0        0x0001
#define BIT1        0x0002
#define BIT2        0x0004
#define BIT3        0x0008
#define BIT4        0x0010
#define BIT5        0x0020
#define BIT6        0x0040
#define BIT7        0x0080
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

#ifdef CONFIG_DEMURA_MT5896
#define REG_BASE	(0x1C000000)

//---------------- AC_FULL/MSK ----------------
//  B3[31:24] | B2[23:16] | B1[15:8] | B0[7:0]
//  W32[31:16] | W21[23:8] | W10 [15:0]
//  DW[31:0]
//---------------------------------------------

#define AC_FULLB0           1
#define AC_FULLB1           2
#define AC_FULLB2           3
#define AC_FULLB3           4
#define AC_FULLW10          5
#define AC_FULLW21          6
#define AC_FULLW32          7
#define AC_FULLDW           8
#define AC_MSKB0            9
#define AC_MSKB1            10
#define AC_MSKB2            11
#define AC_MSKB3            12
#define AC_MSKW10           13
#define AC_MSKW21           14
#define AC_MSKW32           15
#define AC_MSKDW            16

//-------------------- Fld --------------------
//    wid[31:16] | shift[15:8] | ac[7:0]
//---------------------------------------------

#define Fld(wid, shft, ac)    (((MS_U32)wid<<16)|(shft<<8)|ac)
#define Fld_wid(fld)        (MS_U8)((fld)>>16)
#define Fld_shift(fld)       (MS_U8)((fld)>>8)
#define Fld_ac(fld)         (MS_U8)((fld))

void HAL_DEMURA_Write2Byte (
	MS_U32 u32P_Addr,
	MS_U32 u32Value,
	MS_U32 fld);

void HAL_DEMURA_Write2ByteMask(
	MS_U32 u32P_Addr,
	MS_U32 u32Value,
	MS_U32 fld);

MS_U32 HAL_DEMURA_Read2Byte(
	MS_U32 u32P_Addr);

MS_U32 HAL_DEMURA_Read2bytemask(
	MS_U32 u32P_Addr,
	MS_U32 fld);

MS_U32 HAL_DEMURA_Fld2Mask(MS_U32 fld);

MS_U32 HAL_DEMURA_Val2FldMask (MS_U32 u32Value, MS_U32 fld);

void HAL_DEMURA_WriteByteMask(MS_PHY phy64RegAddr, MS_U8 u8Val, MS_U8 u8mask);
#else
void HAL_DEMURA_SetIOMapBase(MS_VIRT nonpm_base);

MS_U8 HAL_DEMURA_ReadByte(MS_PHY phy64RegAddr);
MS_U16 HAL_DEMURA_Read2Byte(MS_PHY phy64RegAddr);

void HAL_DEMURA_WriteByte(MS_PHY phy64RegAddr, MS_U8 u8Val);
void HAL_DEMURA_Write2Byte(MS_PHY phy64RegAddr, MS_U16 u16Val);

MS_U8 HAL_DEMURA_ReadByteMask(MS_PHY phy64RegAddr, MS_U8 u8mask);
MS_U16 HAL_DEMURA_Read2ByteMask(MS_PHY phy64RegAddr, MS_U16 u16mask);

void HAL_DEMURA_WriteByteMask(MS_PHY phy64RegAddr, MS_U8 u8Val, MS_U8 u8mask);
void HAL_DEMURA_Write2ByteMask(MS_PHY phy64RegAddr, MS_U16 u16Val, MS_U16 u16mask);
#endif /* #ifdef CONFIG_DEMURA_MT5896 */

#endif  // _HAL_REGOP_H_
