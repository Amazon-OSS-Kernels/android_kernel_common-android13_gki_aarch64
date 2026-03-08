/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _INIT_FLOW_H_
#define _INIT_FLOW_H_

int do_basic_command_register(void);
int do_after_uboot_init(void);
int do_before_console_input(void);
int do_after_console_input(void);
int do_before_boot_kernel(void);
int do_jump_to_kernel(void);

#endif

