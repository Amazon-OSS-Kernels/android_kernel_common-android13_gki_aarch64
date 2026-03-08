// SPDX-License-Identifier: GPL-2.0+
/*
 * MediaTek Inc. (C) 2020. All rights reserved.
 */
#include <common.h>
#include <dm.h>
#include <spi.h>
#include <wait_bit.h>
#include <linux/io.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>

static bool isCsGPIO = false;

/* 3 register set per clock mux selection, move 1 mandatory clk mux  to clock enable */
#define MSPI_MOUDLE_REG_SETS_PER_CLKMUX	(2)
#define MSPI_MOUDLE_CLKMUX_SETS		(9) /* 9 clock source for MSPI */

#define MSPI_XFER_TIMEOUT_MS		(10)

#define MAX_MSPI_DIV			(15) /* maximun module clock divider */
/* separate register for two SPI controller clock divider, need nibble mask and shift */
#define MSPI_DIV_EVEN_MSK		(0x0F)
#define MSPI_DIV_ODD_MSK		(0xF0)
#define MSPI_DIV_ODD_SFT		(4)

#define SPI_FDX_MSG_SIZE_MAX		16 /* SPI full duplex message chunk size */
#define SPI_HDX_MSG_SIZE_MAX		32 /* SPI half duplex message chunk size */
#define SPI_BIT_LEN_FULL		BITS_PER_BYTE /* SPI MAX BITS PER WORD */
#define SPI_MAX_TIME_DELAY_TICKS	255 /* SPI MAX delay clk ticks of timing setting */

#define SPI_TRANS_BUSY			BIT(0)
#define MSPI_ENABLE_BIT			BIT(0)
#define MSPI_RESET_BIT			BIT(1)
#define MSPI_ENABLE_INT_BIT		BIT(2)
#define MSPI_3WARE_MODE_BIT		BIT(4)
#define MSPI_CPHA_BIT			BIT(6)
#define MSPI_CPOL_BIT			BIT(7)
#define MSPI_LSB_FIRST_BIT		BIT(0)
#define MSPI_TRIGGER_BIT		BIT(0)
#define MSPI_CLEAR_DONE_FLAG_BIT	BIT(0)
#define MSPI_CHIP_SELECT_BIT		BIT(0)
#define MSPI_ENABLE_DUAL		BIT(0)
#define WB16_INDEX			0x4
#define WB8_INDEX			0x3
#define MAX_TX_BUF_SIZE			0x20
#define MAX_SCLK_DIV			(7)
#define BYTE_SHIFT				(8)

#define TR_BUF_EXT_IDX			2
#define TR_BUF_EXT_CNT			6
#define SPI_OFFSET_IDX			4
#define SPI_BIT_MUX_LEN			3
#define SPI_BIT_MUX_PER_REG		4
#define SPI_BIT_MUX_BITS		3
#define SPI_BIT_MUX_IDX0		0
#define SPI_BIT_MUX_IDX1		1
#define SPI_BIT_MUX_IDX2		2
#define SPI_BIT_MUX_IDX3		3

#define SPI_REG_0000_MSPI0		(0x0000)
#define SPI_REG_0040_MSPI0		(0x0040)
#define SPI_REG_0080_MSPI0		(0x0080)
#define SPI_REG_00A0_MSPI0		(0x00A0)
#define SPI_REG_00E0_MSPI0		(0x00E0)
#define SPI_REG_0100_MSPI0		(0x0100)
#define SPI_REG_0104_MSPI0		(0x0104)
#define SPI_REG_0108_MSPI0		(0x0108)
#define SPI_REG_010C_MSPI0		(0x010C)
#define SPI_REG_0004_MSPI0		(0x0004)
#define SPI_REG_0008_MSPI0		(0x0008)
#define SPI_REG_000C_MSPI0		(0x000C)
#define SPI_REG_0010_MSPI0		(0x0010)
#define SPI_REG_0014_MSPI0		(0x0014)
#define SPI_REG_0018_MSPI0		(0x0018)
#define SPI_REG_001C_MSPI0		(0x001C)
#define SPI_REG_0020_MSPI0		(0x0020)
#define SPI_REG_0024_MSPI0		(0x0024)
#define SPI_REG_0028_MSPI0		(0x0028)
#define SPI_REG_002C_MSPI0		(0x002C)


#define SPI_REG_0110_MSPI0		(0x0110)
#define SPI_REG_0114_MSPI0		(0x0114)
#define SPI_REG_0118_MSPI0		(0x0118)
#define SPI_REG_011C_MSPI0		(0x011C)
#define SPI_REG_0044_MSPI0		(0x0044)
#define SPI_REG_0048_MSPI0		(0x0048)
#define SPI_REG_004C_MSPI0		(0x004C)
#define SPI_REG_0050_MSPI0		(0x0050)
#define SPI_REG_0054_MSPI0		(0x0054)
#define SPI_REG_0058_MSPI0		(0x0058)
#define SPI_REG_005C_MSPI0		(0x005C)
#define SPI_REG_0060_MSPI0		(0x0060)
#define SPI_REG_0064_MSPI0		(0x0064)
#define SPI_REG_0068_MSPI0		(0x0068)
#define SPI_REG_006C_MSPI0		(0x006C)


#define SPI_REG_0120_MSPI0		(0x0120)
#define SPI_REG_0124_MSPI0		(0x0124)
#define SPI_REG_0128_MSPI0		(0x0128)
#define SPI_REG_012C_MSPI0		(0x012C)
#define SPI_REG_0130_MSPI0		(0x0130)
#define SPI_REG_0134_MSPI0		(0x0134)
#define SPI_REG_0138_MSPI0		(0x0138)
#define SPI_REG_013C_MSPI0		(0x013C)
#define SPI_REG_0140_MSPI0		(0x0140)
#define SPI_REG_0170_MSPI0		(0x0170)
#define SPI_REG_017C_MSPI0		(0x017C)
#define SPI_REG_016C_MSPI0		(0x016C)
#define SPI_REG_0168_MSPI0		(0x0168)
#define SPI_REG_01E0_MSPI0		(0x01E0)

static const u16 mspi_txfifoaddr[] = {
	SPI_REG_0100_MSPI0,
	SPI_REG_0104_MSPI0,
	SPI_REG_0108_MSPI0,
	SPI_REG_010C_MSPI0,
	SPI_REG_0000_MSPI0,
	SPI_REG_0004_MSPI0,
	SPI_REG_0008_MSPI0,
	SPI_REG_000C_MSPI0,
	SPI_REG_0010_MSPI0,
	SPI_REG_0014_MSPI0,
	SPI_REG_0018_MSPI0,
	SPI_REG_001C_MSPI0,
	SPI_REG_0020_MSPI0,
	SPI_REG_0024_MSPI0,
	SPI_REG_0028_MSPI0,
	SPI_REG_002C_MSPI0,
};

static const u16 mspi_rxfifoaddr_halfdeplex[] = {
	SPI_REG_0110_MSPI0,
	SPI_REG_0114_MSPI0,
	SPI_REG_0118_MSPI0,
	SPI_REG_011C_MSPI0,
	SPI_REG_0040_MSPI0,
	SPI_REG_0044_MSPI0,
	SPI_REG_0048_MSPI0,
	SPI_REG_004C_MSPI0,
	SPI_REG_0050_MSPI0,
	SPI_REG_0054_MSPI0,
	SPI_REG_0058_MSPI0,
	SPI_REG_005C_MSPI0,
	SPI_REG_0060_MSPI0,
	SPI_REG_0064_MSPI0,
	SPI_REG_0068_MSPI0,
	SPI_REG_006C_MSPI0,
};

struct spi_clk_mux_en_setting {
	u32 clk_reg;
	u32 clk_reg_msk;
	u32 clk_reg_val;
};

struct spi_clk_mux {
	u32 speed;
	struct spi_clk_mux_en_setting clkmux_set[MSPI_MOUDLE_REG_SETS_PER_CLKMUX];
};

struct mt5896_spi {
	struct udevice *dev;
	void __iomem *base;
	struct spi_clk_mux clkmux[MSPI_MOUDLE_CLKMUX_SETS];
	unsigned int sys_freq;
	unsigned int xfer_bitmux[SPI_HDX_MSG_SIZE_MAX];
	unsigned int xfer_trig_d;
	unsigned int xfer_interval;
	unsigned int xfer_done_d;
	unsigned int xfer_turn_d;
	u32 clkdiv_reg;
	u32 clkdiv_msk;
	u8 *tx_buf;
	u8 *rx_buf;
	u8 flags;
	struct gpio_desc gpio_cs;
};

static inline u16 dtvspi_rd(struct mt5896_spi *bs, u32 reg)
{
	return readw((bs->base + reg));
}

static inline u8 dtvspi_rdl(struct mt5896_spi *bs, u16 reg)
{
	return dtvspi_rd(bs, reg)&0xff;
}
static inline void dtvspi_wr(struct mt5896_spi *bs, u16 reg, u32 val)
{
	writew(val, (bs->base + reg));
}

static inline void dtvspi_wrl(struct mt5896_spi *bs, u16 reg, u8 val)
{
	u16 val16 = dtvspi_rd(bs, reg)&0xff00;

	val16 |= val;
	dtvspi_wr(bs, reg, val16);
}

static inline void dtvspi_wrh(struct mt5896_spi *bs, u16 reg, u8 val)
{
	u16 val16 = dtvspi_rd(bs, reg)&0xff;

	val16 |= ((u16)val) << 8;
	dtvspi_wr(bs, reg, val16);
}

static void dtvspi_hw_set_cus(struct mt5896_spi *bs)
{
	u16 reg[SPI_HDX_MSG_SIZE_MAX / SPI_BIT_MUX_PER_REG] = {0};
	int msg_interval = 0;
	int addr_tmp;
	uint i;

	/* generate register value mask */
	for (i = 0; i < SPI_HDX_MSG_SIZE_MAX; i += SPI_BIT_MUX_PER_REG) {
		int bits = 0;

		bits = bs->xfer_bitmux[i + SPI_BIT_MUX_IDX0] - 1;
		bits |= (bs->xfer_bitmux[i + SPI_BIT_MUX_IDX1] - 1)
			<< (SPI_BIT_MUX_IDX1 * SPI_BIT_MUX_BITS);
		bits |= (bs->xfer_bitmux[i + SPI_BIT_MUX_IDX2] - 1)
			<< (SPI_BIT_MUX_IDX2 * SPI_BIT_MUX_BITS);
		bits |= (bs->xfer_bitmux[i + SPI_BIT_MUX_IDX3] - 1)
			<< (SPI_BIT_MUX_IDX3 * SPI_BIT_MUX_BITS);

		reg[i / SPI_BIT_MUX_PER_REG] = bits & U16_MAX;
	}

	/* write registers */
	/* word 0~7 of write */
	dtvspi_wr(bs, SPI_REG_0130_MSPI0, reg[0]);
	dtvspi_wr(bs, SPI_REG_0134_MSPI0, reg[1]);
	/* word 0~7 of read */
	dtvspi_wr(bs, SPI_REG_0138_MSPI0, reg[0]);
	dtvspi_wr(bs, SPI_REG_013C_MSPI0, reg[1]);
	/* word 8~31 of write */
	addr_tmp = SPI_REG_0080_MSPI0;
	for (i = 0; i < TR_BUF_EXT_CNT; i++) {
		dtvspi_wr(bs, addr_tmp, reg[i + TR_BUF_EXT_IDX]);
		addr_tmp += SPI_OFFSET_IDX;
	}
	/* word 8~31 of read */
	addr_tmp = SPI_REG_00A0_MSPI0;
	for (i = 0; i < TR_BUF_EXT_CNT; i++) {
		dtvspi_wr(bs, addr_tmp, reg[i + TR_BUF_EXT_IDX]);
		addr_tmp += SPI_OFFSET_IDX;
	}

	msg_interval = bs->xfer_interval | (bs->xfer_turn_d << BITS_PER_BYTE);
	dtvspi_wr(bs, SPI_REG_012C_MSPI0, msg_interval);
	msg_interval = bs->xfer_trig_d | (bs->xfer_done_d << BITS_PER_BYTE);
	dtvspi_wr(bs, SPI_REG_0128_MSPI0, msg_interval);
}

static void dtvspi_hw_set_mode(struct mt5896_spi *bs, uint spimode)
{
	u8 val = dtvspi_rdl(bs, SPI_REG_0124_MSPI0);

	if (spimode&SPI_CPOL)
		val |= MSPI_CPOL_BIT;
	else
		val &= ~MSPI_CPOL_BIT;

	if (spimode&SPI_CPHA)
		val |= MSPI_CPHA_BIT;
	else
		val &= ~MSPI_CPHA_BIT;

	dtvspi_wrl(bs, SPI_REG_0124_MSPI0, val);
	val = dtvspi_rdl(bs, SPI_REG_0140_MSPI0);
	if (spimode&SPI_LSB_FIRST)
		val |= MSPI_LSB_FIRST_BIT;
	else
		val &= ~MSPI_LSB_FIRST_BIT;

	dtvspi_wrl(bs, SPI_REG_0140_MSPI0, val);

	val = dtvspi_rdl(bs, SPI_REG_00E0_MSPI0);
	if ((spimode&SPI_TX_DUAL) || (spimode&SPI_RX_DUAL))
		val |= MSPI_ENABLE_DUAL;
	else
		val &= ~MSPI_ENABLE_DUAL;

	dtvspi_wrl(bs, SPI_REG_00E0_MSPI0, val);
}

static void dtvspi_hw_set_clock(struct mt5896_spi *bs, uint spispeed)
{
	unsigned int i, j;
	u32 tolerance[MSPI_MOUDLE_CLKMUX_SETS];
	u32 mspi_div[MSPI_MOUDLE_CLKMUX_SETS] = {0}, sclk_div[MSPI_MOUDLE_CLKMUX_SETS] = {0};
	u32 final_tolerance = UINT_MAX;
	unsigned int clk_idx = UINT_MAX;
	u16 reg_val;

	for (i = 0; i < MSPI_MOUDLE_CLKMUX_SETS; i++) {
		tolerance[i] = UINT_MAX;
	}

	// traverse all clock source to get appropriate setting
	for (i = 0; i < MSPI_MOUDLE_CLKMUX_SETS; i++) {
		u32 clk_speed = bs->clkmux[i].speed;

		dev_dbg(bs->dev, "try set mspi speed %d from source clk idx %d speed %d\n",
			spispeed, i, bs->clkmux[i].speed);

		for (j = 0; j <= MAX_SCLK_DIV; j++) {
			u32 real_baud = 0;
			u32 mspi_div_tmp;
			u32 sclk_div_result = clk_speed >> (j + 1);

			if (sclk_div_result <= spispeed) {
				mspi_div_tmp = 0;
				real_baud = sclk_div_result;
			} else {
				mspi_div_tmp = (sclk_div_result / spispeed) - 1;
				mspi_div_tmp += ((sclk_div_result % spispeed) == 0) ? 0 : 1;
				if (mspi_div_tmp > MAX_MSPI_DIV) {
					/*
					 * request speed is too slow to get clk divider by this sclk
					 * divisor, continue by next sclk divisor
					 */
					dev_dbg(bs->dev, "sclk div %d: mspi_div %d too large to fit requirement\n",
						j + 1, mspi_div_tmp);
					continue;
				}
				real_baud = sclk_div_result / (mspi_div_tmp + 1);
			}

			dev_dbg(bs->dev, "sclk div %d: sclk_div result %d mspi_div %d baud rate %d\n",
				j + 1, sclk_div_result, mspi_div_tmp, real_baud);

			if ((spispeed - real_baud) < tolerance[i]) {
				dev_dbg(bs->dev, "source clock %d use sclk_div %d mspi_div %d baud rate %d\n",
				    i, j, mspi_div_tmp, real_baud);
				tolerance[i] = spispeed - real_baud;
				sclk_div[i] = j;
				mspi_div[i] = mspi_div_tmp;
			}

	                if (tolerance[i] == 0) {
	                    // got exactly clock setting, no need further calculate
	                    // break out current source clock rate calculation
	                    break;
	                }

			if (sclk_div_result <= spispeed) {
			    // baud rate after SPI divider smaller than request rate,
			    // no need further calculate of current source clock source
			    // break out current source clock rate and continue next source clock rate
			    break;
			}
		}


		if (tolerance[i] == 0) {
		    // got exactly clock setting, no need further calculate
		    break;
		}
	}

	for (i = 0; i < MSPI_MOUDLE_CLKMUX_SETS; i++)
		if (tolerance[i] < final_tolerance) {
			final_tolerance = tolerance[i];
			clk_idx = i;
			if (final_tolerance == 0)
				break;
		}

	if (clk_idx >= MSPI_MOUDLE_CLKMUX_SETS) {
		dev_err(bs->dev, "no appropriate setting for mspi speed %d\n", spispeed);
		return;
	}

	dev_dbg(bs->dev, "mspi speed %d source clk idx %d speed %d mspi_div %d sclk_div %d\n",
		spispeed, clk_idx, bs->clkmux[clk_idx].speed, mspi_div[clk_idx], sclk_div[clk_idx]);

	// set mspi_div
	reg_val = readb((u64)(bs->clkdiv_reg));
	if (bs->clkdiv_msk == MSPI_DIV_EVEN_MSK) {
		reg_val &= ~MSPI_DIV_EVEN_MSK;
		reg_val |= (mspi_div[clk_idx] & MSPI_DIV_EVEN_MSK);
	} else {
		reg_val &= ~MSPI_DIV_ODD_MSK;
		reg_val |= ((mspi_div[clk_idx] << MSPI_DIV_ODD_SFT) & MSPI_DIV_ODD_MSK);
	}
	writeb(reg_val, (u64)(bs->clkdiv_reg));
	dev_dbg(bs->dev, "mspi_div, addr %X, msk %X, val %X\n", bs->clkdiv_reg, bs->clkdiv_msk,
		mspi_div[clk_idx]);

	// set sclk_div
	dtvspi_wrh(bs, SPI_REG_0124_MSPI0, sclk_div[clk_idx] & U8_MAX);

	// select source clock by index 'clk_idx'
	for (i = 0; i < MSPI_MOUDLE_REG_SETS_PER_CLKMUX; i++) {
		dev_dbg(bs->dev, "module clks, addr %X, msk %X, val %X\n",
			bs->clkmux[clk_idx].clkmux_set[i].clk_reg,
			bs->clkmux[clk_idx].clkmux_set[i].clk_reg_msk,
			bs->clkmux[clk_idx].clkmux_set[i].clk_reg_val);
		reg_val = readw((u64)(bs->clkmux[clk_idx].clkmux_set[i].clk_reg));
		reg_val &= ~(bs->clkmux[clk_idx].clkmux_set[i].clk_reg_msk);
		reg_val |= bs->clkmux[clk_idx].clkmux_set[i].clk_reg_val;
		writew(reg_val, (u64)(bs->clkmux[clk_idx].clkmux_set[i].clk_reg));
	}
}

static inline void dtvspi_hw_chip_select(struct mt5896_spi *bs, bool enable)
{
	u8 val;

	if (isCsGPIO) {
		if (enable)
			dm_gpio_set_value(&bs->gpio_cs, 0);
		else
			dm_gpio_set_value(&bs->gpio_cs, 1);
	} else {
		val = dtvspi_rdl(bs, SPI_REG_017C_MSPI0);
		if (enable)
			val &= ~MSPI_CHIP_SELECT_BIT;
		else
			val |= MSPI_CHIP_SELECT_BIT;

		dtvspi_wrl(bs, SPI_REG_017C_MSPI0, val);
	}
}

static inline void dtvspi_hw_enable(struct mt5896_spi *bs, bool enable)
{
	u8 val;

	val = dtvspi_rdl(bs, SPI_REG_0124_MSPI0);
	if (enable) {
		val |= MSPI_ENABLE_BIT;
		val |= MSPI_RESET_BIT;
	} else {
		val &= ~MSPI_ENABLE_BIT;
		val &= ~MSPI_RESET_BIT;
	}
	dtvspi_wrl(bs, SPI_REG_0124_MSPI0, val);
}

static inline void dtvspi_hw_clear_done(struct mt5896_spi *bs)
{
	dtvspi_wrl(bs, SPI_REG_0170_MSPI0, MSPI_CLEAR_DONE_FLAG_BIT);
}

static inline void dtvspi_hw_transfer_trigger(struct mt5896_spi *bs)
{
	dtvspi_wr(bs, SPI_REG_0168_MSPI0, MSPI_TRIGGER_BIT);
}

#if 0
static void dtvspi_hw_fdx_rx_ext(struct mt5896_spi *bs, int len)
{
	u16 u8Index;
	int u16TempBuf;
	u16 addr_tmp;

	if (!bs->rx_buf)
		// direct return when rx_buf not specified
		return;

	for (u8Index = 0; u8Index < len; u8Index++) {
		if (u8Index & 1) {
			addr_tmp = SPI_REG_01E0_MSPI0 + ((u8Index >> 1) * 4);
			u16TempBuf = dtvspi_rd(bs, addr_tmp);
			(bs->rx_buf)[u8Index] = u16TempBuf >> 8;
			(bs->rx_buf)[u8Index - 1] = u16TempBuf & 0xFF;
		} else if (u8Index == (len - 1)) {
			addr_tmp = SPI_REG_01E0_MSPI0 + ((u8Index >> 1) * 4);
			u16TempBuf = dtvspi_rdl(bs, addr_tmp);
			(bs->rx_buf)[u8Index] = u16TempBuf;
		}
	}

	(bs->rx_buf) += len;
}
#endif

static void dtvspi_hw_rxgetfifo_ext(struct mt5896_spi *bs, u8 *buffer, u8 len)
{
	int	cnt;

	for (cnt = 0; cnt < (len>>1); cnt++) {
		u16 val = dtvspi_rd(bs, mspi_rxfifoaddr_halfdeplex[cnt]);

		buffer[cnt<<1] = val&0xff;
		buffer[(cnt<<1)+1] = val>>BYTE_SHIFT;
	}
	if (len&1)
		buffer[cnt<<1] = dtvspi_rdl(bs, mspi_rxfifoaddr_halfdeplex[cnt]);
}

static void dtvspi_hw_rx_ext(struct mt5896_spi *bs, int len)
{
	dtvspi_hw_rxgetfifo_ext(bs, bs->rx_buf, len);
	(bs->rx_buf) += len;
}

static void dtvspi_hw_txfillfifo(struct mt5896_spi *bs, const u8 *buffer, u8 len)
{
	int cnt;

	for (cnt = 0; cnt < len>>1; cnt++)
		dtvspi_wr(bs, mspi_txfifoaddr[cnt],
			buffer[cnt<<1]|(buffer[(cnt<<1)+1]<<BYTE_SHIFT));

	if (len&1)
		dtvspi_wrl(bs, mspi_txfifoaddr[cnt], buffer[cnt<<1]);

	dtvspi_wrl(bs, SPI_REG_0120_MSPI0, len);
	dtvspi_wrh(bs, SPI_REG_0120_MSPI0, 0);
}

static void dtvspi_hw_xfer_ext(struct mt5896_spi *bs, int len)
{
	if ((bs->tx_buf != NULL)&&(bs->flags !=SPI_XFER_END)) {
		// TX only half duplex or full duplex
		dtvspi_hw_txfillfifo(bs, bs->tx_buf, len);
		(bs->tx_buf) += len;
	} else {
		// RX only half duplex
		if(bs->rx_buf != NULL) {
			dtvspi_wrh(bs, SPI_REG_0120_MSPI0, len);
			dtvspi_wrl(bs, SPI_REG_0120_MSPI0, 0);
		}
	}
	dtvspi_hw_transfer_trigger(bs);
}

static int mt5896_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mt5896_spi *rs = dev_get_priv(bus);

	dtvspi_hw_set_mode(rs, mode);
	return 0;
}

static int mt5896_spi_set_speed(struct udevice *bus, uint speed)
{
	struct mt5896_spi *rs = dev_get_priv(bus);

	dtvspi_hw_set_clock(rs, speed);
	return 0;
}

static int mt5896_spi_set_wordlen(struct udevice *bus, unsigned int wordlen)
{
	struct mt5896_spi *rs = dev_get_priv(bus);
	int bits;
	int addr_tmp;
	uint i;

	if (wordlen > SPI_BIT_LEN_FULL) {
		dev_err(bs->dev, "wordlen %d too large, max valid is %d\n", wordlen,
			SPI_BIT_LEN_FULL);
		return -EINVAL;
	}

	bits = wordlen - 1;

	/* generate register value mask */
	for (i = 0; i < SPI_BIT_MUX_PER_REG; i++) {
		bits <<= SPI_BIT_MUX_LEN;
		bits |= wordlen - 1;
	}

	/* write registers */
	/* word 0~7 of write */
	dtvspi_wr(rs, SPI_REG_0130_MSPI0, bits);
	dtvspi_wr(rs, SPI_REG_0134_MSPI0, bits);
	/* word 0~7 of read */
	dtvspi_wr(rs, SPI_REG_0138_MSPI0, bits);
	dtvspi_wr(rs, SPI_REG_013C_MSPI0, bits);
	/* word 8~31 of write */
	addr_tmp = SPI_REG_0080_MSPI0;
	for (i = 0; i < TR_BUF_EXT_CNT; i++) {
		dtvspi_wr(rs, addr_tmp, bits);
		addr_tmp += SPI_OFFSET_IDX;
	}
	/* word 8~31 of read */
	addr_tmp = SPI_REG_00A0_MSPI0;
	for (i = 0; i < TR_BUF_EXT_CNT; i++) {
		dtvspi_wr(rs, addr_tmp, bits);
		addr_tmp += SPI_OFFSET_IDX;
	}

	for (i = 0; i < SPI_HDX_MSG_SIZE_MAX; i++)
		rs->xfer_bitmux[i] = wordlen;

	return 0;
}

static inline int mt5896_spi_wait_till_ready(struct mt5896_spi *bs)
{
	int ret;

	ret =  wait_for_bit_le32(bs->base + SPI_REG_016C_MSPI0, SPI_TRANS_BUSY, true, 10, 0);
	if (ret)
		dev_err(bs->dev, "Timeout in %s!\n", __func__);

	dtvspi_hw_clear_done(bs);
	return ret;
}

static int mt5896_spi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout, void *din,
			   unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mt5896_spi *rs = dev_get_priv(bus);
	int total_size = bitlen / 8;
	int chunk_size;
	int max_chunk_size;
	int ret = -1;

	rs->tx_buf = (u8 *)dout;
	rs->rx_buf = din;
	rs->flags = flags;

	/* sanity check */
	if (!bitlen || (!dout && !din))
		return -EINVAL;

	max_chunk_size = SPI_HDX_MSG_SIZE_MAX;
	if (dout && din)
		/* max. chunk size of full duplex */
		max_chunk_size = SPI_FDX_MSG_SIZE_MAX;

	/*
	 * Set CS active upon start of SPI message. This message can
	 * be split upon multiple calls to this xfer function
	 */
	if (flags & SPI_XFER_BEGIN)
		dtvspi_hw_chip_select(rs, 1);

	while (total_size > 0) {
		/* Don't exceed the max xfer size */
		chunk_size = min_t(int, total_size, max_chunk_size);

		/* fill TX fifo and trigger transfer */
		dtvspi_hw_xfer_ext(rs, chunk_size);

		/* Wait until xfer is finished on bus */
		ret = mt5896_spi_wait_till_ready(rs);

		if (rs->rx_buf) {
			dtvspi_hw_rx_ext(rs, chunk_size);
		}
		total_size -= chunk_size;
	}

	/* de-assert CS */
	if (flags & SPI_XFER_END)
		dtvspi_hw_chip_select(rs, 0);

	return ret;
}

static int mt5896_spi_bind(struct udevice *dev)
{
	if ((dev->seq == -1) && (dev->req_seq == -1))
		// assign dev->req_seq
		if (dev_read_alias_seq(dev, &dev->req_seq) < 0) {
			// no alias in dts
			dev_warn(dev, "no alias '%s' assigned in dts for device '%s'\n",
				 dev->driver->name, dev->name);
			dev->seq = uclass_resolve_seq(dev);
		}

	dev_dbg(dev, "assign device seq %d req_seq %d\n", dev->seq, dev->req_seq);

	return 0;
}

static int mt5896_spi_probe(struct udevice *dev)
{
	struct mt5896_spi *rs = dev_get_priv(dev);
	int ret;
	u16 reg_val;
	u32 clk_en_count, *clk_en_set = NULL;
	uint i;
	struct spi_clk_mux_en_setting *clk_en = NULL;

	rs->dev = dev;

	/* register base address */
	rs->base = (void __iomem *)dev_read_addr(dev);
	if (rs->base == (void __iomem *)FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = gpio_request_by_name(dev, "cs-gpios", 0, &rs->gpio_cs, GPIOD_IS_OUT);
	if (ret == 0) {
		dev_err(dev, "SPI CS gpio mode %d\n", ret);
		isCsGPIO = true;
	}
	ret = dev_read_u32_array(dev, "tr_mux", rs->xfer_bitmux, SPI_HDX_MSG_SIZE_MAX);
	if (ret) {
		dev_warn(dev, "could not get tr_mux resource (%d)\n", ret);
		for (i = 0; i < SPI_HDX_MSG_SIZE_MAX; i++)
			rs->xfer_bitmux[i] = SPI_BIT_LEN_FULL;
	}
	for (i = 0; i < SPI_HDX_MSG_SIZE_MAX; i++)
		if (rs->xfer_bitmux[i] > SPI_BIT_LEN_FULL) {
			dev_err(dev, "invalid tr_mux resource value %d\n", rs->xfer_bitmux[i]);
			return -EINVAL;
		}

	ret = dev_read_u32(dev, "xfer_trig_d", &rs->xfer_trig_d);
	if (ret) {
		dev_warn(dev, "could not get xfer_trig_d resource (%d)\n", ret);
		rs->xfer_trig_d = 0;
	}
	if (rs->xfer_trig_d > SPI_MAX_TIME_DELAY_TICKS) {
		dev_err(dev, "invalid xfer_trig_d resource value %d\n", rs->xfer_trig_d);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "xfer_interval", &rs->xfer_interval);
	if (ret) {
		dev_warn(dev, "could not get xfer_interval resource (%d)\n", ret);
		rs->xfer_interval = 0;
	}
	if (rs->xfer_interval > SPI_MAX_TIME_DELAY_TICKS) {
		dev_err(dev, "invalid xfer_interval resource value %d\n", rs->xfer_interval);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "xfer_done_d", &rs->xfer_done_d);
	if (ret) {
		dev_warn(dev, "could not get xfer_done_d resource (%d)\n", ret);
		rs->xfer_done_d = 0;
	}
	if (rs->xfer_done_d > SPI_MAX_TIME_DELAY_TICKS) {
		dev_err(dev, "invalid xfer_done_d resource value %d\n", rs->xfer_done_d);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "xfer_turn_d", &rs->xfer_turn_d);
	if (ret) {
		dev_warn(dev, "could not get xfer_turn_d resource (%d)\n", ret);
		rs->xfer_turn_d = 0;
	}
	if (rs->xfer_turn_d > SPI_MAX_TIME_DELAY_TICKS) {
		dev_err(dev, "invalid xfer_turn_d resource value %d\n", rs->xfer_turn_d);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "module-clock-divider", &rs->clkdiv_reg);
	if (ret) {
		dev_warn(dev, "could not get module-clock-divider resource (%d)\n", ret);
		return -EINVAL;
	}

	ret = dev_read_u32(dev, "module-clock-divider-mask", &rs->clkdiv_msk);
	if (ret) {
		dev_warn(dev, "could not get module-clock-divider-mask resource (%d)\n", ret);
		return -EINVAL;
	}

	/*
	 * Read input clock via DT for now. At some point this should be
	 * replaced by implementing a clock driver for this SoC and getting
	 * the SPI frequency via this clock driver.
	 */
	ret = dev_read_u32(dev, "clock-frequency", &rs->sys_freq);
	if (ret) {
		dev_err(dev, "Read 'clock-frequency' from DTS fail (%d)", ret);
		return ret;
	}

	ret = dev_read_u32_array(dev, "module-clock-select", (u32 *)(&rs->clkmux),
				 sizeof(rs->clkmux) / sizeof(u32));
	if (ret) {
		dev_err(dev, "Read 'module-clock-select' from DTS fail (%d)", ret);
		return ret;
	}

	/* enable sw_en */
	/* get property "module-clocks-enable" size in dts */
	ret = dev_read_u32(dev, "module-clocks-enable", &clk_en_count);
	if (!ret) {
		// allocate size need include clk_en_count
		clk_en_set = malloc(sizeof(*clk_en) * clk_en_count + sizeof(clk_en_count));
		if (!clk_en_set)
			return -ENOMEM;

		/* get property "module-clocks" data */
		ret = dev_read_u32_array(dev, "module-clocks-enable", clk_en_set,
					 (sizeof(*clk_en) / sizeof(u32)) * clk_en_count + 1);
		if (ret) {
			free(clk_en_set);
			return ret;
		}

		clk_en = (struct spi_clk_mux_en_setting *)(clk_en_set + 1);

		/* traverse all "module-clocks-enable" setting */
		/* "module-clocks-enable" format: addr, mask, value */
		for (i = 0 ; i < clk_en_count ; i++, clk_en++) {
			dev_dbg(dev, "module clks, addr %X, msk %X, val %X\n", clk_en->clk_reg,
				clk_en->clk_reg_msk, clk_en->clk_reg_val);
			reg_val = readw((u64)(clk_en->clk_reg));
			reg_val &= ~clk_en->clk_reg_msk;
			reg_val |= clk_en->clk_reg_val;
			writew(reg_val, (u64)(clk_en->clk_reg));
		}

		free(clk_en_set);
	}

	dtvspi_hw_set_clock(rs, rs->sys_freq);
	dtvspi_hw_set_cus(rs);
	dtvspi_hw_enable(rs, 1);

	return 0;
}

static const struct dm_spi_ops mt5896_spi_ops = {
	.set_mode = mt5896_spi_set_mode,
	.set_speed = mt5896_spi_set_speed,
	.set_wordlen = mt5896_spi_set_wordlen,
	.xfer = mt5896_spi_xfer,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id mt5896_spi_ids[] = {
	{ .compatible = "mediatek,mt5896-spi" },
	{},
};

U_BOOT_DRIVER(mt5896_spi) = {
	.name = "spi",	/* driver name for lookup aliases in dts */
	.id = UCLASS_SPI,
	.of_match = mt5896_spi_ids,
	.ops = &mt5896_spi_ops,
	.priv_auto_alloc_size = sizeof(struct mt5896_spi),
	.probe = mt5896_spi_probe,
	.bind = mt5896_spi_bind,
};
