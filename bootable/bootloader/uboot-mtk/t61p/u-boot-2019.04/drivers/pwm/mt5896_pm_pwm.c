// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>

#include "mt5896_pm_pwm.h"
#define CHANNEL_OFFSET	(8)
#define REG_OFFSET		(2)
#define XTAL_CLK	12000000

struct mtk_pwm_pm_dat {
	unsigned int div;
	bool pdvalue;
	bool dben;
};

/**
 * struct mediatek_pwm_pm_priv - mtk_dtv_pwm_pm private data
 *
 * @reg_base: register bank base address
 * @clkspeed: IP source clock rate
 * @channel_cnt: PWM channel count
 * @data: pwm proprietary control data
 */
struct mediatek_pwm_pm_priv {
	void __iomem *reg_base;
	u32 clkspeed;
	int channel_cnt;
	struct mtk_pwm_pm_dat data[PWM_PM_MAX_CHANNEL];
};

static int _mediatek_pwm_pm_set_divider(struct udevice *dev, uint channel,
					uint div)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;

	if (unlikely((div >> 16) != 0)) {
		dev_err(dev, "divider value too large to register setting\n");
		return -EINVAL;
	}

	addr = priv->reg_base + ((REG_PWM0_PM_DIV + (channel * CHANNEL_OFFSET)) << REG_OFFSET);

	debug("set pm pwm divider, reg %p value 0x%04X\n", addr,
	      div & 0xFFFF);
	writew(div & 0xFFFF, addr);
	priv->data[channel].div = div;

	return 0;
}

static int _mediatek_pwm_pm_set_dben(struct udevice *dev, uint channel,
				     bool dben)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + ((REG_PWM0_PM_DBEN + (channel * CHANNEL_OFFSET)) << REG_OFFSET);

	val = readw(addr);
	debug
		("set pm pwm double buffer %sable, reg %p ori value 0x%04X\n",
	     (dben) ? "en" : "dis", addr, val);
	if (dben)
		val |= REG_PWM_PM_DBEN_MASK;
	else
		val &= ~REG_PWM_PM_DBEN_MASK;
	debug("set pm pwm double buffer %sable, reg %p value 0x%04X\n",
	      (dben) ? "en" : "dis", addr, val);
	writew(val, addr);
	priv->data[channel].dben = dben;

	return 0;
}

static int _mediatek_pwm_pm_set_pdvalue(struct udevice *dev, uint channel,
					bool pdvalue)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + ((REG_PWM0_PM_PD_VALUE + (channel * CHANNEL_OFFSET)) << REG_OFFSET);

	val = readw(addr);
	debug("set pm pwm power down value %d, reg %p ori value 0x%04X\n",
	      pdvalue, addr, val);
	if (pdvalue)
		val |= REG_PWM_PM_PD_VALUE_MASK;
	else
		val &= ~REG_PWM_PM_PD_VALUE_MASK;
	debug("set pm pwm power down value %d, reg %p value 0x%04X\n", pdvalue,
	      addr, val);
	writew(val, addr);
	priv->data[channel].pdvalue = pdvalue;

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
	clkspeed /= (priv->data[channel].div + 1);

	duty_set = (u64)clkspeed * duty_ns;
	duty_set = lldiv(duty_set, 1000000000);

	if (unlikely((duty_set >> 18) != 0)) {
		dev_err(dev, "duty time too long to register setting\n");
		return -EINVAL;
	}

	debug("duty %d with clock %d div %d, reg value %llu (0x%llX)\n",
	      duty_ns, priv->clkspeed, priv->data[channel].div, duty_set,
	      duty_set);

	addr = priv->reg_base + ((REG_PWM0_PM_DUTY_L + (channel * CHANNEL_OFFSET)) << REG_OFFSET);
	debug("set pm pwm duty, reg %p\n", addr);
	writew(duty_set & 0xFFFF, addr);
	writeb((duty_set >> 16) & 0x03, addr + 4);

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
	clkspeed /= (priv->data[channel].div + 1);

	period_set = (u64)clkspeed * period_ns;
	period_set = lldiv(period_set, 1000000000);

	if (unlikely((period_set >> 18) != 0)) {
		dev_err(dev, "period time too long to register setting\n");
		return -EINVAL;
	}

	debug("period %d with clock %d div %d, reg value %llu (0x%llX)\n",
	      period_ns, priv->clkspeed, priv->data[channel].div, period_set,
	      period_set);

	addr =
	    priv->reg_base + ((REG_PWM0_PM_PERIOD_L + (channel * CHANNEL_OFFSET)) << REG_OFFSET);
	debug("set pm pwm period, reg %p\n", addr);
	writew(period_set & 0xFFFF, addr);
	writeb((period_set >> 16) & 0x03, addr + 4);

	return 0;
}

static int _mediatek_pwm_pm_set_polarity(struct udevice *dev, uint channel,
					 bool inversed)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + ((REG_PWM0_PM_POLARITY + (channel * CHANNEL_OFFSET)) << REG_OFFSET);

	val = readw(addr);
	debug("set pm pwm polarity reg %p ori value 0x%04X\n", addr, val);
	if (inversed)
		val |= REG_PWM_PM_POLARITY_MASK;
	else
		val &= ~REG_PWM_PM_POLARITY_MASK;
	debug("set pm pwm polarity reg %p value 0x%04X\n", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_pm_set_enable(struct udevice *dev, uint channel,
				       bool enable)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + ((REG_PWM0_PM_EN + (channel * CHANNEL_OFFSET)) << REG_OFFSET);

	val = readw(addr);
	debug("set pm pwm enable reg %p ori value 0x%04X\n", addr,
	      val);
	if (enable)
		val &= ~REG_PWM_PM_EN_MASK;
	else
		val |= REG_PWM_PM_EN_MASK;
	debug("set pm pwm enable reg %p value 0x%04X\n", addr, val);
	writew(val, addr);

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

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->reg_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	return _mediatek_pwm_pm_set_enable(dev, channel, enable);
}

static int mediatek_pwm_pm_probe(struct udevice *dev)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	int i;

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// enable PWM clock
	debug("enable pwm IP source clock and gate all PWM channels clock\n");
	// nothing to do

	/* TODO : Select PWM IP source clock rate ? */
	priv->clkspeed = XTAL_CLK;

	/* TODO: pad mode setting, use pinctrl ? */
	debug("set pwm pad modes\n");

	/* PWM channels initialize */
	for (i = 0; i < priv->channel_cnt; i++) {
		/* pm pwm rest (disabled) */
		_mediatek_pwm_pm_set_enable(dev, i, false);
		/* private control setting */
		_mediatek_pwm_pm_set_divider(dev, i, priv->data[i].div);
		_mediatek_pwm_pm_set_dben(dev, i, priv->data[i].dben);
		_mediatek_pwm_pm_set_pdvalue(dev, i, priv->data[i].pdvalue);
	}

	return 0;
}

static int mediatek_pwm_pm_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_pwm_pm_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;
	u32 dts_properity_array[PWM_PM_MAX_CHANNEL] = { 0 };

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	/* num of channels, pad mode for each channel */

	priv->channel_cnt = PWM_PM_MAX_CHANNEL;
	debug("%s, PWM %d channels\n", __func__, priv->channel_cnt);

	/* divider for each channel */
	memset(dts_properity_array, 0, sizeof(dts_properity_array));
	ret =
	    dev_read_u32_array(dev, "div", dts_properity_array,
			       priv->channel_cnt);
	if (ret)
		dev_err(dev,
			"Read property 'div' from DTS fail, default to 0 (%d)\n",
			ret);

	for (i = 0; i < priv->channel_cnt; i++)
		priv->data[i].div = dts_properity_array[i];

	/* enable/disable double buffer for each channel */
	memset(dts_properity_array, 0, sizeof(dts_properity_array));
	ret =
	    dev_read_u32_array(dev, "double-buffer", dts_properity_array,
			       priv->channel_cnt);
	if (ret)
		dev_err(dev,
			"Read property 'double-buffer' from DTS fail, default to disable (%d)\n",
			ret);

	for (i = 0; i < priv->channel_cnt; i++)
		priv->data[i].dben = (dts_properity_array[i]) ? true : false;

	/* powen down value for each channel */
	memset(dts_properity_array, 0, sizeof(dts_properity_array));
	ret =
	    dev_read_u32_array(dev, "pd-value", dts_properity_array,
			       priv->channel_cnt);
	if (ret)
		dev_err(dev,
			"Read property 'pd-valuer' from DTS fail, default to 0 (%d)\n",
			ret);

	for (i = 0; i < priv->channel_cnt; i++)
		priv->data[i].pdvalue = dts_properity_array[i];

	for (i = 0; i < priv->channel_cnt; i++) {
		debug("%s, PM PWM channel %d:\n", __func__, i);
		debug("div %d\n", priv->data[i].div);
		debug("double buffer %sable\n",
		      (priv->data[i].dben) ? "en" : "dis");
		debug("power down value %d\n", priv->data[i].pdvalue);
	}

	return 0;
}

static const struct pwm_ops mediatek_pwm_pm_ops = {
	.set_config = mediatek_pwm_pm_set_config,
	.set_enable = mediatek_pwm_pm_set_enable,
	.set_invert = mediatek_pwm_pm_set_polarity,
};

static const struct udevice_id mediatek_pwm_pm_id[] = {
	{.compatible = "mediatek,mt5896-pm-pwm"},
	{}
};

U_BOOT_DRIVER(mt5896_pm_pwm) = {
	.name = "mediatek_pwm",
	.id = UCLASS_PWM,
	.of_match = mediatek_pwm_pm_id,
	.ops = &mediatek_pwm_pm_ops,
	.probe = mediatek_pwm_pm_probe,
	.ofdata_to_platdata = mediatek_pwm_pm_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_pwm_pm_priv),
};
