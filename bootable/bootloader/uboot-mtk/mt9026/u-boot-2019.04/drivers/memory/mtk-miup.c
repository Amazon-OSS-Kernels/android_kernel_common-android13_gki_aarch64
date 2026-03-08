 // SPDX-License-Iden tifier: GPL-2.0
/*
 * MediaTek DTV MPU Interface driver
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Max Tsai <max-ch.tsai@mediatek.com>
 */

#include <log.h>
#include <linux/soc/mediatek/mtk-miup.h>
#include "mtk-miup-priv.h"
#include <malloc.h>

static int mtk_miup_check_para(struct udevice *dev,
			struct mtk_miup_para *miup)
{
	u32 idx;
	struct mtk_miup *miup_priv = dev_get_priv(dev);

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	debug("protect attribute:%d\n", miup->attr);
	/* check attr value */
	if (miup->attr >= MTK_MIUP_ATTR_MAX) {
		debug("invalid attribute\n");
		return -EINVAL;
	}

	debug("protect block:%d\n", miup->prot_blk);
	/* check prot_blk index */
	if (miup->prot_blk >= miup_priv->max_prot_blk) {
		debug("invalid block\n");
		return -EINVAL;
	}

	/* clean protect check done */
	if (miup->attr == MTK_MIUP_CLEANPROT)
		return 0;

	debug("start_addr:%llX, addr_len:%llX\n",
			miup->start_addr, miup->addr_len);
	/* check address */
	if (miup->addr_len < PROT_ADDR_ALIGN ||
	((miup->start_addr & (PROT_ADDR_ALIGN - 1)) != 0) ||
	((miup->addr_len & (PROT_ADDR_ALIGN - 1)) != 0)) {
		debug("invalid address\n");
		return -EINVAL;
	}

	debug("cid_len:%d", miup->cid_len);
	/* check cid_len size */
	if (miup->cid_len > miup_priv->max_cid_number) {
		debug("invalid cid length\n");
		return -EINVAL;
	}
	/* check array cid */
	if (miup->cid_len != 0 && !miup->cid) {
		debug("null cid\n");
		return -EINVAL;
	}
	for (idx = 0; idx < miup->cid_len; idx++) {
		debug("cid[%d]:0x%llX\n", idx, miup->cid[idx]);
		if (miup->cid[idx] > PROT_CID_MAX_INDEX) {
			debug("invalid cid\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int mtk_miup_clean_prot(struct udevice *dev,
			struct mtk_miup_para *miup)
{
	u8 id_grp = 0;
	u32 idx, idx1;
	u32 pblk = miup->prot_blk;
	struct mtk_miup *miup_priv = dev_get_priv(dev);
	void __iomem *base = NULL;
	u64 *cids_en = NULL;

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	base = miup_priv->base;
	if (!base) {
		debug("no device base\n");
		return -ENXIO;
	}
	cids_en = calloc(miup_priv->max_cid_group, sizeof(u16));
	if (!cids_en)
		return -ENOMEM;
	miup_setbit(pblk, 0, base + REG_PROT_W_EN);
	miup_setbit(pblk, 0, base + REG_PROT_R_EN);
	miup_setbit(pblk, 0, base + REG_PROT_HIT_UNMASK);
	miup_setbit(pblk, 0, base + REG_PROT_ID_GP_SEL);
	writew(0x0, base + PROT_ID_EN_OFFSET(pblk));
	writew(0x0, base + PROT_SADDR_L_OFFSET(pblk));
	writew(0x0, base + PROT_SADDR_H_OFFSET(pblk));
	writew(0x0, base + PROT_EADDR_L_OFFSET(pblk));
	writew(0x0, base + PROT_EADDR_H_OFFSET(pblk));

	/* set cid */
	/* choose cid group */
	id_grp = readb(base + REG_PROT_ID_GP_SEL);
	debug("id group select status[%x]", id_grp);
	for (idx = 0; idx < miup_priv->max_prot_blk; idx++) {
		debug("cids_en_t[%x]", readw(base + PROT_ID_EN_OFFSET(idx)));
		if (((id_grp >> idx) & BIT(0)) >= (miup_priv->max_cid_group))
			goto RET_FUN;

		cids_en[((id_grp >> idx) & BIT(0))] |=
		readw(base + PROT_ID_EN_OFFSET(idx));
	}
	for (idx = 0; idx < miup_priv->max_cid_group; idx++)
		debug("group %d id enable:%llx", idx, cids_en[idx]);
	for (idx = 0; idx < miup_priv->max_cid_group; idx++) {
		for (idx1 = 0; idx1 < miup_priv->max_cid_number; idx1++) {
			if ((cids_en[idx] & BIT(idx1)) == 0) {
				writew(0x0, base + PROT_ID_OFFSET(idx, idx1));
				writew(0xFFFF,
				base + PROT_ID_MASK_OFFSET(idx, idx1));
			}
		}
	}

RET_FUN:
	if (cids_en)
		free(cids_en);
	return 0;
}


static u8 mtk_miup_find_dup(struct udevice *dev,
			u64 *permit_cid,
			u64 *cid,
			u32 cids_idx,
			u8 *find_idx)
{
	u8 ret = 0;
	u32 idx;
	struct mtk_miup *miup_priv = dev_get_priv(dev);

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	for (idx = 0; idx < miup_priv->max_cid_number; idx++) {
		if (permit_cid[idx] == cid[cids_idx]) {
			ret = 1;
			*find_idx = idx;
			break;
		}
	}
	return ret;
}

static int mtk_miup_match_cids(struct udevice *dev,
			struct mtk_miup_para *miup,
			u64 *permit_cid,
			u16 *cids_en)
{
	int ret = 0;
	u32 idx;
	u8 find_idx = 0;
	u64 free_id[1] = {PROT_NULL_CID};

	debug("set cid length: %d", miup->cid_len);
	for (idx = 0; idx < miup->cid_len; idx++) {
		debug("set cid: 0x%llX", miup->cid[idx]);
		if (miup->cid[idx] == PROT_NULL_CID)
			continue;
		if (mtk_miup_find_dup(dev, permit_cid,
				miup->cid, idx, &find_idx)) {
			*cids_en = (*cids_en | BIT(find_idx));
			debug("find 0x%llX in idx%d",
				miup->cid[idx], find_idx);
			continue;
		}

		if (mtk_miup_find_dup(dev, permit_cid,
				free_id, 0, &find_idx) > 0) {
			permit_cid[find_idx] = miup->cid[idx];
			*cids_en = (*cids_en | BIT(find_idx));
			debug("find space idx%d for 0x%llX",
				find_idx, miup->cid[idx]);
			continue;
		}
		ret = -1;
		debug("cannot find space for 0x%llX", miup->cid[idx]);
		break;
	}
	return ret;
}

static int mtk_miup_init_cid(struct udevice *dev,
			struct mtk_miup_para *miup,
			s8 *cid_grp,
			u64 *permit_cid,
			u16 *cid_en)
{
	int ret = 0;
	u8 id_grp = 0;
	u32 idx, idx1;
	struct mtk_miup *miup_priv = dev_get_priv(dev);
	void __iomem *base = NULL;
	u16 *cids_en = NULL;
	u64 *crt_cid = NULL;

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	cids_en = calloc(miup_priv->max_cid_group, sizeof(u16));
	if (!cids_en) {
		ret = -ENOMEM;
		goto RET_FUN;
	}
	crt_cid = calloc(miup_priv->max_cid_number * miup_priv->max_cid_group,
			sizeof(u64));
	if (!crt_cid) {
		ret = -ENOMEM;
		goto RET_FUN;
	}
	base = miup_priv->base;
	if (!base) {
		debug("no device base\n");
		ret = -ENXIO;
		goto RET_FUN;
	}
	id_grp = readb(base + REG_PROT_ID_GP_SEL);
	debug("id group select status[%x]", id_grp);
	for (idx = 0; idx < miup_priv->max_prot_blk; idx++) {
		if (idx == miup->prot_blk)
			continue;

		debug("cids_en_t[%x]", readw(base + PROT_ID_EN_OFFSET(idx)));
		cids_en[((id_grp >> idx) & BIT(0))] |=
		readw(base + PROT_ID_EN_OFFSET(idx));
	}
	for (idx = 0; idx < miup_priv->max_cid_group; idx++) {
		debug("cids enable group %d[%x]", idx, cids_en[idx]);
		for (idx1 = 0; idx1 < miup_priv->max_cid_number; idx1++) {
			crt_cid[(idx * miup_priv->max_cid_number + idx1)] = 0;
			if ((cids_en[idx] & BIT(idx1)) == 0)
				continue;
			crt_cid[(idx * miup_priv->max_cid_number + idx1)] =
			(u64)((readw(base + PROT_ID_OFFSET(idx, idx1)) |
			(readw(base +
			PROT_ID_MASK_OFFSET(idx, idx1)) << 16)) & 0xFFFFFFFF);
		}
	}

	/* for debug print */
	for (idx = 0; idx < miup_priv->max_cid_group; idx++) {
		debug("ID group[%d]", idx);
		for (idx1 = 0; idx1 < miup_priv->max_cid_number; idx1++) {
			debug("ID%d[0x%llX]",
			idx1, crt_cid[idx * miup_priv->max_cid_number + idx1]);
		}
	}

	/* check duplicate cids and free slack */
	for (idx = 0; idx < miup_priv->max_cid_group; idx++) {
		if (*cid_grp >= 0)
			break;
		memcpy(permit_cid, crt_cid + (idx * miup_priv->max_cid_number),
			sizeof(u64) * miup_priv->max_cid_number);

		*cid_grp = idx;
		*cid_en = 0;

		if (mtk_miup_match_cids(dev, miup, permit_cid, cid_en) < 0) {
			*cid_grp = -1;
			*cid_en = 0;
			debug("[can not find space in cid group%d]", idx);
		}
	}
	if (*cid_grp < 0) {
		debug("can not find enough cid space");
		ret = -ENOMEM;
		goto RET_FUN;
	}
	debug("use group%d for cid set", *cid_grp);

RET_FUN:
	if (crt_cid)
		free(crt_cid);
	if (cids_en)
		free(cids_en);
	return ret;
}

static int mtk_miup_set_cid(struct udevice *dev,
			struct mtk_miup_para *miup,
			s8 cid_grp,
			u64 *permit_cid,
			u16 cid_en)
{
	int ret = 0;
	u32 idx;
	struct mtk_miup *miup_priv = dev_get_priv(dev);
	void __iomem *base = NULL;
	u32 pblk = miup->prot_blk;

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	base = miup_priv->base;
	if (!base) {
		debug("no device base\n");
		return -ENXIO;
	}
	if (cid_grp >= miup_priv->max_cid_group) {
		debug("set id group[%d], max_cid_group[%d]",
					cid_grp, miup_priv->max_cid_group);
		ret = -EINVAL;
		goto RET_FUN;
	}
	miup_setbit(pblk, (cid_grp & BIT(0)), base + REG_PROT_ID_GP_SEL);

	/* set ids */
	for (idx = 0; idx < miup_priv->max_cid_number; idx++) {
		debug("id[0x%llX]", permit_cid[idx]);
		writew((u16)(permit_cid[idx] & 0xFFFF),
		base + PROT_ID_OFFSET(cid_grp, idx));
		writew((u16)((permit_cid[idx] >> 16) & 0xFFFF),
		base + PROT_ID_MASK_OFFSET(cid_grp, idx));
	}

	/* set id enable */
	debug("id_en[0x%X]", cid_en);
	writew(cid_en, base + PROT_ID_EN_OFFSET(pblk));

RET_FUN:
	return ret;
}

static int mtk_miup_set_addr(struct udevice *dev,
			struct mtk_miup_para *miup)
{
	int ret = 0;
	struct mtk_miup *miup_priv = dev_get_priv(dev);
	void __iomem *base = NULL;
	u32 pblk = miup->prot_blk;
	u64 saddr_align = miup->start_addr / PROT_ADDR_ALIGN;
	u64 eaddr_align = ((miup->start_addr + miup->addr_len) /
			PROT_ADDR_ALIGN);

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	base = miup_priv->base;
	if (!base) {
		debug("no device base\n");
		return -ENXIO;
	}

	debug("maxlog%d\n", miup_priv->end_addr_ver);
	if (miup_priv->end_addr_ver == END_ADDR_VER_1 || miup_priv->end_addr_ver == END_ADDR_VER_3)
		eaddr_align -= 1;

	debug("saddr:0x%llx, align:0x%llx", miup->start_addr, saddr_align);
	debug("addr_len:0x%llx align:0x%llx", miup->addr_len, eaddr_align);

	writew((saddr_align & 0xFFFF), base + PROT_SADDR_L_OFFSET(pblk));
	writew(((saddr_align >> 16) & 0xFFFF),
		base + PROT_SADDR_H_OFFSET(pblk));
	writew((eaddr_align & 0xFFFF), base + PROT_EADDR_L_OFFSET(pblk));
	writew(((eaddr_align >> 16) & 0xFFFF),
		base + PROT_EADDR_H_OFFSET(pblk));

	return ret;
}

static int mtk_miup_set_prot_en(struct udevice *dev,
			struct mtk_miup_para *miup)
{
	int ret = 0;
	struct mtk_miup *miup_priv = dev_get_priv(dev);
	void __iomem *base = NULL;
	u32 pblk = miup->prot_blk;

	if (!miup_priv) {
		debug("no driver data\n");
		return -ENXIO;
	}
	base = miup_priv->base;
	if (!base) {
		debug("no device base\n");
		return -ENXIO;
	}

	switch (miup->attr) {
	case MTK_MIUP_RPROT:
		miup_setbit(pblk, 1, base + REG_PROT_R_EN);
		miup_setbit(pblk, 0, base + REG_PROT_W_EN);
		break;
	case MTK_MIUP_WPROT:
		miup_setbit(pblk, 0, base + REG_PROT_R_EN);
		miup_setbit(pblk, 1, base + REG_PROT_W_EN);
		break;
	case MTK_MIUP_RWPROT:
		miup_setbit(pblk, 1, base + REG_PROT_R_EN);
		miup_setbit(pblk, 1, base + REG_PROT_W_EN);
		break;
	default:
		debug("set no protect");
		return -EINVAL;
	}
	miup_setbit(pblk, 0, base + REG_PROT_HIT_UNMASK);

	return ret;
}

int mtk_miup_set_protect(struct mtk_miup_para miup)
{
	int ret = 0;
	s8 cid_grp = -1;
	u16 cid_en = 0;
	struct udevice *dev = NULL;
	struct mtk_miup *miup_priv = NULL;
	u64 *permit_cid = NULL;

	uclass_get_device_by_name(UCLASS_MIUP, "miup@1c37e800", &dev);
	if (!dev) {
		debug("No device\n");
		ret = -ENODEV;
		goto RET_FUN;
	}
	miup_priv = dev_get_priv(dev);
	if (!miup_priv || !miup_priv->base) {
		debug("please init first\n");
		ret = -ENODEV;
		goto RET_FUN;
	}
	permit_cid = calloc(miup_priv->max_cid_number, sizeof(u64));
	if (!permit_cid) {
		ret = -ENOMEM;
		goto RET_FUN;
	}

	ret = mtk_miup_check_para(dev, &miup);
	if(ret < 0)
		goto RET_FUN;

	/* clean protection only */
	if (miup.attr == MTK_MIUP_CLEANPROT) {
		mtk_miup_clean_prot(dev, &miup);
		goto RET_FUN;
	}
	/* set ids */
	/* check id free slack and initial ids */
	ret = mtk_miup_init_cid(dev, &miup, &cid_grp, permit_cid, &cid_en);
	if (ret < 0)
		goto RET_FUN;
	mtk_miup_clean_prot(dev, &miup);
	/* wait clean protect first */
	/* set miu protect after original protect clean done */
	wmb();

	ret = mtk_miup_set_cid(dev, &miup, cid_grp, permit_cid, cid_en);
	if (ret < 0)
		goto RET_FUN;

	/* set address */
	ret = mtk_miup_set_addr(dev, &miup);
	if (ret < 0)
		goto RET_FUN;
	/* set protect enable */
	ret = mtk_miup_set_prot_en(dev, &miup);
	if (ret < 0)
		goto RET_FUN;
	/* clean hit log */
	miup_setbit(0, 1, miup_priv->base + REG_PROT_HIT_FLAG);
	miup_setbit(0, 0, miup_priv->base + REG_PROT_HIT_FLAG);

RET_FUN:
	if (permit_cid)
		free(permit_cid);
	debug("set protect done\n");
	return 0;
}

static u32 mtk_miup_get_cpu_base(void)
{
	u32 base = 0;
	ofnode node;
	node = ofnode_path("/memory_info");
	if (!ofnode_valid(node))
		return base;

	if (ofnode_read_u32(node, "cpu_emi0_base", &base) < 0)
		return 0;
	else
		return base;
}

static ofnode mtk_miup_get_kpool_ofnode(u32 kpool_idx)
{
	ofnode node;
	char path[32];

	node.np = NULL;
	if (snprintf(path, sizeof(path), "/mmap_info/MI_KERNEL_POOL%d", kpool_idx) < 0)
		return node;

	node = ofnode_path(path);
	return node;
}

int mtk_miup_set_kernel_protect(void)
{
	int ret = 0;
	ofnode node;
	u32 idx;
	u32 addr[4];
	u32 cpu_base;
	struct udevice *dev = NULL;
	struct mtk_miup *miup_priv = NULL;
	struct mtk_miup_para miup;

	uclass_get_device_by_name(UCLASS_MIUP, "miup@1c37e800", &dev);
	if (!dev) {
		debug("No device\n");
		ret = -ENODEV;
		goto RET_FUN;
	}
	miup_priv = dev_get_priv(dev);

	/* get cpu base */
	cpu_base = mtk_miup_get_cpu_base();

	/* get KERNEL_POOL ofnode */
	for (idx = 0; idx < miup_priv->max_prot_blk; idx++) {
		/* start from kernel_pool1 */
		node = mtk_miup_get_kpool_ofnode((idx + 1));
		if (!ofnode_valid(node)) {
			debug("not find node\n");
			goto RET_FUN;
		}
		if (ofnode_read_u32_array(node, "reg", addr, 4)) {
			debug("not find address\n");
			goto RET_FUN;
		}
		miup.prot_blk = idx;
		miup.start_addr = (((u64)addr[0] << 32) | addr[1]) - cpu_base;
		miup.addr_len = (((u64)addr[2] << 32) | addr[3]);
		miup.cid_len = miup_priv->kcid_num;
		miup.cid = miup_priv->kcid;
		miup.attr = MTK_MIUP_WPROT;
		mtk_miup_set_protect(miup);
	}
RET_FUN:
	return ret;
}

static int mtk_tv_miup_probe(struct udevice *dev)
{
	int ret = 0;
	struct mtk_miup *miup_priv = NULL;

	miup_priv = dev_get_priv(dev);
	if (!miup_priv || !miup_priv->base) {
		debug("please init first\n");
		ret = -ENODEV;
		goto RET_FUN;
	}
	miup_setbit(CHECK_ID_OFFSET, CHECK_AXID, miup_priv->base + REG_PROT_CHECK_ID_SELECT);
	debug("[mtk_miup_probe][init]\n");

RET_FUN:
	return ret;
}

static int mtk_miup_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_miup *miup_priv;
	u32 idx;
	u32 *get_kcid = NULL;

	if (!dev) {
		debug("No device\n");
		return -ENODEV;
	}

	miup_priv = dev_get_priv(dev);
	if (!miup_priv) {
		debug("NULL private data\n");
		return -ENOMEM;
	}

	miup_priv->base = (void *)dev_read_addr_index(dev, 0);

	miup_priv->max_prot_blk =
	dev_read_u32_default(dev, "mediatek,max_prot_blk", 8);

	miup_priv->max_cid_group =
	dev_read_u32_default(dev, "mediatek,max_cid_group", 2);

	miup_priv->max_cid_number =
	dev_read_u32_default(dev, "mediatek,max_cid_number", 16);

	miup_priv->end_addr_ver =
	dev_read_u32_default(dev, "mediatek,end_addr_ver", 1);

	miup_priv->kcid_num =
	dev_read_u32_default(dev, "mediatek,client_cnt", 0);

	if (miup_priv->kcid_num > 0) {
		get_kcid = calloc((miup_priv->kcid_num * 2), sizeof(u32));
		if (!get_kcid) {
			debug("alloc memory error\n");
			miup_priv->kcid_num = 0;
			goto RET_FUNC;
		}
		if (!dev_read_u32_array(dev,
				   "mediatek,client",
				   get_kcid,
				   (miup_priv->kcid_num * 2))) {
			miup_priv->kcid =
				calloc(miup_priv->kcid_num, sizeof(u64));
			if (!miup_priv->kcid) {
				debug("alloc memory error\n");
				miup_priv->kcid_num = 0;
				goto RET_FUNC;
			}
			for (idx = 0; idx < miup_priv->kcid_num; idx++) {
				miup_priv->kcid[idx] =
				((u64)get_kcid[(idx * 2)] |
				((u64)get_kcid[(idx * 2 + 1)] << 32));
			}
		}
	}

RET_FUNC:
	if (miup_priv->base) {
		debug("base=%#lx\n",
				(unsigned long)miup_priv->base);
	}

	debug("%d,%d,%d,%d\n", miup_priv->max_prot_blk,
				miup_priv->max_cid_group,
				miup_priv->max_cid_number,
				miup_priv->kcid_num);

	if (get_kcid)
		free(get_kcid);
	return 0;
}

static const struct udevice_id mtk_miup_ids[] = {
	{ .compatible = "mediatek,mtk-miup" },
	{ }
};

U_BOOT_DRIVER(mtk_miup) = {
	.name = "mtk-tv-miup",
	.id	= UCLASS_MIUP,
	.of_match = mtk_miup_ids,
	.ofdata_to_platdata = mtk_miup_ofdata_to_platdata,
	.probe = mtk_tv_miup_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_miup),
};
