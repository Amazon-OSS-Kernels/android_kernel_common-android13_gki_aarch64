// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>

#include "mtk_dtv_pwm_dac.h"

struct mtk_pwm_dac_dat {
	s32 duty_offset;
	unsigned int div;
	bool duty_auto_correct;
	bool dben;
};

/**
 * struct mediatek_pwm_dac_priv - mtk_dtv_pwm_dac private data
 *
 * @reg_base: register bank base address
 * @pad_mode: PWM pad mode for each channel
 * @clkspeed: IP source clock rate
 * @channel_cnt: PWM channel count
 * @data: pwm dac proprietary control data
 */
struct mediatek_pwm_dac_priv {
	void __iomem *reg_base;
	u32 pad_mode[2];
	u32 clkspeed;
	int channel_cnt;
	struct mtk_pwm_dac_dat data;
};

static int _mediatek_pwm_dac_set_offset(struct udevice *dev, uint channel,
					s32 offset)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	if (unlikely(abs(offset) > 127)) {
		dev_err(dev, "offset value too large to register setting\n");
		return -EINVAL;
	}

	val = abs(offset);
	if (offset < 0) {
		val |= 0x80;
	}

	addr =
	    priv->reg_base +
	    ((REG_PWM0_DAC_DUTY_OFFSET + (channel * 0x10)) << 2);
	debug("set pwm dac duty offset, register %p value 0x%02X\n", addr, val);
	writeb(val, addr);
	priv->data.duty_offset = offset;

	return 0;
}

static int _mediatek_pwm_dac_set_divider(struct udevice *dev, uint channel,
					 uint div)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	void __iomem *addr;

	if (unlikely((div >> 8) != 0)) {
		dev_err(dev, "divider value too large to register setting\n");
		return -EINVAL;
	}

	addr = priv->reg_base + ((REG_PWM0_DAC_DIV + (channel * 0x10)) << 2);
	debug("set pwm dac divider, register %p value 0x%02X\n", addr,
	      div & 0x00FF);
	writeb(div & 0x00FF, addr);
	priv->data.div = div;

	return 0;
}

static int _mediatek_pwm_dac_set_dben(struct udevice *dev, uint channel,
				      bool dben)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr =
	    priv->reg_base + ((REG_PWM0_DAC_DBEN + (channel * 0x10)) << 2) + 1;
	val = readb(addr);
	debug
	    ("set pwm dac double buffer %sable, register %p original value 0x%02X\n",
	     (dben) ? "en" : "dis", addr, val);
	if (dben)
		val |= (REG_PWM_DAC_DBEN_MASK >> 8);
	else
		val &= ~(REG_PWM_DAC_DBEN_MASK >> 8);
	debug("set pwm dac double buffer %sable, register %p value 0x%02X\n",
	      (dben) ? "en" : "dis", addr, val);
	writeb(val, addr);
	priv->data.dben = dben;

	return 0;
}

static int _mediatek_pwm_dac_set_duty_auto_correct(struct udevice *dev,
						   uint channel,
						   bool auto_correct)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr =
	    priv->reg_base +
	    ((REG_PWM0_DAC_DUTY_AUTO_CORRECT + (channel * 0x10)) << 2);
	val = readb(addr);
	debug
	    ("set pwm dac duty auto correct %sable, register %p original value 0x%02X\n",
	     (auto_correct) ? "en" : "dis", addr, val);
	if (auto_correct)
		val |= REG_PWM_DAC_DUTY_AUTO_CORRECT_MASK;
	else
		val &= ~REG_PWM_DAC_DUTY_AUTO_CORRECT_MASK;
	debug
	    ("set pwm dac duty auto correct %sable, register %p value 0x%02X\n",
	     (auto_correct) ? "en" : "dis", addr, val);
	writeb(val, addr);
	priv->data.duty_auto_correct = auto_correct;

	return 0;
}

static int _mediatek_pwm_dac_set_dutycycle(struct udevice *dev, uint channel,
					   uint duty_ns)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	u32 clkspeed = priv->clkspeed;
	u64 duty_set;
	void __iomem *addr;

	/* calculate real PWM clock rate by divider */
	// real divider value is priv->data.div + 1
	clkspeed /= (priv->data.div + 1);

	duty_set = (u64)clkspeed * duty_ns;
	duty_set = lldiv(duty_set, 1000000000);

	if (unlikely((duty_set >> 16) != 0)) {
		dev_err(dev,
			"Due to hardware limitation, duty time too long to register setting\n");
		return -EINVAL;
	}

	debug
	    ("duty %d with clock %d divider %d, register value %llu (0x%llX)\n",
	     duty_ns, priv->clkspeed, priv->data.div, duty_set, duty_set);
	addr = priv->reg_base + ((REG_PWM0_DAC_DUTY + (channel * 0x10)) << 2);
	debug("set pwm duty, register %p value 0x%04llX\n", addr,
	      duty_set & 0xFFFF);
	writew(duty_set & 0xFFFF, addr);

	return 0;
}

static int _mediatek_pwm_dac_set_period(struct udevice *dev, uint channel,
					uint period_ns)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	u32 clkspeed = priv->clkspeed;
	u64 period_set;
	void __iomem *addr;

	/* calculate real PWM clock rate by divider */
	// real divider value is priv->data.div + 1
	clkspeed /= (priv->data.div + 1);

	period_set = (u64)clkspeed * period_ns;
	period_set = lldiv(period_set, 1000000000);

	if (unlikely((period_set >> 16) != 0)) {
		dev_err(dev,
			"Due to hardware limitation, period time too long to register setting\n");
		return -EINVAL;
	}

	debug
	    ("period %d with clock %d divider %d, register value %llu (0x%llX)\n",
	     period_ns, priv->clkspeed, priv->data.div, period_set, period_set);
	addr = priv->reg_base + ((REG_PWM0_DAC_PERIOD + (channel * 0x10)) << 2);
	debug("set pwm period, register %p value 0x%04llX\n", addr,
	      period_set & 0xFFFF);
	writew(period_set & 0xFFFF, addr);

	return 0;
}

static int _mediatek_pwm_dac_set_polarity(struct udevice *dev, uint channel,
					  bool inversed)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr =
	    priv->reg_base + ((REG_PWM0_DAC_POLARITY + (channel * 0x10)) << 2) +
	    1;
	val = readb(addr);
	debug("set pwm polarity register %p original value 0x%04X\n", addr,
	      val);
	if (inversed)
		val |= (REG_PWM_DAC_POLARITY_MASK >> 8);
	else
		val &= ~(REG_PWM_DAC_POLARITY_MASK >> 8);
	debug("set pwm polarity register %p value 0x%04X\n", addr, val);
	writeb(val, addr);

	return 0;
}

static int mediatek_pwm_dac_set_config(struct udevice *dev, uint channel,
				       uint period_ns, uint duty_ns)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s, reg %p, channel %d, period %d, duty %d\n", __func__,
	      priv->reg_base, channel, period_ns, duty_ns);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	ret = _mediatek_pwm_dac_set_dutycycle(dev, channel, duty_ns);
	if (ret)
		return ret;

	return _mediatek_pwm_dac_set_period(dev, channel, period_ns);
}

static int mediatek_pwm_dac_set_polarity(struct udevice *dev, uint channel,
					 bool polarity)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p, channel %d, polarity %s\n", __func__, priv->reg_base,
	      channel, (polarity) ? "inversed" : "normal");

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	return _mediatek_pwm_dac_set_polarity(dev, channel, polarity);
}

static int mediatek_pwm_dac_set_enable(struct udevice *dev, uint channel,
				       bool enable)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	int val;

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->reg_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}
	// PWM DAC output enable
	val = readb(CHIP_REG_PWM_DAC_OEN) & ~(1 << channel);
	if (!enable)
		val |= 1 << channel;
	writeb(val, CHIP_REG_PWM_DAC_OEN);

	return 0;
}

static int mediatek_pwm_dac_probe(struct udevice *dev)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	int val;
	int i;

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// enable PWM DAC clock
	debug("enable pwm dac IP source clock\n");
	writeb(0, priv->reg_base + (REG_CKG_PWM_DAC << 2));

	/* TODO : Select PWM DAC IP source clock rate ? */
	priv->clkspeed = 24000000;

	// MDrv_PWM_Init(E_PWM_DBGLV_ERR_ONLY);
	// nothing to do

	/* set all pwm channels PAD mode */
	debug("set pwm pad modes\n");
	val = readb(CHIP_REG_PWM_DAC0) & ~CHIP_PWM_DAC0_PAD_MSK;
	writeb(val | priv->pad_mode[0], CHIP_REG_PWM_DAC0);
	val = readb(CHIP_REG_PWM_DAC1) & ~CHIP_PWM_DAC1_PAD_MSK;
	writeb(val | (priv->pad_mode[1] << 4), CHIP_REG_PWM_DAC1);

	/* disable all pwm output */
	debug("disable all pwm channels output\n");
	writeb(readb(CHIP_REG_PWM_DAC_OEN) | CHIP_PWMS_DAC_OEN_MSK,
	       CHIP_REG_PWM_DAC_OEN);

	/* private control setting */
	for (i = 0; i < priv->channel_cnt; i++) {
		_mediatek_pwm_dac_set_offset(dev, i, priv->data.duty_offset);
		_mediatek_pwm_dac_set_divider(dev, i, priv->data.div);
		_mediatek_pwm_dac_set_dben(dev, i, priv->data.dben);
		_mediatek_pwm_dac_set_duty_auto_correct(dev, i,
							priv->data.duty_auto_correct);
	}

	return 0;
}

static int mediatek_pwm_dac_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_pwm_dac_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	/*num of channels, pad mode for each channel */
	priv->channel_cnt = 2;
	debug("%s, PWM %d channels\n", __func__, priv->channel_cnt);

	priv->pad_mode[0] = 0;
	priv->pad_mode[1] = 0;
	ret = dev_read_u32_array(dev, "pad-modes", priv->pad_mode, 2);
	if (ret)
		dev_err(dev, "Read 'pad-modes' from DTS fail (%d)", ret);

	for (i = 0; i < priv->channel_cnt; i++)
		debug("%s, PWM %d pad mode %d\n", __func__, i, priv->pad_mode[i]);

	priv->data.duty_offset = dev_read_s32_default(dev, "duty-offset", 0);
	priv->data.div = dev_read_u32_default(dev, "div", 0);
	priv->data.duty_auto_correct = dev_read_bool(dev, "duty-auto-correct");
	priv->data.dben = dev_read_bool(dev, "double-buffer");

	debug("%s, duty offset %d\n", __func__, priv->data.duty_offset);
	debug("%s, div %d\n", __func__, priv->data.div);
	debug("%s, duty auto correct %sable\n", __func__,
	      (priv->data.duty_auto_correct) ? "en" : "dis");
	debug("%s, double buffer %sable\n", __func__,
	      (priv->data.dben) ? "en" : "dis");

	return 0;
}

static const struct pwm_ops mediatek_pwm_dac_ops = {
	.set_config = mediatek_pwm_dac_set_config,
	.set_enable = mediatek_pwm_dac_set_enable,
	.set_invert = mediatek_pwm_dac_set_polarity,
};

static const struct udevice_id mediatek_pwm_dac_id[] = {
	{.compatible = "mediatek,mtk-dtv-pwm-dac"},
	{}
};

U_BOOT_DRIVER(mediatek_pwm_dac) = {
	.name = "mtk_dtv_pwm_dac",
	.id = UCLASS_PWM,
	.of_match = mediatek_pwm_dac_id,
	.ops = &mediatek_pwm_dac_ops,
	.probe = mediatek_pwm_dac_probe,
	.ofdata_to_platdata = mediatek_pwm_dac_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_pwm_dac_priv),
};
