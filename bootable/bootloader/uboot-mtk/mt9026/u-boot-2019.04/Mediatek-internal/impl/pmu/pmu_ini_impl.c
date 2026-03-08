// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*****************************************************************************/
#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <linux/io.h>

#include "common.h"
#include "command.h"
#include "debug_impl.h"

/*****************************************************************************/
/* Alloc buffer. */
#define BUFF_ALIGN			(0x10000)
#define BUFF_SIZE			(0x1000)
#define BUFF_BOUNDARY		(BIT(32 + 12))
/* Address ops. */
#define ADDR_NUMS			(4)
#define ADDR_BIT32			(32)
#define ADDR_BYTE0			(0)
#define ADDR_BYTE1			(1)
#define ADDR_BYTE2			(2)
#define ADDR_BYTE3			(3)
#define ADDR_00_15			(0)
#define ADDR_16_31			(16)
#define ADDR_32_47			(32)
#define ADDR_48_63			(48)
/* Dummy for ini. */
#define PM_OFS_INI_48_63	(((0x52 << 1) * 2) + 0)
#define PM_OFS_INI_32_47	(((0x53 << 1) * 2) + 0)
#define PM_OFS_INI_16_31	(((0x54 << 1) * 2) + 0)
#define PM_OFS_INI_00_15	(((0x55 << 1) * 2) + 0)
/* Internal MMAP. */
#define PM_OFS_WIFI			(0x0000)
#define PM_OFS_BT			(0x0010)
#define PM_OFS_EMER			(0x0020)
#define PM_OFS_MIC_MUTE			(0x0030)
#define PM_OFS_HDMI33V_CONTROL		(0x0040)
#define PM_OFS_EXT_GPIO			(0x0050)
#define PM_OFS_IR			(0x0100)
#define PM_OFS_SAR			(0x0200)
#define PM_OFS_SAR_UPGRADE  		(0x0290)
#define PM_OFS_WIFI_RESET		(0x0300)
#define PM_OFS_GPIO_CONTROL		(0x0400)
#define PM_OFS_IR_EXTEND		(0x0500)


/* Misc */
#define DTS_STAT_DISABLE_NUM		(8)

#define PM_IR_EXTEND_FORMAT     (1)

/*****************************************************************************/
struct pm_ini_device {
	u32 base_addr;
	ulong buff_addr;
	ulong buff_size;
	ulong dram_addr;
	ofnode ini_node;
	void *ptr;
};

/*****************************************************************************/
static int pm_ini_probe(struct pm_ini_device *priv, bool *disable)
{
	ofnode node = {0};
	u32 addr[ADDR_NUMS] = {0};
	const char *prop = NULL, *status = NULL;
	int size = 0;
	struct resource regs;
	void __iomem *base;

	/* Check device. */
	priv->ini_node = ofnode_path("/pmu_ini");
	if (!ofnode_valid(priv->ini_node)) {
		UBOOT_ERROR("ofnode_path(/pmu_ini) not found.\n");
		return -ENODEV;
	}
	status = ofnode_read_string(priv->ini_node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		*disable = 1;
		return 0;
	}
	prop = ofnode_get_property(priv->ini_node, "compatible", &size);
	if ((!prop) || (strncmp(prop, "mediatek,pmu_ini", size - 1))) {
		UBOOT_ERROR("compatible no match.\n");
		return -ENOENT;
	}

	/* Get dummy address and write share memory address. */
	if (ofnode_read_resource(priv->ini_node, 0, &regs) < 0) {
		UBOOT_ERROR("ofnode_read_resource() not found.\n");
		return -ENOENT;
	}

	/* Get cpu bus address. */
	node = ofnode_path("/memory_info");
	if (!ofnode_valid(node) || (ofnode_read_u32(node, "cpu_emi0_base", &priv->base_addr) < 0)) {
		UBOOT_ERROR("node(/memory_info/cpu_emi0_base) not found.\n");
		return -EIO;
	}
	UBOOT_INFO("base_addr=0x%X.\n", priv->base_addr);

	/* Get share memory. */
	node = ofnode_path("/mmap_info/MI_PM51_VAR_MEM");
	if ((ofnode_valid(node)) && (ofnode_read_u32_array(node, "reg", addr, ADDR_NUMS) == 0)) {
		UBOOT_DEBUG("buff from node(/memory_info/MI_PM51_VAR_MEM/reg) path.\n");
		priv->buff_addr = (((u64)addr[ADDR_BYTE0] << ADDR_BIT32) | addr[ADDR_BYTE1]);
		priv->buff_size = (((u64)addr[ADDR_BYTE2] << ADDR_BIT32) | addr[ADDR_BYTE3]);
	} else if ((priv->ptr = memalign(BUFF_ALIGN, BUFF_SIZE))) {
		UBOOT_DEBUG("buff from malloc path.\n");
		priv->buff_addr = (u64)priv->ptr;
		priv->buff_size = BUFF_SIZE;
	} else {
		UBOOT_ERROR("buff node not found and malloc fail.\n");
		return -EFAULT;
	}
	priv->dram_addr = priv->buff_addr - priv->base_addr;
	UBOOT_INFO("dram_addr=0x%lX.\n", priv->dram_addr);
	UBOOT_INFO("buff_addr=0x%lX.\n", priv->buff_addr);
	UBOOT_INFO("buff_size=0x%lX.\n", priv->buff_size);
	if ((priv->dram_addr >= BUFF_BOUNDARY) || (priv->buff_size != BUFF_SIZE)) {
		UBOOT_ERROR("buff out of access range(0x%lX, 0x%lX).\n", priv->dram_addr, priv->buff_size);
		return -ENOMEM;
	}
	memset((void *)priv->buff_addr, 0, priv->buff_size);

	base = ioremap(regs.start, regs.end - regs.start);
	if (!base) {
		UBOOT_ERROR("ioremap() fail.\n");
		return -ENOMEM;
	}
	UBOOT_INFO("base=0x%tX.\n", (size_t)base);
	writew(priv->dram_addr >> ADDR_48_63, base + PM_OFS_INI_48_63);
	writew(priv->dram_addr >> ADDR_32_47, base + PM_OFS_INI_32_47);
	writew(priv->dram_addr >> ADDR_16_31, base + PM_OFS_INI_16_31);
	writew(priv->dram_addr >> ADDR_00_15, base + PM_OFS_INI_00_15);
	iounmap(base);
	return 0;
}

/*****************************************************************************/
#ifndef PM_IR_EXTEND_FORMAT
#define IR_KEY_NUMS	(16*10)

struct pm_ini_ir {
	u8 key[IR_KEY_NUMS];
};

static int pm_ini_ir_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_ir ini = {0};
	u32 key[IR_KEY_NUMS] = {0};

	node = ofnode_find_subnode(priv->ini_node, "ir");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("ofnode_path(ir) not found.\n");
		return -ENODEV;
	}
	if (ofnode_read_u32_array(node, "key", key, IR_KEY_NUMS) == 0) {
		for (int idx = 0; idx < IR_KEY_NUMS; idx++) {
			ini.key[idx] = key[idx];
			UBOOT_INFO("KEY[%02d]=0x%X\n", idx, ini.key[idx]);
		}
	}
	memcpy((void *)priv->buff_addr + PM_OFS_IR, (void *)&ini, sizeof(ini));
	return 0;
}

#else
#define IR_KEY_NUMS	(40*6)
#define IR_HEADCODE_NUMS	(2*5)

struct pm_ini_ir {
    u8 headcode[IR_HEADCODE_NUMS];
	u8 key[IR_KEY_NUMS];
};

static int pm_ini_ir_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_ir ini = {0};
	u32 key[IR_KEY_NUMS+IR_HEADCODE_NUMS] = {0};

	node = ofnode_find_subnode(priv->ini_node, "ir");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("ofnode_path(ir) not found.\n");
		return -ENODEV;
	}
	if (ofnode_read_u32_array(node, "key", key, (IR_KEY_NUMS+IR_HEADCODE_NUMS)) == 0) {
        for (int idx = 0; idx < IR_HEADCODE_NUMS; idx++) {
			ini.headcode[idx] = key[idx];
			UBOOT_INFO("HEADCODE[%02d]=0x%X\n", idx, ini.headcode[idx]);
		}
		for (int idx = 0; idx < IR_KEY_NUMS; idx++) {
			ini.key[idx] = key[IR_HEADCODE_NUMS+idx];
			UBOOT_INFO("KEY[%02d]=0x%X\n", idx, ini.key[idx]);
		}
	}
	memcpy((void *)priv->buff_addr + PM_OFS_IR_EXTEND, (void *)&ini, sizeof(ini));
	return 0;
}

#endif

/*****************************************************************************/
#define SAR_KEY_NUMS	(5)
#define ADC_OFFSET	(5)
typedef struct  __attribute__((packed))
{
	u8 wakeup_ch;
	u16 wakeadc[SAR_KEY_NUMS];
	u16 wakeup_lb;
	u8 adc_offset;
	u8 keypad_reset;
}st_pm_ini_sar;
static int pm_ini_sar_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
        st_pm_ini_sar ini = {0};
	const char *status;
	u32 adc[SAR_KEY_NUMS] = {0};
	u32 wakeup_ch = 0;
	u32 adc_offset = 0;
	u32 wakeup_lb = 0;
	u32 keypad_reset = 1;
#if !defined(CONFIG_MT58XX_SARADC)
	u32 upgrade_ch = 0;
	u32 upgrade_lb = 0;
#endif
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "sar");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[SAR] ofnode_path(SAR) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[SAR] skip sar\n");
		return 0;
	}
	ret = ofnode_read_u32(node, "wakeup-ch", &wakeup_ch);
	if (ret) {
		UBOOT_ERROR("[SAR] read (wakeup-ch) failed.(%d)\n", ret);
		return -EINVAL;
	}
	ret = ofnode_read_u32(node, "wakeup-lb", &wakeup_lb);
	if (ret) {
		UBOOT_ERROR("[SAR] read (wakeup-lb) failed.(%d)\n", ret);
		return -EINVAL;
	}
	ret = ofnode_read_u32(node, "adc_offset", &adc_offset);
	if (ret) {
		UBOOT_INFO("[SAR] read (adc_offset) failed.(%d) and use default\n", ret);
		adc_offset = ADC_OFFSET;
	}
	if (ofnode_read_u32_array(node, "wake-adc", adc, SAR_KEY_NUMS) == 0) {
		for (int idx = 0; idx < SAR_KEY_NUMS; idx++) {
                        ini.wakeadc[idx] = cpu_to_be16(adc[idx]);
                        UBOOT_INFO("wakeadc[%02d]=0x%X\n", idx, adc[idx]);
		}
	}
	ret = ofnode_read_u32(node, "keypad-reset", &keypad_reset);
	if (ret) {
		UBOOT_INFO("[SAR] read (keypad-reset) failed.(%d) and set default enable\n", ret);
		keypad_reset = 1;
	}
	/* handle endian for pm51 here */
	ini.wakeup_lb = cpu_to_be16(wakeup_lb);
	ini.wakeup_ch = wakeup_ch;
	ini.adc_offset = adc_offset;
	ini.keypad_reset = keypad_reset;

	memcpy((void *)priv->buff_addr + PM_OFS_SAR, (void *)&ini, sizeof(ini));

// Move keypad upgrade to uboot driver mt58xx-saradc
#if !defined(CONFIG_MT58XX_SARADC)
	/*upgrade key*/
	ret = ofnode_read_u32(node, "upgrade-ch", &upgrade_ch);
	if (ret) {
		UBOOT_ERROR("[SAR] read (upgrade-ch) failed.(%d)\n", ret);
		return -EINVAL;
	}
	ret = ofnode_read_u32(node, "upgrade-lb", &upgrade_lb);
	if (ret) {
		UBOOT_ERROR("[SAR] read (upgrade-lb) failed.(%d)\n", ret);
		return -EINVAL;
	}
	if (ofnode_read_u32_array(node, "upgrade-adc", adc, SAR_KEY_NUMS) == 0) {
		for (int idx = 0; idx < SAR_KEY_NUMS; idx++) {
                        ini.wakeadc[idx] = cpu_to_be16(adc[idx]);
                        UBOOT_INFO("upgrade adc[%02d]=0x%X \n", idx, adc[idx]);
		}
	}
    ini.wakeup_lb = cpu_to_be16(upgrade_lb);
	ini.wakeup_ch = upgrade_ch;

	memcpy((void *)priv->buff_addr + PM_OFS_SAR_UPGRADE, (void *)&ini, sizeof(ini));
#endif

	return 0;
}

/*****************************************************************************/
struct pm_ini_gpio {
	u16 wakeup_gpio;
	u16 wakeup_virq;
	u8 wakeup_virq_polar;
};

static int pm_ini_wifi_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_gpio ini = {0};
	const char *status;
	u32 get_val = 0;
	u16 wakeup_gpio = 0;
	u16 wakeup_virq = 0;
	u8 wakeup_virq_polar = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "wifi-gpio");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[WIFI] ofnode_path(wifi-gpio) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[WIFI] skip wifi-gpio\n");
		return 0;
	}

	ret = ofnode_read_u32(node, "wakeup-gpio-num", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI] read (wakeup-gpio-num) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_gpio = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI] read (wakeup-virq) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq-polar", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI] read (wakeup-virq-polar) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq_polar = (u8)get_val;

	UBOOT_INFO("[WIFI] from dts wakeup_gpio = 0x%x\n", wakeup_gpio);
	UBOOT_INFO("[WIFI] from dts wakeup_virq = 0x%x\n", wakeup_virq);
	UBOOT_INFO("[WIFI] from dts wakeup_virq_polar = 0x%x\n", wakeup_virq_polar);

	/* handle endian for pm51 here */
	ini.wakeup_gpio = cpu_to_be16(wakeup_gpio);
	ini.wakeup_virq = cpu_to_be16(wakeup_virq);
	ini.wakeup_virq_polar = wakeup_virq_polar;

	UBOOT_INFO("[WIFI] to pm51 wakeup_gpio = 0x%x\n", ini.wakeup_gpio);
	UBOOT_INFO("[WIFI] to pm51 wakeup_virq = 0x%x\n", ini.wakeup_virq);
	UBOOT_INFO("[WIFI] to pm51 wakeup_virq_polar = 0x%x\n", ini.wakeup_virq_polar);

	memcpy((void *)priv->buff_addr + PM_OFS_WIFI, (void *)&ini, sizeof(ini));

	return 0;
}

static int pm_ini_bt_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_gpio ini = {0};
	const char *status;
	u32 get_val = 0;
	u16 wakeup_gpio = 0;
	u16 wakeup_virq = 0;
	u8 wakeup_virq_polar = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "bt-gpio");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[BT] ofnode_path(bt-gpio) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[BT] skip bt-gpio\n");
		return 0;
	}

	ret = ofnode_read_u32(node, "wakeup-gpio-num", &get_val);
	if (ret) {
		UBOOT_ERROR("[BT] read (wakeup-gpio-num) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_gpio = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq", &get_val);
	if (ret) {
		UBOOT_ERROR("[BT] read (wakeup-virq) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq-polar", &get_val);
	if (ret) {
		UBOOT_ERROR("[BT] read (wakeup-virq-polar) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq_polar = (u8)get_val;

	UBOOT_INFO("[BT] from dts wakeup_gpio = 0x%x\n", wakeup_gpio);
	UBOOT_INFO("[BT] from dts wakeup_virq = 0x%x\n", wakeup_virq);
	UBOOT_INFO("[BT] from dts wakeup_virq_polar = 0x%x\n", wakeup_virq_polar);

	/* handle endian for pm51 here */
	ini.wakeup_gpio = cpu_to_be16(wakeup_gpio);
	ini.wakeup_virq = cpu_to_be16(wakeup_virq);
	ini.wakeup_virq_polar = wakeup_virq_polar;

	UBOOT_INFO("[BT] to pm51 to pm51 wakeup_gpio = 0x%x\n", ini.wakeup_gpio);
	UBOOT_INFO("[BT] to pm51 wakeup_virq = 0x%x\n", ini.wakeup_virq);
	UBOOT_INFO("[BT] to pm51 wakeup_virq_polar = 0x%x\n", ini.wakeup_virq_polar);

	memcpy((void *)priv->buff_addr + PM_OFS_BT, (void *)&ini, sizeof(ini));

	return 0;
}

static int pm_ini_emer_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_gpio ini = {0};
	const char *status;
	u32 get_val = 0;
	u16 wakeup_gpio = 0;
	u16 wakeup_virq = 0;
	u8 wakeup_virq_polar = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "emer-gpio");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[EMER] ofnode_path(emer-gpio) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[EMER] skip emer-gpio\n");
		return 0;
	}

	ret = ofnode_read_u32(node, "wakeup-gpio-num", &get_val);
	if (ret) {
		UBOOT_ERROR("[EMER] read (wakeup-gpio-num) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_gpio = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq", &get_val);
	if (ret) {
		UBOOT_ERROR("[EMER] read (wakeup-virq) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "wakeup-virq-polar", &get_val);
	if (ret) {
		UBOOT_ERROR("[EMER] read (wakeup-virq-polar) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_virq_polar = (u8)get_val;

	UBOOT_INFO("[EMER] from dts wakeup_gpio = 0x%x\n", wakeup_gpio);
	UBOOT_INFO("[EMER] from dts wakeup_virq = 0x%x\n", wakeup_virq);
	UBOOT_INFO("[EMER] from dts wakeup_virq_polar = 0x%x\n", wakeup_virq_polar);

	/* handle endian for pm51 here */
	ini.wakeup_gpio = cpu_to_be16(wakeup_gpio);
	ini.wakeup_virq = cpu_to_be16(wakeup_virq);
	ini.wakeup_virq_polar = wakeup_virq_polar;

	UBOOT_INFO("[EMER] to pm51 wakeup_gpio = 0x%x\n", ini.wakeup_gpio);
	UBOOT_INFO("[EMER] to pm51 wakeup_virq = 0x%x\n", ini.wakeup_virq);
	UBOOT_INFO("[EMER] to pm51 wakeup_virq_polar = 0x%x\n", ini.wakeup_virq_polar);

	memcpy((void *)priv->buff_addr + PM_OFS_EMER, (void *)&ini, sizeof(ini));

	return 0;
}
struct pm_ini_mute_gpio {
	u16 wakeup_gpio;
};

static int pm_ini_mic_mute_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_mute_gpio ini = {0};
	u32 get_val = 0;
	u16 wakeup_gpio = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "mute-gpio");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[mute_gpio] ofnode_path(mute_gpio) not found.\n");
		return -ENODEV;
	}

	ret = ofnode_read_u32(node, "wakeup-gpio-num", &get_val);
	if (ret) {
		UBOOT_ERROR("[mute_gpio] read (wakeup-gpio-num) failed.(%d)\n", ret);
		return -EINVAL;
	}
	wakeup_gpio = (u16)get_val;

	UBOOT_INFO("[mute_gpio] from dts wakeup_gpio = 0x%x\n", wakeup_gpio);


	/* handle endian for pm51 here */
	ini.wakeup_gpio = cpu_to_be16(wakeup_gpio);


	UBOOT_INFO("[mute_gpio] to pm51 wakeup_gpio = 0x%x\n", ini.wakeup_gpio);


	memcpy((void *)priv->buff_addr + PM_OFS_MIC_MUTE, (void *)&ini, sizeof(ini));

	return 0;
}
/*****************************************************************************/
struct pm_ini_wifi_reset {
	u16 reset_gpio;
	u8 default_gpio;
	u8 invert_ms;
};

static int pm_ini_wifi_reset_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_wifi_reset ini = {0};
	const char *status;
	u32 get_val = 0;
	u16 reset_gpio = 0;
	u8 default_gpio = 0;
	u8 invert_ms = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "wifi-reset");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[WIFI RESET] ofnode_path(wifi-reset) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[WIFI RESET] skip wifi-reset\n");
		return 0;
	}

	ret = ofnode_read_u32(node, "reset-gpio-num", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI RESET] read (reset-gpio-num) failed.(%d)\n", ret);
		return -EINVAL;
	}
	reset_gpio = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "default-gpio-val", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI RESET] read (default-gpio-val) failed.(%d)\n", ret);
		return -EINVAL;
	}
	default_gpio = (u8)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "invert-ms", &get_val);
	if (ret) {
		UBOOT_ERROR("[WIFI RESET] read (invert-ms) failed.(%d)\n", ret);
		return -EINVAL;
	}
	invert_ms = (u8)get_val;

	UBOOT_INFO("[WIFI RESET] from dts reset_gpio = 0x%x\n", reset_gpio);
	UBOOT_INFO("[WIFI RESET] from dts default_gpio = 0x%x\n", default_gpio);
	UBOOT_INFO("[WIFI RESET] from dts invert_ms = 0x%x\n", invert_ms);

	/* handle endian for pm51 here */
	ini.reset_gpio = cpu_to_be16(reset_gpio);
	ini.default_gpio = default_gpio;
	ini.invert_ms = invert_ms;

	UBOOT_INFO("[WIFI RESET] to pm51 to pm51 reset_gpio = 0x%x\n", ini.reset_gpio);
	UBOOT_INFO("[WIFI RESET] to pm51 default_gpio = 0x%x\n", ini.default_gpio);
	UBOOT_INFO("[WIFI RESET] to pm51 invert_ms = 0x%x\n", ini.invert_ms);

	memcpy((void *)priv->buff_addr + PM_OFS_WIFI_RESET, (void *)&ini, sizeof(ini));

	return 0;
}

/*****************************************************************************/
struct pm_ini_gpio_control {
	u16 nonpm_gpio_nbase;
	u16 nonpm_gpio_ngpio;
	u16 pm_gpio_nbase;
	u16 pm_gpio_ngpio;
};

static int pm_ini_gpio_control_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_gpio_control ini = {0};
	const char *status;
	u32 get_val = 0;
	u16 nonpm_gpio_nbase = 0;
	u16 nonpm_gpio_ngpio = 0;
	u16 pm_gpio_nbase = 0;
	u16 pm_gpio_ngpio = 0;
	int ret = 0;

	node = ofnode_find_subnode(priv->ini_node, "gpio-control");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[GPIO CONTROL] ofnode_path(gpio-control) not found.\n");
		return -ENODEV;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[GPIO CONTROL] skip gpio-control\n");
		return 0;
	}

	ret = ofnode_read_u32(node, "nonpm-gpio-nbase", &get_val);
	if (ret) {
		UBOOT_ERROR("[GPIO CONTROL] read (nonpm-gpio-nbase) failed.(%d)\n", ret);
		return -EINVAL;
	}
	nonpm_gpio_nbase = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "nonpm-gpio-ngpio", &get_val);
	if (ret) {
		UBOOT_ERROR("[GPIO CONTROL] read (nonpm-gpio-ngpio) failed.(%d)\n", ret);
		return -EINVAL;
	}
	nonpm_gpio_ngpio = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "pm-gpio-nbase", &get_val);
	if (ret) {
		UBOOT_ERROR("[GPIO CONTROL] read (pm-gpio-nbase) failed.(%d)\n", ret);
		return -EINVAL;
	}
	pm_gpio_nbase = (u16)get_val;

	get_val = 0;
	ret = ofnode_read_u32(node, "pm-gpio-ngpio", &get_val);
	if (ret) {
		UBOOT_ERROR("[GPIO CONTROL] read (pm-gpio-ngpio) failed.(%d)\n", ret);
		return -EINVAL;
	}
	pm_gpio_ngpio = (u16)get_val;

	UBOOT_INFO("[GPIO CONTROL] from dts nonpm_gpio_nbase = 0x%x\n", nonpm_gpio_nbase);
	UBOOT_INFO("[GPIO CONTROL] from dts nonpm_gpio_ngpio = 0x%x\n", nonpm_gpio_ngpio);
	UBOOT_INFO("[GPIO CONTROL] from dts pm_gpio_nbase = 0x%x\n", pm_gpio_nbase);
	UBOOT_INFO("[GPIO CONTROL] from dts pm_gpio_ngpio = 0x%x\n", pm_gpio_ngpio);

	/* handle endian for pm51 here */
	ini.nonpm_gpio_nbase = cpu_to_be16(nonpm_gpio_nbase);
	ini.nonpm_gpio_ngpio = cpu_to_be16(nonpm_gpio_ngpio);
	ini.pm_gpio_nbase = cpu_to_be16(pm_gpio_nbase);
	ini.pm_gpio_ngpio = cpu_to_be16(pm_gpio_ngpio);

	UBOOT_INFO("[GPIO CONTROL] to pm51 nonpm_gpio_nbase = 0x%x\n", ini.nonpm_gpio_nbase);
	UBOOT_INFO("[GPIO CONTROL] to pm51 nonpm_gpio_ngpio = 0x%x\n", ini.nonpm_gpio_ngpio);
	UBOOT_INFO("[GPIO CONTROL] to pm51 pm_gpio_nbase = 0x%x\n", ini.pm_gpio_nbase);
	UBOOT_INFO("[GPIO CONTROL] to pm51 pm_gpio_ngpio = 0x%x\n", ini.pm_gpio_ngpio);

	memcpy((void *)priv->buff_addr + PM_OFS_GPIO_CONTROL, (void *)&ini, sizeof(ini));

	return 0;
}
/*****************************************************************************/
#define EXT_GPIO_LENGTH		4
#define EXT_GPIO_NUM		"ext_gpio_num"
#define EXT_GPIO_POLARITY	"ext_gpio_polarity"
struct pm_ini_ext_gpio {
	u16 ext_gpio[EXT_GPIO_LENGTH];
	u16 ext_gpio_polarity[EXT_GPIO_LENGTH];
};

static int pm_ini_ext_gpio_probe(struct pm_ini_device *priv)
{
	ofnode node = {0};
	struct pm_ini_ext_gpio ini;
	u32 get_val = 0;
	u16 ext_gpio = 0;
	u16 ext_gpio_polarity = 0;
	int ret = 0;
	const char *status;
	int i = 0;
	memset(&ini, 0xFF, sizeof(struct pm_ini_ext_gpio));

	node = ofnode_find_subnode(priv->ini_node, "pm-ext-gpio");
	if (!ofnode_valid(node)) {
		UBOOT_ERROR("[EXT_GPIO] ofnode_path(ps_gpio) not found.\n");
		goto exit_ext_gpio;
	}

	status = ofnode_read_string(node, "status");
	/* only skip if status existed with disable string */
	if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0)) {
		UBOOT_INFO("[EXT_GPIO] skip ext-gpio\n");
		goto exit_ext_gpio;
	}

	for (i=0;i<EXT_GPIO_LENGTH;i++)
	{

		char ext_gpio_node[64];
		ret = snprintf(ext_gpio_node, sizeof(ext_gpio_node), "%s%d", EXT_GPIO_NUM, i);
		if (ret > 0) {
			ret = ofnode_read_u32(node, ext_gpio_node, &get_val);
			if (ret) {
				UBOOT_DEBUG("[EXT_GPIO] read (ext-gpio-num) failed.(%d)\n", ret);
				goto exit_ext_gpio;
			}
			ext_gpio = (u16)get_val;
			UBOOT_INFO("[EXT_GPIO] from dts ext_gpio = %d\n", ext_gpio);
			/* handle endian for pm51 here */
			ini.ext_gpio[i] = cpu_to_be16(ext_gpio);
			UBOOT_INFO("[EXT_GPIO] to pm51 ext_gpio = %d\n", ini.ext_gpio[i]);
			ret = snprintf(ext_gpio_node, sizeof(ext_gpio_node), "%s%d", EXT_GPIO_POLARITY, i);
			if (ret > 0) {
				ret = ofnode_read_u32(node, ext_gpio_node, &get_val);
				if (ret) {
					UBOOT_ERROR("[EXT_GPIO] read (ext-gpio-polarity) failed.(%d)\n", ret);
					goto exit_ext_gpio;
				}
				ext_gpio_polarity = (u16)get_val;
				UBOOT_INFO("[EXT_GPIO] from dts ext_gpio_polarity = %d\n", ext_gpio_polarity);
				/* handle endian for pm51 here */
				ini.ext_gpio_polarity[i] = cpu_to_be16(ext_gpio_polarity);
				UBOOT_INFO("[EXT_GPIO] to pm51 ext_gpio_polarity = %d\n", ini.ext_gpio_polarity[i]);
			}
		}
	}


exit_ext_gpio:

	memcpy((void *)priv->buff_addr + PM_OFS_EXT_GPIO, (void *)&ini, sizeof(ini));

	return 0;
}


/*****************************************************************************/
struct pm_ini_hdmi33v_control
{
    u16 control_gpio_num;
};

static int pm_ini_hdmi33v_control_probe(struct pm_ini_device *priv)
{
    ofnode node = {0};
    struct pm_ini_hdmi33v_control ini = {0};
    u32 get_val = 0;
    const char *status = NULL;
    u16 control_gpio = 0;
    int ret = 0;

    node = ofnode_find_subnode(priv->ini_node, "hdmi-33v-ctrl");
    if (!ofnode_valid(node))
    {
        UBOOT_ERROR("[HDMI 3.3V control] ofnode_path(hdmi-33v-ctrl) not found.\n");
        return -ENODEV;
    }

    status = ofnode_read_string(node, "status");
    /* only skip if status existed with disable string */
    if ((status) && (strncmp(status, "disabled", DTS_STAT_DISABLE_NUM) == 0))
    {
        UBOOT_INFO("[HDMI 3.3V control] hdmi 3.3v control is disabled\n");
        return 0;
    }

    ret = ofnode_read_u32(node, "hdmi-33v-gpio-num", &get_val);
    if (ret)
    {
        UBOOT_ERROR(
        "[HDMI 3.3V control] read (hdmi-33v-gpio-num) failed.(%d)\n", ret);
        return -EINVAL;
    }
    control_gpio = (u16)get_val;

    UBOOT_INFO("[HDMI 3.3V control] from dts control_gpio = 0x%x status = '%s'\n", control_gpio, status);

    /* handle endian for pm51 here */
    ini.control_gpio_num = cpu_to_be16(control_gpio);

    UBOOT_INFO("[HDMI 3.3V control] to pm51 control_gpio = 0x%x, size = %d\n",
                ini.control_gpio_num, (int)sizeof(ini));

    memcpy((void *)priv->buff_addr + PM_OFS_HDMI33V_CONTROL, (void *)&ini, sizeof(ini));

    return 0;
}

/*****************************************************************************/
int pmu_ini_loader(void)
{
	int ret = 0;
	bool disable = 0;
	struct pm_ini_device dev = {0};

	UBOOT_TRACE("load ini via dts.\n");

	if ((ret = pm_ini_probe(&dev, &disable))) {
		UBOOT_ERROR("pmu_ini_loader() probe fail=%d.\n", ret);
		if (dev.ptr)
			free((void *)dev.ptr);
		return ret;
	}

	if (disable) {
		UBOOT_INFO("%s() is disable.\n", __func__);
		if (dev.ptr)
			free((void *)dev.ptr);
		return 0;
	}

	if ((ret = pm_ini_ir_probe(&dev)) ||
		(ret = pm_ini_sar_probe(&dev)) ||
		(ret = pm_ini_wifi_probe(&dev)) ||
		(ret = pm_ini_bt_probe(&dev)) ||
		(ret = pm_ini_emer_probe(&dev)) ||
		(ret = pm_ini_ext_gpio_probe(&dev)) ||
		(ret = pm_ini_mic_mute_probe(&dev)) ||
		(ret = pm_ini_wifi_reset_probe(&dev)) ||
        (ret = pm_ini_gpio_control_probe(&dev)) ||
        (ret = pm_ini_hdmi33v_control_probe(&dev)))
    {
		UBOOT_ERROR("pmu_ini_loader() parser fail=%d.\n", ret);
		if (dev.ptr)
			free((void *)dev.ptr);
		return ret;
	}

	flush_cache((ulong)dev.buff_addr, dev.buff_size);
	return 0;
}
