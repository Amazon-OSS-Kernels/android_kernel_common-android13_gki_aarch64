// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <environment.h>
#include <debug_impl.h>
#include <standby_impl.h>
#include <system_impl.h>
#include <mtk_standby.h>
#include <standby.h>
#include <upgrade_utility.h>
#if defined(CONFIG_MTK_PM)
#include <mtk-pm.h>
#endif
#include <mmc.h>
#define ENV_OAD_UPGRADE_POWER_ON "upgrade_power_mode"

int mtk_main_chip_power_standby(void)
{
    int ret = 0;

    UBOOT_TRACE("start\n");
    ret = emmc_set_pon_off_short();
    if (ret != 0)
    {
        UBOOT_ERROR("power off short failure\n");
    }
#if defined(CONFIG_MTK_PM)
    pm_set_boot_reason(PM_BR_SECONDARY);
#endif
    run_command("poweroff",0);

    UBOOT_TRACE("end\n");
    return 0;
}

int mtk_standby_mode_framework(void)
{
    int ret = 0;
    UBOOT_TRACE("start\n");
    char* upgrade_power_on_flag = NULL;
    char *usb_upgrade_complete_flag = NULL;
#if defined(CONFIG_MTK_PM)
    int bootreason = pm_get_boot_reason();
    if (bootreason == PM_BR_SECONDARY || bootreason == PM_BR_DC || bootreason == PM_BR_MAX_CNT ||
        bootreason == PM_BR_REBOOT || bootreason == PM_BR_REBOOT_SHELL || bootreason == PM_BR_REBOOT_FORCE || bootreason == PM_BR_LONG_PRESS_PWR_KEY)
    {
        UBOOT_DEBUG("Normal boot reason 0x%x \n", bootreason);
        return 0;
    }
#endif
    // after usb upgrade complete control
    usb_upgrade_complete_flag = env_get(ENV_UPGRADE_COMPLETE);
    if (usb_upgrade_complete_flag != NULL && strncmp(usb_upgrade_complete_flag, "1", 1) == 0)
    {
        env_set(ENV_UPGRADE_COMPLETE, NULL); //clean power on flag
        UBOOT_DEBUG("Normal boot reason: USB Upgrade Power on \n");
        env_save();
        return 0;
    }
    // after (Oad / update logic) upgrade power on control
    upgrade_power_on_flag=strdup(env_get(ENV_OAD_UPGRADE_POWER_ON));
    if(upgrade_power_on_flag!=NULL && strncmp(upgrade_power_on_flag, "power_on", strlen("power_on")) == 0)
    {
        env_set(ENV_OAD_UPGRADE_POWER_ON,NULL);//clean power on flag
        UBOOT_DEBUG("Normal boot reason: OAD Upgrade Power on \n");
        env_save();
        free(upgrade_power_on_flag);
        return 0;
    }
    /* Write remote boot status */
    PMU_REG_ACCESS(PMU_DUMMY_ADDR,PMU_DUMMY_OFFSET_BOOT_STATUS) = REMOTE_STATE_SECOND_STANDBY;
    UBOOT_DEBUG("Enter standby [%lx:%lx]=%lx\n",(unsigned long)PMU_DUMMY_ADDR, (unsigned long)PMU_DUMMY_OFFSET_BOOT_STATUS, (unsigned long)PMU_REG_ACCESS(PMU_DUMMY_ADDR,PMU_DUMMY_OFFSET_BOOT_STATUS));

    ret = do_standby_mode_power_control();
    if(ret != 0)
        UBOOT_ERROR("power control framework execuate failure\n");
    if(upgrade_power_on_flag)
        free(upgrade_power_on_flag);
    UBOOT_TRACE("end\n");
    return 0;
}
