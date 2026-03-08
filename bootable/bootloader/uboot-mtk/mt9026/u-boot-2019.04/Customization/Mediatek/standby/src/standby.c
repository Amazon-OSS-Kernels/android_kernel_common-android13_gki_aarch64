// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <mtk_standby.h>
#include <dm/ofnode.h>
#if defined(CONFIG_MTK_LED_SETTING)
#include <led_impl.h>
#endif
#define LED_QUICK_ON "led_quick_on"

int check_led_quick_on(void);

int do_standby_mode_power_control(void)
{
    int ret = -1;
    UBOOT_TRACE("start\n");
    int led_quickon = 0;

    if (check_led_quick_on() == 1) {
        led_quickon = 1;
    }

#if defined(CONFIG_REMOTEPROC_MTK_PMU_CORTEX_M4)
    run_command("boot_vad", 0);
    run_command("voc notify", 0);
    run_command("voc standby", 0);
#endif

#if defined(CONFIG_MTK_PMU)
    run_command("boot_pmu", 0);
#endif

if (led_quickon == 1) { // for abc123/abc123 which needs Quick LED ON feature
#if defined(CONFIG_MTK_LED_SETTING)
    struct led_setting_info *info = NULL;
    if (!led_get_info(&info) && info != NULL) {
        if (info->bypass_led_uboot_standby) {
            UBOOT_DEBUG("Bypass led behavior when uboot standby!!!\n");
        } else if (info->led_use_setting && info->led_setting == 0) {
            UBOOT_DEBUG("led_ui_setting is %d, set LED OFF.\n", info->led_setting);
            run_command("led mtk-tv:breath-led-0 1",0);
        } else {
            run_command(info->led_command, 0);
        }
    }
#endif
}

#if defined(CONFIG_MTK_BT_USB) || defined(CONFIG_RTK_BT_USB)
    ret = -1;
#if defined(CONFIG_MTK_BT_USB)
    if (ret != 0)
        ret = run_command("setMtkBT", 0);
#endif
#if defined(CONFIG_RTK_BT_USB)
    if (ret != 0)
        ret = run_command("setRtkBT", 0);
#endif
    if (ret != 0)
        UBOOT_ERROR("BT USB failed!\n");
#endif
    run_command("usb stop", 0);

#if defined(CONFIG_MEDIATEK_DTV_GMAC)
    run_command("phy_power_down", 0);
#endif

if (led_quickon == 0) { // for othres which doesn't need Quick LED ON feature
#if defined(CONFIG_MTK_LED_SETTING)
    struct led_setting_info *info = NULL;
    if (!led_get_info(&info) && info != NULL) {
        if (info->bypass_led_uboot_standby) {
            UBOOT_DEBUG("Bypass led behavior when uboot standby!!!\n");
        } else if (info->led_use_setting && info->led_setting == 0) {
            UBOOT_DEBUG("led_ui_setting is %d, set LED OFF.\n", info->led_setting);
            run_command("led mtk-tv:breath-led-0 1",0);
        } else {
            run_command(info->led_command, 0);
        }
    }
#endif
}

    ret = mtk_main_chip_power_standby();
    if (ret != 0)
        UBOOT_ERROR("main chip power down framework execute failure\n");

    UBOOT_TRACE("end\n");
    return ret;
}

int check_led_quick_on(void)
{
    int ret = 0;
    u32 val = 0;
    ofnode node;

    node = ofnode_path("/breath_leds");
    if (!ofnode_valid(node)) {
        UBOOT_ERROR("ofnode_path(%s) not found.\n", LED_QUICK_ON);
        return 0;
    }

    ret = ofnode_read_u32(node, LED_QUICK_ON, &val);
    UBOOT_DEBUG("ofnode_read_u32 %s, return %d, val %d.\n", LED_QUICK_ON, ret, val);
    if (ret ==0 && val == 1) {
        return 1;
    }
    else
        return 0;
}
