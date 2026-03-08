// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>

#include "mt5896_pwm.h"

#include <debug_impl.h>
#define	PWM_MAX_CNT		8

struct mtk_pwm_dat {
	unsigned int shift;
	unsigned int div;
	unsigned int rst_mux;
	unsigned int rst_vsync;
	unsigned int rstcnt;
	unsigned int channel;
};

/**
 * struct mediatek_pwm_priv - mtk_dtv_pwm private data
 *
 * @reg_base: register bank base address
 * @clkspeed: IP source clock rate
 * @channel_cnt: PWM channel count
 * @data: pwm proprietary control data
 */
struct mediatek_pwm_priv {
	void __iomem *reg_base;
	void __iomem *clk_base;
	u32 clkspeed;
	int channel_cnt;
	struct mtk_pwm_dat data;
};

static int _mediatek_pwm_set_shift(struct udevice *dev, uint channel,
				   uint shift)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;

	if (unlikely((shift >> 24) != 0)) {
		dev_err(dev, "shift value too large to register setting\n");
		return -EINVAL;
	}
	/*Avoid 4T Change*/
	if (!shift)
		shift += 1;
	addr = priv->reg_base + REG_PWM0_SHIFT_L;
	/* shift low word */
	debug("pwm shift low, set reg %p value 0x%04X\n", addr, shift & 0xFFFF);
	writew(shift & 0xFFFF, addr);
	/* shift high byte */
	addr = priv->reg_base + REG_PWM0_SHIFT_H + channel*PWM_OFFSET_BASE;
	/* shift low word */
	debug("pwm shift high, set reg %p value 0x%04X\n", addr, shift & 0xFF);
	writew((shift >> 16) & 0xFF, addr);

	return 0;
}

static int _mediatek_pwm_set_divider(struct udevice *dev, uint channel,
				     uint div)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;

	if (unlikely((div >> 16) != 0)) {
		dev_err(dev, "divider value too large to register setting\n");
		return -EINVAL;
	}

	addr = priv->reg_base + REG_PWM0_DIV;
	/* low byte, direct write by byte access */
	debug("pwm divider, set reg %p value 0x%02X\n", addr, div & 0xFFFF);
	writew(div & 0xFFFF, addr);

	priv->data.div = div;

	return 0;
}

static int _mediatek_pwm_set_reset_en(struct udevice *dev, uint channel,
				      bool rst_vsync)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + REG_PWM0_RESET_EN;
	val = readw(addr);
	debug("pwm vsync reset %sable, reg %p original value 0x%02X\n",
	      (rst_vsync) ? "en" : "dis", addr, val);
	if (rst_vsync) {
		val |= REG_PWM_RESET_EN_MASK;
		val |= REG_PWM_VDBEN_MASK;
		val &= ~REG_PWM_DBEN_MASK;
	} else {
		val &= ~REG_PWM_RESET_EN_MASK;
		val &= ~REG_PWM_VDBEN_MASK;
		val |= REG_PWM_DBEN_MASK;
	}
	debug("pwm vsync reset %sable, set reg %p value 0x%02X\n",
	      (rst_vsync) ? "en" : "dis", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_force_low(struct udevice *dev, uint channel,
				      bool force_l)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + REG_PWM_CLKEN_BITMAP;
	val = readw(addr);
	if (force_l) {
		val |= 1 << 3;
		val &= ~(1 << 2);
	} else {
		val &= ~(1 << 3);
		val &= ~(1 << 2);
	}
	writew(val, addr);
	return 0;
}

static int _mediatek_pwm_set_rst_mux(struct udevice *dev, uint channel,
				     bool rst_mux)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + REG_RST_MUX0;
	val = readw(addr);
	debug("pwm reset mux %sable, reg %p original value 0x%02X\n",
	      (rst_mux) ? "en" : "dis", addr, val);
	/* byte access, always set/clear bit 7 */
	if (rst_mux)
		val |= REG_PWM_RST_MUX_EN_MASK_L;
	else
		val &= ~REG_PWM_RST_MUX_EN_MASK_L;
	debug("pwm reset mux %sable, set reg %p value 0x%02X\n",
	      (rst_mux) ? "en" : "dis", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_rst_cnt(struct udevice *dev, uint channel,
				     int rstcnt)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	if (unlikely(rstcnt > 0x0F)) {
		dev_err(dev, "rstcnt value too large to register setting\n");
		return -EINVAL;
	}

	addr =
	    priv->reg_base + REG_HS_RST_CNT0;
	/* byte access, always set/clear bit 3~0 */
	val = readw(addr);
	debug("pwm Hsync reset counter, reg %p read value 0x%02X\n", addr, val);
	val &= ~REG_PWM_RST_CNT_MASK_L;
	val |= rstcnt;
	debug("pwm Hsync reset counter, set reg %p value 0x%02X\n", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_dutycycle(struct udevice *dev, uint channel,
				       uint duty_ns)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	u32 clkspeed = priv->clkspeed;
	u64 duty_set;
	void __iomem *addr;
	u32 mask;
	int val;

	/* calculate real PWM clock rate by divider */
	// real divider value is priv->data.div + 1
	clkspeed /= (priv->data.div + 1);

	duty_set = (u64)clkspeed * duty_ns;
	duty_set = lldiv(duty_set, 1000000000);

	if (unlikely((duty_set >> 24) != 0)) {
		dev_err(dev, "Duty time too long to register setting\n");
		return -EINVAL;
	}

	debug("Duty %d by clk %d div %d, reg value %llu (0x%llX)\n",
	      duty_ns, priv->clkspeed, priv->data.div, duty_set,
	      duty_set);

	/* duty low word */
	addr = priv->reg_base + REG_PWM0_DUTY;
	debug("pwm duty, set reg %p value 0x%04llX\n", addr, duty_set & 0xFFFF);
	writew(duty_set & 0xFFFF, addr);
	addr = priv->reg_base + REG_PWM0_DUTY_EXT;
	/* duty 8-bit msb */
	duty_set >>= 16;
	mask = 0xFF;
	val = readw(addr);
	debug("pwm duty ext, reg %p original value 0x%04X\n", addr, val);
	val &= ~mask;
	val |= duty_set & mask;
	debug("pwm duty ext, set reg %p value 0x%04X\n", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_period(struct udevice *dev, uint channel,
				    uint period_ns)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	u32 clkspeed = priv->clkspeed;
	u64 period_set;
	void __iomem *addr;
	u32 mask;
	int val;

	/* calculate real PWM clock rate by divider */
	// real divider value is priv->data.div + 1
	clkspeed /= (priv->data.div + 1);

	period_set = (u64)clkspeed * period_ns;
	period_set = lldiv(period_set, 1000000000);

	if (unlikely((period_set >> 24) != 0)) {
		dev_err(dev, "Period time too long to register setting\n");
		return -EINVAL;
	}

	debug("Period %d by clk %d div %d, reg value %llu (0x%llX)\n",
	      period_ns, priv->clkspeed, priv->data.div, period_set,
	      period_set);

	/* period low word */
	addr = priv->reg_base + REG_PWM0_PERIOD;
	debug("pwm period, set reg %p value 0x%04llX\n", addr,
	      period_set & 0xFFFF);
	writew(period_set & 0xFFFF, addr);
	addr = priv->reg_base + REG_PWM0_PERIOD_EXT;
	/* period 8-bit msb */
	period_set >>= 16;
	mask = 0xFF ;
	val = readw(addr);
	debug("pwm period ext, reg %p original value 0x%04X\n", addr, val);
	val &= ~mask;
	val |= period_set & mask;
	debug("pwm period ext, set reg %p value 0x%04X\n", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_polarity(struct udevice *dev, uint channel,
				      bool inversed)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + REG_PWM0_PORARITY;
	val = readw(addr);
	debug("pwm polarity reg %p original value 0x%04X\n", addr, val);
	if (inversed)
		val |= REG_PWM_POLARITY_MASK;
	else
		val &= ~REG_PWM_POLARITY_MASK;
	debug("pwm polarity set reg %p value 0x%04X\n", addr, val);
	writew(val, addr);

	return 0;
}

static int _mediatek_pwm_set_enable(struct udevice *dev, uint channel,
				    bool enable)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	u16 val;

	/* enable/disable pwm by stop channel clock */
	val = readb(priv->reg_base + REG_PWM_CLKEN_BITMAP);
	debug("pwm enable ori value 0x%04X\n", val);
	if (enable)
		val |= PWM0_CLK_EN_MASK;
	else
		val &= ~PWM0_CLK_EN_MASK;
	debug("set pwm enable value 0x%04X\n", val);
	val &= ~(1 << 3);
	writew(val, priv->reg_base + REG_PWM_CLKEN_BITMAP);

	/* Avoid pwm glitch after reset by vsync with div setting */
	val = readw(priv->reg_base + REG_PWM_FORCE_SHIFT_SET_EN);

	val |= PWM_FORCE_SHIFT_SET;
	writew(val, priv->reg_base + REG_PWM_FORCE_SHIFT_SET_EN);
	return 0;
}

static int _mediatek_pwm_set_swdben_en(struct udevice *dev, uint channel,
				      bool sw_dben)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr = priv->reg_base + REG_PWM0_VDBEN_SW;
	val = readw(addr);

	if (sw_dben)
		val |= REG_PWM_VDBEN_SW_MASK;
	else
		val &= ~REG_PWM_VDBEN_SW_MASK;

	writew(val, addr);

	return 0;
}

static int mediatek_pwm_set_config(struct udevice *dev, uint channel,
				   uint period_ns, uint duty_ns)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s, reg %p, channel %d, period %d, duty %d\n", __func__,
	      priv->reg_base, channel, period_ns, duty_ns);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}
	ret = _mediatek_pwm_set_swdben_en(dev, channel, 0);
	if (ret)
		return ret;
	ret = _mediatek_pwm_set_dutycycle(dev, channel, duty_ns);
	if (ret)
		return ret;
	ret = _mediatek_pwm_set_period(dev, channel, period_ns);
	if (ret)
		return ret;
	ret = _mediatek_pwm_set_swdben_en(dev, channel, 1);
	if (ret)
		return ret;
	_mediatek_pwm_set_force_low(dev, channel, 1);
	ret = _mediatek_pwm_set_reset_en(dev, channel, priv->data.rst_vsync);
	if (ret)
		return ret;
	/* private control setting */
	_mediatek_pwm_set_shift(dev, priv->data.channel, priv->data.shift);
	_mediatek_pwm_set_divider(dev, priv->data.channel, priv->data.div);
	_mediatek_pwm_set_rst_mux(dev, priv->data.channel, priv->data.rst_mux);
	_mediatek_pwm_set_rst_cnt(dev, priv->data.channel, priv->data.rstcnt);

	return 0;
}

static int mediatek_pwm_set_polarity(struct udevice *dev, uint channel,
				     bool polarity)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p, channel %d, polarity %s\n", __func__, priv->reg_base,
	      channel, (polarity) ? "inversed" : "normal");

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	return _mediatek_pwm_set_polarity(dev, channel, polarity);
}

static int mediatek_pwm_set_enable(struct udevice *dev, uint channel,
				   bool enable)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->reg_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}
	ret = _mediatek_pwm_set_enable(dev, channel, enable);
	if (ret)
		return ret;
	return 0;
}

static int mediatek_pwm_probe(struct udevice *dev)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int val;

	debug("%s, reg %p\n", __func__, priv->reg_base);
	debug("%s, clk %p\n", __func__, priv->clk_base);

	val = readw(priv->clk_base + REG_CLK_XTAL_SW_EN);
	val |= XTAL_SW_EN_BIT;
	writew(val, priv->clk_base + REG_CLK_XTAL_SW_EN);

	val = readw(priv->clk_base + REG_CLK_PWM_SW_EN);
	val |= PWM_SW_EN_BIT;
	writew(val, priv->clk_base + REG_CLK_PWM_SW_EN);


	priv->clkspeed = XTAL_HZ;

	// enable PWM clock
	debug("enable pwm IP source clock\n");
	val = readw(priv->reg_base + REG_PWM_CLKALLEN);
	val |= PWMFULL_IP_ENGINE_EN_MASK;
	writew(val, priv->reg_base + REG_PWM_CLKALLEN);


	/* disable all pwm by stop channel clock */

	/* PWM channels initialize, include private control setting */
	/* pwm disabled */
	_mediatek_pwm_set_enable(dev, priv->data.channel, false);

	return 0;
}

static int mediatek_pwm_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void *)dev_read_addr_name(dev, "scanpwm");
	priv->clk_base = (void *)dev_read_addr_name(dev, "clkgen1_nonpm");
	priv->channel_cnt = PWM_MAX_CNT;
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	if (priv->clk_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_u32(dev, "channel", &priv->data.channel);
	if (ret)
		dev_err(dev,
			"Read property 'channel' from DTS fail, default to 0 (%d)\n",
			ret);

	ret = dev_read_u32(dev, "shift", &priv->data.shift);
	if (ret)
		dev_err(dev,
			"Read property 'shift' from DTS fail, default to 0 (%d)\n",
			ret);

	ret = dev_read_u32(dev, "div", &priv->data.div);
	if (ret)
		dev_err(dev,
			"Read property 'div' from DTS fail, default to 0 (%d)\n",
			ret);

	ret = dev_read_u32(dev, "rst-mux", &priv->data.rst_mux);
	if (ret)
		dev_err(dev,
			"Read property 'rst-mux' from DTS fail, default to disable (%d)\n",
			ret);

	ret = dev_read_u32(dev, "rst-vsync", &priv->data.rst_vsync);
	if (ret)
		dev_err(dev,
			"Read property 'rst-vsync' from DTS fail, default to disable (%d)\n",
			ret);

	ret = dev_read_u32(dev, "rstcnt", &priv->data.rstcnt);
	if (ret)
		dev_err(dev,
			"Read property 'rstcnt' from DTS fail, default to 0 (%d)\n",
			ret);

	debug("%s, PWM channel %d:\n", __func__, i);
	debug("shift %d\n", priv->data.shift);
	debug("div %d\n", priv->data.div);
	debug("rst-mux %sable\n",
	      (priv->data.rst_mux) ? "en" : "dis");
	debug("rst-vsync %sable\n",
	      (priv->data.rst_vsync) ? "en" : "dis");
	debug("rstcnt %d\n", priv->data.rstcnt);


	return 0;
}

static const struct pwm_ops mediatek_pwm_ops = {
	.set_config = mediatek_pwm_set_config,
	.set_enable = mediatek_pwm_set_enable,
	.set_invert = mediatek_pwm_set_polarity,
};

static const struct udevice_id mediatek_pwm_id[] = {
	{.compatible = "mediatek,mt5896-pwm"},
	{}
};

U_BOOT_DRIVER(mt5896_pwm) = {
	.name = "mediatek_pwm",
	.id = UCLASS_PWM,
	.of_match = mediatek_pwm_id,
	.ops = &mediatek_pwm_ops,
	.probe = mediatek_pwm_probe,
	.ofdata_to_platdata = mediatek_pwm_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_pwm_priv),
};
