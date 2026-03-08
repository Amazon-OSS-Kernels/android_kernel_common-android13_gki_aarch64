// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>

#include "debug_impl.h"
#include "mtk_dtv_gmac.h"

#define PHY_CMD_MAXREGS 2
#define PHY_CMD_REP 1

int do_phy_power_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;

    UBOOT_TRACE("do command: phy_power_down.\n");
    ret = mtk_dtv_gmac_hw_power_down_clk();

    return ret;
}

U_BOOT_CMD(
    phy_power_down, PHY_CMD_MAXREGS, PHY_CMD_REP, do_phy_power_down,
    "power down eth phy\n",
    NULL
);

