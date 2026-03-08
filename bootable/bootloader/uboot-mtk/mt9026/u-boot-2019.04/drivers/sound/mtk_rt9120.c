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

#include "mtk_rt9120.h"

u8 AmpInitTblRT9120[] = {
//DataLen	Address	DataN	DataN+1...
	2,	0x20,	0x01,	0x80,		//volume to 0db
        1,	0x05,	0x00,
        1,	0x04,	0x20,			//SDO output
        0x00,
};

u8 AmpSWResetTbl[] = {
	1,	0x04,	0x80,
        0x00,
};

static int mtk_amp_i2c_write(struct udevice *dev,
			u8 reg, u8 *buf, u8 size)
{
	struct amp_rt9120_priv *priv = dev_get_priv(dev);
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

static int amp_write(struct udevice *dev, unsigned char *InitTable)
{
	u8 *pu8Str = InitTable;
	u8 *buf;
	u8 reg;
	u8 size;
	int ret;

	do {
		size = *pu8Str;
		pu8Str++;

		reg = *pu8Str;
		pu8Str++;

		buf = pu8Str;
		ret = mtk_amp_i2c_write(dev, reg, buf, size);
		if (ret < 0) {
			dev_err(dev, "[AMP 9120]i2c write fail %d\n", ret);
			return -1;
		}

		pu8Str = pu8Str + size;
	} while (*pu8Str != 0);

	return 0;
}

static int amp_rt9120_reset(struct udevice *dev, bool bEnable)
{
	/*struct amp_rt9120_priv *priv = dev_get_priv(dev);*/
	int ret = 0;

	if (bEnable) {
		ret = amp_write(dev, AmpSWResetTbl);
		if (ret < 0) {
			dev_err(dev, "[AMP 9120]Unable to reset AMP RT9120\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int amp_rt9120_powerdown(struct udevice *dev, bool bEnable)
{
	struct amp_rt9120_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_powerdown))
		dm_gpio_set_value(&priv->amp_powerdown, bEnable);

	return 0;
}

static int amp_rt9120_set_params(struct udevice *dev, int interface, int rate,
			       int mclk_freq, int bits_per_sample,
			       uint channels)
{
	int ret;

	amp_rt9120_powerdown(dev, 0);
	mdelay(12);
	amp_rt9120_reset(dev, 1);
	mdelay(11);

	ret = amp_write(dev, AmpInitTblRT9120);
	if (ret < 0) {
		dev_err(dev, "[AMP 9120]Unable to init AMP RT9120\n");
		return -EINVAL;
	}

	return 0;
}

static int amp_rt9120_probe(struct udevice *dev)
{
	struct amp_rt9120_priv *priv = dev_get_priv(dev);
	int ret;

	if (dev_read_enabled(dev) == 0)
		return 0;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C) {
		dev_err(dev, "[AMP 9120]get i2c parent fail\n");
        	return -EPROTONOSUPPORT;
        }

	ret = dev_read_u32(dev, "reg", &priv->i2c_addr);
	if (ret) {
		dev_err(dev, "[AMP 9120]can't get slave addr\n");
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "amppowerdown-gpios", 0, &priv->amp_powerdown, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 9120]get amp power down gpio fail %d\n", ret);
		return -EINVAL;
	}
	
	return 0;
}

static const struct audio_codec_ops amp_rt9120_ops = {
	.set_params	= amp_rt9120_set_params,
};

static const struct udevice_id amp_rt9120_ids[] = {
	{ .compatible = "mediatek, rt9120" },
	{ }
};

U_BOOT_DRIVER(rt9120) = {
	.name		= "rt9120",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= amp_rt9120_ids,
	.probe		= amp_rt9120_probe,
	.ops		= &amp_rt9120_ops,
	.priv_auto_alloc_size = sizeof(struct amp_rt9120_priv),
};
