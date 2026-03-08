// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include "common.h"
#include "command.h"
#include "debug_impl.h"
#include <smp/thread_info.h>
#include <smp/thread.h>

void *pm_init_thread_entry(void *args)
{
    int ret = -1;

    ret = run_command("pm_init", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("pm_init command task execute failure.\n");
    }

    return NULL;
}
