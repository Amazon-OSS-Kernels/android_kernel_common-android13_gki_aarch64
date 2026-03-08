// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <mtk_cmdtable.h>
#include <mtk_gegop.h>
#include <bootflow.h>
#include <utility.h>

int do_customized_command_register(void)
{
    mtk_add_command_table("deviceid", 0, AFTER_UBOOT_INIT);
#if defined(CONFIG_ANDROID_CN_PLATFORM) || defined(CONFIG_LINUX_REF_PLATFORM)
    mtk_add_command_table("mmc cap", 0, BEFORE_CONSOLE_INPUT);
#endif
#ifdef CONFIG_CONSOLE_LOCK
#ifdef CONFIG_CONSOLE_UNLOCK_USB
    mtk_add_command_table("cetest cu_clear", 0, BEFORE_CONSOLE_INPUT);
    mtk_add_command_table("ce_usb", 0, BEFORE_CONSOLE_INPUT);
#endif
    mtk_add_command_table("check_ce", 0, BEFORE_BOOT_KERNEL);
#endif

#ifndef CONFIG_MULTICORES_PLATFORM
#ifdef CONFIG_BOOT_LOGO
    mtk_add_command_table("showlogo", 0, AFTER_CONSOLE_INPUT);
#endif
#ifdef CONFIG_MTK_PANEL
    mtk_add_command_table("panel_mute 0", 0, AFTER_CONSOLE_INPUT);
#endif
    mtk_add_command_table("panel_post_init", 0, AFTER_CONSOLE_INPUT);
#endif

#ifdef CONFIG_BOOT_IR
    mtk_add_command_table("bootircheck", 0, BEFORE_BOOT_KERNEL);
#endif

    return 0;
}
