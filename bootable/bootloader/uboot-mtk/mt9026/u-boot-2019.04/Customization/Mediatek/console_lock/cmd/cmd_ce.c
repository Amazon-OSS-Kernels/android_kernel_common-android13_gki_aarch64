// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <command.h>
#include <ce_impl.h>

U_BOOT_CMD(
	ce, 1, 0, do_ce,
	"Console Unlock by Challenge-Response",
	NULL
);

#ifdef CONFIG_CONSOLE_UNLOCK_USB
U_BOOT_CMD(
	ce_usb, 1, 0, do_ce_usb,
	"Check Response file in USB, and verify it to unlock console",
	NULL
);
#endif

U_BOOT_CMD(
	check_ce, 1, 0, do_check_ce,
	"Check should disable console or not",
	NULL
);
