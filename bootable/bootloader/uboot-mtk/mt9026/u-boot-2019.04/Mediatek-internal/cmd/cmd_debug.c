// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/ 

#include <common.h>
#include <command.h>
#include <debug_impl.h>

#define CMD_BUF 128

extern int snprintf(char *str, size_t size, const char *fmt, ...);

int check_debug_level(char *level)
{
    if (strncmp(level, STR_ERROR, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_INFO, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_TRACE, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_DEBUG, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_BOOTTIME, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_UDTBDEBUG, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_KDTBDEBUG, strlen(level)) == 0) {
        return 1;
    } else if (strncmp(level, STR_DISABLE, strlen(level)) == 0) {
        return 1;
    } else {
        return 0;
    }
}

int do_set_debugging_message_level(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char buffer[CMD_BUF] = "\0";
    int ret=0;

    if(argc!=2)
    {
        cmd_usage(cmdtp);
        return -1;
    }
    if(argv[1]==NULL)
    {
        cmd_usage(cmdtp);
        return -1;
    }

    ret = check_debug_level(argv[1]);
    if (ret == 0) {
        return -1;
    }

    ret = snprintf((char *)buffer, CMD_BUF, "setenv %s %s", ENV_DEBUG_LEVLE, argv[1]);
    if (ret < 0)
    {
        UBOOT_ERROR("snprintf buffer is not enough.\n");
        return -1;
    }
    ret=run_command(buffer,0);
    if(ret==-1)
    {
        UBOOT_ERROR("set %s to env fail.\n",ENV_DEBUG_LEVLE);
        return -1;
    }

    ret = snprintf((char *)buffer, CMD_BUF, "saveenv");
    if (ret < 0)
    {
        UBOOT_ERROR("snprintf buffer is not enough.\n");
        return -1;
    }
    ret=run_command(buffer,0);
    if(ret==-1)
    {
        UBOOT_ERROR("save %s to env fail\n",ENV_DEBUG_LEVLE);
        return -1;
    }

    _init_debug_level();

    return 0;
}

U_BOOT_CMD(
    dbg, CONFIG_SYS_MAXARGS, 1,    do_set_debugging_message_level,
    "dbg   - Set debug level as ERROR, INFO, TRACE, DEBUG, BOOTTIME\n",
    NULL
);
