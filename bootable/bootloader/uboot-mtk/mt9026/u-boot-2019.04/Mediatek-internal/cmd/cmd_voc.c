// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <debug_impl.h>
#include <iniutility.h>
#include <voc_impl.h>
#include "mtk-pm.h"
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#define VOC_SECTION		"Voc"
#define VOC_KEY			"VOC_INI_FILE"
#endif

#define VOC_CMD_MAX_ARGS		(3)
#define VOC_CMD_REPEAT			(0)
#define VOC_ENV_PARTITION_PATH		"tvconfig"
#define VOC_ENV_INI_PATH		"config/voc/voc_env.ini"

int do_run_voc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = CMD_RET_SUCCESS;
    struct voc_env_info voc_info;
#ifdef CONFIG_DATA_SEPARATION
    char part[PART_NAME_SIZE], filepath[FILE_PATH_SIZE];
    const char *relpath;
#endif

    UBOOT_TRACE("IN\n");

    if (argc > 2 || argc == 1)
        return CMD_RET_USAGE;

    memset(&voc_info, 0, sizeof(voc_info));
#ifdef CONFIG_DATA_SEPARATION
    memset(part, 0, sizeof(part));
    memset(filepath, 0, sizeof(filepath));
    if (dataindex_get_key(filepath, FILE_PATH_SIZE, VOC_SECTION, VOC_KEY, NULL) == 0)
    {
        if (dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath) == 0)
        {
            ret = get_voc_info(part, relpath, &voc_info);
            if (ret < 0)
            {
                UBOOT_DEBUG("Read voc information failure with data separation.\n");
                ret = get_voc_info(VOC_ENV_PARTITION_PATH, VOC_ENV_INI_PATH, &voc_info);
            }
        }
        else
        {
            UBOOT_DEBUG("resolve path fail: %s",filepath);
            ret = get_voc_info(VOC_ENV_PARTITION_PATH, VOC_ENV_INI_PATH, &voc_info);
        }
    }
    else
    {
        UBOOT_DEBUG("cannot get %s:%s from dataindex file\n", VOC_SECTION, VOC_KEY);
#endif
        ret = get_voc_info(VOC_ENV_PARTITION_PATH, VOC_ENV_INI_PATH, &voc_info);
#ifdef CONFIG_DATA_SEPARATION
    }
#endif

    if (ret < 0)
    {
        UBOOT_ERROR("Read voc information failure with default setting.\n");
        return CMD_RET_FAILURE;
    }

    if (strncmp(argv[1], "info", strlen("info")) == 0)
    {
        printf("MI_VOC_ENV: ENABLE_MIC = %d\n", voc_info.enable_mic);
        printf("MI_VOC_ENV: ENABLE_WAKEUP = %d\n", voc_info.enable_wakeup);
        printf("MI_VOC_ENV: ENABLE_SEAMLESS = %d\n", voc_info.enable_seamless);
        printf("MI_VOC_ENV: ENABLE_SMARTSPEAKER = %d\n", voc_info.enable_smartspeaker);
    }
    else if (strncmp(argv[1], "notify", strlen("notify")) == 0)
    {
        printf("MI_VOC_ENV: ENABLE_MIC = %d\n", voc_info.enable_mic);
        printf("MI_VOC_ENV: ENABLE_WAKEUP = %d\n", voc_info.enable_wakeup);
        printf("MI_VOC_ENV: ENABLE_SEAMLESS = %d\n", voc_info.enable_seamless);

        // set voc_info to CM4
        mtk_voc_set_value(voc_info);

    }
    else if (strncmp(argv[1], "standby", strlen("standby")) == 0)
    {
        // notify standby to CM4
        mtk_voc_standby(voc_info);
    }
    else
        return CMD_RET_USAGE;

    UBOOT_DEBUG("OK\n");
    return ret;
}

U_BOOT_CMD (
    voc, VOC_CMD_MAX_ARGS, VOC_CMD_REPEAT, do_run_voc,
    "Voice function for operating voice dsp.",
    "function list\n"
    "voc info - get environment variable of voice from ini\n"
    "voc notify - send environment variable to voice dsp\n"
    "voc standby - stadnby voice dsp\n"
);
