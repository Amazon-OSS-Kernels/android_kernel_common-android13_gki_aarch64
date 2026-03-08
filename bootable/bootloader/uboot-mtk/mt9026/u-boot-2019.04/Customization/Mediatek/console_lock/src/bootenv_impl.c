// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <linux/stddef.h>
#include <environment.h>
#include <utility.h>
#include "bootenv_impl.h"

static inline int _check_env(const char *env)
{
	char *res = env_get(env);

	if (!res || res[0] == '0')
		return 0;

	return 1;
}

static inline int _set_env(const char *env, bool en)
{
	if (en)
		env_set(env, "1");
	else
		env_set(env, "0");
	env_save();

	return 0;
}

int ce_bootenv_dump_console_unlock_bit(void)
{
	return _check_env("console_unlock");
}

int ce_bootenv_dump_log_enable_bit(void)
{
	return _check_env("log_enable");
}

int ce_bootenv_set_console_unlock_bit(bool enable)
{
	printf("CE bootenv: Set CONSOLE UNLOCK bit to %s\n",
			enable ? "enable" : "disable");

	return _set_env("console_unlock", enable);
}

int ce_bootenv_set_log_enable_bit(bool enable)
{
	printf("CE bootenv: Set LOG ENABLE bit to %s\n",
			enable ? "enable" : "disable");

	return _set_env("log_enable", enable);
}

int ce_bootenv_dump_all(void)
{
	int res;

	res = ce_bootenv_dump_console_unlock_bit();
	printf("CE bootenv: Get CONSOLE UNLOCK bit %s\n",
			res ? "enable" : "disable");

	res = ce_bootenv_dump_log_enable_bit();
	printf("CE bootenv: Get LOG ENABLE bit %s\n",
			res ? "enable" : "disable");

	return 0;
}

int ce_bootenv_get_nonce(void *out, size_t len)
{
	return generate_random_number(out, len);
}

