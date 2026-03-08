// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2020 MediaTek Inc.
 * Author Kevin Ho <kevin-yc.ho@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/io.h>
#include <linux/bitmap.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>
#include <dm/pinctrl.h>
#include <hwspinlock.h>

#define MTK_GPIO_OEN_OFFSET	0x40
#define MTK_GPIO_OEN_IN		0
#define MTK_GPIO_OEN_OUT	1
#define MTK_GPIO_OUT_OFFSET	0x00
#define MTK_GPIO_IN_OFFSET	0x80
#define OFFSET_TO_BIT(bit)	(1UL << (bit))
/* hwspinlock timeout ms */
#define MTK_GPIO_HWSPINLOCK_TIMEOUT	10

struct mtk_dtv_gpio_priv {
	void __iomem *reg_base;
	struct udevice *pinctrl;
	char *bank_name;
	u32 gpio_count;
	u32 gpio_base;
	u32 gpio_max_count;
	struct hwspinlock hws;
	bool is_hwlock;
};

static int mtk_dtv_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	u8 val;
	u32 group, bit, reg_offset;
	int func_id, ret;

	if (offset >= priv->gpio_count) {
		dev_err(dev, "%s: Invalid gpio number %d\n",
			__func__, offset);
		return -EINVAL;
	}

	func_id = pinctrl_get_gpio_mux(priv->pinctrl, 0, offset);
	if (func_id)
		return GPIOF_FUNC;

	group = offset / 16;
	bit = offset % 16;

	reg_offset = MTK_GPIO_OEN_OFFSET + (group * 4);

	if (bit > 7)
		reg_offset += 1;

	if (priv->is_hwlock) {
		ret = hwspinlock_lock_timeout(&priv->hws, MTK_GPIO_HWSPINLOCK_TIMEOUT);

		if (ret) {
			dev_err(dev, "%s: cannot get hwspinlock: offset=%d (%d)\n",
				__func__, offset, ret);
			return ret;
		}
	}

	val = readb(priv->reg_base + reg_offset);

	if (priv->is_hwlock)
		hwspinlock_unlock(&priv->hws);

	bit %= 8;

	if (test_bit(bit, &val)) {
		dev_dbg(dev, "%s: offset %d, bit=0x%x, val=0x%x, output\n",
			__func__, offset, bit, val);
		return GPIOF_OUTPUT;
	} else {
		dev_dbg(dev, "%s: offset %d, bit=0x%x, val=0x%x, input\n",
			__func__, offset, bit, val);
		return GPIOF_INPUT;
	}
}

static int mtk_dtv_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	int func, ret;
	u8 val;
	u32 group, bit, reg_offset;

	if (offset >= priv->gpio_count) {
		dev_err(dev, "%s: Invalid gpio number %d\n",
			__func__, offset);
		return -EINVAL;
	}

	func = mtk_dtv_gpio_get_function(dev, offset);

	group = offset / 16;
	bit = offset % 16;

	if (func == GPIOF_OUTPUT)
		reg_offset = MTK_GPIO_OUT_OFFSET + (group * 4);
	else
		reg_offset = MTK_GPIO_IN_OFFSET + (group * 4);

	if (bit > 7)
		reg_offset += 1;

	if (priv->is_hwlock) {
		ret = hwspinlock_lock_timeout(&priv->hws, MTK_GPIO_HWSPINLOCK_TIMEOUT);

		if (ret) {
			dev_err(dev, "%s: cannot get hwspinlock: offset=%d (%d)\n",
				__func__, offset, ret);
			return ret;
		}
	}

	val = readb(priv->reg_base + reg_offset);

	if (priv->is_hwlock)
		hwspinlock_unlock(&priv->hws);

	bit %= 8;

	if (test_bit(bit, &val)) {
		dev_dbg(dev, "%s: offset %d, bit=0x%x, val=0x%x, 1\n",
			__func__, offset, bit, val);
		return 1;
	} else {
		dev_dbg(dev, "%s: offset %d, bit=0x%x, val=0x%x, 0\n",
			__func__, offset, bit, val);
		return 0;
	}
}

static int mtk_dtv_gpio_set_value(struct udevice *dev, unsigned int offset,
				  int value)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	u8 mask;
	u32 group, bit, reg_offset;
	int ret;

	if (offset >= priv->gpio_count) {
		dev_err(dev, "%s: Invalid gpio number %d\n",
			__func__, offset);
		return -EINVAL;
	}

	group = offset / 16;
	bit = offset % 16;

	reg_offset = MTK_GPIO_OUT_OFFSET + (group * 4);

	if (bit > 7)
		reg_offset += 1;

	bit %= 8;

	mask = OFFSET_TO_BIT(bit);

	if (priv->is_hwlock) {
		ret = hwspinlock_lock_timeout(&priv->hws, MTK_GPIO_HWSPINLOCK_TIMEOUT);

		if (ret) {
			dev_err(dev, "%s: cannot get hwspinlock: offset=%d (%d)\n",
				__func__, offset, ret);
			return ret;
		}
	}

	clrsetbits_8(priv->reg_base + reg_offset, mask, value ? mask : 0);

	if (priv->is_hwlock)
		hwspinlock_unlock(&priv->hws);

	dev_dbg(dev, "%s: offset %d, bit=0x%x, value=0x%x\n",
		__func__, offset, bit, value);

	return 0;
}

static int mtk_dtv_gpio_direction_input(struct udevice *dev,
					unsigned int offset)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	u8 mask;
	u32 group, bit, reg_offset;
	int ret;

	if (offset >= priv->gpio_count) {
		dev_err(dev, "%s: Invalid gpio number %d\n",
			__func__, offset);
		return -EINVAL;
	}

	group = offset / 16;
	bit = offset % 16;

	reg_offset = MTK_GPIO_OEN_OFFSET + (group * 4);

	if (bit > 7)
		reg_offset += 1;

	bit %= 8;

	mask = OFFSET_TO_BIT(bit);

	if (priv->is_hwlock) {
		ret = hwspinlock_lock_timeout(&priv->hws, MTK_GPIO_HWSPINLOCK_TIMEOUT);

		if (ret) {
			dev_err(dev, "%s: cannot get hwspinlock: offset=%d (%d)\n",
				__func__, offset, ret);
			return ret;
		}
	}

	clrsetbits_8(priv->reg_base + reg_offset, mask,
		     (MTK_GPIO_OEN_IN != 0) ? mask : 0);

	if (priv->is_hwlock)
		hwspinlock_unlock(&priv->hws);

	dev_dbg(dev, "%s: offset %d, bit=0x%x\n",
		__func__, offset, bit);

	return 0;
}

static int mtk_dtv_gpio_direction_output(struct udevice *dev,
					 unsigned int offset, int value)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	u8 mask;
	u32 group, bit, reg_offset;
	int ret;

	if (offset >= priv->gpio_count) {
		dev_err(dev, "%s: Invalid gpio number %d\n",
			__func__, offset);
		return -EINVAL;
	}

	/* set value, then direction */
	mtk_dtv_gpio_set_value(dev, offset, value);

	group = offset / 16;
	bit = offset % 16;

	reg_offset = MTK_GPIO_OEN_OFFSET + (group * 4);

	if (bit > 7)
		reg_offset += 1;

	bit %= 8;

	mask = OFFSET_TO_BIT(bit);

	if (priv->is_hwlock) {
		ret = hwspinlock_lock_timeout(&priv->hws, MTK_GPIO_HWSPINLOCK_TIMEOUT);

		if (ret) {
			dev_err(dev, "%s: cannot get hwspinlock: offset=%d (%d)\n",
				__func__, offset, ret);
			return ret;
		}
	}

	clrsetbits_8(priv->reg_base + reg_offset, mask,
		     (MTK_GPIO_OEN_OUT != 0) ? mask : 0);

	if (priv->is_hwlock)
		hwspinlock_unlock(&priv->hws);

	dev_dbg(dev, "%s: offset %d, bit=0x%x\n",
		__func__, offset, bit);

	return 0;
}

static const struct dm_gpio_ops gpio_mtk_dtv_ops = {
	.direction_input = mtk_dtv_gpio_direction_input,
	.direction_output = mtk_dtv_gpio_direction_output,
	.get_value = mtk_dtv_gpio_get_value,
	.set_value = mtk_dtv_gpio_set_value,
	.get_function = mtk_dtv_gpio_get_function,
};

static int gpio_mtk_dtv_probe(struct udevice *dev)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret;

	priv->bank_name = strdup(dev->name);
	if (!priv->bank_name)
		return -ENOMEM;

	uc_priv->gpio_base = priv->gpio_base;
	/* for gpio_renumber */
	uc_priv->gpio_count = priv->gpio_count;
	uc_priv->gpio_max_count = priv->gpio_max_count;
	uc_priv->bank_name = priv->bank_name;

	/* hwspinlock is optional */
	ret = hwspinlock_get_by_index(dev, 0, &priv->hws);
	if (ret) {
		priv->is_hwlock = false;
		dev_dbg(dev, "%s: hwspinlock_get_by_index failed: %d\n",
			__func__, ret);
	} else {
		priv->is_hwlock = true;
		dev_dbg(dev, "%s: hwspinlock supported\n",
			__func__);
	}

	return 0;
}

static int mtk_dtv_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_dtv_gpio_priv *priv = dev_get_priv(dev);
	int ret;

	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_u32(dev, "ngpios", &priv->gpio_count);
	if (ret) {
		dev_err(dev, "Read 'ngpios' from DTS fail (%d)", ret);
		return ret;
	}

	/*
	 * no used, gpio number will be listed automatically.
	 * sequence number is from DTS.
	 */
	ret = dev_read_u32(dev, "base", &priv->gpio_base);
	if (ret) {
		dev_err(dev, "Read 'base' from DTS fail (%d)", ret);
		return ret;
	}

	/*
	 * for gpio_renumber flow
	 */
	ret = dev_read_u32(dev, "max-ngpios", &priv->gpio_max_count);
	if (ret) {
		dev_err(dev, "Read 'max-ngpios' from DTS fail (%d)", ret);
		return ret;
	}

	ret = uclass_get_device_by_phandle(UCLASS_PINCTRL, dev,
					   "pinctrl-parent",
					   &priv->pinctrl);
	if (ret) {
		dev_err(dev, "Read 'pinctrl-parent' from DTS fail (%d)", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id mtk_dtv_gpio_ids[] = {
	{.compatible = "mediatek,mt5896-gpio"},
	{}
};

U_BOOT_DRIVER(gpio_mtk_dtv_mt5896) = {
	.name = "gpio_mtk_dtv",
	.id = UCLASS_GPIO,
	.ops = &gpio_mtk_dtv_ops,
	.of_match = mtk_dtv_gpio_ids,
	.ofdata_to_platdata = mtk_dtv_gpio_ofdata_to_platdata,
	.probe = gpio_mtk_dtv_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_dtv_gpio_priv),
};
