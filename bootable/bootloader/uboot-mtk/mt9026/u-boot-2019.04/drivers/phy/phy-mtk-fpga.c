// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 - 2019 MediaTek Inc.
 * Author: Chunfeng Yun <chunfeng.yun@mediatek.com>
 *	   Ryder Lee <ryder.lee@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/io.h>
#include <linux/compat.h>

#include <dt-bindings/phy/phy.h>
#include "phy-mtk-fpga.h"

struct fpga_phy_instance {
	const struct device_node *np;
	void __iomem *i2c_base;
	struct device *dev;
	u32 index;
	u32 port;
	u32 u3_pclk;
	/*
	 * chip id: assigned from dts, determines which phy init to be called
	 * chip version: read from PHYD version register, sometimes mismatches
	 *				with its real chip id
	 */ 
	u32 chip_id;
	u32 chip_version;
	char name[8];
	u32 type;
};

struct mtk_fpga_phy {
	struct udevice *dev;
	void __iomem *ippc_base;
	//enum mtk_phy_version version;
	struct fpga_phy_instance **phys;
	int nphys;
};

/* ------------------------ I2C  IO API ------------------------------ */
static inline void i2c_dummy_delay(int count)
{
	udelay(count);
}

static void gpio_set_direction(void __iomem *port,
		enum i2c_pin pin, enum i2c_dir dir)
{
	void __iomem  *addr;
	u32 temp;

	addr = port + SSUSB_FPGA_I2C_OUT;
	temp = readl(addr);

	if (pin == I2C_SDA) {
		if (dir == I2C_OUTPUT)
			temp |= SSUSB_FPGA_I2C_SDA_OEN;
		else
			temp &= ~SSUSB_FPGA_I2C_SDA_OEN;
	} else {
		if (dir == I2C_OUTPUT)
			temp |= SSUSB_FPGA_I2C_SCL_OEN;
		else
			temp &= ~SSUSB_FPGA_I2C_SCL_OEN;
	}
	writel(temp, addr);
}

static void gpio_set_value(void __iomem *port,
		enum i2c_pin pin, bool value)
{
	void __iomem  *addr;
	u32 temp;

	addr = port + SSUSB_FPGA_I2C_OUT;
	temp = readl(addr);

	if (pin == I2C_SDA) {
		if (value)
			temp |= SSUSB_FPGA_I2C_SDA_OUT;
		else
			temp &= ~SSUSB_FPGA_I2C_SDA_OUT;
	} else {
		if (value)
			temp |= SSUSB_FPGA_I2C_SCL_OUT;
		else
			temp &= ~SSUSB_FPGA_I2C_SCL_OUT;
	}
	writel(temp, addr);
}

static int gpio_get_value(void __iomem *port, enum i2c_pin pin)
{
	void __iomem *addr;
	u32 temp;

	addr = port + SSUSB_FPGA_I2C_IN;
	temp = readl(addr);

	if (pin == I2C_SDA)
		temp &= SSUSB_FPGA_I2C_SDA_IN;
	else
		temp &= SSUSB_FPGA_I2C_SCL_IN;

	return !!temp;
}

static void i2c_stop(void __iomem *port)
{
	gpio_set_direction(port, I2C_SDA, I2C_OUTPUT);
	gpio_set_value(port, I2C_SCL, 0);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SDA, 0);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SDA, 1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_direction(port, I2C_SCL, I2C_INPUT);
	gpio_set_direction(port, I2C_SDA, I2C_INPUT);
}

/* Prepare the I2C_SDA and I2C_SCL for sending/receiving */
static void i2c_start(void __iomem *port)
{
	gpio_set_direction(port, I2C_SCL, I2C_OUTPUT);
	gpio_set_direction(port, I2C_SDA, I2C_OUTPUT);
	gpio_set_value(port, I2C_SDA, 1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SDA, 0);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 0);
	i2c_dummy_delay(I2C_DELAY);
}

/* return 0 --> ack */
static u32 i2c_send_byte(void __iomem *port, u8 data)
{
	int i, ack;

	gpio_set_direction(port, I2C_SDA, I2C_OUTPUT);

	for (i = 8; --i > 0;) {
		gpio_set_value(port, I2C_SDA, (data >> i) & 0x1);
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(port, I2C_SCL,  1);
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(port, I2C_SCL,  0);
		i2c_dummy_delay(I2C_DELAY);
	}
	gpio_set_value(port, I2C_SDA, (data >> i) & 0x1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL,  1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL,  0);
	i2c_dummy_delay(I2C_DELAY);

	gpio_set_value(port, I2C_SDA, 0);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_direction(port, I2C_SDA, I2C_INPUT);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 1);
	i2c_dummy_delay(I2C_DELAY);
	/* ack 1: error, 0:ok */
	ack = gpio_get_value(port, I2C_SDA);
	gpio_set_value(port, I2C_SCL, 0);
	i2c_dummy_delay(I2C_DELAY);

	return (ack == 1) ? PHY_FALSE : PHY_TRUE;
}

static void i2c_receive_byte(void __iomem *port, u8 *data, u8 ack)
{
	int i;
	u32 dataCache = 0;

	gpio_set_direction(port, I2C_SDA, I2C_INPUT);

	for (i = 8; --i >= 0;) {
		dataCache <<= 1;
		i2c_dummy_delay(I2C_DELAY);
		gpio_set_value(port, I2C_SCL, 1);
		i2c_dummy_delay(I2C_DELAY);
		dataCache |= gpio_get_value(port, I2C_SDA);
		gpio_set_value(port, I2C_SCL, 0);
		i2c_dummy_delay(I2C_DELAY);
	}
	gpio_set_direction(port, I2C_SDA, I2C_OUTPUT);
	gpio_set_value(port, I2C_SDA, ack);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 1);
	i2c_dummy_delay(I2C_DELAY);
	gpio_set_value(port, I2C_SCL, 0);
	i2c_dummy_delay(I2C_DELAY);
	*data = (u8)dataCache;
}

static int i2c_write_reg(void __iomem *port, u8 i2c_addr, u8 addr, u8 data)
{
	int ack = 0;

	i2c_start(port);

	ack = i2c_send_byte(port, (i2c_addr << 1) & 0xff);
	if (ack)
		ack = i2c_send_byte(port, addr);
	else
		return PHY_FALSE;

	ack = i2c_send_byte(port, data);
	if (ack) {
		i2c_stop(port);
		return PHY_FALSE;
	} else {
		return PHY_TRUE;
	}
}

static int i2c_read_reg(void __iomem *port, u8 i2c_addr, u8 addr, u8 *data)
{
	int ack = 0;

	i2c_start(port);

	ack = i2c_send_byte(port, (i2c_addr << 1) & 0xff);
	if (ack)
		ack = i2c_send_byte(port, addr);
	else
		return PHY_FALSE;

	i2c_start(port);

	ack = i2c_send_byte(port, ((i2c_addr << 1) & 0xff) | 0x1);
	/* ack 0: ok, 1 error */
	if (ack)
		i2c_receive_byte(port, data, 1);
	else
		return PHY_FALSE;

	i2c_stop(port);
	return ack;
}

int phy_writeb(void __iomem *port, u8 i2c_addr, u8 addr, u8 value)
{
	i2c_write_reg(port, i2c_addr, addr, value);
	return PHY_TRUE;
}

u8 phy_readb(void __iomem *port, u8 i2c_addr,  u8 addr)
{
	u8 buf;
	int ret;

	ret = i2c_read_reg(port, i2c_addr, addr, &buf);
	if (ret == PHY_FALSE) {
		pr_err("Read failed(i2c_addr: %d, addr: 0x%x)\n",
				i2c_addr, addr);
		return ret;
	}

	return buf;
}

static int phy_writel(void __iomem *port, u8 i2c_addr, u32 addr, u32 data)
{
	u8 addr8;
	u8 data_0, data_1, data_2, data_3;

	addr8 = addr & 0xff;
	data_0 = data & 0xff;
	data_1 = (data >> 8) & 0xff;
	data_2 = (data >> 16) & 0xff;
	data_3 = (data >> 24) & 0xff;

	phy_writeb(port, i2c_addr, addr8, data_0);
	phy_writeb(port, i2c_addr, addr8 + 1, data_1);
	phy_writeb(port, i2c_addr, addr8 + 2, data_2);
	phy_writeb(port, i2c_addr, addr8 + 3, data_3);

	return 0;
}

static u32 phy_readl(void __iomem *port, u8 i2c_addr, u32 addr)
{
	u8 addr8;
	u32 data;

	addr8 = addr & 0xff;

	data = phy_readb(port, i2c_addr, addr8);
	data |= (phy_readb(port, i2c_addr, addr8 + 1) << 8);
	data |= (phy_readb(port, i2c_addr, addr8 + 2) << 16);
	data |= (phy_readb(port, i2c_addr, addr8 + 3) << 24);

	return data;
}

static u32 __maybe_unused phy_readlmsk(void __iomem *port,	u8 i2c_addr,
		u32 reg_addr32, u32 offset, u32 mask)
{
	u32 value;

	value = phy_readl(port, i2c_addr, reg_addr32);
	return ((value & mask) >> offset);
}

static int phy_writelmsk(void __iomem *port, u8 i2c_addr,
		u32 reg_addr32, u32 offset, u32 mask, u32 data)
{
	u32 cur_value;
	u32 new_value;

	cur_value = phy_readl(port, i2c_addr, reg_addr32);
	new_value = (cur_value & (~mask)) | ((data << offset) & mask);
	phy_writel(port, i2c_addr, reg_addr32, new_value);

	return 0;
}

/* ------------------------ SUB BOARD INIT API ------------------------------ */
unsigned int ssusb_read_phy_version(struct mtk_fpga_phy *u3phy,
		struct fpga_phy_instance *instance)
{
	void __iomem *i2c = instance->i2c_base;
	u32 version;

	phy_writeb(i2c, 0x60, 0xff, SSUSB_PHY_VERSION_BANK);

	version = phy_readl(i2c, 0x60, SSUSB_PHY_VERSION_ADDR);
	dev_info(u3phy->dev, "[PHY] ssusb phy version: %x\n", version);

	return version;
}

static int a60931_u3phy_init(struct mtk_fpga_phy *u3phy,
		struct fpga_phy_instance *instance)
{
	void __iomem *i2c = instance->i2c_base;

	/* 0xFC[31:24], Change bank address to 0 */
	phy_writeb(i2c, 0x60, 0xff, 0x0);
	/* 0x14[14:12],  RG_USB20_HSTX_SRCTRL, set U2 slew rate as 4 */
	phy_writelmsk(i2c, 0x60, 0x14, 12, GENMASK(14, 12), 0x4);
	/* 0x18[23:23],  RG_USB20_BC11_SW_EN, Disable BC 1.1 */
	phy_writelmsk(i2c, 0x60, 0x18, 23, BIT(23), 0x0);
	/* 0x68[18:18],  force_suspendm = 0 */
	phy_writelmsk(i2c, 0x60, 0x68, 18, BIT(18), 0x0);
	/* 0xFC[31:24], Change bank address to 0x30 */
	phy_writeb(i2c, 0x60, 0xff, 0x30);
	/* 0x04[29:29],  RG_VUSB10_ON, SSUSB 1.0V power ON */
	phy_writelmsk(i2c, 0x60, 0x04, 29, BIT(29), 0x1);
	/* 0x04[25:21], RG_SSUSB_XTAL_TOP_RESERVE */
	phy_writelmsk(i2c, 0x60, 0x04, 21, GENMASK(25, 21), 0x11);
	/* 0xFC[31:24], Change bank address to 0x40 */
	phy_writeb(i2c, 0x60, 0xff, 0x40);
	/* 0x38[15:0], DA_SSUSB_PLL_SSC_DELTA1 */
	/* fine tune SSC delta1 to let SSC min average ~0ppm */
	phy_writelmsk(i2c, 0x60, 0x38, 0, GENMASK(15, 0)<<0, 0x47);
	/* 0x40[31:16], DA_SSUSB_PLL_SSC_DELTA */
	/* fine tune SSC delta to let SSC min average ~0ppm */
	phy_writelmsk(i2c, 0x60, 0x40, 16, GENMASK(31, 16), 0x44);
	/* 0xFC[31:24], Change bank address to 0x30 */
	phy_writeb(i2c, 0x60, 0xff, 0x30);
	/* 0x14[15:0],  RG_SSUSB_PLL_SSC_PRD */
	/* fine tune SSC PRD to let SSC freq average 31.5KHz */
	phy_writelmsk(i2c, 0x60, 0x14, 0, GENMASK(15, 0), 0x190);
	/* 0xFC[31:24], Change bank address to 0x70 */
	phy_writeb(i2c, 0x70, 0xff, 0x70);
	/* 0x88[3:2], Pipe reset, clk driving current */
	phy_writelmsk(i2c, 0x70, 0x88, 2, GENMASK(3, 2), 0x1);
	/* 0x88[5:4], Data lane 0 driving current */
	phy_writelmsk(i2c, 0x70, 0x88, 4, GENMASK(5, 4), 0x1);
	/* 0x88[7:6], Data lane 1 driving current */
	phy_writelmsk(i2c, 0x70, 0x88, 6, GENMASK(7, 6), 0x1);
	/* 0x88[9:8], Data lane 2 driving current */
	phy_writelmsk(i2c, 0x70, 0x88, 8, GENMASK(9, 8), 0x1);
	/* 0x88[11:10], Data lane 3 driving current */
	phy_writelmsk(i2c, 0x70, 0x88, 10, GENMASK(11, 10), 0x1);
	/* 0x9C[4:0],  rg_ssusb_ckphase, PCLK phase 0x00~0x1F */
	phy_writelmsk(i2c, 0x70, 0x9c, 0, GENMASK(4, 0), 0x19);

	/* Set INTR & TX/RX Impedance */

	/* 0xFC[31:24], Change bank address to 0x30 */
	phy_writeb(i2c, 0x60, 0xff, 0x30);
	/* 0x00[26:26],  RG_SSUSB_INTR_EN */
	phy_writelmsk(i2c, 0x60, 0x00, 26, BIT(26), 0x1);
	/* 0x00[15:10],  RG_SSUSB_IEXT_INTR_CTRL, Set Iext R selection */
	phy_writelmsk(i2c, 0x60, 0x00, 10, GENMASK(15, 10), 0x26);
	/* 0xFC[31:24], Change bank address to 0x10 */
	phy_writeb(i2c, 0x60, 0xff, 0x10);
	/* 0x10[31:31],  rg_ssusb_force_tx_impsel,  enable */
	phy_writelmsk(i2c, 0x60, 0x10, 31, BIT(31), 0x1);
	/* 0x10[28:24],  rg_ssusb_tx_impsel, Set TX Impedance */
	phy_writelmsk(i2c, 0x60, 0x10, 24, GENMASK(28, 24), 0x10);
	/* 0x14[31:31],  rg_ssusb_force_rx_impsel, enable */
	phy_writelmsk(i2c, 0x60, 0x14, 31, BIT(31), 0x1);
	/* 0x14[28:24],  rg_ssusb_rx_impsel, Set RX Impedance */
	phy_writelmsk(i2c, 0x60, 0x14, 24, GENMASK(28, 24), 0x10);
	/* 0xFC[31:24], Change bank address to 0x00 */
	phy_writeb(i2c, 0x60, 0xff, 0x00);
	/* 0x00[05:05],  RG_USB20_INTR_EN, U2 INTR_EN */
	phy_writelmsk(i2c, 0x60, 0x00, 5, BIT(5), 0x1);
	/* 0x04[23:19],  RG_USB20_INTR_CAL, Set Iext R selection */
	phy_writelmsk(i2c, 0x60, 0x04, 19, GENMASK(23, 19), 0x14);

	return 0;
}

static int a60931_u3phy_set_pclk(struct mtk_fpga_phy *u3phy,
		struct fpga_phy_instance *instance, int pclk)
{
	void __iomem *i2c = instance->i2c_base;

	phy_writeb(i2c, 0x70, 0xff, 0x70);
	phy_writelmsk(i2c, 0x70, 0x9c, 0, GENMASK(4, 0), pclk);
	return 0;
}

static int a60810_u3phy_init(struct mtk_fpga_phy *u3phy,
		struct fpga_phy_instance *instance)
{
	void __iomem *i2c = instance->i2c_base;

	phy_writeb(i2c, 0x60, 0xFF, 0x00);
	phy_writeb(i2c, 0x60, 0x05, 0x55);
	phy_writeb(i2c, 0x60, 0x18, 0x84);

	phy_writeb(i2c, 0x60, 0xFF, 0x10);
	phy_writeb(i2c, 0x60, 0x0A, 0x84);

	phy_writeb(i2c, 0x60, 0xFF, 0x40);
	phy_writeb(i2c, 0x60, 0x38, 0x46);
	phy_writeb(i2c, 0x60, 0x42, 0x40);
	phy_writeb(i2c, 0x60, 0x08, 0xAB);
	phy_writeb(i2c, 0x60, 0x09, 0x0C);
	phy_writeb(i2c, 0x60, 0x0C, 0x71);
	phy_writeb(i2c, 0x60, 0x0E, 0x4F);
	phy_writeb(i2c, 0x60, 0x10, 0xE1);
	phy_writeb(i2c, 0x60, 0x14, 0x5F);

	phy_writeb(i2c, 0x60, 0xFF, 0x60);
	phy_writeb(i2c, 0x60, 0x14, 0x03);

	phy_writeb(i2c, 0x60, 0xFF, 0x00);
	phy_writeb(i2c, 0x60, 0x6A, 0x04);
	phy_writeb(i2c, 0x60, 0x68, 0x08);
	phy_writeb(i2c, 0x60, 0x6C, 0x26);
	phy_writeb(i2c, 0x60, 0x6D, 0x36);

	dev_info(u3phy->dev, "%s\n", __func__);
	return 0;

}

static int a60855_u3phy_init(struct mtk_fpga_phy *u3phy,
		struct fpga_phy_instance *instance)
{ 	  
	void __iomem *i2c_port_base = instance->i2c_base;

	phy_writeb(i2c_port_base, 0x10, 0xff, 0x02); 
	phy_writelmsk(i2c_port_base, 0x10, 0x0, 1, 0x1<<1, 0x0);	
	phy_writelmsk(i2c_port_base, 0x10, 0x34, 24, 0x1f<<24, 0x0);

	//SPHY calibration
	phy_writeb(i2c_port_base, 0x60, 0xff, 0x10); 
	phy_writelmsk(i2c_port_base, 0x60, 0x14, 31, 0x1<<31, 0x1);
	phy_writelmsk(i2c_port_base, 0x60, 0x14, 24, 0x1f<<24, 0x5);

	phy_writeb(i2c_port_base, 0x60, 0xff, 0x10); 
	phy_writelmsk(i2c_port_base, 0x60, 0x10, 31, 0x1<<31, 0x1);
	phy_writelmsk(i2c_port_base, 0x60, 0x10, 24, 0x1f<<24, 0x6);

	phy_writeb(i2c_port_base, 0x60, 0xff, 0x30); 
	phy_writelmsk(i2c_port_base, 0x60, 0x0, 10, 0x3f<<10, 0xb);

	//U2 PHY init
	phy_writeb(i2c_port_base, 0x68, 0xff, 0x0); 
	phy_writelmsk(i2c_port_base, 0x68, 0x0, 5, 0x1<<5, 0x1);
	phy_writelmsk(i2c_port_base, 0x68, 0x18, 23, 0x1<<23, 0x0);
	phy_writelmsk(i2c_port_base, 0x68, 0x68, 18, 0x1<<18, 0x0);
	phy_writelmsk(i2c_port_base, 0x68, 0x68, 3, 0x1<<3, 0x1);

	phy_writelmsk(i2c_port_base, 0x68, 0x60, 24, 0x1<<24, 0x0);
	phy_writelmsk(i2c_port_base, 0x68, 0x60, 25, 0x1<<25, 0x1);
	//fine tune HS disconnect threshold
	phy_writelmsk(i2c_port_base, 0x68, 0x18, 4, 0xf<<4, 0xf);
	//phy_writelmsk(i2c_port_base, 0x68, 0x4, 8, 0x7<<8, 0x0);
	//disable disconnect detect
	//phy_writelmsk(i2c_port_base, 0x68, 0x18, 12, 0x3<<12, 0x2);

	//U2 performance fine tune
	//I2C  0x68  0x18[03:00]  0x2   RW  RG_USB20_SQTH 
	//phy_writelmsk(i2c_port_base, 0x68, 0x18, 0, 0xf, 0x2);

	return 0;
}

/* --------------------------- PHY DRIVER API ------------------------------- */

static int fpga_phy_init(struct phy *phy)
{
	struct mtk_fpga_phy *u3phy = dev_get_priv(phy->dev);
	struct fpga_phy_instance *instance = u3phy->phys[phy->id];
	int ret;

	dev_info(dev, "[PHY] %s, id: %x\n", __func__, instance->chip_id);
	switch (instance->chip_id) {
	case PHY_TEST_CHIP_A60931:
		ret = a60931_u3phy_init(u3phy, instance);
		if (ret)
			goto err_ret;
		ret = a60931_u3phy_set_pclk(u3phy, instance, instance->u3_pclk);
		if (ret)
			goto err_ret;
		break;
	case PHY_TEST_CHIP_A60810:
		ret = a60810_u3phy_init(u3phy, instance);
		if (ret)
			goto err_ret;
		break;
	case PHY_TEST_CHIP_A60855:
		ret = a60855_u3phy_init(u3phy, instance);
		if (ret)
			goto err_ret;
		break;
	default:
		dev_err(tphy->dev, "%s: incompatible PHY type:0x%x\n", __func__, instance->chip_id);
		return -EINVAL;
	}

	return 0;

err_ret:
	return ret;
}

static int mtk_phy_power_on(struct phy *phy)
{
	return 0;
}

static int mtk_phy_power_off(struct phy *phy)
{
	return 0;
}

static int mtk_phy_exit(struct phy *phy)
{
	return 0;
}

static int fpga_phy_xlate(struct phy *phy,
			 struct ofnode_phandle_args *args)
{
	struct mtk_fpga_phy *u3phy = dev_get_priv(phy->dev);
	struct fpga_phy_instance *instance = NULL;
	const struct device_node *phy_np = ofnode_to_np(args->node);
	u32 index;

	if (!phy_np) {
		dev_err(phy->dev, "null pointer phy node\n");
		return -EINVAL;
	}

	if (args->args_count < 1) {
		dev_err(phy->dev, "invalid number of cells in 'phy' property\n");
		return -EINVAL;
	}

	for (index = 0; index < u3phy->nphys; index++)
		if (phy_np == u3phy->phys[index]->np) {
			instance = u3phy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(phy->dev, "failed to find appropriate phy\n");
		return -EINVAL;
	}

	phy->id = index;
	instance->type = args->args[1];
	if (!(instance->type == PHY_TYPE_USB2 ||
	      instance->type == PHY_TYPE_USB3)) {
		dev_err(phy->dev, "unsupported device type\n");
		return -EINVAL;
	}

	return 0;
}

static const struct phy_ops mtk_fpga_phy_ops = {
	.init		= fpga_phy_init,
	.exit		= mtk_phy_exit,
	.power_on	= mtk_phy_power_on,
	.power_off	= mtk_phy_power_off,
	.of_xlate	= fpga_phy_xlate,
};

static int mtk_fpga_phy_probe(struct udevice *dev)
{
	struct mtk_fpga_phy *u3phy = dev_get_priv(dev);
	ofnode subnode;
	u32 ippc_ctl;
	int index = 0;
	int ret;

	dev_for_each_subnode(subnode, dev)
		u3phy->nphys++;

	u3phy->phys = devm_kcalloc(dev, u3phy->nphys, sizeof(*u3phy->phys),
				  GFP_KERNEL);
	if (!u3phy->phys)
		return -ENOMEM;

	u3phy->dev = dev;

	ret = dev_read_u32(dev, "mediatek,ippc", &ippc_ctl);
	if (ret) {
		dev_err(dev, "Failed to parse ippc value\n");
		return ret;
	}
	u3phy->ippc_base = map_physmem(ippc_ctl, SSUSB_IPPC_LEN, MAP_NOCACHE);
	if (!u3phy->ippc_base) {
		dev_err(dev, "could not ioremap ippc regs\n");
		return -ENOMEM;
	}
	dev_info(dev, "[PHY] ippc 0x%x\n", ippc_ctl);

	dev_for_each_subnode(subnode, dev) {
		struct fpga_phy_instance *instance;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance)
			return -ENOMEM;

		ofnode_read_u32(subnode, "port",
				&instance->port);
		ofnode_read_u32(subnode, "pclk_phase",
				&instance->u3_pclk);
		ofnode_read_u32(subnode, "chip-id",
				&instance->chip_id);
		printf("[PHY] port: %d, u3pclk %d, chip-id %x\n",
			instance->port, instance->u3_pclk, instance->chip_id);

		instance->i2c_base = u3phy->ippc_base +
			SSUSB_FPGA_I2C_PORT_OFFSET(instance->port);

		instance->chip_version= ssusb_read_phy_version(u3phy, instance);
		if (instance->chip_version) {
			if (instance->chip_version != PHY_TEST_CHIP_NONAME)
				/* this chip has a correct chip version */
				instance->chip_id = instance->chip_version;
			else if(instance->chip_id != instance->chip_version) {
				dev_warn(dev, "WARN: this test chip maybe uses a wrong chip id\n"
							"use dts chip_id:%x\n", instance->chip_id);
			}
		} else {
			dev_info(dev, "ERROR: maybe phy DTB not connected or powered?\n");
		}
		sprintf(instance->name, "%x", instance->chip_id);
		instance->index = index;
		instance->np = ofnode_to_np(subnode);
		u3phy->phys[index] = instance;
		index++;
		if (instance->chip_version)
			dev_info(dev, "[PHY] sub-board: %s, port:%d, u3_pclk: %d\n",
				instance->name,	instance->port, instance->u3_pclk);

	}

	return 0;
}

static const struct udevice_id fpga_u3phy_id_table[] = {
	{ .compatible = "mediatek,fpga-u3phy", },
	{ }
};

U_BOOT_DRIVER(mtk_fpga_phy) = {
	.name		= "mtk-fpga-phy",
	.id		= UCLASS_PHY,
	.of_match	= fpga_u3phy_id_table,
	.ops		= &mtk_fpga_phy_ops,
	.probe		= mtk_fpga_phy_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_fpga_phy),
};
