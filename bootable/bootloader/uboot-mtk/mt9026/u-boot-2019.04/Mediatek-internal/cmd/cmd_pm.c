// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>

#include "debug_impl.h"
#include "mtk-pm.h"

int do_pm_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int32_t pm_init_out;

    pm_init_out = pm_init();
	
    return pm_init_out;
}

U_BOOT_CMD(
	pm_init, 2, 1, do_pm_init,
	"init pm driver data\n",
	NULL
);
