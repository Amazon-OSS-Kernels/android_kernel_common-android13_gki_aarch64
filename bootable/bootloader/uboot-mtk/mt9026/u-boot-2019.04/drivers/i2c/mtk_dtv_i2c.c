// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Inc. (C) 2018-2020. All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <linux/io.h>
#include <i2c.h>

#include "mtk_dtv_i2c.h"

#define MIU0_BUS_BASE               (0x20000000)
#define MIU1_BUS_BASE               (0xA0000000)

#define HWI2C_HAL_RETRY_TIMES       (3)
#define HWI2C_HAL_WAIT_TIMEOUT      (50000)

typedef enum _HAL_HWI2C_STATE {
	E_HAL_HWI2C_STATE_IDEL = 0,
	E_HAL_HWI2C_STATE_START,
	E_HAL_HWI2C_STATE_WRITE,
	E_HAL_HWI2C_STATE_READ,
	E_HAL_HWI2C_STATE_INT,
	E_HAL_HWI2C_STATE_WAIT,
	E_HAL_HWI2C_STATE_STOP
} HAL_HWI2C_STATE;

/**
 * struct mtk_i2c_priv - i2c private data
 *
 * @reg_base: register bank base address
 * @pad_mode: pad mux mode
 * @clkspeed: IP source clock rate
 * @busspeed: I2C bus speed
 * @stop_cnt: SCL and SDA count for stop
 * @sda_cnt: closk count between falling edge SCL and SDA
 * @start_cnt: SCL and SDA count for start
 * @data_lat_cnt: data latch timing
 * @scl_stretch: enable/disable clock stretch
 */
struct mtk_i2c_priv {
	void __iomem *reg_base;
	int pad_mode;
	u32 clkspeed;
	u32 busspeed;
	u32 stop_cnt;
	u32 sda_cnt;
	u32 start_cnt;
	u32 data_lat_cnt;
	bool scl_stretch;
};

/*
 * DMA internal buffer mode internal APIs
 */
static inline void __mtk_i2c_clear_dma_done(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	writeb(_DMA_TXR_DONE, priv->reg_base + REG_HWI2C_DMA_TXR);
}

static int __mtk_i2c_wait_dma_int(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int count = HWI2C_HAL_WAIT_TIMEOUT;

	while (count > 0) {
		if (readb(priv->reg_base + REG_HWI2C_DMA_TXR))
			break;
		udelay(1);
		count--;
	}

	/* clear interrupt */
	__mtk_i2c_clear_dma_done(dev);

	return (count != 0) ? 0 : -ETIMEDOUT;
}

static inline void __mtk_i2c_dma_enable(struct udevice *dev, bool enable)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u8 val;

	val = readb(priv->reg_base + REG_HWI2C_MIIC_CFG);
	val &= ~_MIIC_CFG_EN_DMA;
	if (enable)
		val |= _MIIC_CFG_EN_DMA;
	writeb(val, priv->reg_base + REG_HWI2C_MIIC_CFG);
}

static inline void __mtk_i2c_dma_ctrl(struct udevice *dev, bool direction,
				      bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u8 val;

	val = readb(priv->reg_base + REG_HWI2C_DMA_CTL);
	if (stop)
		/* clear control bit means will send stop condition */
		val &= ~_DMA_CTL_TXNOSTOP;
	else
		/* set control bit means will NOT send stop condition */
		val |= _DMA_CTL_TXNOSTOP;
	if (direction)
		/* set control bit means execute I2C data read */
		val |= _DMA_CTL_RDWTCMD;
	else
		/* clear control bit means execute I2C data write */
		val &= ~_DMA_CTL_RDWTCMD;
	writeb(val, priv->reg_base + REG_HWI2C_DMA_CTL);

	dev_dbg(dev, "%s: set value 0x%02X\n", __func__, val);
}

static inline void __mtk_i2c_dma_rst(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u8 val;

	val = readb(priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val | _DMA_CFG_RESET, priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val & ~_DMA_CFG_RESET, priv->reg_base + REG_HWI2C_DMA_CFG);
}

static inline void __mtk_i2c_dma_rst_miu(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u8 val;

	val = readb(priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val | _DMA_CFG_MIURST, priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val & ~_DMA_CFG_MIURST, priv->reg_base + REG_HWI2C_DMA_CFG);
}

static inline void __mtk_i2c_dma_chipaddr(struct udevice *dev, int addr_fld)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	dev_dbg(dev, "%s: set addr 0x%04X\n", __func__, addr_fld);

	// set slave address, cmd buffer only for 10-bit address SW workaround
	if ((addr_fld & 0xFF00)) {
		// 10-bit address
		// slave addr 1st 7 bits in slave addr
		writeb(addr_fld >> 9, priv->reg_base + REG_HWI2C_DMA_SLVADR);
		// slave addr 2nd byte in cmd
		writeb((addr_fld & 0xFF),
		       priv->reg_base + REG_HWI2C_DMA_CMDDAT0);
		// 1 byte cmd for slave addr 2nd byte
		writeb(1, priv->reg_base + REG_HWI2C_DMA_CMDLEN);
	} else {
		// 7-bit address
		// slave addr 7 bits in slave addr
		writeb(addr_fld >> 1, priv->reg_base + REG_HWI2C_DMA_SLVADR);
		// no cmd for slave addr 2nd byte
		writeb(0, priv->reg_base + REG_HWI2C_DMA_CMDLEN);
	}
}

static inline void __mtk_i2c_dma_setlength(struct udevice *dev, int length)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	writew(length & 0xFFFF, priv->reg_base + REG_HWI2C_DMA_DATLEN_L);
	writew((length >> 16) & 0xFFFF,
	       priv->reg_base + REG_HWI2C_DMA_DATLEN_H);
	dev_dbg(dev, "%s: set length 0x%04X\n", __func__, length);
}

static inline void __mtk_i2c_dma_setmiuaddr(struct udevice *dev, u8 *buf)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u64 addr = virt_to_phys(buf) - MIU0_BUS_BASE;

	dev_dbg(dev, "%s: set buf 0x%016llX\n", __func__, addr);

	writew(addr & 0xFFFF, priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B0_B7);
	writew((addr >> 16) & 0xFFFF,
	       priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B16_B23);
	writeb((addr >> 32) & 0xFF,
	       priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B32_B39);
}

static inline void __mtk_i2c_dma_start(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	writeb(_DMA_CTL_TRIG, priv->reg_base + REG_HWI2C_DMA_CTL_TRIG);
}

static inline void __mtk_i2c_dma_restart(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	writeb(_DMA_CTL_RETRIG, priv->reg_base + REG_HWI2C_DMA_CTL_RETRIG);
}

static inline int __mtk_i2c_dma_get_tfr_len(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int length;

	length = readw(priv->reg_base + REG_HWI2C_DMA_TXFRCNT_H);
	length <<= 16;
	length |= readw(priv->reg_base + REG_HWI2C_DMA_TXFRCNT_L);
	dev_dbg(dev, "%s: transfer length %d\n", __func__, length);
	return length;
}

/*
 * RIU mode internal APIs
 */
static inline void __mtk_i2c_clear_int(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	writeb(_INT_CTL, priv->reg_base + REG_HWI2C_INT_CTL);
}

static int __mtk_i2c_wait_int(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int count = HWI2C_HAL_WAIT_TIMEOUT;

	while (count > 0) {
		if (readb(priv->reg_base + REG_HWI2C_INT_CTL))
			break;
		udelay(1);
		count--;
	}

	/* clear interrupt */
	__mtk_i2c_clear_int(dev);

	return (count != 0) ? 0 : -ETIMEDOUT;
}

static inline int __mtk_i2c_get_state(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int cur_state =
	    readb(priv->reg_base + REG_HWI2C_CUR_STATE) & _CUR_STATE_MSK;

	if (cur_state <= 0)	// 0: idle
		return E_HAL_HWI2C_STATE_IDEL;
	else if (cur_state <= 2)	// 1~2:start
		return E_HAL_HWI2C_STATE_START;
	else if (cur_state <= 6)	// 3~6:write
		return E_HAL_HWI2C_STATE_WRITE;
	else if (cur_state <= 10)	// 7~10:read
		return E_HAL_HWI2C_STATE_READ;
	else if (cur_state <= 11)	// 11:interrupt
		return E_HAL_HWI2C_STATE_INT;
	else if (cur_state <= 12)	// 12:wait
		return E_HAL_HWI2C_STATE_WAIT;
	else			// 13~15:stop
		return E_HAL_HWI2C_STATE_STOP;
}

static int __mtk_i2c_wait_state(struct udevice *dev, int state)
{
	int count = HWI2C_HAL_WAIT_TIMEOUT;

	while (count > 0) {
		if (state == __mtk_i2c_get_state(dev))
			break;
		udelay(1);
		count--;
	}

	return (count != 0) ? 0 : -ETIMEDOUT;
}

static int __mtk_i2c_send_start(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int val;

	val = readb(priv->reg_base + REG_HWI2C_CMD_START) | _CMD_START;
	writeb(val, priv->reg_base + REG_HWI2C_CMD_START);

	if (__mtk_i2c_wait_int(dev)) {
		dev_err(dev, "i2c bus %s send start timeout (0)\n", dev->name);
		return -ETIMEDOUT;
	}

	return __mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_WAIT);
}

static int __mtk_i2c_send_byte(struct udevice *dev, u8 data)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	//if (__mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_WAIT))
	//    return -ETIMEDOUT;

	dev_dbg(dev, "%s, data %02X\n", __func__, data);

	writeb(data, priv->reg_base + REG_HWI2C_WDATA);
	if (__mtk_i2c_wait_int(dev)) {
		dev_err(dev, "i2c bus %s send byte timeout (0)\n", dev->name);
		return -ETIMEDOUT;
	}

	if (__mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_WAIT)) {
		dev_err(dev, "i2c bus %s get byte timeout (1)\n", dev->name);
		return -ETIMEDOUT;
	}

	if (!(readb(priv->reg_base + REG_HWI2C_WDATA_GET) &
	      _WDATA_GET_ACKBIT)) {
		udelay(1);
		return 0;
	} else {
		return -EREMOTEIO;
	}
}

int __mtk_i2c_get_byte(struct udevice *dev, u8 *ptr, bool ack_data)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int val;

	dev_dbg(dev, "%s, ack_data %d\n", __func__, ack_data);

	val = _RDATA_CFG_TRIG;
	if (!ack_data)
		val |= _RDATA_CFG_ACKBIT;
	writeb(val, priv->reg_base + REG_HWI2C_RDATA_CFG);

	__mtk_i2c_wait_int(dev);

	if (__mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_WAIT)) {
		dev_err(dev, "i2c bus %s get byte timeout (0)\n", dev->name);
		return -ETIMEDOUT;
	}

	*ptr = readb(priv->reg_base + REG_HWI2C_RDATA);

	__mtk_i2c_clear_int(dev);
	if (__mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_WAIT)) {
		dev_err(dev, "i2c bus %s get byte timeout (1)\n", dev->name);
		return -ETIMEDOUT;
	}

	return 0;
}

static int __mtk_i2c_send_stop(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int val;

	val = readb(priv->reg_base + REG_HWI2C_CMD_STOP) | _CMD_STOP;
	writeb(val, priv->reg_base + REG_HWI2C_CMD_STOP);

	if (__mtk_i2c_wait_int(dev)) {
		dev_err(dev, "i2c bus %s send stop timeout\n", dev->name);
		return -ETIMEDOUT;
	}

	return __mtk_i2c_wait_state(dev, E_HAL_HWI2C_STATE_IDEL);
}

/*
 * I2C port setup
 */
static int _mtk_i2c_enable_master(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int val;

	dev_dbg(dev, "%s, reg %p\n", __func__, priv->reg_base);

	/*
	 * RIU mode I2C initial
	 */

	/* (1) clear interrupt */
	__mtk_i2c_clear_int(dev);
	/* (2) reset standard master iic */
	val = readb(priv->reg_base + REG_HWI2C_MIIC_CFG);
	writeb(val | _MIIC_CFG_RESET, priv->reg_base + REG_HWI2C_MIIC_CFG);
	writeb(val & ~_MIIC_CFG_RESET, priv->reg_base + REG_HWI2C_MIIC_CFG);
	/* (3) configuration */
	val = readb(priv->reg_base + REG_HWI2C_MIIC_CFG);
	val |= _MIIC_CFG_EN_INT;
	val |= (priv->scl_stretch) ? _MIIC_CFG_EN_CLKSTR : 0;
	val |= _MIIC_CFG_EN_FILTER;
	val |= _MIIC_CFG_EN_PUSH1T;
	writeb(val, priv->reg_base + REG_HWI2C_MIIC_CFG);

	/*
	 * DMA internal buffer mode I2C initial
	 */

	//(1) clear interrupt
	__mtk_i2c_clear_dma_done(dev);
	//(2) reset DMA
	//(2-1) reset DMA engine
	__mtk_i2c_dma_rst(dev);
	//(2-2)  reset MIU module in DMA engine
	__mtk_i2c_dma_rst_miu(dev);

	//(3) default configuration
	//(3-1) address mode: 7-bit address, SW workaround for 10-bit address
	val = readb(priv->reg_base + REG_HWI2C_DMA_SLVCFG);
	writeb(val & ~_DMA_10BIT_MODE, priv->reg_base + REG_HWI2C_DMA_SLVCFG);
	//(3-2) MIU priority: low priority
	val = readb(priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val & ~_DMA_CFG_MIUPRI, priv->reg_base + REG_HWI2C_DMA_CFG);
	//(3-3) MIU channel: MIU_CH0
	val = readb(priv->reg_base + REG_HWI2C_DMA_CTL);
	writeb(val & ~_DMA_CTL_MIUCHSEL, priv->reg_base + REG_HWI2C_DMA_CFG);
	//(3-4) Enable DMA transfer interrupt
	val = readb(priv->reg_base + REG_HWI2C_DMA_CFG);
	writeb(val | _DMA_CFG_INTEN, priv->reg_base + REG_HWI2C_DMA_CFG);
	//(3-5) DMA default disable, enable when use it
	val = readb(priv->reg_base + REG_HWI2C_MIIC_CFG);
	writeb(val & ~_MIIC_CFG_EN_DMA, priv->reg_base + REG_HWI2C_MIIC_CFG);

	//dev_err(dev, "enable i2c bus %s fail (%d)", dev->name, ret);
	return 0;
}

static int _mtk_i2c_set_clock(struct udevice *dev, int speed)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int divider = priv->clkspeed / speed;
	int clk_high_cnt, clk_low_cnt;

	dev_dbg(dev, "%s, reg %p, set bus speed %d\n", __func__, priv->reg_base,
		speed);

	if ((priv->clkspeed % speed) != 0)
		divider++;

	if (divider < (priv->sda_cnt + priv->data_lat_cnt + 4)) {
		dev_err(dev, "set i2c bus %s bus speed %d fail\n", dev->name,
			speed);
		return -EINVAL;
	}

	divider -= 4;
	clk_high_cnt = divider / 2;
	clk_low_cnt = divider - clk_high_cnt;

	writew(clk_high_cnt, priv->reg_base + REG_HWI2C_CKH_CNT);
	writew(clk_low_cnt, priv->reg_base + REG_HWI2C_CKL_CNT);
	writew(priv->stop_cnt, priv->reg_base + REG_HWI2C_STP_CNT);
	writew(priv->sda_cnt, priv->reg_base + REG_HWI2C_SDA_CNT);
	writew(priv->start_cnt, priv->reg_base + REG_HWI2C_STT_CNT);
	writew(priv->data_lat_cnt, priv->reg_base + REG_HWI2C_LTH_CNT);

	priv->busspeed = speed;

	return 0;
}

/*
 * I2C transfer APIs
 */
static int _mtk_i2c_addressing_read(struct udevice *dev, int addr_fld,
				    bool stop)
{
	u8 data_stage_addr;

	/*
	 * addressing stage
	 */
	if ((addr_fld & 0xFF00)) {
		// 10-bit address
		if (__mtk_i2c_send_start(dev)) {
			dev_err(dev, "%s Start cond(addr) failed\n", __func__);
			goto i2c_addressing_read_err;
		}

		udelay(5);
		// slave addr 1st 7 bits
		if (__mtk_i2c_send_byte(dev, (addr_fld >> 8))) {
			dev_err(dev, "%s Chip addr(high) failed\n", __func__);
			goto i2c_addressing_read_err;
		}

		// slave addr 2nd byte
		if (__mtk_i2c_send_byte(dev, (addr_fld & 0xFF))) {
			dev_err(dev, "%s Chip addr(low) failed\n", __func__);
			goto i2c_addressing_read_err;
		}

		data_stage_addr = (addr_fld >> 8) | 1;
	} else
		// 7-bit address (no addressing stage)
		data_stage_addr = (addr_fld & 0xFF) | 1;

	/*
	 * data reveiving stage (no data)
	 */
	if (__mtk_i2c_send_start(dev)) {
		dev_err(dev, "%s Start cond(data) failed\n", __func__);
		goto i2c_addressing_read_err;
	}

	udelay(5);

	if (__mtk_i2c_send_byte(dev, data_stage_addr)) {
		dev_err(dev, "%s Chip addr(data) failed\n", __func__);
		goto i2c_addressing_read_err;
	}

	if (stop)
		__mtk_i2c_send_stop(dev);

	return 0;

i2c_addressing_read_err:
	__mtk_i2c_send_stop(dev);
	return -EREMOTEIO;
}

static int _mtk_i2c_addressing_write(struct udevice *dev, int addr_fld,
				     bool stop)
{
	/*
	 * addressing stage
	 */
	if (__mtk_i2c_send_start(dev)) {
		dev_err(dev, "%s Start cond failed\n", __func__);
		goto i2c_addressing_write_err;
	}

	udelay(5);

	if ((addr_fld & 0xFF00)) {
		// 10-bit address, slave addr 1st 7 bits
		if (__mtk_i2c_send_byte(dev, (addr_fld >> 8))) {
			dev_err(dev, "%s Chip addr(high) failed\n", __func__);
			goto i2c_addressing_write_err;
		}
	}

	// slave addr 2nd byte or 7-bit address
	if (__mtk_i2c_send_byte(dev, (addr_fld & 0xFF))) {
		dev_err(dev, "%s Chip addr(low) failed\n", __func__);
		goto i2c_addressing_write_err;
	}

	/*
	 * data sending stage (no data)
	 */

	if (stop)
		__mtk_i2c_send_stop(dev);

	udelay(60);

	return 0;

i2c_addressing_write_err:
	__mtk_i2c_send_stop(dev);
	return -EREMOTEIO;
}

static int _mtk_i2c_dma_read(struct udevice *dev, int addr_fld, u8 *buf,
			     int len, bool stop)
{
	int ret;
	int xfr_len;

	if ((addr_fld & 0xFF00)) {
		// 10-bit address, addressing sequence the same as write with
		// 0 byte data w/o stop condition
		ret = _mtk_i2c_addressing_write(dev, addr_fld, false);
		if (ret < 0)
			goto i2c_dma_read_error;

		// set data stage slave address for DMA mode
		__mtk_i2c_dma_chipaddr(dev, addr_fld >> 8);
	} else {
		// set slave address for DMA mode
		__mtk_i2c_dma_chipaddr(dev, addr_fld);
	}

	// enable DMA mode
	__mtk_i2c_dma_enable(dev, true);
	// request data transfer length
	__mtk_i2c_dma_setlength(dev, len);

	// set data address from I2C slave to I2C DMA
	__mtk_i2c_dma_setmiuaddr(dev, buf);
	// invalidate cache
	invalidate_dcache_range((ulong)buf,
				(ulong)buf + roundup(len, ARCH_DMA_MINALIGN));

	// need reset dma engine before trigger dma
	__mtk_i2c_dma_rst(dev);
	// need reset dma miu before trigger dma
	__mtk_i2c_dma_rst_miu(dev);

	// direction & transfer with stop
	__mtk_i2c_dma_ctrl(dev, true, stop);
	// trigger DMA
	__mtk_i2c_dma_start(dev);
	// wait DMA done
	ret = __mtk_i2c_wait_dma_int(dev);
	if (ret)
		goto i2c_dma_read_error;

	// check actual data length
	xfr_len = __mtk_i2c_dma_get_tfr_len(dev);
	if (xfr_len != len) {
		dev_err(dev, "xfer(R) len %d, req %d\n", xfr_len, len);
		ret = -EREMOTEIO;
		goto i2c_dma_read_error;
	}
	udelay(60);

	// invalidate cache
	invalidate_dcache_range((ulong)buf,
				(ulong)buf + roundup(len, ARCH_DMA_MINALIGN));

	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	return len;

i2c_dma_read_error:
	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	// send stop condition if transfer error
	__mtk_i2c_send_stop(dev);

	return ret;
}

static int _mtk_i2c_dma_write(struct udevice *dev, int addr_fld, u8 *buf,
			      int len, bool stop)
{
	int ret;
	int xfr_len;

	// set slave address for DMA mode
	__mtk_i2c_dma_chipaddr(dev, addr_fld);

	// enable DMA mode
	__mtk_i2c_dma_enable(dev, true);
	// request data transfer length
	__mtk_i2c_dma_setlength(dev, len);

	// set data address from I2C DMA to I2C slave
	__mtk_i2c_dma_setmiuaddr(dev, buf);
	// flush cache
	flush_dcache_range((ulong)buf,
			   (ulong)buf + roundup(len, ARCH_DMA_MINALIGN));

	// need reset dma engine before trigger dma
	__mtk_i2c_dma_rst(dev);
	// need reset dma miu before trigger dma
	__mtk_i2c_dma_rst_miu(dev);

	// direction & transfer with stop
	__mtk_i2c_dma_ctrl(dev, false, stop);
	// trigger DMA
	__mtk_i2c_dma_start(dev);

	// wait DMA done
	ret = __mtk_i2c_wait_dma_int(dev);
	if (ret)
		goto i2c_dma_write_error;

	// check actual data length
	xfr_len = __mtk_i2c_dma_get_tfr_len(dev);
	if (xfr_len != len) {
		dev_err(dev, "xfer(W) len %d, req %d\n", xfr_len, len);
		ret = -EREMOTEIO;
		goto i2c_dma_write_error;
	}
	udelay(60);

	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	return len;

i2c_dma_write_error:
	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	// send stop condition if transfer error
	__mtk_i2c_send_stop(dev);

	return ret;
}

/*
 * System APIs
 */
static int mtk_i2c_reset_bus(struct udevice *dev)
{
	return _mtk_i2c_enable_master(dev);
}

static int mtk_i2c_get_bus_speed(struct udevice *dev)
{
	return ((struct mtk_i2c_priv *)dev_get_priv(dev))->busspeed;
}

static int mtk_i2c_probe_chip(struct udevice *dev, uint chip_addr,
			      uint chip_flags)
{
	int ret = 0;
	// content of address field, include R/W bit but not value
	int addr_fld;

	if ((chip_flags & I2C_M_TEN)) {
		// 10-bit address

		// address[9:8]
		addr_fld = chip_addr & 0x0300;
		// reserve R/W bit
		addr_fld <<= 1;
		// high byte of 10-bit slave addressing 0b11110XX+(R/W)
		addr_fld |= 0xF000;
		// low byte of 10-bit slave addressing
		addr_fld |= chip_addr & 0xFF;
	} else
		// 7-bit address
		addr_fld = (chip_addr & 0x7F) << 1;

	ret = _mtk_i2c_addressing_write(dev, addr_fld, true);

	return (ret >= 0) ? 0 : -EREMOTEIO;;
}

static int mtk_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret = 0;

	dev_dbg(dev, "%s, reg %p, xfer %d msgs\n", __func__, priv->reg_base,
		nmsgs);

	for (; nmsgs > 0; nmsgs--, msg++) {
		/* address(include R/W bit) field, also for 10-bit addr. */
		int addr_fld;

		if (msg->flags & I2C_M_TEN) {
			/* 10-bit address mode */

			/* address[9:8] */
			addr_fld = msg->addr & 0x0300;
			/* reserve R/W bit */
			addr_fld <<= 1;
			/* high byte value of addressing 0b11110XX+(R/W) */
			addr_fld |= 0xF000;
			/* low byte value of addressing */
			addr_fld |= msg->addr & 0xFF;
		} else
			/* 7-bit address mode, reserve R/W bit */
			addr_fld = (msg->addr & 0x7F) << 1;

		dev_dbg(dev, "i2c_xfer: chip=0x%x, len=0x%x, buf %p\n",
			addr_fld, msg->len, msg->buf);

		/*
		 * Can not detect whether error or not in DMA mode with
		 * zero byte data.
		 */
		if ((msg->len == 0) && (msg->flags & I2C_M_RD))
			ret = _mtk_i2c_addressing_read(dev, addr_fld,
						       nmsgs == 1);
		else if (msg->len == 0)
			ret = _mtk_i2c_addressing_write(dev, addr_fld,
							nmsgs == 1);
		else if (msg->flags & I2C_M_RD)
			ret = _mtk_i2c_dma_read(dev, addr_fld, msg->buf,
						msg->len, nmsgs == 1);
		else
			ret = _mtk_i2c_dma_write(dev, addr_fld, msg->buf,
						 msg->len, nmsgs == 1);

		if (ret < 0)
			return ret;
	}

	return (ret < 0) ? ret : 0;
}

static int mtk_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);

	dev_dbg(dev, "%s, reg %p, set speed %d\n", __func__, priv->reg_base,
		speed);

	if (priv->busspeed != speed)
		return _mtk_i2c_set_clock(dev, speed);

	return 0;
}

static int mtk_i2c_probe(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret = 0;
	u8 pad_reg;

	dev_dbg(dev, "%s, reg %p\n", __func__, priv->reg_base);

	/* TODO : enable module clock */
	priv->clkspeed = 12000000;

	/* (1) Select Pad Mux */
	if (priv->reg_base == (void __iomem *)0x1F223000) {
		pad_reg = readb(CHIP_REG_HWI2C_MIIC0);
		pad_reg &= ~CHIP_MIIC0_PAD_MSK;
		pad_reg |= priv->pad_mode << 0;
		writeb(pad_reg, CHIP_REG_HWI2C_MIIC0);
	} else if (priv->reg_base == (void __iomem *)0x1F223200) {
		pad_reg = readb(CHIP_REG_HWI2C_MIIC1);
		pad_reg &= ~CHIP_MIIC1_PAD_MSK;
		pad_reg |= priv->pad_mode << 4;
		writeb(pad_reg, CHIP_REG_HWI2C_MIIC1);
	} else if (priv->reg_base == (void __iomem *)0x1F223400) {
		pad_reg = readb(CHIP_REG_HWI2C_MIIC2);
		pad_reg &= ~CHIP_MIIC2_PAD_MSK;
		pad_reg |= priv->pad_mode << 0;
		writeb(pad_reg, CHIP_REG_HWI2C_MIIC2);
	} else if (priv->reg_base == (void __iomem *)0x1F223600) {
		pad_reg = readb(CHIP_REG_HWI2C_DDCR);
		pad_reg &= ~CHIP_DDCR_PAD_MSK;
		pad_reg |= priv->pad_mode << 0;
		writeb(pad_reg, CHIP_REG_HWI2C_DDCR);
	} else if (priv->reg_base == (void __iomem *)0x1F001600) {
		pad_reg = readb(CHIP_REG_HWI2C_MIIC4);
		pad_reg &= ~CHIP_MIIC4_PAD_MSK;
		pad_reg |= priv->pad_mode << 6;
		writeb(pad_reg, CHIP_REG_HWI2C_MIIC4);
	} else {
		dev_err(dev, "Register base %p is invalid\n", priv->reg_base);
		return -EINVAL;
	}

	/* (2) Set Clock Speed */
	ret = _mtk_i2c_set_clock(dev, priv->busspeed);
	if (ret)
		return ret;

	return _mtk_i2c_enable_master(dev);
}

static int mtk_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret;

	dev_dbg(dev, "%s, priv %p\n", __func__, priv);
	/* register base address */
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;
	/* pad mode */
	ret = dev_read_s32(dev, "pad-mode", &priv->pad_mode);
	if (ret) {
		dev_err(dev, "Read 'pad-mode' from DTS fail (%d)", ret);
		return ret;
	}
	/* I2C bus speed */
	ret = dev_read_u32(dev, "clock-frequency", &priv->busspeed);
	if (ret)
		priv->busspeed = 100000;

	/* SCL and SDA count for start */
	ret = dev_read_u32(dev, "start-count", &priv->start_cnt);
	if ((ret) || priv->start_cnt > 0xFFFF)
		priv->start_cnt = 73;

	/* closk count between falling edge SCL and SDA */
	ret = dev_read_u32(dev, "sda-count", &priv->sda_cnt);
	if ((ret) || priv->sda_cnt > 0xFFFF)
		priv->sda_cnt = 5;

	/* data latch timing */
	ret = dev_read_u32(dev, "data-latch-count", &priv->data_lat_cnt);
	if ((ret) || priv->start_cnt > 0xFFFF)
		priv->data_lat_cnt = 5;

	/* SCL and SDA count for stop */
	ret = dev_read_u32(dev, "stop-count", &priv->stop_cnt);
	if ((ret) || priv->stop_cnt > 0xFFFF)
		priv->stop_cnt = 73;

	/* scl stretch */
	priv->scl_stretch = dev_read_bool(dev, "scl-stretch");

	return 0;
}

static const struct dm_i2c_ops mtk_i2c_ops = {
	.xfer = mtk_i2c_xfer,
	.probe_chip = mtk_i2c_probe_chip,
	.set_bus_speed = mtk_i2c_set_bus_speed,
	.get_bus_speed = mtk_i2c_get_bus_speed,
	.deblock = mtk_i2c_reset_bus,
};

static const struct udevice_id mtk_i2c_of_match[] = {
	{.compatible = "mediatek,mtk-dtv-i2c"},
	{}
};

U_BOOT_DRIVER(mtk_dtv_i2c) = {
	.name = "mediatek-i2c",
	.id = UCLASS_I2C,
	.of_match = mtk_i2c_of_match,
	.ofdata_to_platdata = mtk_ofdata_to_platdata,
	.probe = mtk_i2c_probe,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),
	.priv_auto_alloc_size = sizeof(struct mtk_i2c_priv),
	.ops = &mtk_i2c_ops,
};

