// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Mediatek 2021 - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_MAILBOX

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mailbox-uclass.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define MTK_MBOX_HDR_FREE_MSK		BIT(7)
#define MTK_MBOX_HDR_ID_MSK		GENMASK(6, 0)
#define MTK_MBOX_SIZE			(16)
#define MTK_MBOX_TXPOLL_PERIOD		(5)
#define MTK_MBOX_CHAN_NUM		(128)
#define MTK_MBOX_HEADER_SIZE		(2)
#define MTK_MBOX_MSG_START_OFFSET	(MTK_MBOX_HEADER_SIZE)
/* total 32 bytes for one channel, include tx(16 bytes) and rx(16 bytes) */
#define MTK_MBOX_TX_START_OFFSET	(0)
#define MTK_MBOX_RX_START_OFFSET	(mbox->packet_size)
#define MTK_MBOX_IPI_FIRE_BIT_NUM_MAX	15

struct mt5896_mbox {
	void __iomem *reg_base;
	void __iomem *ctrl_reg;
	u32 n_chans;
	u32 ipi_fire;
	u32 txdone_timeout;
	int packet_size;
	u8 *rx_msg;
	u8 *tx_msg;
};

static int mt5896_mbox_of_xlate(struct mbox_chan *chan,
				struct ofnode_phandle_args *args)
{
	chan->id = args->args[0];

	return 0;
}

static int mt5896_mbox_request(struct mbox_chan *chan)
{
	struct mt5896_mbox *mbox = dev_get_priv(chan->dev);

	if (chan->id >= mbox->n_chans) {
		dev_err(chan->dev, "failed to request channel: %ld\n",
			chan->id);
		return -EINVAL;
	}

	return 0;
}

static int mt5896_mbox_free(struct mbox_chan *chan)
{
	return 0;
}

static int __mt5896_fire_mbox(struct mbox_chan *chan)
{
	struct mt5896_mbox *mbox = dev_get_priv(chan->dev);
	u8 val;

	if (mbox->ipi_fire > MTK_MBOX_IPI_FIRE_BIT_NUM_MAX) {
		dev_err(chan->dev, "invalid ipi-fire(%d)\n", (int)mbox->ipi_fire);
		return -EINVAL;
	}

	val = readb(mbox->ctrl_reg);
	writeb(val | BIT(mbox->ipi_fire), mbox->ctrl_reg);
	writeb(val & ~BIT(mbox->ipi_fire), mbox->ctrl_reg);

	return 0;
}

static int __mt5896_txdone_mbox(struct mbox_chan *chan)
{
	struct mt5896_mbox *mbox = dev_get_priv(chan->dev);
	ulong start_time;
	u8 val;
	int retry = 3;

	start_time = timer_get_us();

	for (;;) {
		/* read first byte as header to check txdone or not */
		val = readb(mbox->reg_base);
		if (!(val & MTK_MBOX_HDR_FREE_MSK))
			break;

		if ((timer_get_us() - start_time) >= mbox->txdone_timeout) {
			dev_err(chan->dev, "chan[%d] send failed, timeout(%d us)\n",
				(int)chan->id, (int)mbox->txdone_timeout);
			if (--retry) {
				__mt5896_fire_mbox(chan);
				start_time = timer_get_us();
				continue;
			}

			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int mt5896_mbox_send(struct mbox_chan *chan, const void *data)
{
	struct mt5896_mbox *mbox = dev_get_priv(chan->dev);
	int i, ret = 0;
	u32 reg;
	u8 val;

	/* read first byte as header to check busy or not */
	val = readb(mbox->reg_base + MTK_MBOX_TX_START_OFFSET * 2);
	if (val & MTK_MBOX_HDR_FREE_MSK) {
		dev_err(chan->dev, "chan[%d] send failed, busy by id=%d\n",
			(int)chan->id, (int)(val & ~MTK_MBOX_HDR_FREE_MSK));
		return -EBUSY;
	}

	/* gen header */
	mbox->tx_msg[0] = (chan->id & MTK_MBOX_HDR_ID_MSK);

	/* fill msg */
	memcpy(&mbox->tx_msg[MTK_MBOX_MSG_START_OFFSET], data, mbox->packet_size - MTK_MBOX_HEADER_SIZE);

	/* write to reg */
	for (i = 0; i < mbox->packet_size; i++) {
		reg = MTK_MBOX_TX_START_OFFSET * 2;
		if (i % 2)
			reg += (((i - 1) * 2) + 1);
		else
			reg += i * 2;

		//dev_err(chan->dev, "send: reg=0x%x, data[%d]=0x%x\n",
		//	  reg, i, mbox->tx_msg[i]);

		writeb(mbox->tx_msg[i], mbox->reg_base + reg);
	}

	/* write txdone bit at last */
	reg = MTK_MBOX_TX_START_OFFSET * 2;
	writeb(mbox->tx_msg[0] | MTK_MBOX_HDR_FREE_MSK, mbox->reg_base + reg);

	/* fire */
	ret = __mt5896_fire_mbox(chan);

	/* optional: check txdone cleared or not for real txdone */
	if (mbox->txdone_timeout)
		ret = __mt5896_txdone_mbox(chan);

	return ret;
}

static int mt5896_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct mt5896_mbox *mbox = dev_get_priv(chan->dev);
	int i;
	u32 reg;
	u8 val;

	/* if no txdone bit set, return nodata */
	val = readb(mbox->reg_base + MTK_MBOX_RX_START_OFFSET * 2);
	if (!(val & MTK_MBOX_HDR_FREE_MSK)) {
		dev_err(chan->dev, "chan[%d] recv failed, no msg\n",
			(int)chan->id);
		return -ENODATA;
	}

	/* if id from header is not yours, return nodata */
	val = readb(mbox->reg_base + MTK_MBOX_RX_START_OFFSET * 2);
	if ((val & ~MTK_MBOX_HDR_FREE_MSK) != chan->id) {
		dev_err(chan->dev, "chan[%d] recv failed, msg is for id=%d\n",
			(int)chan->id, (int)(val & ~MTK_MBOX_HDR_FREE_MSK));
		return -ENODATA;
	}

	/* get data */
	for (i = 0; i < mbox->packet_size; i++) {
		reg = MTK_MBOX_RX_START_OFFSET * 2;
		if (i % 2)
			reg += (((i - 1) * 2) + 1);
		else
			reg += i * 2;

		mbox->rx_msg[i] = readb(mbox->reg_base + reg);

		//dev_err(chan->dev, "recv: reg=0x%x, data[%d]=0x%x\n",
		//	  reg, i, mbox->rx_msg[i]);
	}

	/* clear txdone bit */
	val = readb(mbox->reg_base + MTK_MBOX_RX_START_OFFSET * 2);
	writeb(val & ~MTK_MBOX_HDR_FREE_MSK, mbox->reg_base + MTK_MBOX_RX_START_OFFSET * 2);

	memcpy(data, &mbox->rx_msg[MTK_MBOX_MSG_START_OFFSET], mbox->packet_size - MTK_MBOX_HEADER_SIZE);

	return 0;
}

static void mt5896_mbox_hw_init(struct mt5896_mbox *mbox)
{
	int i;
	u32 reg = 0;

	/* only clear mailbox dummy reg of tx direction */
	for (i = 0; i < mbox->packet_size; i++) {
		reg = MTK_MBOX_TX_START_OFFSET * 2;
		if (i % 2)
			reg += (((i - 1) * 2) + 1);
		else
			reg += i * 2;

		writeb(0, mbox->reg_base + reg);
	}
}

static int mt5896_mbox_probe(struct udevice *dev)
{
	struct mt5896_mbox *mbox = dev_get_priv(dev);
	fdt_addr_t addr, ctrl_addr;
	int ret;
	u32 packet_size = 0;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	mbox->reg_base = (void __iomem *)addr;

	ctrl_addr = dev_read_addr_index(dev, 1);

	mbox->ctrl_reg = (void __iomem *)ctrl_addr;

	ret = dev_read_u32(dev, "ipi-fire", &mbox->ipi_fire);
	if (ret) {
		dev_err(dev, "cannot get ipi-fire\n");
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "txdone-timeout", &mbox->txdone_timeout);
	if (ret) {
		mbox->txdone_timeout = 0;
	}

	ret = dev_read_u32(dev, "packet-size", &packet_size);
	if (ret) {
		packet_size = MTK_MBOX_SIZE;
	}
	mbox->packet_size = packet_size;

	/* get channel number */
	mbox->n_chans = MTK_MBOX_CHAN_NUM;

	/* create rx buffer */
	mbox->rx_msg = malloc(mbox->packet_size);
	if (!mbox->rx_msg) {
		dev_err(dev, "cannot allocate rx msg\n");
		ret = -ENOMEM;
		goto _mbox_drv_probe_err;
	}
	memset(mbox->rx_msg, 0, mbox->packet_size);

	/* create tx buffer */
	mbox->tx_msg = malloc(mbox->packet_size);
	if (!mbox->tx_msg) {
		dev_err(dev, "cannot allocate tx msg\n");
		ret = -ENOMEM;
		goto _mbox_drv_probe_err;
	}
	memset(mbox->tx_msg, 0, mbox->packet_size);

	/* hardware init */
	mt5896_mbox_hw_init(mbox);

	debug("ipi_fire=%d, txdone_timeout=%d, packet_size=%d\n",
	      (int)mbox->ipi_fire, (int)mbox->txdone_timeout, (int)mbox->packet_size);
	debug("mt5896_mbox_probe end\n");

	return 0;

_mbox_drv_probe_err:
	if (mbox->rx_msg)
		free(mbox->rx_msg);
	if (mbox->tx_msg)
		free(mbox->tx_msg);

	debug("mt5896_mbox_probe failed: %d\n", ret);

	return ret;
}

static const struct udevice_id mt5896_mbox_ids[] = {
	{ .compatible = "mediatek,mt5896-mbox" },
	{ }
};

struct mbox_ops mt5896_mbox_ops = {
	.request = mt5896_mbox_request,
	.free = mt5896_mbox_free,
	.send = mt5896_mbox_send,
	.recv = mt5896_mbox_recv,
	.of_xlate = mt5896_mbox_of_xlate,
};

U_BOOT_DRIVER(mt5896_mbox) = {
	.name = "mt5896_mbox",
	.id = UCLASS_MAILBOX,
	.of_match = mt5896_mbox_ids,
	.probe = mt5896_mbox_probe,
	.priv_auto_alloc_size = sizeof(struct mt5896_mbox),
	.ops = &mt5896_mbox_ops,
};
