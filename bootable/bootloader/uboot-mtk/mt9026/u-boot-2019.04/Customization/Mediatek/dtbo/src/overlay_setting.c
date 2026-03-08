// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <dm/uclass.h>
#include <dm/uclass-id.h>
#include <dm/pinctrl.h>
#include <dtbo.h>
#include <mtk_dtbo.h>
#include <debug_impl.h>
#include <malloc.h>
#include <fdt_support.h>
#include <iniutility.h>
#include <utility.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#define WDT_SECTION		"WDT"
#define WDT_KEY			"WDT_INI_FILE"
#define PANEL_MIRROR_SECTION			"MISC_MIRROR_CFG"
#define PANEL_MIRROR_KEY			"m_pIniMirrorCfgFile"
#define PANEL_SSC_SECTION			"MISC_SSC_CFG"
#define PANEL_SSC_KEY			"m_pSSC_File"
#define CONSOLE_SECTION		"CONSOLE"
#define CONSOLE_KEY		"CONSOLE_INI_FILE"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define DEFAULT_WDT_PART	"tvconfig"
#define DEFAULT_WDT_INI		"/config/wdt.ini"

#define DEFAULT_PANEL_MIRROR_PART	"tvconfig"
#define DEFAULT_PANEL_MIRROR_INI	"/config/panel/mirror_cfg.ini"

#define DEFAULT_PANEL_SSC_PART	"tvconfig"
#define DEFAULT_PANEL_SSC_INI	"/config/panel/ssc_cfg.ini"

#define DEFAULT_CONSOLE_PART	"tvconfig"
#define DEFAULT_CONSOLE_INI	"/config/console.ini"

#define WDT_NODE_PATH		"/wdt0"
#define NEW_PROP_NAME		"status"
#define NEW_PROP_VAL		"disable"

static struct wdt_info wdt_sts;
static struct panel_mirror_info mirror_info;
static struct console_info con_info;

static int fdt_appendprop_from_path(void *fdt, const char *path,
			const char *name, const void *val, int len)
{
	int nodeoffset;
	int ret;

	nodeoffset = fdt_path_offset(fdt, path);
	if (nodeoffset < 0) {
		UBOOT_DEBUG("fdt_path_offset Fail\n");
		return -1;
	}
	ret = fdt_appendprop(fdt, nodeoffset, name, val, len);
	if (ret < 0) {
		UBOOT_DEBUG("WDT OVERLAY Fail ret:%d\n",ret);
		return -1;
	}
	return 0;
}

static int overlay_watchdog_settings(void)
{
	int ret;
	void* dtb_addr = (void*)get_dtb_image_address();
#ifdef CONFIG_DATA_SEPARATION
	char part[PART_NAME_SIZE], filepath[FILE_PATH_SIZE];
	const char *relpath;
#endif
    static int watchdog_read_inited = 0;

    if (watchdog_read_inited == 1)
    {
        goto watchdog_setting;
    }

	memset(&wdt_sts, 0, sizeof(wdt_sts));
#ifdef CONFIG_DATA_SEPARATION
	memset(part, 0, sizeof(part));
	memset(filepath, 0, sizeof(filepath));
	if(dataindex_get_key(filepath, FILE_PATH_SIZE, WDT_SECTION, WDT_KEY, NULL) == 0){
		if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0){
			ret = get_wdt_info(part, relpath, &wdt_sts);
			if(ret < 0){
				UBOOT_DEBUG("Read wdt information failure with data separation.\n");
				/*read from watchdog.ini*/
				ret = get_wdt_info(DEFAULT_WDT_PART, DEFAULT_WDT_INI, &wdt_sts);
			}
		}else{
			UBOOT_DEBUG("resolve path fail: %s",filepath);
			/*read from watchdog.ini*/
			ret = get_wdt_info(DEFAULT_WDT_PART, DEFAULT_WDT_INI, &wdt_sts);
		}
	}else{
		UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",WDT_SECTION,WDT_KEY);
#endif
		/*read from watchdog.ini*/
		ret = get_wdt_info(DEFAULT_WDT_PART, DEFAULT_WDT_INI, &wdt_sts);
#ifdef CONFIG_DATA_SEPARATION
	}
#endif

	if (ret < 0) {
		UBOOT_DEBUG("cannot wdt read /config/wdt.ini from tvconfig\n");
	}
    watchdog_read_inited = 1;

watchdog_setting:
	UBOOT_DEBUG("wdt_sts.status:%s\n",wdt_sts.status);

	/*overlay watchdog0 device node*/
	ret = fdt_appendprop_from_path((void *)dtb_addr, WDT_NODE_PATH,
			NEW_PROP_NAME, wdt_sts.status, strlen(wdt_sts.status)+1);
	if (ret < 0) {
		UBOOT_DEBUG("ini overlay Fail ret:%d\n",ret);
	}
	return ret;
}

static int overlay_panel_ssc_settings(void)
{
	void* dtb_addr = (void*)get_dtb_image_address();
	char *dtb_path = "/video_out/spread_spectrum_info";
	char *dtb_nameEn = "ssc_ctrl_en";
	char *dtb_nameMo = "ssc_modulation";
	char *dtb_nameDe = "ssc_deviation";
	struct panel_ssc_info ssc_info;
	const struct fdt_property *fdt_prop;
	int nodeoffset;
	u32 value_old, value_new;
	int ret;
#ifdef CONFIG_DATA_SEPARATION
	char part[PART_NAME_SIZE], filepath[FILE_PATH_SIZE];
	const char *relpath;
#endif

	memset(&ssc_info, 0, sizeof(ssc_info));
#ifdef CONFIG_DATA_SEPARATION
	memset(part, 0, sizeof(part));
	memset(filepath, 0, sizeof(filepath));
	if(dataindex_get_key(filepath, FILE_PATH_SIZE, PANEL_SSC_SECTION, PANEL_SSC_KEY, NULL) == 0){
		if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0){
			// read from ssc_cfg.ini
			ret = get_panel_ssc_info(part, relpath, &ssc_info);
			if(ret < 0){
				UBOOT_DEBUG("Read panel ssc information failure with data separation.\n");
				// read from ssc_cfg.ini
				ret = get_panel_ssc_info(DEFAULT_PANEL_SSC_PART, DEFAULT_PANEL_SSC_INI, &ssc_info);
			}
		}else{
			UBOOT_DEBUG("resolve path fail: %s",filepath);
			// read from ssc_cfg.ini
			ret = get_panel_ssc_info(DEFAULT_PANEL_SSC_PART, DEFAULT_PANEL_SSC_INI, &ssc_info);
		}
	}else{
		UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",PANEL_SSC_SECTION,PANEL_SSC_KEY);
#endif
		ret = get_panel_ssc_info(DEFAULT_PANEL_SSC_PART, DEFAULT_PANEL_SSC_INI, &ssc_info);
#ifdef CONFIG_DATA_SEPARATION
	}
#endif

	if (ret < 0) {
		UBOOT_ERROR("Cannot read %s:%s\n", DEFAULT_PANEL_SSC_PART, DEFAULT_PANEL_SSC_INI);
		return -1;
	}
	UBOOT_DEBUG("ssc_info.ssc_enable:%d\n", ssc_info.ssc_enable);
	UBOOT_DEBUG("ssc_info.ssc_modulation:%d\n", ssc_info.ssc_modulation);
	UBOOT_DEBUG("ssc_info.ssc_deviation:%d\n", ssc_info.ssc_deviation);

	if (ssc_info.ssc_overwrite_enable == false){
		UBOOT_DEBUG("Disable Overlay\n");
		return 0;
	}
	// Update dts if the setting is valid
	// ssc_enable
	nodeoffset = fdt_path_offset(dtb_addr, dtb_path);
	if (nodeoffset < 0) {
		UBOOT_ERROR("Cannot get offset of '%s'\n", dtb_path);
		return -1;
	}
	fdt_prop = fdt_get_property(dtb_addr, nodeoffset, dtb_nameEn, &ret);
	if (fdt_prop == NULL || ret < sizeof(int)) { // @ret is data size
		UBOOT_ERROR("Cannot get prop '%s'\n", dtb_nameEn);
		return -1;
	}
	value_old = fdt32_to_cpu(*((__be32 *)fdt_prop->data));
	value_new = ssc_info.ssc_enable;
	ret = fdt_setprop_u32(dtb_addr, nodeoffset, dtb_nameEn, value_new);
	if (ret != 0) {
		UBOOT_ERROR("Cannot set prop '%s'\n", dtb_nameEn);
		return -1;
	}
	UBOOT_DEBUG("Set [%s:%s] = %d -> %d (ini: %d)\n",
		dtb_path, dtb_nameEn, value_old, value_new, ssc_info.ssc_enable);

	// ssc_modulation
	nodeoffset = fdt_path_offset(dtb_addr, dtb_path);
	if (nodeoffset < 0) {
		UBOOT_ERROR("Cannot get offset of '%s'\n", dtb_path);
		return -1;
	}
	fdt_prop = fdt_get_property(dtb_addr, nodeoffset, dtb_nameMo, &ret);
	if (fdt_prop == NULL || ret < sizeof(int)) { // @ret is data size
		UBOOT_ERROR("Cannot get prop '%s'\n", dtb_nameMo);
		return -1;
	}
	value_old = fdt32_to_cpu(*((__be32 *)fdt_prop->data));
	value_new = ssc_info.ssc_modulation;
	ret = fdt_setprop_u32(dtb_addr, nodeoffset, dtb_nameMo, value_new);
	if (ret != 0) {
		UBOOT_ERROR("Cannot set prop '%s'\n", dtb_nameMo);
		return -1;
	}
	UBOOT_DEBUG("Set [%s:%s] = %d -> %d (ini: %d)\n",
		dtb_path, dtb_nameMo, value_old, value_new, ssc_info.ssc_modulation);

	// ssc_deviation
	nodeoffset = fdt_path_offset(dtb_addr, dtb_path);
	if (nodeoffset < 0) {
		UBOOT_ERROR("Cannot get offset of '%s'\n", dtb_path);
		return -1;
	}
	fdt_prop = fdt_get_property(dtb_addr, nodeoffset, dtb_nameDe, &ret);
	if (fdt_prop == NULL || ret < sizeof(int)) { // @ret is data size
		UBOOT_ERROR("Cannot get prop '%s'\n", dtb_nameDe);
		return -1;
	}
	value_old = fdt32_to_cpu(*((__be32 *)fdt_prop->data));
	value_new = ssc_info.ssc_deviation;
	ret = fdt_setprop_u32(dtb_addr, nodeoffset, dtb_nameDe, value_new);
	if (ret != 0) {
		UBOOT_ERROR("Cannot set prop '%s'\n", dtb_nameDe);
		return -1;
	}
	UBOOT_DEBUG("Set [%s:%s] = %d -> %d (ini: %d)\n",
		dtb_path, dtb_nameDe, value_old, value_new, ssc_info.ssc_deviation);

	return 0;
}


#define PANEL_MIRROR_VALUE_MASK 0x3
static int overlay_panel_mirror_settings(void)
{
	void* dtb_addr = (void*)get_dtb_image_address();
	char *dtb_path = "/video_out/panel-cus-setting";
	char *dtb_name = "PanelMirrorMode";
	const struct fdt_property *fdt_prop;
	int nodeoffset;
	u32 value_old, value_new;
	int ret;
#ifdef CONFIG_DATA_SEPARATION
	char part[PART_NAME_SIZE], filepath[FILE_PATH_SIZE];
	const char *relpath;
#endif
    static int mirror_read_inited = 0;

    if (mirror_read_inited == 1)
    {
        goto mirror_setting;
    }

	memset(&mirror_info, 0, sizeof(mirror_info));
#ifdef CONFIG_DATA_SEPARATION
	memset(part, 0, sizeof(part));
	memset(filepath, 0, sizeof(filepath));
	if(dataindex_get_key(filepath, FILE_PATH_SIZE, PANEL_MIRROR_SECTION, PANEL_MIRROR_KEY, NULL) == 0){
		if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0){
			// read from mirror_cfg.ini
			ret = get_panel_mirror_info(part, relpath, &mirror_info);
			if(ret < 0){
				UBOOT_DEBUG("Read panel mirror information failure with data separation.\n");
				// read from mirror_cfg.ini
				ret = get_panel_mirror_info(DEFAULT_PANEL_MIRROR_PART, DEFAULT_PANEL_MIRROR_INI, &mirror_info);
			}
		}else{
			UBOOT_DEBUG("resolve path fail: %s",filepath);
			// read from mirror_cfg.ini
			ret = get_panel_mirror_info(DEFAULT_PANEL_MIRROR_PART, DEFAULT_PANEL_MIRROR_INI, &mirror_info);
		}
	}else{
		UBOOT_DEBUG("cannot get %s:%s from dataindex file\n",PANEL_MIRROR_SECTION,PANEL_MIRROR_KEY);
#endif
		ret = get_panel_mirror_info(DEFAULT_PANEL_MIRROR_PART, DEFAULT_PANEL_MIRROR_INI, &mirror_info);
#ifdef CONFIG_DATA_SEPARATION
	}
#endif

	if (ret < 0) {
		UBOOT_ERROR("Cannot read %s:%s\n", DEFAULT_PANEL_MIRROR_PART, DEFAULT_PANEL_MIRROR_INI);
		return -1;
	}
    mirror_read_inited = 1;

mirror_setting:
	UBOOT_DEBUG("mirror_info.valid:%d\n", mirror_info.valid);
	UBOOT_DEBUG("mirror_info.mirror_mode:%d\n", mirror_info.mirror_mode);

	// Update dts if the setting is valid
	if (mirror_info.valid == true) {
		nodeoffset = fdt_path_offset(dtb_addr, dtb_path);
		if (nodeoffset < 0) {
			UBOOT_ERROR("Cannot get offset of '%s'\n", dtb_path);
			return -1;
		}
		fdt_prop = fdt_get_property(dtb_addr, nodeoffset, dtb_name, &ret);
		if (fdt_prop == NULL || ret < sizeof(int)) { // @ret is data size
			UBOOT_ERROR("Cannot get prop '%s'\n", dtb_name);
			return -1;
		}
		value_old = fdt32_to_cpu(*((__be32 *)fdt_prop->data));
		value_new = (value_old ^ mirror_info.mirror_mode) & PANEL_MIRROR_VALUE_MASK;
		ret = fdt_setprop_u32(dtb_addr, nodeoffset, dtb_name, value_new);
		if (ret != 0) {
			UBOOT_ERROR("Cannot set prop '%s'\n", dtb_name);
			return -1;
		}
		UBOOT_DEBUG("Set [%s:%s] = %d -> %d (ini: %d)\n",
			dtb_path, dtb_name, value_old, value_new, mirror_info.mirror_mode);
	}
	return 0;
}

static int overlay_console_switch_ini(void)
{
	int ret = 0;
	struct udevice *dev = NULL;
#ifdef CONFIG_DATA_SEPARATION
	char part[PART_NAME_SIZE], filepath[FILE_PATH_SIZE];
	const char *relpath;
#endif
    static int console_read_inited = 0;

    if (console_read_inited == 1)
    {
        goto console_setting;
    }

	// parse ini for console_onoff and console_hdmi
	memset(&con_info, 0, sizeof(con_info));

#ifdef CONFIG_DATA_SEPARATION
	memset(part, 0, sizeof(part));
	memset(filepath, 0, sizeof(filepath));
	if (dataindex_get_key(filepath, FILE_PATH_SIZE, CONSOLE_SECTION, CONSOLE_KEY, NULL) == 0) {
		if (dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0) {
			ret = get_console_info(part, relpath, &con_info);
			if (ret < 0) {
				UBOOT_DEBUG("Read console information failure with data separation.\n");
				/*read from console.ini*/
				ret = get_console_info(DEFAULT_CONSOLE_PART, DEFAULT_CONSOLE_INI,
						       &con_info);
			}
		} else {
			UBOOT_DEBUG("resolve path fail: %s",filepath);
			/*read from console.ini*/
			ret = get_console_info(DEFAULT_CONSOLE_PART, DEFAULT_CONSOLE_INI,
					       &con_info);
		}
	} else {
		UBOOT_DEBUG("cannot get %s:%s from dataindex file\n", CONSOLE_SECTION, CONSOLE_KEY);
#endif
		/*read from console.ini*/
		ret = get_console_info(DEFAULT_CONSOLE_PART, DEFAULT_CONSOLE_INI, &con_info);
#ifdef CONFIG_DATA_SEPARATION
	}
#endif

	if (ret < 0) {
		UBOOT_DEBUG("cannot read "DEFAULT_CONSOLE_INI" from "DEFAULT_CONSOLE_PART"\n");
		return 0;
	}
    console_read_inited = 1;

console_setting:
	UBOOT_DEBUG("console_onoff = %s\n", con_info.console_onoff);
	UBOOT_DEBUG("console_hdmi = %s\n", con_info.console_hdmi);

	if (!strcmp(con_info.console_onoff, "on")) {
		UBOOT_DEBUG("switch console on in console.ini, delete bootarg.\n");
		gd->flags &= ~GD_FLG_SILENT;
		// module parameter when UART driver is kernel module
		del_bootargs("8250_mtk_tv.log_disable", 0);
		// module parameter when UART driver is build-in
		del_bootargs("log_disable", 0);
	} else {
		UBOOT_DEBUG("switch console off in console.ini, add bootarg.\n");
		gd->flags |= GD_FLG_SILENT;
		// module parameter when UART driver is build-in
		add_bootargs("log_disable", "log_disable=1", 0);
		// module parameter when UART driver is kernel module
		add_bootargs("8250_mtk_tv.log_disable", "8250_mtk_tv.log_disable=1", 0);
	}

	if (!(gd->flags & GD_FLG_SILENT)) {
		volatile unsigned short *u16Ch;

		if (strcmp(con_info.console_hdmi, "on")) {
			// default not switch to hdmi
			UBOOT_DEBUG("console_switch finished\n");
			return 0;
		}

		UBOOT_DEBUG("switch hdmi console on\n");

		// any failure will abort switch hdmi console
		ret = uclass_get_device(UCLASS_PINCTRL, 1, &dev);
		if (ret) {
			UBOOT_ERROR("uclass_get_device failed: %d\n", ret);
			return 0;
		}

		if (!dev) {
			UBOOT_ERROR("get serial device failed\n");
			return 0;
		}

		ret = pinctrl_select_state(dev, "hdmib_pm_uart0");
		if (ret) {
			UBOOT_ERROR("pinctrl select hdmib_pm_uart0 failed: %d\n", ret);
			return 0;
		}

		// disable default console port(debug port) when console switch to hdmi
#define REG_RIU_BASE			0x1C000000
#define REG_PM_BANK			0x10100
#define REG_HK51_UART0_EN		(0x09)
#define REG_HK51_UART0_ENABLE		(0x1000)
#define REG_UART_CM4_EN			(0x7E)
#define REG_UART_CM4_ENABLE		(0x0004)
		UBOOT_INFO("Disable original console when console switch to hdmi\n");

		u16Ch = (unsigned short *)(REG_RIU_BASE +
					   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
		if ((*u16Ch & REG_UART_CM4_ENABLE)) {
			// sboot console is PM_UART0
			UBOOT_INFO("Switch off console by reg bank 0x%X off 0x%X mask 0x%X\n",
				   REG_PM_BANK, REG_UART_CM4_EN, REG_UART_CM4_ENABLE);
			u16Ch = (unsigned short *)(REG_RIU_BASE +
						   ((REG_PM_BANK + (REG_UART_CM4_EN << 1)) << 1));
			*u16Ch &= ~REG_UART_CM4_ENABLE;
		} else {
			// sboot console is FUART0
			UBOOT_INFO("Switch off console by reg bank 0x%X off 0x%X mask 0x%X\n",
				   REG_PM_BANK, REG_HK51_UART0_EN, REG_HK51_UART0_ENABLE);
			u16Ch = (unsigned short *)(REG_RIU_BASE +
						   ((REG_PM_BANK + (REG_HK51_UART0_EN << 1)) << 1));
			*u16Ch |= REG_HK51_UART0_ENABLE;
		}
	}

	UBOOT_DEBUG("console_switch finished\n");
	return 0;
}

int overlay_ini_settings(void)
{
	int ret;

	ret = overlay_console_switch_ini();
	if (ret < 0) {
		UBOOT_ERROR("console switch ini failure with %d\n", ret);
	}

	ret = overlay_watchdog_settings();
	if (ret < 0) {
		UBOOT_ERROR("watchdog dtbo failure with %d\n", ret);
	}

	ret = overlay_panel_mirror_settings();
	if (ret < 0) {
		UBOOT_ERROR("panel mirror dtbo failure with %d\n", ret);
	}

	ret = overlay_panel_ssc_settings();
	if (ret < 0) {
		UBOOT_ERROR("panel ssc dtbo failure with %d\n", ret);
	}
	return ret;
}
