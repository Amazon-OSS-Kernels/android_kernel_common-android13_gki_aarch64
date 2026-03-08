/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_CMD_PRIVATE_HEADER_
#define _MTK_CMD_PRIVATE_HEADER_
#include <linux/list.h>
#include <flow.h>

typedef struct{
    char *cmd;
    int flag;
    int stage;
    struct list_head list;
}ST_CMD_RECORED;

int mtk_init_command_table(void);
bool mtk_show_command_table(int showstage);
#endif
