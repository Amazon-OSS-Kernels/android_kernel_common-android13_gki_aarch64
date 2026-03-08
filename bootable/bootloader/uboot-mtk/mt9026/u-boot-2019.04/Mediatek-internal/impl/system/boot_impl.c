// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <string.h>
#include <common.h>
#include <vsprintf.h>
#include <boot_impl.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <utility.h>
#include <asm/io.h>
#include <standby_impl.h>
#include <partition.h>
#include <dm/ofnode.h>
#include <utility.h>
#if defined(CONFIG_MTK_PM)
#include <mtk-pm.h>
#endif
#if defined(CONFIG_ENABLE_QHB)
#if defined(CONFIG_DATA_SEPARATION)
#include <mtk_dataindex.h>
#endif
#include <standby_mode.h>
#include <iniutility.h>
#endif

#define CHECK_IR_OR_KEYPAD_DATA         15
#define GET_IR_OR_KEYPAD_DATA           0x7FFF

#define UPGRADE_IR_KEY_PAGE_UP          0xBB
#define UPGRADE_KEYPAD_KEY_POWER        0xAA

#if defined(CONFIG_BOOT_IR)
#define RECOVERY_IR_KEY_MENU            0xAA
#endif

#if defined(CONFIG_ENABLE_QHB)
#define FORCE_QHB_MODE_Linux_Control    4
#endif

int uboot_boot_mode = EN_BOOT_MODE_UNKNOWN;

static bool bKeypadLongPress = false;
static bool bPowerButtonPress = false;
static bool bResetButtonPress = false;

bool IsPowerButtonPressed(void)
{
    return bPowerButtonPress;
}

bool IsResetButtonPressed(void)
{
    return bResetButtonPress;
}

#if defined(CONFIG_MT58XX_SARADC)
#define KEYPAD_HOLD_VALUE               1100  // 1100 -> 1.1 seconds
#define RESET_HOLD_VALUE_MS             5000
#define RESET_HOLD_VALUE_INTER_MS       500


extern int adc_channel_single_shot(const char *name, int channel, unsigned int *data);


bool check_sar_key_press(void)
{
    unsigned int KeypadValue = 0;
    static unsigned int channel = 0;
    ofnode node;
    bool bStatus = false;
    static bool bInit = false;

    if (bInit == false) {
        node = ofnode_path("/sar@1c020a00");
        if (!ofnode_valid(node)) {
            UBOOT_DEBUG("cannot get sar dts\n");
            return bStatus;
        }

        if (ofnode_read_u32(node, "sar_keypad-ch", &channel)) {
            UBOOT_DEBUG("cannot get sar_keypad-ch\n");
            return bStatus;
        }
        bInit = true;
    }

    if(adc_channel_single_shot("sar", channel, &KeypadValue))
    {
        UBOOT_DEBUG("Keypad press adc %d\n", KeypadValue);
        bStatus = true;
    }

    return bStatus;
}

void long_press_sar_key_detect(void)
{
    unsigned int pre_time = 0;
    unsigned int cur_time = 0;
    unsigned int interval = 0;
    unsigned int tick = 0;
    UBOOT_TRACE("IN\n");

    if(check_sar_key_press() == true) {
        bPowerButtonPress = true;
        pre_time = get_timer(0);
        UBOOT_INFO("Pressing the KEYPAD_FORCEUGRADE_KEY\n");
        UBOOT_INFO("Ticking: ");
        while (interval < KEYPAD_HOLD_VALUE)
        {
            if (check_sar_key_press() != true)
            {
                UBOOT_INFO("Stop Pressing the KEYPAD_FORCEUGRADE_KEY\n");
                goto out;
            }
            cur_time = get_timer(0);
            interval = cur_time - pre_time;
            if (tick != interval/50)
            {
                tick = interval/50;
                printf("#");
            }
        }
        bKeypadLongPress = true;
    }
    else {
        UBOOT_INFO("No Keypad Press detect \n");
    }
out:
    UBOOT_TRACE("OK\n");
    return;
}

bool check_sar_reset_key_press(void)
{
    unsigned int KeypadValue = 0;
    static unsigned int channel = 0;
    ofnode node;
    bool bStatus = false;
    static bool bInit = false;
    int len;

    if (bInit == false) {
        node = ofnode_path("/sar@1c020a00");
        if (!ofnode_valid(node)) {
            UBOOT_DEBUG("cannot get sar dts\n");
            return bStatus;
        }
        if (!ofnode_get_property(node, "sar_reset-ch", &len)) {
            UBOOT_DEBUG("no property named sar_reset-ch\n");
            return bStatus;
        }
        if (ofnode_read_u32(node, "sar_reset-ch", &channel)) {
            UBOOT_DEBUG("cannot get sar_reset-ch\n");
            return bStatus;
        }
        bInit = true;
    }

    if(adc_channel_single_shot("sar", channel, &KeypadValue)) {
        UBOOT_DEBUG("reset key press adc %d\n", KeypadValue);
        bStatus = true;
    }

    return bStatus;
}

void long_press_sar_reset_key_detect(void)
{
    unsigned int pre_time = 0;
    unsigned int cur_time = 0;
    unsigned int interval = 0;
    unsigned int tick = 0;
    int ret = -1;

    UBOOT_TRACE("IN\n");
    if(check_sar_reset_key_press() == true) {
        bResetButtonPress = true;
        pre_time = get_timer(0);
        UBOOT_INFO("Pressing the RESET KEY\n");
        UBOOT_INFO("Ticking: ");
        while (interval < RESET_HOLD_VALUE_MS) {
            if (check_sar_reset_key_press() != true) {
                UBOOT_INFO("\nStop Pressing the RESET KEY\n");
                bResetButtonPress = false;
                goto out;
            }
            cur_time = get_timer(0);
            interval = cur_time - pre_time;
            if (tick != interval/RESET_HOLD_VALUE_INTER_MS) {
                tick = interval/RESET_HOLD_VALUE_INTER_MS;
                printf("#");
            }
        }
        UBOOT_INFO("\nStart factory reset due to long pressing reset\n");
        /* perform factory reset if long pressing reset key */
        ret = wipe_user_data();
        if (ret < 0) {
            UBOOT_ERROR("Failed to perform factory reset after long pressing reset\n");
            goto out;
        }
        if (run_command("reset", 0) != CMD_RET_SUCCESS) {
            UBOOT_ERROR("Failed to reset.\n");
            goto out;
        }
    } else {
        UBOOT_INFO("No RESET KEY Press detected \n");
    }
out:
    UBOOT_TRACE("OK\n");
    return;
}

#endif
static EN_BOOT_MODE get_upgrade_tigger_mode(void)
{
    EN_BOOT_MODE mode = EN_BOOT_MODE_UNKNOWN;
    volatile unsigned short reg = 0;
    UBOOT_TRACE("IN\n");

    if( bKeypadLongPress == true) {
        if (pm_get_boot_reason() == PM_BR_AC) {
            printf("Keypad Power Tigger Upgrade mode.\n");
            mode = EN_BOOT_MODE_POWERKEYLONGPRESS;
        }
    }

    reg = (unsigned short)PMU_REG_ACCESS(PMU_DUMMY_ADDR, PMU_DUMMY_OFFSET_IR_KEYPAD);
    if((reg >> CHECK_IR_OR_KEYPAD_DATA) == 0x0)        // IR
    {
        reg = reg & GET_IR_OR_KEYPAD_DATA;
        UBOOT_DEBUG("IR Key = 0x%X\n", reg);
        if(reg == UPGRADE_IR_KEY_PAGE_UP)
        {
            printf(" IR Page Up Tigger Upgrade Mode.\n");
            mode = EN_BOOT_MODE_USB_UPGRADE;
        }
    }
    else if((reg >> CHECK_IR_OR_KEYPAD_DATA) == 0x1)   // Keypad
    {
        reg = reg & GET_IR_OR_KEYPAD_DATA;
        UBOOT_DEBUG("Keypad Key = 0x%X\n", reg);
        if(reg == UPGRADE_KEYPAD_KEY_POWER)
        {
            printf(" Keypad Power Tigger Upgrade mode.\n");
            mode = EN_BOOT_MODE_USB_UPGRADE;
        }
    }

    UBOOT_TRACE("OK\n");
    return mode;
}

#if defined(CONFIG_BOOT_IR)
static EN_BOOT_MODE get_bootir_tigger_mode(void)
{
    EN_BOOT_MODE mode = EN_BOOT_MODE_UNKNOWN;
    volatile unsigned short reg = 0;

    UBOOT_TRACE("IN\n");

    reg = (unsigned short)PMU_REG_ACCESS(PMU_DUMMY_ADDR, PMU_DUMMY_OFFSET_IR_KEYPAD);
    if((reg >> CHECK_IR_OR_KEYPAD_DATA) == 0x0)
        reg = reg & GET_IR_OR_KEYPAD_DATA;
    else
        goto out;

    UBOOT_DEBUG("IR Key = 0x%X\n", reg);
    switch(reg)
    {
        case RECOVERY_IR_KEY_MENU:
            printf(" IR Menu Trigger Recovery Mode.\n");
            mode = EN_BOOT_MODE_RECOVERY;
            break;
        default:
            break;
    }

out:
    UBOOT_TRACE("OK\n");
    return mode;
}
#endif

static int get_boot_mode_from_partition(void)
{
    EN_BOOT_MODE mode = EN_BOOT_MODE_UNKNOWN;
    char cmd[CMD_BUF]={0};
    char newstr[CMD_BUF] = {0};
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    //const char bootreason_reboot[] = "androidboot.bootreason=reboot";
#if defined(CONFIG_ANDROID_AB)
    const char force_normal_boot1[] = "androidboot.force_normal_boot=1";
    const char force_normal_boot0[] = "androidboot.force_normal_boot=0";
#endif
    unsigned int device = 0, index = 0;
    int ret;
    int need_clean_misc=0;
    int snprintf_len = 0;

#if defined(CONFIG_ENABLE_QHB)
    struct standby_qhb_info standby_info;
    const char *relpath;
#if defined(CONFIG_DATA_SEPARATION)
    char filepath[FILE_PATH_SIZE],part[PART_NAME_SIZE];
#else
    char *part;
#endif
#endif

    UBOOT_TRACE("IN\n");
    del_bootargs("androidboot.quiescent",false);

    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return mode;
    }

    ret = sys_get_storage_info(device,"misc",storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        // if misc partition doesn't exist, enter fastboot mode
        return EN_BOOT_MODE_FASTBOOT;
    }

    ret = sscanf(storage_info, "%u:%x", &device, &index);
    if(ret == 0)
    {
        UBOOT_ERROR("Error: storage_info execute sscanf failure, return %d\n",ret);
        return -1;
    }

    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb load %s %u %u", device_name, device, index);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb load %s %u %u'.\n", sizeof(cmd), device_name, device, index);
        return -1;
    }

    if(run_command(cmd, 0) == CMD_RET_SUCCESS)
    {
        memset(cmd, 0, CMD_BUF);
        snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_RECOVERY_QUIESCENT_STR);
        if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb test %s = %s'.\n", sizeof(cmd), "command", BOOT_MODE_RECOVERY_QUIESCENT_STR);
            return -1;
        }

        if(run_command(cmd, 0) == CMD_RET_SUCCESS)
        {
#if defined(CONFIG_ANDROID_AB)
            add_bootargs("androidboot.force_normal_boot", (char *)force_normal_boot0, false);
#endif
            mode = EN_BOOT_MODE_RECOVERY;
            snprintf_len = snprintf(newstr,sizeof(newstr)-1, "androidboot.quiescent=%s", "1");
            if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
            {
                UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'androidboot.quiescent=%s'.\n", sizeof(cmd), "1");
                return -1;
            }

            add_bootargs("androidboot.quiescent", newstr, false);
            UBOOT_DEBUG("boot to boot-recovery,quiescent mode\n");
            need_clean_misc=1;
        }
        else
        {
            memset(cmd, 0, CMD_BUF);
            snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_QUIESCENT_STR);
            if(run_command(cmd, 0) == CMD_RET_SUCCESS)
            {
#if defined(CONFIG_ANDROID_AB)
                add_bootargs("androidboot.force_normal_boot", (char *)force_normal_boot1, false);
#endif
                mode = EN_BOOT_MODE_NORMAL;
                snprintf(newstr,sizeof(newstr)-1, "androidboot.quiescent=%s", "1");
                add_bootargs("androidboot.quiescent", newstr, false);
                UBOOT_DEBUG("boot to quiescent mode\n");
                need_clean_misc=1;
            }
            else
            {
                char *s;
                s = env_get("force_boot_recovery");
                memset(cmd, 0, CMD_BUF);
                snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_RECOVERY_STR);
                if((s != NULL && strncmp(s, "yes", strlen("yes")) == 0) || run_command(cmd, 0) == CMD_RET_SUCCESS)
                {
#if defined(CONFIG_ANDROID_AB)
                    add_bootargs("androidboot.force_normal_boot", (char *)force_normal_boot0, false);
                    UBOOT_DEBUG("boot to androidboot.force_normal_boot=0\n");
#endif
                    mode = EN_BOOT_MODE_RECOVERY;
                    UBOOT_DEBUG("boot to recovery booot mode\n");
                }
                else
                {
#if defined(CONFIG_FASTBOOT)
                    memset(cmd, 0, CMD_BUF);
                    snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_BOOTLOADER_STR);
                    if(run_command(cmd, 0) == CMD_RET_SUCCESS)
                    {
                        need_clean_misc=1;
                        mode = EN_BOOT_MODE_FASTBOOT;
                        UBOOT_DEBUG(" Boot mode = EN_BOOT_MODE_FASTBOOT \n");
                    }
                    else
                    {
                        memset(cmd, 0, CMD_BUF);
                        snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_FASTBOOTD_STR);
                        if(run_command(cmd, 0) == CMD_RET_SUCCESS)
                        {
#if defined(CONFIG_ANDROID_AB)
                            add_bootargs("androidboot.force_normal_boot", (char *)force_normal_boot0, false);
                            UBOOT_DEBUG("boot to androidboot.force_normal_boot=0\n");
#endif
                            mode = EN_BOOT_MODE_RECOVERY;
                            UBOOT_DEBUG("boot to recovery fast booot mode\n");
                        }
                        else
#endif
                        {
#if defined(CONFIG_ANDROID_AB)
                            add_bootargs("androidboot.force_normal_boot", (char *)force_normal_boot1, false);
                            UBOOT_DEBUG("boot to androidboot.force_normal_boot=1\n");
#endif
                            UBOOT_DEBUG("boot to normal mode\n");
                        }
#if defined(CONFIG_FASTBOOT)
                    }
#endif
                }
            }
        }
#if defined(CONFIG_ENABLE_QHB)
#if defined(CONFIG_DATA_SEPARATION)
        memset(part, 0, sizeof(part));
        memset(filepath, 0, sizeof(filepath));
        UBOOT_DEBUG("standby_init with data separation : STANDBY_SECTION[%s]STANDBY_KEY[%s]\n",STANDBY_SECTION,STANDBY_KEY);
        if(dataindex_get_key(filepath, FILE_PATH_SIZE, STANDBY_SECTION, STANDBY_KEY, NULL) != 0)
        {
            UBOOT_ERROR("cannot get %s:%s from dataindex file\n",STANDBY_SECTION,STANDBY_KEY);
            relpath = NULL;
        }
        else
        {
            if(dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) != 0)
            {
                UBOOT_ERROR("resolve path fail: %s \n",filepath);
                relpath = NULL;
            }
        }
#else
        part = STANDBY_PARTITION;
        relpath = STANDBY_FILE_PATH;
#endif
       // memcpy(part, "persist", sizeof("persist"));
       // relpath = "/standby_mode.ini";
        if(relpath != NULL)
        {
            memset(cmd, 0, CMD_BUF);
            snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", "boot-quiet");
            if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", sizeof(cmd), cmd);
                return -1;
            }
            if(run_command(cmd, 0) == CMD_RET_SUCCESS)
            {
                UBOOT_INFO("[QHB] Set qhb_mode to 4\n");
                ret = load_standby_qhb_info(part,relpath,&standby_info);
                if(ret < 0)
                {
                    UBOOT_ERROR("Read ini information failure.[%s][%s]\n",part,relpath);
                }
                else
                {
                    standby_info.qhb_mode = FORCE_QHB_MODE_Linux_Control;
                    ret = store_standby_qhb_info(part,relpath,&standby_info);
                    if(ret < 0)
                    {
                        UBOOT_ERROR("Write ini information failure.[%s][%s]\n",part,relpath);
                    }
                }
                need_clean_misc = 1;
            }
            else
            {
                ret = load_standby_qhb_info(part,relpath,&standby_info);
                if(ret < 0)
                {
                    UBOOT_ERROR("Read ini information failure.[%s][%s]\n",part,relpath);
                }
                else
                {
                    if (standby_info.qhb_mode == FORCE_QHB_MODE_Linux_Control)
                    {
                        UBOOT_INFO("[QHB] Set qhb_mode to 0\n");
                        standby_info.qhb_mode = 0;
                        ret = store_standby_qhb_info(part,relpath,&standby_info);
                        if(ret < 0)
                        {
                            UBOOT_ERROR("Write ini information failure.[%s][%s]\n",part,relpath);
                        }
                    }
                }
            }
        }
#endif
    }
    else
    {
        UBOOT_DEBUG("%s: '%s' fails, at %d\n", __FUNCTION__, cmd, __LINE__);
    }
    if(need_clean_misc==1)
    {
        memset(cmd, 0, CMD_BUF);
        strncpy(cmd, "bcb clear", CMD_BUF-1);
        cmd[CMD_BUF-1] = '\0';
        if(run_command(cmd, 0) == CMD_RET_SUCCESS)
        {
            memset(cmd, 0, CMD_BUF);
            snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb store %s", device_name);
            if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
            {
                UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb store %s'.\n", sizeof(cmd), device_name);
                UBOOT_ERROR("Not to write to MISC data partition.\n");
            }
            else if(run_command(cmd, 0) == CMD_RET_FAILURE)
            {
                UBOOT_ERROR("Write to MISC data partition failure.\n");
            }
        }
        else
        {
            UBOOT_ERROR("Clear MISC data failure.\n");
        }
    }
    UBOOT_TRACE("OK\n");

    return mode;
}

#define PRT_BOOTREASON(r)\
    do{\
        printf("Boot reason is %s, 0x%02X\n", #r, r); \
    }while(0)

#define PRT_WAKEUPREASON(r)\
    do{\
        printf("Wakeup from PM_BR_SECONDARY, reason is %s\n", r); \
    }while(0)

void print_bootreason(int boot_reason)
{
    switch (boot_reason) {
    case PM_BR_AC:
        PRT_BOOTREASON(PM_BR_AC);
        break;
    case PM_BR_STR_STANDBY:
        PRT_BOOTREASON(PM_BR_STR_STANDBY);
        break;
    case PM_BR_STR_SLEEP:
        PRT_BOOTREASON(PM_BR_STR_SLEEP);
        break;
    case PM_BR_STR_DSLEEP:
        PRT_BOOTREASON(PM_BR_STR_DSLEEP);
        break;
    case PM_BR_MAX_CNT:
        PRT_BOOTREASON(PM_BR_MAX_CNT);
        break;
    case PM_BR_REBOOT:
        PRT_BOOTREASON(PM_BR_REBOOT);
        break;
    case PM_BR_REBOOT_SHELL:
        PRT_BOOTREASON(PM_BR_REBOOT_SHELL);
        break;
    case PM_BR_REBOOT_FORCE:
        PRT_BOOTREASON(PM_BR_REBOOT_FORCE);
        break;
    case PM_BR_WATCHDOG:
        PRT_BOOTREASON(PM_BR_WATCHDOG);
        break;
    case PM_BR_WATCHDOG_FORCE:
        PRT_BOOTREASON(PM_BR_WATCHDOG_FORCE);
        break;
    case PM_BR_PANIC:
        PRT_BOOTREASON(PM_BR_PANIC);
        break;
    case PM_BR_PANIC_FORCE:
        PRT_BOOTREASON(PM_BR_PANIC_FORCE);
        break;
    case PM_BR_SECONDARY:
        PRT_BOOTREASON(PM_BR_SECONDARY);
        PRT_WAKEUPREASON(pm_get_wakeup_reason_str());
        break;
    case PM_BR_DC:
        PRT_BOOTREASON(PM_BR_DC);
        break;
    case PM_BR_LONG_PRESS_PWR_KEY:
        PRT_BOOTREASON(PM_BR_LONG_PRESS_PWR_KEY);
        break;
    case PM_BR_REBOOT_OVER_TEMP:
        PRT_BOOTREASON(PM_BR_REBOOT_OVER_TEMP);
        break;
    case PM_BR_SW_WATCHDOG:
        PRT_BOOTREASON(PM_BR_SW_WATCHDOG);
        break;
    default:
        UBOOT_INFO("Boot reason is 0x%02X\n", boot_reason);
    }
}

extern unsigned int g_TCONLESS_FORCE_RESET_FLAG;

static void get_bootreason(void)
{
    char *rd_buf = NULL;

    UBOOT_TRACE("IN\n");
#if defined(CONFIG_MTK_PM)
    int boot_reason;
    char bootreason_watchdog[] = "androidboot.bootreason=watchdog";
    char bootreason_kernel_panic[] = "androidboot.bootreason=kernel_panic";
    char bootreason_quiescent[] = "androidboot.bootreason=quiescent";
    char bootreason_reboot[] = "androidboot.bootreason=reboot";
    char newstr[COMMAND_BUF_SIZE] = {0};

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    char *quiescent = env_get("bootconfig");
#else
    char *quiescent = env_get("bootargs");
#endif
    boot_reason = pm_get_boot_reason();
    /* Check TCONLESS_FORCE_RESET */
    if (boot_reason == PM_BR_TCONLESS_FORCE_RESET) {
        /* restore saved boot reason and set g_TCONLESS_FORCE_RESET_FLAG */
        UBOOT_INFO("boot reason is PM_BR_TCONLESS_FORCE_RESET.\n");
        g_TCONLESS_FORCE_RESET_FLAG = 1;
        rd_buf = env_get("save_boot_reason");
        if (rd_buf) {
            boot_reason = (int)(*rd_buf);
            UBOOT_INFO("Restore the saved boot reason, 0x%X.\n", boot_reason);
            pm_set_boot_reason(boot_reason);
        }
        else {
            UBOOT_ERROR("Failed to read the saved boot reason.\n");
        }
    }

    if(quiescent != NULL) {
        if(boot_reason == PM_BR_PANIC) {
            printf("Add Android bootreason %s\n",bootreason_kernel_panic);
            add_bootargs("androidboot.bootreason", bootreason_kernel_panic,false);
        }
        else if(boot_reason == PM_BR_WATCHDOG) {
            printf("Add Android bootreason %s\n",bootreason_watchdog);
            add_bootargs("androidboot.bootreason", bootreason_watchdog,false);
        }
        else if(strstr(quiescent, "androidboot.quiescent=1")!=NULL) {
            printf("Add Android bootreason %s\n",bootreason_quiescent);
            add_bootargs("androidboot.bootreason", bootreason_quiescent,false);
        }
        else {
            printf("Add Android bootreason %s\n",bootreason_reboot);
            add_bootargs("androidboot.bootreason", bootreason_reboot,false);
        }
    }
    print_bootreason(boot_reason);
    if (boot_reason == PM_BR_SECONDARY) {
        int snprintf_len;

        snprintf_len = snprintf(newstr, sizeof(newstr), "androidboot.wakeup_reason=%s", pm_get_wakeup_reason_str());
        if (snprintf_len > COMMAND_BUF_SIZE) {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail\n", snprintf_len);
        } else {
            add_bootargs("androidboot.wakeup_reason", newstr, false);
        }
    }
#endif
    UBOOT_TRACE("OK\n");

}
int get_boot_mode_flag(void)
{
    return uboot_boot_mode;
}

int boot_mode_check(void)
{
#if defined(CONFIG_FASTBOOT)
    int ret = -1;
#endif
    int boot_mode = EN_BOOT_MODE_UNKNOWN;
    UBOOT_TRACE("IN\n");
    boot_mode = get_boot_mode_from_partition();
    switch(boot_mode)
    {
        case EN_BOOT_MODE_RECOVERY:
            uboot_boot_mode = EN_BOOT_MODE_RECOVERY;
            break;
        case EN_BOOT_MODE_NORMAL:
            uboot_boot_mode = EN_BOOT_MODE_NORMAL;
            break;
#if defined(CONFIG_FASTBOOT)
        case EN_BOOT_MODE_FASTBOOT:
            uboot_boot_mode = EN_BOOT_MODE_FASTBOOT;
            ret = run_command("fastboot usb 0", 0);
            if(ret != 0)
                UBOOT_DEBUG("fastboot usb failure\n");
            break;
#endif
        default:
            UBOOT_DEBUG("non available case\n");
            uboot_boot_mode = EN_BOOT_MODE_NORMAL;
            break;
    }
    get_bootreason();

    UBOOT_TRACE("OK\n");
    return 0;
}

int upgrade_mode_check(void)
{
    int ret = 0;
    int boot_mode = EN_BOOT_MODE_UNKNOWN;
    UBOOT_TRACE("IN\n");
    boot_mode=get_upgrade_tigger_mode();
    if (boot_mode == EN_BOOT_MODE_USB_UPGRADE)
    {
        uboot_boot_mode = EN_BOOT_MODE_USB_UPGRADE;
        ret = run_command("mtkupgrade usb ", 0);
    }
    else if (boot_mode == EN_BOOT_MODE_POWERKEYLONGPRESS)
    {
        uboot_boot_mode = EN_BOOT_MODE_USB_UPGRADE;
        UBOOT_DEBUG("Try to download the image in USB disk.\n");
        if(run_command("mtkupgrade usb ", 0))
        {
            UBOOT_DEBUG("No USB disk or no vaild image. Enter Recovery Mode.\n");
            uboot_boot_mode = EN_BOOT_MODE_RECOVERY;
            ret = run_command("setrecovery;reset", 0);
        }
    }
    UBOOT_TRACE("OK\n");
    return ret;
}

int get_kernel_start(unsigned long long *kernel_start)
{
#define MAX_PATH_LENGTH                 (32)
#define REG_SHIFT_BITS                  (32)
#define REG_ARRAY_LENGTH                 (4)
    ofnode node;
    char path[MAX_PATH_LENGTH];
    unsigned int addr[REG_ARRAY_LENGTH];

    node.np = NULL;
    if (snprintf(path, sizeof(path), "/mmap_info/MI_KERNEL_POOL%d", 1) < 0) {
        UBOOT_DEBUG("cannot get mmap kernel_pool path\n");
        return -1;
    }

    node = ofnode_path(path);
    if (!ofnode_valid(node)) {
        UBOOT_DEBUG("cannot get kernel_start\n");
        return -1;
    }

    if (ofnode_read_u32_array(node, "reg", addr, REG_ARRAY_LENGTH)) {
        UBOOT_DEBUG("cannot get address\n");
        return -1;
    }

    *kernel_start = ((unsigned long long)addr[0] << REG_SHIFT_BITS) | addr[1];
    return 0;
}

#if defined(CONFIG_BOOT_IR)
int bootir_mode_check(void)
{
    int ret = 0;
    EN_BOOT_MODE boot_mode = EN_BOOT_MODE_UNKNOWN;
    EN_BOOT_MODE flag = EN_BOOT_MODE_UNKNOWN;

    UBOOT_TRACE("IN\n");

    boot_mode = get_bootir_tigger_mode();
    flag = get_boot_mode_flag();
    if(boot_mode == EN_BOOT_MODE_RECOVERY)
    {
        if(flag == EN_BOOT_MODE_RECOVERY)
        {
            printf("Already in recovery mode\n");
        }
        else
        {
            uboot_boot_mode = EN_BOOT_MODE_RECOVERY;
            ret = run_command("setrecovery;reset", 0);
        }
    }

    UBOOT_TRACE("OK\n");
    return ret;
}
#endif

int set_recovery(void)
{
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    char cmd[COMMAND_BUF_SIZE] = {0};
    unsigned int device = 0;
    unsigned int index = 0;
    int snprintf_len = 0;
    int ret = -1;

    //load misc partition to bcb
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure\n");
        return ret;
    }

    ret = sys_get_storage_info(device, "misc", storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return ret;
    }

    ret = sscanf(storage_info, "%u:%x", &device, &index);
    if(ret == 0)
    {
        UBOOT_ERROR("Error: storage_info execute sscanf failure, return %d\n", ret);
        return -1;
    }

    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd) - 1, "bcb load %s %u %u", device_name, device, index);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd) - 1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb load %s %u %u'.\n", sizeof(cmd), device_name, device, index);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d command '%s' fail\n", __FILE__, __func__, __LINE__, cmd);
        return -1;
    }

    //set 'boot-recovery' to bcb command
    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd) - 1, "bcb set %s %s", "command", BOOT_MODE_RECOVERY_STR);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd) - 1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb set %s %s'.\n", sizeof(cmd), "command", BOOT_MODE_RECOVERY_STR);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d Set command boot-recovery fail!!\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    //store to misc partition
    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd) - 1, "bcb store %s", device_name);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd) - 1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb store %s'.\n", sizeof(cmd), device_name);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d Write to MISC data partition failure.\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    return 0;
}

int wipe_user_data(void)
{
    int ret = -1;
    char cmd[COMMAND_BUF_SIZE];
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    unsigned int index = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    int snprintf_len = 0;

    //load misc partition to bcb
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure\n");
        return ret;
    }

    ret = sys_get_storage_info(device, "misc", storage_info);
     if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return ret;
    }

    ret = sscanf(storage_info, "%u:%x", &device, &index);
    if(ret == 0)
    {
        UBOOT_ERROR("Error: storage_info execute sscanf failure, return %d\n",ret);
        return -1;
    }

    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb load %s %u %u", device_name, device, index);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb load %s %u %u'.\n", sizeof(cmd), device_name, device, index);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d command '%s' fail\n", __FILE__, __func__, __LINE__, cmd);
        return -1;
    }

    //set 'boot-recovery' to bcb command
    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb set %s %s", "command", BOOT_MODE_RECOVERY_STR);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb set %s %s'.\n", sizeof(cmd), "command", BOOT_MODE_RECOVERY_STR);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d Set command boot-recovery fail!!\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    //set 'recovery --wipe_data' to bcb recovery
    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb set %s %s", "recovery", BOOT_MODE_RECOVERY_WIPE_DATA);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb set %s %s'.\n", sizeof(cmd), "recovery", BOOT_MODE_RECOVERY_WIPE_DATA);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d Set recovery wipe_data fail!!\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    //store to MISC
    memset(cmd, 0, COMMAND_BUF_SIZE);
    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb store %s", device_name);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb store %s'.\n", sizeof(cmd), device_name);
        return -1;
    }

    if(run_command(cmd, 0) != CMD_RET_SUCCESS)
    {
        UBOOT_ERROR("%s:%s:%d Write to MISC data partition failure.\n", __FILE__, __func__, __LINE__);
        return -1;
    }

    return 0;
}

static int set_partition_data_to_zero(char *partition_name, unsigned long long addr)
{
    struct blk_desc *blk_dev_desc = NULL;
    disk_partition_t partition_info;
    char command[COMMAND_BUF_SIZE]={0}, device_name[STORAGE_DEVICE_BUF_SIZE]={0};
    unsigned int device = 0, partition_num = 0;
    unsigned long long remain_write_size = 0, pos = 0;
    int ret = -1, snprintf_len;

    memset(device_name, 0, sizeof(device_name));
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("ERROR with sys_get_boot_device execution failure. No boot device found!!\n");
        return ret;
    }

    blk_dev_desc = blk_get_dev(device_name, device);
    ret = get_partition_info(blk_dev_desc, partition_name, &partition_info, &partition_num);
    if(ret != 0)
    {
        UBOOT_ERROR("Error: %s get_partition_info failure.\n",partition_name);
        return ret;
    }
#ifndef ERASE_FIRST_4K_TO_SPEEDUP
    remain_write_size = ((unsigned long long)partition_info.size * partition_info.blksz);
#else
    remain_write_size = ((unsigned long long)CLEAN_BUFFER_SIZE);
#endif
    memset(command, 0, sizeof(command));
    while(remain_write_size != 0)
    {
#ifndef ERASE_FIRST_4K_TO_SPEEDUP
        if(remain_write_size < CLEAN_BUFFER_SIZE){
            snprintf_len = snprintf(command, sizeof(command)-1, "partition write %s 0 %s 0x%llX 0x%llX 0x%llx",device_name, partition_name, addr, remain_write_size, pos);
            if (snprintf_len < 0)
            {
                UBOOT_ERROR("snprintf fail at compose buffer for read dtbo partition\n");
                return -1;
            }
            ret = run_command(command, 0);
            pos += remain_write_size;
            remain_write_size = 0;
            UBOOT_DEBUG("pos:%llx remain_write_size:%llx\n",pos,remain_write_size);
        }else{
#endif
            snprintf_len = snprintf(command, sizeof(command)-1, "partition write %s 0 %s 0x%llX 0x%llX 0x%llx",device_name, partition_name, addr, (unsigned long long)CLEAN_BUFFER_SIZE, pos);
            if (snprintf_len < 0)
            {
                UBOOT_ERROR("snprintf fail at compose buffer for read dtbo partition\n");
                return -1;
            }
            ret = run_command(command, 0);
            pos += CLEAN_BUFFER_SIZE;
            remain_write_size -= (unsigned long long)CLEAN_BUFFER_SIZE;
            UBOOT_DEBUG("pos:%llx remain_write_size:%llx\n",pos,remain_write_size);
#ifndef ERASE_FIRST_4K_TO_SPEEDUP
        }
#endif
    }
    return 0;
}

int wipe_user_data_in_uboot(void)
{
    char *addr = NULL;
    int ret = -1;

    printf("wipe metadata and userdata\n");

    addr = (char *)CLEAN_BUFFER_ADDR;
    memset(addr, 0, CLEAN_BUFFER_SIZE);
    ret = set_partition_data_to_zero(METADATA_PARTITON_NAME, CLEAN_BUFFER_ADDR);
    if(ret != 0)
    {
        UBOOT_ERROR("Set %s partition data to all zero failure,\n",METADATA_PARTITON_NAME);
    }

    ret = set_partition_data_to_zero(USERDATA_PARTITON_NAME, CLEAN_BUFFER_ADDR);
    if(ret != 0)
    {
        UBOOT_ERROR("Set %s partition data to all zero failure,\n",USERDATA_PARTITON_NAME);
        return -1;
    }
    return 0;
}

bool is_fastboot_bootloader_mode(void)
{
    char cmd[COMMAND_BUF_SIZE];
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    unsigned int device = 0, index = 0;
    int ret, snprintf_len=0;
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return false;
    }

    ret = sys_get_storage_info(device,"misc",storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        // if misc partition doesn't exist, enter fastboot mode
        return true;
    }

    ret = sscanf(storage_info, "%u:%x", &device, &index);
    if(ret == 0)
    {
        UBOOT_ERROR("Error: storage_info execute sscanf failure, return %d\n",ret);
        return false;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb load %s %u %u", device_name, device, index);
    if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
    {
        UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb load %s %u %u'.\n", sizeof(cmd), device_name, device, index);
        return false;
    }

    if(run_command(cmd, 0) == CMD_RET_SUCCESS)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf_len = snprintf(cmd, sizeof(cmd)-1, "bcb test %s = %s", "command", BOOT_MODE_BOOTLOADER_STR);
        if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(cmd)-1))
        {
            UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'bcb test %s = %s'.\n", sizeof(cmd), "command", BOOT_MODE_BOOTLOADER_STR);
            return false;
        }
        if(run_command(cmd, 0) == CMD_RET_SUCCESS)
        {
            UBOOT_ERROR("boot mode: reboot bootloader\n");
            return true;
        }
        else
        {
            UBOOT_ERROR("boot mode: not reboot-bootloader\n");
            return false;
        }

    }else{
        UBOOT_ERROR("Error: bcb load failure\n");
        return false;
    }
    return false;
}

int is_ldm_support(void)
{
    ofnode node;
    static bool hasLdmInfo = false;
    static unsigned int ldm_support = 0;

    if (hasLdmInfo == false) {
        node = ofnode_path("/mediatek-ldm");
        if (!ofnode_valid(node)) {
            UBOOT_DEBUG("Cannot get dts /mediatek-ldm\n");
            return -1;
        }
        if (ofnode_read_u32(node, "LDM_SUPPORT", &ldm_support)) {
            UBOOT_DEBUG("Cannot get LDM_SUPPORT\n");
            return -1;
        }
        hasLdmInfo = true;
    }
    return ldm_support;
}
