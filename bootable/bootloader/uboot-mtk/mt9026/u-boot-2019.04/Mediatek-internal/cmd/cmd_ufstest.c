// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <vsprintf.h>
#include <debug_impl.h>
#include <mtk_ufstest.h>
#include <utility.h>

int do_mtk_ufstest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd;
	unsigned int u32_buf,u32_BlkStartAddr;
	unsigned int u32_readbuf;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = argv[1];

	if (strcmp(cmd, "speed") == 0) {
		if (argc < 4) {
			UBOOT_ERROR("too few argument!\n");
			return CMD_RET_USAGE;
		}

		u32_buf = simple_strtoul(argv[2], NULL, 16);
		u32_BlkStartAddr = simple_strtoul(argv[3], NULL, 16);

		if (ufstest_speed((u8*)(unsigned long)u32_buf, u32_BlkStartAddr) == 0)
			UBOOT_INFO("UFS Info: UFS Speed Test OK\n");
		else {
			UBOOT_ERROR("UFS Err: UFS Speed Test Fail\n");
			return CMD_RET_FAILURE;
		}

		return CMD_RET_SUCCESS;
	}
	else if (strcmp(cmd, "pwr_cut") == 0) {
		if (argc < 5) {
			UBOOT_ERROR("too few argument!\n");
			return CMD_RET_USAGE;
		}

		if(strcmp(argv[2], "init") == 0) {
			u32_buf = simple_strtoul(argv[3], NULL, 16);
			u32_BlkStartAddr = simple_strtoul(argv[4], NULL, 16);

			if(ufstest_pwrcut_initdata((u8*)(unsigned long)u32_buf, u32_BlkStartAddr) == 0)
				UBOOT_INFO("UFS Info: UFS Power Cut Init OK\n");
			else {
				UBOOT_ERROR("UFS Err: UFS Power Cut Init Fail\n");
				return CMD_RET_FAILURE;
			}
		}
		else if(strcmp(argv[2], "test") == 0) {
			u32_buf = simple_strtoul(argv[3], NULL, 16);
			u32_BlkStartAddr = simple_strtoul(argv[4], NULL, 16);

			if(argc == 6)
				u32_readbuf = simple_strtoul(argv[5], NULL, 16);
			else
				u32_readbuf = 0;

			if(ufstest_pwrcut_test((u8*)(unsigned long)u32_buf, u32_BlkStartAddr, (u8*)(unsigned long)u32_readbuf) == 0)
				UBOOT_INFO("UFS Info: UFS Power Cut Test OK\n");
			else {
				UBOOT_ERROR("UFS Err: UFS Power Cut Test Fail\n");
				return CMD_RET_FAILURE;
			}
		}

		return CMD_RET_SUCCESS;
	}
	else if (strcmp(cmd, "init") == 0) {
		u32 u32_LoopCnt, u32_i;
		ulong start, t;

		if (argc == 2)
			u32_LoopCnt = 1;
		else
			u32_LoopCnt = simple_strtoul(argv[2], NULL, 16);

		for(u32_i=0; u32_i<u32_LoopCnt; u32_i++) {
			start = get_timer(0);
			if (run_command("ufs scan 0", 0) < 0) {
				printf("ufs scan failed\n");
				return CMD_RET_FAILURE;
			}

			t = get_timer(start);
			printf("init time: %lu ms\n", t);
		}

		return CMD_RET_SUCCESS;
    }

	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	ufstest, CONFIG_SYS_MAXARGS, 1, do_mtk_ufstest,
	"UFSTEST sub system",
	"\nufstest speed addr start_block - ufstest speed test\n"
	"ufstest pwr_cut init addr start_block - ufstest Power Cut Init\n"
	"ufstest pwr_cut test addr start_block [read_addr] - ufstest Power Cut Test\n"
	"ufstest init [count] - reset & init UFS for count loops\n"
);
