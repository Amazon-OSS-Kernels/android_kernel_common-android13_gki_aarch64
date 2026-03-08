// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <debug_impl.h>
#include <mtk_cmdtable.h>
#include <flow.h>
#include <utility.h>
#include <system_impl.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <flow_thread.h>
#endif

#ifdef UFBL_FEATURE_IDME
#include <idme.h>
#endif
#if defined(CONFIG_MTK_LED_SETTING)
#include <led_impl.h>
#endif
#include <amzn_tv_secure_boot.h>
int do_basic_command_register(void)
{

    mtk_add_command_table("chipid", 0, AFTER_UBOOT_INIT);
    mtk_add_command_table("revisionid", 0, AFTER_UBOOT_INIT);
    mtk_add_command_table("mtk_drv_init", 0, AFTER_UBOOT_INIT);
#ifdef CONFIG_DATA_SEPARATION
    mtk_add_command_table("dataindex setpath", 0, AFTER_UBOOT_INIT);
#endif
#if defined(CONFIG_SECOND_STANDBY)
    mtk_add_command_table("standbyinit", 0, AFTER_UBOOT_INIT);
#endif
#if defined(CONFIG_MTK_PM)
    mtk_add_command_table("pm_init", 0, AFTER_UBOOT_INIT);
#endif
#ifndef CONFIG_MULTICORES_PLATFORM
#if defined(CONFIG_BOOTPMU_EARLY)
#if defined(CONFIG_MTK_PMU)
    mtk_add_command_table("boot_pmu", 0, AFTER_UBOOT_INIT);
#elif defined(CONFIG_REMOTEPROC_MTK_PQU)
    mtk_add_command_table("rproc init", 0, AFTER_UBOOT_INIT);
#endif
#endif
#endif
#if defined(CONFIG_USERDATA_PARTITION_SELF_ADAPTION)
    mtk_add_command_table("emmc_self_adaption", 0, AFTER_UBOOT_INIT);
#endif
    mtk_add_command_table("bootcheck", 0, AFTER_UBOOT_INIT);
    mtk_add_command_table("propagate_mmap_filename", 0, BEFORE_CONSOLE_INPUT);
    mtk_add_command_table("setup_kernelcore_size", 0, BEFORE_CONSOLE_INPUT);
    mtk_add_command_table("keypadlongdetect", 0, AFTER_UBOOT_INIT);

#ifndef CONFIG_MULTICORES_PLATFORM
    mtk_add_command_table("panel_pre_init", 0, BEFORE_CONSOLE_INPUT);
#ifdef CONFIG_MTK_PANEL
    mtk_add_command_table("panel_mute 1", 0, BEFORE_CONSOLE_INPUT);
    mtk_add_command_table("panel_enable 1", 0, BEFORE_CONSOLE_INPUT);
#endif
#endif
#ifdef CONFIG_OAD_UPGRADE
    mtk_add_command_table("check_upgrade_mode", 0, AFTER_CONSOLE_INPUT);
#endif

#ifndef CONFIG_MULTICORES_PLATFORM
#ifdef CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4
    mtk_add_command_table("boot_vad", 0, AFTER_CONSOLE_INPUT);
#endif

#if !defined(CONFIG_BOOTPMU_EARLY)
#if defined(CONFIG_MTK_PMU)
    mtk_add_command_table("boot_pmu", 0, AFTER_CONSOLE_INPUT);
#endif
#if defined(CONFIG_MTK_LED_SETTING)
    struct led_setting_info *info = NULL;
    if (!led_get_info(&info) && info != NULL) {
        if (info->led_use_setting && info->led_setting == 0) {
            UBOOT_DEBUG("led_ui_setting is %d, don't set LED state.\n", info->led_setting);
        }
        else if (info->led_use_screen_state && info->led_scree_state == 0) {
            UBOOT_DEBUG("last screen state is %d, don't set LED state.\n", info->led_scree_state);
        }
        else {
            mtk_add_command_table(info->led_command, 0, AFTER_CONSOLE_INPUT);
        }
    }
    else {
        UBOOT_ERROR("get led info failure.\n");
    }
#endif
#endif
#endif

#ifdef CONFIG_SECOND_STANDBY
    mtk_add_command_table("standby", 0, BEFORE_CONSOLE_INPUT);
#endif

    mtk_add_command_table("boot_dramc", 0, BEFORE_BOOT_KERNEL);

#ifndef CONFIG_MULTICORES_PLATFORM
#if defined(CONFIG_SOUND)
    mtk_add_command_table("bootmusic init", 0, BEFORE_BOOT_KERNEL);
    mtk_add_command_table("bootmusic play", 0, BEFORE_BOOT_KERNEL);
#endif
#endif
#ifdef CONFIG_USB_AUTO_UPGRADE
    mtk_add_command_table("check_usb_auto_upgrade", 0, BEFORE_BOOT_KERNEL);
#endif
    mtk_add_command_table("upgradecheck", 0, BEFORE_BOOT_KERNEL);
    return 0;
}

int do_after_uboot_init(void)
{
    mtk_run_command_table(AFTER_UBOOT_INIT);
#ifdef CONFIG_MULTICORES_PLATFORM
    do_after_uboot_init_task_create();
#endif
    return 0;
}

int do_before_console_input(void)
{
    mtk_run_command_table(BEFORE_CONSOLE_INPUT);
#ifdef CONFIG_MULTICORES_PLATFORM
    do_before_console_input_task_create();
#endif
    return 0;
}

int do_after_console_input(void)
{
    mtk_run_command_table(AFTER_CONSOLE_INPUT);
#ifdef CONFIG_MULTICORES_PLATFORM
    do_after_console_input_task_create();
#endif
    return 0;
}

int do_before_boot_kernel(void)
{
    mtk_run_command_table(BEFORE_BOOT_KERNEL);
#ifdef CONFIG_MULTICORES_PLATFORM
    do_before_boot_kernel_task_create();
#endif
    return 0;
}

/*
 * read fos_flags from idme
 */
unsigned long get_fos_flags(void)
{
    unsigned long flags = 0;

    char fos_buf[16];
    int ret = 0;
#ifdef UFBL_FEATURE_IDME
    ret = idme_get_var_external("fos_flags", fos_buf, sizeof(fos_buf));
#endif

    if (ret < 0) {
        printf("get idme fos_flags Error\n");
        return 0;
    }
    flags = simple_strtoul(fos_buf, NULL, 16);

    printf("fos_flags=%lx\n", flags);
    return flags;
}

/*
 * Checks whether dm-verity is disabled
 * For locked production device , always return false
 * For unlocked/engineering device, check amazon fos_flags
 *      if bit7 is set, return true
 *      if bit7 is clear, return false
 *
 */
int amzn_dm_verity_is_off(void)
{
    int lock_state = is_lockdown();
    if (lock_state) {
        /* Locked device: dm-verity should be on and cannot be off */







        return 0;
    } else if (get_fos_flags() & FOS_FLAGS_DM_VERITY_OFF) {
        /*
         * Unlocked/Engineering device with bit 7 set
         * in fos_flags, dm-verity is off
         */
        return 1;
    } else {
        /* dm-verity is on otherwise */
        return 0;
    }
}
int do_jump_to_kernel(void)
{
    run_command("checkteesuccess", 0);
#if (CONFIG_CMD_AVB == 1)
    run_command("avb init 0", 0);
    if (amzn_dm_verity_is_off() == 0) {

        run_command("avb set-verity enable", 0);
        run_command("avb set-devicestate 1", 0);
    } else {
        run_command("avb set-devicestate 0", 0);
        run_command("avb set-verity disable", 0);
    }
#if (CONFIG_AB_SIDELOAD == 1)
    run_command("ab_slt 0", 0);
#else
    run_command("avb verify", 0);
#endif
#else
#if (CONFIG_AB_SIDELOAD == 1)
    run_command("ab_slt 0", 0);
#endif
#endif
    return 0;
}
