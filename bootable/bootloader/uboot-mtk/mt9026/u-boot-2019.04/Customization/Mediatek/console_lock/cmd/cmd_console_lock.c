// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <console_impl.h>

U_BOOT_CMD(
	console, do_set_console_argc, 0, do_set_console,
	"Console Log Switch",
	"consle [rs, ws, wf, en]\n"
	"[rs] read storage [offset] [bytenum]\n"
	"[ws] write storage [offset] [bytenum]\n"
	"[wf] write default value\n"
	"[en] enable the console lock\n"
);
