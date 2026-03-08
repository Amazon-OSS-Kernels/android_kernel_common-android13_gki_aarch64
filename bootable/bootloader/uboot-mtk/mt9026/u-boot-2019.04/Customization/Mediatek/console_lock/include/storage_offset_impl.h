/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __STORAGE_OFFSET_IMPL_H__
#define __STORAGE_OFFSET_IMPL_H__

int ce_offset_dump_console_unlock_bit(void);
int ce_offset_set_console_unlock_bit(bool enable);
int ce_offset_dump_log_enable_bit(void);
int ce_offset_set_log_enable_bit(bool enable);
int ce_offset_dump_all(void);
int ce_offset_get_nonce(void *out, size_t len);

#endif
