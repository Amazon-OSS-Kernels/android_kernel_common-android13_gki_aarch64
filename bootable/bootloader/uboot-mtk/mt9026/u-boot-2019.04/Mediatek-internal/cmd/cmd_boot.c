// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include "dts_parser.h"
#include "utility.h"
#include <debug_impl.h>
#include <boot_impl.h>
#include <romtblo_impl.h>

int do_boot_mode_check(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    ret = boot_mode_check();
    if(ret != 0)
    {
        UBOOT_DEBUG("Do boot mode check failure\n");
        return -1;
    }

    return 0;
}

U_BOOT_CMD(
    bootcheck,    1,    1,     do_boot_mode_check,
    "Mediatek boot mode check command",
    "command: bootcheck\n"
);

int do_keypad_long_press_detect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_MT58XX_SARADC)
    long_press_sar_key_detect();
#endif
    return 0;
}

U_BOOT_CMD(
    keypadlongdetect,    1,    1,     do_keypad_long_press_detect,
    "keypad Long press detect command",
    "command: keypadlongdetect\n"
);

int do_reset_key_long_press_detect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_MT58XX_SARADC)
    long_press_sar_reset_key_detect();
#endif
    return 0;
}

U_BOOT_CMD(
    resetkeylongdetect,    1,    1,     do_reset_key_long_press_detect,
    "reset key long press detect command",
    "command: resetkeylongdetect\n"
);

int do_upgrade_mode_check(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    ret = upgrade_mode_check();
    if(ret != 0)
    {
        UBOOT_DEBUG("Do boot mode check failure\n");
        return -1;
    }

    return 0;
}

U_BOOT_CMD(
    upgradecheck,    1,    1,     do_upgrade_mode_check,
    "upgrade mode check command",
    "command: upgradecheck\n"
);

#if defined(CONFIG_BOOT_IR)
int do_bootir_mode_check(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret;
    ret = bootir_mode_check();
    if(ret != 0)
    {
        UBOOT_DEBUG("Do boot mode check failure\n");
        return -1;
    }

    return 0;
}

U_BOOT_CMD(
    bootircheck,    1,    1,     do_bootir_mode_check,
    "bootir mode check command",
    "command: bootircheck\n"
);
#endif

int do_propagate_mmap_filename(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#define MAX_MMAP_FILENAME_LEN (128)
#define MAX_BUF_LEN (256)
    char mmap_filename[MAX_MMAP_FILENAME_LEN] = {0};
    char buf[MAX_BUF_LEN] = {0};

    UBOOT_DEBUG("do_propagate_mmap_filename\n");

    if(parse_dt("/mmap_info", string_dt_parser, mmap_filename, "mmap_filename"))
    {
        //node or field is not found; notify error and continue booting
        UBOOT_ERROR("FAIL to read mmap_filename\n");
    }
    else
    {
        if (!strstr(mmap_filename, " ") && !strstr(mmap_filename, ";"))
        {
            if (snprintf(buf, MAX_MMAP_FILENAME_LEN, "mmap_filename=%s", mmap_filename) < 0)
            {
                UBOOT_ERROR("FAIL to construct mmap_filename\n");
            }
            else
            {
                if (add_bootargs("mmap_filename", buf, 0))
                {
                    UBOOT_ERROR("FAIL to add mmap_filename to bootargs\n");
                }
            }
        }
    }

    return 0;
}

U_BOOT_CMD(
    propagate_mmap_filename,    1,    1,     do_propagate_mmap_filename,
    "U-Boot sets mmap_filename to bootargs",
    "command: propagate_mmap_filename\n"
);

/*
 * unbalanced dram: set "kernelcore" to locate high bandwidth region.
 * balanced dram: no need.
 */
extern unsigned long long get_dram_size(void);
int do_setup_kernelcore_size(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_ANDROID_CN_PLATFORM
#define MAX_KERNELCORE_LEN                 (20)
#define MAX_KERNELCORE_BUF_LEN             (30)
#define IN_KERNEL_RESERVED         (0x08000000)
#define WIDE_LENGTH                (0x80000000)
	static unsigned long long dram_size = 1;
	unsigned long long kernel_start = 0;
	char buf[MAX_BUF_LEN] = {0};
	int size_mb = 0;
	int ret = 0;

	UBOOT_DEBUG("do_setup_kernelcore_size\n");

	ret = get_kernel_start(&kernel_start);
	if (ret < 0) {
		UBOOT_DEBUG("failed to get kernel_start\n");
		return 0;
	}

	dram_size = get_dram_size();
	UBOOT_DEBUG("dram_size: 0x%llx, kernel_start: 0x%llx\n", dram_size, kernel_start);

	/* 5879 3G */
	if (MT5879 == romtbl_get_chip_id_info() && dram_size == SIZE_3G)
	{
		size_mb = (WIDE_LENGTH - IN_KERNEL_RESERVED - kernel_start) / SIZE_1M;
		UBOOT_DEBUG("#################### size_mb: %d\n", size_mb);
	}
	else
	{
		UBOOT_DEBUG("balanced dram, no need kernelcore setting\n");
		return 0;
	}

	if (size_mb != 0)
	{
		ret = snprintf(buf, MAX_KERNELCORE_BUF_LEN, "kernelcore=%dM", size_mb);
		if (ret < 0)
		{
			UBOOT_ERROR("FAIL to construct kernelcore\n");
			return -1;
		}

		ret = add_bootargs("kernelcore", buf, 0);
		if (ret < 0)
		{
			UBOOT_ERROR("FAIL to add kernelcore to bootargs\n");
			return -1;
		}

		UBOOT_DEBUG("unbalanced dram, set %s\n", buf);
		return 0;
	}

	return -1;
#else
	return 0;
#endif
}

U_BOOT_CMD(
    setup_kernelcore_size,    1,    1,     do_setup_kernelcore_size,
    "U-Boot sets kernelcore to bootargs",
    "command: setup_kernelcore_size\n"
);
