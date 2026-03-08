// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <mtk_emmc.h>
#include <ufs_scsi.h>
#include <debug_impl.h>
#include <utility.h>

unsigned int is_RPMBKey_exist(void)
{
    char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
    unsigned int is_exist=0;
    if(sys_get_boot_device(device_name, sizeof(device_name))==-1)
    {
        UBOOT_ERROR("[ERROR] Get boot device fail !! \n");
        return 0;
    }
#if defined(CONFIG_SUPPORT_EMMC_RPMB)
    if (strncmp(device_name, "mmc", strlen("mmc")) == 0 || strncmp(device_name, "usb", strlen("usb")) == 0)
    {
        is_exist = emmc_rpmb_if_key_written();
    }
#endif
#if defined(CONFIG_SUPPORT_UFS_RPMB)
    if(strncmp(device_name, "ufs", strlen("ufs")) == 0)
    {
        if(ufs_rpmb_if_key_written()==1)
            is_exist=1;
    }
#endif
    return is_exist;
}

