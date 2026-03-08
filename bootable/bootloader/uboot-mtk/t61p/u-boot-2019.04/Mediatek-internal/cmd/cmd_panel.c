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
#include <usb_upgrade.h>
#include <upgrade_osd.h>
#include <upgrade_utility.h>
#include <usb.h>
#include <environment.h>
#include <boot_impl.h>

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

    mtk_panel_init_device();

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

    mtk_panel_init_device();

    if (mtk_is_panel_ready())
    {
        UBOOT_DEBUG("do_panel_output_enable : enable\n");
        mtk_panel_enable(true);
    }

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
        if (standby_mode_enter_standby())
        {
            char *env_secondary = NULL;
            env_secondary = env_get("secondary_do_logomusic");
            if (env_secondary == NULL)
            {
                printf("second_standby_mode skip backlight !!! \n");
                return 0;
            }
        }
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


static int _init_usb_disk(void)
{
    UBOOT_TRACE("IN\n");
    char idx = 0;

#if   defined(ENABLE_FIFTH_EHC)
    const char u8_usb_port_count = 5;
#elif defined(ENABLE_FOURTH_EHC)
    const char u8_usb_port_count = 4;
#elif defined(ENABLE_THIRD_EHC)
    const char u8_usb_port_count = 3;
#elif defined(ENABLE_SECOND_EHC)
    const char u8_usb_port_count = 2;
#else
    const char u8_usb_port_count = 1;
#endif

    for (idx = 0; idx < u8_usb_port_count; idx++)
    {
        usb_stop();
#if defined (CONFIG_USB_PREINIT)
        if (usb_post_init(idx) == 0)
#else
        if (usb_init() == 0)
#endif
        {
            if (usb_stor_scan(1) == 0)
            {
                UBOOT_TRACE("OK\n");
                return 0 ;
            }
        }
    }
    UBOOT_TRACE("OK\n");
    return -1;
}

#define OLED_ERROR_CLEAR_FILEPATH "OledErrorClear.ini"
#define OLED_ERROR_LIMIT 3
#define OLED_ERROR_CLEAR_TIME 30
bool _is_oled_panel_error_count_valid(void)
{
    int errCnt = 0;
    char *OLEDPnlErrCnt = NULL;

    OLEDPnlErrCnt = env_get("OLEDPanelErrorCount");
    if (OLEDPnlErrCnt != NULL)
    {
        errCnt = simple_strtoul(OLEDPnlErrCnt, NULL, 16);
    }

    if ((OLEDPnlErrCnt == NULL) || ((errCnt >= 0) && (errCnt < OLED_ERROR_LIMIT)))
    {
        UBOOT_DEBUG("No need clear OLEDPanelErrorCount env.\n");
        return true;
    }
    else
    {
        UBOOT_ERROR("The OLEDPanelErrorCount has a value of %d which is over %d!\n", errCnt, OLED_ERROR_LIMIT);
        return false;
    }

}

int do_oled_panel_error_count_clear(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");

    if (_is_oled_panel_error_count_valid() == true)
    {
        UBOOT_TRACE("OK\n");
        return 0;
    }

    if (_init_usb_disk() != 0)
    {
        UBOOT_ERROR("Can NOT init usb!!\n");
        return -1;
    }

    if (check_file_exist("usb", OLED_ERROR_CLEAR_FILEPATH) == 0)  // check if the specified file in USB or not
    {
        env_set("OLEDPanelErrorCount", "0x0");
        env_save();
        UBOOT_ERROR("Clear OLEDPanelErrorCount env.\n");
    }
    else
    {
        UBOOT_ERROR("No %s in usb disk.\n", OLED_ERROR_CLEAR_FILEPATH);
    }

    UBOOT_TRACE("OK\n");
    return 0;
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

U_BOOT_CMD(
    oled_panel_err_cnt_clear,    CONFIG_SYS_MAXARGS,    1,     do_oled_panel_error_count_clear,
    "clear oled panel error count by USB file\n",
    "command: oled_panel_err_cnt_clear"
);

#endif
