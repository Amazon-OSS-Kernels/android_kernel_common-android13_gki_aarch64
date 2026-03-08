/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019-2020 MediaTek Inc.
 * Author: Max Tsai <Max-CH.Tsai@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __MTK_MIUP_H
#define __MTK_MIUP_H
/*********************************************************************
 *                         Private Define                            *
 *********************************************************************/
#define PROT_CID_MAX_INDEX 0xFFFFFFFF
#define PROT_ADDR_ALIGN 0x1000
#define PROT_NULL_CID 0x0

#define miup_setbit(offset, flag, reg)					\
({									\
	typeof(offset) _offset = (offset);				\
	typeof(flag) _flag = (flag);					\
	typeof(reg) _reg = (reg);					\
	(_flag) ?							\
	writew((readw(_reg) | (1 << _offset)), _reg) :	\
	writew((readw(_reg) & ~(1 << _offset)), _reg);	\
})									\

#ifndef wmb
#define wmb()                   asm volatile (""   : : : "memory")
#endif

/*********************************************************************
 *                         Private Structure                         *
 *********************************************************************/

struct mtk_miup {
	void __iomem	*base;
	u32		max_prot_blk;
	u32		max_cid_group;
	u32		max_cid_number;
	u32		kcid_num;
	u64		*kcid;
	u32		end_addr_ver;
};

#define REG_HIT_LAST_LOG_OFFSET		BIT(3)
#define REG_PROT_W_EN			0x0000UL /* 0x0000UL */
#define REG_PROT_R_EN			0x0004UL /* 0x0001UL */
#define REG_PROT_INV			0x0008UL /* 0x0002UL */
#define REG_PROT_ID_GP_SEL		0x0018UL /* 0x0006UL */
#define REG_PROT_HIT_FLAG		0x0020UL /* 0x0008UL */
#define REG_PROT_W_HIT_MASK		0x0024UL /* 0x0009UL */
#define REG_PROT_R_HIT_MASK		0x0028UL /* 0x000AUL */
#define REG_PROT_HIT_UNMASK		0x002CUL /* 0x000BUL */
#define REG_PROT_OUT_OF_AREA_START_L	0x0030UL /* 0x000CUL */
#define REG_PROT_OUT_OF_AREA_START_H	0x0034UL /* 0x000DUL */
#define REG_PROT_OUT_OF_AREA_END_L	0x0038UL /* 0x000EUL */
#define REG_PROT_OUT_OF_AREA_END_H	0x003CUL /* 0x000FUL */
#define REG_PROT_0_START_L		0x0040UL /* 0x0010UL */
#define REG_PROT_0_START_H		0x0044UL /* 0x0011UL */
#define REG_PROT_0_END_L		0x0048UL /* 0x0012UL */
#define REG_PROT_0_END_H		0x004CUL /* 0x0013UL */
#define REG_PROT_0_ID_EN		0x00C0UL /* 0x0030UL */
#define REG_PROT_OUT_OF_AREA_ID_EN	0x00E0UL /* 0x0038UL */
#define REG_PROT_DMA_EN			0x00E4UL /* 0x0039UL */
#define REG_PROT_DMA_ADDR_START_L	0x00E8UL /* 0x003AUL */
#define REG_PROT_CHECK_ID_SELECT	0x00E8UL /* 0x003AUL */
#define REG_PROT_DMA_ADDR_START_H	0x00ECUL /* 0x003BUL */
#define REG_PROT_DMA_ADDR_END_L		0x00F0UL /* 0x003CUL */
#define REG_PROT_DMA_ADDR_END_H		0x00F4UL /* 0x003DUL */
#define REG_PROT_DMA_ID_1_0		0x00F8UL /* 0x003EUL */
#define REG_PROT_DMA_ID_3_2		0x00FCUL /* 0x003FUL */
#define REG_PROT_ID_0			0x0100UL /* 0x0040UL */
#define REG_PROT_G1_ID_0		0x0140UL /* 0x0050UL */
#define REG_PROT_ID_0_MASK		0x0180UL /* 0x0060UL */
#define REG_PROT_G1_ID_0_MASK		0x01C0UL /* 0x0070UL */
#define REG_PROT_W_HIT_ADDR_L		0x0000UL /* 0x0000UL */
#define REG_PROT_W_HIT_ADDR_H		0x0004UL /* 0x0001UL */
#define REG_PROT_W_HIT_LOG		0x0008UL /* 0x0002UL */
#define REG_PROT_W_HIT_ID		0x000CUL /* 0x0003UL */
#define REG_PROT_R_HIT_ADDR_L		0x0020UL /* 0x0008UL */
#define REG_PROT_R_HIT_ADDR_H		0x0024UL /* 0x0009UL */
#define REG_PROT_R_HIT_LOG		0x0028UL /* 0x000AUL */
#define REG_PROT_R_HIT_ID		0x002CUL /* 0x000BUL */
#define PROT_SADDR_L_OFFSET(blk)	((REG_PROT_0_START_L) + ((blk) * 0x10))
#define PROT_SADDR_H_OFFSET(blk)	(PROT_SADDR_L_OFFSET(blk) + 0x4)
#define PROT_EADDR_L_OFFSET(blk)	((REG_PROT_0_END_L) + ((blk) * 0x10))
#define PROT_EADDR_H_OFFSET(blk)	(PROT_EADDR_L_OFFSET(blk) + 0x4)
#define PROT_ID_EN_OFFSET(blk)		((REG_PROT_0_ID_EN) + ((blk) * 0x4))
#define PROT_ID_OFFSET(gp, idx) \
	((REG_PROT_ID_0) + (idx) * 0x4 + (gp) * 0x80)
#define PROT_ID_MASK_OFFSET(gp, idx) \
	((REG_PROT_ID_0_MASK) + (idx) * 0x4 + (gp) * 0x80)

#define CHECK_ID_OFFSET			(0)
#define CHECK_AID			(0)
#define CHECK_AXID			(1)

#define END_ADDR_VER_1			(1)
#define END_ADDR_VER_2			(2)
#define END_ADDR_VER_3			(3)

#endif /* __MTK_MIUP_H */
