// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <debug_impl.h>
#include <standby_impl.h>
#include <iniutility.h>
#include <standby_mode.h>
#include <upgrade_utility.h>
#include <string.h>
#include <boot_impl.h>
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------
#define COMMAND_ARGUMENT_NUM_TWO    2
extern bool IsPowerButtonPressed(void);

int do_run_standby_mode (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    char *usb_upgrade_complete = NULL;

    int enter_standby = standby_mode_is_enter_standby();
    int second_standby_mode = standby_mode_get_second_standby_mode();

    UBOOT_TRACE("start do_run_standby_mode\n");
    UBOOT_DEBUG("second_standby_mode =%d  enter_standby=%d\n", second_standby_mode, enter_standby);

    if (standby_mode_enter_standby())
    {
        UBOOT_TRACE("start standby framework.\n");
        ret = mtk_standby_mode_framework();
        if(ret != 0)
        {
            UBOOT_ERROR("do_run_standby_mode failure\n");
            return ret;
        }
    }
    UBOOT_TRACE("continue booting!\n");
    usb_upgrade_complete = env_get(ENV_UPGRADE_COMPLETE);
    if (usb_upgrade_complete != NULL && strncmp(usb_upgrade_complete, "1", 1) == 0)
    {
        env_set(ENV_UPGRADE_COMPLETE, NULL);
        env_save();
    }

    UBOOT_TRACE("do_run_standby_mode end\n");
    return 0;
}

U_BOOT_CMD(
    standby, 2, 1,    do_run_standby_mode,
    "standby  - run standby mode framework\n",
    NULL
);

int do_standby_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;

    UBOOT_TRACE("start\n");
    ret = standby_init();
    if (ret != 0)
    {
        UBOOT_ERROR("standby_init failure, can not read data from standby_mode.ini\n");
    }

    UBOOT_TRACE("end\n");
    return 0;
}

U_BOOT_CMD(
    standbyinit, COMMAND_ARGUMENT_NUM_TWO, 1,    do_standby_init,
    "standbyinit  - Get standby mode information\n",
    NULL
);
