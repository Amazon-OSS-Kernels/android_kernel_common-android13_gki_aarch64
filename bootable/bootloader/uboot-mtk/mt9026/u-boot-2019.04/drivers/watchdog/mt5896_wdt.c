// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2022. All rights reserved.
 */


#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>

#define WDT_2ND_MASK_REG    (0x24)
#define WDT_TOGGLE_REG      (0x180)
#define	WDT_CNT_MAX0_REG    (0x190)
#define	WDT_CNT_MAX1_REG    (0x194)
#define	WDT_CNT_CTL_REG     (0x198)
#define LOW_BYTE_MASK       (0xFFFF)
#define HIGH_BYTE_OFFSET    (16)
#define XTAL_CLOCK_KHZ      (12000UL)
#define WDT_CYCLE_M(msec)   ((msec) * XTAL_CLOCK_KHZ)
#define	WDT_CNT_CTL_1ST_EN  (1 << 1)
#define	WDT_CNT_CTL_2ST_EN  (1 << 2)
#define WDT_2ND_CNT_OFFSET  (2 << 8)
#define WDT_CPU_2ND_EN      (1 << 0)
#define WDT_TOG_BIT         (1 << 1)

/**
 * struct mt5896_wdt_priv - mt5896 private data
 *
 * @reg_base: register bank base address
 */
struct mt5896_wdt_priv {
	void __iomem *reg_base;
	void __iomem *pm_reg_base;
};


static int mt5896_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + WDT_CNT_MAX0_REG;

	val = WDT_CYCLE_M(timeout)&LOW_BYTE_MASK;
	writew(val, addr);
	addr =
	    priv->reg_base + WDT_CNT_MAX1_REG;

	val = (WDT_CYCLE_M(timeout) >> HIGH_BYTE_OFFSET);
	writew(val, addr);

	addr =
	    priv->reg_base + WDT_CNT_CTL_REG;

	val = readw(addr);
	val |= WDT_CNT_CTL_1ST_EN;
	val |= WDT_CNT_CTL_2ST_EN;
	val &= ~WDT_2ND_CNT_OFFSET;
	writew(val, addr);

	addr =
	    priv->pm_reg_base + WDT_2ND_MASK_REG;

	val = readw(addr);
	val &= ~WDT_CPU_2ND_EN;
	writew(val, addr);
	return 0;
}

static int mt5896_wdt_stop(struct udevice *dev)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->pm_reg_base + WDT_2ND_MASK_REG;

	val = readw(addr);
	val |= WDT_CPU_2ND_EN;
	writew(val, addr);

	addr =
	    priv->reg_base + WDT_CNT_MAX0_REG;

	writew(0, addr);
	addr =
	    priv->reg_base + WDT_CNT_MAX1_REG;

	writew(0, addr);
	return 0;
}

static int mt5896_wdt_reset(struct udevice *dev)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + WDT_TOGGLE_REG;

	val = readw(addr);
	val |= WDT_TOG_BIT;
	writew(val, addr);
	return 0;
}

static int mt5896_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + WDT_TOGGLE_REG;

	val = readw(addr);
	val |= WDT_TOG_BIT;
	writew(val, addr);
	hang();

	return 0;
}

static int mt5896_wdt_probe(struct udevice *dev)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// nothing to do

	return 0;
}

static int mt5896_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct mt5896_wdt_priv *priv = dev_get_priv(dev);

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void *)dev_read_addr_name(dev, "cpu_wdt");
	priv->pm_reg_base = (void *)dev_read_addr_name(dev, "pm_ctl");
	if (!priv->reg_base)
		return -EINVAL;

	if (!priv->pm_reg_base)
		return -EINVAL;

	return 0;
}

static struct wdt_ops mt5896_wdt_ops = {
	.start = mt5896_wdt_start,
	.reset = mt5896_wdt_reset,
	.stop = mt5896_wdt_stop,
	.expire_now = mt5896_wdt_expire_now,
};

static const struct udevice_id mt5896_wdt_of_match[] = {
	{ .compatible = "mediatek,mt5896-watchdog", },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(mt5896_watchdog) = {
	.name = "mt5896_watchdog",
	.id = UCLASS_WDT,
	.of_match = mt5896_wdt_of_match,
	.ops = &mt5896_wdt_ops,
	.probe = mt5896_wdt_probe,
	.ofdata_to_platdata = mt5896_wdt_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mt5896_wdt_priv),
};
