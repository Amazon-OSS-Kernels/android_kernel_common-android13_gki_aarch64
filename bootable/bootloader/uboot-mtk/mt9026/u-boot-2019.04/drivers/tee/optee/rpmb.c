// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2018 Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <tee.h>
#include <mmc.h>

#include "optee_msg.h"
#include "optee_private.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
static int emmc_spin_en = 0;
static thread_t  *hold_lock_thread;
extern smp_spin_lock_t emmc_spin_lock;
#endif
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
	u8 rpmb_size_mult;	/* EXT CSD-slice 168: RPMB Size */
	u8 rel_wr_sec_c;	/* EXT CSD-slice 222: Reliable Write Sector */
				/*                    Count */
	u8 ret_code;
#define RPMB_CMD_GET_DEV_INFO_RET_OK     0x00
#define RPMB_CMD_GET_DEV_INFO_RET_ERROR  0x01
};

static void release_mmc(struct optee_private *priv)
{
	int rc;

	if (!priv->rpmb_mmc)
		return;

	rc = blk_select_hwpart_devnum(IF_TYPE_MMC, priv->rpmb_dev_id,
				      priv->rpmb_original_part);
	if (rc)
		debug("%s: blk_select_hwpart_devnum() failed: %d\n",
		      __func__, rc);

	priv->rpmb_mmc = NULL;
}

static struct mmc *get_mmc(struct optee_private *priv, int dev_id)
{
	struct mmc *mmc;
	int rc;
	#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
	thread_t *cur_thread = get_current_thread(get_cpu_id());
	#endif
	if (priv->rpmb_mmc && priv->rpmb_dev_id == dev_id) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		if (cur_thread == hold_lock_thread)
		#endif
		return priv->rpmb_mmc;
	}

	#ifdef CONFIG_MULTICORES_PLATFORM
	smp_spin_lock_save(&emmc_spin_lock, irq_flag);
	hold_lock_thread = get_current_thread(get_cpu_id());

	//For build error
	if (!irq_flag)
		emmc_spin_en = 1;
	else
		emmc_spin_en = irq_flag+1;
	#endif

	release_mmc(priv);

	mmc = find_mmc_device(dev_id);
	if (!mmc) {
		debug("Cannot find RPMB device\n");
		return NULL;
	}
	if (!(mmc->version & MMC_VERSION_MMC)) {
		debug("Device id %d is not an eMMC device\n", dev_id);
		return NULL;
	}
	if (mmc->version < MMC_VERSION_4_41) {
		debug("Device id %d: RPMB not supported before version 4.41\n",
		      dev_id);
		return NULL;
	}

	priv->rpmb_original_part = mmc_get_blk_desc(mmc)->hwpart;

	rc = blk_select_hwpart_devnum(IF_TYPE_MMC, dev_id, MMC_PART_RPMB);
	if (rc) {
		debug("Device id %d: cannot select RPMB partition: %d\n",
		      dev_id, rc);
		return NULL;
	}

	priv->rpmb_mmc = mmc;
	priv->rpmb_dev_id = dev_id;
	return mmc;
}

static u32 rpmb_get_dev_info(u16 dev_id, struct rpmb_dev_info *info)
{
	struct mmc *mmc = find_mmc_device(dev_id);

	if (!mmc)
		return TEE_ERROR_ITEM_NOT_FOUND;

	if (!mmc->ext_csd)
		return TEE_ERROR_GENERIC;

#if 1
//MTK Modified, convert format of MMC CID to TEEOS
	unsigned char ttemp[sizeof(info->cid)];
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			ttemp[4*i+j]=((unsigned char*)mmc->cid)[i*4+3-j];
		}
	}

	memcpy(info->cid, ttemp, sizeof(info->cid));
#else
	memcpy(info->cid, mmc->cid, sizeof(info->cid));
#endif
	info->rel_wr_sec_c = mmc->ext_csd[222];
	info->rpmb_size_mult = mmc->ext_csd[168];
	info->ret_code = RPMB_CMD_GET_DEV_INFO_RET_OK;

	return TEE_SUCCESS;
}

static u32 rpmb_process_request(struct optee_private *priv, void *req,
				ulong req_size, void *rsp, ulong rsp_size)
{
	struct rpmb_req *sreq = req;
	struct mmc *mmc;
	void *req_align = NULL;
	void *rsp_align = NULL;

	if (req_size < sizeof(*sreq))
		return TEE_ERROR_BAD_PARAMETERS;

	switch (sreq->cmd) {
	case RPMB_CMD_DATA_REQ:
		mmc = get_mmc(priv, sreq->dev_id);
		if (!mmc)
			return TEE_ERROR_ITEM_NOT_FOUND;

		req_align = memalign(CONFIG_SYS_CACHELINE_SIZE, (req_size - sizeof(struct rpmb_req)));
		if (!req_align)
			return TEE_ERROR_OUT_OF_MEMORY;
		memcpy(req_align, RPMB_REQ_DATA(req), (req_size - sizeof(struct rpmb_req)));

		rsp_align = memalign(CONFIG_SYS_CACHELINE_SIZE, rsp_size);
		if (!rsp_align) {
			free(req_align);
			return TEE_ERROR_OUT_OF_MEMORY;
		}
		memcpy(rsp_align, rsp, rsp_size);

		if (mmc_rpmb_route_frames(mmc, req_align,
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
	#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
	#endif
	#ifndef CONFIG_MULTICORES_PLATFORM
	release_mmc(dev_get_priv(dev));
	#endif
	#ifdef CONFIG_MULTICORES_PLATFORM
	if (emmc_spin_en > 0 && get_current_thread(get_cpu_id())==hold_lock_thread) {
		release_mmc(dev_get_priv(dev));
		emmc_spin_en = 0;
		hold_lock_thread = NULL;
		smp_spin_unlock_restore(&emmc_spin_lock, irq_flag);
	}
	#endif
}
