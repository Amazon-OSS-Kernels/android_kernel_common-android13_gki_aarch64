// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <time.h>
#include <debug_impl.h>
#include <mtk_cmdtable.h>
#include <cmdtable.h>

static struct list_head cmd_list;

int mtk_init_command_table(void)
{
    UBOOT_TRACE("IN\n");

    INIT_LIST_HEAD(&cmd_list);

    UBOOT_TRACE("OK\n");
    return 0;
}

void mtk_add_command_table(char *cmd, int flag, int stage)
{
    ST_CMD_RECORED *pcmd=NULL;
    UBOOT_TRACE("IN\n");
    pcmd=(ST_CMD_RECORED *)(unsigned long)malloc(sizeof(ST_CMD_RECORED));
    if(pcmd==NULL)
    {
        UBOOT_ERROR("malloc for pcmd fail==========\n");
        return;
    }
    memset(pcmd,0,sizeof(ST_CMD_RECORED));
    INIT_LIST_HEAD(&pcmd->list);
    pcmd->cmd=(char *)(unsigned long)malloc(strlen(cmd)+1);
    if(pcmd->cmd==NULL)
    {
        UBOOT_ERROR("malloc for cmd fail\n");
        free(pcmd);
        return;
    }
    memset(pcmd->cmd,0,strlen(cmd)+1);
    strncpy (pcmd->cmd, cmd, strlen(cmd));
    pcmd->flag = flag;
    pcmd->stage = stage;
    list_add_tail(&pcmd->list,&cmd_list);

    UBOOT_TRACE("OK\n");
    return;
}

bool mtk_run_command_table(int eCustomizedStage)
{
    struct list_head *ptr;
    ST_CMD_RECORED *pCmd=NULL;
    UBOOT_TRACE("IN\n");

    list_for_each(ptr, &cmd_list)
    {
        pCmd = list_entry(ptr, ST_CMD_RECORED, list);
        if(pCmd->stage == eCustomizedStage)
        {
            UBOOT_BOOTTIME("[AT][MB][%s][%lu]_start\n",pCmd->cmd, get_timer(0));
            run_command(pCmd->cmd,  pCmd->flag);
            UBOOT_BOOTTIME("[AT][MB][%s][%lu]_end\n",pCmd->cmd, get_timer(0));
        }
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

static int mtk_sort_command_table(struct list_head *sort_list, enum uboot_stage stage)
{
    struct list_head *ptr;
    ST_CMD_RECORED *pCmd=NULL;
    ST_CMD_RECORED *pTmpCmd=NULL;

    list_for_each(ptr, &cmd_list)
    {
        pCmd = list_entry(ptr, ST_CMD_RECORED, list);
        if(pCmd->stage == stage)
        {
            pTmpCmd = (ST_CMD_RECORED *)(unsigned long)malloc(sizeof(ST_CMD_RECORED));
            if(pTmpCmd == NULL)
            {
                UBOOT_ERROR("malloc for pcmd fail\n");
                return 1;
            }
            memset(pTmpCmd, 0, sizeof(ST_CMD_RECORED));
            INIT_LIST_HEAD(&pTmpCmd->list);
            pTmpCmd->cmd = (char *)(unsigned long)malloc(strlen(pCmd->cmd)+1);
            if(pTmpCmd->cmd == NULL)
            {
                UBOOT_ERROR("malloc for cmd fail\n");
                free(pTmpCmd);
                return 1;
            }
            memset(pTmpCmd->cmd, 0, strlen(pCmd->cmd)+1);
            strncpy (pTmpCmd->cmd, pCmd->cmd, strlen(pCmd->cmd));
            pTmpCmd->flag = pCmd->flag;
            pTmpCmd->stage = pCmd->stage;
            list_add_tail(&pTmpCmd->list, sort_list);
        }
    }

    return 0;
}

bool mtk_show_command_table(int showstage)
{
    int index=0;
    struct list_head tmp_list;
    struct list_head *ptr;
    ST_CMD_RECORED *pCmd=NULL;
    ST_CMD_RECORED *pTmpCmd=NULL;

    UBOOT_TRACE("IN\n");

    INIT_LIST_HEAD(&tmp_list);
    if(mtk_sort_command_table(&tmp_list, AFTER_UBOOT_INIT) != 0)
    {
        printf("sort cmd table for stage AFTER_UBOOT_INIT fail\n");
        return 1;
    }
    if(mtk_sort_command_table(&tmp_list, BEFORE_CONSOLE_INPUT) != 0)
    {
        printf("sort cmd table for stage BEFORE_CONSOLE_INPUT fail\n");
        return 1;
    }
    if(mtk_sort_command_table(&tmp_list, AFTER_CONSOLE_INPUT) != 0)
    {
        printf("sort cmd table for stage AFTER_CONSOLE_INPUT fail\n");
        return 1;
    }
    if(mtk_sort_command_table(&tmp_list, BEFORE_BOOT_KERNEL) != 0)
    {
        printf("sort cmd table for stage BEFORE_BOOT_KERNEL fail\n");
        return 1;
    }

    printf("No.  CmdName    Stage\n");
    list_for_each(ptr, &tmp_list)
    {
        pCmd = list_entry(ptr, ST_CMD_RECORED, list);
        if((pCmd->stage == AFTER_UBOOT_INIT) && (showstage == 4 ||  showstage == 0))
        {
           printf("%d  %s  AFTER_UBOOT_INIT\n", (index+1), pCmd->cmd);
        }
        if ((pCmd->stage == BEFORE_CONSOLE_INPUT) && (showstage == 4 ||  showstage == 1))
        {
           printf("%d  %s  BEFORE_CONSOLE_INPUT\n", (index+1), pCmd->cmd);
        }
        if ((pCmd->stage == AFTER_CONSOLE_INPUT) && (showstage == 4 ||  showstage == 2))
        {
           printf("%d  %s  AFTER_CONSOLE_INPUT\n", (index+1), pCmd->cmd);
        }
        if ((pCmd->stage == BEFORE_BOOT_KERNEL) && (showstage == 4 ||  showstage == 3))
        {
           printf("%d  %s  BEFORE_BOOT_KERNEL\n", (index+1), pCmd->cmd);
        }
        index++;
    }

    list_for_each(ptr, &tmp_list)
    {
        pTmpCmd = list_entry(ptr, ST_CMD_RECORED, list);
        free(pTmpCmd->cmd);
        free(pTmpCmd);
    }

    return 0;
}
