#include <stdio.h>
#include <amzn_secure_boot.h>
#include <amzn_unlock.h>
#include <common.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <idme.h>
#include <utility.h>
#include <unlock_pub_key.h>
#include <amzn_tv_secure_boot.h>
#include <spinlock.h>

#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
#include <program_efuse_rollback_index.h>
#endif
#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
#include <program_rpmb_rollback_index.h>
#endif

extern int secure_is_tee_fail(void);
extern unsigned int CountBitsFromU32(unsigned int val);
extern unsigned int EFuseBitsToVersion(unsigned int val, unsigned int *Is_Inconsistent);
//workaround for emmc cmd sequence broken issue
extern smp_spin_lock_t g_amzn_lock;

anti_rollback_status_type anti_rollback_enabled(void)
{
	unsigned int enabling_bit = 0;
	static anti_rollback_status_type anti_rollback_status = AR_NOT_INITED;

#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
	if (get_efuse_rollback_enabling_bit(&enabling_bit) == 0) {
		anti_rollback_status |= (enabling_bit ? AR_ENABLED_EFUSE : 0);
	} else {
		UBOOT_ERROR("Reading AR efuse failed\n");
		goto ar_failed;
	}
#endif

#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
	if (get_rpmb_rollback_enabling_bit(&enabling_bit) == 0) {
		anti_rollback_status |= (enabling_bit ? AR_ENABLED_RPMB : 0);
		UBOOT_INFO("[%s] RPMB AR enabling_bit:%d\n", __func__, enabling_bit);
	} else {
		UBOOT_ERROR("Reading AR RPMB failed\n");
		goto ar_failed;
	}
#endif

	if (anti_rollback_status == AR_NOT_INITED)
		anti_rollback_status = AR_DISABLED;

	UBOOT_INFO("[%s] anti_rollback_status:%d\n", __func__, (unsigned)anti_rollback_status);
	return anti_rollback_status;

ar_failed:
	/* Reading anti-rollback flag is failed and it's assumed as anti-rollback is enabled */
	anti_rollback_status = AR_ENABLED_ASSUMED;
	UBOOT_ERROR("[%s] anti_rollback_status:%d\n", __func__, (unsigned)anti_rollback_status);
	return anti_rollback_status;
}

#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
#include <errno.h>
#include <malloc.h>
#include <tee.h>
#include <amzn_replay_protected_unlock_mtk_impl.h>
#endif

#if defined(UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)
#include <yubikey_unlock_public_key_tag.h>
#endif


int amzn_target_device_type(void)
{
	if ((anti_rollback_enabled() & (AR_ENABLED_EFUSE|AR_ENABLED_RPMB|AR_ENABLED_ASSUMED)) != AR_NOT_INITED) {
		return AMZN_PRODUCTION_DEVICE;
	}

	return AMZN_ENGINEERING_DEVICE;
}

int target_is_production(void)
{
	if (AMZN_PRODUCTION_DEVICE == amzn_target_device_type())
		return 1;

	return 0;
}

// Override the weak amzn_target_is_unlocked in UFBL
int amzn_target_is_unlocked(void)
{
#if defined(UFBL_FEATURE_UNLOCK)
	unsigned char signed_code[SIGNED_UNLOCK_CODE_LEN] = { 0 };

	if (!idme_get_var_external((const char *)"unlock_code", (char *)signed_code, sizeof(signed_code)) &&
		!amzn_verify_unlock((void*)signed_code, sizeof(signed_code))) {
		return 1;
	}
#endif
	return 0;
}

// 0: locked, otherwise: unlocked
int amzn_device_is_unlocked(void)
{
	int ret = 0;
	unsigned long irq_flag = 0;

	//protect by spin_lock to avoid OPTEE(secure world)
	//and mboot multicore(normal world) access emmc h/w at the same time
	smp_spin_lock_save(&g_amzn_lock, irq_flag);
	ret = amzn_target_is_unlocked()
#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
			|| amzn_target_is_replay_protected_unlocked()
#endif
	;
	smp_spin_unlock_restore(&g_amzn_lock, irq_flag);

	return ret;
}

// 0: success, otherwise: failed
int amzn_device_relock(void)
{
	return idme_update_var_ex("unlock_code", "", 0) != 0
#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
			|| amzn_rpu_relock() != 0
#endif
	;
}

int amzn_unlock_get_soc_id(uint8_t *buf, size_t *len)
{
	uint8_t efuse_id[EFUSE_BUF_SIZE] = {0};
	if (buf == NULL || len == NULL || *len < EFUSE_BUF_SIZE) {
		return -1;
	}

	if (get_device_unique_id(efuse_id, EFUSE_BUF_SIZE) != 0) {
		UBOOT_ERROR("get_device_unique_id failed\n");
		return -1;
	}

	memset(buf, 0, *len);
	memcpy(buf, efuse_id, EFUSE_BUF_SIZE);
	*len = EFUSE_BUF_SIZE;
	return 0;
}

int amzn_get_unlock_code(unsigned char *code, unsigned int *len)
{
	unsigned short efuse_id[EFUSE_BUF_SIZE] = { 0x00, };
	size_t efuse_id_len = sizeof(efuse_id);

	if (!code || !len || *len < (EFUSE_BUF_SIZE + 1))
		return -1;

	if (amzn_unlock_get_soc_id((unsigned char *)efuse_id, &efuse_id_len) != 0)
		return -1;

	*len = sprintf((char *)code, "%04x%04x%04x%04x%04x%04x", efuse_id[5], efuse_id[4],\
							efuse_id[3], efuse_id[2], efuse_id[1], efuse_id[0]);

	return 0;
}

int is_lockdown(void)
{
	/* locked production device */
	if (target_is_production() &&
		!amzn_device_is_unlocked())
		return 1;
	else
		return 0;
}

int chk_cmd_lockdown(const char* command)
{
	if ((is_lockdown()) &&
		((strstr(command, ";")) ||  /* only accept single command in a line */
		(strstr(command, "$")) ||  /* no $ is allowed */
		((strncmp("fastboot", command, strlen("fastboot"))) &&
			(strncmp("reset", command, strlen("reset"))) &&
			(strncmp(command, "usb", strlen("usb")) || strncmp("usb", command, strlen(command)))
#if defined(UFBL_FEATURE_UNLOCK)
			&& (strncmp("unlock_status", command, strlen("unlock_status")))
			&& (strncmp("relock", command, strlen("relock")))
#endif
#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
			&& (strncmp("rpunlock", command, strlen("rpunlock")))
#endif
#if defined(UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)
			&& (strncmp("yk_prov", command, strlen("yk_prov")))
#endif
			)))
		return 0;
	else
		return 1;
}

const unsigned char *amzn_get_unlock_key(unsigned int *key_len)
{
	static const unsigned char unlock_key[] =
#if (defined(SBOOT_abc123_BOARD)||defined(SBOOT_abc123_BOARD)||defined(SBOOT_abc123_BOARD))
	{ MT9025_3P_UNLOCK_PUBKEY };
#elif (defined(SBOOT_GROVEEU_BOARD))
        { MT9025_1P_GROVEEU_UNLOCK_PUBKEY };
#elif (defined(SBOOT_GROVE_BOARD))
        { MT9025_1P_GROVE_UNLOCK_PUBKEY };
#elif (defined(SBOOT_abc123EU_BOARD))
        { abc123EU_UNLOCK_PUBKEY };
#elif (defined(SBOOT_abc123_BOARD))
        { abc123_UNLOCK_PUBKEY };
#else
	{ 0x00 };
#endif

	const int unlock_key_size = sizeof(unlock_key);

	if (!key_len)
		return NULL;

	*key_len = unlock_key_size;
	return unlock_key;
}

#if defined(UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK)
static int invoke_pta(struct tee_invoke_arg *invoke_arg,
		struct tee_param *params, size_t params_len)
{
	int ret = 0;
	struct udevice *dev = NULL;
	struct tee_open_session_arg open_arg = {0};
	const struct tee_optee_ta_uuid uuid = TA_AMZN_UNLOCK_UUID;

	if (invoke_arg == NULL || params == NULL || params_len == 0)
	{
		UBOOT_ERROR("[amzn_rpu] invalid parameters");
		ret = -EINVAL;
		goto exit;
	}

	if (secure_is_tee_fail() != 1)
	{
		UBOOT_ERROR("[amzn_rpu] optee is not ready\n");
		ret = -ENODEV;
		goto exit;
	}

	dev = tee_find_device(NULL, NULL, NULL, NULL);
	if (dev == NULL)
	{
		UBOOT_ERROR("[amzn_rpu] tee_find_device() failed");
		ret = -ENODEV;
		goto exit;
	}

	tee_optee_ta_uuid_to_octets(open_arg.uuid, &uuid);
	ret = tee_open_session(dev, &open_arg, 0, NULL);
	if (ret != 0)
	{
		UBOOT_ERROR("[amzn_rpu] tee_open_session() failed, ret = 0x%x\n", ret);
		goto exit;
	}
	else if (open_arg.ret != TEE_SUCCESS)
	{
		UBOOT_ERROR("[amzn_rpu] tee_open_session result failed, ret = 0x%x, ret_origin=0x%x\n",
				open_arg.ret, open_arg.ret_origin);
		ret = open_arg.ret;
		goto exit;
	}

	invoke_arg->session = open_arg.session;
	ret = tee_invoke_func(dev, invoke_arg, params_len, params);
	if (ret != 0)
	{
		UBOOT_ERROR("[amzn_rpu] tee_invoke_func() failed, ret = 0x%x\n", ret);
	}
	else if (invoke_arg->ret != TEE_SUCCESS)
	{
		UBOOT_ERROR("[amzn_rpu] tee_invoke_func result failed, ret = 0x%x, origin = %d\n",
				invoke_arg->ret, invoke_arg->ret_origin);
		ret = invoke_arg->ret;
	}

	tee_close_session(dev, open_arg.session);

exit:
	return ret;
}

int amzn_rpu_read_unlock_nonce_from_rpmb(amzn_unlock_nonce_t *nonce)
{
	int ret = 0;
	struct tee_invoke_arg invoke_arg = {0};
	struct tee_param params[1] = {0};
	void *invoke_buf = NULL;

	const size_t invoke_buf_len = sizeof(amzn_unlock_nonce_t);
	invoke_arg.func = CMD_AMZN_UNLOCK_READ_RPU_NONCE;

	if (nonce == NULL)
	{
		ret = -EINVAL;
		goto exit;
	}

	invoke_buf =  malloc_TEE(invoke_buf_len);
	if (invoke_buf == NULL)
	{
		UBOOT_ERROR("[amzn_rpu] no enough memory.\n");
		ret = -ENOMEM;
		goto exit;
	}

	params[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	params[0].u.memref.size = invoke_buf_len;
	params[0].u.memref.shm_offs = (ulong)invoke_buf;

	ret = invoke_pta(&invoke_arg, params, sizeof(params) / sizeof(struct tee_param));
	if (ret != 0)
	{
		goto exit;
	}
	if (params[0].u.memref.size != invoke_buf_len)
	{
		UBOOT_ERROR("[amzn_rpu] The data length from PTA is invalid.\n");
		ret = -EINVAL;
		goto exit;
	}

	memcpy(nonce, invoke_buf, invoke_buf_len);

exit:
	free_TEE(invoke_buf);
	return ret;
}

int amzn_rpu_write_unlock_nonce_to_rpmb(const amzn_unlock_nonce_t *nonce)
{
	int ret = 0;
	struct tee_invoke_arg invoke_arg = {0};
	struct tee_param params[1] = {0};
	void *invoke_buf = NULL;

	const size_t invoke_buf_len = sizeof(amzn_unlock_nonce_t);
	invoke_arg.func = CMD_AMZN_UNLOCK_WRITE_RPU_NONCE;

	if (nonce == NULL)
	{
		ret = -EINVAL;
		goto exit;
	}

	invoke_buf =  malloc_TEE(invoke_buf_len);
	if (invoke_buf == NULL)
	{
		UBOOT_ERROR("[amzn_rpu] no enough memory.\n");
		ret = -ENOMEM;
		goto exit;
	}
	memcpy(invoke_buf, nonce, invoke_buf_len);

	params[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	params[0].u.memref.size = invoke_buf_len;
	params[0].u.memref.shm_offs = (ulong)invoke_buf;

	ret = invoke_pta(&invoke_arg, params, sizeof(params) / sizeof(struct tee_param));
	if (ret != 0)
	{
		goto exit;
	}

exit:
	free_TEE(invoke_buf);
	return ret;
}

int amzn_rpu_gen_random_number(uint8_t *buf, size_t len)
{
	return generate_random_number(buf, len);
}

int amzn_rpu_get_soc_id(uint8_t *buf, size_t *len)
{
	return amzn_unlock_get_soc_id(buf, len);
}

#endif    // UFBL_FEATURE_REPLAY_PROTECTED_UNLOCK

#if defined(UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG)
int amzn_yk_get_pub_key_tag_from_rpmb(amzn_yk_pub_key_tag_t *yk_pub_key_tag)
{
	int ret = 0;
	struct tee_invoke_arg invoke_arg = {0};
	struct tee_param params[1] = {0};
	void *invoke_buf = NULL;

	const size_t invoke_buf_len = sizeof(amzn_yk_pub_key_tag_t);
	invoke_arg.func = CMD_AMZN_UNLOCK_READ_PUB_KEY_TAG;

	if (yk_pub_key_tag == NULL)
	{
		ret = -EINVAL;
		goto exit;
	}

	invoke_buf =  malloc_TEE(invoke_buf_len);
	if (invoke_buf == NULL)
	{
		UBOOT_ERROR("[amzn_yk] no enough memory.\n");
		ret = -ENOMEM;
		goto exit;
	}

	params[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_OUTPUT;
	params[0].u.memref.size = invoke_buf_len;
	params[0].u.memref.shm_offs = (ulong)invoke_buf;

	ret = invoke_pta(&invoke_arg, params, sizeof(params) / sizeof(struct tee_param));
	if (ret != 0)
	{
		goto exit;
	}
	if (params[0].u.memref.size != invoke_buf_len)
	{
		UBOOT_ERROR("[amzn_pk] The data length from PTA is invalid.\n");
		ret = -EINVAL;
		goto exit;
	}

	memcpy(yk_pub_key_tag, invoke_buf, invoke_buf_len);

exit:
	free_TEE(invoke_buf);
	return ret;
}

int amzn_yk_write_pub_key_tag_to_rpmb(amzn_yk_pub_key_tag_t *yk_pub_key_tag)
{
	int ret = 0;
	struct tee_invoke_arg invoke_arg = {0};
	struct tee_param params[1] = {0};
	void *invoke_buf = NULL;

	const size_t invoke_buf_len = sizeof(amzn_yk_pub_key_tag_t);
	invoke_arg.func = CMD_AMZN_UNLOCK_WRITE_PUB_KEY_TAG;

	if (yk_pub_key_tag == NULL)
	{
		ret = -EINVAL;
		goto exit;
	}

	invoke_buf =  malloc_TEE(invoke_buf_len);
	if (invoke_buf == NULL)
	{
		UBOOT_ERROR("[amzn_rpu] no enough memory.\n");
		ret = -ENOMEM;
		goto exit;
	}
	memcpy(invoke_buf, yk_pub_key_tag, invoke_buf_len);

	params[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	params[0].u.memref.size = invoke_buf_len;
	params[0].u.memref.shm_offs = (ulong)invoke_buf;

	ret = invoke_pta(&invoke_arg, params, sizeof(params) / sizeof(struct tee_param));
	if (ret != 0)
	{
		goto exit;
	}

exit:
	free_TEE(invoke_buf);
	return ret;
}

// Get the key list for all AMMO variants of one build target
int amzn_yk_get_pub_key_list(const amzn_yk_pub_key_t **key_list)
{
	// keys must end with {NULL, NULL, 0}
	static const amzn_yk_pub_key_t keys[] = {
#if defined(UFBL_PROJ_abc123)
		{YUBIKEY_PUB_KEY_TAG_abc123_LP, g_yubikey_unlock_public_key_abc123_lp,
				sizeof(g_yubikey_unlock_public_key_abc123_lp)},
#elif defined(UFBL_PROJ_abc123)
		{YUBIKEY_PUB_KEY_TAG_abc123_GP, g_yubikey_unlock_public_key_abc123_gp,
				sizeof(g_yubikey_unlock_public_key_abc123_gp)},
#elif defined(UFBL_PROJ_abc123)
		{YUBIKEY_PUB_KEY_TAG_abc123_WP, g_yubikey_unlock_public_key_abc123_wp,
				sizeof(g_yubikey_unlock_public_key_abc123_wp)},
#elif defined(UFBL_PROJ_GROVE)
		{YUBIKEY_PUB_KEY_TAG_GROVE_GM, g_yubikey_unlock_public_key_grove_gm,
				sizeof(g_yubikey_unlock_public_key_grove_gm)},
#elif defined(UFBL_PROJ_GROVEEU)
		{YUBIKEY_PUB_KEY_TAG_GROVEEU_GM, g_yubikey_unlock_public_key_groveeu_gm,
				sizeof(g_yubikey_unlock_public_key_groveeu_gm)},
#elif defined(UFBL_PROJ_abc123)
		{YUBIKEY_PUB_KEY_TAG_abc123_CH, g_yubikey_unlock_public_key_abc123_ch,
				sizeof(g_yubikey_unlock_public_key_abc123_ch)},
#elif defined(UFBL_PROJ_abc123EU)
		{YUBIKEY_PUB_KEY_TAG_abc123EU_PV, g_yubikey_unlock_public_key_abc123eu_pv,
				sizeof(g_yubikey_unlock_public_key_abc123eu_pv)},
		{YUBIKEY_PUB_KEY_TAG_abc123EU_PD, g_yubikey_unlock_public_key_abc123eu_pd,
				sizeof(g_yubikey_unlock_public_key_abc123eu_pd)},
		{YUBIKEY_PUB_KEY_TAG_abc123EU_QV, g_yubikey_unlock_public_key_abc123eu_qv,
				sizeof(g_yubikey_unlock_public_key_abc123eu_qv)},
#endif
		{NULL, NULL, 0},
	};

	*key_list = keys;
	return 0;
}

#endif    // UFBL_FEATURE_YUBIKEY_UNLOCK_PUB_KEY_TAG

#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
unsigned int amzn_antirollback_efuse_version(unsigned char *ar_vers)
{
    struct ar_efuse_version_type *ar_efuse = (struct ar_efuse_version_type *)ar_vers;
    unsigned int i, j, val = 0;
    unsigned int *version_out = 0, version_size = 0;

	for (i = EFUSE_H1L_VER; i < EFUSE_RESERVED;) {
		switch (i)	{
			case EFUSE_H1L_VER:
				version_out = &ar_efuse->hash1_version;
				version_size = EFUSE_H1L_SIZE;
				break;
			case EFUSE_TL_VER:
				version_out = &ar_efuse->teeloader_version;
				version_size = EFUSE_TL_SIZE;
				break;
			case EFUSE_ATF_VER:
				version_out = &ar_efuse->armfw_version;
				version_size = EFUSE_ATF_SIZE;
				break;
			case EFUSE_OPTEE_VER:
				version_out = &ar_efuse->optee_version;
				version_size = EFUSE_OPTEE_SIZE;
				break;
			case EFUSE_RL_VER:
				version_out = &ar_efuse->reeloader_version;
				version_size = EFUSE_RL_SIZE;
				break;
			case EFUSE_UBOOT_VER:
				version_out = &ar_efuse->uboot_version;
				version_size = EFUSE_UBOOT_SIZE;
				break;
			case EFUSE_PMU_VER:
				version_out = &ar_efuse->pmufw_version;
				version_size = EFUSE_PMU_SIZE;
				break;
			case EFUSE_VBMETA_VER:
				version_out = &ar_efuse->vbmeta_version;
				version_size = EFUSE_VBMETA_SIZE;
				break;
			case EFUSE_BOOT_VER:
				version_out = &ar_efuse->boot_version;
				version_size = EFUSE_BOOT_SIZE;
				break;
			case EFUSE_RECOVERY_VER:
				version_out = &ar_efuse->recovery_version;
				version_size = EFUSE_RECOVERY_SIZE;
				break;
			default:
				UBOOT_ERROR("efuse version is not suppored\n");
				return 0;
		}
		*version_out = 0;
		for (j = 0; j < version_size; i++, j++) {
			if (mdrv_efuse_read_subbank_reg_U32(i, &val)) {
				UBOOT_ERROR("failed to read efuse value\n");
				return 0;
			}
			*version_out += CountBitsFromU32(val);
		}
		*version_out = EFuseBitsToVersion(*version_out, NULL);
	}
    return 1;
}
#endif

#if (CONFIG_ROLLBACK_INDEX_IN_RPMB == 1)
unsigned int amzn_antirollback_rpmb_version(unsigned char *ar_vers)
{
	#define RPMB_AVB_OFFSET RPMB_OPTEECUST_VERSION
	uint64_t *version_out;
	unsigned int i, rollback_index_slot;
	AvbIOResult avb_io_result;
	struct ar_rpmb_version_type *ar_rpmb = (struct ar_rpmb_version_type *)ar_vers;
	AvbIOResult (*read_rpmb_version_fn)(size_t rollback_index_slot, u64 *out_rollback_index);

	for (i = RPMB_H1L_VERSION; i < PMU_ROLLBACK_INDEX_LOCATION+1; i++) {
		rollback_index_slot = i;
		switch (i)	{
			case RPMB_H1L_VERSION:
				version_out = &ar_rpmb->hash1_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case RPMB_RL_VERSION:
				version_out = &ar_rpmb->reeloader_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case RPMB_TL_VERSION:
				version_out = &ar_rpmb->teeloader_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case RPMB_OPTEE_VERSION:
				version_out = &ar_rpmb->optee_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case RPMB_ATF_VERSION:
				version_out = &ar_rpmb->armfw_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case RPMB_UBOOT_VERSION:
				version_out = &ar_rpmb->uboot_version;
				read_rpmb_version_fn = read_sboot_rollback_index_by_rpmb;
				break;
			case (RPMB_AVB_OFFSET + VBMETA_ROLLBACK_INDEX_LOCATION):
				version_out = &ar_rpmb->vbmeta_version;
				read_rpmb_version_fn = read_avb_rollback_index_by_rpmb;
				rollback_index_slot = i - RPMB_AVB_OFFSET;
				break;
			case (RPMB_AVB_OFFSET + RECOVERY_ROLLBACK_INDEX_LOCATION):
				version_out = &ar_rpmb->recovery_version;
				read_rpmb_version_fn = read_avb_rollback_index_by_rpmb;
				rollback_index_slot = i - RPMB_AVB_OFFSET;
				break;
			case (RPMB_AVB_OFFSET + BOOT_ROLLBACK_INDEX_LOCATION):
				version_out = &ar_rpmb->boot_version;
				read_rpmb_version_fn = read_avb_rollback_index_by_rpmb;
				rollback_index_slot = i - RPMB_AVB_OFFSET;
				break;
			case PMU_ROLLBACK_INDEX_LOCATION:
				version_out = &ar_rpmb->pmufw_version;
				read_rpmb_version_fn = read_avb_rollback_index_by_rpmb;
				break;
			default:
				UBOOT_ERROR("RPMB anti-rollback version at index #%d is not suppored\n", i);
				return 0;
		}
		*version_out = 0;
		avb_io_result = read_rpmb_version_fn(rollback_index_slot, version_out);

		if (avb_io_result != AVB_IO_RESULT_OK) {
			UBOOT_ERROR("[%s], i:%d, rollback_index_slot:%d, version_out:%llu, avb_io_result:%d\n", \
				__func__, i, rollback_index_slot, *version_out, (unsigned )avb_io_result);
			return 0;
		}
	}
	return 1;
}
#endif
