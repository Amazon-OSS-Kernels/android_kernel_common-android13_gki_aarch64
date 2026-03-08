// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 * Author Kevin Ho <kevin-yc.ho@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <hwspinlock.h>
#include <asm/io.h>

struct mtk_dtv_hwspinlock {
	fdt_addr_t base;
};

/* ID of this processor */
/**
 * PM51: 1
 * R2  : 2
 * ARM : 3
 */
#define HW_SPINLOCK_PROCESSOR_ID 3
/* Number of Hardware Spinlocks*/
#define HW_SPINLOCK_NUMBER 16

/* Hardware spinlock register offsets */
#define HW_SPINLOCK_OFFSET(x) (0x4 * (x))

static int mtk_dtv_hwspinlock_lock(struct udevice *dev, int index)
{
	struct mtk_dtv_hwspinlock *priv = dev_get_priv(dev);
	u32 val, reg_offset;

	if (index >= HW_SPINLOCK_NUMBER)
		return -EINVAL;

	reg_offset = HW_SPINLOCK_OFFSET(index);

	writel(HW_SPINLOCK_PROCESSOR_ID, priv->base + reg_offset);
	val = readl(priv->base + reg_offset);

	dev_dbg(dev,
		"hwspinlock lock: addr=0x%lx, val=0x%x, id=0x%x\n",
		(unsigned long)(priv->base + reg_offset),
		val, HW_SPINLOCK_PROCESSOR_ID);

	if (val != HW_SPINLOCK_PROCESSOR_ID)
		return -EBUSY;
	else
		return 0;
}

static int mtk_dtv_hwspinlock_unlock(struct udevice *dev, int index)
{
	struct mtk_dtv_hwspinlock *priv = dev_get_priv(dev);
	u32 val, reg_offset;

	if (index >= HW_SPINLOCK_NUMBER)
		return -EINVAL;

	reg_offset = HW_SPINLOCK_OFFSET(index);

	val = readl(priv->base + reg_offset);

	if ((val != 0) && (val != HW_SPINLOCK_PROCESSOR_ID)) {
		dev_warn(dev,
			 "hwspinlock unlock: lock not owned by us\n");
		dev_warn(dev,
			 "hwspinlock unlock: addr=0x%lx, val=0x%x, id=0x%x\n",
			 (unsigned long)(priv->base + reg_offset),
			 val, HW_SPINLOCK_PROCESSOR_ID);
	}

	dev_dbg(dev,
		"hwspinlock unlock: addr=0x%lx, val=0x%x, id=0x%x\n",
		(unsigned long)(priv->base + reg_offset),
		val, HW_SPINLOCK_PROCESSOR_ID);

	writel(0, priv->base + reg_offset);

	return 0;
}

static int mtk_dtv_hwpinlock_probe(struct udevice *dev)
{
	struct mtk_dtv_hwspinlock *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct hwspinlock_ops mtk_dtv_hwpinlock_ops = {
	.lock = mtk_dtv_hwspinlock_lock,
	.unlock = mtk_dtv_hwspinlock_unlock,
};

static const struct udevice_id mtk_dtv_hwpinlock_ids[] = {
	{ .compatible = "mediatek,mt5896-hwspinlock" },
	{}
};

U_BOOT_DRIVER(hwspinlock_mtk_dtv_mt5896) = {
	.name = "mt5896-hwspinlock",
	.id = UCLASS_HWSPINLOCK,
	.of_match = mtk_dtv_hwpinlock_ids,
	.ops = &mtk_dtv_hwpinlock_ops,
	.probe = mtk_dtv_hwpinlock_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_dtv_hwspinlock),
};
