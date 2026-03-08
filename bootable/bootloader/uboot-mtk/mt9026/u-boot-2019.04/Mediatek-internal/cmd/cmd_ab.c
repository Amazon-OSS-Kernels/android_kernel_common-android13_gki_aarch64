// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <debug_impl.h>
#include <mtk_ab.h>
#include <utility.h>

int do_mtk_ab_pre_select (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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

    device = simple_strtoul(argv[1], NULL, 10);
    ret = sys_get_storage_info(device,"misc",storage_info);
    if(ret < 0)
        UBOOT_ERROR("Read misc partition information failure\n");

    ret = ab_selection(storage, storage_info, 1);
    UBOOT_DEBUG("A/B selection information: %d\n",ret);
    if(ret < 0)
        UBOOT_ERROR("Get ab selection information failure\n");

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    ab_pre_slt, CONFIG_SYS_MAXARGS, 1,    do_mtk_ab_pre_select,
    "ab_pre_slt - do a/b partition preselection\n",
    "<device_number>: Access the 'device_number'th storage device."
    "command: ab_pre_slt <device_number>\n"
);

int do_mtk_ab_select (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char storage[STORAGE_DEVICE_BUF_SIZE] = {0};
    char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
#if (CONFIG_AVB_VERIFY == 1)
    char cmd[COMMAND_BUF_SIZE] = {0};
    char *slot_suffix;
#endif
    int ret;
    unsigned int device = 0;
	uint64_t start = 0, end = 0;

	start = get_timer(0);
    ret = sys_get_boot_device(storage, sizeof(storage));
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
        return -1;
    }

    device = simple_strtoul(argv[1], NULL, 10);
    ret = sys_get_storage_info(device, "misc", storage_info);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: Select MISC partition failure.\n");
        return -1;
    }

    ret = ab_selection(storage, storage_info, 0);
    if (ret < 0)
    {
        UBOOT_ERROR("A/B partition select failure\n");
        if (ret == -ENOTSUPP)
        {
            UBOOT_ERROR("misc partition is not exist.\n");
        }
        else if (ret == -ENODATA)
        {
            UBOOT_ERROR("Data is not exist in MISC partition.\n");
        }
        else
        {
            UBOOT_ERROR("Create default A/B structure failure.\n");
        }
        goto AB_SELECTION_RET;
    }

    end = get_timer(0);
    UBOOT_BOOTTIME("[ab slt][start:%llu][end:%llu][total time:%llu]\n", start, end, end-start);
#if (CONFIG_AVB_VERIFY == 1)
    slot_suffix = env_get("ab_select");
    memset(cmd, 0, COMMAND_BUF_SIZE);
    strncpy(cmd, "avb verify", sizeof(cmd));
    UBOOT_DEBUG("cmd=%s\n", cmd);
    if (run_command(cmd, 0) != 0)
    {
        UBOOT_ERROR("Verify partition %s failure\n", slot_suffix);
#if defined(CONFIG_AB_FROM_ROM) && defined(CONFIG_AB_SIDELOAD)
        reset_cpu(0);
#endif
    }
    else
    {
        UBOOT_DEBUG("Verify partition %s pass\n", slot_suffix);
    }
#endif

AB_SELECTION_RET:
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
    ab_slt, CONFIG_SYS_MAXARGS, 1,    do_mtk_ab_select,
    "ab_slt - do a/b partition selection\n",
    "<device_number>: Access the 'device_number'th storage device."
    "command: ab_slt <device_number>\n"
);
