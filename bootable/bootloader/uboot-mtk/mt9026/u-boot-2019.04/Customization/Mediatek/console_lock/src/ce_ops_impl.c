// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include <errno.h>
#include <vsprintf.h> /* for panic() */
#include "ta_rpmb_impl.h"
#include "bootenv_impl.h"
#include "storage_offset_impl.h"
#include "ce_ops_impl.h"

int ce_ops_init(struct ce_ops *ops)
{
	if (!ops)
		return -ENOMEM;

	memset(ops, 0, sizeof(struct ce_ops));

#ifdef CE_STORAGE_USE_UBOOTENV
	ops->cu_read = ce_bootenv_dump_console_unlock_bit;
	ops->cu_write = ce_bootenv_set_console_unlock_bit;
	ops->le_read = ce_bootenv_dump_log_enable_bit;
	ops->le_write = ce_bootenv_set_log_enable_bit;
	ops->read_all = ce_bootenv_dump_all;
	ops->get_nonce = ce_bootenv_get_nonce;
#elif defined CE_STORAGE_USE_TA_RPMB
	ops->cu_read = ce_rpmb_dump_console_unlock_bit;
	ops->cu_write = ce_rpmb_set_console_unlock_bit;
	ops->le_read = ce_rpmb_dump_log_enable_bit;
	ops->le_write = ce_rpmb_set_log_enable_bit;
	ops->read_all = ce_rpmb_dump_all;
	ops->get_nonce = ce_rpmb_get_nonce;
#elif defined CE_STORAGE_USE_OFFSET
	ops->cu_read = ce_offset_dump_console_unlock_bit;
	ops->cu_write = ce_offset_set_console_unlock_bit;
	ops->le_read = ce_offset_dump_log_enable_bit;
	ops->le_write = ce_offset_set_log_enable_bit;
	ops->read_all = ce_offset_dump_all;
	ops->get_nonce = ce_offset_get_nonce;
#else
#error CE storage method is not SET !
#endif
	return 0;
}

int ce_ops_cu_read(struct ce_ops *ops)
{
	if (!ops || !ops->cu_read)
		panic("%s: invaild usage!\n", __func__);

	return ops->cu_read();
}

int ce_ops_cu_write(struct ce_ops *ops, bool enable)
{
	if (!ops || !ops->cu_write)
		panic("%s: invaild usage!\n", __func__);

	return ops->cu_write(enable);
}

int ce_ops_le_read(struct ce_ops *ops)
{
	if (!ops || !ops->le_read)
		panic("%s: invaild usage!\n", __func__);

	return ops->le_read();
}

int ce_ops_le_write(struct ce_ops *ops, bool enable)
{
	if (!ops || !ops->le_write)
		panic("%s: invaild usage!\n", __func__);

	return ops->le_write(enable);
}

int ce_ops_read_all(struct ce_ops *ops)
{
	if (!ops || !ops->read_all)
		panic("%s: invaild usage!\n", __func__);

	return ops->read_all();
}

int ce_ops_get_nonce(struct ce_ops *ops, void *out, size_t len)
{
	if (!ops || !ops->get_nonce)
		panic("%s: invaild usage!\n", __func__);

	return ops->get_nonce(out, len);
}

