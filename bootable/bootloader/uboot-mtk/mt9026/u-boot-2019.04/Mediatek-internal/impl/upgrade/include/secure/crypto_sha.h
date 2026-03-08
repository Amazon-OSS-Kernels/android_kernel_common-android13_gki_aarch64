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
 * Redistribution and use in source and binary forms, with or without
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

#ifndef SHA2_H
#define SHA2_H

#ifdef CRYPTO_RSA_C
#define EXTERN
#else
#define EXTERN extern
#endif

#include <MsTypes.h>
#include <image.h>
#include "u-boot/drv_sha.h"

#define SHA256_BLOCK_SIZE  (512 / 8)
#define SHA512_BLOCK_SIZE  (1024 / 8)
#define SHA384_BLOCK_SIZE  SHA512_BLOCK_SIZE

#if defined(CONFIG_SECURE_AES256_SHA384)
#define SHA_DIGEST_SIZE (384/8)
#else
#define SHA_DIGEST_SIZE (256/8)
#endif

#ifndef SHA2_TYPES
#define SHA2_TYPES
typedef unsigned char uint8;
typedef unsigned int  uint32;
typedef unsigned long long uint64;
#endif

typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA256_BLOCK_SIZE];
    uint32 h[8];
} sha256_ctx;

typedef struct {
    unsigned int tot_len;
    unsigned int len;
    unsigned char block[2 * SHA512_BLOCK_SIZE];
    uint64 h[8];
} sha512_ctx;

typedef sha512_ctx sha384_ctx;

int secure_sha_hw(Drv_SHAMode eMode, unsigned char* in_buf, unsigned char* out_buf, unsigned int len);

void sha256_init_usb_upgrade(sha256_ctx * ctx);
void sha256_update_usb_upgrade(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha256_final_usb_upgrade(sha256_ctx *ctx, unsigned char *digest);
void sha256_usb_upgrade(const unsigned char *message, unsigned int len,
            unsigned char *digest);
void sha256_transf_usb_upgrade(sha256_ctx *ctx, const unsigned char *message,
                   unsigned int block_nb);

void sha384_init(sha384_ctx *ctx);
void sha384_update(sha384_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha384_final(sha384_ctx *ctx, unsigned char *digest);
void sha384(const unsigned char *message, unsigned int len,
            unsigned char *digest);
void sha384_mgf(struct image_region *region, unsigned int region_count, unsigned char *digest);

void sha512_init(sha512_ctx *ctx);
void sha512_update(sha512_ctx *ctx, const unsigned char *message,
                   unsigned int len);
void sha512_final(sha512_ctx *ctx, unsigned char *digest);
void sha512(const unsigned char *message, unsigned int len,
            unsigned char *digest);

EXTERN int common_sha(unsigned char* in_buf, unsigned char* out_buf, unsigned int u32len);

#undef EXTERN

#endif
