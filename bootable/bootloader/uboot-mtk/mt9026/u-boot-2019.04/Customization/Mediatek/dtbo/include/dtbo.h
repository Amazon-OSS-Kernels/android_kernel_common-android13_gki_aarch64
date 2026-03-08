/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DTBO_H_
#define _DTBO_H_

#define DEFAULT_DTB_PART	"bootdata"
#ifdef CONFIG_MULTIPLE_DTB_SELECTION
#define KEY_BUFFER_SIZE  32
#define STRING_BUFFER_SIZE  256
#define DIGIT_NUM  10
#define ANDROID_BOOTARGS_DTBO_IDX	"androidboot.dtbo_idx"

#define DTBO_DEF_ENV_NAME	"dtbo_cfg_part"
#define DTBO_DEF_PART	"bootdata"
#define MAPPING_CFG_ENV_NAME	"dtbo_mapping_cfg"
#define MAPPING_CFG_DEF_PATH	"dtbo_mapping.ini"
#define WHITELIST_ENV_NAME	"dtbo_whitelist"
#define WHITELIST_DEF_PATH	"dtb/dtbo_whitelist.ini"
#define CFG_SEL_ENV_NAME	"dtbo_cfg_sel"
#define CFG_SEL_DEF_PATH	"dtb/mt5896_h2v1.ini"

#ifdef CONFIG_DATA_SEPARATION
#define DTB_OVERLAY_SECTION	"Boot"
#define MAPPING_CFG_KEY	"m_pboot_mapping_cfg_File"
#define WHITELIST_KEY	"m_pboot_whitelist_File"
#define CFG_SEL_KEY	"m_pboot_cfg_sel_File"
#endif

enum INI_FILE_TYPE{
    DTB_MAPPING = 0,
    WHITELIST,
    CFG_SELECT,
    INI_TYPE_NOT_SUPPORT
};

int get_dtbo_cfg_selection(char *partition, char *cfg_name, char *type, char *setting);
int set_dtbo_cfg_selection(char *partition, char *cfg_name);
int ls_dtbo_cfg(char *partition);

typedef struct{
	int dtbo_id;
	struct list_head list;
}DTBO_IDX_T;

#else
typedef enum {
	DEVICE_BOARD,
	DEVICE_MAX,
}E_DEVICE_TYPE;

int is_device_id_valid(int device_id);
int get_device_id(int device_type);
#endif
int dtbo_selection_and_overlay(void);
int get_dtbo_img_size(unsigned long dtbo_img_addr, unsigned long *dtbo_img_size);
int verify_dtbo_img(unsigned char *auth_data, unsigned char *sign_data, unsigned long auth_size);

#endif

