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

#include "ad82088.h"

u8 AmpInitTblAD82088[] = {
//DataLen	Address	DataN	DataN+1...
	1, 0x00, 0x04,//##State_Control_1
    1, 0x01, 0x81,//##State_Control_2
    1, 0x02, 0x40,//##State_Control_3 ; default enable SW mute
    1, 0x03, 0x04,//##Master_volume_control ; default set to +10dB
    1, 0x04, 0x18,//##Channel_1_volume_control
    1, 0x05, 0x18,//##Channel_2_volume_control
    1, 0x06, 0x18,//##Channel_3_volume_control
    1, 0x07, 0x18,//##Channel_4_volume_control
    1, 0x08, 0x18,//##Channel_5_volume_control
    1, 0x09, 0x18,//##Channel_6_volume_control
    1, 0x0a, 0x10,//##Bass_Tone_Boost_and_Cut
    1, 0x0b, 0x10,//##treble_Tone_Boost_and_Cut
    1, 0x0c, 0x90,//##State_Control_4
    1, 0x0d, 0x00,//##Channel_1_configuration_registers
    1, 0x0e, 0x00,//##Channel_2_configuration_registers
    1, 0x0f, 0x00,//##Channel_3_configuration_registers
    1, 0x10, 0x00,//##Channel_4_configuration_registers
    1, 0x11, 0x00,//##Channel_5_configuration_registers
    1, 0x12, 0x00,//##Channel_6_configuration_registers
    1, 0x13, 0x00,//##Channel_7_configuration_registers
    1, 0x14, 0x00,//##Channel_8_configuration_registers
    1, 0x15, 0x6a,//##DRC1_limiter_attack/release_rate
    1, 0x16, 0x6a,//##DRC2_limiter_attack/release_rate
    1, 0x17, 0x6a,//##DRC3_limiter_attack/release_rate
    1, 0x18, 0x6a,//##DRC4_limiter_attack/release_rate
    1, 0x19, 0x06,//##Error_Delay
    1, 0x1a, 0x32,//##State_Control_5
    1, 0x1b, 0x01,//##HVUV_selection
    1, 0x1c, 0x00,//##State_Control_6
    1, 0x30, 0x00,//##Power_Stage_Status(Read_only)
    1, 0x31, 0x00,//##PWM_Output_Control
    1, 0x32, 0x00,//##Test_Mode_Control_Reg.
    1, 0x33, 0x6d,//##Qua-Ternary/Ternary_Switch_Level
    1, 0x37, 0x52,//##Device_ID_register
    0x00,
};

static int mtk_amp_i2c_write(struct udevice *dev, u8 reg, u8 *buf, u8 size)
{
	struct amp_ad82088_priv *priv = dev_get_priv(dev);
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
			dev_err(dev, "[AMP 82088]i2c write fail %d\n", ret);
			return -1;
		}

		pu8Str = pu8Str + size;
	} while (*pu8Str != 0);

	return 0;
}

static int amp_ad82088_reset(struct udevice *dev, bool bEnable)
{
	struct amp_ad82088_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_reset))
		dm_gpio_set_value(&priv->amp_reset, bEnable);

	return 0;
}

static int amp_ad82088_powerdown(struct udevice *dev, bool bEnable)
{
	struct amp_ad82088_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_powerdown))
		dm_gpio_set_value(&priv->amp_powerdown, bEnable);

	return 0;
}

static int amp_ad82088_set_params(struct udevice *dev, int interface, int rate,
			       int mclk_freq, int bits_per_sample,
			       uint channels)
{
	int ret;

	amp_ad82088_reset(dev, 1);
	amp_ad82088_powerdown(dev, 1);
	mdelay(10);
	amp_ad82088_reset(dev, 0);
	amp_ad82088_powerdown(dev, 0);
	mdelay(20);

	ret = amp_write(dev, AmpInitTblAD82088);
	if (ret < 0) {
		dev_err(dev, "[AMP 82088]Unable to init AMP AD82088\n");
		return -EINVAL;
	}

	return 0;
}

static int amp_ad82088_probe(struct udevice *dev)
{
	struct amp_ad82088_priv *priv = dev_get_priv(dev);
	int ret;

	if (dev_read_enabled(dev) == 0)
		return 0;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C) {
		dev_err(dev, "[AMP 82088]get i2c parent fail\n");
        	return -EPROTONOSUPPORT;
	}

	ret = dev_read_u32(dev, "reg", &priv->i2c_addr);
	if (ret) {
		dev_err(dev, "[AMP 82088]can't get slave addr\n");
		return -EINVAL;
	}
	
	ret = gpio_request_by_name(dev, "ampreset-gpios", 0, &priv->amp_reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 82088]get amp reset gpio fail %d\n", ret);
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "amppowerdown-gpios", 0, &priv->amp_powerdown, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 82088]get amp power down gpio fail %d\n", ret);
		return -EINVAL;
	}

	return 0;
}

static const struct audio_codec_ops amp_ad82088_ops = {
	.set_params	= amp_ad82088_set_params,
};

static const struct udevice_id amp_ad82088_ids[] = {
	{ .compatible = "esmt, ad82088" },
	{ }
};

U_BOOT_DRIVER(ad82088) = {
	.name		= "ad82088",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= amp_ad82088_ids,
	.probe		= amp_ad82088_probe,
	.ops		= &amp_ad82088_ops,
	.priv_auto_alloc_size = sizeof(struct amp_ad82088_priv),
};
