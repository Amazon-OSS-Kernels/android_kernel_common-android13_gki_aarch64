/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __CONSOLE_IMPL_H__
#define __CONSOLE_IMPL_H__

#define auth_data_len 12
#define do_set_console_argc 3

int do_set_console(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[]);
int is_rel_build(void);
int ce_console_guard(const char *cmd);

#endif
