// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include "ce_test_impl.h"
#include "ce_ops_impl.h"
#include "ce_impl.h"

static int test_dump_all(cmd_tbl_t *cmdtp, int flag, int argc,

			char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_read_all(&storage_ops);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_clear_all(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_cu_write(&storage_ops, false);
	ret |= ce_ops_le_write(&storage_ops, false);

	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_set_all(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_cu_write(&storage_ops, true);
	ret |= ce_ops_le_write(&storage_ops, true);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_ce_xor(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	int bit;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	bit = ce_ops_cu_read(&storage_ops);
	ret = ce_ops_cu_write(&storage_ops, (bit ^ 0x01));
	bit = ce_ops_cu_read(&storage_ops);
	ret |= ce_ops_le_write(&storage_ops, (bit ^ 0x01));

	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_console_unlock_set(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_cu_write(&storage_ops, true);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_console_unlock_clear(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_cu_write(&storage_ops, false);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_console_unlock_dump(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	printf("[CETEST] CONSOLE_UNLOCK: %d\n", ce_ops_cu_read(&storage_ops));
	return 0;
}

static int test_log_enable_set(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_le_write(&storage_ops, true);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_log_enable_clear(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = ce_ops_le_write(&storage_ops, false);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

static int test_log_enable_dump(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	printf("[CETEST] LOG_ENABLE: %d\n", ce_ops_le_read(&storage_ops));
	return 0;
}

static inline int __test_set_to_hi(int (*set_fn)(bool en),
				int (*dump_fn)(void))
{
	int tmp;

	set_fn(true);
	tmp = dump_fn();
	if (!tmp)
		return -EINVAL;

	return 0;
}

static inline int __test_set_to_lo(int (*set_fn)(bool en),
				int (*dump_fn)(void))
{
	int tmp;

	set_fn(false);
	tmp = dump_fn();
	if (tmp)
		return -EINVAL;

	return 0;
}

static inline int _ce_test(int (*set_fn)(bool en),
			int (*dump_fn)(void))
{
	int ret;
	int bit = dump_fn();

	if (bit) {
		ret = __test_set_to_lo(set_fn, dump_fn);
		if (ret)
			return ret;

		ret = __test_set_to_hi(set_fn, dump_fn);
		if (ret)
			return ret;
	} else {
		ret = __test_set_to_hi(set_fn, dump_fn);
		if (ret)
			return ret;

		ret = __test_set_to_lo(set_fn, dump_fn);
		if (ret)
			return ret;
	}

	return 0;
}

static int test_console_unlock(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = _ce_test(storage_ops.cu_write, storage_ops.cu_read);
	if (ret)
		printf("[CETEST] CONSOLE_UNLOCK test fail!\n");
	else
		printf("[CETEST] CONSOLE_UNLOCK test pass!\n");

	return 0;
}

static int test_log_enable(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	int ret;
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	ret = _ce_test(storage_ops.le_write, storage_ops.le_read);
	if (ret)
		printf("[CETEST] LOG_ENABLE test fail!\n");
	else
		printf("[CETEST] LOG_ENABLE test pass!\n");

	return 0;
}

static int test_nonce(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
#define CE_TEST_ROUND 32
	int ret, i, j;
	int err;
	uint32_t seed = 0;
	uint32_t seed_pool[CE_TEST_ROUND] = {0};
	struct ce_ops storage_ops = { };

	ret = ce_ops_init(&storage_ops);
	if (ret) {
		printf("[CETEST] storage_ops init failed!\n");
		return CMD_RET_FAILURE;
	}

	for (i = 0; i < CE_TEST_ROUND; ++i) {
		err = ce_ops_get_nonce(&storage_ops, &seed, sizeof(seed));
		if (err) {
			printf("[CETEST] get nonce failed!\n");
			return err;
		}
		printf("[CETEST] get nonce 0x%08x\n", seed);
		for (j = 0; j < i + 1; ++j) {
			if (seed_pool[j] == seed) {
				printf("[CETEST] Duplicate nonce, test failed!\n");
				return -EINVAL;
			}
		}
		seed_pool[i] = seed;
		seed = 0;
	}
	printf("[CETEST] nonce test pass!\n");

	return 0;
}

static int test_show_buildenv(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
#ifdef CONFIG_CONSOLE_UNLOCK_USB
	printf("device_ID as (%s), FW_version as (%s), serial_number as (%s)\n",
		get_device_ID(), get_FW_version(), get_serial_number());
#else
	printf("build project as (%s), type as (%s)\n", get_build_project(), get_build_type());
#endif
	return 0;
}

static cmd_tbl_t cmd_ce_test_sub[] = {
	U_BOOT_CMD_MKENT(dump_all, 0, 1, test_dump_all,
			"Dump RPMB block 0", ""),
	U_BOOT_CMD_MKENT(set, 0, 1, test_set_all,
			"Set the console unlock bit & log enable bit", ""),
	U_BOOT_CMD_MKENT(clear, 0, 1, test_clear_all,
			"Clear the console unlock bit & log enable bit", ""),
	U_BOOT_CMD_MKENT(xor, 0, 1, test_ce_xor,
			"Xor the consloe unlock & log enable bit", ""),
	U_BOOT_CMD_MKENT(cu_set, 0, 1, test_console_unlock_set,
			"Force to set the console unlock bit",""),
	U_BOOT_CMD_MKENT(cu_clear, 0, 1, test_console_unlock_clear,
			"Force to clear the console unlock bit", ""),
	U_BOOT_CMD_MKENT(cu_dump, 0, 1, test_console_unlock_dump,
			"Dump the console unlock bit", ""),
	U_BOOT_CMD_MKENT(cu_test, 0, 1, test_console_unlock,
			"Test console lock bit R/W", ""),
	U_BOOT_CMD_MKENT(le_set, 0, 1, test_log_enable_set,
			"Force to set the log enable bit",""),
	U_BOOT_CMD_MKENT(le_clear, 0, 1, test_log_enable_clear,
			"Force to clear the log enable bit", ""),
	U_BOOT_CMD_MKENT(le_dump, 0, 1, test_log_enable_dump,
			"Dump the log enable bit", ""),
	U_BOOT_CMD_MKENT(le_test, 0, 1, test_log_enable,
			"Test log enable bit R/W", ""),
	U_BOOT_CMD_MKENT(nonce, 0, 1, test_nonce,
			"Generate nonce test", ""),
	U_BOOT_CMD_MKENT(env, 0, 1, test_show_buildenv,
			"Show the build env", ""),
};

int do_ce_test(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], cmd_ce_test_sub,
				ARRAY_SIZE(cmd_ce_test_sub));
	if (!cmd)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}
