// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <common.h>
#include <command.h>
#include <exports.h>
#include <environment.h>
#include <MsTypes.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <usb_upgrade.h>
#include <upgrade_osd.h>
#include <upgrade_utility.h>
#include <secure/secure_upgrade.h>
#include <iniutility.h>
#include <utility.h>
#include <oad_upgrade.h>
#if defined(CONFIG_MTK_PM)
#include <mtk-pm.h>
#endif
#if (CONFIG_USB_UPGRADE == 1)
#include <usb.h>
#endif
#if (CONFIG_LED == 1)
#include <led.h>
#endif
#if defined(UFBL_FEATURE_IDME)
#include <idme.h>
#endif
#include <dm/ofnode.h>

#include <mtk_panel.h>
#include <display.h>
#include "MsCommon.h"

#define KEY_BREATH_LED_PATH			"/breath_leds"
#define KEY_USB_UPGRADE_LED			"usb_upgrade_led_command"

extern struct upgrade_info gupgradeinfo;


#if (CONFIG_USB_UPGRADE == 1)
// Init usb storage in application layer
static int _init_usb_disk(void)
{
    UBOOT_TRACE("IN\n");
    char idx=0;

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

    for(idx=0; idx<u8_usb_port_count; idx++)
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


// Do integrated USB upgrade, the upgrade steps are as follows,
// init. USB -> (register OSD call back func.) -> check bin. file -> upgrade -> (set auto-upgrade flag) -> reset
int usb_upgrade_pkg(void)
{
    UBOOT_TRACE("IN\n");
    int snprintf_len;
    int complete_flag = 0;
    int retry = 20;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    char *c_upgrade_status = NULL;
#if (CONFIG_USB_AUTO_UPGRADE == 1) && defined(UFBL_FEATURE_IDME)
    char bootmode[128] = "\0";
#endif
#if (CONFIG_LED == 1)
#if defined(CONFIG_USB_UPGRADE_LED)
    char cmd[COMMAND_BUF_SIZE] = {0};
    ofnode node;
    const char *str = NULL;
#endif
#endif

    /* 1. clean the env upgrade_status before upgrade */
    env_set(ENV_UPGRADE_STATUS, NULL);
    env_set(ENV_UPGRADE_MODE, NULL);
    env_set(ENV_UPGRADE_COMPLETE, NULL);
    env_save();

    /* 2. usb init */
    if (_init_usb_disk() != 0)
    {
        UBOOT_ERROR("Can NOT init usb!! \n");
        env_set(ENV_UPGRADE_STATUS, "ERROR");
        env_save();
        UBOOT_ERROR("Upgrade Fail !!!!!\n\n");
        return -1;
    }

#if (CONFIG_UPGRADE_OSD == 1)
    /* 3. register OAD function */
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

    /* 4. check upgrade file exist or not */
    snprintf_len=snprintf(gupgradeinfo.upgrade_filename, (sizeof(gupgradeinfo.upgrade_filename)-1), "%s",AP_BIN_PATH);
    if(snprintf_len<0)
    {
        UBOOT_ERROR("snprintf Error !!!\n");
        return -1;
    }

    snprintf_len=snprintf(gupgradeinfo.upgrade_mode, (sizeof(gupgradeinfo.upgrade_mode)-1), "%s","usb");
    if(snprintf_len<0)
    {
        UBOOT_ERROR("snprintf Error !!!\n");
        return -1;
    }

    if (check_file_exist("usb", gupgradeinfo.upgrade_filename) != 0)  // check if the specified file in USB or not
    {
        env_set(ENV_UPGRADE_STATUS, "ERROR");
        env_save();
        UBOOT_ERROR("Upgrade Fail !!!!!\n\n");
        return -1;
    }

#if (CONFIG_LED == 1)
#if defined(CONFIG_USB_UPGRADE_LED)
    UBOOT_INFO("USB upgrade: get LED command from dt key %s/%s\n", KEY_BREATH_LED_PATH, KEY_USB_UPGRADE_LED);
    node = ofnode_path(KEY_BREATH_LED_PATH);
    if (!ofnode_valid(node)) {
        UBOOT_ERROR("ofnode_path(%s) not found.\n", KEY_BREATH_LED_PATH);
    }
    else {
        str = ofnode_read_string(node, KEY_USB_UPGRADE_LED);
	if (str) {
	    snprintf(cmd, COMMAND_BUF_SIZE, "%s", str);
	}
    }
    UBOOT_INFO("USB upgrade: run led command: %s\n", cmd);
    run_command(cmd, 0);  // LED upgrade mode
#endif
#endif

    /* Wait for panel init all done, avoid DRAM wrong access leading system crash while USB upgrade */
    while (mtk_is_panel_enable() == FALSE && retry > 0)
    {
        printf("waiting for mtk_is_panel_enable, count %d\n", retry);
        MsOS_DelayTask(50);
        if (retry--)
            continue;
    }

    /* 5. start upgrading by application binary */
    env_set(ENV_UPGRADE_MODE, "usb");
    if (do_upgrade(gupgradeinfo.upgrade_filename, EN_UPDATE_MODE_USB, IF_TYPE_USB) == 0)  // read upgrade file and execute upgrade flow
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
        if (strncmp(c_upgrade_status, "ERROR", 5) == 0)
        {
#if (CONFIG_LED == 1)
#if defined(CONFIG_USB_UPGRADE_LED)
            snprintf(cmd, COMMAND_BUF_SIZE, "led %s %d", "mtk-tv:breath-led-0", LED_ON_1);
            run_command(cmd, 0);
#endif
#endif
            UBOOT_ERROR("Upgrade Fail !!!!!\n\n");
            mdelay(5000);
            char *p_str = env_get("usb_upgrade_auto_reset");
            int usb_upgrade_auto_reset = 1;
            if (p_str != NULL)
            {
                usb_upgrade_auto_reset = (int)simple_strtol(p_str, NULL, 10);
                UBOOT_DEBUG("usb_upgrade_auto_reset=%d\n", usb_upgrade_auto_reset);
            }

            if (usb_upgrade_auto_reset == 0)
            {
                UBOOT_DEBUG("jump to console!\n");
                jump_to_console();
            }
            else
            {
                UBOOT_DEBUG("reset system!\n");
                run_command("reset", 0);
            }
        }
    }

    /* write upgrade_info in storage*/
    if(sys_get_boot_device(device_name, sizeof(device_name)) < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
    }
    complete_flag = 1;
    if(write_upgrade_complete(device_name, &gupgradeinfo, complete_flag) == 0)
    {
        UBOOT_ERROR("write_upgrade_info fail\n");
    }
#ifdef CONFIG_SECOND_STANDBY
    env_set(ENV_UPGRADE_COMPLETE, "1");
#endif

    /* 7. clean the env upgrade_status after upgrade */
    env_set(ENV_UPGRADE_STATUS, NULL);
    env_set(ENV_UPGRADE_MODE, NULL);
    env_save();
    UBOOT_TRACE("OK\n");
    printf("\n\n########## Upgrade Succeed ##########\n");
    printf("########## Upgrade Succeed ##########\n");
    printf("########## Upgrade Succeed ##########\n\n");
#if (CONFIG_LED == 1)
#if defined(CONFIG_USB_UPGRADE_LED)
    snprintf(cmd, COMMAND_BUF_SIZE, "led %s %d", "mtk-tv:breath-led-0", LED_OFF_1);
    run_command(cmd, 0);
#endif
#endif

    /* 8. set auto-upgrade flag disable once after reboot */
#if (CONFIG_USB_AUTO_UPGRADE == 1)
#if defined(UFBL_FEATURE_IDME)
    idme_get_var_external("bootmode", bootmode, (127));
    printf("bootmode = %s\n", bootmode);
    if (strcmp(bootmode, "2")) // If Diag mode then do not disable the AUTO UPGRADE env
    {
        env_set(ENV_USB_AUTO_UPGRADE, "disable");     // do NOT do auto-upgrade after next reboot
        env_save();
        UBOOT_DEBUG("Close USB auto-upgrade once!!!\n");

    }
#else
    env_set(ENV_USB_AUTO_UPGRADE, "disable");     // do NOT do auto-upgrade after next reboot
    env_save();
    UBOOT_DEBUG("Close USB auto-upgrade once!!!\n");
#endif
#endif

    /* 9. check auto-reset flag */
    char *p_str = env_get("usb_upgrade_auto_reset");
    int usb_upgrade_auto_reset = 1;
    if (p_str != NULL)
    {
        usb_upgrade_auto_reset = (int)simple_strtol(p_str, NULL, 10);
        UBOOT_DEBUG("usb_upgrade_auto_reset=%d\n", usb_upgrade_auto_reset);
    }
    if (usb_upgrade_auto_reset == 0)
    {
        jump_to_console();
    }
    else
    {
        run_command("reset", 0);
    }
    return 0;
}

#if defined(CONFIG_MTK_INTERNAL_USB_UPGRADE_SCRIPPT)
int usb_upgrade_script(void)
{
    UBOOT_TRACE("IN\n");
    char cmd_buffer[CMD_BUF] = {0};
    unsigned char *fatload_buf = (unsigned char*)UPGRADE_LOAD_SCRIPT_BUFFER_ADDR;
    char *ree_script_buf = NULL;
    char *script_buf = NULL;
    char *next_line = NULL;
    char *script_file = SCRIPT_PATH;
    char *tmp = NULL;
    unsigned int file_size = 0;
    int partition = 0;
    int device = 0;
    int cmd_fail_abort = 1;
    int snprintf_len;

    /* 1. check script_file exist or not, and get device number, partition number */
    if (check_usb_file_partition("usb", &device, &partition, script_file) != 0)
    {
        UBOOT_ERROR("Can NOT find '%s' in usb!!\n", script_file);
        return -1;
    }

    /* 2. set commmand abort or not when cmd error. */
    tmp = env_get ("CmdAbort");
    if (tmp != NULL)
    {
        cmd_fail_abort = (int)simple_strtol(tmp, NULL, 10);
        UBOOT_DEBUG("cmd_fail_abort = %d\n", cmd_fail_abort);
    }

    /* 3. check file size */
    memset(cmd_buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(cmd_buffer, CMD_BUF, "fatsize usb %d:%d %s", device, partition, script_file);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, cmd_buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd = %s\n", cmd_buffer);
    if (run_command(cmd_buffer, 0) != 0)
    {
        UBOOT_ERROR("cmd: %s\n", cmd_buffer);
        return -1;
    }
    file_size = simple_strtol(env_get("filesize"), NULL, 16);
    UBOOT_DEBUG("file_size = 0x%u\n", file_size);

    /* 4. load all script into fatload_buf */
    memset(cmd_buffer, 0, CMD_BUF);
    snprintf_len = snprintf(cmd_buffer, CMD_BUF, "fatload usb %d:%d 0x%p %s 0x%u", device, partition, fatload_buf, script_file, file_size);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, cmd_buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd = %s\n",cmd_buffer);
    if (run_command(cmd_buffer, 0) != 0)
    {
        UBOOT_ERROR("cmd: %s\n", cmd_buffer);
        return -1;
    }

    /* 5. copy script from fatload_buf to ree_script_buf (avoid old script replaced by new script) */
    ree_script_buf = malloc(file_size+2);  // +2 because need to add '\n' & '%' in the end of script
    if (ree_script_buf == NULL)
    {
        UBOOT_ERROR("malloc fail \n");
        return -1;
    }
    memset(ree_script_buf, 0, file_size);
    memcpy(ree_script_buf, fatload_buf, file_size);
    UBOOT_DEBUG("Copy 0x%u bytes script '%s' from 0x%p to 0x%p\n", file_size, script_file, fatload_buf, ree_script_buf);

    /* 6. run script in ree_script_buf */
    script_buf = ree_script_buf;
    script_buf[file_size] = '\n';     // make sure there is an ending symbol in the end of script
    script_buf[file_size+1] = '%';    // make sure there is an ending symbol in the end of script
    while ((next_line = get_script_next_line(&script_buf)) != NULL)
    {
        UBOOT_INFO("\n>> %s \n", next_line);
        if(run_command(next_line, 0) != 0)
        {
            UBOOT_ERROR("cmd: %s\n", cmd_buffer);
            if(cmd_fail_abort == 1)
            {
                UBOOT_ERROR("Executer cmd error abort!!\n");
                free(ree_script_buf);
                return -1;
            }
        }
    }
    free(ree_script_buf);
    UBOOT_TRACE("OK\n");
    return 0;
}
#endif


#if (CONFIG_USB_AUTO_UPGRADE == 1)
int do_check_usb_auto_upgrade(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    char* auto_upgrade_flag = NULL;

#if defined(CONFIG_MTK_PM)
#if defined(UFBL_FEATURE_IDME)
    char bootmode[128] = "\0";
    char oem_data[128] = "\0";
    char *needle = NULL;

    if (!idme_get_var_external("oem_data", oem_data, (127)))
    {
        UBOOT_TRACE("oem_data = %s\n", oem_data);
        if ((needle = strstr(oem_data, ":usbboot")))
        {
            strncpy(needle, "\0\0\0\0\0\0\0\0", 8);
            if (!idme_update_var_ex("oem_data", oem_data, (127)))
            {
                UBOOT_TRACE("idme oem_data has ':usbboot'. Remove and run usb upgrade.\n");
                env_set(ENV_USB_AUTO_UPGRADE, "enable");
                env_save();
                run_command("mtkupgrade usb", 0);
                return 0;
            }
        }
    }
    else
    {
        UBOOT_ERROR("failed to read idme oem_data. %s:%d\n",__FUNCTION__, __LINE__);
    }

    idme_get_var_external("bootmode", bootmode, (127));
    printf("bootmode = %s\n", bootmode);
    if (strcmp(bootmode, "2")) // If Diag mode then no need to check the boot reason.
    {
        if(pm_get_boot_reason()==PM_BR_SECONDARY)
        {
            UBOOT_DEBUG("last status is secondary skip upgrade check  \n");
            return 0;
        }
        if ( pm_get_boot_reason() != PM_BR_AC ) {
            printf("Reason 0x%x ,Only AC ON need to check auto usb upgrade !\n", pm_get_boot_reason());
            return 0;
        }
    }
#else
    if(pm_get_boot_reason()==PM_BR_SECONDARY)
    {
        UBOOT_DEBUG("last status is secondary skip upgrade check  \n");
        return 0;
    }
    if ( pm_get_boot_reason() != PM_BR_AC ) {
        printf("Reason 0x%x ,Only AC ON need to check auto usb upgrade !\n", pm_get_boot_reason());
        return 0;
    }
#endif
#endif

    auto_upgrade_flag = env_get(ENV_USB_AUTO_UPGRADE);     // get auto-upgrade flag in uboot's env. variable

    if (auto_upgrade_flag == NULL)                          // if auto-upgrade flag is null, do auto-upgrade now
    {
        UBOOT_DEBUG("Not find usb_auto_upgrade in env, open & do USB auto-upgrade now!!!\n");
        env_set(ENV_USB_AUTO_UPGRADE, "enable");
        env_save();
        run_command("mtkupgrade usb", 0);
        return 0;
    }
    else
    {
        UBOOT_DEBUG("usb_auto_upgrade = %s\n", auto_upgrade_flag);

        if (strcmp(auto_upgrade_flag, "disable_once") == 0)  // if auto-upgrade flag is disable_once, NOT do auto-upgrade this time
        {
            UBOOT_DEBUG("Not need to do USB auto-upgrade this time!!!\n");
            env_set(ENV_USB_AUTO_UPGRADE, "enable");        //  enable auto-upgrade flag, so do auto-upgrade at next reboot
            env_save();
            UBOOT_DEBUG("Open USB auto-upgrade next time!!!\n");
            return 0;
        }
        else if (strcmp(auto_upgrade_flag, "disable") == 0) // if auto-upgrade flag is disable, NOT do auto-upgrade forever
        {
            UBOOT_DEBUG("Always not to do USB auto-upgrade!!!\n");
            return 0;
        }
        else                                              // if auto-upgrade flag is ohter setting(include enable), do auto-upgrade now
        {
            UBOOT_DEBUG("Do USB auto-upgrade now!!!\n");
            env_set(ENV_USB_AUTO_UPGRADE, "enable");     // enable auto-upgrade flag, so do auto-upgrade at next reboot
            env_save();
            run_command("mtkupgrade usb", 0);
            return 0;
        }
    }

    UBOOT_TRACE("Run command fail!!!!\n");
    return -1;
}
#endif
#endif

