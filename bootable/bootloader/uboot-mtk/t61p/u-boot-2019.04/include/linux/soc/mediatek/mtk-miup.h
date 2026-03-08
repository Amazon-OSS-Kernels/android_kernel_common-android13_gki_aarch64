/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020
 */

#ifndef _MTK_MIUP_H_
#define _MTK_MIUP_H_

#ifndef __UBOOT__
#include <linux/types.h>
#include <linux/uio.h>
#include <linux/notifier.h>
#include <linux/device.h>
#include <asm/div64.h>
#else
#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <div64.h>
#endif
#include <dm/device.h>
#include <dm/read.h>
#include <asm/io.h>
#include <linux/bitops.h>

enum {
	MTK_MIUP_RPROT = 0,
	MTK_MIUP_WPROT,
	MTK_MIUP_RWPROT,
	MTK_MIUP_CLEANPROT,
	MTK_MIUP_ATTR_MAX,
};

struct mtk_miup_hitlog {
	u64	cid;
	u64	hit_addr;
	u32	hit_blk;
	/* 0: read; 1: write; 2: read/write */
	u32	attr;
};

struct mtk_miup_para {
	u32	prot_blk;
	u64	start_addr;
	u64	addr_len;
	u32	cid_len;
	u64	*cid;
	s32	attr;
};

struct miup_info{
	int (*_memprot)(struct mtk_miup_para miup);
};

int mtk_miup_probe(struct udevice *dev);
int mtk_miup_probe_devices(void);
int mtk_miup_set_protect(struct mtk_miup_para miup);
int mtk_miup_set_kernel_protect(void);

#endif  /* _MTK_MIUP_H_ */

