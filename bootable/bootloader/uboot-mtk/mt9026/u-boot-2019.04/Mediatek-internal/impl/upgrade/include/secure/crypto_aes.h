/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef __CRYPTO_AES_H__
#define __CRYPTO_AES_H__

#include <MsTypes.h>

int secure_aes_cbc_decrypt_sw(unsigned char *input_addr, MS_U32 input_len, MS_U8 *aes_key, unsigned char *aes_iv, MS_U32 aes_key_size);
int secure_aes_cbc_encrypt_hw(unsigned char *input_addr, unsigned int input_len, unsigned char *aes_key, unsigned char *aes_iv);
int secure_aes_cbc_decrypt_hw(unsigned char *input_addr, unsigned int input_len, unsigned char *aes_key, unsigned char *aes_iv);
int aes_main(unsigned char *input_addr, unsigned int input_len, unsigned char *aes_key, unsigned char *aes_iv, unsigned int aes_key_size);
#endif //#ifndef __CRYPTO_AES_H__
