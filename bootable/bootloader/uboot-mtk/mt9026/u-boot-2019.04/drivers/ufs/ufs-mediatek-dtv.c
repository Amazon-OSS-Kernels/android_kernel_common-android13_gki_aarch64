// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek UFS Host Driver
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Edward-CH Lee <Edward-CH.Lee@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <scsi.h>
#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/arm-smccc.h>
#include <hexdump.h>
#include "ufshcd.h"
#include "ufshcd-pltfrm.h"
#include "ufs-mediatek-dtv.h"

#define ufs_mtk_smc(cmd, val, res) \
	arm_smccc_smc(MTK_SIP_UFS_CONTROL, \
		      cmd, val, 0, 0, 0, 0, 0, &(res))

#define ufs_mtk_va09_pwr_ctrl(res, on) \
	ufs_mtk_smc(UFS_MTK_SIP_VA09_PWR_CTRL, on, res)

#define ufs_mtk_crypto_ctrl(res, enable) \
	ufs_mtk_smc(UFS_MTK_SIP_CRYPTO_CTRL, enable, res)

#define ufs_mtk_ref_clk_notify(on, res) \
	ufs_mtk_smc(UFS_MTK_SIP_REF_CLK_NOTIFICATION, on, res)

#define ufs_mtk_device_reset_ctrl(high, res) \
	ufs_mtk_smc(UFS_MTK_SIP_DEVICE_RESET, high, res)

extern int secure_is_tee_fail(void);

void ufs_mtk_dtv_dbg_register_dump(struct ufs_hba *hba)
{
	ufshcd_dump_regs(hba, UFS_MTK_REG_START, UFS_MTK_REG_SIZE, "Ext Reg ");

	/* Direct debugging information to REG_MTK_PROBE */
	ufshcd_writel(hba, 0x20, REG_UFS_DEBUG_SEL);
	ufshcd_dump_regs(hba, REG_MTK_PROBE, 0x4, "Debug Probe ");
}

static int ufs_mtk_dtv_init(struct ufs_hba *hba)
{
	struct udevice *dev = hba->dev;
	int err = 0;
	struct ufs_mtk_dtv_host *host = dev_get_priv(dev);

	struct ufs_crypto_disable_info *info;
	u32 *val;
	int len = 0;
	size_t sz = 0;
	int i;
	int ret;

	host->crypto_enable = ufshcd_readl(hba, REG_CONTROLLER_ENABLE) & CRYPTO_GENERAL_ENABLE;
	ufshcd_crypto_enable(hba);
	host->use_atf = 0;

	if (host->crypto_enable) {
		// printf("%s:CGE=1\n", __func__);
		// check if we can use ATF to set CGE or not
		ret = secure_is_tee_fail();
		if (ret != 1) {
			// printf("%s:TEE fail or unknow, %d, use QUIRK_BROKEN_HCE\n", __func__, ret);
			hba->quirks |= UFSHCI_QUIRK_BROKEN_HCE;
		}
		else {
			// printf("%s:TEE pass, use atf\n", __func__);
			host->use_atf = 1;
		}
	} else {
		// printf("%s:CGE=0, use standard flow\n", __func__);
	}

	/* ex : crypto-disable-list = <0x0 0x0 0x0 0x10000>, <0x0 0x20000 0x0 0x20000>;
	 * in unit of bytes
	 */

	INIT_LIST_HEAD(&host->crypto_disable_list_head);

	if (!dev_read_prop(dev, "crypto-disable-list", &len)) {
		dev_err(dev, "crypto-disable-list not specified\n");
		goto out;
	}

	if (len <= 0)
		goto out;

	sz = len / sizeof(*val);

	val = devm_kcalloc(dev, sz, sizeof(*val),
			       GFP_KERNEL);
	if (!val) {
		err = -ENOMEM;
		goto out;
	}

	err = dev_read_u32_array(dev, "crypto-disable-list", val, sz);
	if (err && (err != -EINVAL)) {
		dev_err(dev, "%s: error reading array %d\n",
				"crypto-disable-list", err);
		goto out;
	}

	for (i = 0; i < sz; i += 4) {
		info = devm_kzalloc(dev, sizeof(*info), GFP_KERNEL);
		if (!info) {
			err = -ENOMEM;
			goto out;
		}

		info->lba = (((u64)val[i]<<32) | (u64)val[i+1])>>12;
		info->length = (((u64)val[i+2]<<32) | (u64)val[i+3])>>12;
		#ifdef CONFIG_UFS_DEBUG
		printf("%s: lba %llx length %llx\n", "crypto-disable-list",
				info->lba, info->length);
		#endif
		list_add_tail(&info->list, &host->crypto_disable_list_head);
	}
out:

	return err;
}

static void ufs_mtk_dtv_setup_xfer_req(struct ufs_hba *hba, int tag, bool is_scsi_cmd)
{
	struct ufshcd_lrb *lrbp = &hba->lrb[tag];
	struct scsi_cmnd *cmd;
	struct utp_transfer_req_desc *req_desc;
	struct ufs_crypto_disable_info *info;
	struct ufs_mtk_dtv_host *host = dev_get_priv(hba->dev);
	struct list_head *head = &host->crypto_disable_list_head;
	u64 lba = 0;
	u8 crypto_key_slot = 0;
	u32 dword_0;

	if (host->crypto_enable == 0 || lrbp->lun != 0 || is_scsi_cmd == false
		|| (hba->caps & UFSHCD_CAP_CRYPTO) == 0) {
		#ifdef CONFIG_UFS_DEBUG
		printf("crypto-disable-list: crypto_enable %x, lun %x, is_scsi_cmd %x, caps %x\n",
				host->crypto_enable, lrbp->lun, is_scsi_cmd, hba->caps);
		#endif
		return;
	}

	cmd = lrbp->cmd;
	switch (cmd->cmnd[0]) {
	case READ_6:
	case WRITE_6:
		lba = (u64)(be16_to_cpu(*((u16 *)&cmd->cmnd[SCSI_READ_WRITE_CMD_LBA_LOC])));
		break;
	case READ_10:
	case WRITE_10:
		lba = (u64)(be32_to_cpu(*((u32 *)&cmd->cmnd[SCSI_READ_WRITE_CMD_LBA_LOC])));
		break;
	case READ_16:
	case WRITE_16:
		lba = (u64)(be64_to_cpu(*((u64 *)&cmd->cmnd[SCSI_READ_WRITE_CMD_LBA_LOC])));
		break;
	default:
		#ifdef CONFIG_UFS_DEBUG
		printf("crypto-disable-list: cmnd %x\n", cmd->cmnd[0]);
		#endif
		return;
	}

	if (!list_empty(head)) {
		list_for_each_entry(info, head, list) {
			if ((lba >= info->lba) && (lba < (info->lba+info->length))) {
				#ifdef CONFIG_UFS_DEBUG
				printf("crypto-disable-list: lba %llx length %llx\n", info->lba, info->length);
				#endif
				return;
			}
		}
	}

	req_desc = lrbp->utr_descriptor_ptr;
	dword_0 = le32_to_cpu(req_desc->header.dword_0);
	dword_0 |= UTP_REQ_DESC_CRYPTO_ENABLE_CMD;
	dword_0 |= crypto_key_slot;
	req_desc->header.dword_0 = cpu_to_le32(dword_0);
	req_desc->header.dword_1 = cpu_to_le32(lower_32_bits(lba));
	req_desc->header.dword_3 = cpu_to_le32(upper_32_bits(lba));
}

static int ufs_mtk_dtv_hce_enable_notify(struct ufs_hba *hba,
				     enum ufs_notify_change_status status)
{
	struct arm_smccc_res res;
	struct ufs_mtk_dtv_host *host = dev_get_priv(hba->dev);

	if (status == PRE_CHANGE) {
		if (host->crypto_enable && host->use_atf) {
			// printf("%s:use atf\n", __func__);
			ufs_mtk_crypto_ctrl(res, 1);
			if (res.a0) {
				dev_info(hba->dev, "%s: crypto enable failed, err: %lu\n",
					 __func__, res.a0);
			}
		} else {
			// printf("%s:not use atf\n", __func__);
		}
	}

	return 0;
}

static int ufs_mtk_dtv_pre_pwr_change(struct ufs_hba *hba,
				  struct ufs_pa_layer_attr *dev_max_params,
				  struct ufs_pa_layer_attr *dev_req_params)
{
	struct ufs_dev_params host_cap;
	int err;

	host_cap.tx_lanes = UFS_MTK_DTV_LIMIT_NUM_LANES_TX;
	host_cap.rx_lanes = UFS_MTK_DTV_LIMIT_NUM_LANES_RX;
	host_cap.hs_rx_gear = UFS_MTK_DTV_LIMIT_HSGEAR_RX;
	host_cap.hs_tx_gear = UFS_MTK_DTV_LIMIT_HSGEAR_TX;
	host_cap.pwm_rx_gear = UFS_MTK_DTV_LIMIT_PWMGEAR_RX;
	host_cap.pwm_tx_gear = UFS_MTK_DTV_LIMIT_PWMGEAR_TX;
	host_cap.rx_pwr_pwm = UFS_MTK_DTV_LIMIT_RX_PWR_PWM;
	host_cap.tx_pwr_pwm = UFS_MTK_DTV_LIMIT_TX_PWR_PWM;
	host_cap.rx_pwr_hs = UFS_MTK_DTV_LIMIT_RX_PWR_HS;
	host_cap.tx_pwr_hs = UFS_MTK_DTV_LIMIT_TX_PWR_HS;
	host_cap.hs_rate = UFS_MTK_DTV_LIMIT_HS_RATE;
	host_cap.desired_working_mode = UFS_MTK_DTV_LIMIT_DESIRED_MODE;

	err = ufshcd_get_pwr_dev_param(&host_cap,
				       dev_max_params,
				       dev_req_params);
	if (err) {
		dev_err(hba->dev, "%s: failed to determine capabilities\n", __func__);
	}

	return err;
}

static int ufs_mtk_dtv_post_pwr_change(struct ufs_hba *hba,
				  struct ufs_pa_layer_attr *dev_max_params,
				  struct ufs_pa_layer_attr *dev_req_params)
{
	if(dev_req_params->pwr_rx == FASTAUTO_MODE || dev_req_params->pwr_rx == FAST_MODE)
	{
		dev_dbg((const struct device *)hba->dev, "HS-GEAR%d\n", dev_req_params->gear_rx);
	}
	else
	{
		dev_dbg((const struct device *)hba->dev, "PWM-GEAR%d\n", dev_req_params->gear_rx);
	}

	return 0;
}

static int ufs_mtk_dtv_pwr_change_notify(struct ufs_hba *hba,
				     enum ufs_notify_change_status stage,
				     struct ufs_pa_layer_attr *dev_max_params,
				     struct ufs_pa_layer_attr *dev_req_params)
{
	int ret = 0;

	switch (stage) {
	case PRE_CHANGE:
		ret = ufs_mtk_dtv_pre_pwr_change(hba, dev_max_params,
					     dev_req_params);
		break;
	case POST_CHANGE:
		ret = ufs_mtk_dtv_post_pwr_change(hba, dev_max_params,
					     dev_req_params);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*
static int ufs_mtk_pre_link(struct ufs_hba *hba)
{
	int ret = 0;

	return ret;
}

static int ufs_mtk_dtv_post_link(struct ufs_hba *hba)
{
	int ret = 0;

	return ret;
}

static int ufs_mtk_dtv_link_startup_notify(struct ufs_hba *hba,
				       enum ufs_notify_change_status stage)
{
	int ret = 0;

	switch (stage) {
	case PRE_CHANGE:
		ret = ufs_mtk_dtv_pre_link(hba);
		break;
	case POST_CHANGE:
		ret = ufs_mtk_dtv_post_link(hba);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
*/

/**
 * struct ufs_hba_mtk_vops - UFS MTK specific variant operations
 *
 * The variant operations configure the necessary controller and PHY
 * handshake during initialization.
 */
static struct ufs_hba_variant_ops ufs_hba_mtk_vops = {
	.name                = "mediatek-dtv.ufshci",
	.init                = ufs_mtk_dtv_init,
	.hce_enable_notify	 = ufs_mtk_dtv_hce_enable_notify,
	/*.link_startup_notify = ufs_mtk_dtv_link_startup_notify,*/
	.pwr_change_notify   = ufs_mtk_dtv_pwr_change_notify,
	.dbg_register_dump   = ufs_mtk_dtv_dbg_register_dump,
	.setup_xfer_req      = ufs_mtk_dtv_setup_xfer_req
};

static int ufs_mtk_dtv_drv_probe(struct udevice *dev)
{
	int err;

	err = ufshcd_pltfrm_init(dev, &ufs_hba_mtk_vops);
	if (err) {
		dev_err(dev, "ufshcd_pltfrm_init failed %Xh(%d)\n", err, err);
	}

	return err;
}

static const struct udevice_id ufs_mtk_dtv_ids[] = {
	{ .compatible = "mediatek,mt8183-ufshci" },
	{}
};

U_BOOT_DRIVER(mtk_ufs_drv) = {
	.name = "ufshcd-mtk-dtv",
	.id = UCLASS_UFS,
	.of_match = ufs_mtk_dtv_ids,
	.probe = ufs_mtk_dtv_drv_probe,
	.priv_auto_alloc_size = sizeof(struct ufs_mtk_dtv_host),
};

