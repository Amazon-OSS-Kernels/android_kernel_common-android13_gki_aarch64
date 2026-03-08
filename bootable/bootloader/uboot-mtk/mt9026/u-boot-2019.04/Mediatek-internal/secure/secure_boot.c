// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <time.h>
#include <MsCommon.h>
#include <debug_impl.h>
#include <utility.h>
#include <secure_boot.h>

//--------------------------------------------------------------------------------------------------
// Local Variables
//--------------------------------------------------------------------------------------------------
#define REE_ARG_DATA_ADDR  (0x20140000)//<--to do

#define CONFIG_CUSTOMER_KEYBANK_LOADADDR (0x20120000)//<<-- TO_DO

#define CUSTOMER_KEYBANK_NUM_OFFSET (0x10)

int secure_is_tee_fail(void)
{
    #define TEE_FAIL_FLAG (0x0000EEFF)
    UBOOT_TRACE("IN \n");
    int ret=0;
    ST_REE_ARG_DATA* ree_arg_data=NULL;
    ree_arg_data =(ST_REE_ARG_DATA *)(REE_ARG_DATA_ADDR);
    UBOOT_DEBUG("reeloader_version = %d\n",ree_arg_data->reeloader_version);
    UBOOT_DEBUG("uboot_version = %d\n",ree_arg_data->uboot_version);
    UBOOT_DEBUG("rollback_enabled = %d\n",ree_arg_data->rollback_enabled);
    UBOOT_DEBUG("teefail_flag = 0x%x\n",ree_arg_data->teefail_flag);
    if(ree_arg_data->teefail_flag==0)
    {
        UBOOT_DEBUG("TEE PASS !!!\n");
        ret =1;
    }
    else if (ree_arg_data->teefail_flag==TEE_FAIL_FLAG)
    {
        UBOOT_ERROR("TEE FAIL !!!\n");
        ret =-1;
    }
    else
    {
        UBOOT_ERROR("TEE UNKNOW !!!\n");
        ret =0;
    }
    UBOOT_TRACE("OK \n");
    return ret;
}
int do_check_tee_success(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int ret=0;
   ret=secure_is_tee_fail();
   if(ret==-1)
   {
        ret=run_command("retrycount",0);
        ret=run_command("reset",0);
   }
   return ret;
}

U_BOOT_CMD(
    checkteesuccess, 1, 0, do_check_tee_success,
    "check tee success\n",
    "is fail will retry count +1 and reset system \n"
    "For example: checkteesuccess \n"
);

int secure_get_key(EN_SECURE_KEY_TYPE secureKeyType,st_secure_key* secure_key)
{

    UBOOT_TRACE("IN \n");
    unsigned int *numberOfKeys=NULL;
    st_key_info* key_info=NULL;

    numberOfKeys = (unsigned int *) CONFIG_CUSTOMER_KEYBANK_LOADADDR;
    if(*numberOfKeys==0)
    {
        UBOOT_ERROR("key not found!!!\n");
        return -1;
    }
    if(secureKeyType>=MAX_KEY_SIZE)
    {
        UBOOT_ERROR("key Inedx Error !!!\n");
        return -1;
    }
    UBOOT_DEBUG("number Of Keys 0x%x \n",(unsigned int)*numberOfKeys);
    key_info=(st_key_info*)(CONFIG_CUSTOMER_KEYBANK_LOADADDR+0x10);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_A_RI_KEY,key_info[KEY_TYPE_CKB_A_RI_KEY].offset,key_info[KEY_TYPE_CKB_A_RI_KEY].size);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_D_RI_KEY,key_info[KEY_TYPE_CKB_D_RI_KEY].offset,key_info[KEY_TYPE_CKB_D_RI_KEY].size);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_A_UPG_KEY,key_info[KEY_TYPE_CKB_A_UPG_KEY].offset,key_info[KEY_TYPE_CKB_A_UPG_KEY].size);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_D_UPG_KEY,key_info[KEY_TYPE_CKB_D_UPG_KEY].offset,key_info[KEY_TYPE_CKB_D_UPG_KEY].size);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_A_CE_KEY,key_info[KEY_TYPE_CKB_A_CE_KEY].offset,key_info[KEY_TYPE_CKB_A_CE_KEY].size);
    UBOOT_DEBUG("key %d offset 0x%x size 0x%x \n",(int)KEY_TYPE_CKB_D_CE_KEY,key_info[KEY_TYPE_CKB_D_CE_KEY].offset,key_info[KEY_TYPE_CKB_D_CE_KEY].size);
    secure_key->len=key_info[secureKeyType].size;
    secure_key->data=(unsigned char*)(unsigned long)(CONFIG_CUSTOMER_KEYBANK_LOADADDR+key_info[secureKeyType].offset);
    UBOOT_DEBUG("secure_key->len 0x%x\n",secure_key->len);
    UBOOT_DEBUG("secure_key->data %p\n",secure_key->data);
    UBOOT_TRACE("OK \n");
    return 1;
}

int secure_clean_key(void)
{
    UBOOT_TRACE("IN \n");
    unsigned long *numberOfKeys=NULL;

    numberOfKeys = (unsigned long *)CONFIG_CUSTOMER_KEYBANK_LOADADDR;
    if (*numberOfKeys == 0 || *numberOfKeys > MAX_KEY_SIZE)
    {
        UBOOT_ERROR("key not found or more than max!!!\n");
        return -1;
    }
    UBOOT_DEBUG("number Of Keys 0x%lx \n",*numberOfKeys);
    memset((void *)numberOfKeys, 0, (CUSTOMER_KEYBANK_NUM_OFFSET + sizeof(st_key_info)*(*numberOfKeys)));
    UBOOT_TRACE("OK \n");
    return 1;
}



