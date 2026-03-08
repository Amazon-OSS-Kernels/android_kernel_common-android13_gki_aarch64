// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>

#include "mtk_dtv_pwm_pm.h"

struct mtk_pwm_pm_dat {
	unsigned int div;
	bool dben;
};

/**
 * struct mediatek_pwm_pm_priv - mtk_dtv_pwm_pm private data
 *
 * @reg_base: register bank base address
 * @pad_mode: PWM pad mode for each channel
 * @clkspeed: IP source clock rate
 * @channel_cnt: PWM channel count
 * @data: pwm proprietary control data
 */
struct mediatek_pwm_pm_priv {
	void __iomem *reg_base;
	u32 pad_mode[2];
	u32 clkspeed;
	int channel_cnt;
	struct mtk_pwm_pm_dat data;
};

static int _mediatek_pwm_pm_set_divider(struct udevice *dev, uint channel,
					uint div)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;

	if (unlikely((div >> 8) != 0)) {
		dev_err(dev, "divider value too large to register setting\n");
		return -EINVAL;
	}

	addr = priv->reg_base + ((REG_PWM0_PM_DIV + (channel * 0x10)) << 2);

	debug("set pwm divider, register %p value 0x%02X\n", addr,
	      div & 0x00FF);
	writeb(div & 0x00FF, addr);
	priv->data.div = div;

	return 0;
}

static int _mediatek_pwm_pm_set_dben(struct udevice *dev, uint channel,
				     bool dben)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr = priv->reg_base + ((REG_PWM0_PM_DBEN + (channel * 0x10)) << 2);

	val = readb(addr);
	debug
	    ("set pwm pm double buffer %sable, register %p original value 0x%02X\n",
	     (dben) ? "en" : "dis", addr, val);
	if (dben)
		val |= REG_PWM_PM_DBEN_MASK;
	else
		val &= ~REG_PWM_PM_DBEN_MASK;
	debug("set pwm pm double buffer %sable, register %p value 0x%02X\n",
	      (dben) ? "en" : "dis", addr, val);
	writeb(val, addr);
	priv->data.dben = dben;

	return 0;
}

static int _mediatek_pwm_pm_set_dutycycle(struct udevice *dev, uint channel,
					  uint duty_ns)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
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

	addr = priv->reg_base + ((REG_PWM0_PM_DUTY + (channel * 0x10)) << 2);
	debug("set pwm duty, register %p value 0x%04llX\n", addr,
	      duty_set & 0xFFFF);
	writew(duty_set & 0xFFFF, addr);

	return 0;
}

static int _mediatek_pwm_pm_set_period(struct udevice *dev, uint channel,
				       uint period_ns)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
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

	addr = priv->reg_base + ((REG_PWM0_PM_PERIOD + (channel * 0x10)) << 2);
	debug("set pwm period, register %p value 0x%04llX\n", addr,
	      period_set & 0xFFFF);
	writew(period_set & 0xFFFF, addr);

	return 0;
}

static int _mediatek_pwm_pm_set_polarity(struct udevice *dev, uint channel,
					 bool inversed)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr =
	    priv->reg_base + ((REG_PWM0_PM_POLARITY + (channel * 0x10)) << 2);

	val = readb(addr);
	debug("set pwm polarity register %p original value 0x%04X\n", addr,
	      val);
	if (inversed)
		val |= REG_PWM_PM_POLARITY_MASK;
	else
		val &= ~REG_PWM_PM_POLARITY_MASK;
	debug("set pwm polarity register %p value 0x%04X\n", addr, val);
	writeb(val, addr);

	return 0;
}

static int mediatek_pwm_pm_set_config(struct udevice *dev, uint channel,
				      uint period_ns, uint duty_ns)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s, reg %p, channel %d, period %d, duty %d\n", __func__,
	      priv->reg_base, channel, period_ns, duty_ns);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	ret = _mediatek_pwm_pm_set_dutycycle(dev, channel, duty_ns);
	if (ret)
		return ret;

	return _mediatek_pwm_pm_set_period(dev, channel, period_ns);
}

static int mediatek_pwm_pm_set_polarity(struct udevice *dev, uint channel,
					bool polarity)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p, channel %d, polarity %s\n", __func__, priv->reg_base,
	      channel, (polarity) ? "inversed" : "normal");

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	return _mediatek_pwm_pm_set_polarity(dev, channel, polarity);
}

static int mediatek_pwm_pm_set_enable(struct udevice *dev, uint channel,
				      bool enable)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	int val;

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->reg_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}
	// PWM channel sw rst
	if (channel == 0) {
		addr = priv->reg_base + (REG_PWM0_PM_SW_RST << 2);
		if (enable)
			val = readw(addr) & ~REG_PWM0_PM_SW_RST_MASK;
		else
			val = readw(addr) | REG_PWM0_PM_SW_RST_MASK;
		writew(val, addr);
	} else {
		addr = priv->reg_base + (REG_PWM1_PM_SW_RST << 2);
		if (enable)
			val = readw(addr) & ~REG_PWM1_PM_SW_RST_MASK;
		else
			val = readw(addr) | REG_PWM1_PM_SW_RST_MASK;
		writew(val, addr);
	}

	return 0;
}

static int mediatek_pwm_pm_probe(struct udevice *dev)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	int val;
	int i;

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// enable PWM clock
	debug("enable pwm IP source clock and gate all PWM channels clock\n");
	// nothing to do

	/* TODO : Select PWM IP source clock rate ? */
	priv->clkspeed = 24000000;

	// MDrv_PWM_Init(E_PWM_DBGLV_ERR_ONLY);
	// nothing to do

	/* set all pwm channels PAD mode */
	debug("set pwm pad modes\n");
	/* pwm pm channel 0 */
	val = readb(priv->reg_base + (REG_PM_PWM0_IS_GPIO << 2));
	if (priv->pad_mode[0])
		val &= ~REG_PM_PWM0_IS_GPIO_MASK;
	else
		val |= REG_PM_PWM0_IS_GPIO_MASK;
	writeb(val, priv->reg_base + (REG_PM_PWM0_IS_GPIO << 2));
	/* pwm pm channel 0 */
	val =
	    readb(priv->reg_base + (REG_GPIO_IS_PWM1 << 2) +
		  1) | ((priv->pad_mode[1] & 0x03) << 3);
	writeb(val, priv->reg_base + (REG_GPIO_IS_PWM1 << 2) + 1);

	/* tie all pwm pm rest (disabled) */
	debug("disable all pwm channels output\n");
	/* pwm pm channel 0 */
	val =
	    readw(priv->reg_base +
		  (REG_PWM0_PM_SW_RST << 2)) | REG_PWM0_PM_SW_RST_MASK;
	writew(val, priv->reg_base + (REG_PWM0_PM_SW_RST << 2));
	/* pwm pm channel 1 */
	val =
	    readw(priv->reg_base +
		  (REG_PWM1_PM_SW_RST << 2)) | REG_PWM1_PM_SW_RST_MASK;
	writew(val, priv->reg_base + (REG_PWM1_PM_SW_RST << 2));

	/* private control setting */
	for (i = 0; i < priv->channel_cnt; i++) {
		_mediatek_pwm_pm_set_divider(dev, i, priv->data.div);
		_mediatek_pwm_pm_set_dben(dev, i, priv->data.dben);
	}

	return 0;
}

static int mediatek_pwm_pm_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
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

	priv->data.div = dev_read_u32_default(dev, "div", 0);
	priv->data.dben = dev_read_bool(dev, "double-buffer");

	debug("%s, div %d\n", __func__, priv->data.div);
	debug("%s, double buffer %sable\n", __func__,
	      (priv->data.dben) ? "en" : "dis");

	return 0;
}

static const struct pwm_ops mediatek_pwm_pm_ops = {
	.set_config = mediatek_pwm_pm_set_config,
	.set_enable = mediatek_pwm_pm_set_enable,
	.set_invert = mediatek_pwm_pm_set_polarity,
};

static const struct udevice_id mediatek_pwm_pm_id[] = {
	{.compatible = "mediatek,mtk-dtv-pwm-pm"},
	{}
};

U_BOOT_DRIVER(mediatek_pwm_pm) = {
	.name = "mtk_dtv_pwm_pm",
	.id = UCLASS_PWM,
	.of_match = mediatek_pwm_pm_id,
	.ops = &mediatek_pwm_pm_ops,
	.probe = mediatek_pwm_pm_probe,
	.ofdata_to_platdata = mediatek_pwm_pm_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_pwm_pm_priv),
};
