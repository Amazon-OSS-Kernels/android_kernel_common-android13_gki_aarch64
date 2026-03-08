// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <cmdtable.h>
#include <debug_impl.h>
#include <time.h>

int show_cmd_table(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ShowStage=0;

    UBOOT_TRACE("IN\n");
    if (argc < 2)
    {
       return cmd_usage(cmdtp);
    }
    ShowStage= (int)argv[1][0]-'0';

    mtk_show_command_table(ShowStage);

    return 0;
}

U_BOOT_CMD(
    showtb,    4,    1,     show_cmd_table,
    "Show register command table."
    "    -  showtb [stage]\n",
    "0:AFTER_UBOOT_INIT, 1:BEFORE_CONSOLE_INPUT, 2:AFTER_CONSOLE_INPUT, 3:BEFORE_BOOT_KERNEL, 4:ALL\n"
);
