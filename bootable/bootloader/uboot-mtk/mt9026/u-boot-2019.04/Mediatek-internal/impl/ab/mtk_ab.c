// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <part.h>
#include <debug_impl.h>
#include <mmc.h>
#include <partition.h>
#include <android_ab.h>
#include <mtk_ab.h>
#include <utility.h>

#define BOOTARGS_BUF_SIZE       64
#define BOOTARGS_KEY_BUF_SIZE   32
#define AB_SLOT_BUF_SIZE         2

enum PARTITION_SELECT ab_selection_result = PARTITION_SELECT_MAX;

int get_ab_selection_result(void)
{
    return ab_selection_result;
}

#if (CONFIG_AB_SIDELOAD == 1)
int ab_pre_selection(void)
{
    char storage[STORAGE_DEVICE_BUF_SIZE] = {0};
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
    int ret;
    unsigned int device = 0;

    ret = sys_get_boot_device(storage, sizeof(storage));
    if(ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return -1;
    }

    ret = sys_get_storage_info(device,"misc",storage_info);
    if(ret < 0)
        UBOOT_ERROR("Read misc partition information failure\n");

    ret = ab_selection(storage, storage_info, 1);
    UBOOT_DEBUG("A/B selection information: %d\n",ret);
    if(ret < 0)
        UBOOT_ERROR("Get ab selection information failure\n");

    return 0;
}
#endif

#if defined(CONFIG_LINUX_REF_PLATFORM)
static int replace_linux_ab_roots(struct blk_desc *dev_desc)
{
    #define SLOT_A_SUFFIX "a\0"
    #define SLOT_B_SUFFIX "b\0"
    #define ROOTFILESYSTEM_NAME "rootfs\0"
    disk_partition_t info;
    int part_num;
    char key_linux[BOOTARGS_KEY_BUF_SIZE]={0};
    char set_cfg_linux[BOOTARGS_BUF_SIZE]={0};
    char partition_name[PART_NAME_LEN]= {0};

    if (get_ab_selection_result() == PARTITION_SELECT_A)
    {
        snprintf(partition_name, sizeof(partition_name)-1,"%s_%s",ROOTFILESYSTEM_NAME,SLOT_A_SUFFIX);
    }
    else if (get_ab_selection_result() == PARTITION_SELECT_B)
    {
        snprintf(partition_name, sizeof(partition_name)-1,"%s_%s",ROOTFILESYSTEM_NAME,SLOT_B_SUFFIX);
    }
    else
    {
        UBOOT_ERROR("[ERROR] Convert ab name occured unknown error\n");
        return -1;
    }
    part_num = part_get_info_by_name(dev_desc, partition_name, &info);
    if (part_num <= 0)
    {
        UBOOT_ERROR("[ERROR]%s partition # not found !!!\n",partition_name);
        return -1;
    }
    strncpy(key_linux,"root",sizeof(key_linux)-1);
    snprintf(set_cfg_linux, sizeof(set_cfg_linux)-1,"%s=/dev/mmcblk0p%d",key_linux,part_num);
    add_bootargs(key_linux,set_cfg_linux,0);
    UBOOT_INFO("LINUX: Add root partition %s\n", set_cfg_linux);
    return 0;
}
#endif

int ab_selection(char *storage, char *part, int pre_select)
{
    int ret;
    struct blk_desc *dev_desc = NULL;
    disk_partition_t part_info;
    char slot[AB_SLOT_BUF_SIZE];
    char key[BOOTARGS_KEY_BUF_SIZE]={0},set_cfg[BOOTARGS_BUF_SIZE]={0};

    /* Lookup the "misc" partition */
    if (pre_select == 1){
        UBOOT_DEBUG("Execute A/B pre-select\n");
    }else{
        if (part_get_info_by_dev_and_name_or_num(storage, part,&dev_desc, &part_info) < 0) {
            UBOOT_ERROR("Get partition information failure\n");
            return -ENOTSUPP;
        }
    }

    if(pre_select == 1)
        ret = ab_pre_select_slot();
    else
        ret = ab_select_slot(dev_desc, &part_info);

    if (ret < 0) {
        UBOOT_ERROR("Android boot failed, error %d.\n", ret);
        return ret;
    }

    /* Android standard slot names are 'a', 'b', ... */
    ab_selection_result = ret;
    UBOOT_INFO("A/B selection result: %d\n", ret);
    if(pre_select == 1)
        return 0;

    slot[0] = ANDROID_BOOT_SLOT_NAME(ret);
    slot[1] = '\0';
    strncpy(key,"androidboot.slot_suffix",sizeof(key)-1);

#if defined(CONFIG_LINUX_REF_PLATFORM)
    char key_linux[BOOTARGS_KEY_BUF_SIZE]={0};
    char set_cfg_linux[BOOTARGS_BUF_SIZE]={0};
    strncpy(key_linux,"linuxboot.slot_suffix",sizeof(key_linux)-1);

    if (!strncmp(slot, "b", sizeof(slot))) {
        snprintf(set_cfg_linux, sizeof(set_cfg_linux)-1,"%s=_%s",key_linux,slot);
    }
    else {
        snprintf(set_cfg_linux, sizeof(set_cfg_linux)-1,"%s=_%s",key_linux,"a");
    }
    add_bootargs(key_linux,set_cfg_linux,0);
    UBOOT_INFO("LINUX: Booting slot: %s\n", slot);
    replace_linux_ab_roots(dev_desc);
#endif

    snprintf(set_cfg, sizeof(set_cfg)-1,"%s=_%s",key,slot);
    add_bootargs(key,set_cfg,0);
    env_set("ab_select", slot);
    UBOOT_INFO("ANDROID: Booting slot: %s\n", slot);
    return 0;
}
