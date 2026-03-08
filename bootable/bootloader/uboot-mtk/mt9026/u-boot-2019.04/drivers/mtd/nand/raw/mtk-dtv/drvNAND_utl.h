// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#ifndef __DRVNAND_UTL_H__
#define __DRVNAND_UTL_H__

#include "drvNAND.h"

#define PAIRED_PAGE_COUNT  0x200
extern PAIRED_PAGE_MAP_t ga_tPairedPageMap[];

extern U32 drvNAND_ProbeReadSeq(void);
extern U32 drvNAND_CheckPartInfo(U32 u32_PageIdx);
extern U32 drvNAND_SearchCIS(void);
extern void drvNAND_ParseNandInfo(NAND_FLASH_INFO_t *pNandInfo);
extern void dump_mem(unsigned char *buf, int cnt);
extern void dump_nand_info(NAND_FLASH_INFO_t *pNandInfo);
extern void dump_part_records(PARTITION_RECORD_t *pRecord, int cnt);
extern void dump_part_info(PARTITION_INFO_t *pPartInfo);
extern void dump_nand_driver(NAND_DRIVER *pNandDrv);
extern void dump_nand_PPM(void);

extern U32 drvNAND_CompareCISTag(U8 *tag);
extern U8  drvNAND_CountBits(U32 u32_x);
extern U32 drvNAND_CheckSum(U8 *pu8_Data, U32 u32_ByteCnt);

extern PARTITION_RECORD_t *drvNAND_SearchPartition(PARTITION_RECORD_t *pRecord,
	U16 u16_PartType);

extern U32 drvNAND_CheckSpareSpacePattern(U8 *au8_SpareBuf, U32 u32_PageIdx);
extern U32 drvNAND_IsGoodBlk(U16 u16_BlkIdx);
extern U32 drvNAND_MarkBadBlk(U16 u16_BlkIdx);
extern U32 drvNAND_ParseAddrToADMA(U8 * pu8_DMADataBuf,  U32 u32_ByteCnt,
	U8 u8_Direction, ADMA_DESC_t* ADMADesc, U8 u8_IsEnd);

#endif /* __DRVNAND_UTL_H__ */
