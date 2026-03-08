/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_CMD_PUBLIC_HEADER_
#define _MTK_CMD_PUBLIC_HEADER_

enum uboot_stage{
    AFTER_UBOOT_INIT,
    BEFORE_CONSOLE_INPUT,
    AFTER_CONSOLE_INPUT,
    BEFORE_BOOT_KERNEL,
    MAX_STAGE
};

void mtk_add_command_table(char *cmd, int flag, int stage);
bool mtk_run_command_table(int eCustomizedStage);
#endif
