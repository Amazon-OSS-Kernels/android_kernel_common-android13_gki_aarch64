// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#define CRYPTO_AUTH_C

#include <common.h>
#include <linux/types.h>
#include <system_impl.h>
#include <MsTypes.h>
#include <image.h>
#include <debug_impl.h>
#include <secure/crypto_auth.h>
#include <secure/crypto_rsa.h>
#include <secure/crypto_sha.h>
#include <secure/secure_upgrade.h>

#if defined(CONFIG_SECURE_AES256_SHA384)
static const unsigned char rsa_padding[] __attribute__((unused)) = {
        0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x30, 0x41, 0x30,
        0x0D, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05, 0x00, 0x04, 0x30,
    };
#else
static const unsigned char rsa_padding[] __attribute__((unused)) = {
        0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x30, 0x31, 0x30,
        0x0D, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20,
    };
#endif
#define SAFE_FREE(x)   \
    if(x != NULL) \
    {   \
        free(x); \
    }


typedef void (*checksum_cb)(struct image_region*, unsigned int , unsigned char *);

static void u32_i2osp(uint32_t val, uint8_t *buf)
{
    buf[0] = (uint8_t)((val >> 24) & 0xff);
    buf[1] = (uint8_t)((val >> 16) & 0xff);
    buf[2] = (uint8_t)((val >>  8) & 0xff);
    buf[3] = (uint8_t)((val >>  0) & 0xff);
}

/**
 * mask_generation_function1() - generate an octet string
 *
 * Generate an octet string used to check rsa signature.
 * It use an input octet string and a hash function.
 *
 * @checksum:   A Hash function
 * @seed:   Specifies an input variable octet string
 * @seed_len:   Size of the input octet string
 * @output: Specifies the output octet string
 * @output_len: Size of the output octet string
 * @return 0 if the octet string was correctly generated, others on error
 */
#define MGF_REGION_COUNT 2
#define BUF_I2OSP 4
static int mask_generation_function1(checksum_cb checksum,
                     uint8_t *seed, int seed_len,
                     uint8_t *output, int output_len)
{
    struct image_region region[MGF_REGION_COUNT];

    int i, i_output = 0, region_count = MGF_REGION_COUNT;
    uint32_t counter = 0;
    uint8_t buf_counter[BUF_I2OSP], *tmp;
    int hash_len = SHA_DIGEST_SIZE;

    memset(output, 0, output_len);

    region[0].data = seed;
    region[0].size = seed_len;
    region[1].data = &buf_counter[0];
    region[1].size = BUF_I2OSP;

    tmp = malloc(hash_len);
    if (!tmp) {
        printf("%s: can't allocate array tmp\n", __func__);
        return -1;
    }

    while (i_output < output_len) {
        u32_i2osp(counter, &buf_counter[0]);

        checksum(region, region_count ,tmp);

        i = 0;
        while ((i_output < output_len) && (i < hash_len)) {
            output[i_output] = tmp[i];
            i_output++;
            i++;
        }

        counter++;
    }

    SAFE_FREE(tmp);
    return 0;
}

#define HASH_PRIME_REGION_COUNT 3
static int compute_hash_prime(checksum_cb checksum,
                  uint8_t *pad, int pad_len,
                  uint8_t *hash, int hash_len,
                  uint8_t *salt, int salt_len,
                  uint8_t *hprime)
{
    struct image_region region[HASH_PRIME_REGION_COUNT];
    int region_count = HASH_PRIME_REGION_COUNT;

    region[0].data = pad;
    region[0].size = pad_len;
    region[1].data = hash;
    region[1].size = hash_len;
    region[2].data = salt;
    region[2].size = salt_len;

    checksum(region, region_count , hprime);

    return 0;
}

#define PAD_ZERO_LENGTH 8
#define PKCS1_PSS_MAGIC_ID 0xbc
#define PKCS1_PSS_MAGIC_ID_LEN 1
#define PKCS1_PSS_CONSISTENT_ID 0x01
#define LEFTMOST_BIT 1
#define OCTET_LEN 8

static int padding_pss_verify(checksum_cb checksum,
               uint8_t *msg, unsigned int msg_len,
               const uint8_t *hash, unsigned int hash_len)
{
    uint8_t *masked_db = NULL;
    int masked_db_len = 0;
    uint8_t *h = NULL, *hprime = NULL;
    int h_len = 0;
    uint8_t *db_mask = NULL;
    int db_mask_len = 0;
    uint8_t *db = NULL, *salt = NULL;
    int db_len = 0, salt_len = 0;
    uint8_t pad_zero[PAD_ZERO_LENGTH] = { 0 };
    int ret, i;
    uint8_t leftmost_mask;

    /* input NULL check*/
    if(msg==NULL || hash==NULL)
    {
        printf("%s: invalid pss padding (0xbc is missing)\n", __func__);
        return -1;
    }

    /* overflow check*/
    if((msg_len - hash_len - PKCS1_PSS_MAGIC_ID_LEN) > msg_len)
    {
        printf("%s: integer underflow fail!!\n", __func__);
        return -1;
    }

    /* variable init*/
    masked_db_len = msg_len - hash_len - PKCS1_PSS_MAGIC_ID_LEN;
    db_mask_len = masked_db_len;
    db_len = masked_db_len;
    h_len = hash_len;

    /* step 4: check if the last byte is 0xbc */
    if (msg[msg_len - 1] != PKCS1_PSS_MAGIC_ID) {
        printf("%s: invalid pss padding (0xbc is missing)\n", __func__);
        return -1;
    }

    /* first, allocate everything */
    masked_db = malloc(masked_db_len);
    if (masked_db == NULL) {
        printf("%s: can't allocate masked_db\n", __func__);
        ret = -ENOMEM;
        goto out;
    }
    h = malloc(h_len);
    if (h == NULL) {
        printf("%s: can't allocate h\n", __func__);
        ret = -ENOMEM;
        goto out;
    }
    db_mask = malloc(db_mask_len);
    if (db_mask == NULL) {
        printf("%s: can't allocate db_mask\n", __func__);
        ret = -ENOMEM;
        goto out;
    }
    db = malloc(db_len);
    if (db == NULL) {
        printf("%s: can't allocate db\n", __func__);
        ret = -ENOMEM;
        goto out;
    }
    hprime = malloc(hash_len);
    if (hprime == NULL) {
        printf("%s: can't allocate hprime\n", __func__);
        ret = -ENOMEM;
        goto out;
    }

    /* step 5 */
    memcpy(masked_db, msg, masked_db_len);
    memcpy(h, msg + masked_db_len, h_len);

    /* step 6 */
    leftmost_mask = (0xff >> (OCTET_LEN - LEFTMOST_BIT)) << (OCTET_LEN - LEFTMOST_BIT);
    if (masked_db[0] & leftmost_mask) {
        printf("%s: invalid pss padding ", __func__);
        printf("(leftmost bit of maskedDB not zero)\n");
        ret = -EINVAL;
        goto out;
    }

    /* step 7 */
    if(mask_generation_function1(checksum, h, h_len, db_mask, db_mask_len)!=0)
    {
        printf("mask_generation_function1 Fail!\n ");
        ret = -EINVAL;
        goto out;
    }

    /* step 8 */
    for (i = 0; i < db_len; i++)
        db[i] = masked_db[i] ^ db_mask[i];

    /* step 9 */
    db[0] &= 0xff >> LEFTMOST_BIT;

    /* step 10 */
    for (i = 0; db[i] == 0 && i < (db_len - 1); i++) ;
    if (db[i++] != PKCS1_PSS_CONSISTENT_ID) {
        printf("%s: invalid pss padding ", __func__);
        printf("(leftmost byte of db isn't 0x01)\n");
        ret = EINVAL;
        goto out;
    }

    /* step 11 : RSA_PSS_SALTLEN_AUTO */
    salt_len = db_mask_len-i;
    salt = malloc(salt_len);
    if (!salt)
    {
        printf("%s: can't allocate some buffer\n", __func__);
        ret = -ENOMEM;
        goto out;
    }

    memcpy(salt, &db[i], salt_len);

    /* step 12 & 13 */
    compute_hash_prime(checksum, pad_zero, PAD_ZERO_LENGTH,
               (uint8_t *)hash, hash_len,
               salt, salt_len, hprime);

    /* step 14 */
    ret = memcmp(h, hprime, hash_len);
    if(ret !=0)
    {
        printf("Error: Compare Fail between EM and input data\n");
    }
    else
    {
        UBOOT_DEBUG("Verify OK!\n");
    }

out:
    SAFE_FREE(hprime);
    SAFE_FREE(salt);
    SAFE_FREE(db);
    SAFE_FREE(db_mask);
    SAFE_FREE(h);
    SAFE_FREE(masked_db);

    return ret;
}

int secure_do_authentication(unsigned char *u8public_key_n, unsigned char *u8public_key_e, unsigned char *u8signature, unsigned char *u8auth_data, unsigned int u32auth_data_len)
{
    UBOOT_TRACE("IN\n");
    int ret;
    unsigned char digest[SHA_DIGEST_SIZE]= "\0";
    unsigned char encode_message[RSA_KEY_DIGI_LEN*4] = "\0";

    ret = rsa_main(u8signature, u8public_key_n, u8public_key_e, encode_message);
    if(-1 == ret)
    {
        return -1;
    }

    ret = common_sha(u8auth_data, digest, u32auth_data_len);
    if(-1 == ret)
    {
        return -1;
    }

    UBOOT_DUMP(encode_message, RSA_KEY_DIGI_LEN);
    UBOOT_DUMP(digest, SHA_DIGEST_SIZE);

    /* if PKCS1_PSS is disable, try PKCS#1-PSS first, then try PKCS#1-1.5. (Forward Compatibility)
       if PKCS1_PSS is enable, only support PKCS#1-PSS. */
    UBOOT_DEBUG("Verify with PKCS#1-PSS\n");
    ret = padding_pss_verify(sha384_mgf, encode_message, RSA_KEY_DIGI_LEN, digest, SHA_DIGEST_SIZE);
#ifndef CONFIG_RSA_EMSA_PSS
    if(ret !=0)
    {
        //PKCS#1-V1.5
        UBOOT_INFO("PKCS#1-PSS fail. Retry PKCS#1-V1.5");
        if(memcmp(encode_message+(RSA_KEY_DIGI_LEN-SHA_DIGEST_SIZE),digest, sizeof(digest))==0 && \
           memcmp(encode_message,rsa_padding,sizeof(rsa_padding))==0)
        {
            UBOOT_INFO("Retry PKCS#1-V1.5 Success!\n");
            ret = 0;
        }
    }
#endif
    if(ret!=0)
    {
        UBOOT_ERROR("Error: Authentication Fail!!\n");
        UBOOT_DEBUG("\033[0;31m Image address=0x%lx\033[0m\n", (unsigned long)u8auth_data);
        UBOOT_DEBUG("\033[0;31m RSA encryption's result=\033[0m\n");
        UBOOT_DUMP(encode_message, RSA_KEY_DIGI_LEN);
        UBOOT_DEBUG("\033[0;31m Image's SHA result=\033[0m\n");
        UBOOT_DUMP(digest, SHA_DIGEST_SIZE);
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

#undef CRYPTO_AUTH_C
