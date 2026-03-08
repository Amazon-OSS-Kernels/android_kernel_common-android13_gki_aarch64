// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <time.h>
#include <debug_impl.h>
#include <utility.h>
#include <mtk_panel.h>
//#include <standby_mode.h>
#include <demura_impl.h>

int do_demura_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = 0;
    uint64_t start,end;
    UBOOT_TRACE("IN\n");
    start = get_timer(0);

    if (argc < 2)
    {
        ret = mtk_demura_process();
    }
    else
    {
        if(strncmp(argv[1], "-d", 2) == 0)
        {
            ret = mtk_demura_process();
        }
        else if (strncmp(argv[1], "-s", 2) == 0)
        {
            ;
        }
        else
        {
           cmd_usage(cmdtp);
           return ret;
        }
    }
    end = get_timer(0);
    UBOOT_BOOTTIME("[demura_init][start:%llu][end:%llu][total time:%llu]\n", start, end, end-start);
    UBOOT_TRACE("OK\n");
    return ret;
}

U_BOOT_CMD(
    demura_init, 2, 1, do_demura_init,
    "demura_init  - init panel\n",
    "command: panel_init [option]\n"
    " -s : static init : demura init para from uboot\n"
);

