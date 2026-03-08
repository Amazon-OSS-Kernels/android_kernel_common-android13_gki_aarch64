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
//#include <dm/device_compat.h>
//#include <dm/devres.h>

#include <dt-bindings/phy/phy.h>

/* version V1 sub-banks offset base address */
/* banks shared by multiple phys */
#define SSUSB_SIFSLV_V1_SPLLC		0x000	/* shared by u3 phys */
#define SSUSB_SIFSLV_V1_U2FREQ		0x100	/* shared by u2 phys */
#define SSUSB_SIFSLV_V1_CHIP		0x300	/* shared by u3 phys */
/* u2 phy bank */
#define SSUSB_SIFSLV_V1_U2PHY_COM	0x000
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V1_U3PHYD		0x000
#define SSUSB_SIFSLV_V1_U3PHYA		0x200

/* version V2 sub-banks offset base address */
/* u2 phy banks */
#define SSUSB_SIFSLV_V2_MISC		0x000
#define SSUSB_SIFSLV_V2_U2FREQ		0x100
#define SSUSB_SIFSLV_V2_U2PHY_COM	0x300
/* u3/pcie/sata phy banks */
#define SSUSB_SIFSLV_V2_SPLLC		0x000
#define SSUSB_SIFSLV_V2_CHIP		0x100
#define SSUSB_SIFSLV_V2_U3PHYD		0x200
#define SSUSB_SIFSLV_V2_U3PHYA		0x400

#define U3P_MISC_REG0			0x000
#define PA0_RG_U2_PLL_CTRL		GENMASK(23, 16)
#define PA0_RG_U2_PLL_CTRL_VAL(x)	((0xff & (x)) << 16)

#define U3P_USBPHYACR0			0x000
#define PA0_RG_U2PLL_FORCE_ON		BIT(15)
#define PA0_RG_USB20_INTR_EN		BIT(5)

#define U3P_USBPHYACR4			0x010
#define PA4_RG_USB20_LS_SR		GENMASK(6, 4)
#define PA4_RG_USB20_LS_SR_VAL(x)	((0x7 & (x)) << 4)

#define U3P_USBPHYACR5			0x014
#define PA5_RG_U2_HSTX_SRCAL_EN		BIT(15)
#define PA5_RG_U2_HSTX_SRCTRL		GENMASK(14, 12)
#define PA5_RG_U2_HSTX_SRCTRL_VAL(x)	((0x7 & (x)) << 12)
#define PA5_RG_U2_HS_100U_U3_EN		BIT(11)

#define U3P_USBPHYACR6			0x018
#define PA6_RG_U2_BC11_SW_EN		BIT(23)
#define PA6_RG_U2_OTG_VBUSCMP_EN	BIT(20)
#define PA6_RG_U2_SQTH			GENMASK(3, 0)
#define PA6_RG_U2_SQTH_VAL(x)		(0xf & (x))
#define PA6_RG_U2_PHY_REV		GENMASK(31, 24)
#define PA6_RG_U2_PHY_REV_VAL(x)	((0xff & (x)) << 24)
#define PA6_RG_U2_PRE_EMPHASIS		GENMASK(31, 30)
#define PA6_RG_U2_PRE_EMPHASIS_VAL(x)	((0x3 & (x)) << 30)

#define U3P_U2PHYACR4			0x020
#define P2C_RG_USB20_GPIO_CTL		BIT(9)
#define P2C_USB20_GPIO_MODE		BIT(8)
#define P2C_U2_GPIO_CTR_MSK	\
		(P2C_RG_USB20_GPIO_CTL | P2C_USB20_GPIO_MODE)

#define U3P_U2PHYDTM0			0x068
#define P2C_FORCE_UART_EN		BIT(26)
#define P2C_FORCE_DATAIN		BIT(23)
#define P2C_FORCE_DM_PULLDOWN		BIT(21)
#define P2C_FORCE_DP_PULLDOWN		BIT(20)
#define P2C_FORCE_XCVRSEL		BIT(19)
#define P2C_FORCE_SUSPENDM		BIT(18)
#define P2C_FORCE_TERMSEL		BIT(17)
#define P2C_RG_DATAIN			GENMASK(13, 10)
#define P2C_RG_DATAIN_VAL(x)		((0xf & (x)) << 10)
#define P2C_RG_DMPULLDOWN		BIT(7)
#define P2C_RG_DPPULLDOWN		BIT(6)
#define P2C_RG_XCVRSEL			GENMASK(5, 4)
#define P2C_RG_XCVRSEL_VAL(x)		((0x3 & (x)) << 4)
#define P2C_RG_SUSPENDM			BIT(3)
#define P2C_RG_TERMSEL			BIT(2)
#define P2C_DTM0_PART_MASK	\
		(P2C_FORCE_DATAIN | P2C_FORCE_DM_PULLDOWN | \
		P2C_FORCE_DP_PULLDOWN | P2C_FORCE_XCVRSEL | \
		P2C_FORCE_TERMSEL | P2C_RG_DMPULLDOWN | \
		P2C_RG_DPPULLDOWN | P2C_RG_TERMSEL)

#define U3P_U2PHYDTM1			0x06C
#define P2C_RG_UART_EN			BIT(16)
#define P2C_FORCE_IDDIG			BIT(9)
#define P2C_RG_VBUSVALID		BIT(5)
#define P2C_RG_SESSEND			BIT(4)
#define P2C_RG_AVALID			BIT(2)
#define P2C_RG_IDDIG			BIT(1)

#define U3P_U3_CHIP_GPIO_CTLD		0x0c
#define P3C_REG_IP_SW_RST		BIT(31)
#define P3C_MCU_BUS_CK_GATE_EN		BIT(30)
#define P3C_FORCE_IP_SW_RST		BIT(29)

#define U3P_U3_CHIP_GPIO_CTLE		0x10
#define P3C_RG_SWRST_U3_PHYD		BIT(25)
#define P3C_RG_SWRST_U3_PHYD_FORCE_EN	BIT(24)

#define U3P_U3_PHYA_REG0		0x000
#define P3A_RG_CLKDRV_OFF		GENMASK(3, 2)
#define P3A_RG_CLKDRV_OFF_VAL(x)	((0x3 & (x)) << 2)

#define U3P_U3_PHYA_REG1		0x004
#define P3A_RG_CLKDRV_AMP		GENMASK(31, 29)
#define P3A_RG_CLKDRV_AMP_VAL(x)	((0x7 & (x)) << 29)

#define U3P_U3_PHYA_REG6		0x018
#define P3A_RG_TX_EIDLE_CM		GENMASK(31, 28)
#define P3A_RG_TX_EIDLE_CM_VAL(x)	((0xf & (x)) << 28)

#define U3P_U3_PHYA_REG9		0x024
#define P3A_RG_RX_DAC_MUX		GENMASK(5, 1)
#define P3A_RG_RX_DAC_MUX_VAL(x)	((0x1f & (x)) << 1)

#define U3P_U3_PHYA_DA_REG0		0x100
#define P3A_RG_XTAL_EXT_PE2H		GENMASK(17, 16)
#define P3A_RG_XTAL_EXT_PE2H_VAL(x)	((0x3 & (x)) << 16)
#define P3A_RG_XTAL_EXT_PE1H		GENMASK(13, 12)
#define P3A_RG_XTAL_EXT_PE1H_VAL(x)	((0x3 & (x)) << 12)
#define P3A_RG_XTAL_EXT_EN_U3		GENMASK(11, 10)
#define P3A_RG_XTAL_EXT_EN_U3_VAL(x)	((0x3 & (x)) << 10)

#define U3P_U3_PHYA_DA_REG4		0x108
#define P3A_RG_PLL_DIVEN_PE2H		GENMASK(21, 19)
#define P3A_RG_PLL_BC_PE2H		GENMASK(7, 6)
#define P3A_RG_PLL_BC_PE2H_VAL(x)	((0x3 & (x)) << 6)

#define U3P_U3_PHYA_DA_REG5		0x10c
#define P3A_RG_PLL_BR_PE2H		GENMASK(29, 28)
#define P3A_RG_PLL_BR_PE2H_VAL(x)	((0x3 & (x)) << 28)
#define P3A_RG_PLL_IC_PE2H		GENMASK(15, 12)
#define P3A_RG_PLL_IC_PE2H_VAL(x)	((0xf & (x)) << 12)

#define U3P_U3_PHYA_DA_REG6		0x110
#define P3A_RG_PLL_IR_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_IR_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG7		0x114
#define P3A_RG_PLL_BP_PE2H		GENMASK(19, 16)
#define P3A_RG_PLL_BP_PE2H_VAL(x)	((0xf & (x)) << 16)

#define U3P_U3_PHYA_DA_REG20		0x13c
#define P3A_RG_PLL_DELTA1_PE2H		GENMASK(31, 16)
#define P3A_RG_PLL_DELTA1_PE2H_VAL(x)	((0xffff & (x)) << 16)

#define U3P_U3_PHYA_DA_REG25		0x148
#define P3A_RG_PLL_DELTA_PE2H		GENMASK(15, 0)
#define P3A_RG_PLL_DELTA_PE2H_VAL(x)	(0xffff & (x))

#define U3P_U3_PHYD_LFPS1		0x00c
#define P3D_RG_FWAKE_TH			GENMASK(21, 16)
#define P3D_RG_FWAKE_TH_VAL(x)		((0x3f & (x)) << 16)

#define U3P_U3_PHYD_CDR1		0x05c
#define P3D_RG_CDR_BIR_LTD1		GENMASK(28, 24)
#define P3D_RG_CDR_BIR_LTD1_VAL(x)	((0x1f & (x)) << 24)
#define P3D_RG_CDR_BIR_LTD0		GENMASK(12, 8)
#define P3D_RG_CDR_BIR_LTD0_VAL(x)	((0x1f & (x)) << 8)

#define U3P_U3_PHYD_RXDET1		0x128
#define P3D_RG_RXDET_STB2_SET		GENMASK(17, 9)
#define P3D_RG_RXDET_STB2_SET_VAL(x)	((0x1ff & (x)) << 9)

#define U3P_U3_PHYD_RXDET2		0x12c
#define P3D_RG_RXDET_STB2_SET_P3	GENMASK(8, 0)
#define P3D_RG_RXDET_STB2_SET_P3_VAL(x)	(0x1ff & (x))

#define U3P_SPLLC_XTALCTL3		0x018
#define XC3_RG_U3_XTAL_RX_PWD		BIT(9)
#define XC3_RG_U3_FRC_XTAL_RX_PWD	BIT(8)

enum mtk_phy_version {
	MTK_TPHY_V1 = 1,
	MTK_TPHY_V2,
};

struct u2phy_banks {
	void __iomem *misc;
	void __iomem *fmreg;
	void __iomem *com;
};

struct u3phy_banks {
	void __iomem *spllc;
	void __iomem *chip;
	void __iomem *phyd; /* include u3phyd_bank2 */
	void __iomem *phya; /* include u3phya_da */
};

/* New 22 nm APHY bank */
#define U3P_U3_PHYA_TOP0A       (0x0a * 4)
#define U3P_U3_PHYA_TOP16       (0x16 * 4)
#define U3P_U3_PHYA_TOP33       (0x33 * 4)
#define U3P_U3_PHYA_TOP4B       (0x4b * 4)
#define U3P_U3_PHYA_TOP71       (0x71 * 4)
#define U3P_U3_PHYA_TOP73       (0x73 * 4)
#define U3P_U3_PHYA_TOP76       (0x76 * 4)
#define U3P_U3_PHYA_WRAP0B      (0x0b * 4)
#define U3P_U3_PHYA_WRAP0C      (0x0c * 4)
#define U3P_U3_PHYA_WRAP0D      (0x0d * 4)
#define U3P_U3_PHYA_WRAP40      (0x40 * 4)
#define U3P_U3_PHYA_WRAP5C      (0x5c * 4)
#define U3P_U3_PHYA_WRAP5F      (0x5f * 4)
#define U3P_U3_PHYA_WRAP60      (0x60 * 4)
#define U3P_U3_PHYA_WRAP66      (0x66 * 4)
#define U3P_U3_PHYA_WRAP67      (0x67 * 4)
#define U3P_U3_PHYA_WRAP7F      (0x7f * 4)

#define MPHY_ATOP_AWRAP_OFFSET 0x200

/* X32_U3_PHY */
#define U3P_SSPXTP_DIG_LN_DAIF_REG0	0x3700
#define PA0_RG_SSPXTP0_DAIF_FRC_LN_TX_LCTXCM1_VAL	BIT(7)

struct u3mphy_banks {
        void __iomem *phy_upll;
        void __iomem *phy_atop;
        void __iomem *phy_awrap;
        void __iomem *phy_u3phy;
};

struct mtk_phy_instance {
	void __iomem *port_base;
	void __iomem *port_base_mphy;
	void __iomem *port_base_upll;
	void __iomem *port_base_u3phy;
	const struct device_node *np;
	union {
		struct u2phy_banks u2_banks;
		struct u3phy_banks u3_banks;
	};
	struct u3mphy_banks u3_mphy_banks;

	struct clk ref_clk;	/* reference clock of (digital) phy */
	struct clk da_ref_clk;	/* reference clock of analog phy */
	u32 index;
	u32 type;
	/* analog recomended adjustment */
	u32 eye_ls_sr;
	u32 eye_pre_em;
};

struct mtk_tphy {
	struct udevice *dev;
	void __iomem *sif_base;
	void __iomem *i2c_ctrl;
	void __iomem *i2c_ctrl_u2;
	enum mtk_phy_version version;
	struct mtk_phy_instance **phys;
	int nphys;
	bool is_phy_mixed_mode;
	bool is_phy_2nd_i2c;
};

static void u2_phy_instance_init(struct mtk_tphy *tphy,
				 struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	/*
	 * [PHY] phy_v2 22 nm MTK HQ UPLL init.
	 * misc bank is for instance 0 only.
	 */
	if (tphy->is_phy_mixed_mode && (instance->index == 0)) {
		clrsetbits_le32(u2_banks->misc + U3P_MISC_REG0,
			PA0_RG_U2_PLL_CTRL, PA0_RG_U2_PLL_CTRL_VAL(0x64));
		debug("PLL default 0x%x\n", readl(u2_banks->misc));
	}

	/* switch to USB function, and enable usb pll */
	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM0,
			P2C_FORCE_UART_EN | P2C_FORCE_SUSPENDM,
			P2C_RG_XCVRSEL_VAL(1) | P2C_RG_DATAIN_VAL(0));

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM1, P2C_RG_UART_EN);
	setbits_le32(u2_banks->com + U3P_USBPHYACR0, PA0_RG_USB20_INTR_EN);

	/* disable switch 100uA current to SSUSB */
	clrbits_le32(u2_banks->com + U3P_USBPHYACR5, PA5_RG_U2_HS_100U_U3_EN);

	clrbits_le32(u2_banks->com + U3P_U2PHYACR4, P2C_U2_GPIO_CTR_MSK);

	/* DP/DM BC1.1 path Disable */
	if (tphy->is_phy_mixed_mode) {
		clrsetbits_le32(u2_banks->com + U3P_USBPHYACR6,
			PA6_RG_U2_BC11_SW_EN | PA6_RG_U2_SQTH,
			PA6_RG_U2_SQTH_VAL(8));
	} else {
		clrsetbits_le32(u2_banks->com + U3P_USBPHYACR6,
			PA6_RG_U2_BC11_SW_EN | PA6_RG_U2_SQTH,
			PA6_RG_U2_SQTH_VAL(2));
	}

	/* set HS slew rate */
	#if 0 // 7 nm HW mode
	clrsetbits_le32(u2_banks->com + U3P_USBPHYACR5,
			PA5_RG_U2_HSTX_SRCTRL, PA5_RG_U2_HSTX_SRCTRL_VAL(4));
	#endif

	if (instance->eye_ls_sr != ~0) {
		clrsetbits_le32(u2_banks->com + U3P_USBPHYACR4,
			PA4_RG_USB20_LS_SR,
			PA4_RG_USB20_LS_SR_VAL(instance->eye_ls_sr));
	}

	if (instance->eye_pre_em != ~0) {
		clrsetbits_le32(u2_banks->com + U3P_USBPHYACR6,
			PA6_RG_U2_PRE_EMPHASIS,
			PA6_RG_U2_PRE_EMPHASIS_VAL(instance->eye_pre_em));
	}

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u2_phy_instance_power_on(struct mtk_tphy *tphy,
				     struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM0,
		     P2C_RG_XCVRSEL | P2C_RG_DATAIN | P2C_DTM0_PART_MASK);

	/* OTG Enable */
	setbits_le32(u2_banks->com + U3P_USBPHYACR6,
		     PA6_RG_U2_OTG_VBUSCMP_EN);

	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM1,
			P2C_RG_SESSEND, P2C_RG_VBUSVALID | P2C_RG_AVALID);

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u2_phy_instance_power_off(struct mtk_tphy *tphy,
				      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;

	clrbits_le32(u2_banks->com + U3P_U2PHYDTM0,
		     P2C_RG_XCVRSEL | P2C_RG_DATAIN);

	/* OTG Disable */
	clrbits_le32(u2_banks->com + U3P_USBPHYACR6,
		     PA6_RG_U2_OTG_VBUSCMP_EN);

	clrsetbits_le32(u2_banks->com + U3P_U2PHYDTM1,
			P2C_RG_VBUSVALID | P2C_RG_AVALID, P2C_RG_SESSEND);

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void u3_phy_instance_init(struct mtk_tphy *tphy,
				 struct mtk_phy_instance *instance)
{
	if (tphy->is_phy_mixed_mode) {
		struct u3mphy_banks *u3_mphy_banks = &instance->u3_mphy_banks;

		clrbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP0B,
				GENMASK(14, 0));
		clrbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP0C,
				GENMASK(14, 0));
		clrbits_le32(u3_mphy_banks->phy_upll, BIT(1));
		setbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP33,
				BIT(11));
		clrbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP4B,
				BIT(4));
		udelay(80);
		setbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP0B,
				0x7002);
		setbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP0C,
				0x04ec);
		udelay(20);
		clrbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP33,
				BIT(11));

		clrsetbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP16,
				BIT(5), BIT(4));
		clrbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP71,
				BIT(11));
		clrbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP73,
				BIT(11));
		clrsetbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP76,
				BIT(5), BIT(4));
		setbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP0D,
				BIT(2));
		setbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP40,
				BIT(7) | BIT(8));
		clrsetbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP67,
				BIT(6), BIT(5));

		clrbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP5C,
				BIT(13));
		clrsetbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP5F,
				GENMASK(10, 0), BIT(8));
		clrsetbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP60,
				GENMASK(4, 2), GENMASK(7, 5));
		clrsetbits_le32(u3_mphy_banks->phy_awrap + U3P_U3_PHYA_WRAP66,
				GENMASK(4, 0), BIT(2));
		clrsetbits_le32(u3_mphy_banks->phy_atop + U3P_U3_PHYA_TOP0A,
				GENMASK(15, 0), BIT(10));
		/* overwrite x32 u3 phy */
		if (u3_mphy_banks->phy_u3phy) {
			setbits_le32(u3_mphy_banks->phy_u3phy + U3P_SSPXTP_DIG_LN_DAIF_REG0,
				PA0_RG_SSPXTP0_DAIF_FRC_LN_TX_LCTXCM1_VAL);
		}
	} else {
		struct u3phy_banks *u3_banks = &instance->u3_banks;

		/* gating PCIe Analog XTAL clock */
		setbits_le32(u3_banks->spllc + U3P_SPLLC_XTALCTL3,
		     XC3_RG_U3_XTAL_RX_PWD | XC3_RG_U3_FRC_XTAL_RX_PWD);

		/* gating XSQ */
		clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG0,
			P3A_RG_XTAL_EXT_EN_U3, P3A_RG_XTAL_EXT_EN_U3_VAL(2));

		clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG9,
			P3A_RG_RX_DAC_MUX, P3A_RG_RX_DAC_MUX_VAL(4));

		clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG6,
			P3A_RG_TX_EIDLE_CM, P3A_RG_TX_EIDLE_CM_VAL(0xe));

		clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_CDR1,
			P3D_RG_CDR_BIR_LTD0 | P3D_RG_CDR_BIR_LTD1,
			P3D_RG_CDR_BIR_LTD0_VAL(0xc) |
			P3D_RG_CDR_BIR_LTD1_VAL(0x3));

		clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_LFPS1,
			P3D_RG_FWAKE_TH, P3D_RG_FWAKE_TH_VAL(0x34));

		clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET1,
			P3D_RG_RXDET_STB2_SET, P3D_RG_RXDET_STB2_SET_VAL(0x10));

		clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET2,
			P3D_RG_RXDET_STB2_SET_P3,
			P3D_RG_RXDET_STB2_SET_P3_VAL(0x10));
	}

	dev_dbg(tphy->dev, "%s(%d)\n", __func__, instance->index);
}

static void pcie_phy_instance_init(struct mtk_tphy *tphy,
				   struct mtk_phy_instance *instance)
{
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	if (tphy->version != MTK_TPHY_V1)
		return;

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG0,
			P3A_RG_XTAL_EXT_PE1H | P3A_RG_XTAL_EXT_PE2H,
			P3A_RG_XTAL_EXT_PE1H_VAL(0x2) |
			P3A_RG_XTAL_EXT_PE2H_VAL(0x2));

	/* ref clk drive */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG1, P3A_RG_CLKDRV_AMP,
			P3A_RG_CLKDRV_AMP_VAL(0x4));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_REG0, P3A_RG_CLKDRV_OFF,
			P3A_RG_CLKDRV_OFF_VAL(0x1));

	/* SSC delta -5000ppm */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG20,
			P3A_RG_PLL_DELTA1_PE2H,
			P3A_RG_PLL_DELTA1_PE2H_VAL(0x3c));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG25,
			P3A_RG_PLL_DELTA_PE2H,
			P3A_RG_PLL_DELTA_PE2H_VAL(0x36));

	/* change pll BW 0.6M */
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG5,
			P3A_RG_PLL_BR_PE2H | P3A_RG_PLL_IC_PE2H,
			P3A_RG_PLL_BR_PE2H_VAL(0x1) |
			P3A_RG_PLL_IC_PE2H_VAL(0x1));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG4,
			P3A_RG_PLL_DIVEN_PE2H | P3A_RG_PLL_BC_PE2H,
			P3A_RG_PLL_BC_PE2H_VAL(0x3));

	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG6,
			P3A_RG_PLL_IR_PE2H, P3A_RG_PLL_IR_PE2H_VAL(0x2));
	clrsetbits_le32(u3_banks->phya + U3P_U3_PHYA_DA_REG7,
			P3A_RG_PLL_BP_PE2H, P3A_RG_PLL_BP_PE2H_VAL(0xa));

	/* Tx Detect Rx Timing: 10us -> 5us */
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET1,
			P3D_RG_RXDET_STB2_SET,
			P3D_RG_RXDET_STB2_SET_VAL(0x10));
	clrsetbits_le32(u3_banks->phyd + U3P_U3_PHYD_RXDET2,
			P3D_RG_RXDET_STB2_SET_P3,
			P3D_RG_RXDET_STB2_SET_P3_VAL(0x10));

	/* wait for PCIe subsys register to active */
	udelay(3000);
}

static void pcie_phy_instance_power_on(struct mtk_tphy *tphy,
				       struct mtk_phy_instance *instance)
{
	struct u3phy_banks *bank = &instance->u3_banks;

	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	clrbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void pcie_phy_instance_power_off(struct mtk_tphy *tphy,
					struct mtk_phy_instance *instance)

{
	struct u3phy_banks *bank = &instance->u3_banks;

	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLD,
		     P3C_FORCE_IP_SW_RST | P3C_REG_IP_SW_RST);
	setbits_le32(bank->chip + U3P_U3_CHIP_GPIO_CTLE,
		     P3C_RG_SWRST_U3_PHYD_FORCE_EN | P3C_RG_SWRST_U3_PHYD);
}

static void phy_v1_banks_init(struct mtk_tphy *tphy,
			      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc = NULL;
		u2_banks->fmreg = tphy->sif_base + SSUSB_SIFSLV_V1_U2FREQ;
		u2_banks->com = instance->port_base + SSUSB_SIFSLV_V1_U2PHY_COM;
		break;
	case PHY_TYPE_USB3:
	case PHY_TYPE_PCIE:
		u3_banks->spllc = tphy->sif_base + SSUSB_SIFSLV_V1_SPLLC;
		u3_banks->chip = tphy->sif_base + SSUSB_SIFSLV_V1_CHIP;
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V1_U3PHYD;
		u3_banks->phya = instance->port_base + SSUSB_SIFSLV_V1_U3PHYA;
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return;
	}
}

static void phy_v2_banks_init(struct mtk_tphy *tphy,
			      struct mtk_phy_instance *instance)
{
	struct u2phy_banks *u2_banks = &instance->u2_banks;
	struct u3phy_banks *u3_banks = &instance->u3_banks;

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_banks->misc = instance->port_base + SSUSB_SIFSLV_V2_MISC;
		u2_banks->fmreg = instance->port_base + SSUSB_SIFSLV_V2_U2FREQ;
		u2_banks->com = instance->port_base + SSUSB_SIFSLV_V2_U2PHY_COM;
		break;
	case PHY_TYPE_USB3:
	case PHY_TYPE_PCIE:
		u3_banks->spllc = instance->port_base + SSUSB_SIFSLV_V2_SPLLC;
		u3_banks->chip = instance->port_base + SSUSB_SIFSLV_V2_CHIP;
		u3_banks->phyd = instance->port_base + SSUSB_SIFSLV_V2_U3PHYD;
		u3_banks->phya = instance->port_base + SSUSB_SIFSLV_V2_U3PHYA;
		if (tphy->is_phy_mixed_mode) {
			struct u3mphy_banks *u3_mphy_banks;

			u3_mphy_banks = &instance->u3_mphy_banks;
			u3_mphy_banks->phy_upll = instance->port_base_upll;
			u3_mphy_banks->phy_atop = instance->port_base_mphy;
			u3_mphy_banks->phy_awrap = instance->port_base_mphy +
							MPHY_ATOP_AWRAP_OFFSET;
			if (instance->port_base_u3phy)
				u3_mphy_banks->phy_u3phy = instance->port_base_u3phy;
			else
				u3_mphy_banks->phy_u3phy = NULL;
			debug("[MTK] upll 0x%p\n", u3_mphy_banks->phy_upll);
			debug("[MTK] atop 0x%p\n", u3_mphy_banks->phy_atop);
			debug("[MTK] awrap 0x%p\n", u3_mphy_banks->phy_awrap);
		}
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return;
	}
}

static int mtk_phy_init(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	switch (instance->type) {
	case PHY_TYPE_USB2:
		u2_phy_instance_init(tphy, instance);
		break;
	case PHY_TYPE_USB3:
		u3_phy_instance_init(tphy, instance);
		break;
	case PHY_TYPE_PCIE:
		pcie_phy_instance_init(tphy, instance);
		break;
	default:
		dev_err(tphy->dev, "incompatible PHY type\n");
		return -EINVAL;
	}

	return 0;
}

static int mtk_phy_power_on(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		u2_phy_instance_power_on(tphy, instance);
	else if (instance->type == PHY_TYPE_PCIE)
		pcie_phy_instance_power_on(tphy, instance);

	return 0;
}

static int mtk_phy_power_off(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	if (instance->type == PHY_TYPE_USB2)
		u2_phy_instance_power_off(tphy, instance);
	else if (instance->type == PHY_TYPE_PCIE)
		pcie_phy_instance_power_off(tphy, instance);

	return 0;
}

static int mtk_phy_exit(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = tphy->phys[phy->id];

	(void)(instance);
	return 0;
}

static int mtk_phy_xlate(struct phy *phy,
			 struct ofnode_phandle_args *args)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	struct mtk_phy_instance *instance = NULL;
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

	for (index = 0; index < tphy->nphys; index++)
		if (phy_np == tphy->phys[index]->np) {
			instance = tphy->phys[index];
			break;
		}

	if (!instance) {
		dev_err(phy->dev, "failed to find appropriate phy\n");
		return -EINVAL;
	}

	phy->id = index;
	instance->type = args->args[1];
	if (!(instance->type == PHY_TYPE_USB2 ||
	      instance->type == PHY_TYPE_USB3 ||
	      instance->type == PHY_TYPE_PCIE)) {
		dev_err(phy->dev, "unsupported device type\n");
		return -EINVAL;
	}

	if (tphy->version == MTK_TPHY_V1) {
		phy_v1_banks_init(tphy, instance);
	} else if (tphy->version == MTK_TPHY_V2) {
		phy_v2_banks_init(tphy, instance);
	} else {
		dev_err(phy->dev, "phy version is not supported\n");
		return -EINVAL;
	}

	return 0;
}

static const struct phy_ops mtk_tphy_ops = {
	.init		= mtk_phy_init,
	.exit		= mtk_phy_exit,
	.power_on	= mtk_phy_power_on,
	.power_off	= mtk_phy_power_off,
	.of_xlate	= mtk_phy_xlate,
};

/* DTV CPU i2c control setting */
int mtk_tphy_i2c_ctrl(struct phy *phy)
{
	struct mtk_tphy *tphy = dev_get_priv(phy->dev);
	u32 index;
	u32 u3phys = 0;
	u32 i2c_ctl_msk = 0;

	for (index = 0; index < tphy->nphys; index++)
		if (tphy->phys[index]->type == PHY_TYPE_USB3)
			u3phys++;

	for (index = 0; index < u3phys+1; index++)
		i2c_ctl_msk |= 1 << index;
	debug("\ti2c_ctl_msk = 0x%x\n", i2c_ctl_msk);

	writew(readw(tphy->i2c_ctrl) & ~i2c_ctl_msk, tphy->i2c_ctrl);
	debug("\ti2c mode: 0x%x\n", readw(tphy->i2c_ctrl));
	if (tphy->is_phy_mixed_mode && tphy->is_phy_2nd_i2c) {
		writew(readw(tphy->i2c_ctrl_u2) & ~i2c_ctl_msk,
							tphy->i2c_ctrl_u2);
		debug("\ti2c mode mixed: 0x%x\n", readw(tphy->i2c_ctrl_u2));
	}
	return 0;
}

static int mtk_tphy_probe(struct udevice *dev)
{
	struct mtk_tphy *tphy = dev_get_priv(dev);
	ofnode subnode;
	int index = 0;

	debug("[PHY] %s, dev %p\n", __func__, dev);
	tphy->is_phy_mixed_mode = dev_read_bool(dev, "mediatek,phy_mixed_mode");
	debug("[MTK][PHY] is_phy_mixed_mode: %d\n", tphy->is_phy_mixed_mode);

	tphy->is_phy_2nd_i2c = dev_read_bool(dev, "mediatek,phy_2nd_i2c");
	debug("[MTK][PHY] is_phy_2nd_i2c: %d\n", tphy->is_phy_2nd_i2c);

	tphy->i2c_ctrl = dev_remap_addr_name(dev, "i2c_ctrl_reg");
	if (!tphy->i2c_ctrl) {
		pr_err("Failed to get i2c_ctrl_reg address\n");
		return -EINVAL;
	}

	tphy->i2c_ctrl_u2 = dev_remap_addr_name(dev, "i2c_ctrl_reg_u2");

	dev_for_each_subnode(subnode, dev) {
		/* only count phy which's 'status' property is 'ok' */
		if (!ofnode_is_available(subnode))
			continue;
		tphy->nphys++;
	}
	printf("[MTK] number of Available phy %d\n", tphy->nphys);

	tphy->phys = devm_kcalloc(dev, tphy->nphys, sizeof(*tphy->phys),
				  GFP_KERNEL);
	if (!tphy->phys)
		return -ENOMEM;

	tphy->dev = dev;
	tphy->version = dev_get_driver_data(dev);

	/* v1 has shared banks */
	if (tphy->version == MTK_TPHY_V1) {
		tphy->sif_base = dev_read_addr_ptr(dev);
		if (!tphy->sif_base)
			return -ENOENT;
	}

	dev_for_each_subnode(subnode, dev) {
		struct mtk_phy_instance *instance;
		fdt_addr_t addr;

		if (!ofnode_is_available(subnode))
			continue;

		instance = devm_kzalloc(dev, sizeof(*instance), GFP_KERNEL);
		if (!instance)
			return -ENOMEM;

		addr = ofnode_get_addr_index(subnode, 0);
		if (addr == FDT_ADDR_T_NONE) {
			pr_err("Failed to get xHCI PHY address\n");
			devm_kfree(dev, instance);
			return -ENOMEM;
		}

		instance->port_base = map_sysmem(addr, 0);
		instance->index = index;
		instance->eye_ls_sr = ofnode_read_u32_default(subnode,
					"mediatek,eye-ls-sr", ~0);
		instance->eye_pre_em = ofnode_read_u32_default(subnode,
					"mediatek,eye-pre-em", ~0);
		debug("[MTK] ls-sr:%x, pre-em:%x\n", instance->eye_ls_sr,
						instance->eye_pre_em);

		addr = ofnode_get_addr_index(subnode, 1);
		instance->port_base_mphy = map_sysmem(addr, 0);

		addr = ofnode_get_addr_index(subnode, 2);
		instance->port_base_upll = map_sysmem(addr, 0);

		addr = ofnode_get_addr_index(subnode, 3);
		if (addr == FDT_ADDR_T_NONE)
			instance->port_base_u3phy = NULL;
		else
			instance->port_base_u3phy = map_sysmem(addr, 0);
		instance->np = ofnode_to_np(subnode);
		tphy->phys[index] = instance;
		index++;
	}

	return 0;
}

static const struct udevice_id mtk_tphy_id_table[] = {
	{ .compatible = "mediatek,generic-tphy-v1", .data = MTK_TPHY_V1, },
	{ .compatible = "mediatek,generic-tphy-v2", .data = MTK_TPHY_V2, },
	{ }
};

U_BOOT_DRIVER(mtk_tphy) = {
	.name		= "mtk-tphy",
	.id		= UCLASS_PHY,
	.of_match	= mtk_tphy_id_table,
	.ops		= &mtk_tphy_ops,
	.probe		= mtk_tphy_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_tphy),
};
