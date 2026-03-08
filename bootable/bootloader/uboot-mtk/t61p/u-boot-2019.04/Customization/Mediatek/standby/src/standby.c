// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <mtk_standby.h>

#if defined(CONFIG_MTK_LED_SETTING)
#include <led_impl.h>
#endif

int do_standby_mode_power_control(void)
{
    int ret = -1;
    UBOOT_TRACE("start\n");

#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4_STANDBY)
    run_command("boot_vad", 0);
    run_command("voc notify", 0);
    run_command("voc standby", 0);
#endif

#if defined(CONFIG_MTK_PMU)
    run_command("boot_pmu", 0);
#endif

#if defined(CONFIG_MTK_BT_USB)
    run_command("setMtkBT", 0);
#endif
    run_command("usb stop", 0);

    // force panel vcc off
    run_command("force_control_panel_vcc 0", 0 );

#if defined(CONFIG_MTK_LED_IN_STANDBY)
    struct led_setting_info *info = NULL;
    if (!led_get_info(&info) && info != NULL) {
        if (info->led_use_setting && info->led_setting == 0) {
            UBOOT_DEBUG("led_ui_setting: don't set standby LED state.\n");
        } else {
            run_command(info->led_command, 0);
        }
    }
#endif

    ret = mtk_main_chip_power_standby();
    if(ret != 0)
        UBOOT_ERROR("main chip power down framework execuate failure\n");

    UBOOT_TRACE("end\n");
    return ret;
}
