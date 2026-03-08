// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2021. All rights reserved.
 */


#include <common.h>
#include <adc.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>

#define	SARADC_CTRL_REG		0x0
#define SARADC_AISEL_REG	0x40
#define CONTROL_WORD		0x220
#define SAR_CHANNEL_RD_REG	0x100
#define SAR_LOAD_EN_BIT		14
#define SAR_CHANNEL_OFFSET	4

/**
 * struct mt58xx_sar_priv - mt58xx private data
 *
 * @reg_base: register bank base address
 */
#define LOW_ADC(X) ((X>0)?(X):0)

#define SAR_KEY_NUMS    (5)

struct mt58xx_sar_priv {
	void __iomem *reg_base;
	u32 channel_nums;
	u32 delay_us;
	u32 adc_offset;
	u32 sarkey_ch;
	u32 sarkey_lb;
	u32 adc[SAR_KEY_NUMS];
};


static int mt58xx_start_channel(struct udevice *dev, int channel)
{
	struct mt58xx_sar_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + SARADC_CTRL_REG;

	writew(CONTROL_WORD, addr);
	addr =
	    priv->reg_base + SARADC_AISEL_REG;

	val = readw(addr);
	val |= (1 << channel);
	writew(val, addr);
	debug("saradc reg %p ori value 0x%04X\n", addr, val);

	return 0;
}

static int mt58xx_saradc_channel_data(struct udevice *dev, int channel,
				 unsigned int *data)
{
	struct mt58xx_sar_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u16 val;

	addr =
	    priv->reg_base + SARADC_CTRL_REG;

	val = readw(addr);
	val |= (1 << SAR_LOAD_EN_BIT);
	/* Toggle sar */
	writew(val, addr);
	addr =
	    priv->reg_base + (SAR_CHANNEL_RD_REG + (channel*SAR_CHANNEL_OFFSET));
	*data = readw(addr);

	for (int idx = 0; idx < SAR_KEY_NUMS; idx++) {
		if (*data >= priv->sarkey_lb)
		{
			dev_err(dev, "Skip unexpected sar interrupt %d\n", *data);
			break;
		}
		else if ((*data <= (priv->adc[idx] + priv->adc_offset))&&
			(*data >= LOW_ADC(priv->adc[idx] - priv->adc_offset)))
		{
			return true;
		}
	}

	return 0;
}

static int mt58xx_saradc_probe(struct udevice *dev)
{
	struct mt58xx_sar_priv *priv = dev_get_priv(dev);

	debug("%s, reg %p\n", __func__, priv->reg_base);

	// nothing to do

	return 0;
}

static int mt58xx_saradc_ofdata_to_platdata(struct udevice *dev)
{
	struct mt58xx_sar_priv *priv = dev_get_priv(dev);
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	int ret;

	debug("%s, priv %p\n", __func__, priv);
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_u32(dev, "adc_offset", &priv->adc_offset);
	if (ret) {
		dev_err(dev, "Read 'priv->adc_offset' from DTS fail (%d)\n", ret);
		return -EINVAL;
	}
	ret = dev_read_u32(dev, "sar_keypad-ch", &priv->sarkey_ch);
	if (ret) {
		dev_err(dev, "Read 'sar_keypad-ch' from DTS fail (%d)\n", ret);
		return -EINVAL;
	}
	ret = dev_read_u32(dev, "sar_keypad-lb", &priv->sarkey_lb);
	if (ret) {
		dev_err(dev, "Read 'sar_keypad-lb' from DTS fail (%d)\n", ret);
		return -EINVAL;
	}
	ret = dev_read_u32_array(dev, "sar_keypad-adc", priv->adc, SAR_KEY_NUMS);
	if (ret) {
		dev_err(dev, "Read 'sar_keypad-adc' from DTS fail (%d)\n", ret);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "nums", &priv->channel_nums);
	if (ret) {
		dev_err(dev, "Read 'priv->channel_nums' from DTS fail (%d)", ret);
		return ret;
	}
	ret = dev_read_u32(dev, "delay", &priv->delay_us);
	if (ret) {
		dev_err(dev, "Read 'priv->delay_us' from DTS fail (%d)", ret);
		return ret;
	}
	uc_pdata->data_timeout_us = priv->delay_us;
	uc_pdata->channel_mask = (1 << (priv->channel_nums)) - 1;
	return 0;
}

static struct adc_ops mt58xx_saradc_ops = {
	.start_channel = mt58xx_start_channel,
	.channel_data = mt58xx_saradc_channel_data,
};

static const struct udevice_id mt58xx_saradc_of_match[] = {
	{ .compatible = "mediatek,mt5896-sar", },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(mt58xx_saradc) = {
	.name = "mt58xx_saradc",
	.id = UCLASS_ADC,
	.of_match = mt58xx_saradc_of_match,
	.ops = &mt58xx_saradc_ops,
	.probe = mt58xx_saradc_probe,
	.ofdata_to_platdata = mt58xx_saradc_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mt58xx_sar_priv),
};
