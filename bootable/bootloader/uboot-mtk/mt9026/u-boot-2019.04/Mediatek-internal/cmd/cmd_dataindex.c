// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <time.h>
#include <vsprintf.h>
#include <iniutility.h>
#include <iniparser.h>
#include <utility.h>
#include <debug_impl.h>
#include <mtk_dataindex.h>

static int _do_dataindex_setpath(void)
{
    struct project_id_info *info;
    char buf[FILE_PATH_SIZE];

    if (dataindex_get_info(&info) != 0) {
        UBOOT_ERROR("cannot find project_id.ini\n");
        return CMD_RET_FAILURE;
    }

    if (snprintf(buf, FILE_PATH_SIZE, "projectid=%s", info->project_id) < 0)
    {
        UBOOT_ERROR("cannot generate %s from %s\n",
                "projectid", info->project_id);
        return CMD_RET_FAILURE;
    }
    add_bootargs("projectid", buf, 0);
    if (snprintf(buf, FILE_PATH_SIZE, "cusdata_dataIndex_path=%s", info->data_path) < 0)
    {
        UBOOT_ERROR("cannot generate %s from %s\n",
                "cusdata_dataIndex_path", info->data_path);
        return CMD_RET_FAILURE;
    }
    add_bootargs("cusdata_dataIndex_path", buf, 0);

    return CMD_RET_SUCCESS;
}

static int _do_dataindex_info(void)
{
    char partition[INI_INFO_SIZE];
    struct project_id_info *info;
    const char *relpath;

    if (dataindex_get_info(&info) != 0) {
        UBOOT_ERROR("cannot find project_id.ini\n");
        return CMD_RET_FAILURE;
    }

    printf("project_id = %s\n", info->project_id);
    printf("dataindex file = %s\n", info->data_path);

    if (dataindex_resolve_path(partition, sizeof(partition),
                &relpath, info->data_path) != ERR_DATAINDEX_OK)
    {
        UBOOT_ERROR("dataIndex directory resolves fail: '%s'\n",
                info->data_path);
        return ERR_DATAINDEX_FAIL;
    }
    printf("dataindex partition = %s\n", partition);
    printf("dataindex relative path = %s\n", relpath);

    return CMD_RET_SUCCESS;
}

static int _do_dataindex_dump(void)
{
    struct keyval_pair *keyval_entry;
    struct section *section_entry;
    iniparser_handle_t handle;
    int ret = CMD_RET_FAILURE;

    if (dataindex_get_handle(&handle) == 0)
    {
        iniparser_for_each_section(handle, section_entry)
        {
            printf("\n[%s]\n", section_entry->section);
            iniparser_for_each_keyval(section_entry, keyval_entry)
            {
                printf("%s=%s\n", keyval_entry->key, keyval_entry->value);
            }
            ret = CMD_RET_SUCCESS;
        }
    }

    return ret;
}

static int _do_dataindex_sections(void)
{
    iniparser_handle_t handle;
    struct section *section_entry;
    int ret = CMD_RET_FAILURE;

    if (dataindex_get_handle(&handle) == 0)
    {
        iniparser_for_each_section(handle, section_entry)
        {
            printf("[%s]\n", section_entry->section);
        }
        ret = CMD_RET_SUCCESS;
    }

    return ret;
}

static int _do_dataindex_keys(const char *section)
{
    struct keyval_pair *keyval_entry;
    struct section *section_entry;
    int ret = CMD_RET_FAILURE;

    if (dataindex_get_section(&section_entry, section) == 0)
    {
        printf("\n[%s]\n", section);
        iniparser_for_each_keyval(section_entry, keyval_entry)
        {
            printf("%s=%s\n", keyval_entry->key, keyval_entry->value);
        }
        ret = CMD_RET_SUCCESS;
    }

    return ret;
}

static int _do_dataindex_get(const char *section, const char *key)
{
    char buf[FILE_PATH_SIZE];
    int ret = CMD_RET_FAILURE;

    if (dataindex_get_key(buf, FILE_PATH_SIZE, section, key, NULL) == 0)
    {
        printf("[%s] %s = \"%s\"\n", section, key, buf);
        ret = CMD_RET_SUCCESS;
    }
    return ret;
}

static int _do_dataindex_resolve(const char *path)
{
    char partition[INI_INFO_SIZE];
    const char *relpath;

    if (dataindex_resolve_path(partition, sizeof(partition), &relpath, path) == 0)
    {
        printf("resolve path '%s' to partition '%s' and relpath '%s'\n",
                path, partition, relpath);
        return CMD_RET_SUCCESS;
    }
    else
    {
        UBOOT_ERROR("cannot resolve path: %s\n", path);
    }

    return CMD_RET_FAILURE;
}

static int do_dataindex(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret = CMD_RET_USAGE;

    switch (argc)
    {
    case 2:
        if (!strcmp(argv[1], "setpath"))
        {
            ret =_do_dataindex_setpath();
        }
        else if (!strcmp(argv[1], "info"))
        {
            ret =_do_dataindex_info();
        }
        else if (!strcmp(argv[1], "dump"))
        {
            ret = _do_dataindex_dump();
        }
        else if (!strcmp(argv[1], "sections"))
        {
            ret = _do_dataindex_sections();
        }
        break;
    case 3:
        if (!strcmp(argv[1], "keys"))
        {
            ret = _do_dataindex_keys(argv[2]);
        }
        else if (!strcmp(argv[1], "resolve"))
        {
            ret = _do_dataindex_resolve(argv[2]);
        }
        break;
    case 4:
        if (!strcmp(argv[1], "get"))
        {
            ret = _do_dataindex_get(argv[2], argv[3]);
        }
        break;
    }

    return ret;
}

#define DATAINDEX_MAX_ARGS      4
U_BOOT_CMD(
    dataindex, CONFIG_SYS_MAXARGS, DATAINDEX_MAX_ARGS, do_dataindex,
    "show dataindex infomation",
    "setpath - set cusdata_dataIndex_path to bootargs\n"
    "dataindex info - show dataindex infomation\n"
    "dataindex dump - dump dataindex data\n"
    "dataindex get SECTION KEY - get a value from dataindex file\n"
    "dataindex keys SECTION - list all key=value of [SECTION] dataindex file\n"
    "dataindex sections - list all sections in the dataindex file\n"
    "dataindex resolve PATH - resolve full path to partition and relative path\n"
);

