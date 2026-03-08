// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 MediaTek Inc.
 *
 * Author: Kevin Ho <kevin-yc.ho@mediatek.com>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <miiphy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <wait_bit.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <linux/ethtool.h>

#define NUM_TX_DESC		1
#define NUM_RX_DESC		24
#define TX_TOTAL_BUF_SIZE	(NUM_TX_DESC * PKTSIZE_ALIGN)
#define RX_TOTAL_BUF_SIZE	(NUM_RX_DESC * PKTSIZE_ALIGN)
#define TOTAL_PKT_BUF_SIZE	(TX_TOTAL_BUF_SIZE + RX_TOTAL_BUF_SIZE)

#define TX_FIFO_RETRY_CNT_MAX	20

#define IRQ_TSR_OVER (0x1UL)
#define IRQ_TSR_COL (0x1UL << 1)
#define IRQ_TSR_RLE (0x1UL << 2)
#define IRQ_TSR_IDLE (0x1UL << 3)
#define IRQ_TSR_BNQ (0x1UL << 4)
#define IRQ_TSR_COMP (0x1UL << 5)
#define IRQ_TSR_UND (0x1UL << 6)
#define IRQ_TSR_TBNQ (0x1UL << 7)
#define IRQ_TSR_FBNQ (0x1UL << 8)
#define IRQ_TSR_FIFO1_IDLE (0x1UL << 9)
#define IRQ_TSR_FIFO2_IDLE (0x1UL << 10)
#define IRQ_TSR_FIFO3_IDLE (0x1UL << 11)
#define IRQ_TSR_FIFO4_IDLE (0x1UL << 12)

/* rbna/tovr/tund/rtry/rovr */
#define IRQ_ENABLE_BIT 0x43CUL
/* rbna/tovr/tund/rtry/rovr/tcom */
//#define IRQ_ENABLE_BIT 0x4BCUL
#define IRQ_DISABLE_ALL 0xFFFFFFFFUL

/* hw limit tx max length */
#define MAC_TX_MAX_LEN 1580
/* hw limit rx max length */
#define MAC_RX_MAX_LEN 1522

#define RX_DESC_DONE 0x00000001UL
#define RX_DESC_WRAP 0x00000002UL
#define RX_DESC_SIZE 0x000007ffUL

/* workaround, it should be from dts */
#define MIU0_BUS_BASE 0x20000000UL
#define MIU1_BUS_BASE 0xA0000000UL

#define MTK_W8(reg, val8)       writeb((val8), (reg))
#define MTK_W16(reg, val16)     writew((val16), (reg))
#define MTK_W32(reg, val32)     writel((val32), (reg))
#define MTK_R8(reg)             readb((reg))
#define MTK_R16(reg)            readw((reg))
#define MTK_R32(reg)            readl((reg))

#define BANK_BASE_MAC_0 (priv->mac_bank_0)
#define BANK_BASE_MAC_1 (priv->mac_bank_1)
#define BANK_BASE_ALBANY_0 (priv->albany_bank_0)
#define BANK_BASE_ALBANY_1 (priv->albany_bank_1)
#define BANK_BASE_ALBANY_2 (priv->albany_bank_2)
#define BANK_BASE_CLKGEN_0 (priv->clkgen_bank_0)
#define BANK_BASE_CHIP (priv->chip_bank)

#define REG_OFFSET_00_L(base) (base + 0x0000)
#define REG_OFFSET_00_H(base) (base + 0x0001)
#define REG_OFFSET_01_L(base) (base + 0x0004)
#define REG_OFFSET_01_H(base) (base + 0x0005)
#define REG_OFFSET_02_L(base) (base + 0x0008)
#define REG_OFFSET_02_H(base) (base + 0x0009)
#define REG_OFFSET_03_L(base) (base + 0x000c)
#define REG_OFFSET_03_H(base) (base + 0x000d)
#define REG_OFFSET_04_L(base) (base + 0x0010)
#define REG_OFFSET_04_H(base) (base + 0x0011)
#define REG_OFFSET_05_L(base) (base + 0x0014)
#define REG_OFFSET_05_H(base) (base + 0x0015)
#define REG_OFFSET_06_L(base) (base + 0x0018)
#define REG_OFFSET_06_H(base) (base + 0x0019)
#define REG_OFFSET_07_L(base) (base + 0x001c)
#define REG_OFFSET_07_H(base) (base + 0x001d)
#define REG_OFFSET_08_L(base) (base + 0x0020)
#define REG_OFFSET_08_H(base) (base + 0x0021)
#define REG_OFFSET_09_L(base) (base + 0x0024)
#define REG_OFFSET_09_H(base) (base + 0x0025)
#define REG_OFFSET_0A_L(base) (base + 0x0028)
#define REG_OFFSET_0A_H(base) (base + 0x0029)
#define REG_OFFSET_0B_L(base) (base + 0x002c)
#define REG_OFFSET_0B_H(base) (base + 0x002d)
#define REG_OFFSET_0C_L(base) (base + 0x0030)
#define REG_OFFSET_0C_H(base) (base + 0x0031)
#define REG_OFFSET_0D_L(base) (base + 0x0034)
#define REG_OFFSET_0D_H(base) (base + 0x0035)
#define REG_OFFSET_0E_L(base) (base + 0x0038)
#define REG_OFFSET_0E_H(base) (base + 0x0039)
#define REG_OFFSET_0F_L(base) (base + 0x003c)
#define REG_OFFSET_0F_H(base) (base + 0x003d)

#define REG_OFFSET_10_L(base) (base + 0x0040)
#define REG_OFFSET_10_H(base) (base + 0x0041)
#define REG_OFFSET_11_L(base) (base + 0x0044)
#define REG_OFFSET_11_H(base) (base + 0x0045)
#define REG_OFFSET_12_L(base) (base + 0x0048)
#define REG_OFFSET_12_H(base) (base + 0x0049)
#define REG_OFFSET_13_L(base) (base + 0x004c)
#define REG_OFFSET_13_H(base) (base + 0x004d)
#define REG_OFFSET_14_L(base) (base + 0x0050)
#define REG_OFFSET_14_H(base) (base + 0x0051)
#define REG_OFFSET_15_L(base) (base + 0x0054)
#define REG_OFFSET_15_H(base) (base + 0x0055)
#define REG_OFFSET_16_L(base) (base + 0x0058)
#define REG_OFFSET_16_H(base) (base + 0x0059)
#define REG_OFFSET_17_L(base) (base + 0x005c)
#define REG_OFFSET_17_H(base) (base + 0x005d)
#define REG_OFFSET_18_L(base) (base + 0x0060)
#define REG_OFFSET_18_H(base) (base + 0x0061)
#define REG_OFFSET_19_L(base) (base + 0x0064)
#define REG_OFFSET_19_H(base) (base + 0x0065)
#define REG_OFFSET_1A_L(base) (base + 0x0068)
#define REG_OFFSET_1A_H(base) (base + 0x0069)
#define REG_OFFSET_1B_L(base) (base + 0x006c)
#define REG_OFFSET_1B_H(base) (base + 0x006d)
#define REG_OFFSET_1C_L(base) (base + 0x0070)
#define REG_OFFSET_1C_H(base) (base + 0x0071)
#define REG_OFFSET_1D_L(base) (base + 0x0074)
#define REG_OFFSET_1D_H(base) (base + 0x0075)
#define REG_OFFSET_1E_L(base) (base + 0x0078)
#define REG_OFFSET_1E_H(base) (base + 0x0079)
#define REG_OFFSET_1F_L(base) (base + 0x007c)
#define REG_OFFSET_1F_H(base) (base + 0x007d)

#define REG_OFFSET_20_L(base) (base + 0x0080)
#define REG_OFFSET_20_H(base) (base + 0x0081)
#define REG_OFFSET_21_L(base) (base + 0x0084)
#define REG_OFFSET_21_H(base) (base + 0x0085)
#define REG_OFFSET_22_L(base) (base + 0x0088)
#define REG_OFFSET_22_H(base) (base + 0x0089)
#define REG_OFFSET_23_L(base) (base + 0x008c)
#define REG_OFFSET_23_H(base) (base + 0x008d)
#define REG_OFFSET_24_L(base) (base + 0x0090)
#define REG_OFFSET_24_H(base) (base + 0x0091)
#define REG_OFFSET_25_L(base) (base + 0x0094)
#define REG_OFFSET_25_H(base) (base + 0x0095)
#define REG_OFFSET_26_L(base) (base + 0x0098)
#define REG_OFFSET_26_H(base) (base + 0x0099)
#define REG_OFFSET_27_L(base) (base + 0x009c)
#define REG_OFFSET_27_H(base) (base + 0x009d)
#define REG_OFFSET_28_L(base) (base + 0x00a0)
#define REG_OFFSET_28_H(base) (base + 0x00a1)
#define REG_OFFSET_29_L(base) (base + 0x00a4)
#define REG_OFFSET_29_H(base) (base + 0x00a5)
#define REG_OFFSET_2A_L(base) (base + 0x00a8)
#define REG_OFFSET_2A_H(base) (base + 0x00a9)
#define REG_OFFSET_2B_L(base) (base + 0x00ac)
#define REG_OFFSET_2B_H(base) (base + 0x00ad)
#define REG_OFFSET_2C_L(base) (base + 0x00b0)
#define REG_OFFSET_2C_H(base) (base + 0x00b1)
#define REG_OFFSET_2D_L(base) (base + 0x00b4)
#define REG_OFFSET_2D_H(base) (base + 0x00b5)
#define REG_OFFSET_2E_L(base) (base + 0x00b8)
#define REG_OFFSET_2E_H(base) (base + 0x00b9)
#define REG_OFFSET_2F_L(base) (base + 0x00bc)
#define REG_OFFSET_2F_H(base) (base + 0x00bd)

#define REG_OFFSET_30_L(base) (base + 0x00c0)
#define REG_OFFSET_30_H(base) (base + 0x00c1)
#define REG_OFFSET_31_L(base) (base + 0x00c4)
#define REG_OFFSET_31_H(base) (base + 0x00c5)
#define REG_OFFSET_32_L(base) (base + 0x00c8)
#define REG_OFFSET_32_H(base) (base + 0x00c9)
#define REG_OFFSET_33_L(base) (base + 0x00cc)
#define REG_OFFSET_33_H(base) (base + 0x00cd)
#define REG_OFFSET_34_L(base) (base + 0x00d0)
#define REG_OFFSET_34_H(base) (base + 0x00d1)
#define REG_OFFSET_35_L(base) (base + 0x00d4)
#define REG_OFFSET_35_H(base) (base + 0x00d5)
#define REG_OFFSET_36_L(base) (base + 0x00d8)
#define REG_OFFSET_36_H(base) (base + 0x00d9)
#define REG_OFFSET_37_L(base) (base + 0x00dc)
#define REG_OFFSET_37_H(base) (base + 0x00dd)
#define REG_OFFSET_38_L(base) (base + 0x00e0)
#define REG_OFFSET_38_H(base) (base + 0x00e1)
#define REG_OFFSET_39_L(base) (base + 0x00e4)
#define REG_OFFSET_39_H(base) (base + 0x00e5)
#define REG_OFFSET_3A_L(base) (base + 0x00e8)
#define REG_OFFSET_3A_H(base) (base + 0x00e9)
#define REG_OFFSET_3B_L(base) (base + 0x00ec)
#define REG_OFFSET_3B_H(base) (base + 0x00ed)
#define REG_OFFSET_3C_L(base) (base + 0x00f0)
#define REG_OFFSET_3C_H(base) (base + 0x00f1)
#define REG_OFFSET_3D_L(base) (base + 0x00f4)
#define REG_OFFSET_3D_H(base) (base + 0x00f5)
#define REG_OFFSET_3E_L(base) (base + 0x00f8)
#define REG_OFFSET_3E_H(base) (base + 0x00f9)
#define REG_OFFSET_3F_L(base) (base + 0x00fc)
#define REG_OFFSET_3F_H(base) (base + 0x00fd)

#define REG_OFFSET_40_L(base) (base + 0x0100)
#define REG_OFFSET_40_H(base) (base + 0x0101)
#define REG_OFFSET_41_L(base) (base + 0x0104)
#define REG_OFFSET_41_H(base) (base + 0x0105)
#define REG_OFFSET_42_L(base) (base + 0x0108)
#define REG_OFFSET_42_H(base) (base + 0x0109)
#define REG_OFFSET_43_L(base) (base + 0x010c)
#define REG_OFFSET_43_H(base) (base + 0x010d)
#define REG_OFFSET_44_L(base) (base + 0x0110)
#define REG_OFFSET_44_H(base) (base + 0x0111)
#define REG_OFFSET_45_L(base) (base + 0x0114)
#define REG_OFFSET_45_H(base) (base + 0x0115)
#define REG_OFFSET_46_L(base) (base + 0x0118)
#define REG_OFFSET_46_H(base) (base + 0x0119)
#define REG_OFFSET_47_L(base) (base + 0x011c)
#define REG_OFFSET_47_H(base) (base + 0x011d)
#define REG_OFFSET_48_L(base) (base + 0x0120)
#define REG_OFFSET_48_H(base) (base + 0x0121)
#define REG_OFFSET_49_L(base) (base + 0x0124)
#define REG_OFFSET_49_H(base) (base + 0x0125)
#define REG_OFFSET_4A_L(base) (base + 0x0128)
#define REG_OFFSET_4A_H(base) (base + 0x0129)
#define REG_OFFSET_4B_L(base) (base + 0x012c)
#define REG_OFFSET_4B_H(base) (base + 0x012d)
#define REG_OFFSET_4C_L(base) (base + 0x0130)
#define REG_OFFSET_4C_H(base) (base + 0x0131)
#define REG_OFFSET_4D_L(base) (base + 0x0134)
#define REG_OFFSET_4D_H(base) (base + 0x0135)
#define REG_OFFSET_4E_L(base) (base + 0x0138)
#define REG_OFFSET_4E_H(base) (base + 0x0139)
#define REG_OFFSET_4F_L(base) (base + 0x013c)
#define REG_OFFSET_4F_H(base) (base + 0x013d)

#define REG_OFFSET_50_L(base) (base + 0x0140)
#define REG_OFFSET_50_H(base) (base + 0x0141)
#define REG_OFFSET_51_L(base) (base + 0x0144)
#define REG_OFFSET_51_H(base) (base + 0x0145)
#define REG_OFFSET_52_L(base) (base + 0x0148)
#define REG_OFFSET_52_H(base) (base + 0x0149)
#define REG_OFFSET_53_L(base) (base + 0x014c)
#define REG_OFFSET_53_H(base) (base + 0x014d)
#define REG_OFFSET_54_L(base) (base + 0x0150)
#define REG_OFFSET_54_H(base) (base + 0x0151)
#define REG_OFFSET_55_L(base) (base + 0x0154)
#define REG_OFFSET_55_H(base) (base + 0x0155)
#define REG_OFFSET_56_L(base) (base + 0x0158)
#define REG_OFFSET_56_H(base) (base + 0x0159)
#define REG_OFFSET_57_L(base) (base + 0x015c)
#define REG_OFFSET_57_H(base) (base + 0x015d)
#define REG_OFFSET_58_L(base) (base + 0x0160)
#define REG_OFFSET_58_H(base) (base + 0x0161)
#define REG_OFFSET_59_L(base) (base + 0x0164)
#define REG_OFFSET_59_H(base) (base + 0x0165)
#define REG_OFFSET_5A_L(base) (base + 0x0168)
#define REG_OFFSET_5A_H(base) (base + 0x0169)
#define REG_OFFSET_5B_L(base) (base + 0x016c)
#define REG_OFFSET_5B_H(base) (base + 0x016d)
#define REG_OFFSET_5C_L(base) (base + 0x0170)
#define REG_OFFSET_5C_H(base) (base + 0x0171)
#define REG_OFFSET_5D_L(base) (base + 0x0174)
#define REG_OFFSET_5D_H(base) (base + 0x0175)
#define REG_OFFSET_5E_L(base) (base + 0x0178)
#define REG_OFFSET_5E_H(base) (base + 0x0179)
#define REG_OFFSET_5F_L(base) (base + 0x017c)
#define REG_OFFSET_5F_H(base) (base + 0x017d)

#define REG_OFFSET_60_L(base) (base + 0x0180)
#define REG_OFFSET_60_H(base) (base + 0x0181)
#define REG_OFFSET_61_L(base) (base + 0x0184)
#define REG_OFFSET_61_H(base) (base + 0x0185)
#define REG_OFFSET_62_L(base) (base + 0x0188)
#define REG_OFFSET_62_H(base) (base + 0x0189)
#define REG_OFFSET_63_L(base) (base + 0x018c)
#define REG_OFFSET_63_H(base) (base + 0x018d)
#define REG_OFFSET_64_L(base) (base + 0x0190)
#define REG_OFFSET_64_H(base) (base + 0x0191)
#define REG_OFFSET_65_L(base) (base + 0x0194)
#define REG_OFFSET_65_H(base) (base + 0x0195)
#define REG_OFFSET_66_L(base) (base + 0x0198)
#define REG_OFFSET_66_H(base) (base + 0x0199)
#define REG_OFFSET_67_L(base) (base + 0x019c)
#define REG_OFFSET_67_H(base) (base + 0x019d)
#define REG_OFFSET_68_L(base) (base + 0x01a0)
#define REG_OFFSET_68_H(base) (base + 0x01a1)
#define REG_OFFSET_69_L(base) (base + 0x01a4)
#define REG_OFFSET_69_H(base) (base + 0x01a5)
#define REG_OFFSET_6A_L(base) (base + 0x01a8)
#define REG_OFFSET_6A_H(base) (base + 0x01a9)
#define REG_OFFSET_6B_L(base) (base + 0x01ac)
#define REG_OFFSET_6B_H(base) (base + 0x01ad)
#define REG_OFFSET_6C_L(base) (base + 0x01b0)
#define REG_OFFSET_6C_H(base) (base + 0x01b1)
#define REG_OFFSET_6D_L(base) (base + 0x01b4)
#define REG_OFFSET_6D_H(base) (base + 0x01b5)
#define REG_OFFSET_6E_L(base) (base + 0x01b8)
#define REG_OFFSET_6E_H(base) (base + 0x01b9)
#define REG_OFFSET_6F_L(base) (base + 0x01bc)
#define REG_OFFSET_6F_H(base) (base + 0x01bd)

#define REG_OFFSET_70_L(base) (base + 0x01c0)
#define REG_OFFSET_70_H(base) (base + 0x01c1)
#define REG_OFFSET_71_L(base) (base + 0x01c4)
#define REG_OFFSET_71_H(base) (base + 0x01c5)
#define REG_OFFSET_72_L(base) (base + 0x01c8)
#define REG_OFFSET_72_H(base) (base + 0x01c9)
#define REG_OFFSET_73_L(base) (base + 0x01cc)
#define REG_OFFSET_73_H(base) (base + 0x01cd)
#define REG_OFFSET_74_L(base) (base + 0x01d0)
#define REG_OFFSET_74_H(base) (base + 0x01d1)
#define REG_OFFSET_75_L(base) (base + 0x01d4)
#define REG_OFFSET_75_H(base) (base + 0x01d5)
#define REG_OFFSET_76_L(base) (base + 0x01d8)
#define REG_OFFSET_76_H(base) (base + 0x01d9)
#define REG_OFFSET_77_L(base) (base + 0x01dc)
#define REG_OFFSET_77_H(base) (base + 0x01dd)
#define REG_OFFSET_78_L(base) (base + 0x01e0)
#define REG_OFFSET_78_H(base) (base + 0x01e1)
#define REG_OFFSET_79_L(base) (base + 0x01e4)
#define REG_OFFSET_79_H(base) (base + 0x01e5)
#define REG_OFFSET_7A_L(base) (base + 0x01e8)
#define REG_OFFSET_7A_H(base) (base + 0x01e9)
#define REG_OFFSET_7B_L(base) (base + 0x01ec)
#define REG_OFFSET_7B_H(base) (base + 0x01ed)
#define REG_OFFSET_7C_L(base) (base + 0x01f0)
#define REG_OFFSET_7C_H(base) (base + 0x01f1)
#define REG_OFFSET_7D_L(base) (base + 0x01f4)
#define REG_OFFSET_7D_H(base) (base + 0x01f5)
#define REG_OFFSET_7E_L(base) (base + 0x01f8)
#define REG_OFFSET_7E_H(base) (base + 0x01f9)
#define REG_OFFSET_7F_L(base) (base + 0x01fc)
#define REG_OFFSET_7F_H(base) (base + 0x01fd)

struct rx_desc {
	u32 addr;
	u32 size;
};

struct mtk_dtv_eth_priv {
	char pkt_pool[TOTAL_PKT_BUF_SIZE] __aligned(ARCH_DMA_MINALIGN);

	char *tx_buff_addr;
	struct rx_desc *rx_ring;

	int rx_ring_idx;

	void __iomem *mac_bank_0;
	void __iomem *mac_bank_1;
	void __iomem *albany_bank_0;
	void __iomem *albany_bank_1;
	void __iomem *albany_bank_2;
	void __iomem *clkgen_bank_0;
	void __iomem *chip_bank;

	int phy_addr;
};

/* reg ops */
/* phy reg access ops */
static u32 mtk_dtv_eth_phy_read_internal(struct mtk_dtv_eth_priv *priv, u8 reg_addr)
{
	return MTK_R32(BANK_BASE_ALBANY_0 + (reg_addr << 2));
}

static void mtk_dtv_eth_phy_write_internal(struct mtk_dtv_eth_priv *priv, u8 reg_addr, u32 val)
{
	MTK_W32((BANK_BASE_ALBANY_0 + (reg_addr << 2)), val);
}

/* packet data dump for debug usage */
static void mtk_dtv_eth_pkt_dump(ulong addr, u32 len)
{
	u8 *ptr = (u8 *)addr;
	u32 i;

	printf("===== Dump %lx, len %d(%02x) =====\n",
			(long unsigned int)ptr, len, len);
	printf("              00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f\n");
	for (i = 0; i < len; i++) {
		printf("%lx(%02x): %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x\n",
				(long unsigned int)ptr, i,
				*ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7),
				*(ptr + 8), *(ptr + 9), *(ptr + 10), *(ptr + 11), *(ptr + 12), *(ptr + 13), *(ptr + 14), *(ptr + 15));
		ptr += 16;
		i += 15;
	}
	printf("\n");
}

/* memory init */
static void mtk_dtv_eth_mem_init(struct mtk_dtv_eth_priv *priv)
{
	priv->rx_ring = (struct rx_desc *)
		noncached_alloc(sizeof(struct rx_desc) * NUM_RX_DESC,
				ARCH_DMA_MINALIGN);
	printf("rx_ring=%#010x\n", priv->rx_ring);
}

/* fifo init */
static void mtk_dtv_eth_fifo_init(struct mtk_dtv_eth_priv *priv)
{
	char *pkt_base = priv->pkt_pool;
	int i;

	memset(priv->rx_ring, 0, NUM_RX_DESC * sizeof(struct rx_desc));
	memset(priv->pkt_pool, 0, TOTAL_PKT_BUF_SIZE);

	flush_dcache_range((ulong)pkt_base, (ulong)(pkt_base + TOTAL_PKT_BUF_SIZE));

	priv->rx_ring_idx = 0;

	priv->tx_buff_addr = pkt_base;
	pkt_base += PKTSIZE_ALIGN;

	for (i = 0; i < NUM_RX_DESC; i++) {
		priv->rx_ring[i].addr = (virt_to_phys(pkt_base) - MIU0_BUS_BASE);
		/* last descriptor */
		if (i == (NUM_RX_DESC - 1))
			priv->rx_ring[i].addr |= RX_DESC_WRAP;
		pkt_base += PKTSIZE_ALIGN;
	}

	writel((virt_to_phys(priv->rx_ring) - MIU0_BUS_BASE), priv->mac_bank_0 + 0x0030);
	printf("pkt_pool=%#010x\n", priv->pkt_pool);
	printf("tx_buff_addr=%#010x\n", priv->tx_buff_addr);
	printf("desc=%#010x\n", (virt_to_phys(priv->rx_ring) - MIU0_BUS_BASE));
}

/* phy init */
static void mtk_dtv_eth_phy_init(struct mtk_dtv_eth_priv *priv)
{
	u8 reg_read;

	/* swith rx discriptor format to mode 1 */
	MTK_W8(REG_OFFSET_1D_L(BANK_BASE_MAC_1), 0x00);
	MTK_W8(REG_OFFSET_1D_H(BANK_BASE_MAC_1), 0x01);

	/* gain shift */
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_1), 0x02);

	/* det max */
	MTK_W8(REG_OFFSET_27_H(BANK_BASE_ALBANY_1), 0x02);

	/* det min */
	MTK_W8(REG_OFFSET_28_H(BANK_BASE_ALBANY_1), 0x01);

	/* snr len (emc noise) */
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_1), 0x18);

	/* lpbk_enable set to 0 */
	MTK_W8(REG_OFFSET_39_L(BANK_BASE_ALBANY_0), 0xa0);

	/* power-on LD0 */
	MTK_W8(REG_OFFSET_7E_L(BANK_BASE_ALBANY_1), 0x00);
	MTK_W8(REG_OFFSET_7E_H(BANK_BASE_ALBANY_1), 0x00);
	/* power-on SADC */
	MTK_W8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), 0x80);
	/* Power-on ADCPL */
	MTK_W8(REG_OFFSET_66_L(BANK_BASE_ALBANY_1), 0x40);
	/* Power-on REF */
	MTK_W8(REG_OFFSET_5D_H(BANK_BASE_ALBANY_1), 0x04);
	/* Power-on TX */
	MTK_W8(REG_OFFSET_1D_L(BANK_BASE_ALBANY_2), 0x00);
	/* Power-on TX */
	MTK_W8(REG_OFFSET_78_H(BANK_BASE_ALBANY_2), 0x00);
	/* CLKO_ADC_SEL */
	MTK_W8(REG_OFFSET_45_L(BANK_BASE_ALBANY_2), 0x01);
	/* reg_adc_clk_select */
	MTK_W8(REG_OFFSET_1D_H(BANK_BASE_ALBANY_1), 0x01);
	/* Test */
	MTK_W8(REG_OFFSET_62_L(BANK_BASE_ALBANY_1), 0x44);
	/* sadc timer */
	MTK_W8(REG_OFFSET_40_L(BANK_BASE_ALBANY_2), 0x30);

	/* 100 gat */
	MTK_W8(REG_OFFSET_62_H(BANK_BASE_ALBANY_2), 0x00);

	/* 200 gat */
	MTK_W8(REG_OFFSET_18_L(BANK_BASE_ALBANY_2), 0x43);

	/* en_100t_phase */
	MTK_W8(REG_OFFSET_1C_H(BANK_BASE_ALBANY_2), 0x41);

	/* LP mode, DAC OFF */
	MTK_W8(REG_OFFSET_79_L(BANK_BASE_ALBANY_2), 0xf5);
	MTK_W8(REG_OFFSET_79_H(BANK_BASE_ALBANY_2), 0x0d);

	/* Prevent packet drop by inverted waveform */
	MTK_W8(REG_OFFSET_3C_H(BANK_BASE_ALBANY_0), 0xd0);
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_0), 0x5a);

	/* Disable eee */
	MTK_W8(REG_OFFSET_16_H(BANK_BASE_ALBANY_0), 0x7c);

	/* 10T waveform */
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x06);
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), 0x00);
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x00);
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), 0x00);
	/* shadow ctrl */
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x06);
	/* tin17_s2 */
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), 0x19);
	/* tin18_s2 */
	MTK_W8(REG_OFFSET_56_L(BANK_BASE_ALBANY_0), 0x19);
	MTK_W8(REG_OFFSET_56_H(BANK_BASE_ALBANY_0), 0x19);
	/* tin19_s2 */
	MTK_W8(REG_OFFSET_57_L(BANK_BASE_ALBANY_0), 0x19);
	MTK_W8(REG_OFFSET_57_H(BANK_BASE_ALBANY_0), 0x19);
	/* shadow ctrl */
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x00);
	/* tin17_s0 */
	MTK_W8(REG_OFFSET_55_H(BANK_BASE_ALBANY_0), 0x28);
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), 0x19);

	/* Set MII Mode */
	MTK_W8(REG_OFFSET_60_L(BANK_BASE_CLKGEN_0), 0x00);
	MTK_W8(REG_OFFSET_60_H(BANK_BASE_CLKGEN_0), 0x00);
	MTK_W8(REG_OFFSET_61_L(BANK_BASE_CLKGEN_0), 0x00);
	MTK_W8(REG_OFFSET_61_H(BANK_BASE_CLKGEN_0), 0x00);
	MTK_W8(REG_OFFSET_62_L(BANK_BASE_CLKGEN_0), 0x01);
	MTK_W8(REG_OFFSET_62_H(BANK_BASE_CLKGEN_0), 0x00);

	/* speed up timing recovery */
	MTK_W8(REG_OFFSET_7A_H(BANK_BASE_ALBANY_1), 0x02);

	/* signal_det_k */
	MTK_W8(REG_OFFSET_07_H(BANK_BASE_ALBANY_1), 0xc9);

	/* snr_h */
	MTK_W8(REG_OFFSET_44_H(BANK_BASE_ALBANY_1), 0x50);
	MTK_W8(REG_OFFSET_45_H(BANK_BASE_ALBANY_1), 0x80);
	MTK_W8(REG_OFFSET_47_L(BANK_BASE_ALBANY_1), 0x0e);
	MTK_W8(REG_OFFSET_48_L(BANK_BASE_ALBANY_1), 0x04);

	/* 10t_8bt */
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_0), 0x5a);
	MTK_W8(REG_OFFSET_5B_L(BANK_BASE_ALBANY_0), 0x50);
	MTK_W8(REG_OFFSET_7F_H(BANK_BASE_ALBANY_0), 0x1a);

	/* snr check threshold define when snr locked */
	MTK_W8(REG_OFFSET_49_H(BANK_BASE_ALBANY_2), 0x04);

	/* fix eee tx issue */
	MTK_W8(REG_OFFSET_76_L(BANK_BASE_ALBANY_2), 0x10);

	/* release snr setting after 1ms */
	MTK_W8(REG_OFFSET_4A_L(BANK_BASE_ALBANY_2), 0x18);

	/* TR K1 for 28nm process */
	MTK_W8(REG_OFFSET_19_H(BANK_BASE_ALBANY_1), 0x55);
	MTK_W8(REG_OFFSET_08_L(BANK_BASE_ALBANY_1), 0x56);

	/* chiptop [15] allpad_in */
	reg_read = MTK_R8(REG_OFFSET_50_H(BANK_BASE_CHIP));
	reg_read &= 0x7f;
	MTK_W8(REG_OFFSET_50_H(BANK_BASE_CHIP), reg_read);

	/* chiptop pad_top [9:8] */
	reg_read = MTK_R8(REG_OFFSET_6F_H(BANK_BASE_CHIP));
	reg_read &= 0xfe;
	MTK_W8(REG_OFFSET_6F_H(BANK_BASE_CHIP), reg_read);
}

/* scan phy address */
static int mtk_dtv_eth_phy_addr_scan(struct mtk_dtv_eth_priv *priv)
{
    int addr = 0;
    u32 val;

    do {
        val = mtk_dtv_eth_phy_read_internal(priv, MII_BMSR);
        if ((val != 0) && (val != 0xffff)) {
            printf("phy addr(%d)\n", addr);
            break;
        }
        addr++;
    } while (addr < 32);

    if (addr >= 32) {
        printf("bad phy addr(%d), set phy addr to 0\n", addr);
        priv->phy_addr = 0;
        return -1;
    }

    priv->phy_addr = addr;

    return 0;
}

static void mtk_dtv_eth_hw_patch(struct mtk_dtv_eth_priv *priv)
{
	MTK_W8(REG_OFFSET_1B_L(BANK_BASE_MAC_1), 0x44);
	MTK_W8(REG_OFFSET_1B_H(BANK_BASE_MAC_1), 0x32);
	MTK_W8(REG_OFFSET_1C_L(BANK_BASE_MAC_1), 0x05);
	MTK_W8(REG_OFFSET_1C_H(BANK_BASE_MAC_1), 0x01);
}

/* phy link status */
static int mtk_dtv_eth_link_status_update(struct mtk_dtv_eth_priv *priv)
{
	u32 bmsr, bmcr, lpa, adv, neg, speed, duplex, reg;
	u32 hcd_link_st_ok, an_100t_link_st;

	/* latch link status bit to 1 */
	bmsr = mtk_dtv_eth_phy_read_internal(priv, MII_BMSR);
	bmsr |= 0x4UL;
	mtk_dtv_eth_phy_write_internal(priv, MII_BMSR, bmsr);
	bmsr = mtk_dtv_eth_phy_read_internal(priv, MII_BMSR);
	/* check hcd link status */
	hcd_link_st_ok = mtk_dtv_eth_phy_read_internal(priv, 0x21);
	if (!(hcd_link_st_ok & 0x100UL)) {
		/* link down */
		printf("link down: status=%#010x\n", hcd_link_st_ok);

		return -1;
	}

	bmcr = mtk_dtv_eth_phy_read_internal(priv, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		/* auto-negotiation */
		if (!(bmsr & BMSR_ANEGCOMPLETE)) {
			/* link down */
			printf("auto-negotiation still running\n");
			printf("link down: bmcr=%#010x, bmsr=%#010x\n", bmcr, bmsr);

			return -2;
		}

		/* get link partner and advertisement from the phy not from the mac */
		adv = mtk_dtv_eth_phy_read_internal(priv, MII_ADVERTISE);
		lpa = mtk_dtv_eth_phy_read_internal(priv, MII_LPA);

		/* for link parterner adopts force mode and ephy used,
		 * ephy lpa reveals all zero value.
		 * ephy would be forced to full-duplex mode.
		 */
		if (!lpa) {
			/* 100Mbps full-duplex */
			if (bmcr & BMCR_SPEED100)
				lpa |= LPA_100FULL;
			else /* 10Mbps full-duplex */
				lpa |= LPA_10FULL;
		}

		neg = (adv & lpa);

		if (neg & LPA_100FULL) {
			speed = SPEED_100;
			duplex = DUPLEX_FULL;
		} else if (neg & LPA_100HALF) {
			speed = SPEED_100;
			duplex = DUPLEX_HALF;
		} else if (neg & LPA_10FULL) {
			speed = SPEED_10;
			duplex = DUPLEX_FULL;
		} else if (neg & LPA_10HALF) {
			speed = SPEED_10;
			duplex = DUPLEX_HALF;
		} else {
			speed = SPEED_10;
			duplex = DUPLEX_HALF;
			printf("no speed and mode found (lpa=%#, adv=%#)\n",
					lpa, adv);
		}
	} else {
		speed = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
		duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	}

	/* get status from mac */
	reg = MTK_R32(REG_OFFSET_02_L(BANK_BASE_MAC_0));
	if (speed == SPEED_100)
		reg |= 0x1UL;
	else
		reg &= ~(0x1UL);

	if (duplex == DUPLEX_FULL)
		reg |= 0x2UL;
	else
		reg &= ~(0x2UL);

	/* update status to mac */
	MTK_W32(REG_OFFSET_02_L(BANK_BASE_MAC_0), reg);

	/* link up */
	printf("link up: speed=%#x, duplex=%#x\n", speed, duplex);

	return 0;
}

/* mac init */
static int mtk_dtv_eth_mac_init(struct mtk_dtv_eth_priv *priv)
{
	int ret;
	u32 reg_read;

	/* phy init */
	mtk_dtv_eth_phy_init(priv);

	/* todo: internal/external phy selection */
	/* use internal phy */
	MTK_W8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x11);
	MTK_W8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);

	/* scan phy address */
	ret = mtk_dtv_eth_phy_addr_scan(priv);
	if (ret) {
		printf("scan phy failed\n");
		return -1;
	}

	/* 0x8f */
	MTK_W8(REG_OFFSET_01_L(BANK_BASE_MAC_1), 0x00);
	/* 0x8f */
	MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), 0x8f);

	/* delay interrupt: 0x0402 */
	MTK_W8(REG_OFFSET_03_L(BANK_BASE_MAC_1), 0x02);
	MTK_W8(REG_OFFSET_03_H(BANK_BASE_MAC_1), 0x04);

	/* default speed-duplex 100-full */
	reg_read = MTK_R32(REG_OFFSET_02_L(BANK_BASE_MAC_0));
	reg_read |= 0x03;
	MTK_W32(REG_OFFSET_02_L(BANK_BASE_MAC_0), reg_read);

	/* hw patch */
	mtk_dtv_eth_hw_patch(priv);

	return 0;
}


static int mtk_dtv_eth_start(struct udevice *dev)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	int ret;
	u32 reg_read;
	u8 reg_irq;

	/* update link status */
	ret = mtk_dtv_eth_link_status_update(priv);
	if (ret) {
		printf("update link status failed\n");
		return -1;
	}

	/* fifo init */
	mtk_dtv_eth_fifo_init(priv);

	/* clear hardware statistic */
	reg_read = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_read |= 0x20;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_read);

	/* disable all irq first */
	MTK_W32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
	reg_irq = MTK_R8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
	reg_irq &= 0x7f;
	MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
	/* enable irq */
	MTK_W32(REG_OFFSET_14_L(BANK_BASE_MAC_0), IRQ_ENABLE_BIT);
	reg_irq = MTK_R8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
	reg_irq |= 0x80;
	MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

	/* enable hw tx rx */
	reg_read = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_read |= 0x0c;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_read);

	return 0;
}

static int mtk_dtv_eth_send(struct udevice *dev, void *packet, int length)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	u32 tsr_val, cnt = 0, dma_addr;
	u8 tx_fifo[8] = {0};
	u8 i, tx_fifo_token = 0;
	void *pkt_base;

	if (length > MAC_TX_MAX_LEN) {
		printf("bad tx length(%d)\n", length);
		return -EPERM;
	}

	do {
		tsr_val = MTK_R32(REG_OFFSET_0A_L(BANK_BASE_MAC_0));

		tx_fifo[0] = (((tsr_val & IRQ_TSR_IDLE) != 0) ? 1 : 0);
		tx_fifo[1] = (((tsr_val & IRQ_TSR_BNQ) != 0) ? 1 : 0);
		tx_fifo[2] = (((tsr_val & IRQ_TSR_TBNQ) != 0) ? 1 : 0);
		tx_fifo[3] = (((tsr_val & IRQ_TSR_FBNQ) != 0) ? 1 : 0);
		tx_fifo[4] = (((tsr_val & IRQ_TSR_FIFO1_IDLE) != 0) ? 1 : 0);
		tx_fifo[5] = (((tsr_val & IRQ_TSR_FIFO2_IDLE) != 0) ? 1 : 0);
		tx_fifo[6] = (((tsr_val & IRQ_TSR_FIFO3_IDLE) != 0) ? 1 : 0);
		tx_fifo[7] = (((tsr_val & IRQ_TSR_FIFO4_IDLE) != 0) ? 1 : 0);

		for (i = 0; i < 8; i++) {
			tx_fifo_token += tx_fifo[i];
		}
		cnt++;
	} while ((tx_fifo_token <= 4) && (cnt < TX_FIFO_RETRY_CNT_MAX));

	if ((tx_fifo_token <= 4) && (cnt >= TX_FIFO_RETRY_CNT_MAX)) {
		printf("tx fifo full: %#x\n", tsr_val);
		return -EPERM;
	}

	pkt_base = priv->tx_buff_addr;
	memcpy(pkt_base, packet, length);
	flush_dcache_range((ulong)pkt_base, (ulong)pkt_base +
			   roundup(length, ARCH_DMA_MINALIGN));

	dma_addr = (virt_to_phys(pkt_base) - MIU0_BUS_BASE);

	//printf("tx packet dma: 0x%p\n", dma_addr);
	//mtk_dtv_eth_pkt_dump(pkt_base, length);

	MTK_W32(REG_OFFSET_06_L(BANK_BASE_MAC_0), dma_addr);
	MTK_W32(REG_OFFSET_08_L(BANK_BASE_MAC_0), length);

	return 0;
}

static int mtk_dtv_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	int idx = priv->rx_ring_idx;
	uchar *pkt_base;
	u32 length;

	if (!(priv->rx_ring[idx].addr & RX_DESC_DONE)) {
		//printf("rx ring is empty: %d\n", idx);
		return -EAGAIN;
	}

	length = (priv->rx_ring[idx].size & RX_DESC_SIZE);
	pkt_base = (void *)(phys_to_virt(priv->rx_ring[idx].addr + MIU0_BUS_BASE & ~(RX_DESC_DONE | RX_DESC_WRAP)));
	invalidate_dcache_range((ulong)pkt_base, (ulong)pkt_base +
				roundup(length, ARCH_DMA_MINALIGN));

	//mtk_dtv_eth_pkt_dump(pkt_base, length);

	if (packetp)
		*packetp = pkt_base;

	return length;
}

static void mtk_dtv_eth_stop(struct udevice *dev)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	u8 reg_irq;
	u32 reg_trx;


	/* disable hw tx rx */
	reg_trx = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_trx &= 0xfffffff3;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_trx);

	/* disable all irq */
	MTK_W32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
	reg_irq = MTK_R8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
	reg_irq &= 0x7f;
	MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
}

static int mtk_dtv_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	int idx = priv->rx_ring_idx;

	flush_dcache_range((ulong)packet, (ulong)(packet + PKTSIZE_ALIGN));
	priv->rx_ring[idx].addr &= ~RX_DESC_DONE;
	priv->rx_ring_idx = ((priv->rx_ring_idx + 1) % NUM_RX_DESC);

	return 0;
}

static int mtk_dtv_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 val, reg_read;

	printf("mac address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	val = (((u32)mac[3] << 24) | ((u32)mac[2] << 16) |
			((u32)mac[1] << 8) | ((u32)mac[0]));
	MTK_W32(REG_OFFSET_4C_L(BANK_BASE_MAC_0), val);
	reg_read = MTK_R32(REG_OFFSET_4E_L(BANK_BASE_MAC_0));
	val = ((((u32)mac[5] << 8) | ((u32)mac[4])) & 0xffff) |
			(reg_read & 0xffff0000);
	MTK_W32(REG_OFFSET_4E_L(BANK_BASE_MAC_0), val);

	return 0;
}

static int mtk_dtv_eth_probe(struct udevice *dev)
{
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);
	int ret;

	/* memory init */
	mtk_dtv_eth_mem_init(priv);

	/* mac init */
	ret = mtk_dtv_eth_mac_init(priv);
	if (ret != 0) {
		printf("eth init failed\n");
		return -ENODEV;
	}

	return 0;
}

static const struct eth_ops mtk_dtv_eth_ops = {
	.start			= mtk_dtv_eth_start,
	.send			= mtk_dtv_eth_send,
	.recv			= mtk_dtv_eth_recv,
	.stop			= mtk_dtv_eth_stop,
	.free_pkt		= mtk_dtv_eth_free_pkt,
	.write_hwaddr	= mtk_dtv_eth_write_hwaddr,
};

static int mtk_dtv_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mtk_dtv_eth_priv *priv = dev_get_priv(dev);

//	pdata->iobase = devfdt_get_addr(dev);
	pdata->iobase = dev_read_addr_index(dev, 0);
	priv->mac_bank_0 = dev_read_addr_name(dev, "mac0");
	priv->mac_bank_1 = dev_read_addr_name(dev, "mac1");
	priv->albany_bank_0 = dev_read_addr_name(dev, "phy0");
	priv->albany_bank_1 = dev_read_addr_name(dev, "phy1");
	priv->albany_bank_2 = dev_read_addr_name(dev, "phy2");
	priv->clkgen_bank_0 = dev_read_addr_name(dev, "clkgen0");
	priv->chip_bank = dev_read_addr_name(dev, "chip");

	//printf("\x1b[37;41m [%s %d] iobase=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, pdata->iobase);
	//printf("\x1b[37;41m [%s %d] mac_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->mac_bank_0);
	//printf("\x1b[37;41m [%s %d] mac_bank_1=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->mac_bank_1);
	//printf("\x1b[37;41m [%s %d] albany_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->albany_bank_0);
	//printf("\x1b[37;41m [%s %d] albany_bank_1=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->albany_bank_1);
	//printf("\x1b[37;41m [%s %d] albany_bank_2=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->albany_bank_2);
	//printf("\x1b[37;41m [%s %d] clkgen_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->clkgen_bank_0);
	//printf("\x1b[37;41m [%s %d] chip_bank=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->chip_bank);

	return 0;
}

static const struct udevice_id mtk_dtv_eth_ids[] = {
	{ .compatible = "mediatek,mtk-dtv-eth" },
	{ }
};

U_BOOT_DRIVER(mtk_dtv_eth) = {
	.name	= "mtk_dtv_eth",
	.id	= UCLASS_ETH,
	.of_match = mtk_dtv_eth_ids,
	.ofdata_to_platdata = mtk_dtv_eth_ofdata_to_platdata,
	.probe	= mtk_dtv_eth_probe,
	.ops	= &mtk_dtv_eth_ops,
	.priv_auto_alloc_size = sizeof(struct mtk_dtv_eth_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
