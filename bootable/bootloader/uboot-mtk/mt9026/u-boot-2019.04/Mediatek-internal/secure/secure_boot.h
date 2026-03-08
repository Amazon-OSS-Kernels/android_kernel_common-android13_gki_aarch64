/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _INIT_SECURE_BOOT_H_
#define _INIT_SECURE_BOOT_H_
typedef struct{ //ree_arg_data
    unsigned int reeloader_version;
    unsigned int uboot_version;
    unsigned int rollback_enabled;
    unsigned int teefail_flag;
}ST_REE_ARG_DATA; 

typedef enum
{
    KEY_TYPE_CKB_A_RI_KEY = 0,
    KEY_TYPE_CKB_D_RI_KEY,
    KEY_TYPE_CKB_A_UPG_KEY ,
    KEY_TYPE_CKB_D_UPG_KEY ,
    KEY_TYPE_CKB_A_CE_KEY ,
    KEY_TYPE_CKB_D_CE_KEY ,
    KEY_TYPE_NULL = 0xFFFF
}EN_SECURE_KEY_TYPE;

#define MAX_KEY_SIZE 0x6


typedef struct {
    unsigned int size;
    unsigned int offset;
    unsigned int keytype;
    unsigned int reserved;
}st_key_info;

typedef struct
{
    unsigned int len;
    unsigned char* data;
}st_secure_key;

//-------------------------------------------------------------------------------------------------
/// check tee arg data is tee fail
/// @return  int                              \b OUT: 1,  sucessfully , 0 ,   unknow , -1,  fail
//-------------------------------------------------------------------------------------------------
int secure_is_tee_fail(void);
/******************************************************************************
 *
 * secure_get_key:     get secure key.
 * EN_SECURE_KEY_TYPE: secure key type,  ex: KEY_TYPE_CKB_A_RI_KEY,KEY_TYPE_CKB_D_RI_KEY...
 * st_secure_key:      key,
 *
 * return :            1 = Success , -1 = Failure
 *
*****************************************************************************/
int secure_get_key(EN_SECURE_KEY_TYPE secureKeyType,st_secure_key* secure_key);

/******************************************************************************
 *
 * secure_clean_key:     clean secure key.
 *
 * return :            1 = Success , -1 = Failure
 *
*****************************************************************************/
int secure_clean_key(void);
#endif

