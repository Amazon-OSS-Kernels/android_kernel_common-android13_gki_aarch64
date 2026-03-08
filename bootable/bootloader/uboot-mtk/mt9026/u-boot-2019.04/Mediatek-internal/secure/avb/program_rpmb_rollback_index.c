// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */


#include <debug_impl.h>
#include <program_rpmb_rollback_index.h>
#include <utility.h>

extern int secure_is_tee_fail(void);

#define TA_RAMLOG_UUID                                                                             \
    {                                                                                              \
        0xa9aa0a93, 0xe9f5, 0x4234, { 0x8f, 0xec, 0x21, 0x09, 0xcb, 0xa2, 0xf6, 0x70 }             \
    }

#define TA_AVB_UUID                                                                                \
    {                                                                                              \
        0x023f8f1a, 0x292a, 0x432b, { 0x8f, 0xc4, 0xde, 0x84, 0x71, 0x35, 0x80, 0x67 }             \
    }


#define TA_AVB_CMD_READ_ROLLBACK_INDEX 0
#define TA_CMD_GET_RPMB_ANTIROLLBACK_CONTROL 109
#define TA_CMD_GET_RPMB_ANTIROLLBACK_VERSION 110


AvbIOResult get_rpmb_rollback_enabling_bit(unsigned int* val)
{
	struct udevice *tee = NULL;
	const struct tee_optee_ta_uuid ramlog_uuid = TA_RAMLOG_UUID;
	int rc = AVB_IO_RESULT_ERROR_IO;
	struct tee_open_session_arg sess_arg;
	memset(&sess_arg, 0, sizeof(sess_arg));

	if(!val)
		return AVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE;

	if (secure_is_tee_fail() != 1)
	{
		UBOOT_ERROR("[ramlog TA] optee is not ready\n");
		return AVB_IO_RESULT_ERROR_IO;
	}

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return AVB_IO_RESULT_ERROR_IO;

	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &ramlog_uuid);
	if (tee_open_session(tee, &sess_arg, 0, NULL))
	{
		UBOOT_ERROR("open ramlog session failed \n");
		return AVB_IO_RESULT_ERROR_IO;
	}
	struct tee_invoke_arg invoke_arg;
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = TA_CMD_GET_RPMB_ANTIROLLBACK_CONTROL;
	invoke_arg.session = sess_arg.session;

	struct tee_param param_ramlog[4];
	memset(param_ramlog, 0, sizeof(param_ramlog));
	param_ramlog[0].attr = TEE_PARAM_ATTR_TYPE_NONE;
	param_ramlog[1].attr = TEE_PARAM_ATTR_TYPE_NONE;
	param_ramlog[2].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	param_ramlog[3].attr = TEE_PARAM_ATTR_TYPE_NONE;
	if (tee_invoke_func(tee, &invoke_arg, ARRAY_SIZE(param_ramlog), param_ramlog))
	{
		UBOOT_ERROR("Failed to invoke\n");
		goto close;
	}

	if(invoke_arg.ret != TEE_SUCCESS)
	{
		UBOOT_ERROR("tee_invoke result code=0x%x\n", invoke_arg.ret);
		goto close;
	}

	*val =  param_ramlog[2].u.value.b;
	rc = AVB_IO_RESULT_OK;

close:
	tee_close_session(tee, sess_arg.session);
	return rc;
}


AvbIOResult read_sboot_rollback_index_by_rpmb(size_t rollback_index_slot, u64 *out_rollback_index)
{
	struct udevice *tee = NULL;
	const struct tee_optee_ta_uuid ramlog_uuid = TA_RAMLOG_UUID;
	int rc = AVB_IO_RESULT_ERROR_IO;
	struct tee_open_session_arg sess_arg;
	memset(&sess_arg, 0, sizeof(sess_arg));

	if(rollback_index_slot > RPMB_OPTEECUST_VERSION)
				return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	if(!out_rollback_index)
		return AVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE;

	if (secure_is_tee_fail() != 1)
	{
		UBOOT_ERROR("[ramlog TA] optee is not ready\n");
		return AVB_IO_RESULT_ERROR_IO;
	}

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return AVB_IO_RESULT_ERROR_IO;

	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &ramlog_uuid);
	if (tee_open_session(tee, &sess_arg, 0, NULL))
	{
		UBOOT_ERROR("open ramlog session failed \n");
		return AVB_IO_RESULT_ERROR_IO;
	}
	struct tee_invoke_arg invoke_arg;
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = TA_CMD_GET_RPMB_ANTIROLLBACK_VERSION;
	invoke_arg.session = sess_arg.session;

	struct tee_param param_ramlog[4];
	memset(param_ramlog, 0, sizeof(param_ramlog));
	param_ramlog[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param_ramlog[0].u.value.a = rollback_index_slot;
	param_ramlog[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	param_ramlog[2].attr = TEE_PARAM_ATTR_TYPE_NONE;
	param_ramlog[3].attr = TEE_PARAM_ATTR_TYPE_NONE;
	if (tee_invoke_func(tee, &invoke_arg, ARRAY_SIZE(param_ramlog), param_ramlog))
	{
		UBOOT_ERROR("Failed to invoke\n");
		goto close;
	}

	if(invoke_arg.ret != TEE_SUCCESS)
	{
		UBOOT_ERROR("tee_invoke result code=0x%x\n", invoke_arg.ret);
		goto close;
	}

	*out_rollback_index =  param_ramlog[1].u.value.b;
	rc = AVB_IO_RESULT_OK;

close:
	tee_close_session(tee, sess_arg.session);
	return rc;
}

AvbIOResult read_avb_rollback_index_by_rpmb(size_t rollback_index_slot, u64 *out_rollback_index)
{

	//if (rollback_index_slot >= TA_AVB_MAX_ROLLBACK_LOCATIONS)
	//	return AVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	struct udevice *tee = NULL;
	const struct tee_optee_ta_uuid avbta_uuid = TA_AVB_UUID;
	int rc = AVB_IO_RESULT_ERROR_IO;
	struct tee_open_session_arg sess_arg;
	memset(&sess_arg, 0, sizeof(sess_arg));

	if(!out_rollback_index)
		return AVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE;

	if (secure_is_tee_fail() != 1)
	{
		UBOOT_ERROR("[avbta] optee is not ready\n");
		return AVB_IO_RESULT_ERROR_IO;
	}

	tee = tee_find_device(tee, NULL, NULL, NULL);
	if (!tee)
		return AVB_IO_RESULT_ERROR_IO;

	tee_optee_ta_uuid_to_octets(sess_arg.uuid, &avbta_uuid);
	if(tee_open_session(tee, &sess_arg, 0, NULL))
	{
		UBOOT_ERROR("open avbta session failed \n");
		return AVB_IO_RESULT_ERROR_IO;
	}

	struct tee_invoke_arg invoke_arg;
	memset(&invoke_arg, 0, sizeof(invoke_arg));
	invoke_arg.func = TA_AVB_CMD_READ_ROLLBACK_INDEX;
	invoke_arg.session = sess_arg.session;

	struct tee_param param[2];
	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param[0].u.value.a = rollback_index_slot;
	param[1].attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT;
	rc = tee_invoke_func(tee, &invoke_arg, ARRAY_SIZE(param), param);

//MTK code starts
//In case of AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS
	if( rc == AVB_IO_RESULT_ERROR_RPMBKEYNOTEXISTS) {
		*out_rollback_index = 0;
		rc = AVB_IO_RESULT_OK;
		goto close;
	}

	if(rc)
	{
		UBOOT_ERROR("tee_invoke result code=0x%x\n", invoke_arg.ret);
		goto close;
	}

	if(invoke_arg.ret != TEE_SUCCESS)
	{
		UBOOT_ERROR("tee_invoke result code=0x%x\n", invoke_arg.ret);
		goto close;
	}


	*out_rollback_index = (u64)param[1].u.value.a << 32 |
			      (u32)param[1].u.value.b;
	rc = AVB_IO_RESULT_OK;

close:
	tee_close_session(tee, sess_arg.session);
	return rc;

	//MTK code ends
}


AvbIOResult read_pmu_rollback_index_by_rpmb(u64 *out_rollback_index)
{
	return read_avb_rollback_index_by_rpmb(PMU_ROLLBACK_INDEX_LOCATION, out_rollback_index);
}

