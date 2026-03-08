// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <linux/stddef.h>
#include <linux/sizes.h>
#include <memalign.h>
#include <utility.h>
#include <errno.h>
#include <blk.h>
#include <mmc.h>
#include "storage_offset_impl.h"
#include "ce_impl.h"
#include <debug_impl.h>
#include <secure_boot.h>
#include <secure_common.h>
#include <crypto_aes.h>

#define STORAGE_OFFSET_SIZE	        (SZ_512)
#define STORAGE_OFFSET_BASE	        (SZ_512 * 56) // LBA=0x38
#define MASK_0XFF                   (0xff)
#define ANSR_SIZE                   (32)

#define LOG_ENABLE_OFFSET_BASE      (SZ_512 * 57) // LBA=0x39
#define LOG_ENABLE_FLAG_SIZE        (4)
static const unsigned char log_enable_flag_suffix[LOG_ENABLE_FLAG_SIZE] = {0x5A, 0xA5, 0x5A, 0xA5};

// DEV_ID
#define APMCU_RIUBASE               (0x1C000000)
#define EFUSE_DID_BASE              (APMCU_RIUBASE + 0x40600)
#define DEV_ID_LENGTH               (6)
#define DEV_ID_SIZE                 (12)
#define LOG_ENABLE_SIZE             (16)

enum DID_INDEX
{
    DID_INDEX_0, DID_INDEX_1, DID_INDEX_2, DID_INDEX_3, DID_INDEX_4, DID_INDEX_5
};
#define REG_DEVICE_ID_0             (0x0000)    //0x00
#define REG_DEVICE_ID_1             (0x0004)    //0x01
#define REG_DEVICE_ID_2             (0x0008)    //0x02
#define REG_DEVICE_ID_3             (0x0010)    //0x03
#define REG_DEVICE_ID_4             (0x0014)    //0x04
#define REG_DEVICE_ID_5             (0x0018)    //0x05

#define IO_READ16(base, offset)     *((volatile unsigned short*) (base + offset))

static void get_did(unsigned char *did)
{
    unsigned short device_id[DEV_ID_LENGTH] = {0};

    device_id[DID_INDEX_0] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_0);
    device_id[DID_INDEX_1] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_1);
    device_id[DID_INDEX_2] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_2);
    device_id[DID_INDEX_3] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_3);
    device_id[DID_INDEX_4] = IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_4);
    device_id[DID_INDEX_5] = (IO_READ16(EFUSE_DID_BASE, REG_DEVICE_ID_5) & MASK_0XFF);

    memcpy(did, device_id, DEV_ID_LENGTH * sizeof(unsigned short));
}

static int get_aes_key(EN_SECURE_KEY_TYPE en_key, unsigned char *buffer, unsigned int len)
{
    UBOOT_TRACE("IN\n");
    st_secure_key secure_key = {0};

    if (buffer == NULL)
    {
        UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
        return -1;
    }

    if (len != AES_KEY_LEN)
    {
        UBOOT_ERROR("The len is smaller than the lenght of AES Key(%d bytes).\n", AES_KEY_LEN);
        return -1;
    }

    if (secure_get_key(en_key, &secure_key) != -1)
    {
        memcpy(buffer, secure_key.data, len);
    }
    else
    {
        UBOOT_ERROR("Wrong Key Index !!\n");
        return -1;
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

static inline struct mmc *__init_mmc_dev(int dev)
{
	struct mmc *mmc;

	mmc = find_mmc_device(dev);
	if (!mmc) {
		UBOOT_ERROR("no mmc device at slot %x\n", dev);
		return NULL;
	}

	if (mmc_init(mmc))
		return NULL;

	return mmc;
}

static struct mmc *_get_mmc_dev(void)
{
	if (get_mmc_num() <= 0) {
		UBOOT_ERROR("no emmc available\n");
		return NULL;
	}

	return __init_mmc_dev(0);
}

static int storage_read(void *buf, size_t buf_len, uint32_t blk_addr)
{
	unsigned long n;
	lbaint_t blk_cnt, blk_start;
	struct mmc *mmc;
	struct blk_desc *desc;

	mmc = _get_mmc_dev();
	if (!mmc)
		return -ENODEV;

	desc = mmc_get_blk_desc(mmc);
	if (!desc) {
		UBOOT_ERROR("mmc_get_blk_desc failed\n");
		return -ENODEV;
	}

	blk_start = ALIGN(blk_addr, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt = ALIGN(buf_len, mmc->write_bl_len) / mmc->write_bl_len;
	n = blk_dread(desc, blk_start, blk_cnt, buf);
	if (n != blk_cnt) {
		UBOOT_ERROR("blk_dread failed\n");
		return -EINVAL;
	}

	return 0;
}

static int storage_write(void *buf, size_t buf_len, uint32_t blk_addr)
{
	unsigned long n;
	lbaint_t blk_cnt, blk_start;
	struct mmc *mmc;
	struct blk_desc *desc;

	mmc = _get_mmc_dev();
	if (!mmc)
		return -ENODEV;

	desc = mmc_get_blk_desc(mmc);
	if (!desc) {
		UBOOT_ERROR("mmc_get_blk_desc failed\n");
		return -ENODEV;
	}

	blk_start = ALIGN(blk_addr, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt = ALIGN(buf_len, mmc->write_bl_len) / mmc->write_bl_len;
	n = blk_dwrite(desc, blk_start, blk_cnt, buf);
	if (n != blk_cnt) {
		UBOOT_ERROR("blk_dwrite failed\n");
		return -EINVAL;
	}

	return 0;
}

static inline bool __is_buffer_empty(uint8_t *buf, size_t len)
{
	int i;

	for (i = 0; i < len; ++i) {
		if (buf[i])
			return false;
	}

	return true;
}

static inline bool __is_storage_empty(uint8_t *buf, size_t len)
{
	int i;

	for (i = 0; i < len; ++i) {
		if (buf[i] != MASK_0XFF)
			return false;
	}

	return true;
}

#ifdef CMD_CE_DEBUG
static int _verify_emmc_write(uint8_t *ans, size_t len)
{
	int ret;
	uint8_t *buf = malloc_cache_aligned(STORAGE_OFFSET_SIZE);

	if (!buf) {
		UBOOT_ERROR("alloc verify buffer failed!\n");
		return -ENOMEM;
	}

	ret = storage_read(buf, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage_read failed\n");
		free(buf);
		return ret;
	}

	ret = memcmp(buf, ans, len);
	if (ret) {
		UBOOT_ERROR("data compare failed!\n");
		trace_dump_hex(buf, STORAGE_OFFSET_SIZE, "read after write:");
		trace_dump_hex(ans, len, "write data:");
		ret = -EINVAL;
	} else {
		UBOOT_INFO("verify emmc write OK!\n");
		ret = 0;
	}
	free(buf);

	return ret;
}
#endif

static int _clear_response_signed(void)
{
	int ret;
	char *data;

	data = malloc_cache_aligned(STORAGE_OFFSET_SIZE);
	if (!data) {
		UBOOT_ERROR("alloc buffer failed\n");
		return -ENOMEM;
	}

	memset(data, MASK_0XFF, STORAGE_OFFSET_SIZE);
	ret = storage_write(data, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage_write failed!\n");
		goto out;
	}

#ifdef CMD_CE_DEBUG
	// verify again
	ret = _verify_emmc_write(data, STORAGE_OFFSET_SIZE);
	if (ret) {
		UBOOT_ERROR("verify emmc write data failed!\n");
		goto out;
	}
#endif
out:
	free(data);
	return 0;
}

#ifdef CONFIG_CONSOLE_UNLOCK_USB
static int _write_response_signed(void)
{
	int ret = 0;
	uint8_t *response, *prefix_in;
	size_t resp_len;
	size_t prefix_in_len;
	bool buf_empty = false;
	char *data;

	response = get_response_signed(&resp_len);
	prefix_in = get_preset_prefix(&prefix_in_len);
	if (!response || !prefix_in) {
		UBOOT_ERROR("preset buffer is missing\n");
		return -EINVAL;
	}

	// check response
	if (__is_buffer_empty(response, resp_len)) {
		UBOOT_ERROR("response is empty\n");
		buf_empty = true;
	}

	// check prefix_in
	trace_dump_hex(prefix_in, prefix_in_len, "prefix_in");
	if (__is_buffer_empty(prefix_in, prefix_in_len)) {
		UBOOT_ERROR("prefix is empty\n");
		buf_empty = true;
	}

	if (buf_empty) {
#ifdef CMD_CE_DEBUG
		// TODO: to use cetest command,
		// generate new response signed and nonce
		// or the cetest cu_set is useless
		UBOOT_ERROR("please do ce first!\n");
#endif
		return -EINVAL;
	}

	if ((prefix_in_len + resp_len) > STORAGE_OFFSET_SIZE) {
		UBOOT_ERROR("invalid size (nonce: %ld, resp: %ld)\n",
			prefix_in_len, resp_len);
		return -EINVAL;
	}

	data = malloc_cache_aligned(STORAGE_OFFSET_SIZE);
	if (!data) {
		UBOOT_ERROR("alloc buffer failed!\n");
		return -ENOMEM;
	}

	memset(data, 0, STORAGE_OFFSET_SIZE);
	memcpy(data, prefix_in, prefix_in_len);
	memcpy(&data[prefix_in_len], response, resp_len);

	trace_dump_hex(data, resp_len + prefix_in_len, "write to emmc");

	ret = storage_write(data, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage_write failed !\n");
		goto out;
	}
#ifdef CMD_CE_DEBUG
	// verify again
	ret = _verify_emmc_write(data, resp_len + prefix_in_len);
	if (ret) {
		UBOOT_ERROR("verify emmc write data failed!\n");
		ret = -EINVAL;
	}
#endif
out:
	free(data);
	return ret;
}

int ce_offset_dump_console_unlock_bit(void)
{
	int ret;
	uint8_t *response;
	uint8_t *prefix_in;
	uint8_t ansr[ANSR_SIZE] = { };
	bool enabled = false;
	size_t resp_len;
	size_t prefix_in_len;
	char *data;

	get_response_signed(&resp_len);
	if (resp_len == 0) {
		UBOOT_ERROR("invailed response size\n");
		return (int)enabled;
	}

	get_preset_prefix(&prefix_in_len);
	if (prefix_in_len == 0) {
		UBOOT_ERROR("invailed prefix_in_len size\n");
		return (int)enabled;
	}

	data = malloc_cache_aligned(STORAGE_OFFSET_SIZE);
	if (!data) {
		UBOOT_ERROR("alloc failed\n");
		return (int)enabled;
	}
	memset(data, 0, STORAGE_OFFSET_SIZE);

	ret = storage_read(data, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage read failed!\n");
		goto out;
	}

	prefix_in = data;
	response = &data[prefix_in_len];
	// check response
	if (__is_storage_empty(response, resp_len) ||
		__is_buffer_empty(response, resp_len)) {
		UBOOT_DEBUG("response is empty\n");
		goto out;
	}

	// check prefix_in
	if (__is_storage_empty(prefix_in, prefix_in_len) ||
		__is_buffer_empty(prefix_in, prefix_in_len)) {
		UBOOT_DEBUG("prefix_in is empty\n");
		goto out;
	}

	trace_dump_hex(prefix_in, prefix_in_len, "prefix_in from storage");
	trace_dump_hex(response, resp_len, "response from storage");

	ret = sha256_crypto_prefix(prefix_in, prefix_in_len, ansr, sizeof(ansr));
	if (ret) {
		UBOOT_ERROR("sha256_crypto_prefix failed\n");
		goto out;
	}

	enabled = ce_verify(response, resp_len, ansr, sizeof(ansr));
out:
	free(data);

	return (int)enabled;
}

#else
static int _write_response_signed(void)
{
	int ret = 0;
	uint8_t *response, *nonce;
	size_t resp_len, nonce_len;
	bool buf_empty = false;
	char *data;

	response = get_response_signed(&resp_len);
	nonce = get_preset_nonce(&nonce_len);
	if (!response || !nonce) {
		UBOOT_ERROR("preset buffer is missing\n");
		return -EINVAL;
	}

	// check response
	if (__is_buffer_empty(response, resp_len)) {
		UBOOT_ERROR("response is empty\n");
		buf_empty = true;
	}

	// check nonce
	if (__is_buffer_empty(nonce, nonce_len)) {
		UBOOT_ERROR("nonce is empty\n");
		buf_empty = true;
	}

	if (buf_empty) {
#ifdef CMD_CE_DEBUG
		// TODO: to use cetest command,
		// generate new response signed and nonce
		// or the cetest cu_set is useless
		UBOOT_ERROR("please do ce first!");
#endif
		return -EINVAL;
	}

	if ((nonce_len + resp_len) > STORAGE_OFFSET_SIZE) {
		UBOOT_ERROR("invalid size (nonce: %ld, resp: %ld)\n",
			nonce_len, resp_len);
		return -EINVAL;
	}

	data = malloc_cache_aligned(STORAGE_OFFSET_SIZE);
	if (!data) {
		UBOOT_ERROR("alloc buffer failed!\n");
		return -ENOMEM;
	}

	memset(data, 0, STORAGE_OFFSET_SIZE);
	memcpy(data, nonce, nonce_len);
	memcpy(&data[nonce_len], response, resp_len);

	trace_dump_hex(data, resp_len + nonce_len, "write to emmc");

	ret = storage_write(data, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage_write failed !\n");
		goto out;
	}
#ifdef CMD_CE_DEBUG
	// verify again
	ret = _verify_emmc_write(data, resp_len + nonce_len);
	if (ret) {
		UBOOT_ERROR("verify emmc write data failed!\n");
		ret = -EINVAL;
	}
#endif
out:
	free(data);
	return ret;
}

int ce_offset_dump_console_unlock_bit(void)
{
	int ret;
	uint8_t *response;
	uint8_t *nonce;
	uint8_t ansr[ANSR_SIZE] = { };
	bool enabled = false;
	size_t resp_len, nonce_len;
	char *data;

	get_response_signed(&resp_len);
	if (resp_len == 0) {
		UBOOT_ERROR("invailed response size\n");
		return (int)enabled;
	}

	get_preset_nonce(&nonce_len);
	if (nonce_len == 0) {
		UBOOT_ERROR("invailed nonce size\n");
		return (int)enabled;
	}

	data = malloc_cache_aligned(STORAGE_OFFSET_SIZE);
	if (!data) {
		UBOOT_ERROR("alloc failed\n");
		return (int)enabled;
	}
	memset(data, 0, STORAGE_OFFSET_SIZE);

	ret = storage_read(data, STORAGE_OFFSET_SIZE, STORAGE_OFFSET_BASE);
	if (ret) {
		UBOOT_ERROR("storage read failed!\n");
		goto out;
	}

	nonce = data;
	response = &data[nonce_len];
	// check response
	if (__is_storage_empty(response, resp_len) ||
		__is_buffer_empty(response, resp_len)) {
		UBOOT_DEBUG("response is empty\n");
		goto out;
	}

	// check nonce
	if (__is_storage_empty(nonce, nonce_len) ||
		__is_buffer_empty(nonce, nonce_len)) {
		UBOOT_DEBUG("nonce is empty\n");
		goto out;
	}

	trace_dump_hex(nonce, nonce_len, "nonce from storage");
	trace_dump_hex(response, resp_len, "resp from storage");

	ret = sha256_crypto_magic(nonce, nonce_len, ansr, sizeof(ansr));
	if (ret) {
		UBOOT_ERROR("sha256_crypto_magic failed\n");
		goto out;
	}

	enabled = ce_verify(response, resp_len, ansr, sizeof(ansr));
out:
	free(data);
	return (int)enabled;
}
#endif

int ce_offset_set_console_unlock_bit(bool enable)
{
	if (enable) {
		printf("CE offset: Set CONSOLE UNLOCK bit enable\n");
		return _write_response_signed();
	} else {
		printf("CE offset: Set CONSOLE UNLOCK bit disable\n");
		return _clear_response_signed();
	}
}

int ce_offset_dump_log_enable_bit(void)
{
    int ret = 0;
    unsigned char did[DEV_ID_SIZE] = {0};
    unsigned char log_enable_flag[STORAGE_OFFSET_SIZE] = {0};
    unsigned char aes_key[AES_KEY_LEN] = {0};
    unsigned char aes_iv[AES_IV_LEN] = {0};

    ret = storage_read(log_enable_flag, STORAGE_OFFSET_SIZE, LOG_ENABLE_OFFSET_BASE);
    if (ret)
    {
        UBOOT_ERROR("storage_read failed !\n");
        return ret;
    }
    UBOOT_DEBUG("Dump read log_enable_flag:\n");
    UBOOT_DUMP(log_enable_flag, STORAGE_OFFSET_SIZE);

    ret = get_aes_key(KEY_TYPE_CKB_D_RI_KEY, aes_key, AES_KEY_LEN);
    if (ret)
    {
        UBOOT_ERROR("get_aes_key failed !\n");
        return ret;
    }

    ret = secure_aes_cbc_decrypt_hw(log_enable_flag, LOG_ENABLE_SIZE, aes_key, aes_iv);
    if (ret)
    {
        UBOOT_ERROR("secure_aes_cbc_decrypt_sw failed !\n");
        return ret;
    }

    get_did(did);
    if (!memcmp(log_enable_flag, log_enable_flag_suffix, LOG_ENABLE_FLAG_SIZE) && !memcmp(log_enable_flag + LOG_ENABLE_FLAG_SIZE, did, DEV_ID_SIZE))
    {
        ret = 1;
    }

    return ret;
}

int ce_offset_set_log_enable_bit(bool enable)
{
    int ret = 0;
    unsigned char did[DEV_ID_SIZE] = {0};
    unsigned char log_enable_flag[STORAGE_OFFSET_SIZE] = {0};

    ret = storage_read(log_enable_flag, STORAGE_OFFSET_SIZE, LOG_ENABLE_OFFSET_BASE);
    if (ret)
    {
        UBOOT_ERROR("storage_read failed !\n");
        return ret;
    }

    if (enable)
    {
        unsigned char aes_key[AES_KEY_LEN] = {0};
        unsigned char aes_iv[AES_IV_LEN] = {0};

        get_did(did);
        memcpy(log_enable_flag, log_enable_flag_suffix, LOG_ENABLE_FLAG_SIZE);
        memcpy(log_enable_flag + LOG_ENABLE_FLAG_SIZE, did, DEV_ID_SIZE);

        ret = get_aes_key(KEY_TYPE_CKB_D_RI_KEY, aes_key, AES_KEY_LEN);
        if (ret)
        {
            UBOOT_ERROR("get_aes_key failed !\n");
            return ret;
        }

        ret = secure_aes_cbc_encrypt_hw(log_enable_flag, LOG_ENABLE_SIZE, aes_key, aes_iv);
        if (ret)
        {
            UBOOT_ERROR("secure_aes_cbc_encrypt_sw failed !\n");
            return ret;
        }
    }
    else
    {
        memset(log_enable_flag, 0x0, LOG_ENABLE_SIZE);
    }

    ret = storage_write(log_enable_flag, STORAGE_OFFSET_SIZE, LOG_ENABLE_OFFSET_BASE);
    if (ret)
    {
        UBOOT_ERROR("storage_read failed !\n");
        return ret;
    }

    return ret;
}

int ce_offset_dump_all(void)
{
	int res;

	res = ce_offset_dump_console_unlock_bit();
	printf("CE offset: Get CONSOLE UNLOCK bit %s\n",
			res ? "enable" : "disable");

	res = ce_offset_dump_log_enable_bit();
	printf("CE offset: Get LOG ENABLE bit %s\n",
			res ? "enable" : "disable");

	return 0;
}

int ce_offset_get_nonce(void *out, size_t len)
{
	return generate_random_number(out, len);
}

