// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <utility.h>
#include <iniparser.h>
#include <iniutility.h>
#include <idme.h>
#include <debug_impl.h>
#include <standby_mode.h>
#include <mtk-pm.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#endif
#include <dm/ofnode.h>

#define NODE_ODM_PATH                   "/odm"
#define KEY_SCREEN_STATE_FILE           "screen_state_file"
#define KEY_STOREMODE_SCREEN            "storemode_screen"
#define KEY_SCREEN_STATE_DEFAULT_NAME   "screen_state"
#define KEY_BYPASS_SECONDARY_STANDBY    "bypass_secondary_standby"
#define SCREEN_STATE_PARTITION          "uenv"

static bool already_init = false;
static bool pre_screen_state = false;
static struct standby_qhb_info standby_info;

extern bool IsPowerButtonPressed(void);
extern bool IsResetButtonPressed(void);

static bool standby_check_devicetree(const char *entry)
{
    bool ret = false;
    ofnode node;

    node = ofnode_path(NODE_ODM_PATH);
    if (ofnode_valid(node)) {
        ret = ofnode_read_bool(node, entry);
    }
    else {
        UBOOT_ERROR("ofnode_path %s failed.\n", NODE_ODM_PATH);
    }
    if (ret)
        UBOOT_TRACE("%s is defined.\n", entry);
    else
        UBOOT_TRACE("%s is not defined.\n", entry);
    return ret;
}

static const char *standby_check_devicetree_str(const char *entry)
{
    static const char *str = NULL;
    ofnode node;

    node = ofnode_path(NODE_ODM_PATH);
    if (ofnode_valid(node)) {
        str = ofnode_read_string(node, entry);
    }
    else {
        UBOOT_ERROR("ofnode_path %s failed.\n", NODE_ODM_PATH);
    }

    if (str)
        UBOOT_TRACE("screen state node is %s, and screen file is %s.\n", entry, str);
    else
        UBOOT_TRACE("screen state node is %s, but screen file is null.\n", entry);
    return str;
}

static bool standby_read_screen_state(const char *name)
{
    unsigned char *file_buf = NULL;
    bool ret = false;

    loff_t size;

    file_buf = read_storage_file_to_memory(SCREEN_STATE_PARTITION, name,  &size);
    if (file_buf && !strncmp((const char*)file_buf, "off", 3)) {
        UBOOT_TRACE("last screen state is off, set pre_screen_state to false.\n");
    }
    else {
        ret = true;
        UBOOT_TRACE("last screen state is on, set pre_screen_state to true.\n");
    }
    if (file_buf)
	free(file_buf);
    return ret;
}

static bool standby_check_idme(void)
{
    bool ret = true;
    // if the device is in store demo mode, bypass standby
    UBOOT_TRACE("check idme flags start\n");
    if (env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE))
    {
	if (standby_check_devicetree(KEY_STOREMODE_SCREEN)){
            UBOOT_TRACE("store mode enabled, check the prescreen state.\n");
            if (pre_screen_state){
                UBOOT_TRACE("store mode screen state is on, bypass standby mode\n");
	        ret = false;
	   }
	}
	else
		ret = false; //all other TV will bypass standby mode.
    }
    if (env_get("dev_flags") && (simple_strtoul(env_get("dev_flags"), NULL, 16) & DEV_FLAGS_BYPASS_SECONDARY_BOOT))
    {
        UBOOT_INFO("dev_flag set to bypass standby mode\n");
        ret = false;
    }
    UBOOT_TRACE("check idme flags standby end\n");
    return ret;
}
int standby_init(void)
{
    const char *str = NULL;
    int bootreason;
    bool bypass_standby = false;

#ifdef CONFIG_DATA_SEPARATION
    int ret = 0;
    char filepath[FILE_PATH_SIZE];
    char part[PART_NAME_SIZE];
    const char *relpath;

    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    UBOOT_DEBUG("standby_init with data separation : STANDBY_SECTION[%s]STANDBY_KEY[%s]\n",STANDBY_SECTION,STANDBY_KEY);
    if(dataindex_get_key(filepath, FILE_PATH_SIZE, STANDBY_SECTION, STANDBY_KEY, NULL) == 0)
    {
        if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret = load_standby_qhb_info(part,relpath,&standby_info);
            if(ret < 0)
            {
                UBOOT_ERROR("Read ini information failure with data separation.[%s][%s]\n",part,relpath);
                return -1;
            }
        }
        else
        {
            UBOOT_ERROR("resolve path fail: %s \n",filepath);
            return -1;
        }
    }
    else
    {
        UBOOT_ERROR("cannot get %s:%s from dataindex file\n",STANDBY_SECTION,STANDBY_KEY);
        return -1;
    }
#else
    ret = load_standby_qhb_info(STANDBY_PARTITION,STANDBY_FILE_PATH,&standby_info);
    if (ret < 0)
    {
        UBOOT_ERROR("Read ini information failure with default setting.[%s][%s]\n",STANDBY_PARTITION,STANDBY_FILE_PATH);
        return -1;
    }
#endif
    // check if set qhb mode based on last screen state.
    bootreason = pm_get_boot_reason();
    // reuse bypass_secondary_standby key to determine if set qhb mode or not.
    // the device bypassing secondary standby boots quiescently when last screen state is off.
    bypass_standby = standby_check_devicetree(KEY_BYPASS_SECONDARY_STANDBY);
    //get screen state file name
    str = standby_check_devicetree_str(KEY_SCREEN_STATE_FILE);
    // update the sreeen state if the file name is not default
    if (str)
       pre_screen_state = standby_read_screen_state(str);
    else
       //get previous screen status using default name
       pre_screen_state = standby_read_screen_state(KEY_SCREEN_STATE_DEFAULT_NAME);

    /* dev_flags has the highest priority to enable bypass standby mode*/
    if (env_get("dev_flags") && (simple_strtoul(env_get("dev_flags"), NULL, 16) & DEV_FLAGS_BYPASS_SECONDARY_BOOT)) {
        UBOOT_TRACE("dev_flag set to bypass standby mode. don't set qhb mode based on last screen state\n");
	return 0;
    }

    if (bootreason == PM_BR_PANIC ||
        bootreason == PM_BR_WATCHDOG ||
        bootreason == PM_BR_SW_WATCHDOG ||
        (bypass_standby && (bootreason == PM_BR_AC || bootreason == PM_BR_STR_STANDBY))) {
        if ( !pre_screen_state ) {
             UBOOT_TRACE("last screen state is off, enter quiescent mode!\n");
             standby_info.qhb_mode = 2;
	}
    }

    //disable qhb mode, or will no backlight when system last screen state is off.
    if ((!bypass_standby && IsPowerButtonPressed())||PM_BR_SECONDARY == bootreason) {
       UBOOT_INFO("bypass standby is not set in device tree, boot into standby mode!\n");
       standby_info.qhb_mode = 0;
    }
    return 0;
}

int standby_mode_get_mode(void)
{
    if (!already_init) {
        standby_init();
        already_init = true;
    }
    UBOOT_DEBUG("qhb_mode: %d\n",standby_info.qhb_mode);

    return standby_info.qhb_mode;
}

int standby_mode_is_enter_standby(void)
{
    if (!already_init) {
        standby_init();
        already_init = true;
    }
    UBOOT_DEBUG("enter_standby: %d\n",standby_info.enter_standby);

    return standby_info.enter_standby;
}

int standby_mode_get_second_standby_mode(void)
{
    if (!already_init) {
        standby_init();
        already_init = true;
    }
    UBOOT_DEBUG("second_standby_mode: %d\n",standby_info.second_standby_mode);

    return standby_info.second_standby_mode;
}

/*
* standby_mode_enter_standby: Check standby mode status API
* 0: Normal booting mode
* 1: second standby mode
*/
int standby_mode_enter_standby(void)
{
    int bootreason, standby_mode_pm_status;
    if (!already_init)
    {
        if (standby_init() != 0)
        {
            printf("Get standby flag failure, Normal booting mode!\n");
            return 0;
        }
        already_init = true;
    }

    if (standby_check_devicetree(KEY_BYPASS_SECONDARY_STANDBY)) {
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

    //check idme setting which has higher priority
    if (!standby_check_idme() ){
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

#ifndef CONFIG_ANDROID_AB
    // In non-AB, bypass standby when an OTA is interrupted in recovery mode
    if (is_recovery_mode()) {
        printf("Bypass standby due to recovery mode interrupted\n");
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }
#endif

    if (true == IsPowerButtonPressed())
    {
        UBOOT_INFO("Detect Keypad press do not enter stnadby mode.\n");
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

    if (IsResetButtonPressed() == true)
    {
        UBOOT_INFO("Detect Reset button press do not enter standby mode.\n");
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

    bootreason = pm_get_boot_reason();
    if (bootreason == PM_BR_AC)
    {
	    UBOOT_INFO("Enter second standby mode!, boot reason is 0x%02X\n",bootreason);
	    standby_mode_pm_status = 1;
    }
    else
    {
	    UBOOT_INFO("Normal booting mode!, boot reason is 0x%02X\n",bootreason);
	    standby_mode_pm_status = 0;
    }

    // ini file setting has the lowest priority
    UBOOT_DEBUG("check second_standby_mode setting from ini file: mode =  %d enter_standby: %d\n",
		 standby_info.second_standby_mode, standby_info.enter_standby);
    if ((standby_info.second_standby_mode == STANDBY_MEMORY_MODE && standby_info.enter_standby == 1)
       || (standby_info.second_standby_mode == STANDBY_SECOND_MODE))
    {
        UBOOT_INFO("Enter second standby mode by ini file setting.\n");
        standby_mode_pm_status = 1;
    }

    return(standby_mode_pm_status);
}
