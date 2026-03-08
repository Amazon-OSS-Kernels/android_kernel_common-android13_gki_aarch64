// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include "debug_impl.h"

#include <system_impl.h>
#include <romtblo_impl.h>

#define UBOOT_CMD_flag 2
#define UBOOT_CMD_argc 1
#define REG_DRAMC_ADDR                      (0x100)
#define REG_DRAMC_IDX                       (0x0A)
#define DRAMC_WDT_SET_BIT0   (0x0001)
#define DRAMC_WDT_SET_BIT8   (0x0100)
#define REG_WDT_RST_SEL         (0x0100)
#define REG_WDT_DRAMC_SREF_MODE  (0xC000)


int do_boot_dramc(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -ENXIO;
	int chipid = romtbl_get_chip_id_info();

	if(chipid == MT5897)
	{
		CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) | (DRAMC_WDT_SET_BIT0));
		CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) | (DRAMC_WDT_SET_BIT8));
	}
	else if(chipid == MT5879)
	{
		CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) | (REG_WDT_RST_SEL));
		CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) = (CPU_REGISTER_ACCESS(REG_DRAMC_ADDR, REG_DRAMC_IDX) | (REG_WDT_DRAMC_SREF_MODE));
	}
	UBOOT_TRACE("do dramc command.\n");

	return ret;
}

U_BOOT_CMD(
	boot_dramc, (int)UBOOT_CMD_flag, (int)UBOOT_CMD_argc, do_boot_dramc,
	"boot coprocessor with dramc\n",
	NULL
);
