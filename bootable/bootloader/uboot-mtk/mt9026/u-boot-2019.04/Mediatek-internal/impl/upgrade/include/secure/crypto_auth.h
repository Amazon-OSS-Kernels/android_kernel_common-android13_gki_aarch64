/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _AUTH_H_
#define _AUTH_H_


int secure_do_authentication(unsigned char *u8public_key_n, unsigned char *u8public_key_e, unsigned char *u8signature, unsigned char *u8auth_data, unsigned int u32auth_data_len);


#endif

