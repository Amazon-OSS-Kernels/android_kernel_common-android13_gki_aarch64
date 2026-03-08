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

#define SCREEN_STATE_PARTITION          "uenv"
#define NODE_ODM_PATH                   "/odm"
#define KEY_SPECIFIC_STANDBY_BEHAVIOR    "specific_standby_behavior"
#define KEY_BYPASS_UBOOTSTANDBY         "bypass_ubootstandby_behavior"

static bool already_init = false;
static struct standby_qhb_info standby_info;

extern bool IsPowerButtonPressed(void);
extern bool IsResetButtonPressed(void);

static bool screen_is_off = false;
bool oobe_completed = false;

bool check_specific_standby_behavior(void)
{
    bool ret = false;
    ofnode node;

    node = ofnode_path(NODE_ODM_PATH);
    if (ofnode_valid(node))
        ret = ofnode_read_bool(node, KEY_SPECIFIC_STANDBY_BEHAVIOR);
    else
        UBOOT_DEBUG("ofnode_path %s failed.\n", NODE_ODM_PATH);

    UBOOT_DEBUG("check_specific_standby_behavior, ret=%d\n", ret);
    return ret;
}

static bool check_bypass_ubootstandby_flag(void)
{
    bool ret = false;
    ofnode node;

    node = ofnode_path(NODE_ODM_PATH);
    if (ofnode_valid(node))
        ret = ofnode_read_bool(node, KEY_BYPASS_UBOOTSTANDBY);
    else
        UBOOT_DEBUG("ofnode_path %s failed.\n", NODE_ODM_PATH);

    UBOOT_DEBUG("check_bypass_ubootstandby_flag, ret=%d\n", ret);
    return ret;
}

static bool standby_check_idme(void)
{
    bool ret = true;
    // if the device is in store demo mode, bypass standby
    UBOOT_TRACE("check idme flags start\n");
    if (env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE))
    {
        UBOOT_INFO("usr_flags set to store dome mode, bypass standby mode\n");
        ret =  false;
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
    int ret=0;
#ifdef CONFIG_DATA_SEPARATION
    char filepath[FILE_PATH_SIZE],part[PART_NAME_SIZE];
    const char *relpath;
    unsigned char *file_buf = NULL;
    loff_t size;
    int bootreason = pm_get_boot_reason();

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

    if ((bootreason == PM_BR_AC && (env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE)) )) {
        UBOOT_INFO("store mode set to bypass qhb mode.\n");
        standby_info.qhb_mode = 0;
        return 0;
    }

    file_buf = read_storage_file_to_memory(SCREEN_STATE_PARTITION, "screen_state", &size);
    if (file_buf) {
        screen_is_off = !strncmp((const char*)file_buf, "off", 3);
        if (screen_is_off) {
            UBOOT_INFO("last screen state is off. set qhb_mode to 2. boot quiescent mode.\n");
            standby_info.qhb_mode = 2;
        }
        free(file_buf);
    }

    if (check_specific_standby_behavior()) {
        file_buf = read_storage_file_to_memory(SCREEN_STATE_PARTITION, "oobe_status", &size);
        if (file_buf) {
            oobe_completed = !strncmp((const char*)file_buf, "completed", 9);
            if (oobe_completed) {
                UBOOT_INFO("last system status is oobe completed status.\n");
            }
            free(file_buf);
        }
    }

    bootreason = pm_get_boot_reason();
    if (check_specific_standby_behavior()) {
        bool store_mode = env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE);
        char *abnormal_reboot_flag = env_get("abnormal_reboot_flag");

        if (true == IsPowerButtonPressed() || PM_BR_SECONDARY == bootreason ||
            (abnormal_reboot_flag && !strncmp(abnormal_reboot_flag, "true", 4))) {
            UBOOT_INFO("force bypass enter quiescent mode!\n");
            standby_info.qhb_mode = 0;
        } else if (store_mode && PM_BR_AC == bootreason) {
            UBOOT_INFO("force bypass quiescent mode bootup system for AC on when store mode!!!\n");
            standby_info.qhb_mode = 0;
        } else if (oobe_completed && PM_BR_AC == bootreason) {
            UBOOT_INFO("force quiescent mode bootup system for AC on!!!\n");
            standby_info.qhb_mode = 2;
        }
    } else if (check_bypass_ubootstandby_flag()) {
        bool store_mode = env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE);

        if (true == IsPowerButtonPressed() || PM_BR_SECONDARY == bootreason) {
            UBOOT_INFO("last screen state is off, bypass enter quiescent mode!\n");
            standby_info.qhb_mode = 0;
        } else if (PM_BR_AC == bootreason && !store_mode) {
            UBOOT_INFO("force quiescent mode bootup system for AC on!!!\n");
            standby_info.qhb_mode = 2;
        }
    } else {
        //disable qhb mode, or will no backlight when system last screen state is off.
        if (standby_info.qhb_mode == 2) {
            if (true == IsPowerButtonPressed() || PM_BR_SECONDARY == bootreason ||
                PM_BR_AC == bootreason || bootreason == PM_BR_LONG_PRESS_PWR_KEY) {
                UBOOT_INFO("last screen state is off, bypass enter quiescent mode!\n");
                standby_info.qhb_mode = 0;
            }
        }
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

    //check idme setting which has higher priority
    if (!standby_check_idme() ){
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

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
    //Only florida product support the standby behavior!!!
    if (check_specific_standby_behavior()) {
        bool store_mode = env_get("usr_flags") && (simple_strtoul(env_get("usr_flags"), NULL, 16) & USR_FLAGS_STOREDEMO_MODE);
        UBOOT_INFO("store mode = %s\n", store_mode ? "YES" : "NO");
        if (oobe_completed || store_mode) {
            if (bootreason == PM_BR_AC) {
                standby_mode_pm_status = 0;
                UBOOT_INFO("AC on bypass uboot standby bootup!!!\n");
                return standby_mode_pm_status;
            } else if (screen_is_off && (bootreason == PM_BR_PANIC ||
                    bootreason == PM_BR_WATCHDOG || bootreason == PM_BR_SW_WATCHDOG)) {
                UBOOT_INFO("abnormally reboot need force enter uboot standby when last screen is off!!!\n");
                standby_mode_pm_status = 1;
                return standby_mode_pm_status;
            }
        } else {
            //florida AC on/ reboot, will force enter uboot standby before oobe.
            if (bootreason == PM_BR_REBOOT || bootreason == PM_BR_REBOOT_SHELL || bootreason == PM_BR_REBOOT_FORCE) {
                standby_mode_pm_status = 0;
            } else if (bootreason == PM_BR_AC) {
                standby_mode_pm_status = 1;
            } else {
                standby_mode_pm_status = screen_is_off ? 1 : 0;
            }
            UBOOT_INFO("%s uboot standby...........\n", screen_is_off ? "force" : "bypass");
            return standby_mode_pm_status;
        }
    }

    if (check_bypass_ubootstandby_flag()) {
        standby_mode_pm_status = 0;
        return(standby_mode_pm_status);
    }

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
