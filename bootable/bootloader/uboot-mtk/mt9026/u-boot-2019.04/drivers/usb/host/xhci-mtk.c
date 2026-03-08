// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 MediaTek, Inc.
 * Authors: Chunfeng Yun <chunfeng.yun@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <malloc.h>
#include <usb.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <power/regulator.h>
#include <linux/iopoll.h>
#include <asm/gpio.h> /* GPIO API header */

#include <usb/xhci.h>

#define MTK_UBOOT_XHCI_VERSION "2021-12-06"
#define MTK_XHCI_NODE_STR_ID "mediatek,mtk-dtv-xhci"

#ifdef __DEBUG_MSG_ON__
#define dev_err(dev, fmt, arg...) printf(fmt, ##arg)
#define dev_info(dev, fmt, arg...) printf(fmt, ##arg)
#endif

#define MU3C_U3_PORT_MAX 4
#define MU3C_U2_PORT_MAX 5

/**
 * struct mtk_ippc_regs: MTK ssusb ip port control registers
 * @ip_pw_ctr0~3: ip power and clock control registers
 * @ip_pw_sts1~2: ip power and clock status registers
 * @ip_xhci_cap: ip xHCI capability register
 * @u3_ctrl_p[x]: ip usb3 port x control register, only low 4bytes are used
 * @u2_ctrl_p[x]: ip usb2 port x control register, only low 4bytes are used
 * @u2_phy_pll: usb2 phy pll control register
 */
struct mtk_ippc_regs {
	__le32 ip_pw_ctr0;
	__le32 ip_pw_ctr1;
	__le32 ip_pw_ctr2;
	__le32 ip_pw_ctr3;
	__le32 ip_pw_sts1;
	__le32 ip_pw_sts2;
	__le32 reserved0[3];
	__le32 ip_xhci_cap;
	__le32 reserved1[2];
	__le64 u3_ctrl_p[MU3C_U3_PORT_MAX];
	__le64 u2_ctrl_p[MU3C_U2_PORT_MAX];
	__le32 reserved2;
	__le32 u2_phy_pll;
	__le32 reserved3[33]; /* 0x80 ~ 0xff */
};

/* ip_pw_ctrl0 register */
#define CTRL0_IP_SW_RST	BIT(0)

/* ip_pw_ctrl1 register */
#define CTRL1_IP_HOST_PDN	BIT(0)

/* ip_pw_ctrl2 register */
#define CTRL2_IP_DEV_PDN	BIT(0)

/* ip_pw_sts1 register */
#define STS1_IP_SLEEP_STS	BIT(30)
#define STS1_U3_MAC_RST	BIT(16)
#define STS1_XHCI_RST		BIT(11)
#define STS1_SYS125_RST	BIT(10)
#define STS1_REF_RST		BIT(8)
#define STS1_SYSPLL_STABLE	BIT(0)

/* ip_xhci_cap register */
#define CAP_U3_PORT_NUM(p)	((p) & 0xff)
#define CAP_U2_PORT_NUM(p)	(((p) >> 8) & 0xff)

/* u3_ctrl_p register */
#define CTRL_U3_PORT_HOST_SEL	BIT(2)
#define CTRL_U3_PORT_PDN	BIT(1)
#define CTRL_U3_PORT_DIS	BIT(0)

/* u2_ctrl_p register */
#define CTRL_U2_PORT_HOST_SEL	BIT(2)
#define CTRL_U2_PORT_PDN	BIT(1)
#define CTRL_U2_PORT_DIS	BIT(0)

struct mtk_xhci {
	struct xhci_ctrl ctrl;	/* Needs to come first in this struct! */
	struct xhci_hccr *hcd;
	struct mtk_ippc_regs *ippc;
	struct udevice *dev;
	struct udevice *vusb33_supply;
	struct udevice *vbus_supply;
	struct clk *sys_clk;
	struct clk *xhci_clk;
	struct clk *ref_clk;
	struct clk *mcu_clk;
	struct clk *dma_clk;
	int num_u2_ports;
	int num_u3_ports;
	struct phy *phys;
	int num_phys;
	struct gpio_desc *gpios;
	int num_gpios;
	u32 u2p_dis_msk;
	u32 u3p_dis_msk;
	bool pm_domain;
	bool phy_mixed;
};

static int xhci_mtk_host_enable(struct mtk_xhci *mtk)
{
	struct mtk_ippc_regs *ippc = mtk->ippc;
	u32 value, check_val, u3_ports_disabed;
	int ret;
	int i;

	u3_ports_disabed = 0;
	/* power on host ip */
	value = readl(&ippc->ip_pw_ctr1);
	value &= ~CTRL1_IP_HOST_PDN;
	writel(value, &ippc->ip_pw_ctr1);

	/* power on and enable all u3 ports */
	for (i = 0; i < mtk->num_u3_ports; i++) {
		/* if U3PHY port 1 bounded to ground */
		if (mtk->u3p_dis_msk  & (0x1 << i)) {
			u3_ports_disabed++;
			printf("[MTK] ippc skip u3 port %d\n", i);
			continue;
		}

		value = readl(&ippc->u3_ctrl_p[i]);
		value &= ~(CTRL_U3_PORT_PDN | CTRL_U3_PORT_DIS);
		value |= CTRL_U3_PORT_HOST_SEL;
		writel(value, &ippc->u3_ctrl_p[i]);
	}

	/* power on and enable all u2 ports */
	for (i = 0; i < mtk->num_u2_ports; i++) {
		if (mtk->u2p_dis_msk  & (0x1 << i)) {
			printf("[MTK] ippc skip u2 port %d\n", i);
			continue;
		}

		value = readl(&ippc->u2_ctrl_p[i]);
		value &= ~(CTRL_U2_PORT_PDN | CTRL_U2_PORT_DIS);
		value |= CTRL_U2_PORT_HOST_SEL;
		writel(value, &ippc->u2_ctrl_p[i]);
	}

	/*
	 * wait for clocks to be stable, and clock domains reset to
	 * be inactive after power on and enable ports
	 */
	check_val = STS1_SYSPLL_STABLE | STS1_REF_RST |
			STS1_SYS125_RST | STS1_XHCI_RST;

	if (mtk->num_u3_ports > u3_ports_disabed)
		check_val |= STS1_U3_MAC_RST;

	ret = readl_poll_timeout(&ippc->ip_pw_sts1, value,
				 (check_val == (value & check_val)), 20000);
	if (ret) {
		dev_err(mtk->dev, "clocks are not stable (0x%x)\n", value);
		return ret;
	}

	return 0;
}

static int xhci_mtk_host_disable(struct mtk_xhci *mtk)
{
	struct mtk_ippc_regs *ippc = mtk->ippc;
	u32 value;
	int i;

	/* power down all u3 ports */
	for (i = 0; i < mtk->num_u3_ports; i++) {
		value = readl(&ippc->u3_ctrl_p[i]);
		value |= CTRL_U3_PORT_PDN;
		/* fix: xhci reg read fail after jumping to kernel */
		value |= CTRL_U3_PORT_DIS;
		writel(value, &ippc->u3_ctrl_p[i]);
	}

	/* power down all u2 ports */
	for (i = 0; i < mtk->num_u2_ports; i++) {
		value = readl(&ippc->u2_ctrl_p[i]);
		value |= CTRL_U2_PORT_PDN;
		/* fix: xhci reg read fail after jumping to kernel */
		value |= CTRL_U2_PORT_DIS;
		writel(value, &ippc->u2_ctrl_p[i]);
	}

	/* power down host ip */
	value = readl(&ippc->ip_pw_ctr1);
	value |= CTRL1_IP_HOST_PDN;
	writel(value, &ippc->ip_pw_ctr1);

	return 0;
}

static int xhci_mtk_ssusb_init(struct mtk_xhci *mtk)
{
	struct mtk_ippc_regs *ippc = mtk->ippc;
	u32 value;

	/* reset whole ip */
	value = readl(&ippc->ip_pw_ctr0);
	value |= CTRL0_IP_SW_RST;
	writel(value, &ippc->ip_pw_ctr0);
	udelay(1);
	value = readl(&ippc->ip_pw_ctr0);
	value &= ~CTRL0_IP_SW_RST;
	writel(value, &ippc->ip_pw_ctr0);

	value = readl(&ippc->ip_xhci_cap);
	mtk->num_u3_ports = CAP_U3_PORT_NUM(value);
	mtk->num_u2_ports = CAP_U2_PORT_NUM(value);
#ifdef CONFIG_USB_MTU3EX_PHY
	printf("[HAPS] force number of u3 port from %d to 0\n",
		mtk->num_u3_ports);
	mtk->num_u3_ports = 0;
#endif
	printf("[MTK] %s u2p:%d, u3p:%d\n", __func__,
		 mtk->num_u2_ports, mtk->num_u3_ports);

	return xhci_mtk_host_enable(mtk);
}

static int xhci_mtk_clks_enable(struct mtk_xhci *mtk)
{
	return 0;
}

static void xhci_mtk_clks_disable(struct mtk_xhci *mtk)
{
}

#define U2PORT_DIABLE_MASK_DEFAULT (0x0)
#define U3PORT_DIABLE_MASK_DEFAULT (0x2)
static int xhci_mtk_ofdata_get(struct mtk_xhci *mtk)
{
	struct udevice *dev = mtk->dev;
	struct udevice *cdev;
	fdt_addr_t reg_base;
	void *addr_t;
	ofnode subnode;
	int ret;

	reg_base = (fdt_addr_t)dev_remap_addr_name(dev, "ippc");
	if (reg_base == FDT_ADDR_T_NONE) {
		pr_err("Failed to get SSUSB IPPC address\n");
		return -ENXIO;
	}

	mtk->ippc = (struct mtk_ippc_regs *)reg_base;

	mtk->phy_mixed = dev_read_bool(dev, "mediatek,phy_mixed_mode");
	debug("[MTK] phy_mixed_mode: 0x%x\n", mtk->phy_mixed);

	mtk->pm_domain = dev_read_bool(dev, "standbypower-domain");
	debug("[MTK] standbypower-domain: 0x%x\n", mtk->pm_domain);

	if (mtk->pm_domain)
		goto ofdata_get_end;

	mtk->u2p_dis_msk = dev_read_u32_default(dev, "mediatek,u2p-dis-msk",
					U2PORT_DIABLE_MASK_DEFAULT);
	mtk->u3p_dis_msk = dev_read_u32_default(dev, "mediatek,u3p-dis-msk",
					U3PORT_DIABLE_MASK_DEFAULT);

	reg_base = (fdt_addr_t)dev_remap_addr_name(dev, "misc");
	if (reg_base == FDT_ADDR_T_NONE) {
		pr_err("Failed to get xHCI misc_1 address\n");
		return -ENXIO;
	}

	addr_t = (void *)reg_base;
	if (addr_t == NULL) {
		pr_err("xHCI misc_1 address NULL\n");
		return -ENXIO;
	}
	writew(readw((void*)addr_t) | 0x1, (void*)addr_t);
	debug("\tXHCI EMI setting: 0x%x\n", readw((void*)addr_t));
	writew(readw((void*)((ulong)addr_t+0x2f*4)) | 0x1,
		(void*)((ulong)addr_t+0x2f*4));
	debug("\tXHCI SSUSB UTMI select: 0x%x\n",
		readw((void*)((ulong)addr_t+0x2f*4)));

/* M6 series */
#define MTK_MT5896_CHIPID 0x101
#define MTK_CHIPID_BASE 0x1c021000
	debug("[MTK] chip id %x\n", readw((void *)MTK_CHIPID_BASE));
	if (MTK_MT5896_CHIPID != readw((void *)MTK_CHIPID_BASE))
		goto ofdata_get_end;

	ret = dev_read_stringlist_search(dev, "reg-names", "dma_gt");
	if (ret == -EINVAL) {
		pr_err("Something wrong when getting dma bus gate\n");
		return ret;
	} else if (ret == -ENODATA) {
		debug("[MTK] no DMA gate setting\n");
		goto ofdata_get_end;
	}

	reg_base = (fdt_addr_t)dev_remap_addr_name(dev, "dma_gt");
	if (reg_base == FDT_ADDR_T_NONE) {
		pr_err("Failed to get xHCI dma gate address\n");
		return -ENXIO;
	}

	addr_t = (void *)reg_base;
	if (addr_t == NULL) {
		pr_err("xHCI dma gate address NULL\n");
		return -ENXIO;
	}
	writew(readw(addr_t) | 0x800, addr_t);
	debug("\tSSUSB dma bus ck: 0x%x\n", readw(addr_t));

ofdata_get_end:
	subnode = ofnode_first_subnode(dev_ofnode(dev));
	cdev = container_of(&subnode, struct udevice, node);
	if (!device_is_compatible(cdev, MTK_XHCI_NODE_STR_ID)) {
		pr_err("xhci child device NOT found\n");
		return -EINVAL;
	}

	reg_base = (fdt_addr_t)dev_remap_addr_name(cdev, "mac");
	if (reg_base == FDT_ADDR_T_NONE) {
		pr_err("Failed to get xHCI base address\n");
		return -ENXIO;
	}
	mtk->hcd = (struct xhci_hccr *)reg_base;

	debug("hcd: %p, ippc: %p\n", mtk->hcd, mtk->ippc);
	return 0;
}

static int xhci_mtk_ldos_enable(struct mtk_xhci *mtk)
{
	return 0;
}

static void xhci_mtk_ldos_disable(struct mtk_xhci *mtk)
{
}

static int xhci_mtk_gpios_enable(struct mtk_xhci *mtk)
{
	struct udevice *dev = mtk->dev;
	struct udevice *cdev;
	ofnode subnode;
	struct gpio_desc *vbus_gpio;
	int i, count;

	subnode = ofnode_first_subnode(dev_ofnode(dev));
	if (!ofnode_valid(subnode)) {
		debug("Failed to find xhci subnode\n");
		return -EINVAL;
	}
	/* Return if no gpios declared */
	cdev = container_of(&subnode, struct udevice, node);
	if (!dev_read_prop(cdev, "vbus-gpios", NULL)) {
		pr_err("NO gpio setting in dts\n");
		return 0;
	}

	count = ofnode_count_phandle_with_args(subnode, "vbus-gpios", "#gpio-cells");
	if (count <= 0) {
		pr_err("Get vbus gpios error(%d)\n", count);
		return count;
	}

	vbus_gpio = devm_kcalloc(dev, count, sizeof(*vbus_gpio),
				GFP_KERNEL);
	if (!vbus_gpio)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		gpio_request_by_name_nodev(subnode, "vbus-gpios", i,
			&vbus_gpio[i], GPIOD_IS_OUT);
	}

	/* Active Vbus here */
	for (i = 0; i < count; i++) {
		if (dm_gpio_is_valid(&vbus_gpio[i]))
			dm_gpio_set_value(&vbus_gpio[i], 1);
	}

	mtk->gpios = vbus_gpio;
	mtk->num_gpios =  count;
	debug("Set Vbus GPIO done! count = %d\n", count);

	return 0;
}

static void xhci_mtk_gpios_disable(struct mtk_xhci *mtk)
{
	struct gpio_desc *vbus_gpio;
	int i;

	vbus_gpio = mtk->gpios;
	for (i = 0; i < mtk->num_gpios; i++) {
		if (dm_gpio_is_valid(&vbus_gpio[i]))
			dm_gpio_set_value(&vbus_gpio[i], 0);
	}
}

static int xhci_mtk_port_count(struct mtk_xhci *mtk)
{
	struct mtk_ippc_regs *ippc = mtk->ippc;
	u32 value;

	value = readl(&ippc->ip_xhci_cap);
	return CAP_U3_PORT_NUM(value) + CAP_U2_PORT_NUM(value);
}

#ifdef CONFIG_PHY_MTK_TPHY
extern int mtk_tphy_i2c_ctrl(struct phy *);
#endif
static int xhci_mtk_phy_setup(struct mtk_xhci *mtk)
{
	struct udevice *dev = mtk->dev;
	struct phy *usb_phys;
	int i, ret, count;

	/* Return if no phy declared */
	if (!dev_read_prop(dev, "phys", NULL)) {
		pr_err("No USB PHY declared in dts\n");
		return 0;
	}

	count = dev_count_phandle_with_args(dev, "phys", "#phy-cells");
	if (count <= 0)
		return count;

	usb_phys = devm_kcalloc(dev, count, sizeof(*usb_phys),
				GFP_KERNEL);
	if (!usb_phys)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = generic_phy_get_by_index(dev, i, &usb_phys[i]);
		if (ret && ret != -ENOENT) {
			dev_err(dev, "Failed to get USB PHY%d for %s\n",
				i, dev->name);
			devm_kfree(dev, usb_phys);
			return ret;
		}
	}

#ifdef CONFIG_PHY_MTK_TPHY
	mtk_tphy_i2c_ctrl(usb_phys);
#endif
	if (count != xhci_mtk_port_count(mtk))
		dev_err(dev, "USB PHY not matched with dts\n");
	count = min(count, xhci_mtk_port_count(mtk));

	for (i = 0; i < count; i++) {
		ret = generic_phy_init(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't init USB PHY%d for %s\n",
				i, dev->name);
			goto phys_init_err;
		}
	}

	for (i = 0; i < count; i++) {
		ret = generic_phy_power_on(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't power USB PHY%d for %s\n",
				i, dev->name);
			goto phys_poweron_err;
		}
	}

	mtk->phys = usb_phys;
	mtk->num_phys =  count;

	return 0;

phys_poweron_err:
	for (i = count - 1; i >= 0; i--)
		generic_phy_power_off(&usb_phys[i]);

	for (i = 0; i < count; i++)
		generic_phy_exit(&usb_phys[i]);

	devm_kfree(dev, usb_phys);

	return ret;

phys_init_err:
	for (; i >= 0; i--)
		generic_phy_exit(&usb_phys[i]);

	devm_kfree(dev, usb_phys);

	return ret;
}

static void xhci_mtk_phy_shutdown(struct mtk_xhci *mtk)
{
	struct udevice *dev = mtk->dev;
	struct phy *usb_phys;
	int i, ret;

	usb_phys = mtk->phys;
	for (i = 0; i < mtk->num_phys; i++) {
		if (!generic_phy_valid(&usb_phys[i]))
			continue;

		ret = generic_phy_power_off(&usb_phys[i]);
		ret |= generic_phy_exit(&usb_phys[i]);
		if (ret) {
			dev_err(dev, "Can't shutdown USB PHY%d for %s\n",
				i, dev->name);
		}
	}
}

static int xhci_mtk_misc_setup(struct mtk_xhci *mtk)
{
#ifdef CONFIG_USB_MTU3EX_PHY
#define _MSTAR_PM_BASE         0x1C000000
#define MTK_SSUSB_MISC_1_BASE    (_MSTAR_PM_BASE+(0x3D8100 << 1))
#define MTK_SSUSB_SRAM_SRC     (MTK_SSUSB_MISC_1_BASE+0x30*4)
#define MTK_SSUSB_SRAM_SRCSEL  (0x4)
	u32 temp;

	writew(readw((void*)(MTK_SSUSB_MISC_1_BASE+0x00*4)) | 0x1, (void*)(MTK_SSUSB_MISC_1_BASE+0x00*4));
	printf("\tXHCI EMI setting: 0x%x\n", readw((void*)(MTK_SSUSB_MISC_1_BASE+0x00*4)));
	writew(readw((void*)(MTK_SSUSB_MISC_1_BASE+0x2f*4)) | 0x1, (void*)(MTK_SSUSB_MISC_1_BASE+0x2f*4));
	printf("\tXHCI SSUSB UTMI select: 0x%x\n", readw((void*)(MTK_SSUSB_MISC_1_BASE+0x2f*4)));
#if 0 // M6E1
#define MTK_CHIPTOP_BASE    	(_MSTAR_PM_BASE+(0x101000 << 1))
	writew(readw((void*)(MTK_CHIPTOP_BASE+0x2d*4)) & 0xfeff, (void*)(MTK_CHIPTOP_BASE+0x2d*4));
	printf("\tBoot sram src: 0x%x\n", readw((void*)(MTK_CHIPTOP_BASE+0x2d*4)));
	writew((readw((void*)(MTK_CHIPTOP_BASE+0x1e*4)) & (~7))| 0x0002, (void*)(MTK_CHIPTOP_BASE+0x1e*4));
	printf("\tXHCI dump setting: 0x%x\n", readw((void*)(MTK_CHIPTOP_BASE+0x1e*4)));
#else
	temp = ((readw((void*)(MTK_SSUSB_SRAM_SRC)) & (~0xf)) | MTK_SSUSB_SRAM_SRCSEL);
	writew(temp, (void*)(MTK_SSUSB_SRAM_SRC));
	printf("\tXHCI dump setting + Boot sram src: 0x%x\n", readw((void*)(MTK_SSUSB_SRAM_SRC)));
#endif
#endif
	return 0;
}

#define HFCNTR_CFG       0x944
#define INIT_FRMCNT_LEV1_FULL_RANG_MASK (((u32)0xFFF) << 8)
#define INIT_FRMCNT_LEV1_FULL_RANG(V)   (V << 8)
#define REG_CP6_CP8_TXDEEMPH_10G 0x242c
#define TXDEEMPH_10G_VAL 0xe
static void xhci_mtk_post_adjust(struct mtk_xhci *mtk)
{
#ifdef CONFIG_USB_MTU3EX_PHY
	u32 temp;
	void __iomem *addr;

	addr = (void __iomem *)((ulong)mtk->hcd + HFCNTR_CFG);
	temp = readl(addr);
	//printf("[MTK] +++== SOF setting %x @0x%p\n", temp, addr);
	temp &= ~(INIT_FRMCNT_LEV1_FULL_RANG_MASK);
	//temp |= INIT_FRMCNT_LEV1_FULL_RANG(74); // 12M clock
	//temp |= INIT_FRMCNT_LEV1_FULL_RANG(299); // 48M clock, M6E1, M6L
	//temp |= INIT_FRMCNT_LEV1_FULL_RANG(374); // 60M clock
	temp |= INIT_FRMCNT_LEV1_FULL_RANG(149); // 24M clock
	writel(temp, addr);
	temp = readl(addr);
	//printf("[MTK] === SOF setting %x\n", temp);
#endif
	if (mtk->phy_mixed && (mtk->num_u3_ports != 0)) {
		void __iomem *addr_t;

		addr_t = (void __iomem *)
				((ulong)mtk->hcd + REG_CP6_CP8_TXDEEMPH_10G);
		writel(readl(addr_t) | TXDEEMPH_10G_VAL, addr_t);
		debug("\tSSUSB TXDEEMPH_10G_VAL value: 0x%x\n", readl(addr_t));
	}
}

static int xhci_mtk_probe(struct udevice *dev)
{
	struct mtk_xhci *mtk = dev_get_priv(dev);
	struct xhci_hcor *hcor;
	int ret;

	printf("MTK XHCI version: %s\n", MTK_UBOOT_XHCI_VERSION);
	mtk->dev = dev;
	ret = xhci_mtk_ofdata_get(mtk);
	if (ret)
		return ret;

	ret = xhci_mtk_misc_setup(mtk);
	if (ret)
		goto ldos_err;

	ret = xhci_mtk_ldos_enable(mtk);
	if (ret)
		goto ldos_err;

	ret = xhci_mtk_clks_enable(mtk);
	if (ret)
		goto clks_err;

	ret = xhci_mtk_gpios_enable(mtk);
	if (ret)
		goto gpios_err;

	ret = xhci_mtk_phy_setup(mtk);
	if (ret)
		goto phys_err;

	ret = xhci_mtk_ssusb_init(mtk);
	if (ret)
		goto ssusb_init_err;

	hcor = (struct xhci_hcor *)((uintptr_t)mtk->hcd +
			HC_LENGTH(xhci_readl(&mtk->hcd->cr_capbase)));

	xhci_mtk_post_adjust(mtk);

	return xhci_register(dev, mtk->hcd, hcor);

ssusb_init_err:
	xhci_mtk_phy_shutdown(mtk);
phys_err:
	xhci_mtk_gpios_disable(mtk);
gpios_err:
	xhci_mtk_clks_disable(mtk);
clks_err:
	xhci_mtk_ldos_disable(mtk);
ldos_err:
	return ret;
}

static int xhci_mtk_remove(struct udevice *dev)
{
	struct mtk_xhci *mtk = dev_get_priv(dev);

	xhci_deregister(dev);
	xhci_mtk_host_disable(mtk);
	xhci_mtk_ldos_disable(mtk);
	xhci_mtk_clks_disable(mtk);

	return 0;
}

static const struct udevice_id xhci_mtk_ids[] = {
	{ .compatible = "mediatek,mtk-dtv-mtu3" },
	{ .compatible = "mediatek,mtk-xhci" },
	{ }
};

U_BOOT_DRIVER(usb_xhci) = {
	.name	= "xhci-mtk",
	.id	= UCLASS_USB,
	.of_match = xhci_mtk_ids,
	.probe = xhci_mtk_probe,
	.remove = xhci_mtk_remove,
	.ops	= &xhci_usb_ops,
	.bind	= dm_scan_fdt_dev,
	.priv_auto_alloc_size = sizeof(struct mtk_xhci),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};
