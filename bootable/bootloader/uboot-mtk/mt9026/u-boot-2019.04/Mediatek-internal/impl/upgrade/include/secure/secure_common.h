/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in sournd binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTInN) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************s********************************************/

#ifndef _SECURE_COMMON_H_
#define _SECURE_COMMON_H_

#include <config.h>
#include <common.h>

#define SIGNATURE_LEN               256
#define RSA_PUBLIC_KEY_N_LEN 256
#define RSA_PUBLIC_KEY_E_LEN 4
#define RSA_PUBLIC_KEY_LEN          (RSA_PUBLIC_KEY_N_LEN+RSA_PUBLIC_KEY_E_LEN)

#define AES_IV_LEN 16
#if defined(CONFIG_SECURE_AES256_SHA384)
#define AES_KEY_LEN 32
#define AES_KEY_LEN_BIT 256
#else
#define AES_KEY_LEN 16
#define AES_KEY_LEN_BIT 128
#endif
#define HMAC_KEY_LEN 32

#define FRAGMENT_NUM 16

typedef enum
{
    SECURE_ENG_AES_MODE_ECB=0x01,
    SECURE_ENG_AES_MODE_CBC=0x00
    /*0xFF has specail meaning, don't use this value*/
} SECURE_ENG_AES_MOD;



typedef enum{
    E_RSA_BOOT_PUBLIC_KEY=0,
    E_RSA_UPGRADE_PUBLIC_KEY,
    E_RSA_IMAGE_PUBLIC_KEY,
    E_AES_BOOT_KEY,
    E_AES_UPGRADE_KEY,
    E_AES_EFUSE_KEY,
    E_HMAC_KEY
}key_index;


typedef enum{
    E_KEY_NO_SWAP=1,
    E_KEY_SWAP=0,
}key_swap;



typedef enum
{
    EN_SUCCESS      =0,
    EN_ERROR_OF_CMD   ,
    EN_ERROR_OF_CRC
}signature_storage;

typedef enum
{
    E_SECUREMERGE_NONE =0,
    E_SECUREMERGE_CONTINUE,
    E_SECUREMERGE_INTERLEAVE
}secure_merge_type;

typedef enum
{
    E_SECUREINFO =0,
    E_SECUREIMAGE,
    E_NUTTXCONFIG
}secure_merge_data;

typedef struct
{
    unsigned int u32num;
    unsigned int u32size;
}image_info;

typedef struct
{
     unsigned char u8rsa_kernel_public_key[RSA_PUBLIC_KEY_LEN];
     unsigned char u8rsa_rootfs_public_key[RSA_PUBLIC_KEY_LEN];
     unsigned char u8rsa_mslib_public_key[RSA_PUBLIC_KEY_LEN];
     unsigned char u8rsa_config_public_key[RSA_PUBLIC_KEY_LEN];
     unsigned char u8rsa_app_public_key[RSA_PUBLIC_KEY_LEN];
     unsigned char u8aes_kernel_key[AES_KEY_LEN];
}secure_key_set;

typedef struct
{
  unsigned char u8sec_identify[8];
  image_info info;
  unsigned char u8signature[SIGNATURE_LEN];
}_sub_secure_info;


typedef struct
{
  _sub_secure_info s_info;
  _sub_secure_info s_info_interleave;
}sub_secure_info;

typedef struct
{
  _sub_secure_info s_info[FRAGMENT_NUM];
  _sub_secure_info s_info_interleave[FRAGMENT_NUM];
}sub_secure_info_for_partial_auth;


typedef struct
{
    //Dont change this sequence of tee,recovery and Boot,because android have this struct.
    //If this sequence is changed,android struct need change.
    _sub_secure_info reserve1; //Reserve this space is for backward compatible. It's for Key in the beginning
    _sub_secure_info tee;
    _sub_secure_info recovery;
    #if defined (CONFIG_ARMv8_ARM_TRUSTED_FIRMWARE) && defined (CONFIG_SECURITY_BOOT)
    _sub_secure_info arm_fw;
    #endif
    #if (ENABLE_MODULE_ANDROID_BOOT == 1)
    _sub_secure_info boot;
    #else
    _sub_secure_info kernel;
    _sub_secure_info rootfs[FRAGMENT_NUM];
    _sub_secure_info ms_lib[FRAGMENT_NUM];
    _sub_secure_info lib[FRAGMENT_NUM];
    _sub_secure_info mw_lib[FRAGMENT_NUM];
    _sub_secure_info config[FRAGMENT_NUM];
    _sub_secure_info app[FRAGMENT_NUM];
    _sub_secure_info cacert[FRAGMENT_NUM];

    #if (ENABLE_STB_BOOT == 1) && defined (CONFIG_SECURITY_BOOT)
    _sub_secure_info rtpm;
    #endif

    #endif
    secure_key_set key;
    #if defined (CONFIG_SEC_SYSTEM)
    _sub_secure_info sec_kernel;
    #endif
}security_info_data;


typedef struct
{
    unsigned long crc;                  /* CRC32 over data bytes    */
    security_info_data data;
    security_info_data data_interleave;
}security_info;


typedef struct _rawdata_header_ {
    unsigned int offset;
    unsigned int size;
    unsigned int crc;
    char flag;
    char res1;
    char res2;
    char res3;
    char name[112];
}rawdata_header;


#endif


