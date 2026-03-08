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

#include "mtk_ad82089.h"

u8 ampinittblad82089[] = {
//DataLen	Address	DataN	DataN+1...
	/* don't need to do sw reset here, hw reset is applied */
	1,	0x00,	0x04,//##State_Control_1
	1,	0x01,	0x81,//##State_Control_2
	1,	0x02,	0x40,//##State_Control_3 ; default enable SW mute
	1,	0x03,	0x18,//##Master_volume_control ; default set to +0dB
	1,	0x04,	0x18,//##Channel_1_volume_control
	1,	0x05,	0x18,//##Channel_2_volume_control
	1,	0x06,	0x18,//##Channel_3_volume_control
	1,	0x07,	0x18,//##Channel_4_volume_control
	1,	0x08,	0x18,//##Channel_5_volume_control
	1,	0x09,	0x18,//##Channel_6_volume_control
	1,	0x0a,	0x10,//##Bass_Tone_Boost_and_Cut
	1,	0x0b,	0x10,//##treble_Tone_Boost_and_Cut
	1,	0x0c,	0x90,//##State_Control_4
	1,	0x0d,	0x00,//##Channel_1_configuration_registers
	1,	0x0e,	0x00,//##Channel_2_configuration_registers
	1,	0x0f,	0x00,//##Channel_3_configuration_registers
	1,	0x10,	0x00,//##Channel_4_configuration_registers
	1,	0x11,	0x00,//##Channel_5_configuration_registers
	1,	0x12,	0x00,//##Channel_6_configuration_registers
	1,	0x13,	0x00,//##Channel_7_configuration_registers
	1,	0x14,	0x00,//##Channel_8_configuration_registers
	1,	0x15,	0x6a,//##DRC1_limiter_attack/release_rate
	1,	0x16,	0x6a,//##DRC2_limiter_attack/release_rate
	1,	0x17,	0x6a,//##DRC3_limiter_attack/release_rate
	1,	0x18,	0x6a,//##DRC4_limiter_attack/release_rate
	1,	0x19,	0x06,//##Error_Delay
	1,	0x1a,	0x32,//##State_Control_5
	1,	0x1b,	0x01,//##HVUV_selection
	1,	0x1c,	0x00,//##State_Control_6
	1,	0x30,	0x00,//##Power_Stage_Status(Read_only)
	1,	0x31,	0x00,//##PWM_Output_Control
	1,	0x32,	0x00,//##Test_Mode_Control_Reg.
	1,	0x33,	0x6d,//##Qua-Ternary/Ternary_Switch_Level
	1,	0x37,	0x52,//##Device_ID_register
	0x00,
};

u8 ampswresettblad82089[] = {
	1,	0x1a,	0x12,
	0x00,
};

u8 ampnormaltblad82089[] = {
	1,	0x1a,	0x32,
	0x00,
};

u8 ampmutetblad82089[] = {
	1,	0x02,	0x7f,
	0x00,
};

u8 ampunmutetblad82089[] = {
	1,	0x02,	0x00,
	0x00,
};

static int mtk_amp_i2c_write(struct udevice *dev, u8 reg, u8 *buf, u8 size)
{
	struct amp_ad82089_priv *priv = dev_get_priv(dev);
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
			dev_err(dev, "[AMP 82089]i2c write fail %d\n", ret);
			return -1;
		}

		pu8str = pu8str + size;
	} while (*pu8str != 0);

	return 0;
}

static int amp_ad82089_powerdown(struct udevice *dev, bool enable)
{
	struct amp_ad82089_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->amp_powerdown))
		dm_gpio_set_value(&priv->amp_powerdown, enable);

	return 0;
}

static int amp_ad82089_set_params(struct udevice *dev, int interface, int rate, int mclk_freq,
				  int bits_per_sample, uint channels)
{
	int ret;

	//wait t5 min time = 10ms
	mdelay(10);
	//pull high amp PD pin
	amp_ad82089_powerdown(dev, 0);
	//wait t9 min time = 20ms then we can start I2C command
	mdelay(20);
	//software reset amp, if no have hardware reset pin(24pin's ad82088)
	amp_write(dev, ampswresettblad82089);
	mdelay(5);
	//normal operation
	amp_write(dev, ampnormaltblad82089);
	mdelay(20);
	//set all channel mute during writing all of registers and RAM
	amp_write(dev, ampmutetblad82089);

	ret = amp_write(dev, ampinittblad82089);
	if (ret < 0) {
		dev_err(dev, "[AMP 82089]Unable to init AMP AD82089\n");
		return -EINVAL;
	}

	mdelay(2);
	//set all channel un-mute
	amp_write(dev, ampunmutetblad82089);

	return 0;
}

static int amp_ad82089_probe(struct udevice *dev)
{
	struct amp_ad82089_priv *priv = dev_get_priv(dev);
	int ret;

	if (dev_read_enabled(dev) == 0)
		return 0;

	if (device_get_uclass_id(dev->parent) != UCLASS_I2C) {
		dev_err(dev, "[AMP 82089]get i2c parent fail\n");
		return -EPROTONOSUPPORT;
	}

	ret = dev_read_u32(dev, "reg", &priv->i2c_addr);
	if (ret) {
		dev_err(dev, "[AMP 82089]can't get slave addr\n");
		return -EINVAL;
	}

	ret = gpio_request_by_name(dev, "amppowerdown-gpios", 0, &priv->amp_powerdown, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "[AMP 82089]get amp power down gpio fail %d\n", ret);
		return -EINVAL;
	}

	return 0;
}

static const struct audio_codec_ops amp_ad82089_ops = {
	.set_params	= amp_ad82089_set_params,
};

static const struct udevice_id amp_ad82089_ids[] = {
	{ .compatible = "ESMT, ad82089" },
	{ }
};

U_BOOT_DRIVER(ad82089) = {
	.name		= "ad82089",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= amp_ad82089_ids,
	.probe		= amp_ad82089_probe,
	.ops		= &amp_ad82089_ops,
	.priv_auto_alloc_size = sizeof(struct amp_ad82089_priv),
};
