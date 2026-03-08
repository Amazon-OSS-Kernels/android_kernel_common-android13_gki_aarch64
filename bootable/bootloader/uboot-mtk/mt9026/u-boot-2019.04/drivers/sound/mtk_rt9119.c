// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#include <common.h>
#include <audio_codec.h>
#include <div64.h>
#include <dm.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include <asm/gpio.h>

#include "mtk_rt9119.h"

u8 ampinittblrt9119[] = {
//DataLen	Address	DataN	DataN+1...
	1,	0x02,	0x06,
	1,	0x0E,	0x07,
	1,	0xD8,	0x01,
	1,	0x10,	0x21,
	1,	0x06,	0x07,
	1,	0xE0,	0x31,
	2,	0x07,	0x01,	0x80,	//volume to 0db
	1,	0x05,	0x01,
	0x00,
};

static int mtk_amp_i2c_write(struct udevice *dev, u8 reg, u8 *buf, u8 size)
{
	struct amp_rt9119_priv *priv = dev_get_priv(dev);
	struct i2c_msg msgs;
	u8 msg_buf[10];
	int ret;
	int i;

	msg_buf[0] = reg;

	for (i = 1; i <= size; i++)
		msg_buf[i] = buf[i - 1];

	msgs.addr  = priv->i2c_addr;
	msgs.len   = size + 1;
	msgs.buf   = msg_buf;
	msgs.flags = I2C_M_STOP;

	ret = dm_i2c_xfer(dev, &msgs, 1);
	if (ret < 0)
		return ret;

	return 0;
}

static int amp_write(struct udevice *dev, unsigned char *inittable)
{
	u8 *pu8str = inittable;
	u8 *buf;
	u8 reg;
	u8 size;
	int ret;

	do {
		size = *pu8str;
		pu8str++;

		reg = *pu8str;
		pu8str++;

		buf = pu8str;
		ret = mtk_amp_i2c_write(dev, reg, buf, size);
		if (ret < 0) {
			dev_err(dev, "[AMP 9119]i2c write fail %d\n", ret);
			return -1;
		}

		pu8str = pu8str + size;
	} while (*pu8str != 0);

	return 0;
}

static int amp_rt9119_reset(struct udevice *dev, bool enable)
{
	struct amp_rt9119_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_reset))
		dm_gpio_set_value(&priv->amp_reset, enable);

	return 0;
}

static int amp_rt9119_powerdown(struct udevice *dev, bool enable)
{
	struct amp_rt9119_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_powerdown))
		dm_gpio_set_value(&priv->amp_powerdown, enable);

	return 0;
}

static int amp_rt9119_set_params(struct udevice *dev, int interface, int rate,
				 int mclk_freq, int bits_per_sample, uint channels)
{
	int ret;

	amp_rt9119_reset(dev, 1);
	mdelay(1);
	amp_rt9119_powerdown(dev, 0);
	mdelay(1);
	amp_rt9119_reset(dev, 0);
	mdelay(11);

	ret = amp_write(dev, ampinittblrt9119);
	if (ret < 0) {
		dev_err(dev, "[AMP 9119]Unable to init AMP RT9119\n");
		return -EINVAL;
	}

	return 0;
}

static int amp_rt9119_probe(struct udevice *dev)
{
	struct amp_rt9119_priv *priv = dev_get_priv(dev);
	int ret;

	if (dev_read_enabled(dev) == 0)
		return 0;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C) {
		dev_err(dev, "[AMP 9119]get i2c parent fail\n");
		return -EPROTONOSUPPORT;
	}

	ret = dev_read_u32(dev, "reg", &priv->i2c_addr);
	if (ret) {
		dev_err(dev, "[AMP 9119]can't get slave addr\n");
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "amppowerdown-gpios", 0, &priv->amp_powerdown, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 9119]get amp power down gpio fail %d\n", ret);
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "ampreset-gpios", 0, &priv->amp_reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 9119]get amp reset gpio fail %d\n", ret);
		return -EINVAL;
	}

	return 0;
}

static const struct audio_codec_ops amp_rt9119_ops = {
	.set_params	= amp_rt9119_set_params,
};

static const struct udevice_id amp_rt9119_ids[] = {
	{ .compatible = "mediatek, rt9119" },
	{ }
};

U_BOOT_DRIVER(rt9119) = {
	.name		= "rt9119",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= amp_rt9119_ids,
	.probe		= amp_rt9119_probe,
	.ops		= &amp_rt9119_ops,
	.priv_auto_alloc_size = sizeof(struct amp_rt9119_priv),
};
