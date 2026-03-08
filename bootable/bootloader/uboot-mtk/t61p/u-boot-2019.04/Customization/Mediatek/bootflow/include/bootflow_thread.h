/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _CUSTOMIZED_FLOW_THREAD_H_
#define _CUSTOMIZED_FLOW_THREAD_H_

#if defined(CONFIG_MULTICORES_PLATFORM)
int do_after_uboot_init_customer(void);
int do_before_console_input_customer(void);
int do_after_console_input_customer(void);
int do_before_boot_kernel_customer(void);
#endif

#endif
