// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <dtbo.h>
#include <mtk_dtbo.h>
#include <debug_impl.h>
#include <overlay_setting.h>
#include <dtbo_cfg_sel.h>
#ifdef CONFIG_MULTIPLE_DTB_SELECTION
#include <environment.h>
#include <romtblo_impl.h>
#include <iniparser.h>
#include <fs.h>
#include <utility.h>
#include "secure_boot.h"
#include "secure/crypto_auth.h"
#include "secure/secure_common.h"
#include <image-android-dt.h>
#include <mapmem.h>
#include <dt_table.h>
#ifdef CONFIG_DATA_SEPARATION
#include <mtk_dataindex.h>
#endif
#if (CONFIG_AB_SIDELOAD == 1)
#include <mtk_ab.h>
#endif
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#include <linux/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asm/gpio.h>
#ifdef CONFIG_VBYONE_CUSTOMIZED
#include <idme.h>
#include <amzn_tv_common.h>
#include <usb.h>
char new_path[STRING_BUFFER_SIZE];
int usb_file_exist(const char *file_name)
{
	static int iCheckedValue = -1;
	if (iCheckedValue != -1)
	{
		return iCheckedValue;
	}

	char file_path[STRING_BUFFER_SIZE];
	if (snprintf(file_path, sizeof(file_path), "/%s", file_name) > sizeof(file_path)) {
		UBOOT_ERROR("The size of usb file path is too long.\n");
		iCheckedValue = 0;
		return 0;
	}
	run_command("usb start", 0);
	if (!file_exists("usb", "0", file_path, FS_TYPE_ANY))
	{
		UBOOT_ERROR("File not found.\n");
		iCheckedValue = 0;
		run_command("usb stop", 0);
		return 0;
	}
	iCheckedValue = 1;
	run_command("usb stop", 0);
	return 1;
}

void model_name_factory_customized(const char **relpath)
{
	#define FACTORY_NAME_VAR_SIZE 16
	char factory_name[FACTORY_NAME_VAR_SIZE] = "\0";
	idme_get_oem_data_field("fac=", factory_name, FACTORY_NAME_VAR_SIZE);
	if (strcmp(factory_name, "")) {
		if (!strcmp(factory_name, "hisense")) {
			#define HW_BUILD_NAME_NUM 6
			#define HW_BUILD_NAME_LEN 3
			char build_names[HW_BUILD_NAME_NUM][HW_BUILD_NAME_LEN] = {"PRO", "HVT", "EVT", "DVT", "PVT", "NUL"};
			char *build_name_ptr = strchr(*relpath, '/') + 1;
			char *model_name_ptr = strchr(*relpath, '_') + 1;
			if (build_name_ptr == NULL || model_name_ptr == NULL) {
                                UBOOT_ERROR("The format of model_name is wrong.\n");
                                return;
			}
			for (int i = 0; i < HW_BUILD_NAME_NUM; i++)
			{
				if (i == (HW_BUILD_NAME_NUM - 1)) {
					UBOOT_ERROR("The format of model_name is wrong.\n");
					return;
				}
				if (strncmp(build_name_ptr, build_names[i], HW_BUILD_NAME_LEN) == 0) {
					break;
				}
			}
			if (snprintf(new_path, sizeof(new_path), "dtb/FAC_%s", model_name_ptr) > sizeof(new_path)) {
                                UBOOT_ERROR("The size of dtbo cfg file path is too long.\n");
                                return;
                        }
			if (usb_file_exist(CONFIG_VBYONE_FLAG_NAME)) {
				*relpath = new_path;
				UBOOT_DEBUG("The usb flag exists and model_name updated to %s.\n", *relpath);
			}
		}
	}
}
#endif

static iniparser_handle_t dtbo_mapping = NULL;
static iniparser_handle_t dtbo_selection = NULL;
static iniparser_handle_t dtbo_whitelist = NULL;
static unsigned char *mapping_ini = NULL;
static unsigned char *selection_ini = NULL;
static unsigned char *whitelist_ini = NULL;
static unsigned int mapping_ini_size = 0;
static unsigned int selection_ini_size = 0;
static unsigned int whitelist_ini_size = 0;
static struct list_head dtbo_idx_list;


extern unsigned long long get_dram_size(void);


#define DTBO_CALLBACK_MAX_NUM (32)
#define DTBO_CALLBACK_NAME_MAX_LEN (64)
typedef int (*dtbo_callback)(char*,int);

static dtbo_callback dtbo_callbacks[DTBO_CALLBACK_MAX_NUM] = {0};
static char dtbo_callback_section_name[DTBO_CALLBACK_MAX_NUM][DTBO_CALLBACK_NAME_MAX_LEN] = {{0}};
static unsigned int cur_dtbo_callback_number = 0;

static const char *_get_default_dtbo_cfg(const char *default_path)
{
	static char cfg_path[COMMAND_BUF_SIZE] = { 0 };
	u32 chipid = romtbl_get_chip_id_info_dec();
	u32 revision = romtbl_get_chip_revision_info();

	UBOOT_TRACE("IN\n");
	if (chipid == 0 || revision == 0) {
		UBOOT_ERROR("Wrong chip id/revision = %d/%d\n", chipid, revision);
		return default_path;
	}
    if (dtbo_cfg_sel_get_pcb(cfg_path, sizeof(cfg_path), chipid, revision))
    {
        UBOOT_TRACE("OUT\n");
        return (const char *)cfg_path;
    }
	UBOOT_TRACE("OUT\n");
    return default_path;
}

static int get_device_and_partition_info(char *partition, unsigned int device, char *device_name, char *storage_info)
{
	int ret;

	UBOOT_TRACE("IN\n");
	ret = sys_get_boot_device(device_name, STORAGE_DEVICE_BUF_SIZE);
	if(ret < 0){
		UBOOT_ERROR("Error: Get booting device name failure, Unknown storage device.\n");
		return -1;
	}

	ret = sys_get_storage_info(device,partition,storage_info);
	if(ret < 0){
		UBOOT_ERROR("Error: sys_get_storage_info failure\n");
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

static int dtb_cfg_exist_check(char *partition, char *path)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	unsigned int device = 0;
	char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
	int ret;
#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
#endif

	UBOOT_TRACE("IN\n");
	ret = get_device_and_partition_info(partition, device, device_name, storage_info);
	if(ret < 0){
		UBOOT_ERROR("Error: Get device & partition failure, Unknown storage & partition device.\n");
		return -1;
	}

#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
	if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT)){
#if defined(CONFIG_MULTICORES_PLATFORM)
		smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		UBOOT_ERROR("Error: partition select failure\n");
		return -1;
	}

	if (fs_exists(path) == 0){
#if defined(CONFIG_MULTICORES_PLATFORM)
		smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		UBOOT_ERROR("Error: %s/%s file does not exist\n",partition,path);
		return -1;
	}
#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif

	UBOOT_TRACE("OUT\n");
	return 0;
}

#define DTBO_CFG_BUFFER_SIZE 128
static int get_customization_config_element(char *type, char *setting)
{
    struct section *section_entry;
    struct keyval_pair *keyval_entry;

    UBOOT_TRACE("IN\n");
    UBOOT_DEBUG("type=%s", type);
    iniparser_for_each_section(dtbo_selection, section_entry)
    {
        UBOOT_DEBUG("classification(%s):", section_entry->section);
        iniparser_for_each_keyval(section_entry, keyval_entry)
        {
            UBOOT_DEBUG("[%s]", keyval_entry->key);
            if (!strcmp(type, keyval_entry->key))
            {
                if (strlen(keyval_entry->value) < DTBO_CFG_BUFFER_SIZE)
                {
                    strncpy(setting, keyval_entry->value, strlen(keyval_entry->value));
                    setting[strlen(keyval_entry->value)] = '\0';
                }
                else
                {
                    strncpy(setting, keyval_entry->value, DTBO_CFG_BUFFER_SIZE - 1);
                    setting[DTBO_CFG_BUFFER_SIZE - 1] = '\0';
                }
                UBOOT_TRACE("OUT\n");
                return 0;
            }
        }
    }
    UBOOT_TRACE("OUT\n");
    return -1;
}

static int get_dtbo_mapping_table_info(char *partition, const char *path)
{
	int ret;
	loff_t size;
	unsigned char *file_buf;
	UBOOT_TRACE("IN\n");

	if(mapping_ini == NULL){
		file_buf = read_storage_file_to_memory(partition, path, &size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: %s/%s parse error occur\n",partition, path);
			return -1;
		}
		mapping_ini = (unsigned char *)malloc(size);
		if(mapping_ini != NULL){
			memcpy(mapping_ini, file_buf, (unsigned int)size);
			mapping_ini_size = (unsigned int)size;
		}
	}else{
		file_buf = (unsigned char *)malloc(mapping_ini_size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: allocate file_buf failure\n");
			return -1;
		}
		memcpy(file_buf, mapping_ini, (unsigned int)mapping_ini_size);
		size = mapping_ini_size;
	}

	if (dtbo_mapping){
		UBOOT_INFO("dtbo_mapping exist, destory and renew.\n");
		iniparser_destroy(dtbo_mapping);
	}

	ret = iniparser_create(file_buf, size, &dtbo_mapping);
	free(file_buf);

	if(ret < 0){
		UBOOT_ERROR("Error: dtbo_mapping_table iniparser_create failure\n");
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

static int get_dtbo_customization_config_info(char *partition, const char *path)
{
	int ret;
	loff_t size;
	unsigned char *file_buf;

	UBOOT_TRACE("IN\n");
	if(selection_ini == NULL){
		file_buf = read_storage_file_to_memory(partition, path, &size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: %s/%s parse error occur\n",partition, path);
			return -1;
		}
		selection_ini = (unsigned char *)malloc(size);
		if(selection_ini != NULL){
			memcpy(selection_ini, file_buf, (unsigned int)size);
			selection_ini_size = (unsigned int)size;
		}
	}else{
		file_buf = (unsigned char *)malloc(selection_ini_size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: allocate file_buf failure\n");
			return -1;
		}
		memcpy(file_buf, selection_ini, (unsigned int)selection_ini_size);
		size = selection_ini_size;
	}

	if (dtbo_selection){
		UBOOT_INFO("dtbo_mapping exist, destory and renew.\n");
		iniparser_destroy(dtbo_selection);
	}

	ret = iniparser_create(file_buf, size, &dtbo_selection);
	free(file_buf);

	if(ret < 0){
		UBOOT_ERROR("Error: dtbo_customization_config iniparser_create failure\n");
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

static int get_dtbo_whitelist_info(char *partition, const char *path)
{
	int ret;
	loff_t size;
	unsigned char *file_buf;

	UBOOT_TRACE("IN\n");
	if(whitelist_ini == NULL){
		file_buf = read_storage_file_to_memory(partition, path, &size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: %s/%s parse error occur\n",partition, path);
			return -1;
		}
		whitelist_ini = (unsigned char *)malloc(size);
		if(whitelist_ini != NULL){
			memcpy(whitelist_ini, file_buf, (unsigned int)size);
			whitelist_ini_size = (unsigned int)size;
		}
	}else{
		file_buf = (unsigned char *)malloc(whitelist_ini_size);
		if(file_buf == NULL){
			UBOOT_ERROR("Error: allocate file_buf failure\n");
			return -1;
		}
		memcpy(file_buf, whitelist_ini, (unsigned int)whitelist_ini_size);
		size = whitelist_ini_size;
	}

	if (dtbo_whitelist){
		UBOOT_INFO("dtbo_whitelist exist, destory and renew.\n");
		iniparser_destroy(dtbo_whitelist);
	}

	ret = iniparser_create(file_buf, size, &dtbo_whitelist);
	free(file_buf);

	if(ret < 0){
		UBOOT_ERROR("Error: dtbo_whitelist iniparser_create failure\n");
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

#ifdef CONFIG_DATA_SEPARATION
static int read_dtbo_ini_info(enum INI_FILE_TYPE type, char *section, char *key, char *part, char *env_name, char *default_ini)
#else
static int read_dtbo_ini_info(enum INI_FILE_TYPE type, char *part, char *env_name, char *default_ini)
#endif
{
	int ret = -1, len = 0;
#ifdef CONFIG_DATA_SEPARATION
	char filepath[FILE_PATH_SIZE];
#else
	char filepath[STRING_BUFFER_SIZE];
#endif
	const char *ptr = NULL;

#ifdef CONFIG_DATA_SEPARATION
	char partition[STRING_BUFFER_SIZE];
	const char *relpath = NULL;
	memset(partition, 0, sizeof(partition));
	memset(filepath, 0, sizeof(filepath));

	/* TODO: remove this dtbo_cfg_sel env rule for one package
	 *       when auto-selection dtbo_cfg_sel for data separation is ready.
	 **/
	if (type == CFG_SELECT) {
		relpath = env_get(env_name);
#ifdef CONFIG_VBYONE_CUSTOMIZED
		char bootmode[COMMAND_BUF_SIZE] = "\0";
		if (idme_get_var_external("bootmode", bootmode, sizeof(bootmode) - 1)){
			UBOOT_ERROR("idme bootmode read failed! Skip vbyone customization.\n");
		}
		else {
			if (simple_strtoul(bootmode, NULL, 10) == IDME_BOOTMODE_DIAG) {
				model_name_factory_customized(&relpath);
			}
		}
#endif
		if (relpath) {
			if (!strncmp(relpath, DEFAULT_DTB_CFG_LABEL, sizeof(DEFAULT_DTB_CFG_LABEL)))
				relpath = _get_default_dtbo_cfg(relpath);
			strncpy(partition, part, sizeof(partition));
			partition[sizeof(partition)-1] = 0;
			UBOOT_DEBUG("use %s env to get %s\n", env_name, relpath);
		}
	}

	if (!relpath) {
		if(dataindex_get_key(filepath, FILE_PATH_SIZE, section, key, NULL) != 0)
		{
			UBOOT_ERROR("[DATA_SEPARATION] Get filepath failure base on %s:%s.\n",section,key);
			goto DATA_SEPARATION_FAIL;
		}

		if(dataindex_resolve_path(partition, PART_NAME_SIZE, &relpath, filepath) != 0)
		{
			UBOOT_ERROR("[DATA_SEPARATION] Get partition, relpath failure base on filepath:%s.\n",filepath);
			goto DATA_SEPARATION_FAIL;
		}
	}

	if(type == DTB_MAPPING)
		ret = get_dtbo_mapping_table_info(partition,relpath);
	else if(type == WHITELIST)
		ret = get_dtbo_whitelist_info(partition,relpath);
	else if(type == CFG_SELECT)
		ret = get_dtbo_customization_config_info(partition,relpath);
	else
		UBOOT_ERROR("[DATA_SEPARATION] Input INI_FILE_TYPE type is not support.\n");
	if(ret < 0){
		UBOOT_ERROR("[DATA_SEPARATION][INI_FILE_TYPE:%d] Parse %s file under %s failure.\n",type,relpath,partition);
		goto DATA_SEPARATION_FAIL;
	}

	return 0;
DATA_SEPARATION_FAIL:
#endif
	memset(filepath, 0, sizeof(filepath));
	ptr = env_get(env_name);

	if(type == CFG_SELECT){
		if(ptr == NULL){
			ptr = _get_default_dtbo_cfg(ptr);
		}else if(!strncmp(ptr, DEFAULT_DTB_CFG_LABEL, sizeof(DEFAULT_DTB_CFG_LABEL))){
			ptr = _get_default_dtbo_cfg(ptr);
		}
	}
	if(ptr == NULL){
		UBOOT_DEBUG("%s env is not set, use default %s.\n",env_name,default_ini);
		ptr = default_ini;
	}
	if(sizeof(filepath) <= strlen(ptr))
		len = sizeof(filepath)-1;
	else
		len = strlen(ptr);
	if(len > 0){
		strncpy(filepath, ptr, len);
		filepath[len] = '\0';
	}else{
		UBOOT_ERROR("filepath string length is not correct.\n");
		return -1;
	}

	if(type == DTB_MAPPING)
		ret = get_dtbo_mapping_table_info(part,filepath);
	else if(type == WHITELIST)
		ret = get_dtbo_whitelist_info(part,filepath);
	else if(type == CFG_SELECT)
		ret = get_dtbo_customization_config_info(part,filepath);
	else
		UBOOT_ERROR("Input INI_FILE_TYPE type is not support.\n");
	if(ret < 0){
		UBOOT_ERROR("[INI_FILE_TYPE:%d] Parse %s file under %s failure.\n",type,filepath,part);
		return -1;
	}
	return 0;
}

static void add_dtbo_idx_to_bootargs(int kernel_dtb_num)
{
	DTBO_IDX_T *pos, *n;
	char bootargs_dtbo_idx[] = ANDROID_BOOTARGS_DTBO_IDX;
	char key[KEY_BUFFER_SIZE]={0},set_cfg[STRING_BUFFER_SIZE]={0},id_str[DIGIT_NUM];
	char *key_value, *ret;
	int len;

	strncpy(key,bootargs_dtbo_idx,strlen(bootargs_dtbo_idx));
	key[strlen(bootargs_dtbo_idx)] = '\0';
	if(kernel_dtb_num > 0){
		key_value = (char *)malloc(kernel_dtb_num*DIGIT_NUM);
		if(key_value != NULL){
			memset(key_value, 0, kernel_dtb_num*DIGIT_NUM);
			list_for_each_entry_safe(pos, n, &dtbo_idx_list, list){
				memset(id_str, 0, sizeof(id_str));
				len = snprintf(id_str, sizeof(id_str)-1, "%d", pos->dtbo_id);
				if(len > 0){
					ret = strncat(key_value, id_str, strlen(id_str));
					if(ret == NULL)
						UBOOT_DEBUG("Set key_value failure\n");
					UBOOT_DEBUG("Set key_value:%s as %s\n",ANDROID_BOOTARGS_DTBO_IDX,key_value);
					kernel_dtb_num--;
					if(kernel_dtb_num > 0)
						strncat(key_value, ",", 1);
				}
			}
			len = snprintf(set_cfg, sizeof(set_cfg)-1,"%s=%s",key,key_value);
			if (len >= (sizeof(set_cfg)-1))
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(set_cfg)-1), set_cfg);
			free(key_value);
		}else{
			UBOOT_ERROR("Heap memory is not enough\n");
			len = snprintf(set_cfg, sizeof(set_cfg)-1,"%s=%d",key,0);
			if (len >= (sizeof(set_cfg)-1))
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(set_cfg)-1), set_cfg);
		}
	}else{
		UBOOT_ERROR("kernel_dtb_num is zero, kernel dtb overlay selection failure occur\n");
		len = snprintf(set_cfg, sizeof(set_cfg)-1,"%s=%d",key,0);
			if (len >= (sizeof(set_cfg)-1))
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(set_cfg)-1), set_cfg);
	}

	add_bootargs(key,set_cfg,0);
	list_for_each_entry_safe(pos, n, &dtbo_idx_list, list)
		free(pos);
}

int get_dtbo_cfg_selection(char *partition, char *cfg_name, char *type, char *setting)
{
	int ret;

	UBOOT_TRACE("IN\n");
	if(cfg_name == NULL){
		UBOOT_ERROR("cfg_name name is NULL\n");
		return -1;
	}

	ret = get_dtbo_customization_config_info(partition,cfg_name);
	if(ret < 0){
		UBOOT_ERROR("Parse %s file failure\n",cfg_name);
		return -1;
	}

	ret = get_customization_config_element(type, setting);
	if(ret < 0){
		UBOOT_ERROR("Error: Get type and setting error occur\n");
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

int set_dtbo_cfg_selection(char *partition, char *cfg_name)
{
	UBOOT_TRACE("IN\n");
	if(cfg_name == NULL){
		UBOOT_ERROR("cfg_name name is NULL\n");
		return -1;
	}

	if(dtb_cfg_exist_check(partition, cfg_name) != 0){
		UBOOT_ERROR("Error: User select cfg not exist\n");
		return -1;
	}

	env_set("dtbo_cfg_sel",cfg_name);
	env_save();
	UBOOT_TRACE("OUT\n");
	return 0;
}

int ls_dtbo_cfg(char *partition)
{
	char device_name[STORAGE_DEVICE_BUF_SIZE] = {0};
	unsigned int device = 0;
	char storage_info[STORAGE_INDEX_BUF_SIZE] = {0};
	char folder_path[STORAGE_DEVICE_BUF_SIZE] = {0};
	int ret;
#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
#endif

	UBOOT_TRACE("IN\n");
	strncpy(folder_path,"dtb", sizeof(folder_path));
	ret = get_device_and_partition_info(partition, device, device_name, storage_info);
	if(ret < 0){
		UBOOT_ERROR("Error: Get device & partition failure, Unknown storage & partition device.\n");
		return -1;
	}

#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
	if (fs_set_blk_dev(device_name, storage_info, FS_TYPE_EXT)){
#if defined(CONFIG_MULTICORES_PLATFORM)
		smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		UBOOT_ERROR("Error: partition select failure\n");
		return -1;
	}

	if (fs_ls(folder_path)){
#if defined(CONFIG_MULTICORES_PLATFORM)
		smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		UBOOT_ERROR("Error: file list failure\n");
		return -1;
	}
#if defined(CONFIG_MULTICORES_PLATFORM)
	smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
	UBOOT_TRACE("OUT\n");
	return 0;
}

static dtbo_callback dtbo_get_auto_detection_callback(char* section_name)
{
	unsigned int i = 0;

	if (section_name == NULL)
		return NULL;

	for (i = 0; i< cur_dtbo_callback_number; i++){
		if (strncmp(section_name, dtbo_callback_section_name[i], DTBO_CALLBACK_NAME_MAX_LEN) == 0){
			return dtbo_callbacks[i];
		}
	}

	return NULL;
}

int get_device_id(char *classification, char *static_name)
{
	unsigned long id;
	struct section *section_entry;
	struct keyval_pair *keyval_entry;
	char* name = static_name;
	char detected_name[DTBO_CALLBACK_NAME_MAX_LEN] = {0};
	dtbo_callback callback = NULL;

	UBOOT_TRACE("IN\n");
	UBOOT_DEBUG("[dtbo_class=%s, dtbo_name=%s]\n", classification, static_name);

	callback = dtbo_get_auto_detection_callback(classification);

	if (callback){
		if (callback(detected_name, DTBO_CALLBACK_NAME_MAX_LEN)){
			UBOOT_DEBUG("auto_detection failed @ %s \n", classification);
		} else {
			name = detected_name;
			UBOOT_DEBUG("use detected name %s\n", name);
		}
	}

	iniparser_for_each_section(dtbo_mapping, section_entry) {
		if (strncmp(section_entry->section, classification, strlen(classification)) == 0 && strlen(section_entry->section) == strlen(classification)) {
			iniparser_for_each_keyval(section_entry, keyval_entry) {
				UBOOT_INFO("[%s][%s=%s]\n", section_entry->section, keyval_entry->key, keyval_entry->value);
				if (strncmp(keyval_entry->key, name, strlen(name)) == 0 && strlen(keyval_entry->key) == strlen(name)) {
					id = simple_strtoul(keyval_entry->value, NULL, DIGIT_NUM);
					UBOOT_DEBUG("[dtbo_class=%s, dtbo_name=%s][dtbo_id=%lu]\n",
								classification, keyval_entry->key, id);
					UBOOT_TRACE("OUT\n");
					return id;
				}
			}
			UBOOT_ERROR("[dtbo_class=%s, dtbo_name=%s][not found!]\n",
						classification, name);
			UBOOT_TRACE("OUT\n");
			return -1;
		}
	}
	UBOOT_ERROR("[dtbo_class=%s, dtbo_name=%s][not found!]\n",
				classification, name);
	UBOOT_TRACE("OUT\n");
	return -1;
}

/*
 return 0: success
 return otherwise: fail
*/
static int mmap_auto_detection_callback(char* detected_name, int size)
{
#ifdef CONFIG_ANDROID_CN_PLATFORM
    static unsigned long long dram_size = 1;
    UBOOT_DEBUG("MMAP auto detection - AOSP\n");
    {
        dram_size = get_dram_size();
        memset(detected_name, 0, size);

        if (MT5879 == romtbl_get_chip_id_info())
        {
#ifdef AOSP_ONE_IMAGE_SUPPORT
            if (dram_size == 0x100000000ULL) //4GB
            {
                UBOOT_DEBUG("WARNING: DRAM size is 4GB\n");
#ifdef COMMERCIAL_DISPLAY_SUPPORT
                strncpy(detected_name, "mmap_mt5879_h1v1_aosp_4g_iommu_cd", size-1);
#else
                strncpy(detected_name, "mmap_mt5879_h1v1_aosp_4g_iommu", size-1);
#endif
                return 0;
            }
            else if (dram_size == 0xC0000000ULL) //3GB
            {
                UBOOT_DEBUG("WARNING: DRAM size is 3GB\n");
#ifdef COMMERCIAL_DISPLAY_SUPPORT
                strncpy(detected_name, "mmap_mt5879_h1v1_aosp_3g_iommu_cd", size-1);
#else
                strncpy(detected_name, "mmap_mt5879_h1v1_aosp_3g_iommu", size-1);
#endif
                return 0;
            }
#else
            UBOOT_DEBUG("AOSP_ONE_IMAGE_SUPPORT is not supported\n");
#endif
        }
        else
        {
            if (dram_size == 0x180000000ULL) //6GB
            {
                UBOOT_DEBUG("WARNING: DRAM size is 6GB\n");
                strncpy(detected_name, "mmap_mt5896_h1v1_an_6g", size-1);
                return 0;
            }
            else if (dram_size == 0x200000000ULL) //8GB
            {
                UBOOT_DEBUG("WARNING: DRAM size is 8GB\n");
                strncpy(detected_name, "mmap_mt5896_h1v1_an_8g", size-1);
                return 0;
            }
        }

        printf("WARNING: DRAM auto detection failed! Un-supported DRAM size = 0x%llx\n", dram_size);
        return 1;
    }
#else
	UBOOT_DEBUG("MMAP auto detection\n");
	UBOOT_DEBUG("To be implemented\n");
	return 1;
#endif
}

/*
 return 0: success
 return otherwise: fail
*/
static int reserved_memory_auto_detection_callback(char* detected_name, int size)
{
#ifdef CONFIG_ANDROID_CN_PLATFORM
	static unsigned long long dram_size = 1;
	UBOOT_DEBUG("resv_mem auto detection - AOSP\n");
	{
		dram_size = get_dram_size();
		memset(detected_name, 0, size);

		if (MT5879 == romtbl_get_chip_id_info())
		{
#ifdef AOSP_ONE_IMAGE_SUPPORT
			if (dram_size == SIZE_4G)
			{
				UBOOT_DEBUG("WARNING: DRAM size is 4GB\n");
				strlcpy(detected_name, "resv_mem_mt5879_h1v1_aosp_4g_iommu", size-1);
				return 0;
			}
			else if (dram_size == SIZE_3G)
			{
				UBOOT_DEBUG("WARNING: DRAM size is 3GB\n");
				strlcpy(detected_name, "resv_mem_mt5879_h1v1_aosp_3g_iommu", size-1);
				return 0;
			}
#else
			UBOOT_DEBUG("AOSP_ONE_IMAGE_SUPPORT is not supported\n");
#endif
		}
		else if (MT5896 == romtbl_get_chip_id_info())
		{
			if (dram_size == SIZE_6G)
			{
				UBOOT_DEBUG("WARNING: DRAM size is 6GB\n");
				strlcpy(detected_name, "resv_mem_mt5896_h1v1_an_6g", size-1);
				return 0;
			}
			else if (dram_size == SIZE_8G)
			{
				UBOOT_DEBUG("WARNING: DRAM size is 8GB\n");
				strlcpy(detected_name, "resv_mem_mt5896_h1v1_an_8g", size-1);
				return 0;
			}
		}

		UBOOT_ERROR("WARNING: DRAM auto detection (resv_mem) failed! Un-supported DRAM size = 0x%llx\n", dram_size);
		return 1;
	}
#else
	/* non-aosp project, no need to select resv_mem */
	return 1;
#endif
}

/*
 return 0: success
 return otherwise: fail
*/
static int iommu_cust_auto_detection_callback(char* detected_name, int size)
{
#ifdef CONFIG_ANDROID_CN_PLATFORM
	static unsigned long long dram_size = 1;
	int chip_id = -1;
	int chip_number;
	int len = 0;
	UBOOT_DEBUG("iommu_cust auto detection - AOSP\n");

	dram_size = get_dram_size();
	memset(detected_name, 0, size);

	chip_id = romtbl_get_chip_id_info();
	switch (chip_id)
	{
		case MT5896:
			chip_number = MEDIATEK_TV_CHIP_ID_MACHLI;
			break;
		case MT5897:
			chip_number = MEDIATEK_TV_CHIP_ID_MANKS;
			break;
		case MT5876:
			chip_number = MEDIATEK_TV_CHIP_ID_MOKONA;
			break;
		case MT5879:
			chip_number = MEDIATEK_TV_CHIP_ID_MIFFY;
			break;
		case MT5873:
			chip_number = MEDIATEK_TV_CHIP_ID_MOKA;
			break;
		default:
			UBOOT_ERROR("Chip id %d is not support\n", chip_id);
			return 1;
	}

	switch (dram_size)
	{
		case SIZE_8G:
			len = snprintf(detected_name, size - 1, "mt%d_%s", chip_number, "8g");
			break;
		case SIZE_6G:
			len = snprintf(detected_name, size - 1, "mt%d_%s", chip_number, "6g");
			break;
		case SIZE_4G:
			len = snprintf(detected_name, size - 1, "mt%d_%s", chip_number, "4g");
			break;
		case SIZE_3G:
			len = snprintf(detected_name, size - 1, "mt%d_%s", chip_number, "3g");
			break;
		case SIZE_2G:
			len = snprintf(detected_name, size - 1, "mt%d_%s", chip_number, "2g");
			break;
		default:
			UBOOT_ERROR("ERROR: DRAM auto detection (iommu_cust) failed! Un-supported DRAM size = 0x%llx\n", dram_size);
		return 1;
	}

	if (len >= size - 1) {
		UBOOT_ERROR("ERROR: DRAM auto detection (iommu_cust) over sized, len: %d\n", len);
		return 1;
	}

	UBOOT_DEBUG("DEBUG: iommu_cust select name: %s.dtso\n", detected_name);
	return 0;
#else
	UBOOT_DEBUG("iommu_cust auto detection\n");
	UBOOT_DEBUG("To be implemented\n");
	return 1;
#endif
}

static int dtbo_register_auto_detection_callback(char* section_name, dtbo_callback cb)
{
	if (cur_dtbo_callback_number >= DTBO_CALLBACK_MAX_NUM){
		UBOOT_ERROR("dtbo_register_auto_detection_callback failed: no empty slot\n");
		return 1;
	}

	if (section_name == NULL){
		UBOOT_ERROR("dtbo_register_auto_detection_callback failed: invalid section name\n");
		return 1;
	}

	if (strnlen(section_name, DTBO_CALLBACK_NAME_MAX_LEN) == DTBO_CALLBACK_NAME_MAX_LEN){
		UBOOT_ERROR("dtbo_register_auto_detection_callback failed: section name too long\n");
		return 1;
	}

	//here we don't check conflict section names
	//for callbacks with conflict names, the first registered callback will be called

	dtbo_callbacks[cur_dtbo_callback_number] = cb;
	memset(dtbo_callback_section_name[cur_dtbo_callback_number], 0, DTBO_CALLBACK_NAME_MAX_LEN);
	strncpy(dtbo_callback_section_name[cur_dtbo_callback_number], section_name, DTBO_CALLBACK_NAME_MAX_LEN-1);
	cur_dtbo_callback_number++;

	return 0;
}

static int dtbo_init_auto_detection_callbacks(void)
{
	static int inited = 0;

	if (inited)
		return 0;

	inited = 1;

	if (dtbo_register_auto_detection_callback("mmap", mmap_auto_detection_callback))
		UBOOT_INFO("Warning: mmap register failed\n");

	if (dtbo_register_auto_detection_callback("resv_mem", reserved_memory_auto_detection_callback))
		UBOOT_INFO("Warning: resv_mem register failed\n");

	if (dtbo_register_auto_detection_callback("iommu_cust", iommu_cust_auto_detection_callback))
		UBOOT_INFO("Warning: iommu_cust register failed\n");

	return 0;
}

#else
int is_device_id_valid(int device_id)
{
	UBOOT_TRACE("IN\n");
	if(device_id < 0)
	{
		return -1;
	}
	UBOOT_TRACE("OUT\n");
	return 0;
}

int get_device_id(int device_type)
{
	int device_id;
	UBOOT_TRACE("IN\n");
	switch(device_type)
	{
		case DEVICE_BOARD:
			device_id = 0;
			break;
		default:
			device_id = -1;
			break;
	}
	UBOOT_TRACE("OUT\n");
	return device_id;
}
#endif

int dtbo_selection_and_overlay(void)
{
	int ret = -1;
	int id;
	UBOOT_TRACE("IN\n");
#ifdef CONFIG_MULTIPLE_DTB_SELECTION
	const char *ptr;
	char partition[STRING_BUFFER_SIZE];
	struct section *section_entry, *section_entry_wl;
	struct keyval_pair *keyval_entry, *keyval_entry_wl;
	int len, kernel_dtb_num = 0;
	bool uboot_dtbo = false;
	DTBO_IDX_T *dtbo_idx = NULL;

	if (dtbo_init_auto_detection_callbacks())
		UBOOT_ERROR("Warning: dtbo_init_auto_detection_callbacks failed\n");

	memset(partition, 0, sizeof(partition));
	ptr = env_get(DTBO_DEF_ENV_NAME);
	if(ptr == NULL){
		UBOOT_DEBUG("Partition not select, use default %s\n",DTBO_DEF_PART);
		strncpy(partition, DTBO_DEF_PART, strlen(DTBO_DEF_PART));
	}else{
#if (CONFIG_AB_SIDELOAD == 1)
		if(sizeof(partition) <= (strlen(ptr) + strlen(PART_SUFFIX_A)))
			len = sizeof(partition)- strlen(PART_SUFFIX_A) - 1;
#else
		if(sizeof(partition) <= strlen(ptr))
			len = sizeof(partition)-1;
#endif
		else
			len = strlen(ptr);
		strncpy(partition, ptr, len);
	}

#ifdef CONFIG_DATA_SEPARATION
	ret = read_dtbo_ini_info(DTB_MAPPING, DTB_OVERLAY_SECTION, MAPPING_CFG_KEY, partition, MAPPING_CFG_ENV_NAME, MAPPING_CFG_DEF_PATH);
#else
	ret = read_dtbo_ini_info(DTB_MAPPING, partition, MAPPING_CFG_ENV_NAME, MAPPING_CFG_DEF_PATH);
#endif
	if(ret < 0){
		UBOOT_ERROR("DTB_MAPPING ini read failure.\n");
		return -1;
	}

#ifdef CONFIG_DATA_SEPARATION
	ret = read_dtbo_ini_info(WHITELIST, DTB_OVERLAY_SECTION, WHITELIST_KEY, partition, WHITELIST_ENV_NAME, WHITELIST_DEF_PATH);
#else
	ret = read_dtbo_ini_info(WHITELIST, partition, WHITELIST_ENV_NAME, WHITELIST_DEF_PATH);
#endif
	if(ret < 0){
		UBOOT_ERROR("WHITELIST ini read failure.\n");
		return -1;
	}

#ifdef CONFIG_DATA_SEPARATION
	ret = read_dtbo_ini_info(CFG_SELECT, DTB_OVERLAY_SECTION, CFG_SEL_KEY, partition, CFG_SEL_ENV_NAME, CFG_SEL_DEF_PATH);
#else
	ret = read_dtbo_ini_info(CFG_SELECT, partition, CFG_SEL_ENV_NAME, CFG_SEL_DEF_PATH);
#endif
	if(ret < 0){
		UBOOT_ERROR("CFG_SELECT ini read failure.\n");
		return -1;
	}

	uboot_dtbo = is_uboot_dtb();
	if(uboot_dtbo == false)
		INIT_LIST_HEAD(&dtbo_idx_list);
	UBOOT_DEBUG("%s\n", uboot_dtbo?"Doing u-boot dtb overlay":"Doing kernel dtb overlay");
	iniparser_for_each_section(dtbo_selection, section_entry) {
		iniparser_for_each_keyval(section_entry, keyval_entry) {
			UBOOT_DEBUG("[%s=%s]\n", keyval_entry->key, keyval_entry->value);
			iniparser_for_each_section(dtbo_whitelist, section_entry_wl) {
				iniparser_for_each_keyval(section_entry_wl, keyval_entry_wl) {
					UBOOT_DEBUG("[%s=%s]\n", keyval_entry_wl->key, keyval_entry_wl->value);
					if(strncmp(keyval_entry_wl->key, keyval_entry->key, strlen((keyval_entry->key))) == 0 && strlen(keyval_entry_wl->key) == strlen(keyval_entry->key)){
						if((uboot_dtbo == true && (strncmp(keyval_entry_wl->value, "U", 1) == 0 || strncmp(keyval_entry_wl->value, "*", 1) == 0))
						|| (uboot_dtbo == false && (strncmp(keyval_entry_wl->value, "K", 1) == 0 || strncmp(keyval_entry_wl->value, "*", 1) == 0))){
							UBOOT_DEBUG("[%s=%s]\n", keyval_entry_wl->key, keyval_entry_wl->value);
							id = get_device_id(keyval_entry->key, keyval_entry->value);
							if(id < 0){
								UBOOT_ERROR("dtb_overlay id:%d is invaild\n",id);
								return -1;
							}

							ret = dtb_overlay(id);
							if(ret != 0){
								UBOOT_ERROR("dtb_overlay execute failure with %d\n",id);
								return -1;
							}
							if(uboot_dtbo == false){
								dtbo_idx = (DTBO_IDX_T *)malloc(sizeof(DTBO_IDX_T));
								if(dtbo_idx != NULL){
									kernel_dtb_num++;
									dtbo_idx->dtbo_id = id;
									UBOOT_DEBUG("[%d] add dtbo_id:%d\n", kernel_dtb_num, dtbo_idx->dtbo_id);
									list_add_tail(&dtbo_idx->list,&dtbo_idx_list);
								}
							}
						}else{
							UBOOT_DEBUG("%s value %s not match\n", keyval_entry_wl->key, keyval_entry_wl->value);
						}
					}else{
						UBOOT_DEBUG("Target:%s Source:%s\n", keyval_entry->key, keyval_entry_wl->key);
					}
				}
			}
		}
	}
	if(uboot_dtbo == false)
		add_dtbo_idx_to_bootargs(kernel_dtb_num);

	iniparser_destroy(dtbo_selection);
	iniparser_destroy(dtbo_whitelist);
	iniparser_destroy(dtbo_mapping);
	dtbo_selection = NULL;
	dtbo_whitelist = NULL;
	dtbo_mapping = NULL;
	if(uboot_dtbo == false){
		if(mapping_ini != NULL)
			free(mapping_ini);
		if(selection_ini != NULL)
			free(selection_ini);
		if(whitelist_ini != NULL)
			free(whitelist_ini);
	}
#else
	int i;
	for (i=0;i<DEVICE_MAX;i++){
		id = get_device_id(i);
		ret = is_device_id_valid(id);
		if(ret == 0){
			ret = dtb_overlay(id);
			if(ret != 0){
				printf("dtb_overlay execute failure with %d\n",id);
				return -1;
			}
		} else {
			printf("DEVICE_TYPE %X is invalid\n",i);
			return -1;
		}
	}
#endif
	ret = overlay_ini_settings();
	if (ret < 0)
		UBOOT_DEBUG("Maybe lost wdt ini , please check\n");

	UBOOT_TRACE("OUT\n");
	return 0;
}

int get_dtbo_img_size(unsigned long dtbo_img_addr, unsigned long *dtbo_img_size)
{
	int ret = 0;
	const struct dt_table_header *hdr;
	u32 image_size;

	ret = android_dt_check_header(dtbo_img_addr);
	if(ret == false)
	{
		UBOOT_ERROR("Check device tree binary image header magic number failure, please check dtbo.img or storage cmd.\n");
		return -EINVAL;
	}

	hdr = map_sysmem(dtbo_img_addr, sizeof(*hdr));
	image_size = fdt32_to_cpu(hdr->total_size);
	unmap_sysmem(hdr);
	*dtbo_img_size = image_size;

	return 0;
}

static int get_public_key_n(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = {};

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_N_LEN) {
		UBOOT_ERROR("The len is smaller than the lenght of public-N (%d bytes).\n", RSA_PUBLIC_KEY_N_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

static int get_public_key_e(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = {};

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_E_LEN) {
		UBOOT_ERROR("The len is smaller than the lenght of public-E (%d bytes).\n", RSA_PUBLIC_KEY_E_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data + RSA_PUBLIC_KEY_N_LEN, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

int verify_dtbo_img(unsigned char *auth_data, unsigned char *sign_data, unsigned long auth_size)
{
	int ret = 0;
	uint8_t rsa_public_key_N[RSA_PUBLIC_KEY_N_LEN] = {0};
	uint8_t rsa_public_key_E[RSA_PUBLIC_KEY_E_LEN] = {0};

	ret = get_public_key_n(KEY_TYPE_CKB_A_RI_KEY, rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);

	if(ret){
		UBOOT_ERROR("Fail to get public N from CKB_A_RI_KEY.\n");
		return CMD_RET_FAILURE;
	}

	ret = get_public_key_e(KEY_TYPE_CKB_A_RI_KEY, rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);

	if(ret){
		UBOOT_ERROR("Fail to get public E from CKB_A_RI_KEY.\n");
		return CMD_RET_FAILURE;
	}

	if ((ret = secure_do_authentication(rsa_public_key_N, rsa_public_key_E, sign_data, auth_data, auth_size)) != 0) {
		UBOOT_ERROR("secure_do_authentication fail.\n");
		UBOOT_DEBUG("RSA_PUBLIC_KEY_N_LEN:\n");
		UBOOT_DUMP((void *)rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN);
		UBOOT_DEBUG("RSA_PUBLIC_KEY_E_LEN:\n");
		UBOOT_DUMP((void *)rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN);
		UBOOT_DEBUG("signature:\n");
		UBOOT_DUMP((void *)sign_data, SIGNATURE_LEN);
		UBOOT_DEBUG("image :\n");
		UBOOT_DUMP((void *)auth_data, SIGNATURE_LEN);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}
