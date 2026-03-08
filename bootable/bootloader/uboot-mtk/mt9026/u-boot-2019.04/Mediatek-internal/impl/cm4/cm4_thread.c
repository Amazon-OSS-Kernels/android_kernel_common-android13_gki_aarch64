// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*****************************************************************************/
#include <common.h>
#include <command.h>
#include <smp/thread_info.h>
#include <smp/thread.h>
#include <cm4_impl.h>
#include <debug_impl.h>

#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4)
void *run_cm4_thread_entry(void *args)
{
    int ret = -1;

    ret = run_command("boot_vad", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("boot_vad command task execute failure.\n");
    }

    return NULL;
}
#endif
