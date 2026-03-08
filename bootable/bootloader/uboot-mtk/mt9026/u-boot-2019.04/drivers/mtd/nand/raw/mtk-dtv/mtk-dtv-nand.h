// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#ifndef __MTK_DTV_NAND_H__
#define __MTK_DTV_NAND_H__

#include <linux/types.h>
#include <nand.h>

#define MAX_CLK_SOURCE	16

struct mtk_fcie_nand_clk_table {
	u32 clk_hz;
	u32 value;
};

struct mtk_fcie_nand_clk_info {
	void __iomem *clkbase;
	u32 clk_shift;
	u32 clk_bit_cnt;
	u32 clk_cnt;
	struct mtk_fcie_nand_clk_table clk_table[MAX_CLK_SOURCE];
};

struct mtk_fcie_nand_host {
	struct nand_chip chip;
	void __iomem *fciebase;
	struct mtk_fcie_nand_clk_info clk_info;
	struct nand_flash_dev table;
	struct nand_ecclayout ecclayout;
};

#endif
