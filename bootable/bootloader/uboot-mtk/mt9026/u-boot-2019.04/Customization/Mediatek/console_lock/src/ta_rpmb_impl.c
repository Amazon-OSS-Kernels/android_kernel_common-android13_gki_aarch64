// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <common.h>
#include <linux/stddef.h>
#include <linux/compiler.h>
#include <malloc.h>
#include <hexdump.h>
#include <exports.h>
#include <tee.h>
#include <debug_impl.h>
#include "ta_rpmb_impl.h"

#define TA_CE_UUID { 0xaf6f05a8, 0xff9e, 0x4cac, \
                      { 0xbb, 0xd4, 0x40, 0xb0, 0xc2, 0x60, 0x16, 0x25 } }

#define TA_CE_CMD_READ_CONSOLE_UNLOCK_STATE	0
#define TA_CE_CMD_READ_LOG_ENABLE_STATE		1
#define TA_CE_CMD_WRITE_CONSOLE_UNLOCK_STATE	2
#define TA_CE_CMD_WRITE_LOG_ENABLE_STATE	3
#define TA_CE_CMD_GET_NONCE			4

#define CONSOLE_UNLOCK_BIT	0
#define LOG_ENABLE_BIT		1
#define TEE_PARAM_NUM       2

static struct ta_entry {
	struct tee_optee_ta_uuid uuid;
	int (*open_session)(struct udevice *dev, struct tee_open_session_arg *arg,
			uint num_param, struct tee_param *param);
	int (*invoke_func)(struct udevice *dev, struct tee_invoke_arg *arg,
			uint num_param, struct tee_param *param);
	void (*uuid_to_octets)(u8 d[TEE_UUID_LEN], const struct tee_optee_ta_uuid *s);
	struct udevice *(*find_device)(struct udevice *start,
			int (*match)(struct tee_version_data *vers, const void *data),
			const void *data,
			struct tee_version_data *vers);
	int (*close_sess)(struct udevice *dev, u32 session);
} ce_ta_entry = {
	.uuid = TA_CE_UUID,
#ifdef CONFIG_OPTEE_TA_AVB
	.open_session = tee_open_session,
	.invoke_func = tee_invoke_func,
	.close_sess = tee_close_session,
	.uuid_to_octets = tee_optee_ta_uuid_to_octets,
	.find_device = tee_find_device,
#endif
};

static inline int support_check(void)
{
	if (!ce_ta_entry.open_session ||
		!ce_ta_entry.invoke_func ||
		!ce_ta_entry.close_sess ||
		!ce_ta_entry.uuid_to_octets ||
		!ce_ta_entry.find_device) {
		UBOOT_ERROR("Missing u-boot config CONFIG_OPTEE_TA_AVB\n");
		return -EINVAL;
	}

	return 0;
}

static inline int sanity_check(struct udevice *tee)
{
	if (!tee) {
		UBOOT_ERROR("Please find tee device first!\n");
		return -ENODEV;
	}

	return support_check();
}

static inline struct udevice *get_tee_device(void)
{
	struct udevice *tee = NULL;

	tee = ce_ta_entry.find_device(tee,
					NULL, NULL, NULL);
	if (!tee) {
		UBOOT_ERROR("cannot find tee device");
		return NULL;
	}

	UBOOT_DEBUG("CE TA open find tee device done.\n");
	return tee;
}

static int ce_open_session(struct udevice *tee, u32 *session)
{
	int rc;
	struct tee_open_session_arg arg = { };

	rc = sanity_check(tee);
	if (rc)
		return rc;

	ce_ta_entry.uuid_to_octets(arg.uuid, &ce_ta_entry.uuid);
	rc = ce_ta_entry.open_session(tee, &arg, 0, NULL);
	if (rc)
		return rc;

	*session = arg.session;
	UBOOT_DEBUG("Open CE TA with session 0x%x\n", arg.session);

	return 0;
}

static inline int ce_invoke_command(struct udevice *tee,
		struct tee_invoke_arg *arg,
		uint num_param, struct tee_param *param)
{
	if (sanity_check(tee))
		return -EINVAL;

	UBOOT_DEBUG("CE TA invoke command with 0x%x\n", arg->func);
	return ce_ta_entry.invoke_func(tee, arg, num_param, param);
}

static inline int ce_close_session(struct udevice *tee, u32 session)
{
	if (sanity_check(tee))
		return -EINVAL;

	UBOOT_DEBUG("CE TA close session\n");
	return ce_ta_entry.close_sess(tee, session);
}

static int ce_rpmb_get(int bit)
{
	int rc;
	u32 session;
	struct udevice *tee;
	struct tee_invoke_arg arg = { };
	struct tee_param param[TEE_PARAM_NUM] = { };

	rc = support_check();
	if (rc)
		return rc;

	tee = get_tee_device();
	if (!tee)
		return -ENODEV;

	rc = ce_open_session(tee, &session);
	if (rc) {
		UBOOT_ERROR("open session failed with %d\n", rc);
		return rc;
	}

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	arg.session = session;
	switch (bit) {
	case CONSOLE_UNLOCK_BIT:
		arg.func = TA_CE_CMD_READ_CONSOLE_UNLOCK_STATE;
		break;
	case LOG_ENABLE_BIT:
		arg.func = TA_CE_CMD_READ_LOG_ENABLE_STATE;
		break;
	default:
		UBOOT_ERROR("Invaild command with %d\n", bit);
		rc = -EINVAL;
		goto out;
	}

	rc = ce_invoke_command(tee, &arg, ARRAY_SIZE(param), param);
	if (rc) {
		UBOOT_ERROR("invoke command failed with %d\n", rc);
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		rc = (u32)param[0].u.value.a;
		break;
	case TEE_ERROR_OUT_OF_MEMORY:
		rc = -ENOMEM;
		break;
	default:
		rc = -EIO;
	}
out:
	ce_close_session(tee, session);

	return rc;
}

static int ce_rpmb_update(int bit, bool enable)
{
	int rc;
	u32 session;
	struct udevice *tee;
	struct tee_param param[TEE_PARAM_NUM] = { };
	struct tee_invoke_arg arg = { };

	rc = support_check();
	if (rc)
		return rc;

	tee = get_tee_device();
	if (!tee)
		return -ENODEV;

	rc = ce_open_session(tee, &session);
	if (rc) {
		UBOOT_ERROR("open session failed with %d\n", rc);
		return rc;
	}

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = enable;

	arg.session = session;
	switch (bit) {
	case CONSOLE_UNLOCK_BIT:
		arg.func = TA_CE_CMD_WRITE_CONSOLE_UNLOCK_STATE;
		break;
	case LOG_ENABLE_BIT:
		arg.func = TA_CE_CMD_WRITE_LOG_ENABLE_STATE;
		break;
	default:
		UBOOT_ERROR("Invaild command with %d\n", bit);
		goto out;
	}

	rc = ce_invoke_command(tee, &arg, ARRAY_SIZE(param), param);
	if (rc) {
		UBOOT_ERROR("invoke command failed with %d\n", rc);
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		rc = 0;
		break;
	case TEE_ERROR_OUT_OF_MEMORY:
		rc = -ENOMEM;
		break;
	default:
		rc = -EIO;
	}
out:
	ce_close_session(tee, session);

	return rc;
}

int ce_rpmb_get_nonce(void *out, size_t len)
{
	int rc;
	u32 session;
	struct udevice *tee;
	struct tee_invoke_arg arg = { };
	struct tee_param param[TEE_PARAM_NUM] = { };

	rc = support_check();
	if (rc)
		return rc;

	tee = get_tee_device();
	if (!tee)
		return -ENODEV;

	rc = ce_open_session(tee, &session);
	if (rc) {
		UBOOT_ERROR("open session failed with %d\n", rc);
		return rc;
	}

	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;

	arg.session = session;
	arg.func = TA_CE_CMD_GET_NONCE;

	rc = ce_invoke_command(tee, &arg, ARRAY_SIZE(param), param);
	if (rc) {
		UBOOT_ERROR("invoke command failed with %d\n", rc);
		goto out;
	}

	switch (arg.ret) {
	case TEE_SUCCESS:
		memcpy(out, (void *)&param[0].u.value.a, len);
		break;
	case TEE_ERROR_OUT_OF_MEMORY:
		rc = -ENOMEM;
		break;
	default:
		rc = -EIO;
	}
out:
	ce_close_session(tee, session);

	return rc;
}

int ce_rpmb_dump_console_unlock_bit(void)
{
	return ce_rpmb_get(CONSOLE_UNLOCK_BIT);
}

int ce_rpmb_dump_log_enable_bit(void)
{
	return ce_rpmb_get(LOG_ENABLE_BIT);
}

int ce_rpmb_dump_all(void)
{
	printf("CONSOLE UNLOCK: %d\n"
		"LOG ENABLE: %d\n",
		ce_rpmb_dump_console_unlock_bit(),
		ce_rpmb_dump_log_enable_bit());
	return 0;
}

int ce_rpmb_set_console_unlock_bit(bool enable)
{
	return ce_rpmb_update(CONSOLE_UNLOCK_BIT, enable);
}

int ce_rpmb_set_log_enable_bit(bool enable)
{
	return ce_rpmb_update(LOG_ENABLE_BIT, enable);
}
