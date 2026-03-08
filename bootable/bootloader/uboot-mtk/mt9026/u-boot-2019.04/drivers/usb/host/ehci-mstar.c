// SPDX-License-Identifier: GPL-2.0+
/*
 * Mediatek/Mstar USB 2.0 EHCI Controller
 *
 * (C) Copyright 2020 Mediatek Inc.
 */

#include <clk.h>
#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include "ehci.h"
#include "ehci-mstar.h"

//#define debug printf

#if CONFIG_IS_ENABLED(DM_USB)

struct mstar_hcd {
	struct ehci_ctrl ctrl;
	struct udevice *dev;
	u32	root_index;
	u32	utmi_base;
	u32	ehc_base;
	u32	usbc_base;
	u32	bc_base;
	u32     init_flag;
};

static u32 miu0_bus_base_addr = 0;
unsigned int VA2PA(unsigned int u32_DMAAddr)
{
        u32_DMAAddr -= miu0_bus_base_addr;
        u32_DMAAddr += MIU0_PHY_BASE_ADDR;
        return u32_DMAAddr;
}

#if defined(ENABLE_USB_NEW_MIU_SEL)
static void MIU_select_setting_ehc(unsigned int USBC_base)
{
	debug("[USB] config miu select [%x] [%x] [%x] ][%x]\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
	writeb(USB_MIU_SEL0, USBC_base+0x14*2); //Setting MIU0 segment
	writeb(USB_MIU_SEL1, USBC_base+0x16*2); //Setting MIU1 segment
	writeb(USB_MIU_SEL2, USBC_base+0x17*2-1);  //Setting MIU2 segment
	writeb(USB_MIU_SEL3, USBC_base+0x18*2); //Setting MIU3 segment
	writeb(readb(USBC_base+0x19*2-1) | 0x01, USBC_base+0x19*2-1); //Enable miu partition mechanism
#if !defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
	debug("[USB] enable miu lower bound address subtraction\n");
	writeb(readb(USBC_base+0x0F*2-1) | 0x01, USBC_base+0x0F*2-1);
#endif
}
#endif

static int tv_usb_init(struct mstar_hcd *hcd)
{
	printf("\n[USB] tv_usb_init (UTMI Init) ++\n");
	printf("[USB] UHC Base 0x%8x\n", hcd->ehc_base);
	debug("[USB] UTMI Base 0x%8x\n", hcd->utmi_base);
	debug("[USB] USBC Base 0x%8x\n", hcd->usbc_base);

#if defined(ENABLE_USB_NEW_MIU_SEL)
	MIU_select_setting_ehc(hcd->usbc_base);
#endif
	/* force DP/DM pull down while HC reset: set */
	writeb(readb(hcd->utmi_base) | 0xc2, hcd->utmi_base);
	/* new HW term overwrite: on */
	writeb(readb((hcd->utmi_base+0x52*2)) | (MS_BIT5|MS_BIT4|
		MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), (hcd->utmi_base+0x52*2));

	writeb(0x0A, hcd->usbc_base);	/* Disable MAC initial suspend, Reset UHC */
	writeb(0x28, hcd->usbc_base);	/* Release UHC reset, enable UHC XIU function */

	/* Init UTMI squelch level setting before CA */
	if(UTMI_DISCON_LEVEL_2A & (MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0))
	{
		writeb((UTMI_DISCON_LEVEL_2A & (MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0)),
			(hcd->utmi_base+0x2a*2));
		debug("[USB] init squelch level 0x%x\n", readb((hcd->utmi_base+0x2a*2)));
	}

#if defined(ENABLE_NEW_HW_CHRIP_PATCH)
	/* Init chrip detect level setting */
#if defined(UTMI_CHIRP_DCT_LEVEL_42)
	writeb(UTMI_CHIRP_DCT_LEVEL_42, (hcd->utmi_base+0x42*2));
#else
	writeb(0x80, (hcd->utmi_base+0x42*2));
#endif
	/* enable HW control chrip/disconnect level */
	writeb(readb((hcd->utmi_base+0x40*2)) & (u8)(~(MS_BIT3)), (hcd->utmi_base+0x40*2));
#endif


	writeb(readb((hcd->utmi_base+0x3C*2)) | 0x01, (hcd->utmi_base+0x3C*2));	/* set CA_START as 1 */

	mdelay(1); // 10->1

	writeb(readb((hcd->utmi_base+0x3C*2)) & ~0x01, (hcd->utmi_base+0x3C*2));	/* release CA_START */
#if defined(ENABLE_HS_DM_KEEP_HIGH_ECO)
	writeb(readb(hcd->utmi_base+0x10*2) | 0x40, (hcd->utmi_base+0x10*2)); // bit<6> for monkey test and HS current
#endif
#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA)
        /* skip polling on FPGA environment */
#else
	while (((unsigned int)(readb((hcd->utmi_base+0x3C*2))) & 0x02) == 0);	/* polling bit <1> (CA_END) */
#endif

	if (hcd->init_flag & EHCFLAG_DPDM_SWAP)
		writeb(readb(hcd->utmi_base+0x0b*2-1) |0x20, hcd->utmi_base+0x0b*2-1);	/* dp dm swap */

#if defined(ENABLE_UTMI_DPDM_SWAP_P1_FOR_MARLON_P2P_MILAN)
	if (hcd->port_index == 1) {
		/* Marlon may share the same HAL as Milan, so we need to check chip ID register */
		if (MDrv_SYS_GetChipID() == 0x00C8) {
			/* Marlon need to swap DP/DM for P2P Milan 216/256-pin */
			if ((readb((_MSTAR_BONDING_OPTION_BASE+0xC1*2-1)) & 0xC0) == 0xC0) {
				writeb(readb(hcd->utmi_base+0x0b*2-1) |0x20, hcd->utmi_base+0x0b*2-1);	/* dp dm swap */
			}
		}
	}
#endif
#if defined(ENABLE_UTMI_DPDM_SWAP_P1_FOR_MARLON_NEW_216PIN_TCON)
	if (hcd->port_index == 1) {
		/* Marlon may share the same HAL as Milan, so we need to check chip ID register */
		if (MDrv_SYS_GetChipID() == 0x00C8) {
			/* Marlon need to swap DP/DM for new 216-pin (TCON) */
			if ((readb((_MSTAR_BONDING_OPTION_BASE+0xC1*2-1)) & 0xC0) == 0x00) {
				writeb(readb(hcd->utmi_base+0x0b*2-1) |0x20, hcd->utmi_base+0x0b*2-1);	/* dp dm swap */
			}
		}
	}
#endif
	writeb(readb((hcd->usbc_base+0x02*2)) & ~0x03, (hcd->usbc_base+0x02*2));	/* UHC select enable */
	writeb(readb((hcd->usbc_base+0x02*2)) | 0x01, (hcd->usbc_base+0x02*2));	/* UHC select enable */

	writeb(readb((hcd->ehc_base+0x40*2)) & ~0x10, (hcd->ehc_base+0x40*2));	/* 0: VBUS On. */
	udelay(1);	/* delay 1us */
	writeb(readb((hcd->ehc_base+0x40*2)) | 0x08, (hcd->ehc_base+0x40*2));	/* Active HIGH */

	/* force DP/DM pull down while HC reset: release */
	writeb(readb(hcd->utmi_base) & ~0xc2, hcd->utmi_base);
	/* new HW term overwrite: off */
	writeb(readb((hcd->utmi_base+0x52*2)) & ~(MS_BIT5|MS_BIT4|
		MS_BIT3|MS_BIT2|MS_BIT1|MS_BIT0), (hcd->utmi_base+0x52*2));

	writeb((readb((hcd->utmi_base+0x06*2)) & 0x9F) | 0x40, (hcd->utmi_base+0x06*2));	/* reg_tx_force_hs_current_enable */

	writeb(readb(hcd->utmi_base+0x03*2-1) | 0x28, hcd->utmi_base+0x03*2-1);	/* Disconnect window select */
	writeb(readb(hcd->utmi_base+0x03*2-1) & 0xef, hcd->utmi_base+0x03*2-1);	/* Disconnect window select */
	writeb(readb(hcd->utmi_base+0x07*2-1) & 0xfd, hcd->utmi_base+0x07*2-1);	/* Disable improved CDR */

#if defined(ENABLE_UTMI_240_AS_120_PHASE_ECO)
	#if defined(UTMI_240_AS_120_PHASE_ECO_INV)
	writeb(readb(hcd->utmi_base+0x08*2) & ~0x08,  (hcd->utmi_base+0x08*2)); // bit<3> special for Eiffel analog LIB issue
	#elif defined(UTMI_240_AS_120_PHASE_ECO_INV_IF_REV_0)
	if (MDrv_SYS_GetChipRev() == 0x0) {
		writeb(readb(hcd->utmi_base+0x08*2) & ~0x08, (hcd->utmi_base+0x08*2)); // bit<3> special for Eiffel analog LIB issue
	}
	#else
	writeb(readb(hcd->utmi_base+0x08*2) | 0x08, hcd->utmi_base+0x08*2);	/* bit<3> for 240's phase as 120's clock set 1, bit<4> for 240Mhz in mac 0 for faraday 1 for etron */
	#endif
#endif

	writeb(readb(hcd->utmi_base+0x09*2-1) | 0x81, hcd->utmi_base+0x09*2-1);	/* UTMI RX anti-dead-loc, ISI effect improvement */
#if defined(UTMI_TX_TIMING_SEL_LATCH_PATH_INV_IF_REV_0)
	if (MDrv_SYS_GetChipRev() == 0x0) {
		writeb(readb(hcd->utmi_base+0x0b*2-1) & ~0x80, hcd->utmi_base+0x0b*2-1);	/* TX timing select latch path */
	}
#else
	writeb(readb(hcd->utmi_base+0x0b*2-1) | 0x80, hcd->utmi_base+0x0b*2-1);	/* TX timing select latch path */
#endif
	writeb(readb(hcd->utmi_base+0x15*2-1) | 0x20, hcd->utmi_base+0x15*2-1);	/* Chirp signal source select */
#if defined(ENABLE_UTMI_55_INTERFACE)
	writeb(readb(hcd->utmi_base+0x15*2-1) | 0x40, hcd->utmi_base+0x15*2-1);	/* change to 55 interface */
#endif

#if defined(ENABLE_PV2MI_BRIDGE_ECO)
	writeb(readb(hcd->usbc_base+0x0a*2) | 0x40, hcd->usbc_base+0x0a*2);
#endif
	/* Init UTMI eye diagram parameter setting */
	writeb(readb(hcd->utmi_base+0x2c*2) | UTMI_EYE_SETTING_2C, hcd->utmi_base+0x2c*2);
	writeb(readb(hcd->utmi_base+0x2d*2-1) | UTMI_EYE_SETTING_2D, hcd->utmi_base+0x2d*2-1);
	writeb(readb(hcd->utmi_base+0x2e*2) | UTMI_EYE_SETTING_2E, hcd->utmi_base+0x2e*2);
	writeb(readb(hcd->utmi_base+0x2f*2-1) | UTMI_EYE_SETTING_2F, hcd->utmi_base+0x2f*2-1);

#if _USB_MIU_WRITE_WAIT_LAST_DONE_Z_PATCH
	/* Enabe PVCI i_miwcplt wait for mi2uh_last_done_z */
	writeb(readb(hcd->ehc_base+0x83*2-1) | MS_BIT4, hcd->ehc_base+0x83*2-1);
#endif

	//usb_bc_enable(hcd->utmi_base, hcd->bc_base, FALSE);

	debug("[USB] tv_usb_init --\n");
	return 0;
}

static int tv_usb_stop(struct mstar_hcd *hcd)
{
	debug("[USB] tv_usb_stop ++\n");
	return 0;
}

static void mstar_ehci_powerup_fixup(struct ehci_ctrl *dev, uint32_t *status_reg,
			       uint32_t *reg)
{
	struct mstar_hcd *hcd = container_of(dev, struct mstar_hcd, ctrl);

	writeb(0x10, hcd->utmi_base+0x2c*2);
	writeb(0x00, hcd->utmi_base+0x2d*2-1);
	writeb(0x00, hcd->utmi_base+0x2e*2);
	writeb(0x00, hcd->utmi_base+0x2f*2-1);
	writeb(0x80, hcd->utmi_base+0x2a*2);
	mdelay(30);
	writeb(UTMI_DISCON_LEVEL_2A, hcd->utmi_base+0x2a*2);
	mdelay(20);
	writeb(UTMI_EYE_SETTING_2C, hcd->utmi_base+0x2c*2);
	writeb(UTMI_EYE_SETTING_2D, hcd->utmi_base+0x2d*2-1);
	writeb(UTMI_EYE_SETTING_2E, hcd->utmi_base+0x2e*2);
	writeb(UTMI_EYE_SETTING_2F, hcd->utmi_base+0x2f*2-1);
}

static int mstar_ehci_get_port_speed(struct ehci_ctrl *ctrl, uint32_t reg)
{
	int spd, ret = PORTSC_PSPD_HS;

	spd = BMCSR_SPD(ehci_readl(&ctrl->hcor->or_bmcs));

	switch (spd) {
	case 0:    /* full speed */
		ret = PORTSC_PSPD_FS;
		break;
	case 1:    /* low  speed */
		ret = PORTSC_PSPD_LS;
		break;
	case 2:    /* high speed */
		ret = PORTSC_PSPD_HS;
		break;
	default:
		printf("ehci-mstar: invalid device speed\n");
		break;
	}

	return ret;
}

static uint32_t *mstar_ehci_get_portsc_register(struct ehci_ctrl *ctrl, int port)
{
	/* Mstar EHCI has one and only one portsc register */
	if (port) {
		/* Printing the message would cause a scan failure! */
		printf("The request port(%d) is not configured\n", port);
		return NULL;
	}

	/* Mstar EHCI PORTSC register offset is 0x20 from hcor */
	return (uint32_t *)((uint8_t *)ctrl->hcor + 0x20);
}

static const struct ehci_ops mstar_ehci_ops = {
	.set_usb_mode		= NULL,
	.get_port_speed		= mstar_ehci_get_port_speed,
	.powerup_fixup		= mstar_ehci_powerup_fixup,
	.get_portsc_register	= mstar_ehci_get_portsc_register,
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct mstar_hcd *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int err;

	err = 0;

	priv->ehc_base = dev_read_addr_name(dev, "uhc");
	priv->utmi_base = dev_read_addr_name(dev, "utmi");
	priv->usbc_base = dev_read_addr_name(dev, "usbc");
	err = dev_read_u32(dev, "root_index", &priv->root_index);
	if (err)
		goto init_err;

	err = tv_usb_init(priv);
	if (err)
		goto init_err;

	hccr = map_physmem(priv->ehc_base, 0x200, MAP_NOCACHE);
	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, &mstar_ehci_ops, 0, USB_INIT_HOST);

init_err:
	return err;
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct mstar_hcd *priv = dev_get_priv(dev);
	int ret;

	debug("[USB] %s()\n", __func__);
	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	ret = tv_usb_stop(priv);
	return ret;
}

static int mstar_ehci_ofdata_to_platdata(struct udevice *dev)
{
	struct mstar_hcd *priv = dev_get_priv(dev);
	int ret = 0;

	debug("[USB] %s()\n", __func__);
	if (!miu0_bus_base_addr)
		miu0_bus_base_addr = MIU0_BUS_BASE_ADDR;

	return ret;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "mediatek,mstar-ehci-1" },
	{ .compatible = "mediatek,mstar-ehci-2" },
	{ .compatible = "mediatek,mstar-ehci-3" },
	{ .compatible = "mediatek,mstar-ehci-4" },
	{ }
};

U_BOOT_DRIVER(ehci_mstar) = {
	.name	= "ehci_mstar",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.ofdata_to_platdata = mstar_ehci_ofdata_to_platdata,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct mstar_hcd),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

#endif /* CONFIG_IS_ENABLED(DM_USB) */
