// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#include <common.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>

#include <nand.h>
#include <asm/io.h>
#include <dm.h>

#include "mtk-dtv-nand.h"
#include "drvNAND.h"

static int probe_done;
static int read_byte_idx;

/*** nand_chip replacable API ***/
static uint8_t mtk_fcie_nand_read_byte(struct mtd_info *mtd)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;
	uint8_t val;

	val = pu8_SpareBuf[read_byte_idx];
 	read_byte_idx++;

	return (val);
}

static void mtk_fcie_nand_select_chip(struct mtd_info *mtd, int chip)
{
	// DUMMY
}

static int mtk_fcie_nand_block_bad(struct mtd_info *mtd, loff_t ofs)
{
	#define BADBLOCKBITS	8
	#define BADBLOCKMARKER	0xFF
	#define SCAN2NDPAGE	2

	int page, res = 0, i = 0;
	struct nand_chip *chip = mtd_to_nand(mtd);
	u16 bad;
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_DataBuf = pNandDrv->pu8_PageDataBuf;
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	if (chip->bbt_options & NAND_BBT_SCANLASTPAGE)
		ofs += mtd->erasesize - mtd->writesize;

	page = (int)(ofs >> chip->page_shift) & chip->pagemask;

	do {
		u32_Err = NC_ReadPages(page, pu8_DataBuf, pu8_SpareBuf, 1);
		if (u32_Err)
			pr_info("%s error %X\n", __func__, u32_Err);

		bad = pu8_SpareBuf[chip->badblockpos];
		if (likely(chip->badblockbits == BADBLOCKBITS))
			res = bad != BADBLOCKMARKER;
		else
			res = hweight8(bad) < chip->badblockbits;
		ofs += mtd->writesize;
		page = (int)(ofs >> chip->page_shift) & chip->pagemask;
		i++;
	} while (!res && i < SCAN2NDPAGE && (chip->bbt_options & NAND_BBT_SCAN2NDPAGE));

	return res;
}

static void mtk_fcie_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column,
	int page_addr)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	if (command == NAND_CMD_RESET) {
		NC_ResetNandFlash();
	} else if (command == NAND_CMD_READID) {
		read_byte_idx = 0;
		NC_ReadID();
		memcpy(pu8_SpareBuf, pNandDrv->au8_ID, NAND_ID_BYTE_CNT);
	} else if (command == NAND_CMD_STATUS) {
		read_byte_idx = 0;
		NC_ReadStatus();
	} else if (command == NAND_CMD_ERASE1) {
		u32_Err = NC_EraseBlk(page_addr);
		pu8_SpareBuf[0] = u32_Err; //checked by waitfunc
	} else if (command == NAND_CMD_ERASE2) {

	} else {
		pr_err("error, unsupported command 0x%X\n", command);
	}
}

static int mtk_fcie_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	u32_Err = pu8_SpareBuf[0];
	if (u32_Err)
		return NAND_STATUS_FAIL;

	return (int)REG(NC_ST_READ);
}

/*** nand_ecc_ctrl replacable API ***/
static int mtk_fcie_nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf, int oob_required, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = oob_required ? chip->oob_poi : pNandDrv->pu8_PageSpareBuf;

	u32_Err = NC_ReadPages(page, buf, pu8_SpareBuf, 1);
	if (u32_Err)
		pr_info("%s error %X\n", __func__, u32_Err);

	return 0;
}

static int mtk_fcie_nand_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
		const uint8_t *buf, int oob_required, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = oob_required ? chip->oob_poi : pNandDrv->pu8_PageSpareBuf;

	if (!oob_required)
		memset(pNandDrv->pu8_PageSpareBuf, MAX_U8_VALUE, pNandDrv->u16_SpareByteCnt);

	u32_Err = NC_WritePages(page, (U8 *)buf, pu8_SpareBuf, 1);
	if (u32_Err)
		return -EIO;

	return 0;
}

static int mtk_fcie_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf, int oob_required, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = oob_required ? chip->oob_poi : pNandDrv->pu8_PageSpareBuf;
	unsigned int max_bitflips = 0;

	u32_Err = NC_ReadPages(page, buf, pu8_SpareBuf, 1);
	if (u32_Err)
		mtd->ecc_stats.failed++;
	else {
		mtd->ecc_stats.corrected += NC_GetECCBits();
		max_bitflips = mtd->ecc_stats.corrected;
	}

	return max_bitflips;
}

static int mtk_fcie_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
		const uint8_t *buf, int oob_required, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = oob_required ? chip->oob_poi : pNandDrv->pu8_PageSpareBuf;

	if (!oob_required)
		memset(pNandDrv->pu8_PageSpareBuf, MAX_U8_VALUE, pNandDrv->u16_SpareByteCnt);

	u32_Err = NC_WritePages(page, (U8 *)buf, pu8_SpareBuf, 1);
	if (u32_Err)
		return -EIO;

	return 0;
}

static int mtk_fcie_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_DataBuf = pNandDrv->pu8_PageDataBuf;

	u32_Err = NC_ReadPages(page, pu8_DataBuf, chip->oob_poi, 1);
	if (u32_Err)
		pr_info("%s error %X\n", __func__, u32_Err);

	return 0;
}

static int mtk_fcie_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_DataBuf = pNandDrv->pu8_PageDataBuf;

	memset(pNandDrv->pu8_PageDataBuf, MAX_U8_VALUE, pNandDrv->u16_PageByteCnt);

	u32_Err = NC_WritePages(page, pu8_DataBuf, chip->oob_poi, 1);
	if (u32_Err)
		return -EIO;

	return 0;
}

static int fill_ecclayout(struct nand_ecclayout *layout)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 idx, i, oob_poi;

	layout->eccbytes = pNandDrv->u16_ECCCodeByteCnt * pNandDrv->u16_PageSectorCnt;

	for (idx = 0 ; idx < pNandDrv->u16_PageSectorCnt ; idx++) {
		oob_poi = ((idx + 1) * pNandDrv->u16_SectorSpareByteCnt)
			- pNandDrv->u16_ECCCodeByteCnt;
		for (i = 0 ; i < pNandDrv->u16_ECCCodeByteCnt ; i++)
			layout->eccpos[(idx * pNandDrv->u16_ECCCodeByteCnt) + i] = oob_poi++;

		layout->oobfree[idx].offset = idx * pNandDrv->u16_SectorSpareByteCnt;
		layout->oobfree[idx].length = pNandDrv->u16_SectorSpareByteCnt
			- pNandDrv->u16_ECCCodeByteCnt;
	}
	layout->oobfree[idx].length = 0;

	layout->oobavail = 0;
	for (i = 0 ; i < MTD_MAX_OOBFREE_ENTRIES_LARGE && layout->oobfree[i].length ; i++)
		layout->oobavail += layout->oobfree[i].length;

	return 0;
}

static int fill_nand_flash_dev(struct nand_flash_dev *table)
{
	#define SHIFT_1K	10
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	table->name = (char *)pNandDrv->u8_PartNumber;
	memcpy((void *)table->id, (const void *)pNandDrv->au8_ID, pNandDrv->u8_IDByteCnt);
	table->id_len = (uint16_t)pNandDrv->u8_IDByteCnt;
	table->pagesize = (int)pNandDrv->u16_PageByteCnt;
	table->erasesize = (unsigned int)pNandDrv->u16_BlkPageCnt
		* (unsigned int)pNandDrv->u16_PageByteCnt;
	table->chipsize = ((table->erasesize >> SHIFT_1K) * (unsigned int)pNandDrv->u16_BlkCnt)
		>> SHIFT_1K;
	table->options = 0;
	table->oobsize = (uint16_t)pNandDrv->u16_SpareByteCnt;
	table->ecc.strength_ds = pNandDrv->u16_ECCCorretableBit;
	table->ecc.step_ds = pNandDrv->u16_SectorByteCnt;
	table->onfi_timing_mode_default = 0;

	return 0;
}

static int mtk_fcie_nand_probe(struct udevice *dev)
{
	struct mtk_fcie_nand_host *host = dev_get_priv(dev);
	int ret;
	struct mtd_info *mtd;
	struct nand_chip *chip;
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (probe_done == 1)
		return 0;

	u32_Err = drvNAND_Init((void *)host);
	if (u32_Err) {
		dev_err(dev, "Failed to init NAND driver: 0x%X\n", u32_Err);
		return -ENODEV;
	}

	chip = &host->chip;
	chip->flash_node = dev_of_offset(dev);
	chip->read_byte = mtk_fcie_nand_read_byte;
	chip->select_chip = mtk_fcie_nand_select_chip;
	chip->block_bad = mtk_fcie_nand_block_bad;
	chip->cmdfunc = mtk_fcie_nand_cmdfunc;
	chip->waitfunc = mtk_fcie_nand_waitfunc;
	chip->options |= NAND_NO_SUBPAGE_WRITE | NAND_USE_BOUNCE_BUFFER;
	chip->buf_align = ARCH_DMA_MINALIGN;

	ret = fill_ecclayout(&host->ecclayout);
	if (ret) {
		dev_err(dev, "Failed to fill ecc layout: %d\n", ret);
		return ret;
	}
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = pNandDrv->u16_SectorByteCnt;
	chip->ecc.bytes = pNandDrv->u16_ECCCodeByteCnt;
	chip->ecc.strength = pNandDrv->u16_ECCCorretableBit;
	chip->ecc.options |= NAND_ECC_CUSTOM_PAGE_ACCESS;
	chip->ecc.layout = &host->ecclayout;
	chip->ecc.read_page_raw = mtk_fcie_nand_read_page_raw;
	chip->ecc.write_page_raw = mtk_fcie_nand_write_page_raw;
	chip->ecc.read_page = mtk_fcie_nand_read_page;
	chip->ecc.write_page = mtk_fcie_nand_write_page;
	chip->ecc.read_oob = mtk_fcie_nand_read_oob;
	chip->ecc.write_oob = mtk_fcie_nand_write_oob;

	ret = fill_nand_flash_dev(&host->table);
	if (ret) {
		dev_err(dev, "Failed to fill nand flash dev: %d\n", ret);
		return ret;
	}

	mtd = nand_to_mtd(chip);
	mtd->dev = dev;
	ret = nand_scan_ident(mtd, CONFIG_SYS_NAND_MAX_CHIPS, &host->table);
	if (ret) {
		dev_err(dev, "Failed to scan ident MTD: %d\n", ret);
		return ret;
	}

	if (chip->bbt_options & NAND_BBT_USE_FLASH)
		chip->bbt_options |= NAND_BBT_NO_OOB;

	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(dev, "Failed to scan tail MTD: %d\n", ret);
		return ret;
	}

	ret = nand_register(0, mtd);
	if (ret) {
		dev_err(dev, "Failed to register MTD: %d\n", ret);
		return ret;
	}

	probe_done = 1;

	return ret;
}

static int mtk_fcie_nand_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_fcie_nand_host *host = dev_get_priv(dev);
	int ret;
	size_t sz = 0;
	int len = 0;

	// fcie riu base
	host->fciebase = (void *)dev_read_addr_name(dev, "fcie");
	if (!host->fciebase) {
		dev_err(dev, "Failed to read %s fciebase\n", dev->name);
		return -EINVAL;
	}
	dev_info(dev, "fciebase = %p\n", host->fciebase);

	// clkgen base and definition
	host->clk_info.clkbase = (void *)dev_read_addr_name(dev, "clk");
	if (!host->clk_info.clkbase) {
		dev_err(dev, "Failed to read %s clkbase\n", dev->name);
		return -EINVAL;
	}
	dev_info(dev, "clkbase = %p\n", host->clk_info.clkbase);

	if(dev_read_u32(dev, "clk-shift", &host->clk_info.clk_shift)) {
		dev_err(dev, "Failed to read %s clk-shift\n", dev->name);
		return -EINVAL;
	}
	dev_info(dev, "clk-shift = 0x%x\n", host->clk_info.clk_shift);

	if(dev_read_u32(dev, "clk-bit-cnt", &host->clk_info.clk_bit_cnt)) {
		dev_err(dev, "Failed to read %s clk-bit-cnt\n", dev->name);
		return -EINVAL;
	}
	dev_info(dev, "clk-bit-cnt = 0x%x\n", host->clk_info.clk_bit_cnt);

	if (!dev_read_prop(dev, "freq-table-hz", &len) || len <= 0) {
		dev_err(dev, "Failed to read %s freq-table-hz\n", dev->name);
		return -EINVAL;
	}
	sz = len / sizeof(struct mtk_fcie_nand_clk_table);
	if (sz > MAX_CLK_SOURCE)
		sz = MAX_CLK_SOURCE;

	len = (sz * sizeof(struct mtk_fcie_nand_clk_table)) /sizeof(u32);
	ret = dev_read_u32_array(dev, "freq-table-hz", (u32 *)&host->clk_info.clk_table[0], len);
	if (ret) {
		dev_err(dev, "Failed to read %s freq-table-hz settings\n", dev->name);
		return ret;
	}
	host->clk_info.clk_cnt = sz;
	dev_info(dev, "clk_cnt = %ld\n", sz);
	for (len = 0 ; len < sz ; len++) {
		dev_info(dev, "[%d] clk_hz = %d, value = %d\n", len,
			host->clk_info.clk_table[len].clk_hz,
			host->clk_info.clk_table[len].value);
	}

	return 0;
}

static const struct udevice_id mtk_fcie_nand_ids[] = {
	{
		.compatible = "mediatek,nand-fcie",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mtk_fcie_nand) = {
	.name = "mtk_fcie_nand",
	.id = UCLASS_MTD,
	.of_match = mtk_fcie_nand_ids,
	.ofdata_to_platdata = mtk_fcie_nand_ofdata_to_platdata,
	.probe = mtk_fcie_nand_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_fcie_nand_host),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD, DM_GET_DRIVER(mtk_fcie_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name, ret);
}

#if defined(CONFIG_SYS_MTDPARTS_RUNTIME)
#define MTDIDS_MAXLEN		128
#define MTDPARTS_MAXLEN		512
#define PARTITION_MAXLEN	16
static char ids[MTDIDS_MAXLEN];
static char parts[MTDPARTS_MAXLEN];

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	static int initialized = 0;

	struct mtd_info *mtd = get_mtd_device(NULL, 0);
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_DataBuf = pNandDrv->pu8_PageDataBuf;
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;
	PARTITION_INFO_t *pPartInfo = pNandDrv->pPartInfo;
	PARTITION_RECORD_t *pRecord;
	U16 u16_BlkKbyteCnt = pNandDrv->u16_BlkPageCnt * (pNandDrv->u16_PageByteCnt >> 10);
	int i;
	char *p = parts;
	int len = MTDPARTS_MAXLEN;
	u32 PartSize, PartSize1, PartSize2;
	char PartName[PARTITION_MAXLEN];
	U16 u16_PBA;
	U8 u8_MbootCnt= 0;
	U16 u16_MbootBlk[2];

	if (!mtd)
		return;

	if (initialized == 0) {
		snprintf(ids, sizeof(ids), "%s=%s", mtd->name, mtd->dev->name);
		snprintf(p, len, "mtdparts=%s:", mtd->dev->name);
		len -= strlen(p);
		p += strlen(p);

		for (i = 0 ; i < pPartInfo->u16_PartCnt; i++) {
			pRecord = &pPartInfo->records[i];
			if (pRecord->u16_PartType & UNFD_LOGI_PART)
				break;

			PartSize = (pRecord->u16_BackupBlkCnt + pRecord->u16_BlkCnt)
				* u16_BlkKbyteCnt;

			switch(pRecord->u16_PartType) {
			case UNFD_PART_MBOOT:
				// MBOOT partition must be the first partition
				if (i != 0) {
					snprintf(parts, MTDPARTS_MAXLEN,
						"mtdparts=%s:-(ALL)", mtd->dev->name);
					goto out;
				}
				snprintf(PartName, PARTITION_MAXLEN, "MBOOT");
				break;
			case UNFD_PART_ENV:
				snprintf(PartName, PARTITION_MAXLEN, "UBILD");
				break;
			case UNFD_PART_OPTEE:
				snprintf(PartName, PARTITION_MAXLEN, "OPTEE");
				break;
			case UNFD_PART_ARMFW:
				snprintf(PartName, PARTITION_MAXLEN, "ARMFW");
				break;
			case UNFD_PART_TEEKB:
				snprintf(PartName, PARTITION_MAXLEN, "TEEKB");
				break;
			default:
				snprintf(PartName, PARTITION_MAXLEN, "UNUSED%d",
					pRecord->u16_PartType);
				break;
			}

		        if (i) {
				snprintf(p, len, ",%dk(%s)", PartSize, PartName);
				len -= strlen(p);
				p += strlen(p);
			} else { // MBOOT partition must be the first partition
				u16_PBA = pRecord->u16_StartBlk;
				while(1) {
	                        	NC_ReadPages(u16_PBA << pNandDrv->u8_BlkPageCntBits,
						pu8_DataBuf, pu8_SpareBuf, 1);
					//binary ID for chunk header
					//if( ((U32 *)pu8_DataBuf)[0x7] == 0x0000B007) {
					if (pu8_SpareBuf[1] == UBOOT_TAG) {
						u16_MbootBlk[u8_MbootCnt] = u16_PBA;
	                            		u8_MbootCnt++;
	                        	}

	                        	if (u8_MbootCnt == 2)
	                            		break;

					u16_PBA++;
	                        	if (u16_PBA == (pRecord->u16_StartBlk + pRecord->u16_BlkCnt))
	                            		break;
				}

				if (u8_MbootCnt == 1) {
					PartSize -= ((u16_MbootBlk[0] - pRecord->u16_StartBlk)
						* u16_BlkKbyteCnt);
					snprintf(p, len, "%dk@%dk(%s)", PartSize,
						u16_MbootBlk[0] * u16_BlkKbyteCnt, PartName);
					len -= strlen(p);
					p += strlen(p);
				} else if (u8_MbootCnt == 2) {
					PartSize1 = (u16_MbootBlk[1] - u16_MbootBlk[0])
						* u16_BlkKbyteCnt;
					PartSize2 = PartSize - 
						((u16_MbootBlk[1] - pRecord->u16_StartBlk)
						* u16_BlkKbyteCnt);
					snprintf(p, len, "%dk@%dk(%s),%dk(%sBAK)",
						PartSize1,
						u16_MbootBlk[0] * u16_BlkKbyteCnt, PartName,
						PartSize2, PartName);
					len -= strlen(p);
					p += strlen(p);
				} else {
					snprintf(parts, MTDPARTS_MAXLEN,
						"mtdparts=%s:-(ALL)", mtd->dev->name);
					goto out;
				}
			}
		}
		initialized = 1;
		pr_info("%s\n", ids);
		pr_info("%s\n", parts);
	}
out:
	*mtdids = ids;
	*mtdparts = parts;
}
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Edward-CH Lee <edward-ch.lee@mediatek.com>");
MODULE_DESCRIPTION("MTK DTV Nand Flash Controller Driver");
