// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <fs.h>
//#include <drvSYS.h>
#include <environment.h>
#include <boot_impl.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <utility.h>
#include <asm/io.h>
#include <partition.h>
#include <ext4_bin2array.h>
#include <iniutility.h>
#include <standby_mode.h>
#include <mtk-pm.h>
#include <../lib/libavb/libavb.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <android_ab.h>
#ifndef DDI_SUPPORT
#if defined(CONFIG_ENABLE_QHB)
#include <drvEEPROM.h>
#endif
#endif
#include <mtk_ab.h>
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif



#if defined(CONFIG_ROLLBACK_INDEX_IN_RPMB)
#include <program_rpmb_rollback_index.h>
#endif

#if defined(CONFIG_ROLLBACK_INDEX_IN_EFUSE)
#include <program_efuse_rollback_index.h>
#endif
#include <ext4fs.h>

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define SCRIPT_FILE_COMMENT         '#'                 // symbol for comment
#define SCRIPT_FILE_END             '%'                 // symbol for file end
#define SCRIPT_BUF_SIZE             4096
#define BOOTARGS_ITEM_MAX_LEN       192
#define TEMP_BUFFER_SIZE            64
#define UENV_PARTITION_NAME         "uenv"              // Partition name must be 'uenv'
#define EXT4_BIN_SIZE               0x800000            // Size of uenv partition must be 8MB
#define EXTRA_LEN                   3                   // For del_cfg not using magic number
#define EXTRA_ADD_LEN               2                   // For add_bootargs not using magic number
#define BOOT_DEVICE_OFFSET          0x040480
#define BOOT_DEVICE_SHIFT           4
#define BOOT_DEVICE_SHIFT           4
#define NUM_TWO         0x2
#define NUM_THREE       0x3
#define RAND_GENERATE_LOOP          0x200

int uboot_boot_device = EN_BOOT_DEVICE_UNKNOWN;

static unsigned int offset = TIMESTAMP_OFFSET;
static int g_need_add_retry_count = 1;
//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static int del_cfg(char *source, char *delCfg)
{
    char *substr = NULL;
    char *pPreEnvEnd = NULL;
    char *NewDelCfg = NULL;
    int NewDelCfgLen = 0;
    int snprintf_len = 0;
    UBOOT_TRACE("IN\n");
    if (source == NULL)
    {
        UBOOT_ERROR("The input parameter 'source' is a null pointer\n");
        return -1;
    }

    if (delCfg == NULL)
    {
        UBOOT_ERROR("The input parameter 'delCfg' is a null pointer\n");
        return -1;
    }

    if (strlen(source) == 0)
    {
        UBOOT_ERROR("The length of source is zero\n");
        return -1;
    }

    if (strlen(delCfg) == 0)
    {
        UBOOT_ERROR("The length of delCfg is zero\n");
        return -1;
    }

    NewDelCfgLen = strlen(delCfg) + EXTRA_LEN;
    NewDelCfg = (char *)malloc(NewDelCfgLen);
    if (NewDelCfg == NULL)
    {
        UBOOT_ERROR("malloc for NewDelCfg fail\n");
        return -1;
    }
    memset(NewDelCfg, 0, NewDelCfgLen);
    snprintf_len = snprintf(NewDelCfg, NewDelCfgLen, "%c%s%c", ' ', delCfg, '=');
    if (snprintf_len < 0 || snprintf_len >= NewDelCfgLen)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", NewDelCfgLen, NewDelCfg);
        free(NewDelCfg);
        return -1;
    }

    substr = strstr(source, NewDelCfg);
    if (substr == NULL)
    {
        // check "delcfg=" is first bootargs or not.
        memset(NewDelCfg, 0, NewDelCfgLen);
        snprintf_len = snprintf(NewDelCfg, NewDelCfgLen, "%s%c", delCfg, '=');
        if (snprintf_len < 0 || snprintf_len >= NewDelCfgLen)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", NewDelCfgLen, NewDelCfg);
            free(NewDelCfg);
            return -1;
        }
        substr = strstr(source, NewDelCfg);
        if ((substr == NULL) || (strncmp(source, substr, strlen(source)) != 0))
        {
            free(NewDelCfg);
            return 1;
        }
        else
        {
            pPreEnvEnd = strchr(substr, ' ');
            if (pPreEnvEnd != NULL)
            {
                if (((unsigned long)pPreEnvEnd - (unsigned long)source + 1) < strlen(source))
                {
                    strncpy(substr, pPreEnvEnd + 1, strlen(pPreEnvEnd));  // +1 is for skip space
                }
            }
            else
            {
                UBOOT_DEBUG("This member is the only one in the bootargs\n");
                *(substr) = '\0';     // clear the rst of size.
                memset(substr, 0, strlen(substr));
            }
        }
    }
    else        // " delcfg=" match in bootargs.
    {
        pPreEnvEnd = strchr(substr + 1, ' ');    // +1 is for skip first space
        if (pPreEnvEnd != NULL)
        {
            if (((unsigned long)pPreEnvEnd - (unsigned long)source + 1) < strlen(source))
            {
                strncpy(substr, pPreEnvEnd, strlen(pPreEnvEnd) + 1);
            }
        }
        else
        {
            UBOOT_DEBUG("This member is the last one in the bootargs\n");
            *(substr) = '\0';   //clear the rst of size.
            memset(substr, 0, strlen(substr));
        }
    }
    free(NewDelCfg);
    UBOOT_TRACE("OK\n");
    return 0;
}

int add_bootargs(char *key, char *set_cfg, bool save)
{
    char *preCheck = NULL;
    char *bootargs = NULL;
    char *OriArg = NULL;
    char *NewArg = NULL;
    int  NewArgLen = 0;
    char setCfg[BOOTARGS_ITEM_MAX_LEN + 1];
    char scanBuf[BOOTARGS_ITEM_MAX_LEN + 1];
    int  snprintf_len = 0;
    int setCfgLen = 0;
    char *pNextChar = NULL;
    char *NewSetCfg = NULL;
    int NewSetCfgLen = 0;
    UBOOT_TRACE("IN\n");

    if (key == NULL)
    {
        UBOOT_ERROR("The input parameter 'key' is a null pointer\n");
        return -1;
    }

    if (set_cfg == NULL)
    {
        UBOOT_ERROR("The input parameter 'set_cfg' is a null pointer\n");
        return -1;
    }

    // Format the set_cfg
    memset(setCfg,  0, BOOTARGS_ITEM_MAX_LEN + 1);
    memset(scanBuf, 0, BOOTARGS_ITEM_MAX_LEN + 1);
    strncpy(scanBuf, set_cfg, BOOTARGS_ITEM_MAX_LEN);
    strncpy(setCfg, set_cfg, BOOTARGS_ITEM_MAX_LEN);
    setCfg[BOOTARGS_ITEM_MAX_LEN] = '\0';
    UBOOT_DEBUG("setCfg: %s\n", setCfg);

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    if (strstr(key, "androidboot") != NULL)
        bootargs = env_get("bootconfig");
    else //get origin bootargs
#endif
        bootargs = env_get("bootargs");
    if (bootargs == NULL)
    {
        UBOOT_INFO("Warnning!!!!\nbootargs doesn't exist, add a new one!!\n");
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
        if (strstr(key, "androidboot") != NULL)
            env_set("bootconfig", setCfg);
        else
#endif
            env_set("bootargs", setCfg);
        env_save();
        return 0;
    }

    if (strncmp(setCfg, bootargs, strlen(bootargs)) == 0)     // setcfg is the only one bootargs.
    {
        UBOOT_DEBUG("%s is the only one bootargs\n", setCfg);
        UBOOT_TRACE("OK\n");
        return 0;
    }

    NewSetCfgLen = strlen(setCfg) + EXTRA_LEN;
    NewSetCfg = (char *)malloc(NewSetCfgLen);
    if (NewSetCfg == NULL)
    {
        UBOOT_ERROR("malloc for NewSetCfg fail\n");
        return -1;
    }

    memset(NewSetCfg, 0, NewSetCfgLen);
    snprintf_len = snprintf(NewSetCfg, NewSetCfgLen, "%c%s%c", ' ', setCfg, ' ');
    if (snprintf_len < 0 || snprintf_len >= NewSetCfgLen)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", NewSetCfgLen, NewSetCfg);
        free(NewSetCfg);
        return -1;
    }

    preCheck = strstr(bootargs, NewSetCfg);
    setCfgLen = strlen(setCfg);
    if (preCheck != 0)                    // setcfg match in bootargs, but neither the first one nor last one.
    {
        free(NewSetCfg);
        UBOOT_DEBUG("%s has already existed\n", setCfg);
        UBOOT_TRACE("OK\n");
        return 0;
    }
    else
    {
        preCheck = strstr(bootargs, setCfg);
        if (preCheck != 0)               // check the first one and last one bootargs.
        {
            if (strncmp(preCheck, bootargs, strlen(bootargs)) == 0) // setcfg is the first one bootargs.
            {
                // check next char behind of setcfg
                pNextChar = preCheck + setCfgLen;
                if ((*pNextChar == '\0') || (*pNextChar == ' '))
                {
                    free(NewSetCfg);
                    UBOOT_DEBUG("%s has already existed(the first one).\n", setCfg);
                    UBOOT_TRACE("OK\n");
                    return 0;
                }
            }
            else                                                   // check the last one bootargs.
            {
                memset(NewSetCfg, 0, NewSetCfgLen);
                snprintf_len = snprintf(NewSetCfg, NewSetCfgLen, "%c%s", ' ', setCfg);
                if (snprintf_len < 0 || snprintf_len >= NewSetCfgLen)
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", NewSetCfgLen, NewSetCfg);
                    free(NewSetCfg);
                    return -1;
                }
                preCheck = strstr(bootargs, NewSetCfg);
                pNextChar = preCheck + setCfgLen + 1;      // +1 is for skip space
                // check next char behind of setcfg
                if ((preCheck != 0) && (*pNextChar == '\0'))
                {
                    free(NewSetCfg);
                    UBOOT_DEBUG("%s has already existed(the last one).\n", setCfg);
                    UBOOT_TRACE("OK\n");
                    return 0;
                }
            }
        }
    }
    free(NewSetCfg);

    OriArg = (char*)malloc(strlen(bootargs) + 1);
    if (OriArg == NULL)
    {
        UBOOT_ERROR("malloc for OriArg fail\n");
        return -1;
    }
    strncpy(OriArg, bootargs, strlen(bootargs));
    OriArg[strlen(bootargs)] = '\0';

    UBOOT_DEBUG("OriArg: %s\n", OriArg);
    //if setCfg exist, delete it.
    del_cfg(OriArg, key);
    //add the NewCfg to Arg's tail.
    if (strlen(OriArg) == 0)
        NewArgLen = strlen(setCfg) + 1;
    else
        NewArgLen = strlen(OriArg) + strlen(setCfg) + EXTRA_ADD_LEN;
    if (NewArgLen >= CONFIG_SYS_CBSIZE)
    {
        UBOOT_DEBUG("The length of OriArg : %u\n", (unsigned int)strlen(OriArg));
        UBOOT_DEBUG("The length of setCfg : %u\n", (unsigned int)strlen(setCfg));
        UBOOT_ERROR("New bootargs( NewArgLen : %d ) has exceeded the env_size( CONFIG_SYS_CBSIZE :%d )\n",
                     NewArgLen, CONFIG_SYS_CBSIZE);
        UBOOT_INFO("set_bootargs_cfg failed !\n");
        free(OriArg);
        return -1;
    }
    NewArg = malloc(NewArgLen);

    if (NewArg == NULL)
    {
        free(OriArg);
        UBOOT_ERROR("malloc for NewArg fail\n");
        return -1;
    }
    memset(NewArg, 0, NewArgLen);
    if (strlen(OriArg) == 0)             // setcfg is the only one bootargs
        snprintf_len = snprintf(NewArg, NewArgLen, "%s", setCfg);
    else
        snprintf_len = snprintf(NewArg, NewArgLen, "%s %s", OriArg, setCfg);
    if (snprintf_len < 0 || (unsigned)snprintf_len >= NewArgLen)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s %s'.\n", NewArgLen, OriArg, setCfg);
        free(OriArg);
        free(NewArg);
        return -1;
    }

    UBOOT_DEBUG("NewArg: %s\n", NewArg);
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    if (strstr(key, "androidboot") != NULL)
        env_set("bootconfig", NewArg);
    else
#endif
        env_set("bootargs", NewArg);
    free(OriArg);
    free(NewArg);

    if (1 == save)
    {
        env_save();
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

int find_bootargs(char *key)
{
    int ret = 0;
    char *bootarg = NULL;
    UBOOT_TRACE("IN\n");
    if (key == NULL)
    {
        UBOOT_ERROR("The input parameter 'key' is a null pointer\n");
        return ret;
    }
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    if (strstr(key, "androidboot") != NULL)
        bootarg = env_get("bootconfig");
    else
#endif
        bootarg = env_get("bootargs");
    if (bootarg == NULL)
    {
        UBOOT_ERROR("No env 'bootargs'\n");
        return ret;
    }
    if (strstr(bootarg, key))
	    ret = 1;
    return ret;
}

int del_bootargs(char *key, bool save)
{
    int ret = 0;
    char *bootarg = NULL;
    char *OriArg = NULL;
    UBOOT_TRACE("IN\n");
    if (key == NULL)
    {
        UBOOT_ERROR("The input parameter 'key' is a null pointer\n");
        return -1;
    }

#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    if (strstr(key, "androidboot") != NULL)
        bootarg = env_get("bootconfig");
    else
#endif
        bootarg = env_get("bootargs");
    if (bootarg == NULL)
    {
        UBOOT_ERROR("No env 'bootargs'\n");
        return -1;
    }

    OriArg = malloc(strlen(bootarg) + 1);
    if (OriArg == NULL)
    {
        UBOOT_ERROR("malloc for tempBuf fail\n");
        return -1;
    }
    UBOOT_DEBUG("delete [%s] in bootargs\n", key);
    memset(OriArg, 0, strlen(bootarg) + 1);
    strncpy(OriArg, bootarg, strlen(bootarg));
    OriArg[strlen(bootarg)] = '\0';

    UBOOT_DEBUG("OriArg: %s\n", OriArg);
    ret = del_cfg(OriArg, key);
    if (ret == 0)
    {
        UBOOT_DEBUG("delete [%s] in bootargs success\n", key);
        UBOOT_DEBUG("NewArg: %s\n", OriArg);
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
        if (strstr(key, "androidboot") != NULL)
            env_set("bootconfig", OriArg);
        else
#endif
            env_set("bootargs", OriArg);
        if (1 == save)
        {
            env_save();
        }
        UBOOT_TRACE("OK\n");
    }
    else if (ret == 1)
    {
        UBOOT_DEBUG("No [%s] in bootargs\n", key);
    }
    else
    {
        UBOOT_ERROR("delete %s fail\n", key);
    }
    free(OriArg);

    return ret;
}

char* strrps(char* source,char* findstr, char *repstr)
{
    int i= 0;
    int Replenth = 0;
    char *tmp = NULL;
    char *substr = NULL;

    substr=strstr(source,findstr);
    if(substr == NULL)
        return NULL;

    {
        size_t len = strlen(substr);
        tmp=(char *)malloc(len+1);
        if(tmp == NULL)
            return NULL;
        memset(tmp, 0, len+1);
        strncpy(tmp,substr+strlen(findstr), len);
        Replenth = strlen(repstr);
        for(i=0 ; i < Replenth; i++)
        {
            substr[i]=repstr[i];
        }
        substr[Replenth]='\0';
        strncat(substr,tmp, len-strlen(findstr));
        free(tmp);
    }

    return source;
}

char* para_replace(char *str,int argc,char * const argv[])
{
    int index = 0;
    static char tmpstr[SCRIPT_BUF_SIZE];
    char *strptr = NULL;
    char findstr[3];
    findstr[0]='$';
    findstr[2]='\0';
    memset(tmpstr,0,sizeof(tmpstr));
    strncpy(tmpstr,str, SCRIPT_BUF_SIZE-1);
    tmpstr[SCRIPT_BUF_SIZE-1] = '\0';
    strptr = tmpstr;
    while((strptr=strstr(strptr,"$"))!= NULL)
    {
        if( *(strptr+1)>'0' && *(strptr+1) < '9') //check the parameter is between 0 to 9.
        {
            index = *(strptr+1) - '0' ;
            if(index > argc)
            {
                UBOOT_ERROR("Error: Parameters is bigger than argc!\n");
                return NULL;
            }
            else
            {
                findstr[1]=*(strptr+1);
                strrps(tmpstr,findstr,argv[index]);
            }
        }
        else
        {
            strptr=strptr+1;
        }
    }
    return tmpstr;
}
#define NORMAL_QHB_MODE_Android_Control      1
#define FORCE_QHB_MODE_Android_Control       2
#define NORMAL_QHB_MODE_Linux_Control        3
#define FORCE_QHB_MODE_Linux_Control         4

#define QHB_MODE_Linux_Control               2


/* Interrupted OTA is only applicable to non-AB */
#ifndef CONFIG_ANDROID_AB
bool is_qhb_interrupted_ota(void)
{
    char *qhb_recovery = env_get("qhb_recovery");
    int qhb = 0;
    if (!qhb_recovery) {
        return false;
    }

    qhb = simple_strtol(qhb_recovery, NULL, 10);

    /* QHB OTA was interrupted when recovery mode and env flag set */
    if (is_recovery_mode()) {
        return qhb == 1;
    }

    /* Always clear the flag is we are not going to recovery */
    if (qhb == 1) {
        env_set("qhb_recovery", "0");
        env_save();
    }

    return false;
}
#else
static inline bool is_qhb_interrupted_ota(void) { return false; }
#endif


int is_qhb_boot_mode(void)
{
    int flag, standby;
#ifndef DDI_SUPPORT
#if defined(CONFIG_ENABLE_QHB)
    DTVCFG_T rDtvCfg;
    MS_BOOL fgStateOn;
    int i4Ret = 0;
#endif
#endif

    /* Check if we are interrupted OTA with screen off */
    if(is_qhb_interrupted_ota())
    {
        printf("QHB: OTA Interrupted with screen off\n");
        return 1;
    }


    if(pm_get_boot_reason()==PM_BR_REBOOT_SHELL)
    {
        printf("QHB boot_reason:reboot shell Normal boot  \n");
        return 0;
    }
    if(pm_get_boot_reason()==PM_BR_MAX_CNT)
    {
        printf("QHB boot_reason:max count Normal boot  \n");
        return 0;
    }
    if(pm_get_boot_reason()==PM_BR_RECOVERY_QUIESCENT)
    {
        static bool set_done = false;

        if (!set_done) {
            env_set("qhb_recovery", "1");
            env_save();
            set_done = true;
        }

        printf("QHB boot_reason:reboot recovery quiescent Normal boot  \n");

        return 1;
    }

    flag = standby_mode_get_mode();
    standby= standby_mode_is_enter_standby();
    UBOOT_DEBUG("qhb_mode=%d , enter_standby=%d\n", flag, standby);

    if((flag == FORCE_QHB_MODE_Android_Control) || (flag == NORMAL_QHB_MODE_Android_Control && standby == 1))
    {
        return 1;
    }

#ifndef DDI_SUPPORT
#if defined(CONFIG_ENABLE_QHB)
    memset(&rDtvCfg, 0, sizeof(rDtvCfg));
    i4Ret = EEPDTV_GetCfg(&rDtvCfg);
    if (i4Ret)
    {
        UBOOT_ERROR("EEPDTV_GetCfg fail.\n");
        return i4Ret;
    }
    fgStateOn = rDtvCfg.u1Flags & DTVCFG_FLAG_SYSTEM_STATE_ON ? TRUE : FALSE;
    UBOOT_DEBUG("flag:%d  fgStateOn:%d  bootreason:%d ->AC:%d\n",flag,fgStateOn,pm_get_boot_reason(),PM_BR_AC);
    if((flag == FORCE_QHB_MODE_Linux_Control) || (flag == NORMAL_QHB_MODE_Linux_Control && fgStateOn == FALSE && pm_get_boot_reason()==PM_BR_AC))
    {
        UBOOT_INFO("[QHB] Prepare to enter the QHB. -> qhb_mode:%d\n",flag);
        rDtvCfg.u1Flags4 |= (unsigned char)DTVCFG_FLAG4_QUIET_HOT_BOOT;
        EEPDTV_SetCfg(&rDtvCfg);
        return QHB_MODE_Linux_Control;
    }
    else
    {
        UBOOT_INFO("[QHB] prepare to CLEAR QHB\n");
        rDtvCfg.u1Flags4 &= (unsigned char)~DTVCFG_FLAG4_QUIET_HOT_BOOT;
        EEPDTV_SetCfg(&rDtvCfg);
    }
#endif
#endif

    return 0;
}

int pm_check_back_ground_active(void)
{
    int bActive = 0;
#if (CONFIG_MTK_ANDROID_HEADER_VERSION > 3)
    char *quiescent = env_get("bootconfig");
#else
    char *quiescent = env_get("bootargs");
#endif
    if(quiescent == NULL)
    {
        printf("bootargs is NULL, Active Standby Mode FALSE =====\n");
    }
    else if(strstr(quiescent, "androidboot.quiescent=1"))
    {
        printf("Quiescent mode prepare to disable backlight !!!!!\n");
        bActive = 1;
    }

#if defined(CONFIG_ENABLE_QHB)
    if (is_qhb_boot_mode() != 0)
#else
    if (is_qhb_boot_mode() == 1)
#endif
    {
        printf("Active Standby Mode TRUE: QHB disable backlight  =====\n");
        bActive = 1;
    }
    else
    {
        UBOOT_DEBUG("QHB Active Standby Mode FALSE =====\n");
    }

    return bActive;
}

int sys_get_boot_device(char * boot_device, unsigned int boot_device_len)
{
    int snprintf_len = 0;

#if defined(CONFIG_TARGET_MT5888)
    snprintf(boot_device, (boot_device_len-1), "%s","mmc");
    UBOOT_INFO(" boot device is %s \n",boot_device);
    return 1;
#endif
    unsigned char reg=readb(RIU_BUS_ADDR+BOOT_DEVICE_OFFSET)>>BOOT_DEVICE_SHIFT; //<--to do
    if(boot_device==NULL)
    {
        uboot_boot_device=EN_BOOT_DEVICE_UNKNOWN;
        UBOOT_INFO(" boot device is NULL \n");
        return -1;
    }
    if(reg==0x1)
    {
        snprintf_len = snprintf(boot_device, (boot_device_len-1), "%s","usb");
        if(snprintf_len < 0 || (unsigned)snprintf_len >= (boot_device_len-1))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'.\n", (boot_device_len-1), "usb");
            return -1;
        }
        uboot_boot_device=EN_BOOT_DEVICE_USB;
        UBOOT_INFO(" boot device is %s \n",boot_device);
        return 1;
    }
    else if(reg==NUM_TWO)
    {
        uboot_boot_device=EN_BOOT_DEVICE_EMMC;
        snprintf_len = snprintf(boot_device, (boot_device_len-1), "%s","mmc");
        if(snprintf_len < 0 || (unsigned)snprintf_len >= (boot_device_len-1))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'.\n", (boot_device_len-1), "mmc");
            return -1;
        }
        UBOOT_INFO(" boot device is %s \n",boot_device);
        return 1;
    }
    else if(reg==NUM_THREE)
    {
        uboot_boot_device=EN_BOOT_DEVICE_UFS;
        snprintf_len = snprintf(boot_device, (boot_device_len-1), "%s","ufs");
        if(snprintf_len < 0 || (unsigned)snprintf_len >= (boot_device_len-1))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'.\n", (boot_device_len-1), "ufs");
            return -1;
        }
        UBOOT_INFO(" boot device is %s \n",boot_device);
        return 1;
    }
    else
    {
        return -1;
    }
}
int get_boot_device(void)
{

#if defined(CONFIG_TARGET_MT5888)
    uboot_boot_device=EN_BOOT_DEVICE_EMMC;
#endif
    unsigned char reg=readb(RIU_BUS_ADDR+BOOT_DEVICE_OFFSET)>>BOOT_DEVICE_SHIFT; //<--to do
    if(reg==0x1)
    {
        uboot_boot_device=EN_BOOT_DEVICE_USB;
    }
    else if(reg==NUM_TWO)
    {
        uboot_boot_device=EN_BOOT_DEVICE_EMMC;
    }
    else if(reg==NUM_THREE)
    {
        uboot_boot_device=EN_BOOT_DEVICE_UFS;
    }
    else
    {
        uboot_boot_device=EN_BOOT_DEVICE_UNKNOWN;
    }
    return uboot_boot_device;
}

#define RETRY_COUNT_ADDR  (RIU_BUS_ADDR|0x040400)//<--to do
#define RETRY_COUNT_MASK  (0x7)

void sys_add_retry_count(void)
{
#if defined(CONFIG_TARGET_MT5888)
    printf(" MT5888 unsupport !!! \n");
#else
    unsigned char reg=readb(RETRY_COUNT_ADDR);

    if ((reg & RETRY_COUNT_MASK) == RETRY_COUNT_MASK)
    {
        printf("retry_count reached maximum: %d\n", RETRY_COUNT_MASK);
    }
    else
    {
        printf("retry_count: %d \n", reg & RETRY_COUNT_MASK);
        writeb(reg+1,RETRY_COUNT_ADDR);
    }
#endif

}
void sys_show_retry_count(void)
{
#if defined(CONFIG_TARGET_MT5888)
    printf(" MT5888 unsupport !!! \n");
#else
#if (CONFIG_AB_SIDELOAD == 1)
    if (get_ab_selection_result() == PARTITION_SELECT_A)
        printf("  Slot A ");
    else if (get_ab_selection_result() == PARTITION_SELECT_B)
        printf("  Slot B ");
    else if (get_ab_selection_result() == PARTITION_SELECT_MAX)
        printf("  Slot Not Support !! \n");
#endif
    unsigned char reg=readb(RETRY_COUNT_ADDR) & RETRY_COUNT_MASK;
    printf("retry_count: %d \n",reg);
#endif

}

void sys_set_retry_count(unsigned int retry_count)
{
#if defined(CONFIG_TARGET_MT5888)
    printf(" MT5888 unsupport !!! \n");
#else
    unsigned char reg=readb(RETRY_COUNT_ADDR);

    if (retry_count > RETRY_COUNT_MASK)
    {
        printf("invalid retry_count: %d\n", retry_count);
    }
    else
    {
        printf("set retry_count: %d\n", retry_count);
        writeb((reg&(~RETRY_COUNT_MASK))|retry_count, RETRY_COUNT_ADDR);
    }
#endif

}

int sys_get_storage_info(unsigned int device, const char *partition_name, char *storage_info)
{
    struct blk_desc *blk_dev_desc = NULL;
    unsigned int partition_num;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    int ret;

    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_DEBUG("Error: sys_get_boot_device failure.\n");
        return -1;
    }

    blk_dev_desc = blk_get_dev(device_name, device);
    ret = get_partition_info(blk_dev_desc, partition_name, NULL, &partition_num);
    if(ret != 0)
    {
        UBOOT_DEBUG("Error: get_partition_info failure.\n");
        return -1;
    }

    ret = snprintf(storage_info, (size_t)STORAGE_INDEX_BUF_SIZE, "%u:%x", device, partition_num);
    if(ret <= 0)
        UBOOT_DEBUG("Error: storage_info fill data failure.\n");

    return 0;
}

// TODO: add compile flag to disable this API, when uboot env will not apply by mtk turnkey.
int format_uenv_to_ext4(const char *str_interface_type, const char *str_device_number)
{
	char buffer[CMD_BUF];
	int snprintf_len;
	char *ext4_8mb_binary = NULL;

	/* Initialize ext4_8mb_binary buffer */
	ext4_8mb_binary = (char *)malloc(EXT4_BIN_SIZE);
	if(ext4_8mb_binary == NULL)
	{
		UBOOT_ERROR("[ERROR] Out of memory\n");
		return CMD_RET_FAILURE;
	}
	memset(ext4_8mb_binary, 0 , EXT4_BIN_SIZE);


	/* Copy EXT4 data to ext4_8mb_binary */
	memcpy(ext4_8mb_binary+0x400   , ext4_8mb_400_5ff_binary      , sizeof(ext4_8mb_400_5ff_binary));         // 1. 0x400 ~ 0x5ff
	memcpy(ext4_8mb_binary+0x1000  , ext4_8mb_1000_101f_binary    , sizeof(ext4_8mb_1000_101f_binary));       // 2. 0x1000 ~ 0x101f
	memcpy(ext4_8mb_binary+0x9000  , ext4_8mb_9000_9fff_binary    , sizeof(ext4_8mb_9000_9fff_binary));       // 3. 0x9000 ~ 0x9fff
	memcpy(ext4_8mb_binary+0xa000  , ext4_8mb_a000_a00f_binary    , sizeof(ext4_8mb_a000_a00f_binary));       // 4. 0xa000 ~ 0xa00f
	memcpy(ext4_8mb_binary+0xb100  , ext4_8mb_b100_b13f_binary    , sizeof(ext4_8mb_b100_b13f_binary));       // 5. 0xb100 ~ 0xb13f
	memcpy(ext4_8mb_binary+0xb600  , ext4_8mb_b600_b73f_binary    , sizeof(ext4_8mb_b600_b73f_binary));       // 6. 0xb600 ~ 0xb73f
	memcpy(ext4_8mb_binary+0xba00  , ext4_8mb_ba00_ba3f_binary    , sizeof(ext4_8mb_ba00_ba3f_binary));       // 7. 0xba00 ~ 0xba3f
	memcpy(ext4_8mb_binary+0x2b000 , ext4_8mb_2b000_2b04f_binary  , sizeof(ext4_8mb_2b000_2b04f_binary));     // 8. 0x2b000 ~ 0x2b04f
	memcpy(ext4_8mb_binary+0x42b000, ext4_8mb_42b000_42b01f_binary, sizeof(ext4_8mb_42b000_42b01f_binary));   // 9. 0x42b000 ~ 0x42b01f
	memcpy(ext4_8mb_binary+0x42c000, ext4_8mb_42c000_42c02f_binary, sizeof(ext4_8mb_42c000_42c02f_binary));   // 10. 0x42c000 ~ 0x42c02f
	memcpy(ext4_8mb_binary+0x42d000, ext4_8mb_42d000_42d01f_binary, sizeof(ext4_8mb_42d000_42d01f_binary));   // 11. 0x42d000 ~ 0x42d01f


	/* Write ext4_8mb_binary to flash */
	memset(buffer, 0 , CMD_BUF);
	snprintf_len = snprintf(buffer, CMD_BUF, "partition write %s %s %s %p 0x%x", str_interface_type, str_device_number, UENV_PARTITION_NAME, ext4_8mb_binary, EXT4_BIN_SIZE);
	if (snprintf_len >= CMD_BUF)
	{
		UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
		free(ext4_8mb_binary);
		return CMD_RET_FAILURE;
	}
	UBOOT_DEBUG("cmd=%s\n",buffer);
	if(run_command(buffer,0) != CMD_RET_SUCCESS)
	{
		UBOOT_ERROR("[ERROR] Write 0x%x bytes EXT4 data to %s partition fail !!\n", EXT4_BIN_SIZE, UENV_PARTITION_NAME);
		free(ext4_8mb_binary);
		return CMD_RET_FAILURE;
	}

	UBOOT_DEBUG("Convert 0x%x bytes %s partition to EXT4 format success!!\n", EXT4_BIN_SIZE, UENV_PARTITION_NAME);
	free(ext4_8mb_binary);

	return CMD_RET_SUCCESS;
}

int is_recovery_mode(void)
{
#if !defined(CONFIG_ANDROID_AVB_PURELINUX_ENABLE)
    char *s;
    int flag;
    s = env_get("force_boot_recovery");
    if(s != NULL)
    {
        if(strncmp(s, "yes", strlen("yes")) == 0)
        {
            UBOOT_INFO("#### recovery mode enable\n");
            return 1;
        }
    }

    flag = get_boot_mode_flag();
    if(flag == EN_BOOT_MODE_RECOVERY)
    {
        UBOOT_INFO("#### recovery mode enable\n");
        return 1;
    }
#endif
    return 0;
}

int generate_random_number(unsigned char *target, int size)
{
    int i = 0, counter = 0;
    unsigned short *random_num;
    unsigned char *ptr;

    if(size < NUM_TWO)
    {
        UBOOT_ERROR("Input size argument < 2, please check target size.\n");
        return -1;
    }

    random_num = (unsigned short *)malloc(size);
    if(random_num == NULL)
    {
        UBOOT_ERROR("System allocate random_num buffer failure.\n");
        return -1;
    }

    memset(random_num, 0, size);
#if !defined(CONFIG_TARGET_MT5888)
    while(i < (size/sizeof(short)))
    {
        counter = 0;
        CPU_REGISTER_ACCESS(RANDOM_BANK_ADDR,RAND_BANK_OFFSET) = 0x1;
        while(!(CPU_REGISTER_ACCESS(RANDOM_BANK_ADDR,RAND_BANK_OFFSET) & 0x1))
        {
            counter++;
            if(counter > RAND_GENERATE_LOOP){
                UBOOT_ERROR("Try to get random value timeout!!!\n");
                break;
            }
        }
        random_num[i] = CPU_REGISTER_ACCESS(RANDOM_BANK_ADDR,NUM_TWO);
        i++;
    }
#endif

    ptr = (unsigned char *)random_num;
    memcpy(target, ptr, size);
    free(random_num);
    return 0;
}

void time_stamp_store_to_register(uint64_t time, int count)
{
    int i = 0;
    unsigned short temp = 0;

    if(offset > TIMESTAMP_OFFSET_MAX)
        return;

    for(i=0;i<count;i++)
    {
        temp = (unsigned short)(time & TIMESTAMP_MASK);
        CPU_REGISTER_ACCESS(TIMESTAMP_BANK_ADDR,offset) = temp;
        time = (time >> TIMESTAMP_SHIFT);
        offset = offset + 1;
    }
}

unsigned char *read_storage_file_to_memory(const char *partition, const char *path, loff_t *size)
{
    int ret;
    unsigned char *buffer;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    static loff_t len = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    UBOOT_TRACE("IN\n");
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return NULL;
    }
    else
        UBOOT_DEBUG("device_name = %s\n", device_name);

    ret = sys_get_storage_info(device,partition,storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        return NULL;
    }
    else
        UBOOT_DEBUG("device = %d, storage_info = %s\n", device, storage_info);

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        goto READ_FILE_FAIL;
    }

    if (fs_exists(path) == 0)
    {
        UBOOT_DEBUG("cannot find %s\n", path);
        goto READ_FILE_FAIL;
    }

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        goto READ_FILE_FAIL;
    }

    if (fs_size(path, size) < 0)
    {
        UBOOT_ERROR("Error: Get %s file size failure\n", path);
        goto READ_FILE_FAIL;
    }
    else
        UBOOT_DEBUG("size = %llu\n", (unsigned long long)*size);

    buffer = (unsigned char *)malloc((unsigned int)*size);
    if (!buffer)
    {
        UBOOT_ERROR("Error: allocate buffer failure\n");
        goto READ_FILE_FAIL;
    }

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        if (buffer)
            free(buffer);
        goto READ_FILE_FAIL;
    }

    ret = fs_read(path, (ulong)buffer, 0, 0, &len);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Read %s file failure\n",path);
        if (buffer)
            free(buffer);
        goto READ_FILE_FAIL;
    }
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    UBOOT_TRACE("OUT\n");
    return buffer;

READ_FILE_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return NULL;
}

unsigned char *write_storage_file(const char *partition, const char *path,unsigned char *buffer, loff_t *size)
{
    int ret;
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int device = 0;
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    static loff_t len = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

    UBOOT_TRACE("IN\n");
    if (buffer==NULL)
    {
        UBOOT_ERROR("Error: Write buffer is null !!\n");
        goto WRITE_FILE_FAIL;
    }
    ret = sys_get_boot_device(device_name, sizeof(device_name));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        goto WRITE_FILE_FAIL;
    }
    else
        UBOOT_DEBUG("device_name = %s\n", device_name);

    ret = sys_get_storage_info(device,partition,storage_info);
    if(ret < 0)
    {
        UBOOT_ERROR("Error: sys_get_storage_info failure\n");
        goto WRITE_FILE_FAIL;
    }
    else
        UBOOT_DEBUG("device = %d, storage_info = %s\n", device, storage_info);

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        goto WRITE_FILE_FAIL;
    }

    if (fs_exists(path) == 0)
    {
        UBOOT_DEBUG("cannot find %s\n", path);
        goto WRITE_FILE_FAIL;
    }

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        goto WRITE_FILE_FAIL;
    }

    if (fs_size(path, size) < 0)
    {
        UBOOT_ERROR("Error: Get %s file size failure\n", path);
        goto WRITE_FILE_FAIL;
    }
    else
        UBOOT_DEBUG("size = %llu\n", (unsigned long long)*size);

    if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT))
    {
        UBOOT_ERROR("Error: partition %s select failure\n",partition);
        goto WRITE_FILE_FAIL;
    }

    ret = fs_write(path, (ulong)buffer, 0,*size, &len);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Read %s file failure\n",path);
        goto WRITE_FILE_FAIL;
    }
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    UBOOT_TRACE("OUT\n");
    return buffer;

WRITE_FILE_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return NULL;
}

#define SDBOOT_CHECK_FILE "/sdboot"

bool check_sdboot_mode(void)
{
    struct blk_desc *dev_desc = NULL;
    disk_partition_t info;
    int dev, part;
    int err;
    loff_t len;
    unsigned char read_data;
    const char zero_data = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
    part = blk_get_device_part_str(CONFIG_ENV_EXT4_INTERFACE,
                                   CONFIG_ENV_EXT4_DEVICE_AND_PART,
                                   &dev_desc, &info, 1);
    if (part < 0)
    {
        UBOOT_INFO("\n** SDBOOT: failed to get device part **\n");
        goto READ_FILE_FAIL;
    }

    dev = dev_desc->devnum;
    ext4fs_set_blk_dev(dev_desc, &info);

    if (!ext4fs_mount(info.size))
    {
        UBOOT_INFO("\n** SDBOOT: failed to mount %s %s **\n",
                 CONFIG_ENV_EXT4_INTERFACE,
                 CONFIG_ENV_EXT4_DEVICE_AND_PART);
        goto READ_FILE_FAIL;
    }

    err = ext4_read_file(SDBOOT_CHECK_FILE, &read_data, 0, 1, &len);

    if ((err == -1) || (len != 1))
    {
        UBOOT_DEBUG("\n** SDBOOT: not sdboot mode \"%s\"  %s%d:%d **\n",
               SDBOOT_CHECK_FILE, CONFIG_ENV_EXT4_INTERFACE, dev, part);
        goto READ_FILE_FAIL;
    }

    err = ext4fs_write(SDBOOT_CHECK_FILE, &zero_data, 1, FILETYPE_REG);

    ext4fs_close();
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif

    if (err == -1)
    {
        UBOOT_INFO("\n** SDBOOT: failed to write \"%s\" **\n", SDBOOT_CHECK_FILE);
    }

    if (read_data == '1')
    {
        UBOOT_INFO("\n** SDBOOT: sdboot mode **\n");
        return true;
    }
    else
    {
        UBOOT_ERROR("SDBOOT: invalid value. %d \n", read_data);
        return false;
    }

READ_FILE_FAIL:
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
    return false;
}

unsigned int b_isDTBO_Verified = false;
unsigned long getDtboAddr(void)
{
    return DTBO_ADDRESS;
}

static unsigned long getImageAddrParser(const char* part_str, const char** slicedCmdStart, const char** slicedCmdEnd)
{
    const char *partStrPt = NULL;
    if(part_str == NULL || slicedCmdStart == NULL || slicedCmdEnd == NULL)
    {
        return 0;
    }
    while(*slicedCmdStart)
    {
        //Step1: Locate the pointer to the next cmd_slice if available
        *slicedCmdEnd = strstr(*slicedCmdStart, ";");
        if(!*slicedCmdEnd)
        {
            UBOOT_DEBUG("%s:%d No Semicolon in %s.\n", __FUNCTION__, __LINE__, *slicedCmdStart);
        }

        //Step2: Locate partition string in the current cmd_slice
        partStrPt = strstr(*slicedCmdStart, (const char*)part_str);
        if(partStrPt && partStrPt[strlen((const char *)part_str)] == ' ' && (!*slicedCmdEnd || (*slicedCmdEnd && partStrPt <  *slicedCmdEnd)))
        {
            partStrPt = strstr(partStrPt, " ") + strlen(" ");
        }
        else
        {
            //In case parsed partStrPt contains any suffix other than a space char, which is not considered matching part_str
            //,or slicedCmdEnd is in front of partStrPt, this indicates the semicolon(;) doesn't belong to part_str either
            partStrPt = NULL;
        }

        //Step3: Check if Partition string matches to current cmd_slice
        if(partStrPt)
        {
            return strtoul(partStrPt, NULL, 0);
        }

        //Step4: Partition not found, let's search it in the next slice command if available
        if(*slicedCmdEnd)
        {
            *slicedCmdStart = strstr(*slicedCmdEnd, ";");
            if(slicedCmdStart)
            {
                *slicedCmdStart = *slicedCmdStart + strlen(";");
            }
        }
        else
        {
            UBOOT_DEBUG("%s:%d Couldn't get ImageAddr of %s.\n", __FUNCTION__, __LINE__, part_str);
            break;
        }
    }
    return 0;
}

unsigned long getImageAddr(const char* part_str, const char* bootcmd)
{
    const char *slicedCmdStart = bootcmd, *slicedCmdEnd = NULL;
    unsigned long val = 0;

    val = getImageAddrParser(part_str, &slicedCmdStart, &slicedCmdEnd);

    UBOOT_DEBUG("%s:%d Part name=%s Parsed address is 0x%lx.\n", __FUNCTION__, __LINE__, part_str, val);
    return val;
}

unsigned int filterBootcmd(const char* const *part_strings, size_t num_of_parts, char* filtered_bootcmd, size_t outBuffSize)
{
    if(part_strings == NULL || filtered_bootcmd == NULL)
    {
        UBOOT_ERROR("%s:%d There are null inputs.\n", __FUNCTION__, __LINE__);
        return 0;
    }

    for (int i=0; i < num_of_parts; i++)
    {
        const char* part_str = part_strings[i];
        const char *slicedCmdStart = filtered_bootcmd, *slicedCmdEnd = NULL;
        if(getImageAddrParser(part_str, &slicedCmdStart, &slicedCmdEnd))
        {
            //Filter out the corresponding slice command from filtered_bootcmd
            if(slicedCmdEnd)
            {
                slicedCmdEnd = slicedCmdEnd + strlen(";");
                UBOOT_DEBUG("%s:%d Filter out command in terms of %s.\n", __FUNCTION__, __LINE__, part_str);
                memcpy((void*)slicedCmdStart, (void*)slicedCmdEnd, outBuffSize - (slicedCmdEnd - filtered_bootcmd));
            }
        }
    }
    return 1;
}

void skip_add_retry_count(void)
{
    g_need_add_retry_count = 0;
}

void system_fail_handler()
{
    UBOOT_ERROR("system_fail_handler occurs\n");

    if (g_need_add_retry_count)
    {
#if defined(CONFIG_AB_FROM_ROM) && defined(CONFIG_AB_SIDELOAD)
        sys_add_retry_count();
#endif
    }
    reset_cpu(0);
}


#if (CONFIG_AB_SIDELOAD == 1)
static int is_delay_ab_version_update_disabled(void)
{
#define EFUSE_AB_VERSION_UPDATE_CONTROL  (RIU_BUS_ADDR|0x0407C0) //TODO
#define FLAG_AB_VERSION_UPDATE_DISABLE  (1<<7)
    return (ab_get_select_successful_boot() || ((readb(EFUSE_AB_VERSION_UPDATE_CONTROL) & FLAG_AB_VERSION_UPDATE_DISABLE) == FLAG_AB_VERSION_UPDATE_DISABLE));
}
#endif

static unsigned char* avb_data = NULL;
static unsigned int avb_data_size = 0;

void setup_avb_data_for_dtb(char* data, unsigned int size)
{
    avb_data = (unsigned char*)data;
    avb_data_size = size;
}

static int base64_encode(const unsigned char *in,  unsigned long inlen, unsigned char *out, unsigned long *outlen)
{
	unsigned long i, len2, leven;
	unsigned char *p;
    static const char *codes =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	len2 = 4 * ((inlen + 2) / 3);
	if (*outlen < len2 + 1) {
		*outlen = len2 + 1;
		return -EINVAL;
	}
	p = out;
	leven = 3 * (inlen / 3);
	for (i = 0; i < leven; i += 3) {
		*p++ = codes[(in[0] >> 2) & 0x3F];
		*p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
		*p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
		*p++ = codes[in[2] & 0x3F];
		in += 3;
	}
	if (i < inlen) {
		unsigned a = in[0];
		unsigned b = (i+1 < inlen) ? in[1] : 0;
		*p++ = codes[(a >> 2) & 0x3F];
		*p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
		*p++ = (i+1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
		*p++ = '=';
	}
	*p = '\0';
	*outlen = p - out;
	return 0;
}

#define AVB_ENCODED_BUFFER_SIZE (512)
static unsigned char avb_encoded_string[AVB_ENCODED_BUFFER_SIZE];

int insert_avb_data_into_dtb(void *fdt)
{
    int   err;
    int   nodeoffset;
    unsigned long out_size = AVB_ENCODED_BUFFER_SIZE;

    if (avb_data_size == 0)
    {
        return 0;
    }

    err = fdt_check_header(fdt);
    if (err < 0) {
        printf("insert_avb_data_into_dtb: %s\n", fdt_strerror(err));
        return err;
    }

    err = fdt_increase_size(fdt, AVB_ENCODED_BUFFER_SIZE);
    if (err) {
        printf("fdt_increase_size: %s\n", fdt_strerror(err));
        return err;
    }

    /* find or create "/avb_data" node. */
    nodeoffset = fdt_find_or_add_subnode(fdt, 0, "avb_data");
    if (nodeoffset < 0) {
        return nodeoffset;
    }

    err = base64_encode(avb_data, avb_data_size, avb_encoded_string, &out_size);
    if (err) {
        printf("base64_encode: %d\n", err);
        return err;
    }

    err = fdt_setprop(fdt, nodeoffset, "data", avb_encoded_string, out_size+1);
    if (err < 0) {
        printf("WARNING: could not set avb_data: %s\n", fdt_strerror(err));
        return err;
    }

	err = fdt_shrink_to_minimum(fdt, 0);

	if (err < 0) {
        printf("WARNING: fdt_shrink_to_minimum: %s\n", fdt_strerror(err));
        return err;
    }

    return 0;
}

/*
b_is_pass_rollback_indexes_needed determines whether UBoot passes versions to Kernel via DTB
and on CONFIG_ROLLBACK_INDEX_IN_EFUSE whether UBoot passes versions to efuse programmer via ramlog TA while invoking
Currently, there are 3 cases that call set_pass_rollback_indexes_needed() to make b_is_pass_rollback_indexes_needed true,
case 1 is version is NOT updated immediately when delay_version_update_disabled is unset on CONFIG_AB_SIDELOAD,
case 2 is version is needed to update when rollback_enabling_bit is set on CONFIG_ROLLBACK_INDEX_IN_EFUSE,
case 3 is occurrence of inconsistency when rollback_enabling_bit is set on CONFIG_ROLLBACK_INDEX_IN_EFUSE.
set_pass_rollback_indexes_needed() returns 0 on success.
The table below demostrates each case in terms of the stage where efuse programmer is invoked.
--------------------------------------------------------------------------------------------------------------------------
delay_version_update_disabled |   efuse_enabling_bit       | efuse programmer's caller    |pass_rollback_indexes_needed
--------------------------------------------------------------------------------------------------------------------------
TRUE                          |   TRUE                     | UBoot                        | TRUE
--------------------------------------------------------------------------------------------------------------------------
FALSE                         |   TRUE                     | Kernel                       | TRUE
--------------------------------------------------------------------------------------------------------------------------
TRUE                          |   FALSE                    | Kernel                       | FALSE
--------------------------------------------------------------------------------------------------------------------------
FALSE                         |   FALSE                    | Kernel                       | FALSE
--------------------------------------------------------------------------------------------------------------------------
*/
static bool b_is_pass_rollback_indexes_needed = false;
int set_pass_rollback_indexes_needed(void)
{
    int ret = AVB_IO_RESULT_OK;

#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
    unsigned int rpmb_enabling_bit = 1; //rollback feature is enabled by default
    ret = get_rpmb_rollback_enabling_bit(&rpmb_enabling_bit);
    if (ret == AVB_IO_RESULT_OK)
    {
        if (rpmb_enabling_bit)
        {
            b_is_pass_rollback_indexes_needed = true;
        }else{
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
             //for CONFIG_ROLLBACK_INDEX_IN_EFUSE, b_is_pass_rollback_indexes_needed is TRUE when rollback enabling bit is set.
            unsigned int enabling_bit = 1; //rollback feature is enabled by default
            ret = get_efuse_rollback_enabling_bit(&enabling_bit);
            if (ret == AVB_IO_RESULT_OK)
            {
                if (enabling_bit)
                {
                    b_is_pass_rollback_indexes_needed = true;
                }
           }
#endif
        }
    }

#elif (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
    //for CONFIG_ROLLBACK_INDEX_IN_EFUSE, b_is_pass_rollback_indexes_needed is TRUE when rollback enabling bit is set.
    unsigned int enabling_bit = 1; //rollback feature is enabled by default
    ret = get_efuse_rollback_enabling_bit(&enabling_bit);
    if (ret == AVB_IO_RESULT_OK)
    {
        if (enabling_bit)
        {
            b_is_pass_rollback_indexes_needed = true;
            printf("rollback index needs to update\n");
        }
    }
#elif (CONFIG_AB_SIDELOAD == 1)
    if(!is_delay_ab_version_update_disabled())
    {
        b_is_pass_rollback_indexes_needed = true;
        printf("rollback index needs to update\n");
    }
#endif
    return ret;
}

/*
*b_update_rollback_index_needed determines if versions are programmed to AVBTA immediately in uboot
and on CONFIG_ROLLBACK_INDEX_IN_EFUSE if efuse programmer is invoked in UBoot
For CONFIG_ROLLBACK_INDEX_IN_EFUSE, an additional flag ,efuse_rollback_enabling_bit, will be checked to determine *b_update_rollback_index_needed.
When b_update_rollback_index_needed is set, we not only update versions to AVB TA but send them to ramlog TA and program to eFuse.
is_update_rollback_index_needed() returns 0 on success.
------------------------------------------------------------------------------------------
delay_version_update_disabled |   efuse_enabling_bit | *b_update_rollback_index_needed
------------------------------------------------------------------------------------------
TRUE                          |   TRUE               |  TRUE
------------------------------------------------------------------------------------------
FALSE                         |   TRUE               |  FALSE
------------------------------------------------------------------------------------------
TRUE                          |   FALSE              |  FALSE
------------------------------------------------------------------------------------------
FALSE                         |   FALSE              |  FALSE
------------------------------------------------------------------------------------------
*/
int is_update_rollback_index_needed(bool* b_update_rollback_index_needed)
{
    int ret = AVB_IO_RESULT_OK;
#if (CONFIG_AB_SIDELOAD == 1)
    *b_update_rollback_index_needed = is_delay_ab_version_update_disabled();
#else
    *b_update_rollback_index_needed = true;
#endif

#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
    unsigned int rpmb_enabling_bit = 1; //rollback feature is enabled by default
    ret = get_rpmb_rollback_enabling_bit(&rpmb_enabling_bit);
    if (ret == AVB_IO_RESULT_OK)
    {
        if (!rpmb_enabling_bit){
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
            if (*b_update_rollback_index_needed == true)
            {
               //for CONFIG_ROLLBACK_INDEX_IN_EFUSE, it programs all versions to RPMB regarding rollback enabling bit.
               unsigned int enabling_bit = 1; //rollback feature is enabled by default
               ret = get_efuse_rollback_enabling_bit(&enabling_bit);
                if (ret == AVB_IO_RESULT_OK)
                {
                    //it overrides the flag when efuse enabling bit is unset
                   *b_update_rollback_index_needed = (enabling_bit ? true:false);
                }
            }
#else
            *b_update_rollback_index_needed = false;
#endif
        }
    }
#elif (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
    if (*b_update_rollback_index_needed == true)
    {
        //for CONFIG_ROLLBACK_INDEX_IN_EFUSE, it programs all versions to Efuse and RPMB regarding rollback enabling bit.
        unsigned int enabling_bit = 1; //rollback feature is enabled by default
        ret = get_efuse_rollback_enabling_bit(&enabling_bit);
        if (ret == AVB_IO_RESULT_OK)
        {
            //it overrides the flag when efuse enabling bit is unset
            *b_update_rollback_index_needed = (enabling_bit ? true:false);
        }
    }
#endif
    return ret;
}

//is_pass_rollback_index_info_needed() determines if versions are going to be packed and sent to dtbo or ramlogTA
bool is_pass_rollback_index_info_needed(void)
{
    return b_is_pass_rollback_indexes_needed;
}

//rollback index for pmu, the field is only available on CONFIG_ROLLBACK_INDEX_IN_EFUSE so far.
static uint64_t pmu_rollback_index = 0;
void set_pmu_rollback_index(uint64_t rollback_index) { pmu_rollback_index = rollback_index; }
uint64_t get_pmu_rollback_index(void) { return pmu_rollback_index; }
