// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

/*****************************************************************************/
#include <common.h>
#include <command.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <fs.h>
#include <remoteproc.h>

#include "debug_impl.h"
#include "utility.h"
#include "partition.h"
#include "secure_boot.h"
#include "secure/crypto_auth.h"
#include "secure/secure_common.h"
#ifdef CONFIG_MULTICORES_PLATFORM
#include <smp/thread_info.h>
extern smp_spin_lock_t fs_spin_lock;
#endif

#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
#include <program_efuse_rollback_index.h>
#endif

/*****************************************************************************/
#define PMU_FW_DEBUG		(0)
#define PMU_FW_SIGN			(1)
#define PMU_FW_SIZE			(0x10000)
#define PMU_VERSION_SIZE	(3)		//pmu version magic "V=$(VERSION)"
#define PMU_VERSION_MAGIC	"V="
#define PMU_VERSION_MAX		(32)

/*****************************************************************************/
#define PMU_BUFF_ALIGN		(0x10000)
#define PMU_CMD_SIZE		(64)
#define PMU_DEV_NAME		"pmu_mcs51_rproc"
#define PMU_BOOT_DEV_LEN	(8)
#define PMU_PART_DEV_LEN	(8)
#define PMU_FW_PART			"bootdata"
#if PMU_FW_SIGN
#define PMU_FW_FILE			"/pmu/pmu_mcs51_sign.bin"
#else
#define PMU_FW_FILE			"/pmu/pmu_mcs51_raw.bin"
#endif

/*****************************************************************************/
struct pmu_fw_info {
	const char *file;
	void *buff;
	size_t size;
	bool sign;
};

/*****************************************************************************/
static int pmu_mcs51_get_firmware(struct pmu_fw_info *fw)
{
#if PMU_FW_DEBUG
	static uint8_t fw_dat[] = {
		#include "pmu_mcs51.dat"
	};

	fw->sign = false;
	fw->file = "pmu_mcs51.dat";
	fw->size = sizeof(fw_dat);
	fw->buff = memalign(PMU_BUFF_ALIGN, fw->size);
	if (!fw->buff) {
		UBOOT_ERROR("memalign(0x%X, 0x%tX) fail.\n", PMU_BUFF_ALIGN, fw->size);
		return -ENOMEM;
	}
	memcpy((void *)fw->buff, (void *)fw_dat, fw->size);
#else
	char dev_boot[PMU_BOOT_DEV_LEN] = {0};
	char dev_part[PMU_PART_DEV_LEN] = {0};
	struct blk_desc *blk_dev_desc = NULL;
	int idx_dev = -1;
	unsigned idx_part = -1;
#ifdef CONFIG_MULTICORES_PLATFORM
    unsigned long irq_flag = 0;
#endif

	/* Get ext4ls mmc 0:7 ./pmu about "mmc". */
	if (sys_get_boot_device(dev_boot, sizeof(dev_boot)) == -1) {
		UBOOT_ERROR("sys_get_boot_device(...) fail.\n");
		return -ENODEV;
	}

	/* Get ext4ls mmc 0:7 ./pmu about "0". */
	idx_dev = 0; /* TODO: Change device number(0) hardcode by mtk api. */
	if (!(blk_dev_desc = blk_get_dev(dev_boot, idx_dev))) {
		UBOOT_ERROR("blk_get_dev(%s, %d) fail.\n", dev_boot, idx_dev);
		return -ENODEV;
	}

	/* Get ext4ls mmc 0:7 ./pmu about "7". */
	if (get_partition_info(blk_dev_desc, PMU_FW_PART, NULL, &idx_part) != CMD_RET_SUCCESS) {
		UBOOT_ERROR("get_partition_info(%s, ...) fail.\n", PMU_FW_PART);
		return -ENODEV;
	}

	/* Combine ext4ls mmc 0:7 ./pmu about "0:7". */
	if (snprintf(dev_part, sizeof(dev_part) - 1, "%X:%X", idx_dev, idx_part) < 0) {
		UBOOT_ERROR("snprintf() fail.\n");
		return EINVAL;
	}

#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_lock_save(&fs_spin_lock, irq_flag);
#endif
	/* Seek ext4ls mmc 0:7 ./pmu about "ext4ls mmc 0:7". */
	if (fs_set_blk_dev(dev_boot, dev_part, FS_TYPE_EXT)) {
		UBOOT_ERROR("fs_set_blk_dev(%s, %s, FS_TYPE_EXT) fail.\n", dev_boot, dev_part);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		return -EBUSY;
	}

	/* Fill struct info. */
	fw->sign = PMU_FW_SIGN;
	fw->file = PMU_FW_FILE;
	if (fs_size(fw->file, (loff_t *)&fw->size) < 0) {
		UBOOT_ERROR("fs_size(%s) fail.\n", fw->file);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		return -ENOENT;
	}
	fw->buff = memalign(PMU_BUFF_ALIGN, fw->size);
	if (!fw->buff) {
		UBOOT_ERROR("memalign(0x%X, 0x%tX) fail.\n", PMU_BUFF_ALIGN, fw->size);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		return -ENOMEM;
	}

	/* Seek ext4ls mmc 0:7 ./pmu about "ext4ls mmc 0:7". */
	if (fs_set_blk_dev(dev_boot, dev_part, FS_TYPE_EXT)) {
		UBOOT_ERROR("fs_set_blk_dev(%s, %s, FS_TYPE_EXT) fail.\n", dev_boot, dev_part);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		return -EBUSY;
	}

	/* Get firmware from partition to dram. */
	if (fs_read(fw->file, (ulong)fw->buff, 0, 0, (loff_t *)&fw->size) < 0) {
		UBOOT_ERROR("fs_read(%s) fail.\n", fw->file);
#if defined(CONFIG_MULTICORES_PLATFORM)
        smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
		return -EIO;
	}
#if defined(CONFIG_MULTICORES_PLATFORM)
    smp_spin_unlock_restore(&fs_spin_lock, irq_flag);
#endif
#endif
	flush_cache((ulong)fw->buff, fw->size);
	UBOOT_DEBUG("firmware load file=%s buff=0x%p size=0x%tX sign=%d.\n"
		    , fw->file, fw->buff, fw->size, fw->sign);
	return 0;
}

static int pmu_mcs51_verify_firmware(struct pmu_fw_info *fw)
{
	unsigned char *auth_data = (unsigned char *)fw->buff;
	unsigned int pmu_version = 0;
	int ret = 0;
	st_secure_key key = {0};
#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
	u64 efuse_pmu_ver = 0;
#endif
	/* As follow reference signature flow in build time. */
	unsigned int auth_size = PMU_FW_SIZE + PMU_VERSION_SIZE;
	unsigned char *sign_data = (unsigned char *)fw->buff + PMU_FW_SIZE + PMU_VERSION_SIZE;
	unsigned int sign_size = SIGNATURE_LEN;

	if (fw->sign == false)
		return 0;

	/* Get public key. */
	if (secure_get_key(KEY_TYPE_CKB_A_RI_KEY, &key) == -1) {
		UBOOT_ERROR("secure_get_key() fail.\n");
		return -ENOSYS;
	}

	if ((ret = secure_do_authentication(key.data,
										key.data + RSA_PUBLIC_KEY_N_LEN,
										sign_data,
										auth_data,
										auth_size)) != 0) {
		UBOOT_ERROR("secure_do_authentication() fail.\n");
		UBOOT_DEBUG("RSA_PUBLIC_KEY_N_LEN:\n");
		UBOOT_DUMP((void *)key.data, RSA_PUBLIC_KEY_N_LEN);
		UBOOT_DEBUG("RSA_PUBLIC_KEY_E_LEN:\n");
		UBOOT_DUMP((void *)key.data + RSA_PUBLIC_KEY_N_LEN, RSA_PUBLIC_KEY_E_LEN);
		UBOOT_DEBUG("signature:\n");
		UBOOT_DUMP((void *)sign_data, sign_size);
		UBOOT_DEBUG("firmware:\n");
		UBOOT_DUMP((void *)auth_data, 64);
		return -EPERM;
	}

	if (strncmp(PMU_VERSION_MAGIC, (const char *)(auth_data + PMU_FW_SIZE), PMU_VERSION_SIZE -1))
	{
		UBOOT_ERROR("Incorrect pmu version magic\n");
		return -EPERM;
	}
	pmu_version =  (unsigned int) *(auth_data + PMU_FW_SIZE + (PMU_VERSION_SIZE -1));
	UBOOT_DEBUG("pmu_version = %d\n", pmu_version);

#if (CONFIG_ROLLBACK_INDEX_IN_EFUSE == 1)
	if (pmu_version > PMU_VERSION_MAX)
	{
		UBOOT_ERROR("Invalid pmu version from PM bin%d\n", pmu_version);
		return -EPERM;
	}
	if (read_pmu_rollback_index_by_efuse(&efuse_pmu_ver)==0)
	{
		UBOOT_DEBUG("efuse pmu_version= %d\n", (unsigned int)efuse_pmu_ver);
		if ((unsigned int)efuse_pmu_ver < pmu_version) {
			set_pmu_rollback_index(pmu_version);
			set_pass_rollback_indexes_needed();
		}
		else if (efuse_pmu_ver > pmu_version) {
			UBOOT_ERROR("Anti-rollback on PMU version check failed.\n");
#ifdef CONFIG_AB_SIDELOAD
			run_command("retrycount",0);
			run_command("reset",0);
#else
			run_command("fastboot usb 0",0);
#endif
			return -EPERM;
		}
	}
#endif
	fw->size = auth_size - PMU_VERSION_SIZE;

	return 0;
}

/*****************************************************************************/
int pmu_mcs51_loader(void)
{
	struct udevice *dev = NULL;
	struct pmu_fw_info fw = {0};
	int ret = 0;

	UBOOT_TRACE("load mcs51 via rproc.\n");

	/* Decide by dtbo result. */
	ret = uclass_get_device_by_name(UCLASS_REMOTEPROC, PMU_DEV_NAME, &dev);
	if (ret == -ENODEV) {
		UBOOT_DEBUG("get_device(%s) pass=%d.\n", PMU_DEV_NAME, ret);
		return 0;
	} else if (ret != 0) {
		UBOOT_ERROR("get_device(%s) fail=%d.\n", PMU_DEV_NAME, ret);
		return ret;
	}

	/* Init framework. */
	ret = rproc_init();
	if ((ret != 0) && (ret != -EINVAL)) {
		UBOOT_ERROR("rproc_init() fail=%d.\n", ret);
		return ret;
	}

	/* Obtain firmware buffer. */
	ret = pmu_mcs51_get_firmware(&fw);
	if (ret != 0) {
		UBOOT_ERROR("pmu_mcs51_get_firmware() fail=%d.\n", ret);
		goto pmu_mcs51_loader_err;
	}

	/* Verify firmware buffer. */
	ret = pmu_mcs51_verify_firmware(&fw);
	if (ret != 0) {
		UBOOT_ERROR("pmu_mcs51_verify_firmware() fail=%d.\n", ret);
		goto pmu_mcs51_loader_err;
	}

	/* Load firmware. */
	ret = rproc_load(dev->seq, (ulong)fw.buff, (ulong)fw.size);
	if (ret != 0) {
		UBOOT_ERROR("rproc_load() fail=%d.\n", ret);
		goto pmu_mcs51_loader_err;
	}

	/* Release mcu. */
	ret = rproc_start(dev->seq);
	if (ret != 0)
		UBOOT_ERROR("rproc_start() fail=%d.\n", ret);

	ret = rproc_is_running(dev->seq);
	if (ret != 0)
		UBOOT_ERROR("rpoc did not get to running\n");

pmu_mcs51_loader_err:
	/* Release firmware buffer. */
	if (fw.buff) {
		free(fw.buff);
		fw.buff = NULL;
	}
	return ret;
}

int pmu_mcs51_consulter(void)
{
	struct udevice *dev = NULL;
	int ret = 0;

	UBOOT_TRACE("consult mcs51 via rproc.\n");

	/* Check framework init. */
	if (rproc_is_initialized() == false)
		return -EINVAL;

	/* Decide by dtbo result. */
	ret = uclass_get_device_by_name(UCLASS_REMOTEPROC, PMU_DEV_NAME, &dev);
	if (ret == -ENODEV) {
		UBOOT_DEBUG("get_device(%s) pass=%d.\n", PMU_DEV_NAME, ret);
		return 0;
	} else if (ret != 0) {
		UBOOT_ERROR("get_device(%s) fail=%d.\n", PMU_DEV_NAME, ret);
		return ret;
	}

	/* Check mcu ready. */
	return rproc_is_running(dev->seq);
}
