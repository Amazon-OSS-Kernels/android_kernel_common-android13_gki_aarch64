// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>

#include "mtk_dtv_pwm.h"

struct mtk_pwm_dat {
	unsigned int shift;
	unsigned int div;
	bool rst_mux;
	bool rst_vsync;
	unsigned int rstcnt;
};

/**
 * struct mediatek_pwm_priv - mtk_dtv_pwm private data
 *
 * @reg_base: register bank base address
 * @pad_mode: PWM pad mode for each channel
 * @clkspeed: IP source clock rate
 * @channel_cnt: PWM channel count
 * @data: pwm proprietary control data
 * following members use for IP bug workaround
 * @reg_pwm_clken_bitmap : low byte of register 0x01
 * @reg_ctrl: high byte value of register 0x04, 0x07, 0x0A, 0x0D, 0x10, 0x13
 * @reg_rst_mux_hs_rst_cnt: high/low byte value of register 0x14, 0x15, 0x16
 * @reg_period_ext: word value of register 0x20
 * @reg_duty_ext: word value of register 0x21
 * @reg_shift_ctrl: low byte of register 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33
 */
struct mediatek_pwm_priv {
	void __iomem *reg_base;
	u32 pad_mode[4];
	u32 clkspeed;
	int channel_cnt;
	struct mtk_pwm_dat data;
	u8 reg_pwm_clken_bitmap;
	u8 reg_ctrl[6];
	u8 reg_rst_mux_hs_rst_cnt[6];
	u16 reg_period_ext;
	u16 reg_duty_ext;
	u8 reg_shift_ctrl[6];
};

static int _mediatek_pwm_set_shift(struct udevice *dev, uint channel,
				   uint shift)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	if (unlikely((shift >> 18) != 0)) {
		dev_err(dev, "shift value too large to register setting\n");
		return -EINVAL;
	}

	addr = priv->reg_base + (REG_PWM0_SHIFT_L << 2) + (channel << 3);
	/* shift low word */
	debug("set pwm shift low, register %p value 0x%04X\n", addr,
	      shift & 0xFFFF);
	writew(shift & 0xFFFF, addr);
	/* shift msb 2-bit */
	addr += 4;
	//val = readb(addr) & 0xFC;
	val = priv->reg_shift_ctrl[channel];
	debug("set pwm shift high, register %p original value 0x%02X\n", addr,
	      val);
	val |= (shift >> 16) & 0x0003;
	debug("set pwm shift high, register %p value 0x%02X\n", addr, val);
	writeb(val, addr);
	priv->reg_shift_ctrl[channel] = val;

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

	addr = priv->reg_base + ((REG_PWM0_DIV + (channel * 3)) << 2);
	/* low byte, direct write by byte access */
	debug("set pwm divider, register %p value 0x%02X\n", addr,
	      div & 0x00FF);
	writeb(div & 0x00FF, addr);
	/* high byte, direct write by byte access */
	addr =
	    priv->reg_base + ((REG_PWM0_DIV_EXT + (channel >> 1)) << 2) +
	    (channel & 1);
	debug("set pwm divider extend, register %p value 0x%02X\n", addr,
	      div >> 8);
	writeb(div >> 8, addr);

	priv->data.div = div;

	return 0;
}

static int _mediatek_pwm_set_reset_en(struct udevice *dev, uint channel,
				      bool rst_vsync)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr = priv->reg_base + ((REG_PWM0_RESET_EN + (channel * 3)) << 2) + 1;
	//val = readb(addr);
	val = priv->reg_ctrl[channel];
	debug("set pwm vsync reset %sable, register %p original value 0x%02X\n",
	      (rst_vsync) ? "en" : "dis", addr, val);
	if (rst_vsync)
		val |= (REG_PWM_RESET_EN_MASK >> 8);
	else
		val &= ~(REG_PWM_RESET_EN_MASK >> 8);
	debug("set pwm vsync reset %sable, register %p value 0x%02X\n",
	      (rst_vsync) ? "en" : "dis", addr, val);
	writeb(val, addr);
	priv->reg_ctrl[channel] = val;

	return 0;
}

static int _mediatek_pwm_set_rst_mux(struct udevice *dev, uint channel,
				     bool rst_mux)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr =
	    priv->reg_base + ((REG_RST_MUX0 + (channel >> 1)) << 2) +
	    ((channel & 1) ^ 1);
	//val = readb(addr);
	val = priv->reg_rst_mux_hs_rst_cnt[channel];
	debug("set pwm reset mux %sable, register %p original value 0x%02X\n",
	      (rst_mux) ? "en" : "dis", addr, val);
	/* byte access, always set/clear bit 7 */
	if (rst_mux)
		val |= REG_PWM_RST_MUX_EN_MASK_L;
	else
		val &= ~REG_PWM_RST_MUX_EN_MASK_L;
	debug("set pwm reset mux %sable, register %p value 0x%02X\n",
	      (rst_mux) ? "en" : "dis", addr, val);
	writeb(val, addr);
	priv->reg_rst_mux_hs_rst_cnt[channel] = val;

	return 0;
}

static int _mediatek_pwm_set_rst_cnt(struct udevice *dev, uint channel,
				     int rstcnt)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	if (unlikely(rstcnt > 0x0F)) {
		dev_err(dev, "rstcnt value too large to register setting\n");
		return -EINVAL;
	}

	addr =
	    priv->reg_base + ((REG_HS_RST_CNT0 + (channel >> 1)) << 2) +
	    ((channel & 1) ^ 1);
	/* byte access, always set/clear bit 3~0 */
	//val = readb(addr);
	val = priv->reg_rst_mux_hs_rst_cnt[channel];
	debug
	    ("set pwm Hsync reset counter, register %p original value 0x%02X\n",
	     addr, val);
	val &= ~REG_PWM_RST_CNT_MASK_L;
	val |= rstcnt;
	debug("set pwm Hsync reset counter, register %p value 0x%02X\n", addr,
	      val);
	writeb(val, addr);
	priv->reg_rst_mux_hs_rst_cnt[channel] = val;

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

	if (unlikely((duty_set >> 18) != 0)) {
		dev_err(dev,
			"Due to hardware limitation, duty time too long to register setting\n");
		return -EINVAL;
	}

	debug
	    ("duty %d with clock %d divider %d, register value %llu (0x%llX)\n",
	     duty_ns, priv->clkspeed, priv->data.div, duty_set, duty_set);

	/* Vsync double buffer enable by software : disable */
	addr = priv->reg_base + ((REG_PWM0_VDBEN_SW + (channel * 3)) << 2) + 1;
	priv->reg_ctrl[channel] &= ~(REG_PWM_VDBEN_SW_MASK >> 8);
	writeb(priv->reg_ctrl[channel], addr);

	/* duty low word */
	addr = priv->reg_base + ((REG_PWM0_DUTY + (channel * 3)) << 2);
	debug("set pwm duty, register %p value 0x%04llX\n", addr,
	      duty_set & 0xFFFF);
	writew(duty_set & 0xFFFF, addr);
	addr = priv->reg_base + (REG_PWM0_DUTY_EXT << 2);
	/* duty 2-bit msb */
	duty_set >>= 16;
	duty_set <<= (channel * 2);
	mask = 0x03 << (channel * 2);
	//val = readw(addr);
	val = priv->reg_duty_ext;
	debug("set pwm duty extend, register %p original value 0x%04X\n", addr,
	      val);
	val &= ~mask;
	val |= duty_set & mask;
	debug("set pwm duty extend, register %p value 0x%04X\n", addr, val);
	writew(val, addr);
	priv->reg_duty_ext = val;

	/* Vsync double buffer enable by software : enable */
	addr = priv->reg_base + ((REG_PWM0_VDBEN_SW + (channel * 3)) << 2) + 1;
	priv->reg_ctrl[channel] |= (REG_PWM_VDBEN_SW_MASK >> 8);
	writeb(priv->reg_ctrl[channel], addr);

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

	if (unlikely((period_set >> 18) != 0)) {
		dev_err(dev,
			"Due to hardware limitation, period time too long to register setting\n");
		return -EINVAL;
	}

	debug
	    ("period %d with clock %d divider %d, register value %llu (0x%llX)\n",
	     period_ns, priv->clkspeed, priv->data.div, period_set, period_set);

	/* Vsync double buffer enable by software : disable */
	addr = priv->reg_base + ((REG_PWM0_VDBEN_SW + (channel * 3)) << 2) + 1;
	priv->reg_ctrl[channel] &= ~(REG_PWM_VDBEN_SW_MASK >> 8);
	writeb(priv->reg_ctrl[channel], addr);

	/* period low word */
	addr = priv->reg_base + ((REG_PWM0_PERIOD + (channel * 3)) << 2);
	debug("set pwm period, register %p value 0x%04llX\n", addr,
	      period_set & 0xFFFF);
	writew(period_set & 0xFFFF, addr);
	addr = priv->reg_base + (REG_PWM0_PERIOD_EXT << 2);
	/* period 2-bit msb */
	period_set >>= 16;
	period_set <<= (channel * 2);
	mask = 0x03 << (channel * 2);
	//val = readw(addr);
	val = priv->reg_period_ext;
	debug("set pwm period extend, register %p original value 0x%04X\n",
	      addr, val);
	val &= ~mask;
	val |= period_set & mask;
	debug("set pwm period extend, register %p value 0x%04X\n", addr, val);
	writew(val, addr);
	priv->reg_period_ext = val;

	/* Vsync double buffer enable by software : enable */
	addr = priv->reg_base + ((REG_PWM0_VDBEN_SW + (channel * 3)) << 2) + 1;
	priv->reg_ctrl[channel] |= (REG_PWM_VDBEN_SW_MASK >> 8);
	writeb(priv->reg_ctrl[channel], addr);

	return 0;
}

static int _mediatek_pwm_set_polarity(struct udevice *dev, uint channel,
				      bool inversed)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u8 val;

	addr = priv->reg_base + ((REG_PWM0_PORARITY + (channel * 3)) << 2) + 1;
	//val = readb(addr);
	val = priv->reg_ctrl[channel];
	debug("set pwm polarity register %p original value 0x%04X\n", addr,
	      val);
	if (inversed)
		val |= (REG_PWM_POLARITY_MASK >> 8);
	else
		val &= ~(REG_PWM_POLARITY_MASK >> 8);
	debug("set pwm polarity register %p value 0x%04X\n", addr, val);
	writeb(val, addr);
	priv->reg_ctrl[channel] = val;

	return 0;
}

static int mediatek_pwm_set_config(struct udevice *dev, uint channel,
				   uint period_ns, uint duty_ns)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s, reg %p, channel %d, period %d, duty %d\n", __func__,
	      priv->reg_base, channel, period_ns, duty_ns);
	debug
	    ("%s, shift %d, divider %d, vsync rst %sable, rstcnt %d, rst_mux %sable\n",
	     __func__, priv->data.shift, priv->data.div,
	     (priv->data.rst_vsync) ? "en" : "dis", priv->data.rstcnt,
	     (priv->data.rst_mux) ? "en" : "dis");

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	ret = _mediatek_pwm_set_dutycycle(dev, channel, duty_ns);
	if (ret)
		return ret;

	return _mediatek_pwm_set_period(dev, channel, period_ns);
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
	int val;

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->reg_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}
	// PWM channel clock
	//val = readb(priv->reg_base + (REG_PWM_CLKEN_BITMAP << 2));
	val = priv->reg_pwm_clken_bitmap;
	val &= ~(1 << channel);
	if (enable)
		val |= (1 << channel);
	writeb(val, priv->reg_base + (REG_PWM_CLKEN_BITMAP << 2));
	priv->reg_pwm_clken_bitmap = val;

	// PWM output enable
	val = readb(CHIP_REG_PWM_OEN) & ~(1 << (channel + 4));
	if (!enable)
		val |= 1 << (channel + 4);
	writeb(val, CHIP_REG_PWM_OEN);

	return 0;
}

static int mediatek_pwm_probe(struct udevice *dev)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int val;
	int i;

	debug("%s, reg %p\n", __func__, priv->reg_base);

	priv->reg_pwm_clken_bitmap = 0;
	memset(priv->reg_ctrl, 0x40, sizeof(priv->reg_ctrl));
	memset(priv->reg_rst_mux_hs_rst_cnt, 0,
	       sizeof(priv->reg_rst_mux_hs_rst_cnt));
	priv->reg_period_ext = 0;
	priv->reg_duty_ext = 0;
	memset(priv->reg_shift_ctrl, 0, sizeof(priv->reg_shift_ctrl));
	for (i = 0; i < priv->channel_cnt; i++) {
		debug("pwm IP workround registers value 0x%02X 0x%02X 0x%02X\n",
		      priv->reg_ctrl[i], priv->reg_rst_mux_hs_rst_cnt[i],
		      priv->reg_shift_ctrl[i]);
	}

	// enable PWM clock
	debug("enable pwm IP source clock and gate all PWM channels clock\n");
	//val = readb(priv->reg_base + (REG_PWM_CLKEN_BITMAP << 2));
	//val |= PWMFULL_IP_ENGINE_EN_MASK;
	writeb(PWMFULL_IP_ENGINE_EN_MASK,
	       priv->reg_base + (REG_PWM_CLKEN_BITMAP << 2));
	priv->reg_pwm_clken_bitmap = PWMFULL_IP_ENGINE_EN_MASK;

	/* TODO : Select PWM IP source clock rate ? */
	priv->clkspeed = 24000000;

	// MDrv_PWM_Init(E_PWM_DBGLV_ERR_ONLY);
	// nothing to do

	/* set all pwm channels PAD mode */
	debug("set pwm pad modes\n");
	val = readb(CHIP_REG_PWM0) & ~CHIP_PWM0_PAD_MSK;
	writeb(val | priv->pad_mode[0], CHIP_REG_PWM0);
	val = readb(CHIP_REG_PWM1) & ~CHIP_PWM1_PAD_MSK;
	writeb(val | priv->pad_mode[1], CHIP_REG_PWM1);
	val = readb(CHIP_REG_PWM2) & ~CHIP_PWM2_PAD_MSK;
	writeb(val | priv->pad_mode[2], CHIP_REG_PWM2);
	val = readb(CHIP_REG_PWM3) & ~CHIP_PWM3_PAD_MSK;
	writeb(val | priv->pad_mode[3], CHIP_REG_PWM3);

	/* disable all pwm output */
	debug("disable all pwm channels output\n");
	writeb(readb(CHIP_REG_PWM_OEN) | CHIP_PWMS_OEN_MSK, CHIP_REG_PWM_OEN);

	/* private control setting */
	for (i = 0; i < priv->channel_cnt; i++) {
		_mediatek_pwm_set_shift(dev, i, priv->data.shift);
		_mediatek_pwm_set_divider(dev, i, priv->data.div);
		_mediatek_pwm_set_reset_en(dev, i, priv->data.rst_vsync);
		_mediatek_pwm_set_rst_mux(dev, i, priv->data.rst_mux);
		_mediatek_pwm_set_rst_cnt(dev, i, priv->data.rstcnt);
	}

	return 0;
}

static int mediatek_pwm_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_pwm_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	/*num of channels, pad mode for each channel */

	priv->channel_cnt = 6;
	debug("%s, PWM %d channels\n", __func__, priv->channel_cnt);

	priv->pad_mode[0] = 0;
	priv->pad_mode[1] = 0;
	priv->pad_mode[2] = 0;
	priv->pad_mode[3] = 0;
	ret = dev_read_u32_array(dev, "pad-modes", priv->pad_mode, 4);
	if (ret) {
		dev_err(dev, "Read 'pad-modes' from DTS fail (%d)", ret);
	}

	for (i = 0; i < 4 /*priv->channel_cnt */ ; i++) {
		debug("%s, PWM %d pad mode %d\n", __func__, i,
		      priv->pad_mode[i]);
	}

	priv->data.shift = dev_read_u32_default(dev, "shift", 0);
	priv->data.div = dev_read_u32_default(dev, "div", 0);
	priv->data.rst_mux = dev_read_bool(dev, "rst-mux");
	priv->data.rst_vsync = dev_read_bool(dev, "rst-vsync");
	priv->data.rstcnt = dev_read_u32_default(dev, "rstcnt", 0);

	debug("%s, shift %d\n", __func__, priv->data.shift);
	debug("%s, div %d\n", __func__, priv->data.div);
	debug("%s, rst-mux %sable\n", __func__,
	      (priv->data.rst_mux) ? "en" : "dis");
	debug("%s, rst-vsync %sable\n", __func__,
	      (priv->data.rst_vsync) ? "en" : "dis");
	debug("%s, rstcnt %d\n", __func__, priv->data.rstcnt);

	return 0;
}

static const struct pwm_ops mediatek_pwm_ops = {
	.set_config = mediatek_pwm_set_config,
	.set_enable = mediatek_pwm_set_enable,
	.set_invert = mediatek_pwm_set_polarity,
};

static const struct udevice_id mediatek_pwm_id[] = {
	{.compatible = "mediatek,mtk-dtv-pwm"},
	{}
};

U_BOOT_DRIVER(mediatek_pwm) = {
	.name = "mtk_dtv_pwm",
	.id = UCLASS_PWM,
	.of_match = mediatek_pwm_id,
	.ops = &mediatek_pwm_ops,
	.probe = mediatek_pwm_probe,
	.ofdata_to_platdata = mediatek_pwm_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_pwm_priv),
};
