// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <remoteproc.h>
#include <errno.h>
#include <mailbox.h>
#include <time.h>
#include <asm/io.h>
#include <fdtdec.h>

/* address map */
// MI_PQU_FW_MEM
#define RV55_RES_ADR    0x10000
#define EMI_C_MAP_BASE	0x20000000
#define EMI_NC_MAP_BASE	0x60000000

/* register bank */
#define RIUBASE		0x1c000000
#define RV55FRCPLL	(RIUBASE + (0x100600 << 1))
#define CKGEN00		(RIUBASE + (0x102000 << 1))
#define CKGEN01		(RIUBASE + (0x103000 << 1))
#define TZPC_AID_1	(RIUBASE + (0x120400 << 1))
#define PQU_RV55INTG	(RIUBASE + (0x812100 << 1))
#define PQU_RV55INTG_HWDBG	(RIUBASE + (0x812200 << 1))

/* reg */
#define REG_SW_RSTZ	0
#define REG_I0_ADR_OFFSET_0	0x48
#define REG_I0_ADR_OFFSET_1	0x4c
#define REG_D0_ADR_OFFSET_0	0x60
#define REG_D0_ADR_OFFSET_1	0x64

#define REG_DBG_EN	0x40



DEFINE_MUTEX(clk_lock);

/* physical address to emi offset */
static phys_addr_t pa_to_emi_offset(phys_addr_t pa)
{
	if (pa > EMI_NC_MAP_BASE)
		return pa - EMI_NC_MAP_BASE;
	else
		return pa - EMI_C_MAP_BASE;
}

/**
 * struct mtk_pqu_privdata - Structure representing System Controller data.
 * @chan_tx:    Transmit mailbox channel
 * @chan_rx:    Receive mailbox channel
 * @desc:    SoC description for this instance
 * @seq_nr:    Counter for number of messages sent.
 */
struct mtk_pqu_privdata {
	phys_addr_t fw_addr;
	phys_size_t fw_size;
	phys_size_t intg_base;
	phys_size_t hwdbg_base;
};

static int mrv_common_clk_init(void)
{
	unsigned long tmp;

	/* ==== common ==== */
	/* frc pll */
	writew(0x1110, RV55FRCPLL + 0x00);
	writew(0, RV55FRCPLL + 0x10);
	writew(0, RV55FRCPLL + 0x18);
	writew(0, RV55FRCPLL + 0x1c);
	writew(0x53, RV55FRCPLL + 0x20);
	udelay(100);
	writew(0x1100, RV55FRCPLL + 0x00);

	/* xtal_24m */
	/* clk_axi0*/
	tmp = readw(CKGEN00 + 0x1d8);	// reg_ckg_smi_frciopmrv55[11:10]
	__clear_bit(10, &tmp);
	__clear_bit(11, &tmp);
	writew(tmp, CKGEN00 + 0x1d8);

	tmp = readw(CKGEN00 + 0x14e4);
	__set_bit(9, &tmp);	// reg_sw_en_smi2frciopmrv55[9]
	writew(tmp, CKGEN00 + 0x14e4);

	/* clk_axi1 */
	writew(0x4, CKGEN00 + 0x260);	// reg_ckg_frc_imi[3:0]
					// reg_ckg_frc_imi_frciopmrv55[9:8]
					// reg_ckg_frc_imi_rv55frc[11:10]
					// reg_ckg_frc_imi_rv55pqu[13:12]

	tmp = readw(CKGEN00 + 0x1520);
	__set_bit(0, &tmp); // reg_sw_en_frc_imi2frciopm[0]
	__set_bit(1, &tmp); // reg_sw_en_frc_imi2frciopmrv55[1]
	writew(tmp, CKGEN00 + 0x1520);

	/* clk_xiu0 */
	tmp = readw(CKGEN01 + 0x11b8);
	tmp &= ~0x300U;	// reg_ckg_mcu_bus_frciopmrv55[9:8]
	writew(tmp, CKGEN01 + 0x11b8);

	/* clk_xiu1 */
	tmp = readw(CKGEN01 + 0x1128);
	tmp &= ~0x300U;	// reg_ckg_mcu_frc_frciopmrv55[9:8]
	writew(tmp, CKGEN01 + 0x1128);

	/* pclk */
	/* jtag_clk */
	/* misc */
	tmp = readw(CKGEN01 + 0x1b74);	//reg_sw_en_mcu_nonpm2frciopmrv55[6]
	__set_bit(6, &tmp);
	writew(tmp, CKGEN01 + 0x1b74);

	tmp = readw(CKGEN01 + 0x1b7c);
	__set_bit(11, &tmp);	// reg_sw_en_mcu_nonpm2rv55pqu[11]
	__set_bit(10, &tmp);	// reg_sw_en_mcu_nonpm2rv55frc[10]
	writew(tmp, CKGEN01 + 0x1b7c);

	tmp = readw(CKGEN01 + 0x1bb4);
	__set_bit(0, &tmp);	//reg_sw_en_mcu_sc2rv55frc[0]
	__set_bit(1, &tmp);	//reg_sw_en_mcu_sc2rv55pqu[1]
	writew(tmp, CKGEN01 + 0x1bb4);

	/* frc_uart */
	tmp = readw(CKGEN01 + 0x15b4);
	__set_bit(1, &tmp);	//reg_sw_en_xtal_12m2frciopmrv55[1]
	writew(tmp, CKGEN01 + 0x15b4);

	tmp = readw(CKGEN01 + 0x1ae8);
	__set_bit(1, &tmp);	//reg_sw_en_frc_uart02frciopmrv55[1]
	writew(tmp, CKGEN01 + 0x1ae8);

	tmp = readw(CKGEN01 + 0x1af0);
	__set_bit(1, &tmp);	//reg_sw_en_frc_uart12frciopmrv55[1]
	writew(tmp, CKGEN01 + 0x1af0);

	tmp = 0xf2;	//reg_uart_sel4[3:0] = 2
	writew(tmp, 0x1c601008);

	/* ==== pqu ==== */
	/* xtal_24m */
	tmp = readw(CKGEN01 + 0x15c0);
	__set_bit(7, &tmp); // reg_sw_en_xtal_24m2rv55pqu [7]
	writew(tmp, CKGEN01 + 0x15c0);

	/* rv */
	tmp = readw(CKGEN01 + 0x220);
	tmp &= ~0x3U;
	tmp |= 0x4U;	// reg_ckg_rv55pqu [2:0]
	tmp &= ~0x300U;	// reg_ckg_rv55pqu_frciopmrv55 [9:8]
	writew(tmp, CKGEN01 + 0x220);

	tmp = readw(CKGEN01 + 0x218);
	tmp &= ~0x1fU;	// reg_ckg_rv55pqu_rv55pqu [4:0]
	tmp |= 0x4U;
	writew(tmp, CKGEN01 + 0x218);

	tmp = readw(CKGEN01 + 0x15a8);
	tmp |= 0x1U;	// reg_sw_en_rv55pqu2rv55pqu [0]
	writew(tmp, CKGEN01 + 0x15a8);

	/* smi */
	tmp = readw(CKGEN00 + 0x1e8);	// reg_ckg_smi_rv55pqu[13:12]
	__clear_bit(12, &tmp);
	__clear_bit(13, &tmp);
	writew(tmp, CKGEN00 + 0x1e8);

	tmp = readw(CKGEN00 + 0x14e8); // reg_sw_en_smi2rv55pqu[13]
	__set_bit(13, &tmp);
	writew(tmp, CKGEN00 + 0x14e8);

	/* clk_axi1 */
	tmp = readw(CKGEN00 + 0x1520);
	__set_bit(3, &tmp); // reg_sw_en_frc_imi2rv55pqu[3]
	writew(tmp, CKGEN00 + 0x1520);

	/* clk_xiu0 */

	tmp = readw(CKGEN01 + 0x11c0);
	tmp &= ~0x300U;	// reg_ckg_mcu_bus_rv55pqu[9:8]
	writew(tmp, CKGEN01 + 0x11c0);

	tmp = readw(CKGEN01 + 0x1b4c);
	__set_bit(0, &tmp);	// reg_sw_en_mcu_bus2frciopm[0]
	__set_bit(1, &tmp);	// reg_sw_en_mcu_bus2frciopmrv55[1]
	__set_bit(8, &tmp);	// reg_sw_en_mcu_bus2rv55pqu[8]
	writew(tmp, CKGEN01 + 0x1b4c);

	/* clk_xiu1 */
	tmp = readw(CKGEN01 + 0x1128);
	tmp &= ~0x3000U;	// reg_ckg_mcu_frc_rv55pqu [13:12]
	writew(tmp, CKGEN01 + 0x1128);

	tmp = readw(CKGEN01 + 0x1b64);
	__set_bit(3, &tmp);	// reg_sw_en_mcu_frc2rv55pqu[3]
	writew(tmp, CKGEN01 + 0x1b64);

	/* ==== frc ==== */
	/* xtal_24m */
	tmp = readw(CKGEN01 + 0x15c0);
	__set_bit(6, &tmp); // reg_sw_en_xtal_24m2rv55frc [6]
	writew(tmp, CKGEN01 + 0x15c0);

	/* clk_rv */
	tmp = readw(CKGEN01 + 0x208);
	tmp |= 0x4U;	// reg_ckg_rv55frc [2:0]
	tmp &= ~0x300U;	// reg_ckg_rv55frc_frciopmrv55 [9:8]
	writew(tmp, CKGEN01 + 0x208);

	tmp = readw(CKGEN01 + 0x200);
	tmp &= ~0x1fU;	// reg_ckg_rv55pqu_rv55pqu [4:0]
	tmp |= 0x4U;
	writew(tmp, CKGEN01 + 0x200);

	tmp = readw(CKGEN01 + 0x15a4);
	tmp |= 0x1U;	// reg_sw_en_rv55frc2rv55frc [0]
	writew(tmp, CKGEN01 + 0x15a4);

	/* clk_axi0 */
	tmp = readw(CKGEN00 + 0x1e8);	// reg_ckg_smi_rv55frc[10:11]
	__clear_bit(11, &tmp);
	__clear_bit(10, &tmp);
	writew(tmp, CKGEN00 + 0x1e8);

	tmp = readw(CKGEN00 + 0x14e8); // reg_sw_en_smi2rv55frc[12]
	__set_bit(12, &tmp);
	writew(tmp, CKGEN00 + 0x14e8);

	/* clk_axi1 */
	tmp = readw(CKGEN00 + 0x1520);
	__set_bit(2, &tmp); // reg_sw_en_frc_imi2rv55frc[2]
	writew(tmp, CKGEN00 + 0x1520);

	/* clk_xiu0 */
	tmp = readw(CKGEN01 + 0x11c0);
	__clear_bit(6, &tmp); // reg_ckg_mcu_bus_rv55frc[7:6]
	__clear_bit(7, &tmp);
	writew(tmp, CKGEN01 + 0x11c0);

	tmp = readw(CKGEN01 + 0x1b4c);
	__set_bit(7, &tmp);	// reg_sw_en_mcu_bus2rv55frc[7]
	writew(tmp, CKGEN01 + 0x1b4c);

	/* clk_xiu1 */
	tmp = readw(CKGEN01 + 0x1128);
	__clear_bit(11, &tmp);	// reg_ckg_mcu_frc_rv55frc [11:10]
	__clear_bit(10, &tmp);
	writew(tmp, CKGEN01 + 0x1128);

	tmp = readw(CKGEN01 + 0x1b64);
	__set_bit(2, &tmp);	// reg_sw_en_mcu_frc2rv55frc[2]
	writew(tmp, CKGEN01 + 0x1b64);


	/* pclk */
	/* jtag_tck */

	return 0;
}

/**
 * mtk_pqu_init() - Initialize the remote processor
 * @dev:    rproc device pointer
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pqu_init(struct udevice *dev)
{
	int ret;
	struct mtk_pqu_privdata *priv = dev_get_priv(dev);
	phys_addr_t emi_offset;
	static bool common_clk_init = false;

	if (!priv) {
		dev_err(dev, "can not find priv\n");
		return -EINVAL;
	}

	mutex_lock(&clk_lock);
	dev_info(dev, "start rv55 clk init\n");

	if (common_clk_init == false) {
		ret = mrv_common_clk_init();
		if (ret) {
			dev_err(dev, "mrv_common clk init fail\n");
			mutex_unlock(&clk_lock);
			return -EIO;
		}

		common_clk_init = true;
	} else
		dev_info(dev, "skip mrv common clk init\n");


	mutex_unlock(&clk_lock);

	emi_offset = pa_to_emi_offset(priv->fw_addr);
	dev_info(dev, "emi_offset = %lx\n", (unsigned long)emi_offset);

	/* set I/D emi offset to PQU_EMI_OFFSET and set adr_msb. */
	writew((emi_offset & 0xffff), priv->intg_base + REG_I0_ADR_OFFSET_0);
	writew(emi_offset >> 16, priv->intg_base + REG_I0_ADR_OFFSET_1);
	writew((emi_offset & 0xffff), priv->intg_base + REG_D0_ADR_OFFSET_0);
	writew(emi_offset >> 16, priv->intg_base + REG_D0_ADR_OFFSET_1);

	/* enable hwdbg */
	writew(0x1, priv->hwdbg_base + REG_DBG_EN);

	dev_info(dev, "pqu init finish\n");
	return 0;
}

/**
 * mtk_pqu_load() - Loadup the cm4 remote processor
 * @dev:    corresponding pqu remote processor device
 * @addr:    Address in memory where image binary is stored
 * @size:    Size in bytes of the image binary
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_pqu_load(struct udevice *dev, ulong addr, ulong size)
{
	struct mtk_pqu_privdata *priv = dev_get_priv(dev);

	dev_info(dev, "loading firmware to addr %lx start\n", (unsigned long)priv->fw_addr);
	memcpy((void *)priv->fw_addr, (void *)addr, size);
	dev_info(dev, "complete!\n");
	return 0;
}

static int mtk_pqu_stop(struct udevice *dev)
{
	struct mtk_pqu_privdata *priv = dev_get_priv(dev);

	dev_info(dev, "stop %s\n", dev->name);
	writew(0x0, priv->intg_base + REG_SW_RSTZ);
	udelay(100);
	if (readw(priv->intg_base + REG_SW_RSTZ) != 0) {
		dev_err(dev, "fail to stop pqu\n");
		return -EIO;
	}

	return 0;
}

/**
 * mtk_pqu_start() - Start the remote processor
 * @dev:    device to operate upon
 *
 * Return: 0 if all went ok, else return appropriate error
 */
static int mtk_pqu_start(struct udevice *dev)
{
	struct mtk_pqu_privdata *priv = dev_get_priv(dev);

	dev_info(dev, "start %s\n", dev->name);
	writew(0x1, priv->intg_base + REG_SW_RSTZ);
	udelay(100);
	if (readw(priv->intg_base + REG_SW_RSTZ) != 1) {
		dev_err(dev, "fail to start pqu\n");
		return -EIO;
	}

	return 0;
}

static const struct dm_rproc_ops mtk_pqu_ops = {
    .init = mtk_pqu_init,
    .load = mtk_pqu_load,
    .start = mtk_pqu_start,
    .stop = mtk_pqu_stop,
};

/**
 * mtk_of_to_priv() - generate private data from device tree
 * @dev:    corresponding mtk remote processor device
 * @priv:    pointer to driver specific private data
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_pqu_of_to_priv(struct udevice *dev,
             struct mtk_pqu_privdata *priv)
{
	int ret;
	uint32_t val;

	ret = dev_read_u32(dev, "mmap-start", &val);
	priv->fw_addr = (phys_addr_t)val;
	ret |= dev_read_u32(dev, "mmap-size", &val);
	priv->fw_size = (phys_size_t)val;
	priv->intg_base = dev_read_addr_name(dev, "rv55intg");
	priv->hwdbg_base = dev_read_addr_name(dev, "rv55hwdbg");
	dev_info(dev, "intg_base = %lx\n", (unsigned long)priv->intg_base);
	dev_info(dev, "hwdbg_base = %lx\n", (unsigned long)priv->hwdbg_base);
	dev_info(dev, "mmap-start = %lx\n", (unsigned long)priv->fw_addr);
	dev_info(dev, "mmap-size = %lx\n", (unsigned long)priv->fw_size);
	return ret;
}

/**
 * mtk_pqu_probe() - Basic probe
 * @dev:    corresponding mtk remote processor device
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
static int mtk_pqu_probe(struct udevice *dev)
{
	struct mtk_pqu_privdata *priv;
	int ret;

	priv = dev_get_priv(dev);
	ret = mtk_pqu_of_to_priv(dev, priv);
	if (ret) {
		dev_err(dev, "%s: Probe failed with error %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id mtk_pqu_ids[] = {
    {.compatible = "mediatek,mt5896-pqu"},
    {.compatible = "mediatek,mt5896-frc"},
    {}
};

U_BOOT_DRIVER(mtk_pqu) = {
    .name = "pqu_rproc",
    .of_match = mtk_pqu_ids,
    .id = UCLASS_REMOTEPROC,
    .ops = &mtk_pqu_ops,
    .probe = mtk_pqu_probe,
    .priv_auto_alloc_size = sizeof(struct mtk_pqu_privdata),
};
