/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __TA_RPMB_H__
#define __TA_RPMB_H__

#include <stdbool.h>

int ce_rpmb_set_log_enable_bit(bool enable);
int ce_rpmb_set_console_unlock_bit(bool enable);
int ce_rpmb_dump_all(void);
int ce_rpmb_dump_log_enable_bit(void);
int ce_rpmb_dump_console_unlock_bit(void);

int ce_rpmb_get_nonce(void *out, size_t len);
#endif
