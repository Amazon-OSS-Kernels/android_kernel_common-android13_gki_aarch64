// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <smp/thread_info.h>
#include <smp/thread.h>

extern thread_t *panel_panel_enable_thread_id;

void *show_logo_thread_entry(void *args)
{
    int ret = -1;

    if (panel_panel_enable_thread_id)
    {
        thread_join(panel_panel_enable_thread_id, NULL);
    }

    ret = run_command("showlogo", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("showlogo command task execute failure.\n");
    }

    return NULL;
}
