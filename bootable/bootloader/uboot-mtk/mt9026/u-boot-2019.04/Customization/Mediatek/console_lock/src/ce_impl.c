// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <command.h>
#include <exports.h>
#include <malloc.h>
#include <crypto_rsa.h>
#include <crypto_sha.h>
#include <cli.h>
#include <u-boot/sha256.h>
#include <linux/libfdt_env.h>
#include <errno.h>
#include "ce_impl.h"
#include "ce_ops_impl.h"
#include "console_impl.h"
#include <linux/bitops.h>
#include <secure_common.h>
#include <secure_boot.h>
#include <secure/crypto_auth.h>
#include <secure/secure_common.h>
#include "utility.h"
#include "mini-gmp.h"
#include <upgrade_utility.h>
#include <system_impl.h>

#define SHA256_HASH_SIZE	SHA256_SUM_LEN //32
#define RESPONSE_SIZE		256
#define BASE64_ENCODE_SIZE	200
#define BASE64_DECODE_SIZE	400
#define RETRY_CNT		    5
#define NONCE_SIZE          16
#define MAP_SIZE            256
#define ROW_SIZE            16

#define SHA256(_a, _b, _c)  sha256_usb_upgrade((_a), (_b), (_c))

#ifdef CONFIG_CONSOLE_UNLOCK_USB
#define MAX_DEVICE                               3
#define MAX_PARTITION                            5
#define specific_char                            ";"
#define specific_char_num                        2
#define CONSOLE_LOCK_LOAD_RESPONSE_BUFFER_ADDR   (CONFIG_SYS_MIU0_CACHE|0x200000)
#define CHALLENGE_PREFIX_LEN                     0x50

static uint8_t prefix[CHALLENGE_PREFIX_LEN] = {0};
static uint8_t *UUID = NULL;
static unsigned int prefix_len = 0;
#else
#define CHALLENGE_PREFIX_LEN                     0x10
#endif

DECLARE_GLOBAL_DATA_PTR;

static uint8_t hash[SHA256_HASH_SIZE] = {0};
static uint8_t challenge[SHA256_HASH_SIZE + CHALLENGE_PREFIX_LEN] = {0};
static uint8_t challenge_base64[BASE64_ENCODE_SIZE] = {0};
static uint8_t response_signed[RESPONSE_SIZE] = {0};
static uint8_t nonce[NONCE_SIZE] = {0};

static const char *codes =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char map[MAP_SIZE] = {
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
	52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
	255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
	7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
	19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
	255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
	37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
	49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255
};

static inline void __maybe_unused __swap(uint8_t *o1, uint8_t *o2, int size)
{
	int k;
	for (k = 0; k < size; k++) {
		uint8_t tmp = o1[k];
		o1[k] = o2[k];
		o2[k] = tmp;
	}
}

static inline void __maybe_unused reverse(void *objects, size_t size, size_t length)
{
	uint8_t *ptr = objects;
	int i, j;
	for (i = 0, j = length - 1; i < j; i++, j--)
		__swap(ptr + size * i, ptr + size * j, size);
}

static inline void _clear_global_data(void)
{
	memset(hash, 0, sizeof(hash));
	memset(challenge, 0, sizeof(challenge));
	memset(challenge_base64, 0, sizeof(challenge_base64));
	memset(nonce, 0, sizeof(nonce));
	memset(response_signed, 0, sizeof(response_signed));
#ifdef CONFIG_CONSOLE_UNLOCK_USB
	memset(prefix, 0, sizeof(prefix));
#endif
}

static int base64_encode(const unsigned char *in,  unsigned long inlen,
		unsigned char *out, unsigned long *outlen)
{
	unsigned long i, len2, leven;
	unsigned char *p;

	len2 = 4 * ((inlen + 2) / 3);
	if (*outlen < len2 + 1) {
		*outlen = len2 + 1;
		return -EINVAL;
	}
	p = out;
	leven = 3 * (inlen / 3);
	for (i = 0; i < leven; i += 3) {
		*p++ = codes[(in[0] >> 2) & 0x3F];
		*p++ = codes[(((in[0] & 3) << 4) + (in[1] >> 4)) & 0x3F];
		*p++ = codes[(((in[1] & 0xf) << 2) + (in[2] >> 6)) & 0x3F];
		*p++ = codes[in[2] & 0x3F];
		in += 3;
	}
	if (i < inlen) {
		unsigned a = in[0];
		unsigned b = (i+1 < inlen) ? in[1] : 0;
		*p++ = codes[(a >> 2) & 0x3F];
		*p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
		*p++ = (i+1 < inlen) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
		*p++ = '=';
	}
	*p = '\0';
	*outlen = p - out;
	return 0;
}

static int base64_decode(const unsigned char *in,  unsigned long inlen,
		unsigned char *out, unsigned long *outlen)
{
	unsigned long t, x, y, z;
	unsigned char c;
	int           g = 3;

	for (x = y = z = t = 0; x < inlen; x++) {
		c = map[in[x] & 0xFF];
		if (c == 255)
			continue;
		if (c == 254) {
			c = 0;
			if (--g < 0)
				return -2;
		} else if (g != 3) {
			return -2;
		}

		t = (t << 6) | c;

		if (++y == 4) {
			if (z + g > *outlen)
				return -1;
			out[z++] = (unsigned char)((t >> 16) & 255);
			if (g > 1)
				out[z++] = (unsigned char)((t >> 8) & 255);
			if (g > 2)
				out[z++] = (unsigned char)(t & 255);
			y = t = 0;
		}
	}
	if (y != 0)
		return -2;

	*outlen = z;
	return 0;
}

uint8_t *get_response_signed(size_t *len)
{
	if (!len)
		return NULL;
	*len = sizeof(response_signed);

	return response_signed;
}

static int _sha256_crypto(uint8_t *in, size_t in_len,
		uint8_t *out, size_t out_len)
{
#ifdef CONFIG_SHA256
	sha256_context ctx;
#endif

	if (!in || !out) {
		UBOOT_ERROR("invaild argument\n");
		return -EINVAL;
	}

	if (out_len < SHA256_SUM_LEN) {
		UBOOT_ERROR("invaild output buffer\n");
		return -EINVAL;
	}

#ifdef CONFIG_SHA256
	sha256_starts(&ctx);
	sha256_update(&ctx, (void *)in, in_len);
	sha256_finish(&ctx, out);
#else
	SHA256((void *)in, in_len, out);
#endif

	return 0;
}

static inline void console_unlock(struct ce_ops *ops)
{
	printf("\n[CE] unlock console.\n");
	ce_ops_cu_write(ops, true);
}

static int get_public_key_n(EN_SECURE_KEY_TYPE en_key,
		unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = { };

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_N_LEN) {
		UBOOT_ERROR("The len is smaller than the"
			"lenght of public-N (%d bytes).\n", RSA_PUBLIC_KEY_N_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

static int get_public_key_e(EN_SECURE_KEY_TYPE en_key,
			unsigned char *buffer, unsigned int len)
{
	int ret;
	st_secure_key secure_key = { };

	if (!buffer) {
		UBOOT_ERROR("The input parameter 'buffer' is a null pointer\n");
		return -EINVAL;
	}

	if (len < RSA_PUBLIC_KEY_E_LEN) {
		UBOOT_ERROR("The len is smaller than the lenght"
			"of public-E(%d bytes).\n", RSA_PUBLIC_KEY_E_LEN);
		return -EINVAL;
	}

	ret = secure_get_key(en_key, &secure_key);
	if (ret == -1) {
		UBOOT_ERROR("Wrong EN_SECURE_KEY_TYPE Index !!\n");
		return ret;
	}
	memcpy(buffer, secure_key.data + RSA_PUBLIC_KEY_N_LEN, len);
	flush_cache((unsigned long)buffer, len);

	return 0;
}

static int _rsa_decrypt(uint8_t *in, size_t in_len,
			uint8_t **output, size_t *out_len)
{
	int ret = 0;
	uint8_t *rsa_public_key_N = NULL;
	uint8_t *rsa_public_key_E = NULL;
	mpz_t res, base, exp, mod;

	mpz_init(res);
	mpz_init(base);
	mpz_init(exp);
	mpz_init(mod);

	rsa_public_key_N = malloc(RSA_PUBLIC_KEY_N_LEN);
	if (!rsa_public_key_N) {
		UBOOT_ERROR("malloc for rsa_public_key_N %d bytes failed!\n",
				RSA_PUBLIC_KEY_N_LEN);
		ret = -ENOMEM;
		goto out;
	}

	rsa_public_key_E = malloc(RSA_PUBLIC_KEY_E_LEN);
	if (!rsa_public_key_E) {
		UBOOT_ERROR("malloc for rsa_public_key_E %d bytes failed!\n",
				RSA_PUBLIC_KEY_E_LEN);
		ret = -ENOMEM;
		goto out;
	}
	memset(rsa_public_key_N, 0, RSA_PUBLIC_KEY_N_LEN);
	memset(rsa_public_key_E, 0, RSA_PUBLIC_KEY_E_LEN);

	ret = get_public_key_n(KEY_TYPE_CKB_A_CE_KEY, rsa_public_key_N,
				RSA_PUBLIC_KEY_N_LEN);
	if (ret)
		goto out;

	trace_dump_hex(rsa_public_key_N, RSA_PUBLIC_KEY_N_LEN, "dump public_key N:");

	ret = get_public_key_e(KEY_TYPE_CKB_A_CE_KEY, rsa_public_key_E,
				RSA_PUBLIC_KEY_E_LEN);
	if (ret)
		goto out;

	trace_dump_hex(rsa_public_key_E, RSA_PUBLIC_KEY_E_LEN, "dump public_Key E:");

	// prepare base, mod, exp
	mpz_import(base, in_len, 1, sizeof(uint8_t), 0, 0, in);
	mpz_import(mod, RSA_PUBLIC_KEY_N_LEN, 1, sizeof(uint8_t),
					0, 0, rsa_public_key_N);
	mpz_import(exp, RSA_PUBLIC_KEY_E_LEN, 1, sizeof(uint8_t),
					0, 0, rsa_public_key_E);
	// decrypt
	mpz_powm(res, base, exp, mod);

	*output = mpz_export(NULL, out_len, 1, sizeof(uint8_t), 0, 0, res);
	trace_dump_hex(*output, *out_len, "dump decrypt:");
out:
	// clean up
	// for safty, clear data first
	if (rsa_public_key_N) {
		memset(rsa_public_key_N, 0, RSA_PUBLIC_KEY_N_LEN);
		free(rsa_public_key_N);
	}

	if (rsa_public_key_E) {
		memset(rsa_public_key_E, 0, RSA_PUBLIC_KEY_E_LEN);
		free(rsa_public_key_E);
	}

	mpz_clear(res);
	mpz_clear(base);
	mpz_clear(exp);
	mpz_clear(mod);

	return ret;
}

#ifdef CONFIG_CONSOLE_UNLOCK_USB
static int get_UUID(uint8_t *in, size_t in_len)
{
	unsigned int len = in_len - 1;
	unsigned int i = 0, m = 0, n = 0;
	unsigned int UUID_len = 0;
	char c = specific_char[0];

	// find specific_char
	while((*(in + len) != c) && (len > 0)) {
		len--;
	}

	if(len <= 0) {
		UBOOT_ERROR("get UUID specific_char '%s' failed!\n", specific_char);
		return -1;
	}
	m = len;

	UUID_len = in_len - m;
	UBOOT_DEBUG("UUID_len=%d\n", UUID_len);
	UUID = malloc(UUID_len);
	if(!UUID) {
		UBOOT_ERROR("alloc UUID buffer failed\n");
		return -1;
	}
	memset(UUID, 0, UUID_len);

	// get "UUID"
	n = m + 1;
	while(n < in_len) {
		*(UUID+i) = *(in+n);
		i++;
		n++;
	}
	return 0;
}
#endif

bool ce_verify(uint8_t *in, size_t in_len, uint8_t *ans, size_t ans_len)
{
	int ret;
	uint8_t *response = NULL;
	size_t decrypt_sz = 0;
	bool result = false;

	if (!in || !in_len || !ans || !ans_len) {
		UBOOT_ERROR("Invalid argumemnt\n");
		return false;
	}

	ret = _rsa_decrypt(in, in_len, &response, &decrypt_sz);
	if (ret)
		goto out;

#ifdef CONFIG_CONSOLE_UNLOCK_USB
	// get UUID
	if(get_UUID(response, decrypt_sz) != 0) {
		UBOOT_ERROR("get UUID failed\n");
		goto out;
	} else {
		UBOOT_DEBUG("UUID=%s\n", UUID);
		decrypt_sz = decrypt_sz - strlen(UUID) - 1;
	}
	trace_dump_hex(response, decrypt_sz, "dump decrypt:");
#endif

	trace_dump_hex(ans, ans_len, "dump anwser:");
	if (decrypt_sz != ans_len) {
		UBOOT_ERROR("Invalid raw data size after decrypt,"
			" decrypt_sz(%ld) != ans_len(%ld)\n", decrypt_sz, ans_len);
		result = false;
		goto out;
	}

	// verify
	if (!memcmp(response, ans, decrypt_sz))
		result = true;

out:
	if (response) {
		memset(response, 0, decrypt_sz);
		free(response);
	}

	return result;
}

static bool verify_response(uint8_t *in, size_t len)
{
	bool unlock = false;
	unsigned long response_signed_size = RESPONSE_SIZE;

	memset(response_signed, 0, sizeof(response_signed));

	trace_dump_hex(in, len, "dump input response");

	if (base64_decode(in, len, response_signed,
				&response_signed_size)) {
		UBOOT_ERROR("base64 decode failed, format error!\n");
		print_hex_dump("", DUMP_PREFIX_OFFSET, ROW_SIZE, 1, in, len, 1);
		goto out;
	}

	if (response_signed_size != RESPONSE_SIZE) {
		UBOOT_ERROR("base64 decode got invaild response_signed_size %ld\n",
				response_signed_size);
		goto out;
	}

	trace_dump_hex(response_signed, response_signed_size,
			"dump signed response:");

	unlock = ce_verify(response_signed, response_signed_size,
			hash, SHA256_HASH_SIZE);

out:
	return unlock;
}

static void *ce_minigmp_alloc(size_t sz)
{
	void *p;

	if (sz <= 0)
		panic("%s: alloc %ld byte is invaild\n",
			__func__, sz);
	p = malloc(sz);
	if (!p)
		panic("%s: alloc %ld byte failed!\n",
			__func__, sz);
	return p;
}

static void ce_minigmp_free(void *p, size_t sz)
{
	free(p);
}

static void *ce_minigmp_realloc(void *old, size_t old_sz, size_t new_sz)
{
	void *p;

	p = realloc(old, new_sz);
	if (!p)
		panic("%s: realloc %ld byte failed!\n",
			__func__, new_sz);
	return p;
}

#ifdef CONFIG_CONSOLE_UNLOCK_USB
static int prepare_sha_out(void)
{
	trace_dump_hex(prefix, strlen(prefix), "prepare_sha_out input dump:");
	return sha256_crypto_prefix(prefix, strlen(prefix), hash, sizeof(hash));
}

int sha256_crypto_prefix(uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
	if (!in || !out)
		return -EINVAL;

	if (out_len < SHA256_HASH_SIZE) {
		UBOOT_ERROR("The sha256 output buffer size should be %d\n", SHA256_HASH_SIZE);
		return -EINVAL;
	}
	memset(out, 0, SHA256_HASH_SIZE);

	return _sha256_crypto(in, in_len, out, SHA256_HASH_SIZE);
}

uint8_t *get_preset_prefix(size_t *len)
{
	if (!len)
		return NULL;
	*len = prefix_len;

	return prefix;
}

const char *get_device_ID(void)
{
	return env_get("serialno");
}

const char *get_FW_version(void)
{
	static const char *fw_version = "1.540.11.1_mtk";
	return fw_version;
}

const char *get_serial_number(void)
{
	static const char *serial_number = "030210000347";
	return serial_number;
}

const char *get_response_filename(void)
{
	static const char *filename = "response";
	return filename;
}

static int check_load_usb_file(const char *interface, const char *response_file)
{
	UBOOT_TRACE("IN\n");
	char buffer[CMD_BUF] = "\0";
	int snprintf_len = 0;
	int g_device_e = 0, g_partition_e = 0;

	for(g_device_e = 0 ; g_device_e < MAX_DEVICE ;g_device_e++)
	{
		for(g_partition_e = 0 ; g_partition_e < MAX_PARTITION ; g_partition_e++)
		{
			snprintf_len = snprintf(buffer, CMD_BUF, "fatload %s %d:%d 0x%x %s 0x%x", interface, g_device_e, g_partition_e, CONSOLE_LOCK_LOAD_RESPONSE_BUFFER_ADDR, response_file, BASE64_DECODE_SIZE);
			if (snprintf_len >= CMD_BUF) {
				UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
				return -1;
			}
			UBOOT_DEBUG("cmd: %s \n",buffer);
			if (run_command(buffer, 0) == 0) {
				UBOOT_DEBUG("Find '%s' on %s device %d partition %d\n", response_file, interface, g_device_e, g_partition_e);
				UBOOT_TRACE("OK\n");
				return 0;
			}
		}
	}
	UBOOT_TRACE("OK\n");
	return -1;
}

static int load_usb_response(const char *interface, const char *response_file, char *in)
{
	UBOOT_TRACE("IN\n");
	unsigned char *p_size = NULL;
	unsigned long in_size = 0;

	// check and load file on usb
	if (check_load_usb_file(interface, response_file) != 0) {
		UBOOT_ERROR("fail : check and load response file fail !\n");
		return -1;
	}
	UBOOT_DEBUG("Find '%s' on %s filesize\n", response_file, interface);

	// get size of response file from uboot's env variable
	p_size = env_get("filesize");
	if (p_size != NULL) {
		in_size = (int)simple_strtol(p_size, NULL, 16);
		UBOOT_DEBUG("response file size=%ld\n", in_size);
	} else {
		UBOOT_ERROR("env_get filesize failed\n");
	}
	// copy from BUFFER_ADDR to in buffer
	if (in_size > BASE64_DECODE_SIZE) {
		UBOOT_ERROR("input length (%ld) is invalid\n", in_size);
		return -1;
	}
	memcpy(in, (void*)CONSOLE_LOCK_LOAD_RESPONSE_BUFFER_ADDR, in_size);
	memset((void*)CONSOLE_LOCK_LOAD_RESPONSE_BUFFER_ADDR, 0, in_size);
	trace_dump_hex(in, in_size, "Response in USB dump:");
	UBOOT_TRACE("OK\n");
	return 0;
}

int do_ce_usb(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	struct ce_ops ops = { };
	const char *device_ID = NULL, *FW_version = NULL, *serial_number = NULL;
	const char *response_filename = NULL;
	char *in = NULL;
	unsigned long base64_encode_size = BASE64_ENCODE_SIZE;

	// prefix parameters
	device_ID = get_device_ID();
	FW_version = get_FW_version();
	serial_number = get_serial_number();

	if(!device_ID) {
		UBOOT_ERROR("get_device_ID failed\n");
		return CMD_RET_FAILURE;
	}
	if(!FW_version) {
		UBOOT_ERROR("get_FW_version failed\n");
		return CMD_RET_FAILURE;
	}
	if(!serial_number) {
		UBOOT_ERROR("get_serial_number failed\n");
		return CMD_RET_FAILURE;
	}

	trace_dump_hex(device_ID, strlen(device_ID), "device_ID dump:");
	trace_dump_hex(FW_version, strlen(FW_version), "FW_version dump:");
	trace_dump_hex(serial_number, strlen(serial_number), "serial_number dump:");

	if (ce_ops_init(&ops))
		return CMD_RET_FAILURE;

	prefix_len = (strlen(device_ID) + strlen(FW_version) + strlen(serial_number) + specific_char_num);
	strncat(prefix, device_ID, strlen(device_ID));
	strncat(prefix, specific_char, strlen(specific_char));
	strncat(prefix, FW_version, strlen(FW_version));
	strncat(prefix, specific_char, strlen(specific_char));
	strncat(prefix, serial_number, strlen(serial_number));
	trace_dump_hex(prefix, prefix_len, "prefix dump:");

	if (prepare_sha_out()) {
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	// setup mini-gmp alloc/realloc/free callback
	mp_set_memory_functions(ce_minigmp_alloc, ce_minigmp_realloc, ce_minigmp_free);

	memcpy(challenge, prefix, prefix_len);
	memcpy(challenge + prefix_len, hash, SHA256_HASH_SIZE);
	trace_dump_hex(challenge, prefix_len + SHA256_HASH_SIZE, "challenge before base64 encode dump:");

	base64_encode(challenge, prefix_len + SHA256_HASH_SIZE,
		challenge_base64, &base64_encode_size);

	UBOOT_DEBUG("\nchallenge: %s\n\n", challenge_base64);

	ret = run_command("usb start", 0);
	if (ret != 0) {
		UBOOT_ERROR("usb start failed\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	in = malloc(CONFIG_SYS_CBSIZE);
	if (!in) {
		UBOOT_ERROR("[CE] alloc input buffer failed!\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}
	memset(in, 0, CONFIG_SYS_CBSIZE);

	response_filename = get_response_filename();
	UBOOT_DEBUG("response filename = %s\n", response_filename);
	if(!response_filename) {
		UBOOT_ERROR("get_response_filename failed\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	// load response file in in_buffer
	if (load_usb_response("usb", response_filename, in) != 0) {
		UBOOT_ERROR("load_usb_response failed\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	//verify response
	if (verify_response((uint8_t *)in, strlen(in))) {
		console_unlock(&ops);
		ret = CMD_RET_SUCCESS;
	} else {
		UBOOT_ERROR("[CE] console unlock failed!\n");
		ret = CMD_RET_FAILURE;
	}
	printf("UUID=%s\n\n", UUID);

exit:
	_clear_global_data();
	if(in) {
		memset(in, 0, CONFIG_SYS_CBSIZE);
		free(in);
	}
	if(UUID) {
		memset(UUID, 0, strlen(UUID));
		free(UUID);
	}
	return ret;
}

int do_ce(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0, cnt = 0;
	struct ce_ops ops = { };
	const char *device_ID = NULL, *FW_version = NULL, *serial_number = NULL;
	char *in = NULL;
	unsigned long base64_encode_size = BASE64_ENCODE_SIZE;

	// prefix parameters
	device_ID = get_device_ID();
	FW_version = get_FW_version();
	serial_number = get_serial_number();

	if(!device_ID) {
		UBOOT_ERROR("get_device_ID failed\n");
		return CMD_RET_FAILURE;
	}
	if(!FW_version) {
		UBOOT_ERROR("get_FW_version failed\n");
		return CMD_RET_FAILURE;
	}
	if(!serial_number) {
		UBOOT_ERROR("get_serial_number failed\n");
		return CMD_RET_FAILURE;
	}

	trace_dump_hex(device_ID, strlen(device_ID), "device_ID dump:");
	trace_dump_hex(FW_version, strlen(FW_version), "FW_version dump:");
	trace_dump_hex(serial_number, strlen(serial_number), "serial_number dump:");

	if (ce_ops_init(&ops))
		return CMD_RET_FAILURE;

	prefix_len = (strlen(device_ID) + strlen(FW_version) + strlen(serial_number) + specific_char_num);
	strncat(prefix, device_ID, strlen(device_ID));
	strncat(prefix, specific_char, strlen(specific_char));
	strncat(prefix, FW_version, strlen(FW_version));
	strncat(prefix, specific_char, strlen(specific_char));
	strncat(prefix, serial_number, strlen(serial_number));
	trace_dump_hex(prefix, prefix_len, "prefix dump:");

	if (prepare_sha_out()) {
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	// setup mini-gmp alloc/realloc/free callback
	mp_set_memory_functions(ce_minigmp_alloc, ce_minigmp_realloc, ce_minigmp_free);

	memcpy(challenge, prefix, prefix_len);
	memcpy(challenge + prefix_len, hash, SHA256_HASH_SIZE);
	trace_dump_hex(challenge, prefix_len + SHA256_HASH_SIZE, "challenge before base64 encode dump:");

	base64_encode(challenge, prefix_len + SHA256_HASH_SIZE,
		challenge_base64, &base64_encode_size);

	printf("\nchallenge: %s\n\n", challenge_base64);

	in = malloc(CONFIG_SYS_CBSIZE);
	if (!in) {
		UBOOT_ERROR("[CE] alloc input buffer failed!\n");
		ret = CMD_RET_FAILURE;
		goto exit;
	}
	memset(in, 0, CONFIG_SYS_CBSIZE);

	/* unlock retry */
	while (cnt++ < RETRY_CNT) {
		cli_readline_into_buffer("input: ", in, 0);
		if (!strncmp(in, "quit", strlen("quit")) ||
			!strncmp(in, "exit", strlen("exit"))) {
			printf("terminate...\n");
			break;
		}

		// verify response
		if (strlen(in) > BASE64_DECODE_SIZE)
		{
			printf("input length (%ld) is invalid\n", strlen(in));
		}
		else if (verify_response((uint8_t *)in, strlen(in)))
		{
			console_unlock(&ops);
			ret = CMD_RET_SUCCESS;
			break;
		}
		printf("Digest is not the same. %d retry times remained.\n",
			RETRY_CNT - cnt);
		memset(in, 0, CONFIG_SYS_CBSIZE);
	}
	printf("UUID=%s\n\n", UUID);

exit:
	_clear_global_data();
	if(in) {
		memset(in, 0, CONFIG_SYS_CBSIZE);
		free(in);
	}
	if(UUID) {
		memset(UUID, 0, strlen(UUID));
		free(UUID);
	}
	return ret;
}

#else
static int prepare_magic_serial(void)
{
	int ret;
	uint8_t seed[NONCE_SIZE] = { };
	struct ce_ops ops = { };

	ret = ce_ops_init(&ops);
	if (ret)
		return ret;

	ret = ce_ops_get_nonce(&ops, &seed, sizeof(seed));
	if (ret) {
		UBOOT_ERROR("get nonce failed!\n");
		return ret;
	}

	// update global nonce
	memcpy(nonce, seed, sizeof(nonce));

	return sha256_crypto_magic(seed, sizeof(seed), hash, sizeof(hash));
}

static int _nonce_add_sn(uint8_t *n, size_t len)
{
	int i;
	size_t l;
	char *p;

	if (!n || len == 0)
		return -EINVAL;

	// get sn, this sn is generated by unique device id
	p = env_get("serialno");
	if (!p) {
		UBOOT_ERROR("loss serial number!\n");
		return 0;
	}

	l = min(len, strlen(p));

	// add directly
	for (i = 0; i < l; ++i)
		n[i] |= p[i];

	return 0;
}

int sha256_crypto_magic(uint8_t *in, size_t in_len, uint8_t *out, size_t out_len)
{
	int ret;

	if (!in || !out)
		return -EINVAL;

	if (out_len < SHA256_HASH_SIZE) {
		UBOOT_ERROR("The sha256 output buffer size should be %d\n",
			SHA256_HASH_SIZE);
		return -EINVAL;
	}

	trace_dump_hex(in, in_len, "nonce before add SN");
	ret = _nonce_add_sn(in, in_len);
	if (ret)
		return ret;
	trace_dump_hex(in, in_len, "nonce after add SN");

	memset(out, 0, SHA256_HASH_SIZE);

	return _sha256_crypto(in, in_len, out, SHA256_HASH_SIZE);
}

uint8_t *get_preset_nonce(size_t *len)
{
	if (!len)
		return NULL;
	*len = sizeof(nonce);

	return nonce;
}

const char *get_build_type(void)
{
#ifdef CONSOLE_LOCK_ENABLE
	return "REL";
#else
	return "DEV";
#endif
}

const char *get_build_project(void)
{
#define __STR(x) #x
#define STR(x) __STR(x)
#define BUILD_PROJ_IS(y)	!strncmp(STR(AN_BUILD_PROJ), y, strlen(STR(AN_BUILD_PROJ)))
	if (BUILD_PROJ_IS("Trinity"))
		return "TRI";
	else if (BUILD_PROJ_IS("Uroboros"))
		return "URO";
	else if (BUILD_PROJ_IS("Valhalla"))
		return "VAL";
	else if (BUILD_PROJ_IS("Wyvern"))
		return "WYV";
	else
		return "VAL";
}

int do_ce(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int cnt = 0;
	char *in;
	size_t len;
	struct ce_ops ops = { };
	unsigned long base64_encode_size = BASE64_ENCODE_SIZE;
	const char *project = get_build_project();
	const char *type = get_build_type();

	if (ce_ops_init(&ops))
		return CMD_RET_FAILURE;

    in = malloc(CONFIG_SYS_CBSIZE);
	if (!in) {
		UBOOT_ERROR("[CE] alloc input buffer failed!\n");
		return CMD_RET_FAILURE;
	}
    memset(in, 0, CONFIG_SYS_CBSIZE);

	if (prepare_magic_serial()) {
		free(in);
		_clear_global_data();
		return CMD_RET_FAILURE;
	}

	// setup mini-gmp alloc/realloc/free callback
	mp_set_memory_functions(ce_minigmp_alloc,
		ce_minigmp_realloc, ce_minigmp_free);

	len = (strlen(project) > CHALLENGE_PREFIX_LEN / 2 ? CHALLENGE_PREFIX_LEN / 2 : strlen(project));
	memcpy(challenge, project, len);
	len = (strlen(type) > CHALLENGE_PREFIX_LEN / 2 ? CHALLENGE_PREFIX_LEN / 2 : strlen(type));
	memcpy(challenge + strlen(project), type, len);
	memcpy(challenge + strlen(project) + strlen(type), hash, SHA256_HASH_SIZE);
	trace_dump_hex(challenge, sizeof(challenge), "challenge dump:");

	base64_encode(challenge,
		SHA256_HASH_SIZE + strlen(project) + strlen(type),
		challenge_base64, &base64_encode_size);

	printf("\nchallenge: %s\n\n", challenge_base64);

	/* unlock retry */
	while (cnt++ < RETRY_CNT) {
		cli_readline_into_buffer("input: ", in, 0);
		if (!strncmp(in, "quit", strlen("quit")) ||
			!strncmp(in, "exit", strlen("exit"))) {
			printf("terminate...\n");
			break;
		}

        if (strlen(in) > BASE64_DECODE_SIZE)
        {
            printf("input length (%ld) is invalid\n", strlen(in));
        }
        else if (verify_response((uint8_t *)in, strlen(in)))
        {
            console_unlock(&ops);
        break;
        }
		printf("Digest is not the same. %d retry times remained.\n",
			RETRY_CNT - cnt);
        memset(in, 0, CONFIG_SYS_CBSIZE);
	}
	_clear_global_data();
    memset(in, 0, CONFIG_SYS_CBSIZE);
	free(in);

	return CMD_RET_SUCCESS;
}
#endif

int do_check_ce(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
#ifdef CONSOLE_LOCK_ENABLE
    if(is_rel_build()){
        int ret;
        struct ce_ops ops = { };

        ret = ce_ops_init(&ops);
        if (ret)
        {
            return CMD_RET_FAILURE;
        }

        if (ce_ops_le_read(&ops) == 0)
        {
            printf("[CE] log enable bit is clear, close the console\n");
            gd->flags |= GD_FLG_SILENT;
            // module parameter when UART driver is build-in
            ret = add_bootargs("log_disable", "log_disable=1", 0);
            if (ret)
            {
                return CMD_RET_FAILURE;
            }
            // module parameter when UART driver is kernel module
            ret = add_bootargs("8250_mtk_tv.log_disable", "8250_mtk_tv.log_disable=1", 0);
            if (ret)
            {
                return CMD_RET_FAILURE;
            }
        }
        else
        {
            printf("[CE] log enable bit is set\n");
            gd->flags &= ~GD_FLG_SILENT;
            // module parameter when UART driver is kernel module
            ret = del_bootargs("8250_mtk_tv.log_disable", 1);
            if (ret)
            {
                UBOOT_DEBUG("8250_mtk_tv.log_disable has been deleted\n");
            }
            // module parameter when UART driver is build-in
            ret = del_bootargs("log_disable", 1);
            if (ret)
            {
                UBOOT_DEBUG("log_disable has been deleted\n");
            }
        }
    }
    else
    {
        return CMD_RET_SUCCESS;
    }
#endif
    return CMD_RET_SUCCESS;
}

