// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <ce_test_impl.h>

U_BOOT_CMD(
	cetest, do_ce_text_argc, 0, do_ce_test,
	"Console Unlock test command:",
	"[dump_all|(cu/le)_set|(cu/le)_clear|(cu/le)_test|xor|nonce|env]\n"
	"dump_all             - Dump RPMB block 0\n"
	"set                  - Set the console unlock bit & log enable bit\n"
	"clear                - Clear the console unlock bit & log enable bit\n"
	"xor                  - Xor the consloe unlock & log enable bit\n"
	"cu_set               - Force to set the console unlock bit\n"
	"cu_clear             - Force to clear the console unlock bit\n"
	"cu_dump              - Dump the console unlock bit\n"
	"cu_test              - Test console lock bit R/W\n"
	"le_set               - Force to set the log enable bit\n"
	"le_clear             - Force to clear the log enable bit\n"
	"le_dump              - Dump the log enable bit\n"
	"le_test              - Test log enable bit R/W\n"
	"nonce                - Generate nonce test\n"
	"env                  - Show the build env\n"
);
