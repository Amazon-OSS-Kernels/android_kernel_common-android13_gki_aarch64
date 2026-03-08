// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <mtk_gegop.h>
#include <smp/thread_info.h>
#include <smp/thread.h>

thread_t *show_logo_thread_id = NULL;
extern int panel_priority;

int do_after_uboot_init_customer(void)
{
    return 0;
}

int do_before_console_input_customer(void)
{
    return 0;
}

int do_after_console_input_customer(void)
{
    panel_priority--;
    show_logo_thread_id = thread_create_ex("showlogo", show_logo_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (show_logo_thread_id == NULL)
    {
        UBOOT_ERROR("deviceid thread create fail...\n");
    }
    return 0;
}

int do_before_boot_kernel_customer(void)
{
    return 0;
}
