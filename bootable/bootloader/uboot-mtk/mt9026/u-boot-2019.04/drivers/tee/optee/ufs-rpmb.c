// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2018 Linaro Limited
 */

#include <common.h>
#include <memalign.h>
#include <dm.h>
#include <log.h>
#include <tee.h>
#include "../../ufs/ufs.h"
#include <ufs_scsi.h>

#include "optee_msg.h"
#include "optee_private.h"

/*
 * Request and response definitions must be in sync with the secure side of
 * OP-TEE.
 */

/* Request */
struct rpmb_req {
	u16 cmd;
#define RPMB_CMD_DATA_REQ      0x00
#define RPMB_CMD_GET_DEV_INFO  0x01
	u16 dev_id;
	u16 block_count;
	/* Optional data frames (rpmb_data_frame) follow */
};

#define RPMB_REQ_DATA(req) ((void *)((struct rpmb_req *)(req) + 1))

/* Response to device info request */
struct rpmb_dev_info {
	u8 cid[16];
	u8 rpmb_size_mult;	/* Unit Descr 0x0B-0x12: RPMB Logical Block Count */
	u8 rel_wr_sec_c;	/* Undefined: Reliable Write Sector Count */
	u8 ret_code;
#define RPMB_CMD_GET_DEV_INFO_RET_OK     0x00
#define RPMB_CMD_GET_DEV_INFO_RET_ERROR  0x01
};

static void release_ufs(struct optee_private *priv)
{
	if (!priv->rpmb_ufs)
		return;

	priv->rpmb_ufs = NULL;
}

static struct ufs_hba *get_ufs(struct optee_private *priv, int dev_id)
{
	struct ufs_hba *hba;

	if (priv->rpmb_ufs && priv->rpmb_dev_id == dev_id)
		return priv->rpmb_ufs;

	release_ufs(priv);

	hba = find_ufs_device(dev_id);
	if (!hba) {
		debug("Cannot find RPMB device\n");
		return NULL;
	}

	priv->rpmb_ufs = hba;
	priv->rpmb_dev_id = dev_id;
	return hba;
}

static u32 rpmb_get_dev_info(u16 dev_id, struct rpmb_dev_info *info)
{
	struct ufs_hba *hba;
	u8 param_buf[QUERY_DESC_UNIT_DEF_SIZE];
	unsigned long long qLogicalBlockCount;

 	hba = find_ufs_device(dev_id);
	if (!hba)
		return TEE_ERROR_ITEM_NOT_FOUND;

	if (ufs_get_cid(hba, info->cid))
		return TEE_ERROR_GENERIC;

	if (ufs_read_desc(hba, QUERY_DESC_IDN_UNIT, 0xC4, param_buf, QUERY_DESC_UNIT_DEF_SIZE))
		return TEE_ERROR_GENERIC;

	qLogicalBlockCount = ((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 0] << 56)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 1] << 48)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 2] << 40)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 3] << 32)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 4] << 24)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 5] << 16)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 6] << 8)
				|((unsigned long)param_buf[UNIT_DESC_PARAM_LOGICAL_BLK_COUNT + 7] << 0);

	info->rel_wr_sec_c = 1;
	info->rpmb_size_mult = (qLogicalBlockCount >> 9);
	info->ret_code = RPMB_CMD_GET_DEV_INFO_RET_OK;

	return TEE_SUCCESS;
}

static u32 rpmb_process_request(struct optee_private *priv, void *req,
				ulong req_size, void *rsp, ulong rsp_size)
{
	struct rpmb_req *sreq = req;
	struct ufs_hba *hba;
	void *req_align = NULL;
	void *rsp_align = NULL;

	if (req_size < sizeof(*sreq))
		return TEE_ERROR_BAD_PARAMETERS;

	switch (sreq->cmd) {
	case RPMB_CMD_DATA_REQ:
		hba = get_ufs(priv, sreq->dev_id);
		if (!hba)
			return TEE_ERROR_ITEM_NOT_FOUND;

		req_align = memalign(64, (req_size - sizeof(struct rpmb_req)));
		if (!req_align)
			return TEE_ERROR_OUT_OF_MEMORY;
		memcpy(req_align, RPMB_REQ_DATA(req), (req_size - sizeof(struct rpmb_req)));

		rsp_align = memalign(64, rsp_size);
		if (!rsp_align) {
			free(req_align);
			return TEE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(rsp_align, rsp, rsp_size);

		if (ufs_rpmb_route_frames(hba, req_align,
					  req_size - sizeof(struct rpmb_req),
					  rsp_align, rsp_size)) {
			free(req_align);
			free(rsp_align);
			return TEE_ERROR_BAD_PARAMETERS;
		}

		memcpy(rsp, rsp_align, rsp_size);
		free(req_align);
		free(rsp_align);
		return TEE_SUCCESS;

	case RPMB_CMD_GET_DEV_INFO:
		if (req_size != sizeof(struct rpmb_req) ||
		    rsp_size != sizeof(struct rpmb_dev_info)) {
			debug("Invalid req/rsp size\n");
			return TEE_ERROR_BAD_PARAMETERS;
		}
		return rpmb_get_dev_info(sreq->dev_id, rsp);

	default:
		debug("Unsupported RPMB command: %d\n", sreq->cmd);
		return TEE_ERROR_BAD_PARAMETERS;
	}
}

void optee_suppl_cmd_rpmb(struct udevice *dev, struct optee_msg_arg *arg)
{
	struct tee_shm *req_shm;
	struct tee_shm *rsp_shm;
	void *req_buf;
	void *rsp_buf;
	ulong req_size;
	ulong rsp_size;

	if (arg->num_params != 2 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_RMEM_INPUT ||
	    arg->params[1].attr != OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	req_shm = (struct tee_shm *)(ulong)arg->params[0].u.rmem.shm_ref;
	req_buf = (u8 *)req_shm->addr + arg->params[0].u.rmem.offs;
	req_size = arg->params[0].u.rmem.size;

	rsp_shm = (struct tee_shm *)(ulong)arg->params[1].u.rmem.shm_ref;
	rsp_buf = (u8 *)rsp_shm->addr + arg->params[1].u.rmem.offs;
	rsp_size = arg->params[1].u.rmem.size;

	arg->ret = rpmb_process_request(dev_get_priv(dev), req_buf, req_size,
					rsp_buf, rsp_size);
}

void optee_suppl_rpmb_release(struct udevice *dev)
{
	release_ufs(dev_get_priv(dev));
}
