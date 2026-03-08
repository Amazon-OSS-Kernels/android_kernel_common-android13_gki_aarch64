/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _INIT_FLOW_THREAD_H_
#define _INIT_FLOW_THREAD_H_

#if defined(CONFIG_MULTICORES_PLATFORM)
int do_after_uboot_init_task_create(void);
int do_before_console_input_task_create(void);
int do_after_console_input_task_create(void);
int do_before_boot_kernel_task_create(void);
#endif

#endif

