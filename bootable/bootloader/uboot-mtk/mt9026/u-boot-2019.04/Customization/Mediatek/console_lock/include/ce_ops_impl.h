/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __CE_OPS_IMPL_H__
#define __CE_OPS_IMPL_H__
#include <stdbool.h>

struct ce_ops {
	int (*cu_read)(void);
	int (*cu_write)(bool enable);
	int (*le_read)(void);
	int (*le_write)(bool enable);
	int (*read_all)(void);
	int (*get_nonce)(void *out, size_t len);
};

int ce_ops_init(struct ce_ops *ops);

int ce_ops_cu_read(struct ce_ops *ops);
int ce_ops_cu_write(struct ce_ops *ops, bool enable);
int ce_ops_le_read(struct ce_ops *ops);
int ce_ops_le_write(struct ce_ops *ops, bool enable);

int ce_ops_read_all(struct ce_ops *ops);
int ce_ops_get_nonce(struct ce_ops *ops, void *out, size_t len);

#endif
