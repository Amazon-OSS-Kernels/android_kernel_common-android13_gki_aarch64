/*
 * Copyright (c) 2023 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */

#include <common.h>
#include <command.h>

#if defined(UFBL_FEATURE_UNLOCK)
#include <amzn_tv_secure_boot.h>
#endif

#if defined(UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)
#include <yubikey_unlock_public_key_tag.h>
#endif

#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
#include <amzn_replay_protected_unlock.h>

int do_rpunlock(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	uint8_t *b64 = NULL;

	if (argc < 2) {
		printf("Invalid arguments\n");
		return ERR_TEMP_UNLOCK_BAD_ARGUMENT;
	}

	if (!strcmp(argv[1], "getcode")) {
		unsigned char rpu_code[BASE64_LEN(AMZN_RPU_CODE_LEN) + 1] = {0};
		size_t code_len = sizeof(rpu_code);
		if (amzn_rpu_get_unlock_code(rpu_code, &code_len)) {
			printf("get replay protected unlock code error\n");
			ret = ERR_TEMP_UNLOCK_GET_CODE_FAIL;
			goto exit;
		} else {
			printf("%s\n", rpu_code);
		}
	} else if (!strcmp(argv[1], "setcode")) {
		if (argc < 3) {
			printf("Invalid arguments\n");
			ret = ERR_TEMP_UNLOCK_BAD_ARGUMENT;
			goto exit;
		}

		b64 = (uint8_t *)argv[2];
		if (amzn_rpu_set_idme_unlock_code(b64, strlen((const char *)b64))) {
			printf("set replay protected unlock code error\n");
			ret = ERR_TEMP_UNLOCK_SET_CODE_FAIL;
			goto exit;
		} else {
			printf("set replay protected unlock code OKAY\n");
		}
	} else if (!strcmp(argv[1], "setcert")) {
		if(argc < 3) {
			printf("Invalid arguments\n");
			ret = ERR_TEMP_UNLOCK_BAD_ARGUMENT;
			goto exit;
		}

		b64 = (uint8_t *)argv[2];
		if (amzn_rpu_set_idme_unlock_cert(b64, strlen((const char *)b64))) {
			printf("set replay protected unlock cert error\n");
			ret = ERR_TEMP_UNLOCK_SET_CERT_FAIL;
			goto exit;
		} else {
			printf("set replay protected unlock cert OKAY\n");
		}
	} else {
		printf("Invalid argument: %s\n", argv[1]);
		ret = ERR_TEMP_UNLOCK_BAD_ARGUMENT;
		goto exit;
	}

exit:
	if (ret)
		printf("do_rpunlock fail: %d\n", ret);
	else
		printf("do_rpunlock pass\n");
	return ret;
}

U_BOOT_CMD(
	rpunlock, 3, 0, do_rpunlock,
	"Replay protected unlock",
	"getcode\n"
	"rpunlock setcode <signed_code>\n"
	"rpunlock setcert <signed_cert>\n"
);
#endif

#if defined(UFBL_FEATURE_UNLOCK)
int do_relock(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = amzn_device_relock();
	if (ret) {
		printf("do_relock failed\n");
	} else {
		printf("do_relock pass\n");
	}
	return ret;
}

U_BOOT_CMD(
	relock ,    1,    0,     do_relock,
	"Relock device",
	"\n"
);

int do_unlock_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("unlock_status: %d\n", amzn_device_is_unlocked());
	return 0;
}

U_BOOT_CMD(
	unlock_status ,    1,    0,     do_unlock_status,
	"Check device unlock status",
	"\n"
);
#endif

#if defined(UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)
int do_yubikey_tag_prov(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = -1;

	if (argc == 3 && !strcmp(argv[1], "-p")) {
		ret = amzn_yk_prov_tag(argv[2]);
	} else {
		printf("Invalid arguments\n");
	}

	printf("do_yubikey_tag_prov %s\n", ret == 0 ? "success" : "failed");
	return ret;
}

U_BOOT_CMD(
	yk_prov ,    3,    0,     do_yubikey_tag_prov,
	"Yubikey tag provision",
	"Provision tag (one-time-operation):\n"
	"    yk_prov -p <yubikey_tag>\n"
);
#endif
