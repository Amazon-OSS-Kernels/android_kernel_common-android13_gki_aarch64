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

#include "mtk_tas5805.h"

u8 ampinittbltas5805[] = {
//DataLen	Address	DataN	DataN+1...
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x03,	0x02,
	1,	0x01,	0x11,
	1,	0x00,	0x00,
	1,	0x00,	0x00,
	1,	0x00,	0x00,
	1,	0x00,	0x00,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x03,	0x02,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x03,	0x00,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x46,	0x11,
	1,	0x03,	0x02,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x78,	0x80,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x61,	0x0b,
	1,	0x60,	0x01,
	1,	0x7d,	0x11,
	1,	0x7e,	0xff,
	1,	0x00,	0x01,
	1,	0x51,	0x05,
	1,	0x00,	0x00,
	1,	0x02,	0x11,
	1,	0x53,	0x20,
	1,	0x54,	0x00,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x66,	0x84,
	1,	0x7f,	0x8c,
	1,	0x00,	0x2a,
	1,	0x24,	0x01,
	1,	0x25,	0x33,
	1,	0x26,	0x0c,
	1,	0x27,	0xf5,
	1,	0x28,	0x01,
	1,	0x29,	0x33,
	1,	0x2a,	0x0c,
	1,	0x2b,	0xf5,
	1,	0x30,	0x00,
	1,	0x31,	0xe2,
	1,	0x32,	0xc4,
	1,	0x33,	0x6b,
	1,	0x00,	0x2c,
	1,	0x5c,	0x00,
	1,	0x5d,	0x00,
	1,	0x5e,	0xae,
	1,	0x5f,	0xc3,
	1,	0x60,	0x01,
	1,	0x61,	0x9a,
	1,	0x62,	0xf7,
	1,	0x63,	0x20,
	1,	0x64,	0x08,
	1,	0x65,	0x13,
	1,	0x66,	0x85,
	1,	0x67,	0x62,
	1,	0x68,	0xc0,
	1,	0x69,	0x00,
	1,	0x6a,	0x00,
	1,	0x6b,	0x00,
	1,	0x6c,	0x04,
	1,	0x6d,	0xc1,
	1,	0x6e,	0xff,
	1,	0x6f,	0x93,
	1,	0x74,	0x00,
	1,	0x75,	0x80,
	1,	0x76,	0x00,
	1,	0x77,	0x00,
	1,	0x00,	0x2d,
	1,	0x18,	0x7b,
	1,	0x19,	0x3e,
	1,	0x1a,	0x00,
	1,	0x1b,	0x6d,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x30,	0x00,
	1,	0x4c,	0x30,
	1,	0x03,	0x03,
	1,	0x00,	0x00,
	1,	0x7f,	0x00,
	1,	0x78,	0x80,
	0x00,
};

static int mtk_amp_i2c_write(struct udevice *dev, u8 reg, u8 *buf, u8 size)
{
	struct amp_tas5805_priv *priv = dev_get_priv(dev);
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
			dev_err(dev, "[AMP 5805]i2c write fail %d\n", ret);
			return -1;
		}

		pu8str = pu8str + size;
	} while (*pu8str != 0);

	return 0;
}

static int amp_tas5805_powerdown(struct udevice *dev, bool enable)
{
	struct amp_tas5805_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_powerdown))
		dm_gpio_set_value(&priv->amp_powerdown, enable);

	return 0;
}

static int amp_tas5805_set_params(struct udevice *dev, int interface, int rate,
				  int mclk_freq, int bits_per_sample, uint channels)
{
	int ret;

	amp_tas5805_powerdown(dev, 1);
	mdelay(1);
	amp_tas5805_powerdown(dev, 0);
	mdelay(20);

	ret = amp_write(dev, ampinittbltas5805);
	if (ret < 0) {
		dev_err(dev, "[AMP 5805]Unable to init AMP TAS5805\n");
		return -EINVAL;
	}

	return 0;
}

static int amp_tas5805_probe(struct udevice *dev)
{
	struct amp_tas5805_priv *priv = dev_get_priv(dev);
	int ret;

	if (dev_read_enabled(dev) == 0)
		return 0;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C) {
		dev_err(dev, "[AMP 5805]get i2c parent fail\n");
		return -EPROTONOSUPPORT;
	}

	ret = dev_read_u32(dev, "reg", &priv->i2c_addr);
	if (ret) {
		dev_err(dev, "[AMP 5805]can't get slave addr\n");
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "amppowerdown-gpios", 0, &priv->amp_powerdown, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 5805]get amp power down gpio fail %d\n", ret);
		return -EINVAL;
	}

	return 0;
}

static const struct audio_codec_ops amp_tas5805_ops = {
	.set_params	= amp_tas5805_set_params,
};

static const struct udevice_id amp_tas5805_ids[] = {
	{ .compatible = "ti, tas5805" },
	{ }
};

U_BOOT_DRIVER(tas5805) = {
	.name		= "tas5805",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= amp_tas5805_ids,
	.probe		= amp_tas5805_probe,
	.ops		= &amp_tas5805_ops,
	.priv_auto_alloc_size = sizeof(struct amp_tas5805_priv),
};
