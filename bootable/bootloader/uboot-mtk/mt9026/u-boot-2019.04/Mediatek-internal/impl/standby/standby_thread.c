// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <smp/thread_info.h>
#include <smp/thread.h>

#ifdef CONFIG_MTK_PMU
extern thread_t *boot_pmu_thread_id;
#endif

void *standby_mode_thread_entry(void *args)
{
    int ret = -1;
#ifdef CONFIG_MTK_PMU
    if (boot_pmu_thread_id)
    {
        thread_join(boot_pmu_thread_id, NULL);
    }
#endif

    ret = run_command("standby", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("standby command task execute failure.\n");
    }

    return NULL;
}
