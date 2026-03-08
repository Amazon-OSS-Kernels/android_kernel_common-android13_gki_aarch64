// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include <linux/kernel.h>
#include <vsprintf.h>
#include "console_impl.h"
#include "ce_ops_impl.h"
#include <secure_boot.h>
#include <secure_common.h>
#include <secure/crypto_auth.h>
#include <secure/secure_common.h>
#include "is_rel_flag_signature.h"
#include <debug_impl.h>

static inline int is_console_unlock(struct ce_ops *ops)
{
	return ce_ops_cu_read(ops);
}

static int ws_set_log(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	unsigned long en;
	struct ce_ops ops = { };

	// console ws 1, the argc should be 2
	if (argc != 2)
		return CMD_RET_FAILURE;

	if (ce_ops_init(&ops))
		return CMD_RET_FAILURE;

	en = simple_strtoul(argv[1], NULL, 0);

	switch (en) {
	case 0:
		printf("Set LOG disable\n");
		ce_ops_le_write(&ops, false);
		break;
	case 1:
		printf("Set LOG enable\n");
		ce_ops_le_write(&ops, true);
		break;
	default:
		printf("Invalid command %s\n", argv[0]);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int ws_not_support(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	printf("command not supported\n");
	return 0;
}

static int ws_console_lock(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct ce_ops ops = { };

	if (ce_ops_init(&ops))
		return CMD_RET_FAILURE;

	if (!is_console_unlock(&ops))
		return 0;

	printf("Lock the console, please use [ce] to unlock\n");
	ce_ops_cu_write(&ops, false);

	return 0;
}

static cmd_tbl_t cmd_console_sub[] = {
	U_BOOT_CMD_MKENT(rs, 0, 1, ws_not_support,
				"not supported", ""),
	U_BOOT_CMD_MKENT(ws, 1, 1, ws_set_log,
				"enable / disable log", ""),
	U_BOOT_CMD_MKENT(wf, 0, 1, ws_not_support,
				"not supported", ""),
	U_BOOT_CMD_MKENT(en, 0, 1, ws_console_lock,
				"lock the console", ""),
};

int do_set_console(cmd_tbl_t *cmdtp, int flag,
		int argc, char *const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], cmd_console_sub,
				ARRAY_SIZE(cmd_console_sub));
	if (!cmd)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

#ifdef CONFIG_CONSOLE_UNLOCK_USB
static const unsigned char rel_auth_data[auth_data_len] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // is_rel_flag.
#else
static const unsigned char rel_auth_data[auth_data_len] = {0x69, 0x73, 0x5F, 0x72, 0x65, 0x6C, 0x5F, 0x66, 0x6C, 0x61, 0x67, 0x0A}; // is_rel_flag.
#endif

static int get_public_key_n(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = {};

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_N_LEN) {
		UBOOT_ERROR("The len is smaller than the lenght of public-N (%d bytes).\n", RSA_PUBLIC_KEY_N_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

static int get_public_key_e(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = {};

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_E_LEN) {
		UBOOT_ERROR("The len is smaller than the lenght of public-E (%d bytes).\n", RSA_PUBLIC_KEY_E_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data + RSA_PUBLIC_KEY_N_LEN, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

int is_rel_build(void)
{
	int ret = 0;
	uint8_t rsa_public_key_N[RSA_PUBLIC_KEY_N_LEN] = {0};
	uint8_t rsa_public_key_E[RSA_PUBLIC_KEY_E_LEN] = {0};

	ret = get_public_key_n(KEY_TYPE_CKB_A_RI_KEY, rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);

	if(ret){
		UBOOT_ERROR("Fail to get public N from CKB_A_RI_KEY.\n");
		return 1;
	}

	ret = get_public_key_e(KEY_TYPE_CKB_A_RI_KEY, rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);

	if(ret){
		UBOOT_ERROR("Fail to get public E from CKB_A_RI_KEY.\n");
		return 1;
	}

	ret = secure_do_authentication(rsa_public_key_N, rsa_public_key_E, is_rel_flag_signature, (unsigned char *)rel_auth_data, sizeof(rel_auth_data));

	if(!ret){
		UBOOT_DEBUG("DEV signature verification pass! This is a dev build.\n");
		return 0;
	}

	UBOOT_ERROR("DEV signature verification failure! This is a rel build.\n");
	return 1;
}

int ce_console_guard(const char *cmd)
{
#ifdef CONSOLE_LOCK_ENABLE
	if(is_rel_build()){
		int i;
		struct ce_ops ops = {};
		const char *ce_white_list[] = {
			"ce",
			"cetest",
		};

		ce_ops_init(&ops);

		if (is_console_unlock(&ops))
			return 0;

		for (i = 0; i < ARRAY_SIZE(ce_white_list); ++i) {
			if (!strncmp(cmd, ce_white_list[i], strlen(ce_white_list[i])) || cmd[0] == 0)
				// match white lish command
				return 0;
		}
		printf("forbidden command [%s], please usb [ce] to unlock console\n", cmd);

		return -1;
	}else{
		return 0;
	}
#else
	return 0;
#endif
}
