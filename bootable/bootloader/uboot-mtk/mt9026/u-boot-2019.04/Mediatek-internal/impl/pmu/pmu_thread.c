// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include "debug_impl.h"
#include <smp/thread_info.h>
#include <smp/thread.h>

extern thread_t *run_cm4_thread_id;

#if defined(CONFIG_MTK_PMU)
void *boot_pmu_thread_entry(void *args)
{
    int ret = -1;
#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4)
    if (run_cm4_thread_id)
    {
        thread_join(run_cm4_thread_id, NULL);
    }
#endif
    ret = run_command("boot_pmu", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("boot_pmu command task execute failure.\n");
    }

    return NULL;
}
#endif
