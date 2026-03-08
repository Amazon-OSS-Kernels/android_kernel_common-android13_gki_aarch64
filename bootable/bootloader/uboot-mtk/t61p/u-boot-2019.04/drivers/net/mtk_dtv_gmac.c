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
#include <iniutility.h>
#include <environment.h>
#include <debug_impl.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#endif

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

/* rx descriptor addr msb bit32-33, 8bit shift */
#define GMAC_RXDES_ADDR_MSB2_BIT_SHIFT (4)
#define GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT (5)
#define EMAC_RXDES_ADDR_MSB2_BIT_SHIFT (2)
#define EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT (26)

/* tx addr msb bit32-33, 8bit shift */
#define GMAC_TX_ADDR_MSB2_BIT_SHIFT (6)
#define EMAC_TX_ADDR_MSB2_BIT_SHIFT (4)

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
#define BANK_BASE_MAC_2 (priv->mac_bank_1 + 0x200)
#define BANK_BASE_MAC_3 (priv->mac_bank_1 + 0x400)
#define BANK_BASE_MAC_4 (priv->mac_bank_1 + 0x600)
#define BANK_BASE_ALBANY_0 (priv->albany_bank_0)
#define BANK_BASE_ALBANY_1 (priv->albany_bank_0 + 0x200)
#define BANK_BASE_ALBANY_2 (priv->albany_bank_0 + 0x400)
#define BANK_BASE_ALBANY_3 (priv->albany_bank_0 + 0x600)
#define BANK_BASE_CLKGEN_00 (priv->clkgen_bank_0)
#define BANK_BASE_CLKGEN_01 (priv->clkgen_bank_0 + 0x200)
#define BANK_BASE_CLKGEN_02 (priv->clkgen_bank_0 + 0x400)
#define BANK_BASE_CLKGEN_03 (priv->clkgen_bank_0 + 0x600)
#define BANK_BASE_CLKGEN_04 (priv->clkgen_bank_0 + 0x800)
#define BANK_BASE_CLKGEN_05 (priv->clkgen_bank_0 + 0xa00)
#define BANK_BASE_CLKGEN_06 (priv->clkgen_bank_0 + 0xc00)
#define BANK_BASE_CLKGEN_07 (priv->clkgen_bank_0 + 0xe00)
#define BANK_BASE_CLKGEN_08 (priv->clkgen_bank_0 + 0x1000)
#define BANK_BASE_CLKGEN_09 (priv->clkgen_bank_0 + 0x1200)
#define BANK_BASE_CLKGEN_0A (priv->clkgen_bank_0 + 0x1400)
#define BANK_BASE_CLKGEN_0B (priv->clkgen_bank_0 + 0x1600)
#define BANK_BASE_CLKGEN_0C (priv->clkgen_bank_0 + 0x1800)
#define BANK_BASE_CLKGEN_0D (priv->clkgen_bank_0 + 0x1a00)
#define BANK_BASE_CLKGEN_0E (priv->clkgen_bank_0 + 0x1c00)
#define BANK_BASE_CLKGEN_0F (priv->clkgen_bank_0 + 0x1e00)
#define BANK_BASE_CLKGEN_10 (priv->clkgen_bank_0 + 0x2000)
#define BANK_BASE_CLKGEN_11 (priv->clkgen_bank_0 + 0x2200)
#define BANK_BASE_CLKGEN_12 (priv->clkgen_bank_0 + 0x2400)
#define BANK_BASE_CLKGEN_13 (priv->clkgen_bank_0 + 0x2600)
#define BANK_BASE_CLKGEN_14 (priv->clkgen_bank_0 + 0x2800)
#define BANK_BASE_CLKGEN_15 (priv->clkgen_bank_0 + 0x2a00)
#define BANK_BASE_CLKGEN_16 (priv->clkgen_bank_0 + 0x2c00)
#define BANK_BASE_CLKGEN_17 (priv->clkgen_bank_0 + 0x2e00)
#define BANK_BASE_CLKGEN_18 (priv->clkgen_bank_0 + 0x3000)
#define BANK_BASE_CLKGEN_19 (priv->clkgen_bank_0 + 0x3200)
#define BANK_BASE_CLKGEN_1A (priv->clkgen_bank_0 + 0x3400)
#define BANK_BASE_CLKGEN_1B (priv->clkgen_bank_0 + 0x3600)
#define BANK_BASE_CLKGEN_1C (priv->clkgen_bank_0 + 0x3800)
#define BANK_BASE_CLKGEN_1D (priv->clkgen_bank_0 + 0x3a00)
#define BANK_BASE_CLKGEN_1E (priv->clkgen_bank_0 + 0x3c00)
#define BANK_BASE_CLKGEN_1F (priv->clkgen_bank_0 + 0x3e00)
#define BANK_BASE_CHIP (priv->chip_bank)
#define BANK_BASE_CLKGEN_PM_0 (priv->clkgen_pm_bank_0)
#define BANK_BASE_EFUSE (priv->efuse_bank)

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

#ifdef CONFIG_DATA_SEPARATION
#define NETWORK_SECTION		"network"
#define NETWORK_KEY		"NETWORK_INI_FILE"
#endif

#if defined(CONFIG_ANDROID_CN_PLATFORM) || defined(CONFIG_LINUX_REF_PLATFORM)
#define DEFAULT_NETWORK_PART   "tvconfig"
#else
#define DEFAULT_NETWORK_PART   "config"
#endif
#define DEFAULT_NETWORK_INI		"/network.ini"

#define TX_SWING_LEVEL_MAX           (16)
#define EFUSE_TRIG_READ              (0x1UL << 13)
#define EFUSE_RETRY_MAX              (20)
#define EFUSE_ETH_ATOP_ADDR          (0x1c << 2)
#define EFUSE_ETH_ATOP_ACTIVE        (0x1UL << 22)
#define EFUSE_ETH_ATOP_CUR_100T_MASK (0x1f)
#define BIT_ETH_ATOP_CUR_100T        (0)

struct mtk_dtv_gmac_priv;

struct mtk_dtv_gmac_reg_ops {
	void (*write_phy)(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr, u32 val);
	u32 (*read_phy)(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr);
	void (*phy_hw_init)(struct mtk_dtv_gmac_priv *priv);
};

struct rx_desc_gmac {
	u32 addr;
	u32 size;
	u32 size_high;
	u32 reserved;
};

struct rx_desc_emac {
	u32 addr;
	u32 size;
};

enum gmac_ethpll_ictrl {
	ETHPLL_ICTRL_MIN = 0,
	ETHPLL_ICTRL_MT5896 = ETHPLL_ICTRL_MIN,
	ETHPLL_ICTRL_MT5897,
	ETHPLL_ICTRL_MAX
};

enum gmac_phy_type {
	PHY_TYPE_MIN = 0,
	PHY_TYPE_INTERNAL = PHY_TYPE_MIN,
	PHY_TYPE_EXTERNAL,
	PHY_TYPE_MAX
};

enum gmac_mac_type {
	MAC_TYPE_MIN = 0,
	MAC_TYPE_GMAC = MAC_TYPE_MIN,
	MAC_TYPE_EMAC,
	MAC_TYPE_MAX
};

enum gmac_phy_process {
	PHY_PROCESS_MIN = 0,
	PHY_PROCESS_7_12 = PHY_PROCESS_MIN,
	PHY_PROCESS_22,
	PHY_PROCESS_MAX
};

struct mtk_dtv_gmac_priv {
	char pkt_pool[TOTAL_PKT_BUF_SIZE] __aligned(ARCH_DMA_MINALIGN);

	char *tx_buff_addr;
	void *rx_ring;

	int rx_ring_idx;

	void __iomem *mac_bank_0;
	void __iomem *mac_bank_1;
	void __iomem *albany_bank_0;
	void __iomem *clkgen_bank_0;
	void __iomem *chip_bank;
	void __iomem *clkgen_pm_bank_0;
	void __iomem *efuse_bank;

	u8 phy_addr;
	bool is_internal_phy;
	bool is_force_10m;
	bool is_force_100m;
	bool is_fpga_haps;

	enum gmac_ethpll_ictrl ethpll_ictrl_type;
	enum gmac_phy_type phy_type;
	enum gmac_mac_type mac_type;
	enum gmac_phy_process phy_process;

	struct mtk_dtv_gmac_reg_ops *reg_ops;

	/* tx swing */
	u32 tx_swing_level;
};

/* reg ops */
/* phy reg access ops */
static u32 mtk_dtv_gmac_phy_read_internal(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr)
{
	return MTK_R32(BANK_BASE_ALBANY_0 + (reg_addr << 2));
}

static u32 mtk_dtv_gmac_phy_read_external(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr)
{
	u32 reg_man, reg_ctl, reg_stat, reg_read;

	reg_man = ((0x60020000) | ((phy_addr & 0x1f) << 23) | (reg_addr << 18));

	/* enable mdio */
	reg_ctl = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_ctl |= 0x10;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

	MTK_W32(REG_OFFSET_1A_L(BANK_BASE_MAC_0), reg_man);

	/* wait until IDLE bit in Network Status register is cleared */
	do {
	reg_stat = MTK_R32(REG_OFFSET_04_L(BANK_BASE_MAC_0));
	} while (!(reg_stat & 0x04));

	reg_read = MTK_R32(REG_OFFSET_1A_L(BANK_BASE_MAC_0));
	reg_read &= 0xffff;

	/* disable mdio */
	reg_ctl &= 0xef;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

	return reg_read;
}

static void mtk_dtv_gmac_phy_write_internal(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr, u32 val)
{
	MTK_W32((BANK_BASE_ALBANY_0 + (reg_addr << 2)), val);
}

static void mtk_dtv_gmac_phy_write_external(struct mtk_dtv_gmac_priv *priv, u8 phy_addr, u8 reg_addr, u32 val)
{
	u32 reg_man, reg_ctl, reg_stat;

	reg_man = ((0x50020000) | ((phy_addr & 0x1f) << 23) |
		(reg_addr << 18) | (val & 0xffff));

	/* enable mdio */
	reg_ctl = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_ctl |= 0x10;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

	MTK_W32(REG_OFFSET_1A_L(BANK_BASE_MAC_0), reg_man);

	/* wait until IDLE bit in Network Status register is cleared */
	do {
	reg_stat = MTK_R32(REG_OFFSET_04_L(BANK_BASE_MAC_0));
	} while (!(reg_stat & 0x04));

	/* disable mdio */
	reg_ctl &= 0xef;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);
}

/* packet data dump for debug usage */
/*
static void mtk_dtv_gmac_pkt_dump(ulong addr, u32 len)
{
	u8 *ptr = (u8 *)addr;
	u32 i;

	printf("===== Dump %lx, len %d(%02x) =====\n",
			(unsigned long)ptr, len, len);
	printf("              00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f\n");
	for (i = 0; i < len; i++) {
		printf("%lx(%02x): %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x\n",
				(unsigned long)ptr, i,
				*ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7),
				*(ptr + 8), *(ptr + 9), *(ptr + 10), *(ptr + 11), *(ptr + 12), *(ptr + 13), *(ptr + 14), *(ptr + 15));
		ptr += 16;
		i += 15;
	}
	printf("\n");
}
*/

/* memory init */
static void mtk_dtv_gmac_mem_init(struct mtk_dtv_gmac_priv *priv)
{
	if (priv->mac_type == MAC_TYPE_EMAC) {
		priv->rx_ring = (struct rx_desc_emac *)
				noncached_alloc(sizeof(struct rx_desc_emac) * NUM_RX_DESC,
						ARCH_DMA_MINALIGN);
	} else {
		priv->rx_ring = (struct rx_desc_gmac *)
				noncached_alloc(sizeof(struct rx_desc_gmac) * NUM_RX_DESC,
						ARCH_DMA_MINALIGN);
	}
	//printf("rx_ring=0x%lx\n", (unsigned long)priv->rx_ring);
}

/* fifo init */
static void mtk_dtv_gmac_fifo_init(struct mtk_dtv_gmac_priv *priv)
{
	char *pkt_base = priv->pkt_pool;
	int i;
	unsigned long pa_addr = 0;
	u8 reg_read;

	if (priv->mac_type == MAC_TYPE_EMAC)
		memset(priv->rx_ring, 0, NUM_RX_DESC * sizeof(struct rx_desc_emac));
	else
		memset(priv->rx_ring, 0, NUM_RX_DESC * sizeof(struct rx_desc_gmac));

	memset(priv->pkt_pool, 0, TOTAL_PKT_BUF_SIZE);

	flush_dcache_range((ulong)pkt_base, (ulong)(pkt_base + TOTAL_PKT_BUF_SIZE));

	priv->rx_ring_idx = 0;

	priv->tx_buff_addr = pkt_base;
	pkt_base += PKTSIZE_ALIGN;

	if (priv->mac_type == MAC_TYPE_EMAC) {
		struct rx_desc_emac *rx_ring = priv->rx_ring;

		for (i = 0; i < NUM_RX_DESC; i++) {
			pa_addr = virt_to_phys(pkt_base) - MIU0_BUS_BASE;
			rx_ring[i].addr = pa_addr & 0xffffffff;

			if (pa_addr & 0x300000000) {
				/* over 0-31bits, config 32-33bits */
				reg_read = MTK_R8(REG_OFFSET_23_H(BANK_BASE_MAC_1));
				reg_read &= ~(0x3 << EMAC_RXDES_ADDR_MSB2_BIT_SHIFT);
				reg_read |= (((pa_addr & 0x300000000) >> 32) << EMAC_RXDES_ADDR_MSB2_BIT_SHIFT);
				MTK_W8(REG_OFFSET_23_H(BANK_BASE_MAC_1), reg_read);

				rx_ring[i].size &= ~(0x3 << EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT);
				rx_ring[i].size |=
					(((pa_addr & 0x300000000) >> 32) << EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT);
			}

			/* last descriptor */
			if (i == (NUM_RX_DESC - 1))
				rx_ring[i].addr |= RX_DESC_WRAP;
			pkt_base += PKTSIZE_ALIGN;
		}
	} else {
		struct rx_desc_gmac *rx_ring = priv->rx_ring;

		for (i = 0; i < NUM_RX_DESC; i++) {
			pa_addr = virt_to_phys(pkt_base) - MIU0_BUS_BASE;
			rx_ring[i].addr = pa_addr & 0xffffffff;

			if (pa_addr & 0x300000000) {
				/* over 0-31bits, config 32-33bits */
				reg_read = MTK_R8(REG_OFFSET_09_H(BANK_BASE_MAC_4));
				reg_read &= ~(0x3 << GMAC_RXDES_ADDR_MSB2_BIT_SHIFT);
				reg_read |= (((pa_addr & 0x300000000) >> 32) << GMAC_RXDES_ADDR_MSB2_BIT_SHIFT);
				MTK_W8(REG_OFFSET_09_H(BANK_BASE_MAC_4), reg_read);

				rx_ring[i].size_high &= ~(0x3 << GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT);
				rx_ring[i].size_high |=
					(((pa_addr & 0x300000000) >> 32) << GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT);
			}

			/* last descriptor */
			if (i == (NUM_RX_DESC - 1))
				rx_ring[i].addr |= RX_DESC_WRAP;
			pkt_base += PKTSIZE_ALIGN;
		}
	}

	writel((virt_to_phys(priv->rx_ring) - MIU0_BUS_BASE), priv->mac_bank_0 + 0x0030);
	printf("pkt_pool=0x%lx\n", (unsigned long)priv->pkt_pool);
	printf("tx_buff_addr=0x%lx\n", (unsigned long)priv->tx_buff_addr);
	printf("rx_ring=0x%lx\n", (unsigned long)priv->rx_ring);
	printf("desc=0x%lx\n", (unsigned long)(virt_to_phys(priv->rx_ring) - MIU0_BUS_BASE));
}

/* phy init */
static void mtk_dtv_gmac_phy_hw_init_7_12(struct mtk_dtv_gmac_priv *priv)
{
	u8 reg_read;

	//wriu     0x0421b4             0x02
	reg_read = 0x02;
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04214f             0x02
	reg_read = 0x02;
	MTK_W8(REG_OFFSET_27_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042151             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_28_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042177             0x18
	reg_read = 0x18;
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042072             0xa0
	reg_read = 0xa0;
	MTK_W8(REG_OFFSET_39_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0421fc             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7E_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0421fd             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7E_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422a1             0x80
	//reg_read = 0x80;
	//MTK_W8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422fa             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7D_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422fb             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7D_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04228e             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_47_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04228f             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_47_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422dc             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6E_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422dd             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6E_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422d4             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6A_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422d5             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6A_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422d8             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6C_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422d9             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_6C_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f6             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7B_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f7             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7B_H(BANK_BASE_ALBANY_2), reg_read);

	if (priv->ethpll_ictrl_type == ETHPLL_ICTRL_MT5897) {
		//wriu     0x0422f8             0x02
		reg_read = 0x02;
		MTK_W8(REG_OFFSET_7C_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x04228c             0x01
		reg_read = 0x01;
		MTK_W8(REG_OFFSET_46_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x0422d2             0xc4
		reg_read = 0xc4;
		MTK_W8(REG_OFFSET_69_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x0422f4             0x03
		reg_read = 0x03;
		MTK_W8(REG_OFFSET_7A_L(BANK_BASE_ALBANY_2), reg_read);
	} else {
		//wriu     0x0422f8             0x12
		reg_read = 0x12;
		MTK_W8(REG_OFFSET_7C_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x04228c             0x11
		reg_read = 0x11;
		MTK_W8(REG_OFFSET_46_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x0422d2             0xd4
		reg_read = 0xd4;
		MTK_W8(REG_OFFSET_69_L(BANK_BASE_ALBANY_2), reg_read);

		//wriu     0x0422f4             0x13
		reg_read = 0x13;
		MTK_W8(REG_OFFSET_7A_L(BANK_BASE_ALBANY_2), reg_read);
	}

	//wriu     0x0421cc             0x40
	reg_read = 0x40;
	MTK_W8(REG_OFFSET_66_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0421bb             0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_5D_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422aa             0x06
	reg_read = 0x06;
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04223a             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_1D_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f1             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_78_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04228a             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_45_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04213b             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_1D_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042280             0x30
	reg_read = 0x30;
	MTK_W8(REG_OFFSET_40_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422c5             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_62_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x042230             0x43
	reg_read = 0x43;
	MTK_W8(REG_OFFSET_18_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x042239             0x41
	reg_read = 0x41;
	MTK_W8(REG_OFFSET_1C_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f2             0xf5
	reg_read = 0xf5;
	MTK_W8(REG_OFFSET_79_L(BANK_BASE_ALBANY_2), reg_read);

	if (priv->ethpll_ictrl_type == ETHPLL_ICTRL_MT5896) {
		/* turn on tx always: 0422_79[8]=0 (16 bit) */
		reg_read = 0x0c;
		MTK_W8(REG_OFFSET_79_H(BANK_BASE_ALBANY_2), reg_read);
	} else {
		/* default settings */
		//wriu     0x0422f3             0x0d
		reg_read = 0x0d;
		MTK_W8(REG_OFFSET_79_H(BANK_BASE_ALBANY_2), reg_read);
	}

	//wriu     0x042079             0xd0
	reg_read = 0xd0;
	MTK_W8(REG_OFFSET_3C_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x042077             0x5a
	reg_read = 0x5a;
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_0), reg_read);

	if (priv->is_internal_phy) {
		//wriu     0x04202d             0x7c
		reg_read = 0x7c;
		MTK_W8(REG_OFFSET_16_H(BANK_BASE_ALBANY_0), reg_read);
	} else {
		//wriu     0x04202d             0x4c
		reg_read = 0x4c;
		MTK_W8(REG_OFFSET_16_H(BANK_BASE_ALBANY_0), reg_read);
	}

	//wriu     0x0422e8             0x06
	reg_read = 0x06;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04202b             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04202b             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x06
	reg_read = 0x06;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0420aa             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ac             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_56_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ad             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_56_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ae             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_57_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420af             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_57_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0420aa             0x19
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ab             0x28
	reg_read = 0x28;
	MTK_W8(REG_OFFSET_55_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0421f5             0x02
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7A_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04210f             0xc9
	reg_read = 0xc9;
	MTK_W8(REG_OFFSET_07_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042189             0x50
	reg_read = 0x50;
	MTK_W8(REG_OFFSET_44_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04218b             0x80
	reg_read = 0x80;
	MTK_W8(REG_OFFSET_45_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04218e             0x0e
	reg_read = 0x0e;
	MTK_W8(REG_OFFSET_47_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042190             0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_48_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422ad             0x0e
	//reg_read = 0x0e;
	//MTK_W8(REG_OFFSET_56_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422ae             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_57_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422af             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_57_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x0420b4              0x5a
	reg_read = 0x5a;
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x0420b6              0x50
	reg_read = 0x50;
	MTK_W8(REG_OFFSET_5B_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x0420ff              0x1a
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_7F_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x042293              0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_49_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x0422ec              0x10
	reg_read = 0x10;
	MTK_W8(REG_OFFSET_76_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x042294              0x18
	reg_read = 0x18;
	MTK_W8(REG_OFFSET_4A_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x042133              0x55
	reg_read = 0x55;
	MTK_W8(REG_OFFSET_19_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu    0x042110              0x56
	reg_read = 0x56;
	MTK_W8(REG_OFFSET_08_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422a1             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422a1             0x80
	reg_read = 0x80;
	MTK_W8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), reg_read);
}

static void mtk_dtv_gmac_phy_hw_init_22(struct mtk_dtv_gmac_priv *priv)
{
	u8 reg_read;

	//wriu     0x0421b4             0x02
	reg_read = 0x02;
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04214f             0x02
	reg_read = 0x02;
	MTK_W8(REG_OFFSET_27_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042151             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_28_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042177             0x18
	reg_read = 0x18;
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042072             0xa0
	reg_read = 0xa0;
	MTK_W8(REG_OFFSET_39_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0421fc             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7E_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0421fd             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_7E_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422a1             0x80
	reg_read = 0x80;
	MTK_W8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0421cc             0x40
	reg_read = 0x40;
	MTK_W8(REG_OFFSET_66_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0421bb             0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_5D_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04223a             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_1D_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f1             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_78_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04228a             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_45_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04213b             0x01
	reg_read = 0x01;
	MTK_W8(REG_OFFSET_1D_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0421c4             0x044
	reg_read = 0x44;
	MTK_W8(REG_OFFSET_62_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042280             0x30
	reg_read = 0x30;
	MTK_W8(REG_OFFSET_40_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422c5             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_62_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x042230             0x43
	reg_read = 0x43;
	MTK_W8(REG_OFFSET_18_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x042239             0x41
	reg_read = 0x41;
	MTK_W8(REG_OFFSET_1C_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f2             0xf5
	reg_read = 0xf5;
	MTK_W8(REG_OFFSET_79_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0422f3             0x0d
	reg_read = 0x0d;
	MTK_W8(REG_OFFSET_79_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x042079             0xd0
	reg_read = 0xd0;
	MTK_W8(REG_OFFSET_3C_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x042077             0x5a
	reg_read = 0x5a;
	MTK_W8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x04202d             0x7c
	reg_read = 0x7c;
	MTK_W8(REG_OFFSET_16_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x06
	reg_read = 0x06;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04202b             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x04202b             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x06
	reg_read = 0x06;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0420aa             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ac             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_56_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ad             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_56_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ae             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_57_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420af             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_57_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0422e8             0x00
	reg_read = 0x00;
	MTK_W8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu     0x0420aa             0x19
	reg_read = 0x19;
	MTK_W8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0420ab             0x28
	reg_read = 0x28;
	MTK_W8(REG_OFFSET_55_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu     0x0421f5             0x02
	reg_read = 0x02;
	MTK_W8(REG_OFFSET_7A_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04210f             0xc9
	reg_read = 0xc9;
	MTK_W8(REG_OFFSET_07_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042189             0x50
	reg_read = 0x50;
	MTK_W8(REG_OFFSET_44_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04218b             0x80
	reg_read = 0x80;
	MTK_W8(REG_OFFSET_45_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x04218e             0x0e
	reg_read = 0x0e;
	MTK_W8(REG_OFFSET_47_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x042190             0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_48_L(BANK_BASE_ALBANY_1), reg_read);

	//wriu     0x0422ad             0x0e
	//reg_read = 0x0e;
	//MTK_W8(REG_OFFSET_56_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x0420b4              0x5a
	reg_read = 0x5a;
	MTK_W8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x0420b6              0x50
	reg_read = 0x50;
	MTK_W8(REG_OFFSET_5B_L(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x0420ff              0x1a
	reg_read = 0x1a;
	MTK_W8(REG_OFFSET_7F_H(BANK_BASE_ALBANY_0), reg_read);

	//wriu    0x042293              0x04
	reg_read = 0x04;
	MTK_W8(REG_OFFSET_49_H(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x0422ec              0x10
	reg_read = 0x10;
	MTK_W8(REG_OFFSET_76_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x042294              0x18
	reg_read = 0x18;
	MTK_W8(REG_OFFSET_4A_L(BANK_BASE_ALBANY_2), reg_read);

	//wriu    0x042133              0x55
	reg_read = 0x55;
	MTK_W8(REG_OFFSET_19_H(BANK_BASE_ALBANY_1), reg_read);

	//wriu    0x042110              0x56
	reg_read = 0x56;
	MTK_W8(REG_OFFSET_08_L(BANK_BASE_ALBANY_1), reg_read);
}

static void mtk_dtv_gmac_phy_init(struct mtk_dtv_gmac_priv *priv)
{
	struct mtk_dtv_gmac_reg_ops *reg_ops = priv->reg_ops;
	u8 reg_read;

	/* (BK_103_3E4[3:0]) = 0, 0x1037c8 */
	reg_read = MTK_R8(REG_OFFSET_64_L(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x0F);
	MTK_W8(REG_OFFSET_64_L(BANK_BASE_CLKGEN_17), reg_read);
	/* (BK_103_3E4[9:8]) = 0, 0x1037c9 */
	reg_read = MTK_R8(REG_OFFSET_64_H(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x03);
	MTK_W8(REG_OFFSET_64_H(BANK_BASE_CLKGEN_17), reg_read);
	/* ??, 0x1037cc */
	reg_read = MTK_R8(REG_OFFSET_66_L(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x0F);
	MTK_W8(REG_OFFSET_66_L(BANK_BASE_CLKGEN_17), reg_read);
	/* (BK_103_3E6[9:8]) = 0, 0x1037cd */
	reg_read = MTK_R8(REG_OFFSET_66_H(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x03);
	MTK_W8(REG_OFFSET_66_H(BANK_BASE_CLKGEN_17), reg_read);
	/* ??, 0x1037d0 */
	reg_read = MTK_R8(REG_OFFSET_68_L(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x0F);
	MTK_W8(REG_OFFSET_68_L(BANK_BASE_CLKGEN_17), reg_read);
	/* (BK_103_3E8[9:8]) = 0, 0x1037d1 */
	reg_read = MTK_R8(REG_OFFSET_68_H(BANK_BASE_CLKGEN_17));
	reg_read &= ~(0x03);
	MTK_W8(REG_OFFSET_68_H(BANK_BASE_CLKGEN_17), reg_read);


	/* (BK_103_6AB[0]) = 1, 0x103d56 */
	reg_read = MTK_R8(REG_OFFSET_2B_L(BANK_BASE_CLKGEN_1D));
	reg_read |= 0x01;
	MTK_W8(REG_OFFSET_2B_L(BANK_BASE_CLKGEN_1D), reg_read);
	//reg_read = 0x00;
	//MTK_W8(REG_OFFSET_2B_H(BANK_BASE_CLKGEN_1D), reg_read);
	/* ??, 0x103d58 */
	reg_read = MTK_R8(REG_OFFSET_2C_L(BANK_BASE_CLKGEN_1D));
	reg_read |= 0x01;
	MTK_W8(REG_OFFSET_2C_L(BANK_BASE_CLKGEN_1D), reg_read);
	//reg_read = 0x00;
	//MTK_W8(REG_OFFSET_2C_H(BANK_BASE_CLKGEN_1D), reg_read);
	/* ??, 0x103d5a */
	reg_read = MTK_R8(REG_OFFSET_2D_L(BANK_BASE_CLKGEN_1D));
	reg_read |= 0x01;
	MTK_W8(REG_OFFSET_2D_L(BANK_BASE_CLKGEN_1D), reg_read);
	//reg_read = 0x00;
	//MTK_W8(REG_OFFSET_2D_H(BANK_BASE_CLKGEN_1D), reg_read);
	/* ??, 0x103d5c */
	reg_read = MTK_R8(REG_OFFSET_2E_L(BANK_BASE_CLKGEN_1D));
	reg_read |= 0x01;
	MTK_W8(REG_OFFSET_2E_L(BANK_BASE_CLKGEN_1D), reg_read);
	//reg_read = 0x00;
	//MTK_W8(REG_OFFSET_2E_H(BANK_BASE_CLKGEN_1D), reg_read);
	/* ??, 0x102aac */
	reg_read = MTK_R8(REG_OFFSET_56_L(BANK_BASE_CLKGEN_0A));
	reg_read |= 0x04;
	MTK_W8(REG_OFFSET_56_L(BANK_BASE_CLKGEN_0A), reg_read);
	//reg_read = 0x00;
	//MTK_W8(REG_OFFSET_56_H(BANK_BASE_CLKGEN_0A), reg_read);
	/* ??, 0x102140 */
	reg_read = MTK_R8(REG_OFFSET_20_L(BANK_BASE_CLKGEN_01));
	reg_read &= ~(0x03);
	MTK_W8(REG_OFFSET_20_L(BANK_BASE_CLKGEN_01), reg_read);
	/* ??, 0x102141 */
	reg_read = MTK_R8(REG_OFFSET_20_H(BANK_BASE_CLKGEN_01));
	reg_read &= ~(0x03);
	MTK_W8(REG_OFFSET_20_H(BANK_BASE_CLKGEN_01), reg_read);
	/* ??, 0x103dbb */
	reg_read = MTK_R8(REG_OFFSET_5D_H(BANK_BASE_CLKGEN_1D));
	reg_read |= 0x2;
	MTK_W8(REG_OFFSET_5D_H(BANK_BASE_CLKGEN_1D), reg_read);
	/* ??, 0x102a73 */
	reg_read = MTK_R8(REG_OFFSET_39_H(BANK_BASE_CLKGEN_0A));
	reg_read |= 0x10;
	MTK_W8(REG_OFFSET_39_H(BANK_BASE_CLKGEN_0A), reg_read);

	if (priv->is_internal_phy) {
		/* BK_0C_36[3:0] = 0x0, 0x000c6c */
		reg_read = MTK_R8(REG_OFFSET_36_L(BANK_BASE_CLKGEN_PM_0));
		reg_read &= ~(0x0F);
		MTK_W8(REG_OFFSET_36_L(BANK_BASE_CLKGEN_PM_0), reg_read);

		/* BK_0C_35[3:0] = 0x0, 0x000c6a */
		reg_read = MTK_R8(REG_OFFSET_35_L(BANK_BASE_CLKGEN_PM_0));
		reg_read &= ~(0x0F);
		MTK_W8(REG_OFFSET_35_L(BANK_BASE_CLKGEN_PM_0), reg_read);
	} else {
		/* BK_0C_36[3:0] = 0x4, 0x000c6c */
		reg_read = MTK_R8(REG_OFFSET_36_L(BANK_BASE_CLKGEN_PM_0));
		reg_read &= ~(0x0F);
		reg_read |= 0x04;
		MTK_W8(REG_OFFSET_36_L(BANK_BASE_CLKGEN_PM_0), reg_read);

		/* BK_0C_35[3:0] = 0x4, 0x000c6a */
		reg_read = MTK_R8(REG_OFFSET_35_L(BANK_BASE_CLKGEN_PM_0));
		reg_read &= ~(0x0F);
		reg_read |= 0x04;
		MTK_W8(REG_OFFSET_35_L(BANK_BASE_CLKGEN_PM_0), reg_read);
	}

	reg_ops->phy_hw_init(priv);
}

u32 _mtk_dtv_gmac_efuse_read(struct mtk_dtv_gmac_priv *priv, u16 addr)
{
	u16 efuse_addr = addr;
	u32 efuse_value = 0;
	int retry = 0;

	/* trigger read */
	efuse_addr |= EFUSE_TRIG_READ;
	MTK_W16(REG_OFFSET_28_L(BANK_BASE_EFUSE), efuse_addr);

	do {
		if ((efuse_addr & ~EFUSE_TRIG_READ) != addr) {
			efuse_addr = addr;
			efuse_addr |= EFUSE_TRIG_READ;
			MTK_W16(REG_OFFSET_28_L(BANK_BASE_EFUSE), efuse_addr);
		}

		efuse_addr = MTK_R16(REG_OFFSET_28_L(BANK_BASE_EFUSE));
		retry++;
		if (retry >= EFUSE_RETRY_MAX) {
			printf("gmac read efuse failed, retry=%d\n",
			       retry);
			return 0;
		}
	} while ((efuse_addr & EFUSE_TRIG_READ) || ((efuse_addr & ~EFUSE_TRIG_READ) != addr));

	/* get efuse value */
	efuse_value = (MTK_R16(REG_OFFSET_2D_L(BANK_BASE_EFUSE)) << 16) |
		      (MTK_R16(REG_OFFSET_2C_L(BANK_BASE_EFUSE)));

	return efuse_value;
}

static void mtk_dtv_gmac_tx_swing_init(struct mtk_dtv_gmac_priv *priv)
{
	u32 eth_efuse;
	u8 reg_read, level, reg_write;

	if (priv->tx_swing_level) {
		eth_efuse = _mtk_dtv_gmac_efuse_read(priv, EFUSE_ETH_ATOP_ADDR);
		UBOOT_DEBUG("gmac eth_efuse=0x%x, tx_swing_level=0x%x\n",
			    eth_efuse, priv->tx_swing_level);

		if (eth_efuse & EFUSE_ETH_ATOP_ACTIVE) {
			reg_read = ((eth_efuse & EFUSE_ETH_ATOP_CUR_100T_MASK) >>
				    BIT_ETH_ATOP_CUR_100T);
			level = priv->tx_swing_level & EFUSE_ETH_ATOP_CUR_100T_MASK;
			if (reg_read & 0x10) {
				if ((reg_read - level) < TX_SWING_LEVEL_MAX)
					reg_write = (TX_SWING_LEVEL_MAX - 1) + level - reg_read;
				else
					reg_write = reg_read - level;
			} else {
				if ((reg_read + level) > (TX_SWING_LEVEL_MAX - 1))
					reg_write = (TX_SWING_LEVEL_MAX - 1);
				else
					reg_write = reg_read + level;
			}
			reg_write &= EFUSE_ETH_ATOP_CUR_100T_MASK;
			reg_write |= (MTK_R8(REG_OFFSET_34_L(BANK_BASE_ALBANY_2)) &
				      ~EFUSE_ETH_ATOP_CUR_100T_MASK);
			MTK_W8(REG_OFFSET_34_L(BANK_BASE_ALBANY_2), reg_write);

			UBOOT_DEBUG("gmac tx swing read=0x%x, level=0x%x, write=0x%x\n",
				    reg_read, level, reg_write);
		}
	}
}

/* scan phy address */
static int mtk_dtv_gmac_phy_addr_scan(struct mtk_dtv_gmac_priv *priv)
{
	struct mtk_dtv_gmac_reg_ops *reg_ops = priv->reg_ops;
	u8 addr = 0;
	u32 val;

	do {
		val = reg_ops->read_phy(priv, addr, MII_BMSR);
		if ((val != 0) && (val != 0xffff)) {
			UBOOT_DEBUG("phy addr(%d)\n", addr);
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

static void mtk_dtv_gmac_hw_patch(struct mtk_dtv_gmac_priv *priv)
{
	if (priv->mac_type == MAC_TYPE_EMAC) {
		MTK_W8(REG_OFFSET_1B_L(BANK_BASE_MAC_1), 0x44);
		MTK_W8(REG_OFFSET_1B_H(BANK_BASE_MAC_1), 0x32);
		MTK_W8(REG_OFFSET_1C_L(BANK_BASE_MAC_1), 0x05);
		MTK_W8(REG_OFFSET_1C_H(BANK_BASE_MAC_1), 0x01);
	}
}

/* phy link status */
/* TODO: speed 1G */
static int mtk_dtv_gmac_link_status_update(struct mtk_dtv_gmac_priv *priv)
{
	struct mtk_dtv_gmac_reg_ops *reg_ops = priv->reg_ops;
	u32 bmsr, bmcr, lpa, adv, neg, speed, duplex, reg;
	u32 hcd_link_st_ok;

	/* latch link status bit to 1 */
	bmsr = reg_ops->read_phy(priv, priv->phy_addr, MII_BMSR);
	bmsr |= 0x4UL;
	reg_ops->write_phy(priv, priv->phy_addr, MII_BMSR, bmsr);
	bmsr = reg_ops->read_phy(priv, priv->phy_addr, MII_BMSR);
	/* check hcd link status */
	hcd_link_st_ok = reg_ops->read_phy(priv, priv->phy_addr, 0x21);
	if (!(hcd_link_st_ok & 0x100UL)) {
		/* link down */
		printf("link down: status=%#010x\n", hcd_link_st_ok);

		return -1;
	}

	bmcr = reg_ops->read_phy(priv, priv->phy_addr, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		/* auto-negotiation */
		if (!(bmsr & BMSR_ANEGCOMPLETE)) {
			/* link down */
			printf("auto-negotiation still running\n");
			printf("link down: bmcr=%#010x, bmsr=%#010x\n", bmcr, bmsr);

			return -2;
		}

		if (!priv->is_internal_phy) {
			/* external gphy */
			reg = reg_ops->read_phy(priv, priv->phy_addr, MII_STAT1000);
			if ((reg & LPA_1000FULL) || (reg & LPA_1000HALF)) {
				speed = SPEED_1000;
				duplex = DUPLEX_FULL;
				printf("link_stat: 1000 full\n");
				goto link_status_update_to_hw;
			}
		}

		/* get link partner and advertisement from the phy not from the mac */
		adv = reg_ops->read_phy(priv, priv->phy_addr, MII_ADVERTISE);
		lpa = reg_ops->read_phy(priv, priv->phy_addr, MII_LPA);

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
			printf("no speed and mode found, lpa=%#x, adv=%#x\n",
					lpa, adv);
		}
	} else {
		speed = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
		duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	}

link_status_update_to_hw:
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

	if (priv->is_internal_phy == true) {
		if (priv->mac_type == MAC_TYPE_GMAC) {
			/* set xmii_type to TGMII */
			MTK_W8(REG_OFFSET_30_L(BANK_BASE_MAC_4), 0x02);
			MTK_W8(REG_OFFSET_30_H(BANK_BASE_MAC_4), 0x01);
		}
	} else {
		if (speed == SPEED_1000) {
			/* set xmii_type to RGMII 1000M */
			if (priv->mac_type == MAC_TYPE_GMAC) {
				MTK_W8(REG_OFFSET_30_L(BANK_BASE_MAC_4), 0x00);
				MTK_W8(REG_OFFSET_30_H(BANK_BASE_MAC_4), 0x01);
			}
			MTK_W8(REG_OFFSET_32_L(BANK_BASE_ALBANY_3), 0x0c);
			MTK_W8(REG_OFFSET_32_H(BANK_BASE_ALBANY_3), 0x20);
			MTK_W8(REG_OFFSET_33_L(BANK_BASE_ALBANY_3), 0x3e);
			MTK_W8(REG_OFFSET_33_H(BANK_BASE_ALBANY_3), 0x00);
		} else {
			/* set xmii_type to RGMII 100/10M */
			if (priv->mac_type == MAC_TYPE_GMAC) {
				MTK_W8(REG_OFFSET_30_L(BANK_BASE_MAC_4), 0x01);
				MTK_W8(REG_OFFSET_30_H(BANK_BASE_MAC_4), 0x01);
			}
			if (priv->is_fpga_haps != true) {
				MTK_W8(REG_OFFSET_32_L(BANK_BASE_ALBANY_3), 0x0e);
				MTK_W8(REG_OFFSET_32_H(BANK_BASE_ALBANY_3), 0x20);

				if (speed == SPEED_100)
					MTK_W8(REG_OFFSET_33_L(BANK_BASE_ALBANY_3), 0x3e);
				else
					MTK_W8(REG_OFFSET_33_L(BANK_BASE_ALBANY_3), 0x1e);
				MTK_W8(REG_OFFSET_33_H(BANK_BASE_ALBANY_3), 0x00);
			}
		}
	}

	/* link up */
	printf("link up: speed=%#x, duplex=%#x\n", speed, duplex);

	return 0;
}

/* mac init */
static int mtk_dtv_gmac_mac_init(struct mtk_dtv_gmac_priv *priv)
{
	struct mtk_dtv_gmac_reg_ops *reg_ops = priv->reg_ops;
	int ret;
	u32 reg_read;

	/* phy init */
	mtk_dtv_gmac_tx_swing_init(priv);
	mtk_dtv_gmac_phy_init(priv);

	if (priv->is_internal_phy == true) {
		/* use internal phy */
		MTK_W8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x11);
		MTK_W8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);
	} else {
		/* use external phy */
		if (priv->mac_type == MAC_TYPE_EMAC) {
			MTK_W8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x11);
			MTK_W8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);
		} else {
			MTK_W8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x17);
			MTK_W8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);
		}
	}

	/* scan phy address */
	ret = mtk_dtv_gmac_phy_addr_scan(priv);
	if (ret) {
		printf("scan phy failed\n");
		return -1;
	}

	if (priv->is_force_10m == true) {
		/* force speed to 10M full */
		reg_ops->write_phy(priv, priv->phy_addr, MII_BMCR, BMCR_RESET);
		mdelay(10);
		reg_ops->write_phy(priv, priv->phy_addr, MII_ADVERTISE, ADVERTISE_CSMA|ADVERTISE_10HALF|ADVERTISE_10FULL);
		reg_ops->write_phy(priv, priv->phy_addr, MII_CTRL1000, 0x0);
		mdelay(10);
		reg_ops->write_phy(priv, priv->phy_addr, MII_BMCR, BMCR_ANENABLE|BMCR_ANRESTART);
		printf("force speed to 10M full\n");
	}

	if (priv->is_force_100m == true) {
		/* force speed to 100M full */
		reg_ops->write_phy(priv, priv->phy_addr, MII_BMCR, BMCR_RESET);
		mdelay(10);
		reg_ops->write_phy(priv, priv->phy_addr, MII_ADVERTISE, ADVERTISE_CSMA|ADVERTISE_100HALF|ADVERTISE_100FULL);
		reg_ops->write_phy(priv, priv->phy_addr, MII_CTRL1000, 0x0);
		mdelay(10);
		reg_ops->write_phy(priv, priv->phy_addr, MII_BMCR, BMCR_ANENABLE|BMCR_ANRESTART);
		printf("force speed to 100M full\n");
	}

	if (priv->mac_type == MAC_TYPE_EMAC) {
		/* swith rx discriptor format to mode 1 */
		MTK_W8(REG_OFFSET_1D_L(BANK_BASE_MAC_1), 0x00);
		MTK_W8(REG_OFFSET_1D_H(BANK_BASE_MAC_1), 0x01);
	}

	/* 0x8f */
	MTK_W8(REG_OFFSET_01_L(BANK_BASE_MAC_1), 0x08);
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
	mtk_dtv_gmac_hw_patch(priv);

	return 0;
}


static int mtk_dtv_gmac_start(struct udevice *dev)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	int ret;
	u32 reg_read;
	u8 reg_irq;
	int retry = 5;

	/* update link status */
	do {
		ret = mtk_dtv_gmac_link_status_update(priv);
		if (ret == -2)
			printf("retry link %d\n", retry);
		else
			break;

		retry--;
		mdelay(1000);
	} while (retry > 0);

	if (ret) {
		printf("update link status failed\n");
		return -1;
	}

	/* fifo init */
	mtk_dtv_gmac_fifo_init(priv);

	/* clear hardware statistic */
	reg_read = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_read |= 0x20;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_read);

	/* disable all irq first */
	MTK_W32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
	reg_irq = MTK_R8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
	reg_irq &= 0x7f;
	MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
	/* enable irq, irq is useless in uboot */
	//MTK_W32(REG_OFFSET_14_L(BANK_BASE_MAC_0), IRQ_ENABLE_BIT);
	//reg_irq = MTK_R8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
	//reg_irq |= 0x80;
	//MTK_W8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

	/* enable hw tx rx */
	reg_read = MTK_R32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
	reg_read |= 0x0c;
	MTK_W32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_read);

	return 0;
}

static int mtk_dtv_gmac_send(struct udevice *dev, void *packet, int length)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	u32 tsr_val, cnt = 0, dma_addr;
	u8 tx_fifo[8] = {0};
	u8 i, tx_fifo_token = 0;
	void *pkt_base;
	u8 reg_read;
	unsigned long pa_addr = 0;

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

	pa_addr = virt_to_phys(pkt_base) - MIU0_BUS_BASE;
	dma_addr = pa_addr & 0xffffffff;

	//printf("tx packet dma: 0x%p\n", dma_addr);
	//mtk_dtv_gmac_pkt_dump(pkt_base, length);

	/* over 0-31bits, config 32-33bits */
	if (priv->mac_type == MAC_TYPE_EMAC) {
		if (pa_addr & 0x300000000) {
			reg_read = MTK_R8(REG_OFFSET_23_H(BANK_BASE_MAC_1));
			reg_read &= ~(0x3 << EMAC_TX_ADDR_MSB2_BIT_SHIFT);
			reg_read |= (((pa_addr & 0x300000000) >> 32) << EMAC_TX_ADDR_MSB2_BIT_SHIFT);
			MTK_W8(REG_OFFSET_23_H(BANK_BASE_MAC_1), reg_read);
		}
	} else {
		if (pa_addr & 0x300000000) {
			reg_read = MTK_R8(REG_OFFSET_09_H(BANK_BASE_MAC_4));
			reg_read &= ~(0x3 << GMAC_TX_ADDR_MSB2_BIT_SHIFT);
			reg_read |= (((pa_addr & 0x300000000) >> 32) << GMAC_TX_ADDR_MSB2_BIT_SHIFT);
			MTK_W8(REG_OFFSET_09_H(BANK_BASE_MAC_4), reg_read);
		}
	}

	MTK_W32(REG_OFFSET_06_L(BANK_BASE_MAC_0), dma_addr);
	MTK_W32(REG_OFFSET_08_L(BANK_BASE_MAC_0), length);

	return 0;
}

static int mtk_dtv_gmac_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	int idx = priv->rx_ring_idx;
	uchar *pkt_base;
	u32 length;
	unsigned long pa_addr, temp;

	if (priv->mac_type == MAC_TYPE_EMAC) {
		struct rx_desc_emac *rx_ring = priv->rx_ring;

		if (!(rx_ring[idx].addr & RX_DESC_DONE)) {
			//printf("rx ring is empty: %d\n", idx);
			return -EAGAIN;
		}

		length = (rx_ring[idx].size & RX_DESC_SIZE);
		pa_addr = rx_ring[idx].addr & ~(RX_DESC_DONE | RX_DESC_WRAP);

		if (rx_ring[idx].size & (0x3 << EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT)) {
			/* over 0-31bits, confiig 32-33bits */
			temp = (rx_ring[idx].size &
				(0x3 << EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT)) >>
			       EMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT;
			temp <<= 32;
			pa_addr += temp;
		}
	} else {
		struct rx_desc_gmac *rx_ring = priv->rx_ring;

		if (!(rx_ring[idx].addr & RX_DESC_DONE)) {
			//printf("rx ring is empty: %d\n", idx);
			return -EAGAIN;
		}

		length = (rx_ring[idx].size & RX_DESC_SIZE);
		pa_addr = rx_ring[idx].addr & ~(RX_DESC_DONE | RX_DESC_WRAP);

		if (rx_ring[idx].size_high & (0x3 << GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT)) {
			/* over 0-31bits, confiig 32-33bits */
			temp = (rx_ring[idx].size_high &
				(0x3 << GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT)) >>
			       GMAC_RXDES_ADDR_MSB2_FRAME_BIT_SHIFT;
			temp <<= 32;
			pa_addr += temp;
		}
	}

	pkt_base = (void *)(phys_to_virt(pa_addr + MIU0_BUS_BASE));
	invalidate_dcache_range((ulong)pkt_base, (ulong)pkt_base +
				roundup(length, ARCH_DMA_MINALIGN));

	//printf("rx packet: 0x%lx, pa: 0x%lx\n", pkt_base, pa_addr);
	//mtk_dtv_gmac_pkt_dump(pkt_base, length);

	if (packetp)
		*packetp = pkt_base;

	return length;
}

static void mtk_dtv_gmac_stop(struct udevice *dev)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
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

static int mtk_dtv_gmac_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	int idx = priv->rx_ring_idx;

	flush_dcache_range((ulong)packet, (ulong)(packet + PKTSIZE_ALIGN));
	if (priv->mac_type == MAC_TYPE_EMAC) {
		struct rx_desc_emac *rx_ring = priv->rx_ring;
		rx_ring[idx].addr &= ~RX_DESC_DONE;
	} else {
		struct rx_desc_gmac *rx_ring = priv->rx_ring;
		rx_ring[idx].addr &= ~RX_DESC_DONE;
	}
	priv->rx_ring_idx = ((priv->rx_ring_idx + 1) % NUM_RX_DESC);

	return 0;
}

static int mtk_dtv_gmac_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 val, reg_read;

	UBOOT_DEBUG(" mac address: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	val = (((u32)mac[3] << 24) | ((u32)mac[2] << 16) |
			((u32)mac[1] << 8) | ((u32)mac[0]));
	MTK_W32(REG_OFFSET_4C_L(BANK_BASE_MAC_0), val);
	reg_read = MTK_R32(REG_OFFSET_4E_L(BANK_BASE_MAC_0));
	val = ((((u32)mac[5] << 8) | ((u32)mac[4])) & 0xffff) |
			(reg_read & 0xffff0000);
	MTK_W32(REG_OFFSET_4E_L(BANK_BASE_MAC_0), val);

	return 0;
}

static int mtk_dtv_gmac_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct network_info net_info;
	int ret = 0, i;
	unsigned char mac[6];
#ifdef CONFIG_DATA_SEPARATION
	char filepath[FILE_PATH_SIZE], part[PART_NAME_SIZE];
	const char *relpath = NULL;
#endif

	memset(&net_info, 0, sizeof(net_info));
#ifdef CONFIG_DATA_SEPARATION
	memset(part, 0, PART_NAME_SIZE);
	memset(filepath, 0, FILE_PATH_SIZE);
	if(dataindex_get_key(filepath, FILE_PATH_SIZE, NETWORK_SECTION, NETWORK_KEY, NULL) == 0)
	{
		if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
		{
			ret = get_network_info(part, relpath, &net_info);
			if(ret < 0)
			{
				UBOOT_DEBUG("Read ini information failure with data separation.[%s][%s]\n",part,relpath);
				ret = get_network_info(DEFAULT_NETWORK_PART, DEFAULT_NETWORK_INI, &net_info);
			}
		}
		else
		{
			UBOOT_DEBUG("resolve path fail: %s",filepath);
			ret = get_network_info(DEFAULT_NETWORK_PART, DEFAULT_NETWORK_INI, &net_info);
		}
	}
	else
	{
		UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",NETWORK_SECTION,NETWORK_KEY);
#endif
		ret = get_network_info(DEFAULT_NETWORK_PART, DEFAULT_NETWORK_INI, &net_info);
#ifdef CONFIG_DATA_SEPARATION
	}
#endif

	if (ret < 0) {
		UBOOT_DEBUG("cannot gmac read network.ini with default setting\n");
		return -EINVAL;
	}

	eth_parse_enetaddr(net_info.ethaddr, mac);
	if (is_valid_ethaddr(mac)) {
		UBOOT_DEBUG("ethaddr from network.ini is %s\n", net_info.ethaddr);
	} else {
		UBOOT_DEBUG("wrong ethaddr from network.ini (%s)\n", net_info.ethaddr);
		return -EINVAL;
	}

	for (i = 0; i < 6; i++)
		pdata->enetaddr[i] = mac[i];

	return 0;
}

static int mtk_dtv_gmac_probe(struct udevice *dev)
{
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	struct mtk_dtv_gmac_reg_ops *reg_ops = NULL;
	int ret;

	/* reg ops init */
	reg_ops = malloc(sizeof(struct mtk_dtv_gmac_reg_ops));
	if (!reg_ops) {
		printf("gmac malloc failed\n");
		return -ENOMEM;
	}

	if (priv->is_internal_phy == true) {
		UBOOT_DEBUG("gmac use internal phy\n");
		reg_ops->write_phy = mtk_dtv_gmac_phy_write_internal;
		reg_ops->read_phy = mtk_dtv_gmac_phy_read_internal;
	} else {
		UBOOT_DEBUG("gmac use external phy\n");
		reg_ops->write_phy = mtk_dtv_gmac_phy_write_external;
		reg_ops->read_phy = mtk_dtv_gmac_phy_read_external;
	}

	switch (priv->phy_process) {
	case PHY_PROCESS_7_12:
		reg_ops->phy_hw_init = mtk_dtv_gmac_phy_hw_init_7_12;
		UBOOT_DEBUG("gmac set phy init to 7_12\n");
		break;
	case PHY_PROCESS_22:
		reg_ops->phy_hw_init = mtk_dtv_gmac_phy_hw_init_22;
		UBOOT_DEBUG("gmac set phy init to 22\n");
		break;
	default:
		reg_ops->phy_hw_init = mtk_dtv_gmac_phy_hw_init_7_12;
		UBOOT_DEBUG("gmac unknown phy_process(%d), set phy init to default 7_12\n",
			    priv->phy_process);
		break;
	};

	priv->reg_ops = reg_ops;

	if (!priv->reg_ops) {
		printf("gmac null reg_ops\n");
		return -EINVAL;
	}

	/* memory init */
	mtk_dtv_gmac_mem_init(priv);

	/* mac init */
	ret = mtk_dtv_gmac_mac_init(priv);
	if (ret != 0) {
		printf("gmac init failed\n");
		if (priv->reg_ops)
			free(priv->reg_ops);
		return -ENODEV;
	}

	return 0;
}

static const struct eth_ops mtk_dtv_gmac_ops = {
	.start			= mtk_dtv_gmac_start,
	.send			= mtk_dtv_gmac_send,
	.recv			= mtk_dtv_gmac_recv,
	.stop			= mtk_dtv_gmac_stop,
	.free_pkt		= mtk_dtv_gmac_free_pkt,
	.write_hwaddr	= mtk_dtv_gmac_write_hwaddr,
	.read_rom_hwaddr	= mtk_dtv_gmac_read_rom_hwaddr,
};

static int mtk_dtv_gmac_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct mtk_dtv_gmac_priv *priv = dev_get_priv(dev);
	u32 get_val = 0;

	pdata->iobase = dev_read_addr_index(dev, 0);
	priv->mac_bank_0 = (void *)dev_read_addr_name(dev, "mac0");
	priv->mac_bank_1 = (void *)dev_read_addr_name(dev, "mac1");
	priv->albany_bank_0 = (void *)dev_read_addr_name(dev, "phy0");
	priv->clkgen_bank_0 = (void *)dev_read_addr_name(dev, "clkgen0");
	priv->chip_bank = (void *)dev_read_addr_name(dev, "chip");
	priv->clkgen_pm_bank_0 = (void *)dev_read_addr_name(dev, "clkgen0_pm");
	priv->efuse_bank = (void *)dev_read_addr_name(dev, "efuse");

	if (dev_read_u32(dev, "phy-type", &get_val)) {
		priv->phy_type = PHY_TYPE_INTERNAL;
		UBOOT_DEBUG("gmac set phy_type to default value (%d)\n",
			    priv->phy_type);
	} else {
		if (get_val < 0) {
			UBOOT_DEBUG("gmac bad phy_type(%d), set it to default value\n",
				    get_val);
			priv->phy_type = PHY_TYPE_INTERNAL;
		} else {
			priv->phy_type = get_val;
			UBOOT_DEBUG("gmac phy_type (%d)\n",
				    priv->phy_type);
		}
	}

	if (priv->phy_type == PHY_TYPE_EXTERNAL) {
		priv->is_internal_phy = false;
	} else {
		priv->is_internal_phy = true;
	}
	priv->is_force_10m = dev_read_bool(dev, "force-10m");
	priv->is_force_100m = dev_read_bool(dev, "force-100m");
	priv->is_fpga_haps = dev_read_bool(dev, "fpga-haps");

	if (dev_read_u32(dev, "ethpll-ictrl", &get_val)) {
		priv->ethpll_ictrl_type = ETHPLL_ICTRL_MT5896;
		UBOOT_DEBUG("gmac set ethpll_ictrl_type to default value (%d)\n",
			    priv->ethpll_ictrl_type);
	} else {
		if (get_val < 0) {
			UBOOT_DEBUG("gmac bad ethpll_ictrl_type(%d), set it to default value\n",
				    get_val);
			priv->ethpll_ictrl_type = ETHPLL_ICTRL_MT5896;
		} else {
			priv->ethpll_ictrl_type = get_val;
			UBOOT_DEBUG("gmac ethpll_ictrl_type (%d)\n",
				    priv->ethpll_ictrl_type);
		}
	}

	get_val = 0;
	if (dev_read_u32(dev, "mac-type", &get_val)) {
		priv->mac_type = MAC_TYPE_GMAC;
		UBOOT_DEBUG("gmac set mac_type to default value (%d)\n",
			    priv->mac_type);
	} else {
		if (get_val < 0) {
			UBOOT_DEBUG("gmac bad mac_type(%d), set it to default value\n",
				    get_val);
			priv->mac_type = MAC_TYPE_GMAC;
		} else {
			priv->mac_type = get_val;
			UBOOT_DEBUG("gmac mac_type (%d)\n",
				    priv->mac_type);
		}
	}

	get_val = 0;
	if (dev_read_u32(dev, "phy-process", &get_val)) {
		priv->phy_process = PHY_PROCESS_7_12;
		UBOOT_DEBUG("gmac set phy_process to default value (%d)\n",
			    priv->phy_process);
	} else {
		if (get_val < 0) {
			UBOOT_DEBUG("gmac bad phy_process(%d), set it to default value\n",
				    get_val);
			priv->phy_process = PHY_PROCESS_7_12;
		} else {
			priv->phy_process = get_val;
			UBOOT_DEBUG("gmac phy_process (%d)\n",
				    priv->phy_process);
		}
	}

	/* get tx swing property */
	if (dev_read_u32(dev, "tx-swing", &priv->tx_swing_level)) {
		priv->tx_swing_level = 0;
		UBOOT_DEBUG("gmac set tx_swing_level to default value (%d)\n",
			    priv->tx_swing_level);
	} else {
		if (priv->tx_swing_level > TX_SWING_LEVEL_MAX) {
			UBOOT_DEBUG("gmac bad tx_swing_level(%d), set it to default value 0\n",
				    priv->tx_swing_level);
			priv->tx_swing_level = 0;
		}
		UBOOT_DEBUG("gmac tx_swing_level (%d)\n",
			    priv->tx_swing_level);
	}

	//printf("\x1b[37;41m [%s %d] iobase=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, pdata->iobase);
	//printf("\x1b[37;41m [%s %d] mac_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->mac_bank_0);
	//printf("\x1b[37;41m [%s %d] mac_bank_1=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->mac_bank_1);
	//printf("\x1b[37;41m [%s %d] albany_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->albany_bank_0);
	//printf("\x1b[37;41m [%s %d] clkgen_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->clkgen_bank_0);
	//printf("\x1b[37;41m [%s %d] chip_bank=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->chip_bank);
	//printf("\x1b[37;41m [%s %d] clkgen_bank_0=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->clkgen_pm_bank_0);
	//printf("\x1b[37;41m [%s %d] efuse_bank=%#010x\x1b[0m\n ", __FUNCTION__, __LINE__, priv->efuse_bank);

	return 0;
}

static const struct udevice_id mtk_dtv_gmac_ids[] = {
	{ .compatible = "mediatek,mtk-dtv-gmac" },
	{ }
};

U_BOOT_DRIVER(mtk_dtv_gmac) = {
	.name	= "mtk_dtv_gmac",
	.id	= UCLASS_ETH,
	.of_match = mtk_dtv_gmac_ids,
	.ofdata_to_platdata = mtk_dtv_gmac_ofdata_to_platdata,
	.probe	= mtk_dtv_gmac_probe,
	.ops	= &mtk_dtv_gmac_ops,
	.priv_auto_alloc_size = sizeof(struct mtk_dtv_gmac_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
