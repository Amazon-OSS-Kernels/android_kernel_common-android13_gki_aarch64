// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "common.h"
#include <environment.h>
#include "utility.h"
#include "one_package_impl.h"
#include "dts_parser.h"
#include "debug_impl.h"

#define DATA_BUFFER_SIZE (4*1024)
#define ONE_PACKAGE_INFO_FLASH_OFFSET (24*1024)
#define EMMC_BLOCK_SIZE (512)
#define USB_BLOCK_SIZE (512)
#define UFS_BLOCK_SIZE (4*1024)
#define ONE_PACKAGE_INIT_ENV_NAME "ONE_PACKAGE_INIT"

static struct OnePackageInfo* info;
static uint8_t buffer[DATA_BUFFER_SIZE] __attribute__((aligned(4096))) = {0};
static char device[16] = {0};
static char command_buffer[64] = {0};

static int check_device_unique_id(unsigned short* buf)
{
#define RIU_DEVICE_UNIQUE_ID (0x1C000000 + (0x20300<<1))
    int i = 0;

    for (i=0; i<8; i++)
    {
        if (buf[i] != *((volatile unsigned short*)RIU_DEVICE_UNIQUE_ID + (i<<1)) )
            return 0;
    }

    return 1;
}

static void read_device_unique_id(unsigned short* buf)
{
    int i = 0;

    for (i=0; i<8; i++)
    {
        buf[i] = *((volatile unsigned short*)RIU_DEVICE_UNIQUE_ID + (i<<1));
    }
}

int one_package_init(void)
{
    int match = 0;
    unsigned int mmap_id = 0;

    UBOOT_DEBUG("one_package_init\n");

    if (parse_dt("/mmap_info", integer_dt_parser, &mmap_id, "mmap_sha256"))
    {
        //node or field is not found; notify error and continue booting
        printf("Warning: mmap_sha256 is not found in /mmap_info\n");
        return 0;
    }

    if (mmap_id == 0)
    {
        printf("Warning: incorrect mmap_sha256 value\n");
        return 0;
    }

    if (sys_get_boot_device(device, sizeof(device)-1) != 1)
    {
        printf("Error: sys_get_boot_device failed\n");
        return 0;
    }

    // read 4KB from eMMC/UFS/USB to buffer
    if ( strncmp("mmc", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "mmc read %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/EMMC_BLOCK_SIZE, DATA_BUFFER_SIZE/EMMC_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
    }
    else if( strncmp("usb", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "usb read %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/USB_BLOCK_SIZE, DATA_BUFFER_SIZE/USB_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
        match = 1;
    }
    else if( strncmp("ufs", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "ufs read %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/UFS_BLOCK_SIZE, DATA_BUFFER_SIZE/UFS_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
    }
    else
    {
        printf("Error: device %s is not supported\n", device);
        return 0;
    }

    if (run_command(command_buffer, 0))
    {
        printf("Error: Read OnePackageInfo failed\n");
        return 0;
    }

    // convert buffer to struct OnePackageInfo
    info = (struct OnePackageInfo*)buffer;

    // compare device id
    if (!match)
    {
        match = check_device_unique_id(info->unique_id);
    }

    // check whether struct has been inited correctly
    if (info->magic == ONE_PACKAGE_INFO_MAGIC
            && info->version >= ONE_PACKAGE_INFO_VERSION
            && info->tee_model_id == mmap_id
            && match)
    {
        //do nothing
        return 0;
    }


    // info not synced; re-init
    printf("Updating OnePackageInfo...\n");

    info->magic = ONE_PACKAGE_INFO_MAGIC;
    info->version = ONE_PACKAGE_INFO_VERSION;
    info->tee_model_id = mmap_id;
    read_device_unique_id(info->unique_id);

    flush_dcache_all();

    memset(command_buffer, 0, sizeof(command_buffer));

    //write buffer to eMMC/UFS/USB
    if ( strncmp("mmc", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "mmc write %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/EMMC_BLOCK_SIZE, DATA_BUFFER_SIZE/EMMC_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
    }
    else if( strncmp("usb", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "usb write %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/USB_BLOCK_SIZE, DATA_BUFFER_SIZE/USB_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
    }
    else if( strncmp("ufs", device, sizeof(device)-1) == 0)
    {
        if (snprintf(command_buffer, sizeof(command_buffer)-1, "ufs write %llX %X %X", (unsigned long long)buffer, ONE_PACKAGE_INFO_FLASH_OFFSET/UFS_BLOCK_SIZE, DATA_BUFFER_SIZE/UFS_BLOCK_SIZE) < 0)
        {
            UBOOT_ERROR("snprintf failed\n");
            return 0;
        }
    }
    else
    {
        printf("Error: device %s is not supported\n", device);
        return 0;
    }

    if (run_command(command_buffer, 0))
    {
        printf("Error: Write OnePackageInfo failed\n");
        return 0;
    }

    //setenv and save
    if (env_set(ONE_PACKAGE_INIT_ENV_NAME, "1"))
    {
        printf("Error: env_set failed\n");
        return 0;
    }

    if (env_save())
    {
        printf("Error: env_save failed\n");
        return 0;
    }

    //chip reset
    if (run_command("reset", 0))
    {
        printf("Error: reset failed\n");
        return 0;
    }

    return 0;
}
