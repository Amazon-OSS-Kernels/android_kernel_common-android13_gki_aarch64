// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <system_impl.h>

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
EN_DEBUG_LEVEL dbgLevel=DEFAULT_DEBUG_LEVEL;
EN_DEBUG_MODULE dbgModule=DEFAULT_DEBUG_MODULE;

int _init_debug_level(void)
{
    char *pEnv=NULL;

    pEnv=env_get(ENV_DEBUG_LEVLE);
    if(pEnv==NULL)
    {
        dbgLevel=DEFAULT_DEBUG_LEVEL;
        return 0;
    }

    if(strcmp(pEnv,STR_ERROR)==0)
    {
        dbgLevel=EN_DEBUG_LEVEL_ERROR;
        return 0;
    }
    else if(strcmp(pEnv,STR_INFO)==0)
    {
        dbgLevel=(EN_DEBUG_LEVEL_INFO+EN_DEBUG_LEVEL_ERROR);
        return 0;
    }
    else if(strcmp(pEnv,STR_TRACE)==0)
    {
        dbgLevel=(EN_DEBUG_LEVEL_INFO+EN_DEBUG_LEVEL_ERROR+EN_DEBUG_LEVEL_TRACE);
        return 0;
    }
    else if(strcmp(pEnv,STR_DEBUG)==0)
    {
        dbgLevel=(EN_DEBUG_LEVEL_INFO+EN_DEBUG_LEVEL_ERROR+EN_DEBUG_LEVEL_TRACE+EN_DEBUG_LEVEL_DEBUG+EN_DEBUG_LEVEL_BOOTTIME);
        return 0;
    }
    else if(strcmp(pEnv,STR_BOOTTIME)==0)
    {
        dbgLevel=EN_DEBUG_LEVEL_BOOTTIME+DEFAULT_DEBUG_LEVEL;
        return 0;
    }
    else if(strcmp(pEnv,STR_UDTBDEBUG)==0)
    {
        dbgLevel=(EN_DEBUG_LEVEL_ERROR+EN_DEBUG_LEVEL_UDTBDEBUG);
        return 0;
    }
    else if(strcmp(pEnv,STR_KDTBDEBUG)==0)
    {
        dbgLevel=(EN_DEBUG_LEVEL_ERROR+EN_DEBUG_LEVEL_KDTBDEBUG);
        return 0;
    }
    else
    {
        dbgLevel=DEFAULT_DEBUG_LEVEL;
    }

    return 0;
}

void debug_dtb(unsigned long address, char *option)
{
    char command[64];
    char *level;
    int ret;
    int snprintf_len = 0;

    level = env_get("dbgLevel");
    if(level != NULL && address != 0)
    {
        if(strncmp(level, option, strlen(option)) == 0)
        {
            printf("%s address:0x%lx\n", level, address);
            memset(command, 0, sizeof(command));
            snprintf_len = snprintf(command, sizeof(command)-1, "fdt addr 0x%lx", address);
            if(snprintf_len < 0 || (unsigned)snprintf_len >= (sizeof(command)-1))
            {
                UBOOT_ERROR("The array size is too small(%lu), snprintf fail 'fdt addr 0x%lx'.\n", sizeof(command)-1, address);
                return;
            }
            
            ret = run_command(command, 0);
            if(ret != 0)
                printf("Set dtb address failure for debug dtb content\n");

            memset(command, 0, sizeof(command));
            strncpy(command, "fdt print /", sizeof(command)-1);
            ret = run_command(command, 0);
            if(ret != 0)
                printf("Print dtb content data failure\n");
        }
    }
}

void _dump(void *addr, unsigned int size)
{
    char buffer[CMD_BUF]="\0";
    int ret;

    ret = snprintf(buffer, CMD_BUF, "md.b %p %x", addr, size);
    if (ret < 0)
    {
        printf("snprintf execute failure, md.b command is not execution.\n");
    }
    else
    {
        ret = run_command(buffer, 0);
        if (ret != 0)
        {
            printf("md.b command execute failure.\n");
        }
    }
}

