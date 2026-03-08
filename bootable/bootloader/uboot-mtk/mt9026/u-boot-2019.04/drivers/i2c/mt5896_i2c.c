// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */

#include <common.h>
#include <dm.h>
#include <linux/io.h>
#include <i2c.h>

#include "mt5896_i2c.h"

#define MIU0_BUS_BASE			(0x20000000)
#define MIU1_BUS_BASE			(0xA0000000)
#define HWI2C_HAL_WAIT_TIMEOUT		(50000)

#define HWI2C_SCL_DIVISOR_BASE		(6)

#define HWI2C_SLAVE_ADDR_HIGH_MSK	(0xFF00)

#define HWI2C_TICKS_PER_US		(12)
#define HWI2C_START_DELAY_US		(5)
#define HWI2C_STD_MODE_TSUSDA_EXT_DELAY_US	(2)

typedef enum _HAL_HWI2C_STATE {
	E_HAL_HWI2C_STATE_IDEL = 0,
	E_HAL_HWI2C_STATE_START,
	E_HAL_HWI2C_STATE_WRITE,
	E_HAL_HWI2C_STATE_READ,
	E_HAL_HWI2C_STATE_INT,
	E_HAL_HWI2C_STATE_WAIT,
	E_HAL_HWI2C_STATE_STOP
} HAL_HWI2C_STATE;

#define KHZ(freq)		(1000 * freq)

#define I2C_STD_MODE_SPEED	KHZ(100)

struct mtk_i2c_clk_fld {
	u32 clk_freq_out;
	u8 t_high_cnt;
	u8 t_low_cnt;
};

/**
 * struct mtk_i2c_priv - i2c private data
 *
 * @reg_base: register bank base address
 * @clkspeed: IP source clock rate
 * @busspeed: I2C bus speed
 * @ext_clk_fld: extended bus speed register setting
 * @stop_cnt: SCL and SDA count for stop
 * @sda_cnt: closk count between falling edge SCL and SDA
 * @start_cnt: SCL and SDA count for start
 * @data_lat_cnt: data latch timing
 * @byte_ext_dly_cnt: byte-to-byte extra delay count
 * @fifo_depth: I2C IP internal fifo depth
 * @scl_stretch: enable/disable clock stretch
 * @push_disable: disable pull-up
 * @support_dma: hw dma capability
 */
struct mtk_i2c_priv {
	void __iomem *reg_base;
	u32 clkspeed;
	u32 busspeed;
	struct mtk_i2c_clk_fld ext_clk_fld;
	u32 stop_cnt;
	u32 sda_cnt;
	u32 start_cnt;
	u32 data_lat_cnt;
	u32 byte_ext_dly_cnt;
	u32 fifo_depth;
	bool scl_stretch;
	bool push_disable;
	bool support_dma;
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

	dev_dbg(dev, "%s: set value 0x%02X\n", __func__, val);
}

static inline void __mtk_i2c_dma_enable_internal_buf(struct udevice *dev,
						     bool enable)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	u8 val;

	val = readb(priv->reg_base + REG_HWI2C_DMA_CFG);
	val &= ~_DMA_CFG_DMA_USE_REG;
	if (enable)
		val |= _DMA_CFG_DMA_USE_REG;
	writeb(val, priv->reg_base + REG_HWI2C_DMA_CFG);

	dev_dbg(dev, "%s: set value 0x%02X\n", __func__, val);
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
	if ((addr_fld & HWI2C_SLAVE_ADDR_HIGH_MSK)) {
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
	u64 addr;

	if (!buf)
		addr = 0;
	else
		addr = virt_to_phys(buf) - MIU0_BUS_BASE;

	dev_dbg(dev, "%s: set buf 0x%016llX\n", __func__, addr);

	writew(addr & 0xFFFF, priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B0_B7);
	writew((addr >> 16) & 0xFFFF,
	       priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B16_B23);
	writeb((addr >> 32) & 0xFF,
	       priv->reg_base + REG_HWI2C_DMA_MIU_ADR_B32_B39);
}

static inline void __mtk_i2c_dma_fill_internal_buf(struct udevice *dev,
						   u8 *data, int length)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int i;
	u16 val;

	for (i = 0 ; i < length ; i += 2) {
		val = (u16)(*data) | ((u16)(*(data + 1)) << 8);
		writew(val, priv->reg_base + REG_HWI2C_DMA_WBUF00 +
			    ((i >> 1) * 4));
		dev_dbg(dev, "i2c_dma_int_buf:(%02d) 0x%04X\n", i, val);
		data += 2;
	}
}

static inline void __mtk_i2c_dma_get_internal_buf(struct udevice *dev, u8 *data,
						  int length)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int i;
	u16 val;

	for (i = 0 ; i < length ; i += 2) {
		val = readw(priv->reg_base + REG_HWI2C_DMA_RBUF00 +
			    ((i >> 1) * 4));
		dev_dbg(dev, "i2c_dma_int_buf:(%02d) 0x%04X\n", i, val);

		*data = val & 0xFF;
		data++;

		if (i == length - 1)
			break;

		*data = val >> 8;
		data++;
	}
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

static void __mtk_i2c_rst(struct udevice *dev)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int val;

	/* reset standard master iic */
	val = readb(priv->reg_base + REG_HWI2C_MIIC_CFG);
	writeb(val | _MIIC_CFG_RESET, priv->reg_base + REG_HWI2C_MIIC_CFG);
	writeb(val & ~_MIIC_CFG_RESET, priv->reg_base + REG_HWI2C_MIIC_CFG);

	/* reset DMA engine */
	__mtk_i2c_dma_rst(dev);
	/* reset MIU module in DMA engine */
	__mtk_i2c_dma_rst_miu(dev);

	/* clear interrupt */
	__mtk_i2c_clear_int(dev);
	/* clear dma interrupt */
	__mtk_i2c_clear_dma_done(dev);

	/* confirm SDA/SCL is return to high */
	val = readw(priv->reg_base + REG_HWI2C_SCL_SDA_IO);

	if (!(val & _SCLI))
		dev_err(dev, "SCL not high after bus reset!\n");

	if (!(val & _SCLO))
		dev_err(dev, "SCL output not high after bus reset!\n");

	if (!(val & _SDAI))
		dev_err(dev, "SDA not high after bus reset!\n");
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

static int __mtk_i2c_get_byte(struct udevice *dev, u8 *ptr, bool ack_data)
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
	val |=  (priv->push_disable) ? 0 : _MIIC_CFG_EN_PUSH1T;
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

	/* DMA mode byte-to-byte delay, default 0 */
	val = readb(priv->reg_base + REG_HWI2C_RESERVE0);
	writeb(val | _BYTE2BYTE_DLY, priv->reg_base + REG_HWI2C_RESERVE0);
	writew(priv->byte_ext_dly_cnt, priv->reg_base + REG_HWI2C_BYTE2BYTE_CNT);

	return 0;
}

/*
 * Hardware uses the underlying formula to calculate time periods of
 * SCL clock cycle. Firmware uses some additional cycles excluded from the
 * below formula and it is confirmed that the time periods are within
 * specification limits.
 *
 * time of high period of SCL: t_high = (t_high_cnt + 3) / source_clock
 * time of low period of SCL: t_low = (t_low_cnt + 3) / source_clock
 * source_clock = 12 MHz
 */
static const struct mtk_i2c_clk_fld mtk_i2c_clk_map[] = {
	{KHZ(25), 235, 237},
	{KHZ(50), 115, 117},
	{KHZ(100), 57, 59},
	{KHZ(200), 25, 27},
	{KHZ(300), 15, 17},
	{KHZ(400), 11, 13},
};

static int _mtk_i2c_set_clock(struct udevice *dev, int speed)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int divider = priv->clkspeed / speed;
	int clk_high_cnt, clk_low_cnt;
	int i;
	const struct mtk_i2c_clk_fld *itr = mtk_i2c_clk_map;

	dev_dbg(dev, "%s, reg %p, set bus speed %d\n", __func__, priv->reg_base,
		speed);

	if ((priv->clkspeed % speed) != 0)
		divider++;

	if (divider < (priv->sda_cnt + priv->data_lat_cnt + HWI2C_SCL_DIVISOR_BASE)) {
		dev_err(dev, "set i2c bus %s bus speed %d fail\n", dev->name,
			speed);
		return -EINVAL;
	}

	divider -= HWI2C_SCL_DIVISOR_BASE;
	clk_high_cnt = divider / 2;
	clk_low_cnt = divider - clk_high_cnt;

	/* try use traversed mtk_i2c_clk_map to overwrite calculation result. */
	for (i = 0; i < ARRAY_SIZE(mtk_i2c_clk_map); i++, itr++)
		if (itr->clk_freq_out == speed) {
			clk_high_cnt = itr->t_high_cnt;
			clk_low_cnt = itr->t_low_cnt;
			break;
		}

	/* try use customize SCL timing setting(in dts) to overwrite calculation result. */
	if (priv->ext_clk_fld.clk_freq_out == speed) {

		clk_high_cnt = priv->ext_clk_fld.t_high_cnt;
		clk_low_cnt = priv->ext_clk_fld.t_low_cnt;
	}

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
static int _mtk_i2c_nostart_data(struct udevice *dev, bool start, struct i2c_msg *pmsg,
				 bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int i;
	u8 *ptr = pmsg->buf;

	// only riu mode can skip start sequence

	if (start) {
		/*
		 * According Documentation/i2c/i2c-protocol.rst
		 * If you set the I2C_M_NOSTART variable for the first partial message,
		 * we do not generate Addr, but we do generate the startbit S. This will
		 * probably confuse all other clients on your bus, so don't try this.
		 */
		if (__mtk_i2c_send_start(dev))
			goto i2c_nostart_err;

		udelay(HWI2C_START_DELAY_US);
	}

	if (pmsg->flags & I2C_M_RD) {
		for (i = 0; i < pmsg->len; i++, ptr++) {
			if (__mtk_i2c_get_byte(dev, ptr, i < (pmsg->len - 1)))
				goto i2c_nostart_err;
			if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
				udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);
		}
	} else {
		for (i = 0; i < pmsg->len; i++, ptr++) {
			if (__mtk_i2c_send_byte(dev, *ptr)) {
				dev_err(dev, "data #%d failed\n", i);
				goto i2c_nostart_err;
			}
			if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
				udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);
		}
	}

	if (stop)
		__mtk_i2c_send_stop(dev);

	return pmsg->len;

i2c_nostart_err:
	__mtk_i2c_send_stop(dev);
	__mtk_i2c_rst(dev);
	return -EREMOTEIO;
}

static int _mtk_i2c_read_data(struct udevice *dev, int addr_fld, u8 *buf,
			      int len, bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int i;
	u8 *ptr = buf;
	u8 data_stage_addr;

	/*
	 * addressing stage
	 */
	if ((addr_fld & HWI2C_SLAVE_ADDR_HIGH_MSK)) {
		// 10-bit address
		if (__mtk_i2c_send_start(dev)) {
			dev_err(dev, "%s Start cond(addr) failed\n", __func__);
			goto i2c_read_err;
		}
		udelay(HWI2C_START_DELAY_US);

		// slave addr 1st 7 bits
		if (__mtk_i2c_send_byte(dev, (addr_fld >> 8))) {
			dev_dbg(dev, "%s Chip addr(high) failed\n", __func__);
			goto i2c_read_err;
		}
		if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
			udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);

		// slave addr 2nd byte
		if (__mtk_i2c_send_byte(dev, (addr_fld & 0xFF))) {
			dev_dbg(dev, "%s Chip addr(low) failed\n", __func__);
			goto i2c_read_err;
		}
		if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
			udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);

		data_stage_addr = (addr_fld >> 8) | 1;
	} else
		// 7-bit address (no addressing stage)
		data_stage_addr = (addr_fld & 0xFF) | 1;

	/*
	 * data reveiving stage
	 */
	if (__mtk_i2c_send_start(dev)) {
		dev_err(dev, "%s Start cond failed\n", __func__);
		goto i2c_read_err;
	}
	udelay(HWI2C_START_DELAY_US);

	if (__mtk_i2c_send_byte(dev, data_stage_addr)) {
		dev_dbg(dev, "%s Chip addr(data) failed\n", __func__);
		goto i2c_read_err;
	}
	if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
		udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);

	for (i = 0; i < len; i++, ptr++) {
		if (__mtk_i2c_get_byte(dev, ptr, i < (len - 1))) {
			dev_dbg(dev, "%s Data stage failed\n", __func__);
			goto i2c_read_err;
		}
		if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
			udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);
	}

	if (stop)
		__mtk_i2c_send_stop(dev);

	return len;

i2c_read_err:
	__mtk_i2c_send_stop(dev);
	__mtk_i2c_rst(dev);
	return -EREMOTEIO;
}

static int _mtk_i2c_write_data(struct udevice *dev, int addr_fld, u8 *buf,
			       int len, bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int i;
	u8 *ptr = buf;

	/*
	 * addressing stage
	 */
	if (__mtk_i2c_send_start(dev)) {
		dev_err(dev, "%s Start cond failed\n", __func__);
		goto i2c_write_err;
	}
	udelay(HWI2C_START_DELAY_US);

	if ((addr_fld & HWI2C_SLAVE_ADDR_HIGH_MSK)) {
		// 10-bit address, slave addr 1st 7 bits
		if (__mtk_i2c_send_byte(dev, (addr_fld >> 8))) {
			dev_dbg(dev, "%s Chip addr(high) failed\n", __func__);
			goto i2c_write_err;
		}
		if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
			udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);
	}

	// slave addr 2nd byte or 7-bit address
	if (__mtk_i2c_send_byte(dev, (addr_fld & 0xFF))) {
		dev_dbg(dev, "%s Chip addr(low) failed\n", __func__);
		goto i2c_write_err;
	}
	if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
		udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);

	/*
	 * data sending stage
	 */
	for (i = 0; i < len; i++, ptr++) {
		if (__mtk_i2c_send_byte(dev, *ptr)) {
			dev_dbg(dev, "%s Data stage failed\n", __func__);
			goto i2c_write_err;
		}
		if (!!(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US))
			udelay(priv->byte_ext_dly_cnt / HWI2C_TICKS_PER_US);
	}

	if (stop)
		__mtk_i2c_send_stop(dev);

	return len;

i2c_write_err:
	__mtk_i2c_send_stop(dev);
	__mtk_i2c_rst(dev);
	return -EREMOTEIO;
}

static int _mtk_i2c_dma_read(struct udevice *dev, int addr_fld, u8 *buf,
			     int len, bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret;
	int xfr_len;

	if ((addr_fld & HWI2C_SLAVE_ADDR_HIGH_MSK)) {
		// 10-bit address, addressing sequence the same as write with
		// 0 byte data w/o stop condition
		ret = _mtk_i2c_write_data(dev, addr_fld, NULL, 0, false);
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
	// DMA buffer use internal buffer or DRAM
	__mtk_i2c_dma_enable_internal_buf(dev, (len <= priv->fifo_depth));
	// request data transfer length
	__mtk_i2c_dma_setlength(dev, len);

	if (len <= priv->fifo_depth) {
		// use internal buffer, set data address to 0
		__mtk_i2c_dma_setmiuaddr(dev, NULL);
	} else {
		// set data address from I2C slave to I2C DMA
		__mtk_i2c_dma_setmiuaddr(dev, buf);
		invalidate_dcache_range((ulong)buf,
					(ulong)buf +
					roundup(len, ARCH_DMA_MINALIGN));
	}

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

	if (len <= priv->fifo_depth)
		// get data from DMA internal buffer
		__mtk_i2c_dma_get_internal_buf(dev, buf, len);
	else {
		// invalidate cache
		invalidate_dcache_range((ulong)buf,
					(ulong)buf +
					    roundup(len, ARCH_DMA_MINALIGN));
	}

	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	return len;

i2c_dma_read_error:
	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);
	// reset standard master iic
	__mtk_i2c_rst(dev);

	return ret;
}

static int _mtk_i2c_dma_write(struct udevice *dev, int addr_fld, u8 *buf,
			      int len, bool stop)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret;
	int xfr_len;

	// set slave address for DMA mode
	__mtk_i2c_dma_chipaddr(dev, addr_fld);

	// enable DMA mode
	__mtk_i2c_dma_enable(dev, true);
	// DMA buffer use internal or DRAM
	__mtk_i2c_dma_enable_internal_buf(dev, (len <= priv->fifo_depth));
	// request data transfer length
	__mtk_i2c_dma_setlength(dev, len);

	if (len <= priv->fifo_depth) {
		// use DMA internal buffer
		// set data address to 0
		__mtk_i2c_dma_setmiuaddr(dev, NULL);
		// fill data to DMA internal buffer
		__mtk_i2c_dma_fill_internal_buf(dev, buf, len);
	} else {
		// use DRAM as DMA buffer
		// set data address from I2C DMA to I2C slave
		__mtk_i2c_dma_setmiuaddr(dev, buf);
		// flush cache
		flush_dcache_range((ulong)buf,
				   (ulong)buf +
				       roundup(len, ARCH_DMA_MINALIGN));
	}

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

	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);

	return len;

i2c_dma_write_error:
	// disable DMA mode
	__mtk_i2c_dma_enable(dev, false);
	// reset standard master iic
	__mtk_i2c_rst(dev);

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

	ret = _mtk_i2c_write_data(dev, addr_fld, NULL, 0, true);

	return (ret >= 0) ? 0 : -EREMOTEIO;;
}

static int mtk_i2c_xfer(struct udevice *dev, struct i2c_msg msgs[], int nmsgs)
{
	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret = 0;
	int i;

	dev_dbg(dev, "%s, reg %p, xfer %d msgs\n", __func__, priv->reg_base,
		nmsgs);

	for (i = 0; i < nmsgs; i++) {
		/* address(include R/W bit) field, also for 10-bit addr. */
		int addr_fld;
		struct i2c_msg *msg = &msgs[i];
		bool stop = (i == (nmsgs - 1));

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

		/*
		 * do Read/Write, NOTICE: place stop at the end of the last msg
		 * use different transfer mode according data length
		 *
		 * NOTE: Due to tSU:STA of I2C standard mode(bus speed 100KHz), place I2C module
		 * reset make SDA high before next i2c_msg when necessary.
		 */
		if (i > 0 && !(msg->flags & I2C_M_NOSTART)) {
			dev_dbg(dev, "tSU:SDA workaround reset\n");
			__mtk_i2c_rst(dev);
			if (priv->busspeed <= I2C_STD_MODE_SPEED)
				// only needs extra delay in standard mode
				udelay(HWI2C_STD_MODE_TSUSDA_EXT_DELAY_US);
		}

		dev_dbg(dev, "i2c_xfer: chip=0x%x, len=0x%x, buf %p\n",
			addr_fld, msg->len, msg->buf);

		/*
		 * 1. Can not detect whether error or not in DMA mode with
		 *    zero byte data.
		 * 2. PM_I2C DMA engine can not access DRAM
		 */
		if (msg->flags & I2C_M_NOSTART)
			// special case for "NOSTART"
			ret = _mtk_i2c_nostart_data(dev, i == 0, msg, stop);
		else if ((msg->len == 0) && (msg->flags & I2C_M_RD))
			ret = _mtk_i2c_read_data(dev, addr_fld, NULL, 0,
						 stop);
		else if (msg->len == 0)
			ret = _mtk_i2c_write_data(dev, addr_fld, NULL, 0,
						  stop);
		else if ((msg->len <= priv->fifo_depth) &&
			 (msg->flags & I2C_M_RD))
			ret = _mtk_i2c_dma_read(dev, addr_fld, msg->buf,
						msg->len, stop);
		else if (msg->len <= priv->fifo_depth)
			ret = _mtk_i2c_dma_write(dev, addr_fld, msg->buf,
						 msg->len, stop);
		else if ((!priv->support_dma) && (msg->flags & I2C_M_RD))
			ret = _mtk_i2c_read_data(dev, addr_fld, msg->buf,
						 msg->len, stop);
		else if (!priv->support_dma)
			ret = _mtk_i2c_write_data(dev, addr_fld, msg->buf,
						  msg->len, stop);
		else if (msg->flags & I2C_M_RD)
			ret = _mtk_i2c_dma_read(dev, addr_fld, msg->buf,
						msg->len, stop);
		else
			ret = _mtk_i2c_dma_write(dev, addr_fld, msg->buf,
						 msg->len, stop);

		if (ret < 0)
			break;
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
	u16 reg_val;
	int clk_reg_count;
	int i;
	u32 *clk_reg_setting;

	dev_dbg(dev, "%s, reg %p\n", __func__, priv->reg_base);

	/* enable clock mux & sw_en */
	/* get property "module-clocks" size in dts */
	clk_reg_count = dev_read_size(dev, "module-clocks");
	if (clk_reg_count > 0) {
		clk_reg_setting = malloc(clk_reg_count);
		if (!clk_reg_setting)
			return -ENOMEM;

		/* get property "module-clocks" data */
		clk_reg_count /= sizeof(u32);
		ret = dev_read_u32_array(dev, "module-clocks", clk_reg_setting,
					 clk_reg_count);
		if (ret) {
			free(clk_reg_setting);
			return ret;
		}

		/* traverse all "module-clocks" setting */
		/* "module-clocks" format: addr, mask, value */
		clk_reg_count /= 3;
		for (i = 0 ; i < clk_reg_count ; i++) {
			dev_dbg(dev, "module clks, addr %X, msk %X, val %X\n",
				clk_reg_setting[i*3], clk_reg_setting[i*3+1],
				clk_reg_setting[i*3+2]);
			reg_val = readw((u64)(clk_reg_setting[i*3]));
			reg_val &= ~clk_reg_setting[i*3+1];
			reg_val |= clk_reg_setting[i*3+2];
			writew(reg_val, (u64)(clk_reg_setting[i*3]));
		}

		free(clk_reg_setting);
	}

	priv->clkspeed = 12000000;

	/* Set bus scl Speed */
	ret = _mtk_i2c_set_clock(dev, priv->busspeed);
	if (ret)
		return ret;

	return _mtk_i2c_enable_master(dev);
}

static int mtk_ofdata_to_platdata(struct udevice *dev)
{
#define EXT_SCL_FREQ_IDX	0
#define EXT_SCL_HIGH_IDX	1
#define EXT_SCL_LOW_IDX		2

	struct mtk_i2c_priv *priv = dev_get_priv(dev);
	int ret;
	u32 ext_scl_setting[] = { 0, 0, 0 };

	dev_dbg(dev, "%s, priv %p\n", __func__, priv);
	/* register base address */
	priv->reg_base = (void __iomem *)dev_read_addr(dev);
	if (priv->reg_base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

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
	if ((ret) || priv->data_lat_cnt > 0xFFFF)
		priv->data_lat_cnt = 5;

	/* SCL and SDA count for stop */
	ret = dev_read_u32(dev, "stop-count", &priv->stop_cnt);
	if ((ret) || priv->stop_cnt > 0xFFFF)
		priv->stop_cnt = 73;

	/* scl stretch */
	priv->scl_stretch = dev_read_bool(dev, "scl-stretch");

	/* restrict dma engine access dram if hardware not allow */
	priv->support_dma = dev_read_bool(dev, "support-dma");

	/* dma bounce buffer size */
	ret = dev_read_u32(dev, "fifo-depth", &priv->fifo_depth);
	if (ret)
		priv->fifo_depth = 0;
	else if (priv->fifo_depth > HWI2C_BUF_SIZE_MAX)
		priv->fifo_depth = HWI2C_BUF_SIZE_MAX;

	/* pull-up */
	priv->push_disable = dev_read_bool(dev, "push-disable");

	/* customize SCL count */
	if (dev_read_u32_array(dev, "speed-scl-count", ext_scl_setting,
			       ARRAY_SIZE(ext_scl_setting))) {
		priv->ext_clk_fld.clk_freq_out = U32_MAX;
		priv->ext_clk_fld.t_high_cnt = U8_MAX;
		priv->ext_clk_fld.t_low_cnt = U8_MAX;
	} else {
		priv->ext_clk_fld.clk_freq_out = ext_scl_setting[EXT_SCL_FREQ_IDX];
		priv->ext_clk_fld.t_high_cnt = ext_scl_setting[EXT_SCL_HIGH_IDX] & U8_MAX;
		priv->ext_clk_fld.t_low_cnt = ext_scl_setting[EXT_SCL_LOW_IDX] & U8_MAX;
	}

	return 0;
}

static int mtk_i2c_bind(struct udevice *dev)
{
	u32 prop = 0;

	if ((dev->seq == -1) && (dev->req_seq == -1)) {
		// assign dev->req_seq
		if (dev_read_u32(dev, "bus-number", &prop)) {
			// no bus-number property in dts
			dev_warn(dev, "no bus-number property in dts for device '%s'\n",
				 dev->name);
			dev->seq = uclass_resolve_seq(dev);
		} else
			dev->req_seq = (int)prop;
	}

	dev_dbg(dev, "assign device seq %d req_seq %d\n", dev->seq, dev->req_seq);

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
	{.compatible = "mediatek,mt5896-i2c"},
	{}
};

U_BOOT_DRIVER(mt5896_i2c) = {
	.name = "mediatek-i2c",
	.id = UCLASS_I2C,
	.of_match = mtk_i2c_of_match,
	.ofdata_to_platdata = mtk_ofdata_to_platdata,
	.bind = mtk_i2c_bind,
	.probe = mtk_i2c_probe,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),
	.priv_auto_alloc_size = sizeof(struct mtk_i2c_priv),
	.ops = &mtk_i2c_ops,
};

