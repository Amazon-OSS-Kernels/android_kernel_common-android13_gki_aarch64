// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

/* #define DEBUG */

#include <common.h>

#include <command.h>
#include <environment.h>
#include <fdtdec.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <search.h>
#include <errno.h>
#include <ufs_scsi.h>

#define __STR(X) #X
#define STR(X) __STR(X)

#if defined(CONFIG_ENV_SIZE_REDUND) &&  \
	(CONFIG_ENV_SIZE_REDUND != CONFIG_ENV_SIZE)
#error CONFIG_ENV_SIZE_REDUND should be the same as CONFIG_ENV_SIZE
#endif

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET 0
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL)
static inline int ufs_offset_try_partition(const char *str, s64 *val)
{
	disk_partition_t info;
	struct blk_desc *desc;
	int len, i, ret;

	ret = blk_get_device_by_str("ufs", STR(CONFIG_SYS_UFS_ENV_DEV), &desc);
	if (ret < 0)
		return (ret);

	for (i = 1;;i++) {
		ret = part_get_info(desc, i, &info);
		if (ret < 0)
			return ret;

		if (!strncmp((const char *)info.name, str, sizeof(str)))
			break;
	}

	/* round up to info.blksz */
	len = (CONFIG_ENV_SIZE + info.blksz - 1) & ~(info.blksz - 1);

	/* use the top of the partion for the environment */
	*val = (info.start + info.size - 1) - len / info.blksz;

	return 0;
}

static inline s64 ufs_offset(int copy)
{
	const struct {
		const char *offset_redund;
		const char *partition;
		const char *offset;
	} dt_prop = {
		.offset_redund = "u-boot,ufs-env-offset-redundant",
		.partition = "u-boot,ufs-env-partition",
		.offset = "u-boot,ufs-env-offset",
	};
	s64 val = 0, defvalue;
	const char *propname;
	const char *str;
	int err;

	/* look for the partition in mmc CONFIG_SYS_MMC_ENV_DEV */
	str = fdtdec_get_config_string(gd->fdt_blob, dt_prop.partition);
	if (str) {
		/* try to place the environment at end of the partition */
		err = ufs_offset_try_partition(str, &val);
		if (!err)
			return val;
	}

	defvalue = CONFIG_ENV_OFFSET;
	propname = dt_prop.offset;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy) {
		defvalue = CONFIG_ENV_OFFSET_REDUND;
		propname = dt_prop.offset_redund;
	}
#endif
	return fdtdec_get_config_int(gd->fdt_blob, propname, defvalue);
}
#else
static inline s64 ufs_offset(int copy)
{
	s64 offset = CONFIG_ENV_OFFSET;

#if defined(CONFIG_ENV_OFFSET_REDUND)
	if (copy)
		offset = CONFIG_ENV_OFFSET_REDUND;
#endif
	return offset;
}
#endif

__weak int ufs_get_env_dev(void)
{
	return CONFIG_SYS_UFS_ENV_DEV;
}

__weak int ufs_get_env_addr(int copy, u32 *env_addr)
{
	s64 offset = ufs_offset(copy);

	if (offset < 0) {
		struct blk_desc *dev_desc = NULL;

		dev_desc = blk_get_dev("ufs", ufs_get_env_dev());
		if(dev_desc)
			offset += dev_desc->lba * dev_desc->blksz;
	}

	*env_addr = offset;

	return 0;
}

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_SPL_BUILD)
static inline int write_env(unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	struct blk_desc *desc = blk_get_dev("ufs", ufs_get_env_dev());
	if(desc == NULL)
		return -1;

	blk_start	= ALIGN(offset, desc->blksz) / desc->blksz;
	blk_cnt		= ALIGN(size, desc->blksz) / desc->blksz;

	n = blk_dwrite(desc, blk_start, blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

static int env_ufs_save(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int dev = ufs_get_env_dev();
	u32	offset;
	int	ret, copy = 0;

	ret = env_export(env_new);
	if (ret)
		goto fini;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == ENV_VALID)
		copy = 1;
#endif

	if (ufs_get_env_addr(copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sUFS(%d)... ", copy ? "redundant " : "", dev);
	if (write_env(CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == ENV_REDUND ? ENV_VALID : ENV_REDUND;
#endif

fini:
	return ret;
}
#endif /* CONFIG_CMD_SAVEENV && !CONFIG_SPL_BUILD */

static inline int read_env(unsigned long size,
			   unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;
	struct blk_desc *desc = blk_get_dev("ufs", ufs_get_env_dev());
	if(desc == NULL)
		return -1;

	blk_start	= ALIGN(offset, desc->blksz) / desc->blksz;
	blk_cnt		= ALIGN(size, desc->blksz) / desc->blksz;

	n = blk_dread(desc, blk_start, blk_cnt, (uchar *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}

#ifdef CONFIG_ENV_OFFSET_REDUND
static int env_ufs_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	u32 offset1, offset2;
	int read1_fail = 0, read2_fail = 0;
	int ret;
	const char *errmsg = NULL;

	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env1, 1);
	ALLOC_CACHE_ALIGN_BUFFER(env_t, tmp_env2, 1);

	ufs_scan(true);

	if (ufs_get_env_addr(0, &offset1) ||
	    ufs_get_env_addr(1, &offset2)) {
		ret = -EIO;
		goto err;
	}

	read1_fail = read_env(CONFIG_ENV_SIZE, offset1, tmp_env1);
	read2_fail = read_env(CONFIG_ENV_SIZE, offset2, tmp_env2);

	ret = env_import_redund((char *)tmp_env1, read1_fail, (char *)tmp_env2,
				read2_fail);

err:
	if (ret)
		set_default_env(errmsg, 0);

#endif
	return ret;
}
#else /* ! CONFIG_ENV_OFFSET_REDUND */
static int env_ufs_load(void)
{
#if !defined(ENV_IS_EMBEDDED)
	ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
	u32 offset;
	int ret;
	const char *errmsg;

	ufs_scan(true);

	if (ufs_get_env_addr(0, &offset)) {
		ret = -EIO;
		goto err;
	}

	if (read_env(CONFIG_ENV_SIZE, offset, buf)) {
		errmsg = "!read failed";
		ret = -EIO;
		goto err;
	}

	ret = env_import(buf, 1);

err:
	if (ret)
		set_default_env(errmsg, 0);
#endif
	return ret;
}
#endif /* CONFIG_ENV_OFFSET_REDUND */

U_BOOT_ENV_LOCATION(ufs) = {
	.location	= ENVL_UFS,
	ENV_NAME("UFS")
	.load		= env_ufs_load,
#ifndef CONFIG_SPL_BUILD
	.save		= env_save_ptr(env_ufs_save),
#endif
};
