// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <time.h>
#include <debug_impl.h>
#include <utility.h>
#include <mtk_panel.h>
#include <standby_mode.h>
#include <mtk-pm.h>

int do_panel_pre_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;

#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    UBOOT_TRACE("IN\n");

#ifdef CONFIG_MTK_PANEL
    mtk_panel_init_device();
#else
    if (argc < 2)
    {
        ret = mtk_panel_init();
    }
    else
    {
        if(strncmp(argv[1], "-d", 2) == 0)
        {
            ret = mtk_panel_init();
        }
        else if (strncmp(argv[1], "-s", 2) == 0)
        {
            ;
        }
        else
        {
           cmd_usage(cmdtp);
        }
    }
#endif

    UBOOT_TRACE("OK\n");
    return ret;
}

int do_panel_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    UBOOT_TRACE("IN\n");

#ifdef CONFIG_MTK_PANEL
    mtk_panel_init_device();

    if (mtk_is_panel_ready())
    {
        UBOOT_DEBUG("do_panel_output_enable : enable\n");
        mtk_panel_enable(true);
    }
#else
    if (argc < 2)
    {
        ret = mtk_panel_init();
    }
    else
    {
        if(strncmp(argv[1], "-d", 2) == 0)
        {
            ret = mtk_panel_init();
        }
        else if (strncmp(argv[1], "-s", 2) == 0)
        {
            ;
        }
        else
        {
           cmd_usage(cmdtp);
           return ret;
        }
    }
#endif

    if(mtk_is_panel_ready())
    {
        mtk_panel_backlight_on();
    }

    UBOOT_TRACE("OK\n");
    return ret;
}

int do_backlight_on(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    UBOOT_TRACE("IN\n");

    if(mtk_is_panel_ready())
    {
        if (pm_check_back_ground_active() == 0)
        {
            mtk_panel_backlight_on();
        }
        else
        {
            mtk_panel_backlight_off();
        }
    }
    else
    {
        UBOOT_ERROR("do Panel init first!!\n");
    }
    UBOOT_TRACE("OK\n");

    return ret;
}

#ifdef CONFIG_MTK_PANEL
int do_panel_output_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    int ret = 0;
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    UBOOT_TRACE("IN\n");
    if (mtk_is_panel_ready())
    {
        if (argc < 2)
        {
            UBOOT_ERROR("panel_enable command param error\n");
        }
        else
        {
            if (strncmp(argv[1],"1",1) == 0)
            {
                UBOOT_DEBUG("do_panel_output_enable : enable\n");
                mtk_panel_enable(true);
            }
            else if (strncmp(argv[1],"0",1) == 0)
            {
                UBOOT_DEBUG("do_panel_output_enable : disable\n");
                mtk_panel_enable(false);
            }
            else
                UBOOT_ERROR("panel_enable command param error\n");

        }
    }
    else
    {
        UBOOT_ERROR("do panel init first\n");
    }
    UBOOT_TRACE("OK\n");
    return ret;
}

int do_panel_mute(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
#if (CONFIG_HAPS == 1)
    char *ptr;
    ptr = env_get("two_haps");
    if(ptr == NULL)
        return 0;
    else if(strncmp(ptr,ENV_SETTING_ON,ENV_SETTING_ON_SIZE) != 0)
        return 0;
#endif

    if (pm_check_back_ground_active() == 1)
    {
        UBOOT_INFO("QHB case, do_panel_mute return\n");
        return 0;
    }

    UBOOT_TRACE("IN \n");

    if (mtk_is_panel_ready())
    {
        if (argc < 2)
        {
            UBOOT_ERROR("panel_mute command param error\n");
        }
        else
        {
            if (strncmp(argv[1],"1",1) == 0)
            {
                UBOOT_DEBUG("do_panel_mute : enable\n");
                mtk_panel_mute(true);
            }
            else if (strncmp(argv[1],"0",1) == 0)
            {
                UBOOT_DEBUG("do_panel_mute : disable\n");
                if ((MApi_check_is_trunk_flow() == false) && (MApi_check_is_oled() == true))
                {
                    /* unMute flow will control in post init*/
                    UBOOT_DEBUG("do_panel_mute unMute change to post init\n");
                }
                else
                {
                    mtk_panel_mute(false);
                }
            }
            else
                UBOOT_ERROR("panel_enable command param error\n");

        }
    }
    else
    {
        UBOOT_ERROR("do panel init first\n");
    }

    UBOOT_TRACE("OK\n");
    return ret;
}

int do_force_control_panel_vcc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;

    UBOOT_TRACE("IN \n");

    if (argc < 2)
    {
        UBOOT_ERROR("force_control_panel_vcc command param error\n");
    }
    else
    {
        if (strncmp(argv[1],"1",1) == 0)
        {
            UBOOT_DEBUG("do_force_control_panel_vcc : enable\n");
            mtk_panel_enable_vcc(true);
        }
        else if (strncmp(argv[1],"0",1) == 0)
        {
            UBOOT_DEBUG("do_force_control_panel_vcc : disable\n");
            mtk_panel_enable_vcc(false);
        }
        else
            UBOOT_ERROR("panel_enable command param error\n");

    }

    UBOOT_TRACE("OK\n");
    return ret;
}

#endif
#define MAXARGS_2 2

U_BOOT_CMD(
    panel_pre_init, 2, 1, do_panel_pre_init,
    "panel_pre_init  - init panel\n",
    "command: panel_pre_init [option]\n"
    " -s : static init : panel init para from uboot\n"
);

U_BOOT_CMD(
    panel_init, 2, 1, do_panel_init,
    "panel_init  - init panel\n",
    "command: panel_init [option]\n"
    " -s : static init : panel init para from uboot\n"
);

U_BOOT_CMD(
    panel_post_init, CONFIG_SYS_MAXARGS, 0, do_backlight_on,
    "backlight on  - backlight on\n",
    "command: backlight_on\n"
);


#ifdef CONFIG_MTK_PANEL
U_BOOT_CMD(
    panel_mute, MAXARGS_2, 1, do_panel_mute,
    "panel mute  - panel mute\n",
    "command: panel_mute\n"
);


U_BOOT_CMD(
    panel_enable, 2, 1, do_panel_output_enable,
    "panel_enable - enable panel output\n",
    "command: panel_enable 1 or panel_enable 0\n"
);

U_BOOT_CMD(
    force_control_panel_vcc, 2, 1, do_force_control_panel_vcc,
    "force_control_panel_vcc - control panel vcc\n",
    "command: force_control_panel_vcc 0 or force_control_panel_vcc 1\n"
);

#endif
