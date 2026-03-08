// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

// TODO: this is temp solution to get watchdog reset flag
// It will be replaced by wdt instead.

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <reset.h>
#include <reset-uclass.h>
#include <wdt.h>
#include <asm/io.h>

#define	WDT_RESET_REG	0x198

/**
 * struct mt58xx_reset_priv - mt58xx private data
 *
 * @reg_base: register bank base address
 */
struct mt58xx_reset_priv {
	void __iomem *reg_base;
};


/**
 * mt58xx_reset_status() - check device reset status
 * @rst: Handle to a single reset signal
 *
 * This function implements the reset driver op to return the status of a
 * device's reset by mt58xx reset HW.
 * Return: 0 if reset is deasserted, or a non-zero value if reset is asserted
 */
static int mt58xx_reset_status(struct reset_ctl *rst)
{
	struct mt58xx_reset_priv *priv = dev_get_priv(rst->dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + WDT_RESET_REG;

	val = readw(addr);
	debug("wdt reg %p ori value 0x%04X\n", addr, val);

	return (val&0x1);
}

static int mt58xx_reset_probe(struct udevice *dev)
{
	struct mt58xx_reset_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// nothing to do

	return 0;
}

static int mt58xx_reset_ofdata_to_platdata(struct udevice *dev)
{
	struct mt58xx_reset_priv *priv = dev_get_priv(dev);

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static struct reset_ops mt58xx_reset_ops = {
	.rst_status = mt58xx_reset_status,
};

static const struct udevice_id mt58xx_reset_of_match[] = {
	{ .compatible = "mediatek,mt5896-wdtrst", },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(mt58xx_reset) = {
	.name = "mt58xx_reset",
	.id = UCLASS_RESET,
	.of_match = mt58xx_reset_of_match,
	.ops = &mt58xx_reset_ops,
	.probe = mt58xx_reset_probe,
	.ofdata_to_platdata = mt58xx_reset_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mt58xx_reset_priv),
};
