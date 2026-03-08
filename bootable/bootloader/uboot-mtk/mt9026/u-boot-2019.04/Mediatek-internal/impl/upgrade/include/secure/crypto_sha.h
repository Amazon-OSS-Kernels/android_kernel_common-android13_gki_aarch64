/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

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
