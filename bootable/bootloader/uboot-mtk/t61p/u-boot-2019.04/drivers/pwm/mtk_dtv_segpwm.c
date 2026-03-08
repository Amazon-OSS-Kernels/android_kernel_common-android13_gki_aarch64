// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <common.h>
#include <div64.h>
#include <dm.h>
#include <pwm.h>
#include <asm/io.h>


#include <debug_impl.h>
#define	SEG_MAX_CNT		4
#define REG_0004_PWM			0x04
#define CKEN_ALL_EN				(0x1 << 6)
#define REG_00DC_PWM_SEG		0xDC
#define REG_01CC_PWM_SEG		0x1CC
#define REG_0160_PWM_SEG		0x160
#define REG_0014_PWM_SEG		0x14
#define REG_0010_PWM_SEG		0x10
#define REG_0020_PWM_SEG		0x20
#define REG_0024_PWM_SEG		0x24
#define REG_0028_PWM_SEG		0x28
#define REG_002C_PWM_SEG		0x2C
#define REG_0030_PWM_SEG		0x30
#define REG_0034_PWM_SEG		0x34
#define REG_0038_PWM_SEG		0x38
#define REG_003C_PWM_SEG		0x3C
#define REG_0008_PWM_SEG		0x08

#define SEG_OUT_OFFSET			0x4
#define SEG_VRESET_SET			(0x1 << 0)
#define SEG_VSYNC_SYNC_0		(0x1 << 8)
#define SEG_VSYNC_SYNC_1		(0x1 << 9)
#define VSYNC_50_HZ				(50)
#define VSYNC_100_HZ			(100)
#define VSYNC_60_HZ				(60)
#define VSYNC_120_HZ			(120)
#define VSYNC_200_HZ			(200)
#define VSYNC_240_HZ			(240)
#define VSYNC_PULSE_SEL			(3)
#define VSYNC_16384_PULSE		(16384)
#define HALF_DUTY_EVENT			(50)
#define SEG_EVENT_SET			(1 << 14)
#define SEG_EVENT_EN			(1 << 15)
#define SEG_PULS_BYPASS_SET		(1 << 4)
#define SEG_2_TIMES_VSYNC		(2)
#define SEG_4_TIMES_VSYNC		(4)
#define SEG_8_TIMES_VSYNC		(8)
#define SEG_FULL_DUTY			(100)
#define SEG_EVENTS_DEFAULT		(2)
#define HIGH_BYTE_MSK			(0xFF00)
#define BYTE_SHIFT				(8)
#define HALF_BYTE_SHIFT			(4)

struct mtk_seg_dat {
	unsigned int duty_cycle;
	unsigned int shift;
	unsigned int channel;
	unsigned int event;
};

/**
 * struct mediatek_seg_priv - mtk_dtv_pwm private data
 *
 * @seg_clk_base: seg bank clk address
 * @seg_port_base: seg bank port address
 * @seg_ctl_base: seg bank ctl address
 * @seg_pad_base: seg port bank pin address
 * @pwm_clksrc_base: pwm bank src clk address
 * @data: pwm proprietary control data
 */
struct mediatek_seg_priv {
	void __iomem *seg_clk_base;
	void __iomem *seg_port_base;
	void __iomem *seg_ctl_base;
	void __iomem *seg_pad_base;
	void __iomem *pwm_clksrc_base;
	int channel_cnt;
	struct mtk_seg_dat data;
};

static void mtk_segpwm_vsync(struct udevice *dev, unsigned int data)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);
	u16 reg_tmp;

	reg_tmp = readw(priv->seg_ctl_base + REG_0014_PWM_SEG);
	if (!data) {
		reg_tmp |= SEG_VRESET_SET;
		reg_tmp &= ~SEG_VSYNC_SYNC_0;
		reg_tmp &= ~SEG_VSYNC_SYNC_1;
	} else {
		/*reset by n times Vsync*/
		reg_tmp |= SEG_VRESET_SET;
		reg_tmp |= SEG_VSYNC_SYNC_0;
		reg_tmp &= ~SEG_VSYNC_SYNC_1;
		reg_tmp |=(data << HALF_BYTE_SHIFT);
	}
	writew(reg_tmp, priv->seg_ctl_base + REG_0014_PWM_SEG);
	reg_tmp = readw(priv->seg_ctl_base + REG_0010_PWM_SEG);
	reg_tmp |= VSYNC_PULSE_SEL;
	reg_tmp |= SEG_PULS_BYPASS_SET;
	writew(reg_tmp, priv->seg_ctl_base + REG_0010_PWM_SEG);
}

static int _mediatek_seg_set_enable(struct udevice *dev, uint channel,
				    bool enable)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);

	u16 bitop_tmp;
	u16 bitop_org;
	u16 reg_tmp;
	u16 reg_org;

	reg_org = ((priv->data.channel) >> 1)*SEG_OUT_OFFSET;
	bitop_org = readw(priv->seg_pad_base + REG_01CC_PWM_SEG + reg_org);
	/*seg_pad_base port_switch*/
	if (enable) {
		reg_tmp = (priv->data.channel)&1;
		bitop_tmp = bitop_org;
		if (reg_tmp)
			bitop_tmp |= ((priv->data.channel) << BYTE_SHIFT);
		else {
			bitop_tmp &= HIGH_BYTE_MSK;
			bitop_tmp |= (priv->data.channel);
		}
	} else
		bitop_tmp = bitop_org;

	writew(bitop_tmp, priv->seg_pad_base + REG_01CC_PWM_SEG + reg_org);
	/*PWM_SEGx output select*/
	if ((priv->data.channel)< SEG_OUT_OFFSET) {
		reg_org = (priv->data.channel)*SEG_OUT_OFFSET;
		bitop_org = readw(priv->seg_port_base);
		bitop_tmp = ((priv->data.channel)+1) << reg_org;
		bitop_tmp |= bitop_org;
		writew(bitop_tmp, priv->seg_port_base);
	}
	mtk_segpwm_vsync(dev, 0);
	return 0;
}

static int mediatek_seg_set_config(struct udevice *dev, uint channel,
				   uint period_t, uint duty_t)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);

	u16 position;
	u8 vsync_times;

	debug("%s, reg %p, channel %d, period %d, duty %d\n", __func__,
	      priv->seg_clk_base, channel, period_t, duty_t);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	if ((priv->data.duty_cycle) != duty_t) {
		vsync_times = priv->data.event;
		if (vsync_times == SEG_2_TIMES_VSYNC) {
			writew(0, priv->seg_ctl_base + REG_0028_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_002C_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_0030_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_0034_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_0038_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_003C_PWM_SEG);
			position = (priv->data.shift)*(VSYNC_16384_PULSE/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			position |= SEG_EVENT_EN;
			writew(position, priv->seg_ctl_base + REG_0020_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_0024_PWM_SEG);
		}

		if (vsync_times == SEG_4_TIMES_VSYNC) {
			writew(0, priv->seg_ctl_base + REG_0030_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_0034_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_0038_PWM_SEG);
			writew(0, priv->seg_ctl_base + REG_003C_PWM_SEG);
			position = (priv->data.shift)*(VSYNC_16384_PULSE/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			position |= SEG_EVENT_EN;
			writew(position, priv->seg_ctl_base + REG_0020_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_0024_PWM_SEG);
			position += (((SEG_FULL_DUTY - duty_t)*
						VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			writew(position, priv->seg_ctl_base + REG_0028_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_002C_PWM_SEG);
		}

		if (vsync_times == SEG_8_TIMES_VSYNC) {
			position = (priv->data.shift)*(VSYNC_16384_PULSE/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			position |= SEG_EVENT_EN;
			writew(position, priv->seg_ctl_base + REG_0020_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_0024_PWM_SEG);
			position += (((SEG_FULL_DUTY - duty_t)*
						VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			writew(position, priv->seg_ctl_base + REG_0028_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_002C_PWM_SEG);
			position += (((SEG_FULL_DUTY - duty_t)*
						VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			writew(position, priv->seg_ctl_base + REG_0030_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_0034_PWM_SEG);
			position += (((SEG_FULL_DUTY - duty_t)*
						VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position |= SEG_EVENT_SET;//1
			writew(position, priv->seg_ctl_base + REG_0038_PWM_SEG);
			position += ((duty_t*VSYNC_16384_PULSE)/(HALF_DUTY_EVENT*vsync_times));
			position &= ~SEG_EVENT_SET;//0
			writew(position, priv->seg_ctl_base + REG_003C_PWM_SEG);
		}
		writew(1, priv->seg_ctl_base + REG_0008_PWM_SEG);
		priv->data.duty_cycle = duty_t;
	}
	return 0;
}


static int mediatek_seg_set_enable(struct udevice *dev, uint channel,
				   bool enable)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p, %sable pwm channel %d\n", __func__, priv->seg_ctl_base,
	      (enable) ? "en" : "dis", channel);

	if (unlikely(channel >= priv->channel_cnt)) {
		dev_err(dev, "request pwm channel %d not exist\n", channel);
		return -EINVAL;
	}

	return _mediatek_seg_set_enable(dev, channel, enable);
}

static int mediatek_seg_probe(struct udevice *dev)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);
	unsigned int val;

	// enable SEG clock
	val = readw(priv->seg_clk_base);
	val |= 1 << (priv->data.channel);
	writew(val, priv->seg_clk_base);

	val = readw(priv->pwm_clksrc_base);
	val |= CKEN_ALL_EN;
	writew(val, priv->pwm_clksrc_base);

	return 0;
}

static int mediatek_seg_ofdata_to_platdata(struct udevice *dev)
{
	struct mediatek_seg_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;

	debug("%s, priv %p\n", __func__, priv);
	priv->seg_clk_base = (void *)dev_read_addr_name(dev, "seg-clk");
	priv->seg_port_base = (void *)dev_read_addr_name(dev, "seg-port");
	priv->seg_ctl_base = (void *)dev_read_addr_name(dev, "seg-ctl");
	priv->seg_pad_base = (void *)dev_read_addr_name(dev, "seg-pad");
	priv->pwm_clksrc_base = (void *)dev_read_addr_name(dev, "pwm-clksrc");
	priv->channel_cnt = SEG_MAX_CNT;

	if (priv->seg_clk_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	if (priv->seg_port_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	if (priv->seg_ctl_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	if (priv->seg_pad_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	if (priv->pwm_clksrc_base == (void __iomem *)FDT_ADDR_T_NONE)
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

	ret = dev_read_u32(dev, "event", &priv->data.event);
	if (ret)
		dev_err(dev,
			"Read property 'event' from DTS fail, default to 0 (%d)\n",
			ret);


	debug("%s, PWM channel %d:\n", __func__, i);
	debug("shift %d\n", priv->data.shift);
	debug("event %d\n", priv->data.event);


	return 0;
}

static const struct pwm_ops mediatek_seg_ops = {
	.set_config = mediatek_seg_set_config,
	.set_enable = mediatek_seg_set_enable,
};

static const struct udevice_id mediatek_seg_id[] = {
	{.compatible = "mediatek,mtkdtv-seg"},
	{}
};

U_BOOT_DRIVER(mtkdtv_seg) = {
	.name = "mediatek_seg",
	.id = UCLASS_PWM,
	.of_match = mediatek_seg_id,
	.ops = &mediatek_seg_ops,
	.probe = mediatek_seg_probe,
	.ofdata_to_platdata = mediatek_seg_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mediatek_seg_priv),
};
