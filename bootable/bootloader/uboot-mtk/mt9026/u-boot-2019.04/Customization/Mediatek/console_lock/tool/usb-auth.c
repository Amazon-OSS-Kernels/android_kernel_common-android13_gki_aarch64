#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <ctype.h>

#include "mini-gmp.h"

#define PADDING_NUM_1      1
#define PADDING_NUM_2      2
#define MASK               0xFF
#define PROJ_NUM           3
#define BUILD_TYPE_NUM     3
#define FIRST_NUMS         (PROJ_NUM + BUILD_TYPE_NUM)
#define COLS               16
#define BASE64_BYTE_NUM    3
#define BASE64_CHAR_NUM    4
#define ARGC_NUM           2
#define SHA_LEN            32

// Calculates the length of a decoded string
static size_t calcDecodeLength(const char* b64input)
{
	size_t len = strlen(b64input), padding = 0;

	if (b64input[len-PADDING_NUM_1] == '=' && b64input[len-PADDING_NUM_2] == '=') //last two chars are =
		padding = PADDING_NUM_2;
	else if (b64input[len-PADDING_NUM_1] == '=') //last char is =
		padding = PADDING_NUM_1;

	return (len*BASE64_BYTE_NUM)/BASE64_CHAR_NUM - padding;
}

// Decodes a base64 encoded string
static int Base64Decode(char* b64message, unsigned char **buffer, size_t *length)
{
	BIO *bio, *b64;
	int decodeLen = calcDecodeLength(b64message);

	*buffer = (unsigned char*)malloc(decodeLen + 1);
	assert(*buffer);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
	*length = BIO_read(bio, *buffer, strlen(b64message));
	BIO_free_all(bio);

	return 0; //success
}

static size_t Base64Encode(const unsigned char *buffer, size_t length, char **b64text)
{
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	*b64text = (*bufferPtr).data;

	return (size_t)bufferPtr->length;
}

static void hexdump(void *mem, unsigned int len)
{
	unsigned int i, j;

	for (i = 0;
		i < len + ((len % COLS) ? (COLS - len % COLS) : 0);
		i++)
	{
		/* print offset */
		if (i % COLS == 0)
			printf("0x%08x: ", i);

		/* print hex data */
		if (i < len)
			printf("%02x ", MASK & ((char*)mem)[i]);
		else /* end of block, just aligning for ASCII dump */
			printf("   ");

		/* print ASCII dump */
		if (i % COLS == (COLS - 1)) {
			for (j = i - (COLS - 1); j <= i; j++) {
				if (j >= len) /* end of block, not really printing */
					putchar(' ');
				else if (isprint(((char *)mem)[j])) /* printable char */
					putchar(MASK & ((char *)mem)[j]);
				else /* other char */
					putchar('.');
			}
			putchar('\n');
		}
	}
	printf("\n");
}

static char *substr(const char *src)
{
	int t = 0;
	while(*(src+t) != ';')
	{
		t++;
	}
	char *dest = (char*)malloc(sizeof(char) * (t + 1));
	assert(dest);

	for (int i = 0; i < t; i++) {
		*dest = *(src + i);
		dest++;
	}

	// null-terminate the destination string
	*dest = '\0';
	// return the destination string
	return dest - t;
}

static char *substr2(const char *src, size_t challenge_len)
{
	printf("challenge_len=%ld\n", challenge_len);
	int len = challenge_len - SHA_LEN;
	printf("len=%d\n", len);
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	assert(dest);
	// extracts characters between m'th and n'th index from source string
	// and copy them into the destination string
	for (int i = 0; i < len && (*src != '\0'); i++) {
		*dest = *(src + i);
		dest++;
	}

	// null-terminate the destination string
	*dest = '\0';
	// return the destination string
	return dest - len;
}

static char *device_ID;
static char *FW_version;
static char *serial_number;

static size_t get_prv_key(unsigned char **prv_key)
{
	FILE *fp;
	size_t sz;
	const char *private_key_path = "./keys/DEFAULT/private_key";

	fp = fopen(private_key_path, "r");
	if (!fp) {
		printf("[-] key (%s) not found...\n", private_key_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	*prv_key = (unsigned char*)malloc(sz);
	assert(*prv_key);

	rewind(fp);
	fread(*prv_key, sizeof(char), sz, fp);
	fclose(fp);
	printf("[*] private key size is %ld, dump:\n", sz);
	hexdump(*prv_key, sz);

	return sz;
}

static size_t get_pub_key(unsigned char **pub_key)
{
	FILE *fp;
	size_t sz;
	const char *public_key_path = "./keys/DEFAULT/public_key";

	fp = fopen(public_key_path, "r");
	if (!fp) {
		printf("[-] key (%s) not found...\n", public_key_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);

	*pub_key = (unsigned char *)malloc(sz);
	assert(pub_key);

	rewind(fp);
	fread(*pub_key, sizeof(char), sz, fp);
	fclose(fp);

	printf("[*] pubKey size is %ld, dump:\n", sz);
	hexdump(*pub_key, sz);

	return sz;
}

int main(int argc, char **argv)
{
	char *challenge_raw;
	unsigned char *challenge;
	char *UUID = "here is UUID 2022 abcd";
	char *UUID_char = ";";
	size_t challenge_len, base64_len;
	size_t device_ID_len, FW_version_len, serial_number_len;
	size_t sz_exp, sz_mod, sz_resp;
	unsigned char *prv_key, *pub_key, *response;
	char *signature;
	mpz_t res, base, exp, mod;

	if (argc != ARGC_NUM) {
		printf("Usage: %s <challenge>\n", argv[0]);
		return -1;
	}

	challenge_raw = argv[1];
	Base64Decode(challenge_raw, &challenge, &challenge_len);
	printf("[*] base64 decoded challenge.\n");
	hexdump(challenge, challenge_len);

	// check device_ID, FW_version, serial_number
	device_ID = substr((const char *)challenge);
	printf("device_ID=%s\n len=%ld\n", device_ID, strlen(device_ID));
	device_ID_len = strlen(device_ID);
	memmove(challenge, challenge + (device_ID_len + 1), challenge_len);
	challenge_len -= (device_ID_len + 1);

	FW_version = substr((const char *)challenge);
	FW_version_len = strlen(FW_version);
	memmove(challenge, challenge + (FW_version_len + 1), challenge_len);
	challenge_len -= (FW_version_len + 1);
	printf("challenge_len=%ld\n", challenge_len);

	serial_number = substr2((const char *)challenge, challenge_len);
	serial_number_len = strlen(serial_number);
	memmove(challenge, challenge + serial_number_len, challenge_len);
	challenge_len -= serial_number_len;
	printf("[*] device_ID is \"%s\"\nFW_version is \"%s\"\nserial_number is \"%s\"\n\n",
		device_ID, FW_version, serial_number);

	// add UUID
	hexdump(challenge, challenge_len);
	memcpy(challenge + challenge_len, UUID_char, strlen(UUID_char));
	memcpy(challenge + challenge_len + strlen(UUID_char), UUID, strlen(UUID));
	challenge_len = challenge_len + strlen(UUID_char) + strlen(UUID);
	hexdump(challenge, challenge_len);

	// get private key as exp
	sz_exp = get_prv_key(&prv_key);
	assert(sz_exp);

	// get public key as mod
	sz_mod = get_pub_key(&pub_key);
	assert(sz_mod);

	// signature
	mpz_init(res);
	mpz_init(base);
	mpz_init(exp);
	mpz_init(mod);

	mpz_import(base, challenge_len, 1, sizeof(char), 0, 0, challenge);
	mpz_import(exp, sz_exp, 1, sizeof(char), 0, 0, prv_key);
	mpz_import(mod, sz_mod, 1, sizeof(char), 0, 0, pub_key);

	// signed
	mpz_powm(res, base, exp, mod);

	response = mpz_export(NULL, &sz_resp, 1, sizeof(char), 0, 0, res);
	printf("[*] signature response size %ld, dump:\n", sz_resp);
	hexdump(response, sz_resp);

	// encode
	base64_len = Base64Encode(response, sz_resp, &signature);
	signature[base64_len] = '\0';

	printf("[*] response of signature after encode :\n%s\n", signature);

	free(response);
	free(device_ID);
	free(FW_version);
	free(serial_number);
	mpz_clear(res);
	mpz_clear(base);
	mpz_clear(exp);
	mpz_clear(mod);

	return 0;
}
