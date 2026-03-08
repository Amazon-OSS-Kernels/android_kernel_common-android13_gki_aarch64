/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <exports.h>
#include <environment.h>
#include <MsTypes.h>
#include <utility.h>
#include <system_impl.h>
#include <upgrade_utility.h>
#include <secure/secure_upgrade.h>
#include <iniutility.h>
#include <oad_upgrade.h>
#include <debug_impl.h>
#include <upgrade_osd.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#endif


extern struct upgrade_info gupgradeinfo;
#define ENV_OAD_UPGRADE "oad_upgrade"
#define ENV_OAD_UPGRADE_POWER_ON "upgrade_power_mode"
#define OAD_UPGRADE_FAIIL_DELAY 5000
#if defined(CONFIG_ANDROID_CN_PLATFORM)
#define UPGRADE_PARTITION   "persist"
#define UPGRADE_FILE_PATH   "upgrade_mode.ini"
#elif defined(CONFIG_LINUX_REF_PLATFORM)
#define UPGRADE_PARTITION   "persist"
#define UPGRADE_FILE_PATH   "upgrade_mode.ini"
#else
#define UPGRADE_PARTITION   "persist"
#define UPGRADE_FILE_PATH   "/bsp/common/misc/upgrade_mode.ini"
#endif

int oad_upgrade(char *upgrade_file)
{
    UBOOT_TRACE("IN\n");
    int complete_flag=0;
    char *c_upgrade_status = NULL;
    int ret = -1;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    enum if_type iftype=IF_TYPE_UNKNOWN;

    /* 1. clean the env upgrade_status before upgrade */
    env_set(ENV_UPGRADE_STATUS, NULL);
    env_set(ENV_UPGRADE_MODE, NULL);
    env_save();

#if (CONFIG_UPGRADE_OSD == 1)
    /* 2. register OAD function */
    un_register_show_cb();
#if (CONFIG_LVGL_UPGRADE_BAR == 1)
    register_cb_show_error((show_error_cb)show_lvgl_Error);
    register_cb_show_finish((show_finish_cb)show_lvgl_Finish);
    register_cb_show_load_data((show_load_data_cb)show_lvgl_LoadData);
    register_cb_show_start_upgrading((show_start_upgrading_cb)show_lvgl_StartUpgrading);
    register_cb_show_upgrading((show_upgrading_cb)show_lvgl_Upgrading);
#else
    register_cb_show_error((show_error_cb)show_error);
    register_cb_show_finish((show_finish_cb)show_finish);
    register_cb_show_load_data((show_load_data_cb)show_load_data);
    register_cb_show_start_upgrading((show_start_upgrading_cb)show_start_upgrading);
    register_cb_show_upgrading((show_upgrading_cb)show_upgrading);
#endif
#endif
    /* 3. get upgrade infor from upgrade.ini */
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return ret;
    }
    if (strncmp(device_name, "usb", strlen("usb")) == 0)
        iftype=IF_TYPE_USB;
    else if (strncmp(device_name, "mmc", strlen("mmc")) == 0)
        iftype=IF_TYPE_MMC;
    else if (strncmp(device_name, "ufs", strlen("ufs")) == 0)
        iftype=IF_TYPE_UFS;
    else
    {
        UBOOT_ERROR("Not support interface type '%s' (only support usb, mmc, ufs case)\n", device_name);
        return -1;
    }

    /* 5. start upgrading by application binary */
    env_set(ENV_UPGRADE_MODE, "oad");
    if (do_upgrade(gupgradeinfo.upgrade_filename, EN_UPDATE_MODE_OAD, iftype) == 0)  // read upgrade file and execute upgrade flow
    {
        UBOOT_INFO("Upgrade file '%s' finished !!\n", gupgradeinfo.upgrade_filename);
    }
    else
    {
        UBOOT_ERROR("Upgrade file '%s' error!\n", gupgradeinfo.upgrade_filename);
        env_set(ENV_UPGRADE_STATUS, "ERROR");
        env_save();
        UBOOT_ERROR("Upgrade Fail !!!!!\n\n");
        //return -1;
    }

    /* 6. check if there are any errors during the upgrade process */
    c_upgrade_status = env_get(ENV_UPGRADE_STATUS);
    if (c_upgrade_status != NULL)
    {
        if (strncmp(c_upgrade_status, "ERROR", strlen("ERROR")) == 0)
        {
            UBOOT_ERROR("OAD Upgrade Fail !!!!!\n\n");
            mdelay(OAD_UPGRADE_FAIIL_DELAY);
            run_command("reset", 0);
        }
    }
    /* 7. write upgrade_info in storage */
    complete_flag = 1;
    if(write_upgrade_complete(device_name, &gupgradeinfo, complete_flag) == 0)
    {
        UBOOT_DEBUG("write_upgrade_info fail\n");
    }
    UBOOT_DEBUG("upgrade_power_mode =%s \n",gupgradeinfo.upgrade_power_mode);
    if (strncmp(gupgradeinfo.upgrade_power_mode, "power_on", strlen("power_on")) == 0)
        env_set(ENV_OAD_UPGRADE_POWER_ON, "power_on");
    /* 8. clean the env upgrade_status after upgrade */
    env_set(ENV_UPGRADE_STATUS, NULL);
    env_set(ENV_UPGRADE_MODE, NULL);
    env_set(ENV_OAD_UPGRADE, "disable_once");     // do NOT do oad upgrade after next reboot
    UBOOT_DEBUG("Close OAD upgrade once!!!\n");
    env_save();
    UBOOT_TRACE("OK\n");
    UBOOT_INFO("\n\n########## Upgrade Succeed ##########\n");
    UBOOT_INFO("########## Upgrade Succeed ##########\n");
    UBOOT_INFO("########## Upgrade Succeed ##########\n\n");
    run_command("reset", 0);

    return 0;
}
int do_check_upgrade_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    int ret_info = 0;
    char* oad_upgrade_flag = NULL;
    char cmd_buffer[CMD_BUF] = {0};
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE],part[PART_NAME_SIZE];
    const char *relpath;
    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    UBOOT_DEBUG("do_check_upgrade_mode with data separation : UPGRADE_SECTION[%s]UPGRADE_KEY[%s]\n",UPGRADE_SECTION,UPGRADE_KEY);
    if(dataindex_get_key(filepath, FILE_PATH_SIZE, UPGRADE_SECTION, UPGRADE_KEY, NULL) == 0)
    {
        if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret_info = get_upgrade_info(part,relpath,&gupgradeinfo);
            if(ret_info < 0)
            {
                UBOOT_ERROR("Read ini information failure with data separation.[%s][%s]\n",part,relpath);
                return 0;
            }
        }
        else
        {
            UBOOT_ERROR("resolve path fail: %s\n", filepath);
            return 0;
        }
    }
    else
    {
        UBOOT_ERROR("cannot get %s:%s from dataindex file\n",UPGRADE_SECTION,UPGRADE_KEY);
        return 0;
    }

#else
    ret_info =get_upgrade_info(UPGRADE_PARTITION,UPGRADE_FILE_PATH,&gupgradeinfo);
    if(ret_info < 0)
    {
        UBOOT_ERROR("Read upgrade.ini information failure.\n");
        return 0;
    }
#endif

    oad_upgrade_flag = env_get(ENV_OAD_UPGRADE);     // get oad_upgrade_flag in uboot's env. variable
    if ((strcmp(gupgradeinfo.upgrade_mode, "oad") == 0)
            ||(strcmp(gupgradeinfo.upgrade_mode, "updatelogic") == 0))
    {
        UBOOT_DEBUG("Run OAD/Upgrade logic!!!!\n");
        if (oad_upgrade_flag == NULL)                          // if auto-upgrade flag is null, oad/updatelogic upgrade now
        {
            UBOOT_DEBUG("Not find oad_upgrade_flage in env, open & do OAD upgrade now!!!\n");
            env_set(ENV_OAD_UPGRADE, "enable");
            env_save();
            oad_upgrade(NULL);
            return 0;
        }
        else
        {
            UBOOT_DEBUG("oad_upgrade = %s\n", oad_upgrade_flag);

            if (strcmp(oad_upgrade_flag, "disable_once") == 0)  // if oad_upgrade_flag is disable_once, NOT do OAD/updatelogic upgrade this time
            {
                UBOOT_DEBUG("Not need to do OAD upgrade this time!!!\n");
                env_set(ENV_OAD_UPGRADE, "enable");        //  enable oad_upgrade_flag, so do oad/updatelogic upgrade at next reboot
                env_save();
                UBOOT_DEBUG("Open OAD upgrade next time!!!\n");
                return 0;
            }
            else if (strcmp(oad_upgrade_flag, "disable") == 0) // if oad_upgrade_flag is disable, NOT do oad/updatelogic upgrade forever
            {
                UBOOT_DEBUG("Always not to do OAD upgrade!!!\n");
                return 0;
            }
            else                                              // if oad_upgrade_flag is ohter setting(include enable), do oad/updatelogic upgrade now
            {
                UBOOT_DEBUG("Do OAD upgrade now!!!\n");
                env_set(ENV_OAD_UPGRADE, "enable");     // enable oad_upgrade_flag, so do oad/updatelogic upgrade at next reboot
                env_save();
                oad_upgrade(NULL);
                return 0;
            }
        }
    }
    else if(strcmp(gupgradeinfo.upgrade_mode, "usb") == 0)
    {
        UBOOT_DEBUG("UI Tirgger USB Upgread !!!!\n");
        memset(cmd_buffer, 0 , CMD_BUF);
        if(snprintf(cmd_buffer, CMD_BUF, "mtkupgrade usb %s", gupgradeinfo.upgrade_filename)<0)
        {
            UBOOT_ERROR("snprintf Error !!!\n");
            return 0;
        }
        UBOOT_DEBUG(" run command :%s \n",cmd_buffer);
        run_command(cmd_buffer,0);
    }
    UBOOT_TRACE("OK\n");
    return 1;

}

int write_upgrade_complete(char *device_name, struct upgrade_info *upgradeinfo, int complete_flag)
{
    unsigned char *out_ini=NULL;
    unsigned int out_size=0;
    int ret = 0;

    UBOOT_TRACE("IN\n");
    out_ini = (unsigned char *)malloc(sizeof(struct upgrade_info));
    if(out_ini == NULL)
    {
        UBOOT_DEBUG("Failed to allocate out_ini\n");
        return 0;
    }
    memset(out_ini, 0, sizeof(struct upgrade_info));
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE],part[PART_NAME_SIZE];
    const char *relpath;
    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    UBOOT_DEBUG("write_upgrade_complete with data separation : UPGRADE_SECTION[%s]UPGRADE_KEY[%s]\n",UPGRADE_SECTION,UPGRADE_KEY);
    if(dataindex_get_key(filepath, FILE_PATH_SIZE, UPGRADE_SECTION, UPGRADE_KEY, NULL) == 0)
    {
        if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret = set_upgrade_complete(part,relpath, upgradeinfo, complete_flag, &out_ini, &out_size);
            if(ret < 0)
            {
                UBOOT_ERROR("Read ini information failure with data separation.[%s][%s]\n",part,relpath);
                free(out_ini);
                return 0;
            }
        }
        else
        {
            UBOOT_ERROR("resolve path fail: %s",filepath);
            free(out_ini);
            return 0;
        }
    }
    else
    {
        UBOOT_ERROR("cannot get %s:%s from dataindex file\n",UPGRADE_SECTION,UPGRADE_KEY);
        free(out_ini);
        return 0;
    }

#else
    ret =set_upgrade_complete(UPGRADE_PARTITION,UPGRADE_FILE_PATH, upgradeinfo, complete_flag, &out_ini, &out_size);
    if(ret < 0)
    {
        UBOOT_ERROR("Set upgrade.ini information failure.\n");
        if (out_ini)
            free(out_ini);
        return 0;
    }
#endif
    if (out_ini)
        free(out_ini);
    return 1;
}
