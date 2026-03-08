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
#include <mtk_dtbo.h>
#include <mtk_dataindex.h>
#include <romtblo_impl.h>
#ifdef CONFIG_LIBUFDT_OVERLAY
#include <ufdt_overlay.h>
#else
#include <fdt_support.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef CONFIG_ANDROID_CN_PLATFORM
#define CUSDATA_DEFAULT_PROJECT_PARTITION_1     "project_id"
#define CUSDATA_DEFAULT_PROJECT_PARTITION_2     "config"
#elif CONFIG_LINUX_REF_PLATFORM
#define CUSDATA_DEFAULT_PROJECT_PARTITION_1     "project_id"
#define CUSDATA_DEFAULT_PROJECT_PARTITION_2     "config"
#else
#define CUSDATA_DEFAULT_PROJECT_PARTITION_1     "config"
#define CUSDATA_DEFAULT_PROJECT_PARTITION_2     "project_id"
#endif

#define DEFAULT_DATAINDEX_LABEL                 "default"

#ifdef CONFIG_AMZ_ODMTVCONFIG_DTBO_OVERLAY
#define ODMTVCONFIG_CHECKNODE_FILEPATH              "/mnt/vendor/bootdata/dtb/odmtvconfig_dtbo_rule"
#define DTBO_TARGET_OFFSET                      2
#define KEY_WORDLIST_SIZE                       256
extern int dtbo_verify(const char * partition , const char * path, char * *dtbo_buffer , loff_t * size_p);
#endif

struct dataindex_partition {
    char partition[INI_INFO_SIZE];
    char mount[FILE_PATH_SIZE];
};

static struct project_id_info _project_id_info;
static iniparser_handle_t _dataindex_handle = NULL;
static struct dataindex_partition *_dataindex_partition = NULL;
static int _dataindex_partition_size;

static const char *_get_default_dataIndex_cfg(const char *default_id)
{
    static char id_str[FILE_PATH_SIZE] = { 0 };
    u32 chipid = romtbl_get_chip_id_info_dec();
    u32 revision = romtbl_get_chip_revision_info();
    int len;

    UBOOT_TRACE("IN\n");
    if (chipid == 0 || revision == 0) {
        UBOOT_ERROR("Wrong chip id/revision = %d/%d\n",
        chipid, revision);
        return default_id;
    }

    len = snprintf(id_str, sizeof(id_str), "mt%d_1", chipid);

    if (len >= sizeof(id_str)-1) {
        UBOOT_ERROR("The size of default_dataIndex_cfg buffer is too small!\n");
        return default_id;
    }

    UBOOT_INFO("auto select project_id:%s because dataindex_cfg_name=default\n", id_str);
    UBOOT_TRACE("OUT\n");
    return (const char *)id_str;
}

int get_data_separation_info(const char *partition, const char *file,
                struct project_id_info *info)
{
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry;
    char datadir[FILE_PATH_SIZE];
    char id[INI_INFO_SIZE];
    unsigned char *ini_file_buf;
    const char *envstr;
    loff_t size;
    int ret;

    UBOOT_TRACE("IN \n");
    ini_file_buf = read_storage_file_to_memory(partition, file, &size);
    if(ini_file_buf == NULL)
    {
        UBOOT_DEBUG("Error: Read ini file to DRAM failure\n");
        return ERR_DATAINDEX_NOT_FOUND;
    }

    ret = iniparser_create((uchar*)ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0) {
        UBOOT_ERROR("Error: parse ini [%s] failure\n", file);
        return ERR_DATAINDEX_INI_PARSE;
    }

    // get dataindex_cfg_name for auto select dataindex_chip.ini
    envstr = env_get(ENV_DATAINDEX_CFG_NAME);
    if (envstr && !strncmp(envstr, DEFAULT_DATAINDEX_LABEL, sizeof(DEFAULT_DATAINDEX_LABEL))) {
        envstr = _get_default_dataIndex_cfg(envstr);
        strlcpy(info->project_id, envstr, sizeof(info->project_id));
    } else { // else use project_id original flow

#if defined(UFBL_FEATURE_IDME)
        // Use the value comes from dataindex_cfg_name here
        if (envstr && strlen(envstr) != 0) {
            strlcpy(info->data_path, envstr, sizeof(info->data_path));
        }
#endif

        envstr = env_get(ENV_CUS_PROJECTID);
        if (envstr) {
            UBOOT_INFO("%s: use env '%s=%s' for gProjectID\n",
                    __func__, ENV_CUS_PROJECTID, envstr);

            strlcpy(info->project_id, envstr, sizeof(info->project_id));
        } else {
            ret = iniparser_get_section(ini_handle, "model", &section_entry);
            if (ret < 0) {
                iniparser_destroy(ini_handle);
                UBOOT_ERROR("Error: get ini section[%s] failure\n", "model");
                return ERR_DATAINDEX_SECTION;
            }
            ret = iniparser_getstring(section_entry, "gProjectID", "0",
                info->project_id, INI_INFO_SIZE);
            if (ret != 1) {
                iniparser_destroy(ini_handle);
                UBOOT_ERROR("Error: get ini setting %s failure\n", "model:gProjectID");
                return ERR_DATAINDEX_KEY;
            }
        }
    }

    envstr = env_get(ENV_CUS_DATAINDEX_DIR);
    if (envstr) {
        UBOOT_INFO("%s: use env '%s=%s' for DataIndexDir\n",
                __func__, ENV_CUS_DATAINDEX_DIR, envstr);
        strlcpy(datadir, envstr, sizeof(datadir));
    } else {
        ret = iniparser_get_section(ini_handle, "dataIndex", &section_entry);
        if (ret != 1) {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: get ini section[dataIndex] failure\n");
            return ERR_DATAINDEX_SECTION;
        }
        ret = iniparser_getstring(section_entry, "DataIndexDir", "",
                datadir, FILE_PATH_SIZE);
        if (ret != 1) {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("INFO: get ini setting dataIndex:DataIndexDir failure\n");
            return ERR_DATAINDEX_KEY;
        }
    }

    if (iniparser_get_section(ini_handle, PROJECT_ID_PARTITION, &section_entry) == 1)
    {
        struct keyval_pair *keyval_entry;
        int i = 0;

        UBOOT_DEBUG("load partition information\n");
        if (_dataindex_partition)
            free(_dataindex_partition);

        _dataindex_partition_size = 0;
        iniparser_for_each_keyval(section_entry, keyval_entry)
        {
            _dataindex_partition_size ++;
        }
        _dataindex_partition =
            malloc(sizeof(_dataindex_partition[0])*_dataindex_partition_size);
        if (_dataindex_partition == NULL)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Error: cannot allocate enough memory\n");
            return ERR_DATAINDEX_MEMORY;
        }

        iniparser_for_each_keyval(section_entry, keyval_entry)
        {
            strlcpy(_dataindex_partition[i].partition,
                    keyval_entry->key,
                    sizeof(_dataindex_partition[i].partition));
            iniparser_unescape_string(
                    _dataindex_partition[i].mount,
                    keyval_entry->value,
                    sizeof(_dataindex_partition[i].mount));
            i++;
        }
        UBOOT_DEBUG("parse %d partition mount point(s)\n", i);
    }

    // converity fix: the object pointed to by argument "info->project_id"
    // may overlap with the object pointed to by argument "info->data_path"
    // Use another tmp buffer "id" to fix it.
#if defined(UFBL_FEATURE_IDME)
    if (strlen(info->data_path) == 0)
#endif
    {
        memcpy(id, info->project_id, sizeof(id));
        id[sizeof(id)-1] = 0;
        if (snprintf(info->data_path, sizeof(info->data_path),
                "%s/dataIndex_%s.ini", datadir, id) < 0)
        {
            iniparser_destroy(ini_handle);
            UBOOT_ERROR("Cannot generate dataIndex file path. data_path=%s , project+id=%s\n",
                    info->data_path, id);
            return ERR_DATAINDEX_FAIL;
        }
    }

    UBOOT_INFO("Get project_id=%s\n", info->project_id);
    UBOOT_DEBUG("dataIndex file=%s\n", info->data_path);

    iniparser_destroy(ini_handle);
    UBOOT_TRACE("OK \n");
    return ERR_DATAINDEX_OK;
}

/**
 * dataindex_get_info - get dataindex information
 *
 * @info[out]: Pointer of pointer to store the output dataindex info
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_info(struct project_id_info **info)
{
    UBOOT_TRACE("IN \n");
    if (_project_id_info.project_id[0] == 0)
    {
        // search order of project_id partition:
        //   1. uboot env: ${ENV_CUS_PROJECTID_PART}
        //   2. ${CUSDATA_DEFAULT_PROJECT_PARTITION_1}
        //   3. ${CUSDATA_DEFAULT_PROJECT_PARTITION_2}
        static char *_part_project_id[] = {
            NULL,
            CUSDATA_DEFAULT_PROJECT_PARTITION_1,
            CUSDATA_DEFAULT_PROJECT_PARTITION_2
        };

        int ret = ERR_DATAINDEX_NOT_FOUND;
        int i;

        _part_project_id[0] = env_get(ENV_CUS_PROJECTID_PART);
        for (i = 0; i < ARRAY_SIZE(_part_project_id); i++)
        {
            char *part = _part_project_id[i];
            if (part && get_data_separation_info(part,
                        PROJECT_ID_FILENAME, &_project_id_info) == 0)
            {
                ret = ERR_DATAINDEX_OK;
                break;
            }
        }
        if (ret != ERR_DATAINDEX_OK)
        {
            UBOOT_ERROR("cannot find " PROJECT_ID_FILENAME "\n");
            return ret;
        }
    }
    *info = &_project_id_info;
    UBOOT_TRACE("OK \n");
    return ERR_DATAINDEX_OK;
}

/**
 * dataindex_get_handle - get ini handle of dataindex file
 *
 * @phandle[out]: Pointer to store the dataindex ini parser handle
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_handle(iniparser_handle_t *phandle)
{
    char partition[INI_INFO_SIZE];
    struct project_id_info *info;
    unsigned char *ini_file_buf;
    const char *relpath;
    loff_t size;
    int ret;

    UBOOT_TRACE("IN \n");
    if (_dataindex_handle == 0)
    {
        if (dataindex_get_info(&info) != 0)
        {
            UBOOT_ERROR("cannot get dataindex information\n");
            return ERR_DATAINDEX_NOT_FOUND;
        }
        if (dataindex_resolve_path(partition, sizeof(partition),
                    &relpath, info->data_path) != ERR_DATAINDEX_OK)
        {
            UBOOT_ERROR("dataIndex directory resolves fail: '%s'\n",
                    info->data_path);
            return ERR_DATAINDEX_FAIL;
        }

        ini_file_buf = read_storage_file_to_memory(partition, relpath, &size);
        if(ini_file_buf == NULL)
        {
            UBOOT_DEBUG("Error: Read partition %s : %s file to DRAM failure\n",
                    partition, relpath);
            return ERR_DATAINDEX_LOAD_FILE;
        }
        ret = iniparser_create((uchar *)ini_file_buf, size, &_dataindex_handle);
        if (ret < 0) {
            free(ini_file_buf);
            UBOOT_ERROR("Error: parse ini [%s] failure\n", relpath);
            return ERR_DATAINDEX_INI_PARSE;
        }
        free(ini_file_buf);
    }

    *phandle = _dataindex_handle;
    UBOOT_TRACE("OK \n");
    return ERR_DATAINDEX_OK;
}

/**
 * dataindex_get_section - get section handle of dataindex file
 *
 * @pentry[out]: Pointer to store the dataindex section handle
 * @section[in]: the section string
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_section(struct section **pentry, const char *section)
{
    iniparser_handle_t ini_handle;
    struct section *section_entry;
    int ret;

    UBOOT_TRACE("IN \n");
    ret = dataindex_get_handle(&ini_handle);
    if (ret != ERR_DATAINDEX_OK)
    {
        return ret;
    }
    ret = iniparser_get_section(ini_handle, section, &section_entry);
    if (ret != 1) {
        UBOOT_INFO("Error: get section[%s] in dataindex file failure\n", section);
        return ERR_DATAINDEX_SECTION;
    }
    *pentry = section_entry;

    UBOOT_TRACE("OK \n");
    return ERR_DATAINDEX_OK;
}

/**
 * dataindex_get_keys - get the value of key from dataindex file
 *
 * @dest[out]: the output value buffer
 * @size[in]: the size of output value buffer
 * @section[in]: the section string
 * @key[in]: the key string
 * @default_str[in]: the default string if key is missing
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 *     Notice: default_str is copied if default_str is not NULL and
 *     the the key is missing
 */
int dataindex_get_key(char *dest, int size,
       const char *section, const char *key, const char* default_str)
{
    struct section *section_entry;
    int ret;

    UBOOT_TRACE("IN \n");
    ret = dataindex_get_section(&section_entry, section);
    if (ret == ERR_DATAINDEX_OK)
    {
        if (iniparser_getstring(section_entry, key, "", dest, size) != 1)
        {
            UBOOT_ERROR("Error: get %s:%s in dataindex file failure\n",
                    section, key);
            ret = ERR_DATAINDEX_KEY;
        }
    }

    if (ret != ERR_DATAINDEX_OK && default_str)
    {
        strlcpy(dest, default_str, size);
    }

    UBOOT_TRACE("OK \n");
    return ret;
}

/**
 * dataindex_resolve_path - break down "path" to "partition" and "relative path"
 *                          that can be used in uboot.
 *
 * @partition[out]: the output partition buffer
 * @size[in]: the size of output value buffer
 * @relpath[out]: the pointer to output relative path
 * @path[in]: the path string to be resolved
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_resolve_path(char *partition, int size,
       const char **relpath, const char *path)
{
    struct project_id_info *info;
    UBOOT_TRACE("IN \n");

    if (dataindex_get_info(&info) != ERR_DATAINDEX_OK)
    {
        UBOOT_ERROR("project_id partition infomation is not initialized\n");
        return ERR_DATAINDEX_FAIL;
    }
    int path_len = strlen(path);
    int i;

    for (i = 0; i < _dataindex_partition_size; i++)
    {
        const char *part = _dataindex_partition[i].partition;
        const char *mount = _dataindex_partition[i].mount;
        int len = strlen(mount);
        if (mount[len-1] == '/')
            len--;
        UBOOT_DEBUG("keyval: len=%d, %s=%s , path=%s\n", len, part, mount, path);
        if (len <= path_len
            && !strncmp(mount, path, len)
            && (path[len] == '/' || path[len] == 0))
        {
            if (partition)
                strlcpy(partition, part, size);
            if (relpath)
                *relpath = &path[len];
            UBOOT_TRACE("OK \n");
            return ERR_DATAINDEX_OK;
        }
    }

    return ERR_DATAINDEX_FAIL;
}

#ifdef CONFIG_AMZ_ODMTVCONFIG_DTBO_OVERLAY
/**
 * dataindex_dtbo_check:
 * 1. check dtbo overlay whitelist
 * 2. check the node name of target dtbo file in dataindex multi_dtbo folder
 *
 * @dtbo[in]: the pointer of dtbo file
 * @key[in]: include dtbo overlay whitelist and the full path filename of checklist file
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 *     Notice: default_str is copied if default_str is not NULL and
 *     the the key is missing
 */
static int dataindex_dtbo_check(unsigned char *dtbo, unsigned int dtbo_size, const char *key)
{
    int fixups_offset = fdt_path_offset(dtbo, "/__fixups__");
    int nextproperty_offset = 0;
    const struct fdt_property *property;
    const char *name;
    loff_t size = 0;
    int ret = 0;

    char partition[INI_INFO_SIZE] = {0};
    iniparser_handle_t ini_handle = NULL;
    struct section *section_entry = NULL;;
    struct section *section_entry_2 = NULL;;
    unsigned char *ini_file_buf = NULL;
    char keyword[KEY_WORDLIST_SIZE] = {0};
    const char *relpath = NULL;

    UBOOT_TRACE("IN \n");
    ret = dataindex_resolve_path(partition, PART_NAME_SIZE, &relpath, ODMTVCONFIG_CHECKNODE_FILEPATH);
    if (ret)
    {
        UBOOT_ERROR("resolve path=%s fail!\n", relpath);
        return ret;
    }

    ini_file_buf = read_storage_file_to_memory(partition, (char *)relpath, &size);
    if (ini_file_buf == NULL)
    {
        UBOOT_ERROR("Error: Read ini file to DRAM failure, relpath=%s\n", relpath);
        ret = ERR_DATAINDEX_NOT_FOUND;
        return ret;
    }

    ret = iniparser_create((uchar*)ini_file_buf, size, &ini_handle);
    free(ini_file_buf);
    if (ret < 0)
    {
        UBOOT_ERROR("Error: parse file [%s] failure\n", ODMTVCONFIG_CHECKNODE_FILEPATH);
        ret = ERR_DATAINDEX_INI_PARSE;
        return ret;
    }

    ret = iniparser_get_section(ini_handle, "odmtvconfig_dtbo_whitelist", &section_entry);
    if (ret != 1) {
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "odmtvconfig_dtbo_whitelist");
        return ret;
    }

    ret = iniparser_getstring(section_entry, &key[DTBO_TARGET_OFFSET], "", keyword, KEY_WORDLIST_SIZE);
    if (ret != 1) {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini setting %s failure\n", &key[DTBO_TARGET_OFFSET]);
        return ret;
    }

    if (!strcmp(key, keyword)) {
        UBOOT_DEBUG("odmtvconfig dtbo overlay whitelist check OK for [%s]=[%s] \n", key, keyword);
    } else {
        UBOOT_ERROR("odmtvconfig dtbo overlay whitelist check FAIL, right is [%s], isn't [%s] \n", keyword, key);
        return -1;
    }

    ret = iniparser_get_section(ini_handle, "odmtvconfig_dtbo_node", &section_entry_2);
    if (ret != 1)
    {
        UBOOT_ERROR("Error: get ini section[%s] failure\n", "odmtvconfig_dtbo_node");
        ret = ERR_DATAINDEX_SECTION;
        return ret;
    }

    ret = iniparser_getstring(section_entry_2, &key[DTBO_TARGET_OFFSET], "", keyword, KEY_WORDLIST_SIZE);
    if (ret != 1)
    {
        iniparser_destroy(ini_handle);
        UBOOT_ERROR("Error: get ini setting %s failure\n", &key[DTBO_TARGET_OFFSET]);
        return ERR_DATAINDEX_KEY;
    }

    if (fdt_check_header(dtbo) != 0)
    {
        UBOOT_ERROR("Error: Bad device tree blob header\n");
        return ERR_DATAINDEX_KEY;
    }

    if (fixups_offset < 0)
    {
        UBOOT_ERROR("Could not find /__fixups__ in the DTBO\n");
        return ERR_DATAINDEX_NOT_FOUND;
    }

    fdt_for_each_property_offset(nextproperty_offset, dtbo, fixups_offset)
    {
        property = fdt_get_property_by_offset(dtbo, nextproperty_offset, NULL);
        if (!property)
        {
            UBOOT_ERROR("Error getting property\n");
            continue;
        }
        name = fdt_string(dtbo, fdt32_to_cpu(property->nameoff));
        UBOOT_INFO("nextproperty_offset: [%d] / Node name: [%s]\n", nextproperty_offset, name);
        UBOOT_INFO("Checking Node name in keyword list: %s\n", keyword);
        if (!strstr(keyword, name))
        {
            UBOOT_ERROR("Node name %s  is invalid\n", name);
            return ERR_DATAINDEX_KEY;
        }
        else
        {
            UBOOT_INFO("Node name %s is allowed\n", name);
        }
    }

    UBOOT_INFO("Check file successfully\n");
    UBOOT_TRACE("OK \n");
    return 0;
}


static int __load_and_overlay_dtbo(char *ft_addr, const char *filepath, const char *key)
#else
static int __load_and_overlay_dtbo(char *ft_addr, const char *filepath)
#endif
{
    struct fdt_header *source = (struct fdt_header *)ft_addr;
    const char *relpath;
    loff_t blob_len;
    char part[PART_NAME_SIZE];
    unsigned char *blob;
    int ret;


    UBOOT_TRACE("IN \n");
    ret = dataindex_resolve_path(part, PART_NAME_SIZE, &relpath, filepath);
    if (ret)
    {
        UBOOT_DEBUG("resolve path=%s fail!\n", filepath);
        return ret;
    }

    UBOOT_DEBUG("try to load part:relpath=%s:%s\n", part, relpath);
    blob = read_storage_file_to_memory(part, (char *)relpath, &blob_len);
    if (blob == NULL)
    {
        UBOOT_ERROR("read %s:%s file fail!\n", part, relpath);
        return -ERR_DATAINDEX_LOAD_FILE;
    }

#ifdef CONFIG_AMZ_ODMTVCONFIG_DTBO_OVERLAY
    blob = NULL;
    ret = dtbo_verify("odmtvconfig", relpath, (char **)&blob, &blob_len);

    if (blob == NULL || blob_len <= 0) {
        UBOOT_ERROR("dtbo verify FAIL\n");
        free(blob);
        return 0;
    }
    printf("dtbo_verify status = %d\n", ret);
    // check the dtbo file with keyword list
    if (dataindex_dtbo_check(blob, blob_len, key))
    {
        UBOOT_ERROR("Failed on checking node/white list of file!\n");
        free(blob);
        return -ERR_DATAINDEX_FAIL;
    }
#endif

#ifdef CONFIG_LIBUFDT_OVERLAY
    source = ufdt_apply_overlay(source, fdt32_to_cpu(source->totalsize),
            (void*)blob, (size_t)blob_len);
    free(blob);
    if (source == NULL)
    {
        UBOOT_ERROR("ufdt_apply_overlay execute failure.\n");
        return -ERR_DATAINDEX_FAIL;
    }
    else
    {
        ret = fdt_open_into((void *)source, (void *)ft_addr, fdt32_to_cpu(source->totalsize));
        UBOOT_DEBUG("overlay %s:%s pass! , copy to kernel dtb[%d]\n",
                part, relpath, ret);
        free(source);
        return 0;
    }
#else
    ret = fdt_overlay_apply_verbose((void *)ft_addr, blob);
    free(blob);
    if (ret < 0)
    {
        UBOOT_ERROR("fdt_overlay_apply_verbose execute failure.\n");
        return -ERR_DATAINDEX_FAIL;
    }
#endif
    UBOOT_TRACE("OK \n");
    return -ERR_DATAINDEX_FAIL;
}

/**
 * dataindex_dtbo_overlay - overlay dtbo read from dataindex file
 *
 * @ft_addr[in]: the kernel dtb buffer
 * @mode[in]: the overlay mode. 'u' for uboot and 'k' for kernel
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_dtbo_overlay(char *ft_addr, char mode)
{
    struct keyval_pair *keyval_entry;
    struct section *section_entry;
    int ret = -ERR_DATAINDEX_OK;
    char filepath[FILE_PATH_SIZE];
    char *dtbo_str;
    u64 start = 0, end = 0;

    UBOOT_TRACE("IN \n");
    start = get_timer(0);
    dtbo_str = env_get(ENV_DATAINDEX_DTBO);
    if (dtbo_str && (dtbo_str[0] == '0' || dtbo_str[0] == 'n'))
    {
        printf("detect env %s=%s, skip dataindex dtbo overlay\n",
                ENV_DATAINDEX_DTBO, dtbo_str);
        return ret;
    }
    if (dataindex_get_section(&section_entry, "dtbo") == 0)
    {
        /* caller send ft_addr==NULL when in uboot dtbo mode */
        if (ft_addr == NULL)
        {
            unsigned long addr;
            get_uboot_dtb_addr(&addr);
            ft_addr = (char*)addr;
        }
        iniparser_for_each_keyval(section_entry, keyval_entry)
        {
            const char *key = keyval_entry->key;
            UBOOT_INFO("%s: get dtbo info: %s=%s\n", __func__, key, keyval_entry->value);
            if (key[1] != ',')
            {
                UBOOT_ERROR("dataindex dtbo entry format error: get %s=%s\n", key, keyval_entry->value);
                UBOOT_ERROR("Key format should be 'mode,name' which mode is 'u'/'k'/'*'\n");
                continue;
            }
            if (key[0] != mode && key[0] != '*')
                continue;
            iniparser_unescape_string(filepath, keyval_entry->value, sizeof(filepath));
            UBOOT_INFO("%s: overlay %s\n", __func__, filepath);
#ifdef CONFIG_AMZ_ODMTVCONFIG_DTBO_OVERLAY
            if (__load_and_overlay_dtbo(ft_addr, filepath, key))
#else
            if (__load_and_overlay_dtbo(ft_addr, filepath))
#endif
            {
                UBOOT_ERROR("Cannot overlay dtbo file: %s\n", filepath);
                ret = -ERR_DATAINDEX_FAIL;
            }
        }
    }
    end = get_timer(0);
    UBOOT_BOOTTIME("[%s][start:%llu][end:%llu][total time:%llu]\n", __func__, start, end, end - start);

    UBOOT_TRACE("OK \n");
    return ret;
}

