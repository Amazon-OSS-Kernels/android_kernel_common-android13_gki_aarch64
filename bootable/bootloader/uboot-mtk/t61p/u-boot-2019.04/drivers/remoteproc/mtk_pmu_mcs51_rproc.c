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
 * Copyright(C) 2021 MediaTek Inc.
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
 * Copyright(C) 2021 MediaTek Inc.
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
#include <asm/io.h>
#include <common.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass-id.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <remoteproc.h>

#include "debug_impl.h"

/*****************************************************************************/
/* MCS51 */
#define REG_OFS_START_L			(((0x02 << 1) * 2) + 0)
#define REG_OFS_START_H			(((0x00 << 1) * 2) + 0)
#define REG_VAL_DEFAULT			(0x0000)
#define REG_OFS_END_L			(((0x03 << 1) * 2) + 0)
#define REG_OFS_END_H			(((0x01 << 1) * 2) + 0)
#define REG_OFS_MCU_CFG			(((0x0C << 1) * 2) + 0)
#define REG_VAL_EN_PSRAM		(BIT(0))
#define REG_VAL_DIS_SPI			(BIT(1))
#define REG_VAL_DIS_DRAM		(BIT(2))
#define REG_VAL_RST_CACHE		(BIT(3))
#define REG_OFS_MCU_RST			(((0x29 << 1) * 2) + 1)
#define REG_VAL_MCU_RST			(BIT(0) | BIT(4))
#define REG_OFS_MCU_PW			(((0x2A << 1) * 2) + 0)
#define REG_VAL_MCU_PW			(0x829F)
#define REG_OFS_CACHE_CFG		(((0x50 << 1) * 2) + 0)
#define REG_VAL_DIS_ICACHE		(BIT(0))

/* PM_IPI */
#define REG_OFS_IPI_DUMMY		(((0x50 << 1) * 2) + 1)
#define REG_VAL_IPI_BOOT		(0xBB)
#define REG_VAL_IPI_DEAD		(0xDD)
#define REG_VAL_IPI_NONE		(0x00)

#define IPI_DELAY				(20)
#define IPI_TIMEOUT				(2000000)

/* BDMA */
#define REG_OFS_BDMA_CFG		(((0x00 << 1) * 2) + 0)
#define REG_VAL_TRIGGER			(BIT(0))
#define REG_VAL_SRC_A_H			(BIT(10) | BIT(11))
#define REG_OFS_DIRECTION		(((0x02 << 1) * 2) + 0)
#define REG_VAL_DRAM_TO_SRAM	(0x0A40)
#define REG_OFS_SRC_L			(((0x04 << 1) * 2) + 0)
#define REG_OFS_SRC_H			(((0x05 << 1) * 2) + 0)
#define REG_OFS_DST_L			(((0x06 << 1) * 2) + 0)
#define REG_OFS_DST_H			(((0x07 << 1) * 2) + 0)
#define REG_OFS_SZ_L			(((0x08 << 1) * 2) + 0)
#define REG_OFS_SZ_H			(((0x09 << 1) * 2) + 0)

#define BDMA_DELAY				(20)
#define BDMA_TIMEOUT			(10000)

/* CLK */
#define REG_OFS_MCU_GATE_BY_SLEEP	(((0x0B << 1) * 2) + 0)
#define REG_OFS_CKG_MCU_NONPM		(((0x14 << 1) * 2) + 1)
#define REG_OFS_CKG_XTAL_12M		(((0x1E << 1) * 2) + 1)
#define REG_OFS_SW_EN_SMI2MCU		(((0x3A << 1) * 2) + 1)
#define REG_OFS_SW_EN_MCU_PM2ALL	(((0x48 << 1) * 2) + 0)
#define REG_OFS_SW_EN_XTAL_12M2MCU	(((0x4F << 1) * 2) + 1)
#define REG_OFS_SW_EN_MCU_NONPM2MCU	(((0x5F << 1) * 2) + 0)
#define REG_OFS_CKG_SMI				(((0x7A << 1) * 2) + 0)
#define REG_VAL_CLK_SET				(0xFF)
#define REG_VAL_CLK_CLR				(0x00)

/* MISC */
#define BUFF_ALIGN				(0x10000)
#define BUFF_BOUNDARY			(BIT(32))

/*****************************************************************************/
struct mtk_pmu_mcs51_privdata {
	u32 sram_size;
	u32 base_addr;
	ulong reg_pm_misc;
	ulong reg_pm_sleep;
	ulong reg_mcu;
	ulong reg_cache;
	ulong reg_bdma;
	ulong reg_ckgen01_pm;
	ulong reg_pm_por_status;
	ulong reg_ckgen00_0;
	ulong reg_ckgen00_1;
	ulong reg_ckgen00_A;
	ulong reg_ckgen01_D;
	ulong reg_ckgen01_8;
};

/*****************************************************************************/
/**
 * mtk_pmu_mcs51_is_running() - Check the remote processor status
 * @dev:	rproc device pointer
 *
 * Return: 0 if running, 1 if not running
 */
static int mtk_pmu_mcs51_is_running(struct udevice *dev)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);
	int wait_time = 0;

	UBOOT_TRACE("mcs51 check running.\n");

	/* Send event. */
	writeb(REG_VAL_IPI_BOOT, priv->reg_pm_sleep + REG_OFS_IPI_DUMMY);

	/* Wait ack. */
	while (readb(priv->reg_pm_sleep + REG_OFS_IPI_DUMMY) !=
				REG_VAL_IPI_NONE) {
		udelay(IPI_DELAY);
		wait_time += IPI_DELAY;
		if (wait_time >= IPI_TIMEOUT) {
			UBOOT_ERROR("wait mcs51 boot %d us timeout.\n",
				    IPI_TIMEOUT);
			return 1;
		}
		UBOOT_DEBUG("wait mcs51 boot %d us.\n", wait_time);
	}

	return 0;
}

/**
 * mtk_pmu_mcs51_start() - Start the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pmu_mcs51_start(struct udevice *dev)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);

	UBOOT_TRACE("mcs51 start.\n");

	// release 8051 reset pin
	setbits_8(priv->reg_pm_misc + REG_OFS_MCU_RST, REG_VAL_MCU_RST);
	return 0;
}

/**
 * mtk_pmu_mcs51_stop() - Stop the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pmu_mcs51_stop_check(struct udevice *dev, bool check)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);
	int wait_time = 0;

	UBOOT_TRACE("mcs51 stop (%d).\n", check);

	if (check) {
		/* Notify mcs51 to dead, aoivd mcu access interrupt. */
		writeb(REG_VAL_IPI_DEAD,
		       priv->reg_pm_sleep + REG_OFS_IPI_DUMMY);

		/* Wait ack. */
		while (readb(priv->reg_pm_sleep + REG_OFS_IPI_DUMMY) !=
					REG_VAL_IPI_NONE) {
			udelay(IPI_DELAY);
			wait_time += IPI_DELAY;
			if (wait_time >= IPI_TIMEOUT) {
				UBOOT_ERROR("wait mcs51 stop %d us timeout.\n",
					    IPI_TIMEOUT);
				break;
			}
			UBOOT_DEBUG("wait mcs51 stop %d us.\n", wait_time);
		}
	}

	// hold 8051 reset pin
	clrbits_8(priv->reg_pm_misc + REG_OFS_MCU_RST, REG_VAL_MCU_RST);
	return 0;
}

static int mtk_pmu_mcs51_stop(struct udevice *dev)
{
	return mtk_pmu_mcs51_stop_check(dev, true);
}

/**
 * mtk_pmu_mcs51_init() - Initialize the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pmu_mcs51_init(struct udevice *dev)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);

	UBOOT_TRACE("init clk, mcs51.\n");

	// avoid mcs51 running
	mtk_pmu_mcs51_stop_check(dev, false);

	// non-die clk setting
	writeb(REG_VAL_CLK_SET, priv->reg_ckgen01_pm + REG_OFS_SW_EN_MCU_PM2ALL);
	writew(REG_VAL_CLK_CLR, priv->reg_pm_por_status + REG_OFS_MCU_GATE_BY_SLEEP);

	// die clk setting
	writeb(REG_VAL_CLK_SET, priv->reg_ckgen00_A + REG_OFS_SW_EN_SMI2MCU);
	writeb(REG_VAL_CLK_CLR, priv->reg_ckgen00_0 + REG_OFS_CKG_SMI);
	writeb(REG_VAL_CLK_SET, priv->reg_ckgen00_A + REG_OFS_SW_EN_XTAL_12M2MCU);
	writeb(REG_VAL_CLK_CLR, priv->reg_ckgen00_1 + REG_OFS_CKG_XTAL_12M);
	writeb(REG_VAL_CLK_SET, priv->reg_ckgen01_D + REG_OFS_SW_EN_MCU_NONPM2MCU);
	writeb(REG_VAL_CLK_CLR, priv->reg_ckgen01_8 + REG_OFS_CKG_MCU_NONPM);

	// disable i cache (enable icache bypass)
	setbits_8(priv->reg_cache + REG_OFS_CACHE_CFG, REG_VAL_DIS_ICACHE);

	// disable spi
	clrbits_8(priv->reg_mcu + REG_OFS_MCU_CFG, REG_VAL_DIS_SPI);
	// disable dram
	clrbits_8(priv->reg_mcu + REG_OFS_MCU_CFG, REG_VAL_DIS_DRAM);
	// disable cache
	clrbits_8(priv->reg_mcu + REG_OFS_MCU_CFG, REG_VAL_RST_CACHE);
	// enable psram
	setbits_8(priv->reg_mcu + REG_OFS_MCU_CFG, REG_VAL_EN_PSRAM);

	// start addr (0K)
	writew(REG_VAL_DEFAULT, priv->reg_mcu + REG_OFS_START_L);
	writew(REG_VAL_DEFAULT, priv->reg_mcu + REG_OFS_START_H);
	// end addr (sram size)
	writew((u16)(priv->sram_size >>  0), priv->reg_mcu + REG_OFS_END_L);
	writew((u16)(priv->sram_size >> 16), priv->reg_mcu + REG_OFS_END_H);

	// mcu pw
	writew(REG_VAL_MCU_PW, priv->reg_pm_misc + REG_OFS_MCU_PW);

	return 0;
}

/**
 * mtk_pmu_mcs51_load() - Loadup the mcs51 remote processor
 * @dev:	rproc device pointer
 * @addr:	Address in memory where image binary is stored
 * @size:	Size in bytes of the image binary
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_pmu_mcs51_load(struct udevice *dev, ulong addr, ulong size)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);
	u16 data = 0;
	int wait_time = 0;

	UBOOT_TRACE("load fw from host_buff=0x%lX to sram.\n", addr);
	if ((size > priv->sram_size)) {
		UBOOT_ERROR("para_sz=0x%lX but priv_sz=0x%X out of sram.\n", size, priv->sram_size);
		return -ENOMEM;
	}

	// reg_sw_en_smi2mcu_bdma control by sboot
	// BANK=0x1020, OFFSET=0x53Ah, BIT=1, VAL=1;

	// direction
	writew(REG_VAL_DRAM_TO_SRAM, priv->reg_bdma + REG_OFS_DIRECTION);

	// adujust dma addr
	addr -= priv->base_addr;
	UBOOT_TRACE("addr convert dram_buff=0x%lX.\n", addr);
	// adjust src_addr when address out of data.
	if (addr >= BUFF_BOUNDARY) {
		UBOOT_DEBUG("org_dram_buff(0x%lX) out of boundary(0x%lX).\n", addr, BUFF_BOUNDARY);
		data = ((addr >> 32) << 10) & REG_VAL_SRC_A_H;
		writew(data, priv->reg_bdma + REG_OFS_BDMA_CFG);
		addr -= BUFF_BOUNDARY;
		UBOOT_DEBUG("new_dram_buff(0x%lX) adjust result.\n", addr);
	}

	// src_addr
	writew((u16)(addr >>  0), priv->reg_bdma + REG_OFS_SRC_L);
	writew((u16)(addr >> 16), priv->reg_bdma + REG_OFS_SRC_H);

	// dst_addr
	writew(REG_VAL_DEFAULT, priv->reg_bdma + REG_OFS_DST_L);
	writew(REG_VAL_DEFAULT, priv->reg_bdma + REG_OFS_DST_H);

	// data_size
	writew((u16)(size >>  0), priv->reg_bdma + REG_OFS_SZ_L);
	writew((u16)(size >> 16), priv->reg_bdma + REG_OFS_SZ_H);

	// tirgger dma
	writeb(REG_VAL_TRIGGER, priv->reg_bdma + REG_OFS_BDMA_CFG);

	// check done
	do {
		udelay(BDMA_DELAY);
		wait_time += BDMA_DELAY;
		if (wait_time >= BDMA_TIMEOUT) {
			UBOOT_ERROR("wait dma done %d us timeout.\n",
				    BDMA_TIMEOUT);
			return -ETIMEDOUT;
		}
		UBOOT_INFO("wait dma done %d us.\n", wait_time);
	} while (readb(priv->reg_bdma + REG_OFS_BDMA_CFG) & REG_VAL_TRIGGER);

	return 0;
}

/**
 * mtk_pmu_mcs51_reset() - Reset the remote processor
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pmu_mcs51_reset(struct udevice *dev)
{
	int ret = 0;

	UBOOT_TRACE("mcs51 reset.\n");

	if (((ret = mtk_pmu_mcs51_stop(dev)) < 0) ||
		((ret = mtk_pmu_mcs51_start(dev)) < 0))
		UBOOT_ERROR("reset fail.\n");
	return ret;
}

#if defined(SUPPORT_MCS51_ELF_LOADER)
/**
 * mtk_pmu_mcs51_device_to_virt() - Convert device address.
 * @dev:	rproc device pointer
 * @da:		Address in device
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static void * mtk_pmu_mcs51_device_to_virt(struct udevice *dev, ulong da)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);

	UBOOT_TRACE("covert address.\n");

	return (void *)(da + priv->base_addr);
}
#endif

static const struct dm_rproc_ops mtk_pmu_mcs51_ops = {
	.is_running = mtk_pmu_mcs51_is_running,
	.start = mtk_pmu_mcs51_start,
	.stop = mtk_pmu_mcs51_stop,
	.init = mtk_pmu_mcs51_init,
	.load = mtk_pmu_mcs51_load,
	.reset = mtk_pmu_mcs51_reset,
#if defined(SUPPORT_MCS51_ELF_LOADER)
	.device_to_virt = mtk_pmu_mcs51_device_to_virt,
#endif
};

static const struct udevice_id mtk_pmu_mcs51_ids[] = {
	{.compatible = "mediatek,merak-pmu-mcs51-rproc"},
	{}
};

/**
 * mtk_pmu_mcs51_probe() - Basic probe.
 * @dev:	rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pmu_mcs51_probe(struct udevice *dev)
{
	struct mtk_pmu_mcs51_privdata *priv = dev_get_priv(dev);
	ofnode node = {0};

	UBOOT_TRACE("parsing device tree.\n");

	memset(priv, 0, sizeof(struct mtk_pmu_mcs51_privdata));
	/* Get sram size. */
	if (dev_read_u32(dev, "sram_size", &priv->sram_size) < 0) {
		UBOOT_ERROR("sram_size not found.\n");
		return -EIO;
	}
	UBOOT_INFO("sram_size=0x%X.\n", priv->sram_size);

	/* Get cpu bus address. */
	node = ofnode_path("/memory_info");
	if (!ofnode_valid(node) || (ofnode_read_u32(node, "cpu_emi0_base", &priv->base_addr) < 0)) {
		UBOOT_ERROR("node(/memory_info/cpu_emi0_base) not found.\n");
		return -EIO;
	}
	UBOOT_INFO("base_addr=0x%X.\n", priv->base_addr);

	priv->reg_pm_misc = dev_read_addr_name(dev, "pm_misc");
	priv->reg_pm_sleep = dev_read_addr_name(dev, "pm_sleep");
	priv->reg_mcu = dev_read_addr_name(dev, "mcu");
	priv->reg_cache = dev_read_addr_name(dev, "cache");
	priv->reg_bdma = dev_read_addr_name(dev, "bdma");
	priv->reg_ckgen01_pm = dev_read_addr_name(dev, "ckgen01_pm");
	priv->reg_pm_por_status = dev_read_addr_name(dev, "pm_por_status");
	priv->reg_ckgen00_0 = dev_read_addr_name(dev, "ckgen00_0");
	priv->reg_ckgen00_1 = dev_read_addr_name(dev, "ckgen00_1");
	priv->reg_ckgen00_A = dev_read_addr_name(dev, "ckgen00_A");
	priv->reg_ckgen01_D = dev_read_addr_name(dev, "ckgen01_D");
	priv->reg_ckgen01_8 = dev_read_addr_name(dev, "ckgen01_8");
	UBOOT_INFO("pm_misc=0x%lX.\n", priv->reg_pm_misc);
	UBOOT_INFO("pm_sleep=0x%lX.\n", priv->reg_pm_sleep);
	UBOOT_INFO("mcu=0x%lX.\n", priv->reg_mcu);
	UBOOT_INFO("cache=0x%lX.\n", priv->reg_cache);
	UBOOT_INFO("bdma=0x%lX.\n", priv->reg_bdma);
	UBOOT_INFO("ckgen01_pm=0x%lX.\n", priv->reg_ckgen01_pm);
	UBOOT_INFO("pm_por_status=0x%lX.\n", priv->reg_pm_por_status);
	UBOOT_INFO("ckgen00_0=0x%lX.\n", priv->reg_ckgen00_0);
	UBOOT_INFO("ckgen00_1=0x%lX.\n", priv->reg_ckgen00_1);
	UBOOT_INFO("ckgen00_A=0x%lX.\n", priv->reg_ckgen00_A);
	UBOOT_INFO("ckgen01_D=0x%lX.\n", priv->reg_ckgen01_D);
	UBOOT_INFO("ckgen01_8=0x%lX.\n", priv->reg_ckgen01_8);

	return 0;
}

U_BOOT_DRIVER(mtk_pmu_mcs51) = {
	.name = "mtk_pmu_mcs51_rproc",
	.id = UCLASS_REMOTEPROC,
	.of_match = mtk_pmu_mcs51_ids,
	.probe = mtk_pmu_mcs51_probe,
	.ops = &mtk_pmu_mcs51_ops,
	.priv_auto_alloc_size = sizeof(struct mtk_pmu_mcs51_privdata),
};
