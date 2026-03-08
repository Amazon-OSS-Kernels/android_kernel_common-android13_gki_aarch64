// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2020 MediaTek Inc.
 * Author Kevin Ho <kevin-yc.ho@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/libfdt.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct mtk_dtv_pinctrl_pdata {
	fdt_addr_t base;	/* first configuration register */
	int offset;		/* index of last configuration register */
	u32 mask;		/* configuration-value mask bits */
	int width;		/* configuration register bit width */
};

struct mtk_dtv_fdt_pin_cfg {
	fdt32_t reg;		/* configuration register offset */
	fdt32_t sub_mask;	/* configuration register sub-mask */
	fdt32_t val;		/* configuration register value */
};

struct mtk_dtv_pinctrl_soc_data {
	bool can_get_gpio_func;
};

static int mtk_dtv_configure_pins(struct udevice *dev,
				 const struct mtk_dtv_fdt_pin_cfg *pins,
				 int size)
{
	struct mtk_dtv_pinctrl_pdata *pdata = dev->platdata;
	int count = size / sizeof(struct mtk_dtv_fdt_pin_cfg);
	phys_addr_t n, reg;
	u32 val, sub_mask = 0;

	for (n = 0; n < count; n++, pins++) {
		reg = fdt32_to_cpu(pins->reg);
		if ((reg < 0) || (reg > pdata->offset)) {
			dev_dbg(dev, "  invalid register offset 0x%pa\n", &reg);
			continue;
		}
		reg += pdata->base;
		sub_mask = fdt32_to_cpu(pins->sub_mask);
		val = fdt32_to_cpu(pins->val) & pdata->mask;

		switch (pdata->width) {
		case 8:
			if (sub_mask == 0xf0)
				val <<= 4;

			writeb((readb(reg) & ~sub_mask) | val, reg);
			break;
		case 16:
			writew((readw(reg) & ~sub_mask) | val, reg);
			break;
		case 32:
			writel((readl(reg) & ~sub_mask) | val, reg);
			break;
		default:
			dev_warn(dev, "unsupported register width %i\n",
				 pdata->width);
			continue;
		}
		dev_dbg(dev, "width/reg/val/sub_mask %i/0x%pa/0x%08x/0x%08x\n",
			pdata->width, &reg, val, sub_mask);
	}

	return 0;
}

static int mtk_dtv_set_state(struct udevice *dev,
			     struct udevice *config)
{
	const void *fdt = gd->fdt_blob;
	const struct mtk_dtv_fdt_pin_cfg *prop;
	int len;

	prop = fdt_getprop(fdt, dev_of_offset(config), "pinctrl-mtk,pins",
			   &len);
	if (prop) {
		dev_dbg(dev, "configuring pins for %s\n", config->name);
		if (len % sizeof(struct mtk_dtv_fdt_pin_cfg)) {
			dev_dbg(dev, "invalid pin configuration in fdt\n");
			return -FDT_ERR_BADSTRUCTURE;
		}
		mtk_dtv_configure_pins(dev, prop, len);
		len = 0;
	}

	return len;
}

static int mtk_dtv_get_gpio_mux(struct udevice *dev, int banknum,
				int index)
{
	struct mtk_dtv_pinctrl_pdata *pdata = dev->platdata;
	struct mtk_dtv_pinctrl_soc_data *soc_data =
		(struct mtk_dtv_pinctrl_soc_data *)dev_get_driver_data(dev);
	u32 group, reg_offset, bit;
	u8 val;
	int ret = 0;

	if (soc_data->can_get_gpio_func == false) {
		dev_dbg(dev, "get gpio mux not supported\n");
		return -ENOSYS;
	}

	group = index / 4;
	bit = index % 4;
	reg_offset = group * 4;
	if (bit >= 2)
		reg_offset += 1;

	bit %= 2;

	val = readb(pdata->base + reg_offset);
	if (bit)
		ret = (val >> 4) & 0xf;
	else
		ret = val & 0xf;

	dev_dbg(dev, "%s: reg_offset=0x%x, val=0x%x, ret=0x%x\n",
		__func__, reg_offset, val, ret);

	return ret;
}

static int mtk_dtv_ofdata_to_platdata(struct udevice *dev)
{
	fdt_addr_t addr;
	u32 of_reg[2];
	int res;
	struct mtk_dtv_pinctrl_pdata *pdata = dev->platdata;

	pdata->width = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				      "pinctrl-mtk,register-width", 0);

	res = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
				   "reg", of_reg, 2);
	if (res)
		return res;
	pdata->offset = of_reg[1] - pdata->width / 8;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		dev_dbg(dev, "no valid base register address\n");
		return -EINVAL;
	}
	pdata->base = addr;

	pdata->mask = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				     "pinctrl-mtk,function-mask",
				     0xffffffff);

	dev_dbg(dev, "%s: width=0x%x, offset=0x%x, mask=0x%x, base=0x%pa\n",
		__func__, pdata->width, pdata->offset,
		pdata->mask, &(pdata->base));

	return 0;
}

const struct pinctrl_ops mtk_dtv_pinctrl_ops = {
	.set_state = mtk_dtv_set_state,
	.get_gpio_mux = mtk_dtv_get_gpio_mux,
};

static const struct mtk_dtv_pinctrl_soc_data soc_data_pinctrl = {
	.can_get_gpio_func = true,
};

static const struct mtk_dtv_pinctrl_soc_data soc_data_pinconf = {
	.can_get_gpio_func = false,
};

static const struct udevice_id mtk_dtv_pinctrl_match[] = {
	{.compatible = "mediatek,mt5896-pinctrl",
	 .data = (ulong)&soc_data_pinctrl},
	{.compatible = "mediatek,mt5896-pinconf",
	 .data = (ulong)&soc_data_pinconf},
	{}
};

U_BOOT_DRIVER(pinctrl_mtk_dtv_mt5896) = {
	.name = "mtk_dtv_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mtk_dtv_pinctrl_match,
	.ops = &mtk_dtv_pinctrl_ops,
	.platdata_auto_alloc_size = sizeof(struct mtk_dtv_pinctrl_pdata),
	.ofdata_to_platdata = mtk_dtv_ofdata_to_platdata,
};
