/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DATAINDEX_H
#define _DATAINDEX_H

#include <iniparser.h>
#include <iniutility.h>

#ifndef FILE_PATH_SIZE
#define FILE_PATH_SIZE  128
#endif
#ifndef PART_NAME_SIZE
#define PART_NAME_SIZE  32
#endif
/*
 * dataindex error codes
 */
enum {
    ERR_DATAINDEX_OK            =   0,
    ERR_DATAINDEX_FAIL          =  -1,  // general fail
    ERR_DATAINDEX_NOT_FOUND     =  -2,  // file not found
    ERR_DATAINDEX_LOAD_FILE     =  -3,  // load file from file to dram fail
    ERR_DATAINDEX_INI_PARSE     =  -4,  // ini parse fail
    ERR_DATAINDEX_SECTION       =  -5,  // dataindex section error
    ERR_DATAINDEX_KEY           =  -6,  // dataindex key error
    ERR_DATAINDEX_MEMORY        =  -7,  // allocate memory error
    ERR_DATAINDEX_NOT_SUPPORT   =  -8,  // not support error
};

struct project_id_info {
    char project_id[INI_INFO_SIZE];
    char data_path[FILE_PATH_SIZE];
};


/*
 * uboot env vars of data separation
 */
#define ENV_DATAINDEX_DTBO      "dataindex_dtbo"

#define ENV_CUS_PROJECTID_PART  "cus_projectid_part"
#define ENV_CUS_PROJECTID       "cus_projectid"
#define ENV_CUS_DATAINDEX_DIR   "cus_dataindex_dir"

#define ENV_DATAINDEX_CFG_NAME  "dataindex_cfg_name"

#define PROJECT_ID_PARTITION    "Partition"
#define PROJECT_ID_FILENAME     "project_id.ini"

int get_data_separation_info(const char *partition, const char *file,
                struct project_id_info *info);
/**
 * dataindex_get_info - get dataindex information
 *
 * @info[out]: Pointer of pointer to store the output dataindex info
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_info(struct project_id_info **info);

/**
 * dataindex_get_handle - get ini handle of dataindex file
 *
 * @phandle[out]: Pointer to store the dataindex ini parser handle
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_handle(iniparser_handle_t *phandle);

/**
 * dataindex_get_section - get section handle of dataindex file
 *
 * @pentry[out]: Pointer to store the dataindex section handle
 * @section[in]: the section string
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_get_section(struct section **pentry, const char *section);

/**
 * dataindex_get_keys - get the value of key from dataindex file
 *
 * @dest[out]: the output value buffer
 * @size[in]: the size of output value buffer
 * @section[in]: the section string
 * @section[in]: the key string
 * @default_str[in]: the default string if key is missing
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 *     Notice: default_str is copied if default_str is not NULL and
 *     the the key is missing
 */
int dataindex_get_key(char *dest, int size,
        const char *section, const char *key, const char *default_str);

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
       const char **relpath, const char *path);

/**
 * dataindex_dtbo_overlay - overlay dtbo read from dataindex file
 *
 * @ft_addr[in]: the kernel dtb buffer
 * @mode[in]: the overlay mode. 'u' for uboot and 'k' for kernel
 *
 * Returns 0 if successful,
 *         negative if an error occurs
 */
int dataindex_dtbo_overlay(char *ft_addr, char mode);
#endif
