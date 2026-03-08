// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <uuid.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <utility.h>

int check_device_unique_id(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char buf[EFUSE_BUF_SIZE];
    int i,ret;

    ret = get_device_unique_id(buf,EFUSE_BUF_SIZE);
    if(ret != 0)
    {
        UBOOT_ERROR("Get efuse did failure, please check get_efuse_did\n");
        return -1;
    }

    printf("efuse: ");
    for(i=0;i<EFUSE_BUF_SIZE;i++)
    {
        printf("0x%x ",buf[i]);
    }
    printf("\n");

    return 0;
}

U_BOOT_CMD(
    efuse,    2,    1,     check_device_unique_id,
    "Get efuse value.",
    "command: efuse\n"
);

int add_dev_uniqueid_to_bootargs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned char buf[EFUSE_BUF_SIZE];
    char uuid[UUID_BUF_SIZE];
    char key[ADD_BOOTARGS_KEY_SIZE];
    char set_cfg[ADD_BOOTARGS_CFG_SIZE];
    char num[EFUSE_ELEMENT_NUM];
    char *ptr;
    int i,ret;
    int snprintf_len;

    memset(uuid, 0, UUID_BUF_SIZE);
    ptr = env_get("serialno");
    if(ptr != NULL && strlen(ptr) > 0 && strlen(ptr) < UUID_BUF_SIZE)
    {
        strncat(uuid,ptr,strlen(ptr));
    }
    else
    {
        UBOOT_INFO("Serial number had not set in ENV before, get efuse id first.\n");
        memset(buf, 0, sizeof(buf));
        ret = get_device_unique_id(buf,EFUSE_BUF_SIZE);
        if(ret != 0)
        {
            UBOOT_INFO("Get efuse id failure, produce device id by random.\n");
            ret = generate_random_number(buf, EFUSE_BUF_SIZE);
            if(ret != 0)
                UBOOT_ERROR("Generate random number failure.\n");
        }

        for(i=0;i<EFUSE_BUF_SIZE;i++)
        {
            memset(num, 0, sizeof(num));
            snprintf_len = snprintf(num, sizeof(num)-1, "%02X", buf[i]);
            if (snprintf_len >= (sizeof(num)-1))
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(num)-1), num);
                return -1;
            }
            strncat(uuid,num, UUID_BUF_SIZE-1);
        }
        env_set("serialno",uuid);
        env_save();
    }

    UBOOT_INFO("serialno=%s\n",uuid);
    memset(key, 0, ADD_BOOTARGS_KEY_SIZE);
    memset(set_cfg, 0, ADD_BOOTARGS_CFG_SIZE);
    strncpy(key,"androidboot.serialno",sizeof(key)-1);
    snprintf_len = snprintf(set_cfg, sizeof(set_cfg)-1,"%s=%s",key,uuid);
    if (snprintf_len >= (sizeof(set_cfg)-1))
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(set_cfg)-1), set_cfg);
        return -1;
    }
    ret = add_bootargs(key,set_cfg,0);
    if(ret == -1)
    {
        UBOOT_ERROR("Add unique id to bootargs failure\n");
        return -1;
    }
    return 0;
}

U_BOOT_CMD(
    deviceid,    2,    1,     add_dev_uniqueid_to_bootargs,
    "Add unique id to bootargs.",
    "command: deviceid\n"
);

int do_mtk_driver_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;

    ret = mtk_driver_init();
    if(ret != 0)
        UBOOT_ERROR("Mediatek driver already initial, do not initialize twice.\n");

    return 0;
}

U_BOOT_CMD(
    mtk_drv_init,    CONFIG_SYS_MAXARGS,    1,     do_mtk_driver_init,
    "Mediatek driver initialization",
    "command: mtk_drv_init\n"
);