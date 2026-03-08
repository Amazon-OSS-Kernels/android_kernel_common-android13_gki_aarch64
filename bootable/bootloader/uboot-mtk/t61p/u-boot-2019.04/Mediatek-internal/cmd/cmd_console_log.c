// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm/uclass.h>
#include <dm/uclass-id.h>
#include <dm/pinctrl.h>
#include <debug_impl.h>

int do_console_log(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	struct udevice *dev = NULL;

	UBOOT_TRACE("IN\n");
	ret = uclass_get_device(UCLASS_PINCTRL, 1, &dev);
	if (ret) {
		UBOOT_ERROR("uclass_get_device failed: %d\n", ret);
		return ret;
	}

	if (!dev) {
		UBOOT_ERROR("get serial device failed\n");
		return -ENODEV;
	}

	if (argc != 2) {
		cmd_usage(cmdtp);
	} else {
		if(strncmp(argv[1], "hdmi_b", 6) == 0) {
#ifdef CONFIG_PINCTRL
			ret = pinctrl_select_state(dev, "hdmib_pm_uart0");
			if (ret) {
				UBOOT_ERROR("pinctrl select hdmib_pm_uart0 failed: %d\n", ret);
				return ret;
			}
			UBOOT_INFO("pinctrl select hdmib_pm_uart0\n");
#else
			UBOOT_ERROR("pinctrl not supported\n");
#endif
		} else {
			cmd_usage(cmdtp);
		}
	}

	UBOOT_TRACE("OK\n");
	return ret;
}

U_BOOT_CMD(
	console_log, 2, 1, do_console_log,
	"console_log  - print console log from hdmi\n",
	"command: console_log <hdmi_b>\n"
	" hdmi_b : selec hdmi port b to print console log\n"
);
