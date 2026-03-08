/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc.
 */

#ifndef _UFS_MEDIATEK_DTV_H
#define _UFS_MEDIATEK_DTV_H

#include <linux/list.h>

/* UFSHCI MTK Registers */
enum {
	REG_EXTREG1			= 0x2100,
	REG_UFS_DEBUG_SEL	= 0x22C0,
	REG_MTK_PROBE		= 0x22C8,
};

#define UFS_MTK_REG_START	REG_EXTREG1
#define UFS_MTK_REG_END		REG_MTK_PROBE
#define UFS_MTK_REG_SIZE	(UFS_MTK_REG_END-UFS_MTK_REG_START+4)

/*
 * Vendor specific pre-defined parameters
 */
#ifdef CONFIG_HAPS
#define UFS_MTK_DTV_LIMIT_NUM_LANES_RX  1
#define UFS_MTK_DTV_LIMIT_NUM_LANES_TX  1
#define UFS_MTK_DTV_LIMIT_HSGEAR_RX     UFS_HS_G1
#define UFS_MTK_DTV_LIMIT_HSGEAR_TX     UFS_HS_G1
#define UFS_MTK_DTV_LIMIT_PWMGEAR_RX    UFS_PWM_G1
#define UFS_MTK_DTV_LIMIT_PWMGEAR_TX    UFS_PWM_G1
#define UFS_MTK_DTV_LIMIT_RX_PWR_PWM    SLOWAUTO_MODE
#define UFS_MTK_DTV_LIMIT_TX_PWR_PWM    SLOWAUTO_MODE
#define UFS_MTK_DTV_LIMIT_RX_PWR_HS     FAST_MODE
#define UFS_MTK_DTV_LIMIT_TX_PWR_HS     FAST_MODE
#define UFS_MTK_DTV_LIMIT_HS_RATE       0
#define UFS_MTK_DTV_LIMIT_DESIRED_MODE  UFS_PWM_MODE
#else
#define UFS_MTK_DTV_LIMIT_NUM_LANES_RX  1
#define UFS_MTK_DTV_LIMIT_NUM_LANES_TX  1
#define UFS_MTK_DTV_LIMIT_HSGEAR_RX     UFS_HS_G3
#define UFS_MTK_DTV_LIMIT_HSGEAR_TX     UFS_HS_G3
#define UFS_MTK_DTV_LIMIT_PWMGEAR_RX    UFS_PWM_G3
#define UFS_MTK_DTV_LIMIT_PWMGEAR_TX    UFS_PWM_G3
#define UFS_MTK_DTV_LIMIT_RX_PWR_PWM    SLOW_MODE
#define UFS_MTK_DTV_LIMIT_TX_PWR_PWM    SLOW_MODE
#define UFS_MTK_DTV_LIMIT_RX_PWR_HS     FAST_MODE
#define UFS_MTK_DTV_LIMIT_TX_PWR_HS     FAST_MODE
#define UFS_MTK_DTV_LIMIT_HS_RATE       PA_HS_MODE_B
#define UFS_MTK_DTV_LIMIT_DESIRED_MODE  UFS_HS_MODE
#endif

#ifdef CONFIG_ARM64
#define MTK_SIP_SMC_CONVENTION		ARM_SMCCC_SMC_64
#else
#define MTK_SIP_SMC_CONVENTION		ARM_SMCCC_SMC_32
#endif

#define MTK_SIP_SMC_CMD(fn_id) \
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, MTK_SIP_SMC_CONVENTION, \
			ARM_SMCCC_OWNER_SIP, fn_id)
/*
 * SiP commands
 */
#define MTK_SIP_UFS_CONTROL			MTK_SIP_SMC_CMD(0x276)
#define UFS_MTK_SIP_VA09_PWR_CTRL		BIT(0)
#define UFS_MTK_SIP_DEVICE_RESET		BIT(1)
#define UFS_MTK_SIP_CRYPTO_CTRL			BIT(2)
#define UFS_MTK_SIP_REF_CLK_NOTIFICATION	BIT(3)

struct ufs_mtk_dtv_host {
	struct ufs_hba *hba;
	struct list_head crypto_disable_list_head;
	int crypto_enable;
	int use_atf;
};

/**
 * struct ufs_crypto_disable_info - UFS crypto disable info
 * @list: list headed by host->crypto_disable_list_head
 * @lba: logical block address
 * @length: transfer lenth in block
 */
struct ufs_crypto_disable_info {
	struct list_head list;
	u64 lba;
	u64 length;
};

#define SCSI_READ_WRITE_CMD_LBA_LOC	2

#endif /* !_UFS_MEDIATEK_DTV_H */
