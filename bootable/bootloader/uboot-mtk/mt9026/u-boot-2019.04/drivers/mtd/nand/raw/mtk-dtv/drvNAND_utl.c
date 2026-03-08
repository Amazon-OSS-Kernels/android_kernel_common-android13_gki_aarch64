// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#include "drvNAND.h"
#include "drvNAND_utl.h"

typedef struct {
	const char *msg;
	U8  u8_OpCode_RW_AdrCycle;
	U8  u8_OpCode_Erase_AdrCycle;
	U16 u16_Reg48_Spare;
} drvNAND_ReadSeq;

static const drvNAND_ReadSeq seq[] = {
	//{"R_SEQUENCE_003A", ADR_C3TFS0, ADR_C2TRS0, BIT_NC_ONE_COL_ADDR},
	//{"R_SEQUENCE_004A", ADR_C4TFS0,	ADR_C3TRS0,	BIT_NC_ONE_COL_ADDR},
	{"R_SEQUENCE_004A30", ADR_C4TFS0, ADR_C2TRS0, 0},
	{"R_SEQUENCE_005A30", ADR_C5TFS0, ADR_C3TRS0, 0},
	{"R_SEQUENCE_006A30", ADR_C6TFS0, ADR_C3TRS0, 0},
	{NULL, 0, 0, 0},
};

U32 drvNAND_ProbeReadSeq(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 u8_seq_idx = 0;
	U32 u32_Err;
	drvNAND_ReadSeq const *pSeq = seq;

	pNandDrv->u16_BlkCnt = DEFAULT_BLK_CNT;
	pNandDrv->u16_BlkPageCnt = DEFAULT_BLK_PAGE_CNT;
	pNandDrv->u16_PageByteCnt = DEFAULT_PAGE_BYTE_CNT;
	pNandDrv->u16_SpareByteCnt = DEFAULT_SPARE_BYTE_CNT;
	pNandDrv->u16_ECCType = ECC_TYPE_4BIT;
	pNandDrv->u8_Flag_004A30 = 0;

	NC_ConfigNandFlashContext();
	u32_Err = NC_Init();
	if (u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;

	/* Probe read sequence */
	while (pSeq->msg) {
		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "%s\n", pSeq->msg);
		pNandDrv->u8_OpCode_RW_AdrCycle = pSeq->u8_OpCode_RW_AdrCycle;
		pNandDrv->u8_OpCode_Erase_AdrCycle = pSeq->u8_OpCode_Erase_AdrCycle;
		pNandDrv->u16_Reg48_Spare &= ~BIT_NC_ONE_COL_ADDR;
		pNandDrv->u16_Reg48_Spare |= pSeq->u16_Reg48_Spare;

		NC_Config();

		u32_Err = NC_ProbeReadSeq();
		if (u32_Err == UNFD_ST_SUCCESS) {
			// if R_SEQUENCE_004A30
			if(ADR_C4TFS0 == pNandDrv->u8_OpCode_RW_AdrCycle &&
				0 == (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR)) {
				pNandDrv->u8_Flag_004A30 = 1;
			}
			pNandDrv->u8_AddrCycleIdx = u8_seq_idx;
			return UNFD_ST_SUCCESS;
		}
		NC_ResetFCIE();
		pSeq++;
		u8_seq_idx++;
	}

	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Unknown RSEQ\n");

	return UNFD_ST_ERR_UNKNOWN_RSEQ;
}

U32 drvNAND_CheckPartInfo(U32 u32_PageIdx)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *au8_SectorBuf = (U8*)pNandDrv->pu8_PageDataBuf;
	U8 *au8_SpareBuf = (U8*)pNandDrv->pu8_PageSpareBuf;
	U32 u32_Err;
	U32 u32_ChkSum;
	U32 u32_ByteCnt;
	PARTITION_INFO_t *pPartInfo = (PARTITION_INFO_t *)au8_SectorBuf;

	u32_Err = NC_ReadSectors_Retry(u32_PageIdx, 0, au8_SectorBuf, au8_SpareBuf, 1);
	if (u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;

	u32_ChkSum = drvNAND_CheckSum(au8_SectorBuf + sizeof(u32_ChkSum),
		PARTINFO_BYTE_CNT - sizeof(u32_ChkSum));
	if (u32_ChkSum != pPartInfo->u32_ChkSum) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Skip PartInfo (ChkSum: 0x%08X != 0x%08X)\n",
			u32_ChkSum, pPartInfo->u32_ChkSum);

		return UNFD_ST_ERR_CIS_PART_ERR;
	}

	u32_ByteCnt = pPartInfo->u16_PartCnt * pPartInfo->u16_UnitByteCnt;
	memcpy(pNandDrv->pPartInfo, pPartInfo, sizeof(*pPartInfo) + u32_ByteCnt);

	//dump_part_info(pPartInfo);
	return UNFD_ST_SUCCESS;
}

void drvNAND_ParseNandInfo(NAND_FLASH_INFO_t *pNandInfo)
{
	#define BIT_CELL_TYPE_SHIFT	0
	#define BIT_CELL_TYPE_MASK	BIT0
	#define BIT_RANDOMIZER_SHIFT	8
	#define BIT_RANDOMIZER_MASK	(BIT8)
	#define BIT_MAX_CE_NUM_SHIFT	18
	#define BIT_MAX_CE_NUM_MASK	(BIT19|BIT18)
	#define BIT_SLC_MODE_SHIFT	21
	#define BIT_SLC_MODE_MASK	(BIT21)

	int i;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	pNandDrv->u16_PageByteCnt = pNandInfo->u16_PageByteCnt;
	pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
	pNandDrv->u16_BlkPageCnt = pNandInfo->u16_BlkPageCnt;
	pNandDrv->u16_BlkCnt = pNandInfo->u16_BlkCnt;
	pNandDrv->u16_ECCType = pNandInfo->u16_ECCType;
	pNandDrv->u8_IDByteCnt = pNandInfo->u8_IDByteCnt;
	pNandDrv->u32_Config = pNandInfo->u32_Config;
	pNandDrv->u16_tRC = pNandInfo->u16_tRC;
	pNandDrv->u8_tRP = pNandInfo->u8_tRP;
	pNandDrv->u8_tREH = pNandInfo->u8_tREH;
	pNandDrv->u8_tREA = pNandInfo->u8_tREA;
	pNandDrv->u8_tRR = pNandInfo->u8_tRR;
	pNandDrv->u16_tADL = pNandInfo->u16_tADL;
	pNandDrv->u16_tRHW = pNandInfo->u16_tRHW;
	pNandDrv->u16_tWHR = pNandInfo->u16_tWHR;
	pNandDrv->u16_tCCS = pNandInfo->u16_tCCS;
	pNandDrv->u8_tCS = pNandInfo->u8_tCS;
	pNandDrv->u16_tWC = pNandInfo->u16_tWC;
	pNandDrv->u8_tWP = pNandInfo->u8_tWP;
	pNandDrv->u8_tWH = pNandInfo->u8_tWH;
	pNandDrv->u16_tCWAW = pNandInfo->u16_tCWAW;
	pNandDrv->u8_tCLHZ = pNandInfo->u8_tCLHZ;
	pNandDrv->u16_tWW = pNandInfo->u16_tWW;
	pNandDrv->u8_AddrCycleIdx = pNandInfo->u8_AddrCycleIdx;

	if (pNandDrv->u8_AddrCycleIdx != 0) {
		pNandDrv->u8_OpCode_Erase_AdrCycle = 
			seq[pNandDrv->u8_AddrCycleIdx].u8_OpCode_Erase_AdrCycle;
		pNandDrv->u8_OpCode_RW_AdrCycle = 
			seq[pNandDrv->u8_AddrCycleIdx].u8_OpCode_RW_AdrCycle;
		pNandDrv->u16_One_Col_Addr = seq[pNandDrv->u8_AddrCycleIdx].u16_Reg48_Spare;
		pNandDrv->u16_Reg48_Spare &= ~BIT_NC_ONE_COL_ADDR;
		pNandDrv->u16_Reg48_Spare |= seq[pNandDrv->u8_AddrCycleIdx].u16_Reg48_Spare;
	}

	memcpy(pNandDrv->u8_Vendor, pNandInfo->u8_Vendor, NAND_VENDOR_BYTE_CNT);
	memcpy(pNandDrv->u8_PartNumber, pNandInfo->u8_PartNumber, NAND_PART_BYTE_CNT);

	pNandDrv->u8_CellType =
		(pNandDrv->u32_Config & BIT_CELL_TYPE_MASK) >> BIT_CELL_TYPE_SHIFT;
	pNandDrv->u8_RequireRandomizer =
		(pNandDrv->u32_Config & BIT_RANDOMIZER_MASK) >> BIT_RANDOMIZER_SHIFT;
	pNandDrv->u8_MaxCENum =
		(pNandDrv->u32_Config & BIT_MAX_CE_NUM_MASK) >> BIT_MAX_CE_NUM_SHIFT;
	pNandDrv->u8_SlcMode =
		(pNandDrv->u32_Config & BIT_SLC_MODE_MASK) >> BIT_SLC_MODE_SHIFT;
	if (pNandDrv->u8_SlcMode == 1)
		pNandDrv->u16_BlkPageCnt = (pNandInfo->u16_BlkPageCnt>>1);
	
	pNandDrv->u8_PairPageMapLoc = pNandInfo->u8_PairPageMapLoc;
	pNandDrv->u16_BitflipThreshold = pNandInfo->u8_BitflipThreshold;

	pNandDrv->u16_BBtPageCheckCount = pNandInfo->u16_BBtPageCheckCount;
	memcpy(pNandDrv->u16_BBtPageIdx, pNandInfo->u16_BBtPageIdx,
		NAND_BBT_PAGE_COUNT * sizeof(U16));
	memcpy(pNandDrv->u16_BBtMarker, pNandInfo->u16_BBtMarker,
		NAND_BBT_PAGE_COUNT * sizeof(U16));
	
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,"---------------------------------------------------\n");
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,"BBtPageCount= %d\n", pNandDrv->u16_BBtPageCheckCount);
	for (i = 0 ; i < NAND_BBT_PAGE_COUNT; i++)
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,"BBtPageIdx[%d] = %d, BBtMarker[%d] = %d\n",
			i, pNandDrv->u16_BBtPageIdx[i], i, pNandDrv->u16_BBtMarker[i]);

	pNandDrv->u16_PhySpareByteCnt = pNandInfo->u16_PhySpareByteCnt;
	if (pNandDrv->u16_PhySpareByteCnt == 0)
		pNandDrv->u16_PhySpareByteCnt = pNandDrv->u16_SpareByteCnt;

	pNandDrv->u8_IDByteCnt = pNandInfo->u8_IDByteCnt;
	memcpy(pNandDrv->au8_ID, pNandInfo->au8_ID, pNandInfo->u8_IDByteCnt);
}

U32 drvNAND_SearchCIS(void)
{
	#define CHECKSUM_START		0x24
	#define CHECKSUM_SIZE		(0x32 - 0x24)
	#define BLK_PAGE_CNT_BIT	6
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *au8_PageBuf = (U8*)pNandDrv->pu8_PageDataBuf;
	U8 *au8_SpareBuf = (U8*)pNandDrv->pu8_PageSpareBuf;
	NAND_FLASH_INFO_t *pNandInfo = (NAND_FLASH_INFO_t*)au8_PageBuf;
	BLK_INFO_t *pBlkInfo = (BLK_INFO_t*)au8_SpareBuf;
	U16 u16_PBA;
	U32 u32_PageIdx;
	U32 u32_Err;
	U32 u32_ChkSum;
	U32 u32_i;
	U8 u8_BlkPageCntBits = BLK_PAGE_CNT_BIT; /* 64 pages per block */
	#if defined(FCIE_LFSR) && FCIE_LFSR
	U8 u8_RetryRandomizer;
	#endif

	pNandDrv->u8_CISBlk = MAX_U8_VALUE;

	for (u16_PBA = 0; u16_PBA < DEFAULT_BLK_CNT; u16_PBA++) {
		#if defined(FCIE_LFSR) && FCIE_LFSR
		u8_RetryRandomizer = 0;
	LABEL_TRY_RANDOMIZER:
		//force disable Randomizer
		if (!u8_RetryRandomizer)
			NC_DisableLFSR();
		else
			NC_EnableLFSR();
		#endif

		pNandDrv->u16_BlkCnt = DEFAULT_BLK_CNT;
		pNandDrv->u16_BlkPageCnt = DEFAULT_BLK_PAGE_CNT;
		pNandDrv->u16_PageByteCnt = DEFAULT_PAGE_BYTE_CNT;
		pNandDrv->u16_ECCType = pNandDrv->u16_NandInfoECCType;
		if (pNandDrv->u16_ECCType == ECC_TYPE_72BIT1KB)
			pNandDrv->u16_SpareByteCnt = DEFAULT_SPARE_BYTE_CNT72;
		else
			pNandDrv->u16_SpareByteCnt = DEFAULT_SPARE_BYTE_CNT;

		pNandDrv->u16_BitflipThreshold = 0;

		NC_ConfigNandFlashContext();
		NC_RegInit();
		pNandDrv->u16_Reg48_Spare |= BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;

		NC_Config();
		u32_PageIdx = u16_PBA << u8_BlkPageCntBits;
		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 0, "32Pages: %d\n", u16_PBA);

		// ------------------------------------
		if (pNandDrv->u8_Flag_004A30) {
 			pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C4TFS0;
			pNandDrv->u8_OpCode_Erase_AdrCycle = ADR_C2TRS0;
		}
	LABEL_TRY_005A30:
		// ------------------------------------
		u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, 1);
		if (u32_Err != UNFD_ST_SUCCESS || pBlkInfo->u8_BadBlkMark != MAX_U8_VALUE) {
			if( 1 == pNandDrv->u8_Flag_004A30 &&
			    ADR_C5TFS0 != pNandDrv->u8_OpCode_RW_AdrCycle) {
				//nand_debug(0,1,"switch to 005A30, 0 \n");
				pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
				pNandDrv->u8_OpCode_Erase_AdrCycle = ADR_C3TRS0;
				goto LABEL_TRY_005A30;
			}

			#if defined(FCIE_LFSR) && FCIE_LFSR
			if (!u8_RetryRandomizer) {
				u8_RetryRandomizer = 1;
				goto LABEL_TRY_RANDOMIZER;
			}
			#endif

			continue;
		}

		if (drvNAND_CompareCISTag(pNandInfo->au8_Tag)) {
			if (1 == pNandDrv->u8_Flag_004A30 &&
			    ADR_C5TFS0!=pNandDrv->u8_OpCode_RW_AdrCycle) {
				//nand_debug(0,1,"switch to 005A30, 1 \n");
				pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
				pNandDrv->u8_OpCode_Erase_AdrCycle = ADR_C3TRS0;
				goto LABEL_TRY_005A30;
			}

			#if defined(FCIE_LFSR) && FCIE_LFSR
			if (!u8_RetryRandomizer) {
				u8_RetryRandomizer = 1;
				goto LABEL_TRY_RANDOMIZER;
			}
			#endif

			continue;
		}

		u32_ChkSum = drvNAND_CheckSum(au8_PageBuf + CHECKSUM_START, CHECKSUM_SIZE);
		if (u32_ChkSum != pNandInfo->u32_ChkSum) {
			nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1,
				"Skip Blk 0x%08x, ChkSum: 0x%08X != 0x%08X\n",
				u16_PBA, u32_ChkSum, pNandInfo->u32_ChkSum);

			if (1 == pNandDrv->u8_Flag_004A30 &&
			    ADR_C5TFS0!=pNandDrv->u8_OpCode_RW_AdrCycle) {
				//nand_debug(0,1,"switch to 005A30, 2 \n");
				pNandDrv->u8_OpCode_RW_AdrCycle = ADR_C5TFS0;
				pNandDrv->u8_OpCode_Erase_AdrCycle = ADR_C3TRS0;
				goto LABEL_TRY_005A30;
			}

			#if defined(FCIE_LFSR) && FCIE_LFSR
			if (!u8_RetryRandomizer) {
				u8_RetryRandomizer = 1;
				goto LABEL_TRY_RANDOMIZER;
			}
			#endif

			continue;
		}

		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 0, "Complete auto-checking 8/16 bits:0x%X\n",
			pNandDrv->u8_WordMode);
		dump_nand_info(pNandInfo);

		drvNAND_ParseNandInfo(pNandInfo);

		NC_ConfigNandFlashContext();
		NC_RegInit();
		#if defined(FCIE_LFSR) && FCIE_LFSR 
		if (pNandDrv->u8_RequireRandomizer) {
			nand_debug(0,1,"Enable Randomizer\n");
			NC_EnableLFSR();
		}
		#endif
		NC_Config();
		if (pNandDrv->u8_CellType == NAND_CellType_SLC) {
			for (u32_i=0;u32_i<pNandDrv->u16_BlkPageCnt;u32_i++) {
				ga_tPairedPageMap[u32_i].u16_LSB = u32_i;
				ga_tPairedPageMap[u32_i].u16_MSB = u32_i;
			}
		} else {
			nand_debug(UNFD_DEBUG_LEVEL, 1, "Paired Page Map @0x%08x\n",
				pNandDrv->u8_PairPageMapLoc);
			u32_Err = NC_ReadPages(u32_PageIdx+pNandDrv->u8_PairPageMapLoc,
				au8_PageBuf, au8_SpareBuf, 1);
			if (u32_Err != UNFD_ST_SUCCESS) {
				nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1,
					"Skip Blk 0x%08x, Read PPM fail:0x%X\n", u16_PBA, u32_Err);

				continue;
			} else {
				memcpy(&ga_tPairedPageMap[0], au8_PageBuf,
					PAIRED_PAGE_COUNT * sizeof(PAIRED_PAGE_MAP_t));
			}
		}

		if (u32_Err == UNFD_ST_SUCCESS) {
			nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "PartInfo @0x%08x\n",
				ga_tPairedPageMap[1].u16_LSB);
			pNandDrv->u8_HasPNI = 1;
			u32_Err = drvNAND_CheckPartInfo(u32_PageIdx + ga_tPairedPageMap[1].u16_LSB);
			if (u32_Err != UNFD_ST_SUCCESS) {
				pNandDrv->u8_HasPNI = 0;
				nand_debug(UNFD_DEBUG_LEVEL, 1, "No PartInfo\n");
			}
		}
		// BlkLowPCnt
		for (u32_i=0; u32_i<pNandDrv->u16_BlkPageCnt; u32_i++)
			if(ga_tPairedPageMap[u32_i].u16_LSB==pNandDrv->u16_BlkPageCnt-1 ||
			   ga_tPairedPageMap[u32_i].u16_MSB==pNandDrv->u16_BlkPageCnt-1)
				break;
		pNandDrv->u16_BlkLowPCnt = u32_i+1;

		pNandDrv->u8_CISBlk = (u16_PBA<<u8_BlkPageCntBits)>>pNandDrv->u8_BlkPageCntBits;
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "CIS is found @Blk%d\n",
			pNandDrv->u8_CISBlk);
		
		return UNFD_ST_SUCCESS;
	}

	#if defined(FCIE_LFSR) && FCIE_LFSR
	NC_DisableLFSR();
	#endif

	nand_debug(UNFD_DEBUG_LEVEL, 1, "No CIS found\n");
	return UNFD_ST_ERR_NO_CIS;
}

//--------------------------------------------------------------------------
static __inline void dump_mem_line(unsigned char *buf, int cnt)
{
	int i;

	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, " 0x%08lX: ", (unsigned long)buf);
	
	for (i= 0; i < cnt; i++)
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "%02X ", buf[i]);

	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, " | ");

	for (i = 0; i < cnt; i++) {
		nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "%c",
			(buf[i] >= ' ' && buf[i] <= '~') ? buf[i] : '.');
	}

	nand_debug(UNFD_DEBUG_LEVEL_WARNING, 0, "\n");
}

void dump_mem(unsigned char *buf, int cnt)
{
	#define DUMP_ROW_SIZE			16
	int i;

	for (i= 0; i < cnt; i+= DUMP_ROW_SIZE)
		dump_mem_line(buf + i, DUMP_ROW_SIZE);
}

void dump_nand_info(NAND_FLASH_INFO_t *pNandInfo)
{
	int i;
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "###############################################\n");
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "#        NAND INFO                            #\n");
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "###############################################\n");

	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "pNandInfo: 0x%08lX\n", (unsigned long)pNandInfo);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "au8_Tag          : [");
	for (i = 0; i < NAND_TAG_BYTE_CNT; i++)
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "%c", pNandInfo->au8_Tag[i]);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "]\n");

	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u8_IDByteCnt     : 0x%04x\n", pNandInfo->u8_IDByteCnt);

	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "au8_ID           : 0x[ ");
	for (i = 0; i < pNandInfo->u8_IDByteCnt; i++)
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "%02X ", pNandInfo->au8_ID[i]);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "]\n");

	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u32_ChkSum       : 0x%04X\n", pNandInfo->u32_ChkSum);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_SpareByteCnt : 0x%04x\n", pNandInfo->u16_SpareByteCnt);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_PageByteCnt  : 0x%04x\n", pNandInfo->u16_PageByteCnt);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_BlkPageCnt   : 0x%04x\n", pNandInfo->u16_BlkPageCnt);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_BlkCnt       : 0x%04x\n", pNandInfo->u16_BlkCnt);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u32_Config       : 0x%08X\n", pNandInfo->u32_Config);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_ECCType      : 0x%04x\n", pNandInfo->u16_ECCType);
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "u16_tRC: 0x%04x\n\n", pNandInfo->u16_tRC);

	//dump_mem_line((U8*)pNandInfo, sizeof(NAND_FLASH_INFO_t));
	nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "\n");
	
}

void dump_part_records(PARTITION_RECORD_t *records, int cnt)
{
	#define MAX_PART_CNT	1024
	int i;

	if (cnt > MAX_PART_CNT) {
		nand_debug(0, 1, "broken\n");
		return;
	}

	for (i = 0; i < cnt; i++) {
		nand_debug(0, 1, "record[%d]       : 0x%04x, 0x%04x, 0x%04x, 0x%04x\n",
			i,
			records[i].u16_StartBlk, records[i].u16_BlkCnt,
			records[i].u16_PartType, records[i].u16_BackupBlkCnt);
	}
}

void dump_part_info(PARTITION_INFO_t *pPartInfo)
{
	nand_debug(0, 1, "###############################################\n");
	nand_debug(0, 1, "#        PART INFO                            #\n");
	nand_debug(0, 1, "###############################################\n");

	nand_debug(0, 1, "pPartInfo: 0x%08lX\n", (unsigned long)pPartInfo);
	nand_debug(0, 1, "u32_ChkSum      : 0x%04X\n", pPartInfo->u32_ChkSum);
	nand_debug(0, 1, "u16_PartCnt     : 0x%04x\n", pPartInfo->u16_PartCnt);
	nand_debug(0, 1, "u16_UnitByteCnt : 0x%04x\n", pPartInfo->u16_UnitByteCnt);

	dump_part_records(pPartInfo->records, pPartInfo->u16_PartCnt);
}

void dump_nand_driver(NAND_DRIVER *pNandDrv)
{
	U16 u16_i;
	
	nand_debug(0, 1, "NAND_Info:\n");

	nand_debug(0, 1, "  ID [%u bytes]           : ", pNandDrv->u8_IDByteCnt);
	for(u16_i=0; u16_i<pNandDrv->u8_IDByteCnt; u16_i++)
		nand_debug(0, 0, "%02Xh ", pNandDrv->au8_ID[u16_i]);
	nand_debug(0, 0, "\n");
    
	nand_debug(0, 1, "  BlkCnt                 : 0x%X\n", pNandDrv->u16_BlkCnt);
	nand_debug(0, 1, "  BlkPageCnt             : 0x%X\n", pNandDrv->u16_BlkPageCnt);
	nand_debug(0, 1, "  BlkSectorCnt           : 0x%X\n", pNandDrv->u16_BlkSectorCnt);
	nand_debug(0, 1, "  PageByteCnt            : 0x%X\n", pNandDrv->u16_PageByteCnt);
	nand_debug(0, 1, "  SpareByteCnt           : 0x%X\n\n", pNandDrv->u16_SpareByteCnt);

	nand_debug(0, 1, "  BlkPageCntBits         : 0x%X\n", pNandDrv->u8_BlkPageCntBits);
	nand_debug(0, 1, "  BlkSectorCntBits       : 0x%X\n", pNandDrv->u8_BlkSectorCntBits);
	nand_debug(0, 1, "  PageByteCntBits        : 0x%X\n", pNandDrv->u8_PageByteCntBits);

	nand_debug(0, 1, "  BlkPageCntMask         : 0x%X\n", pNandDrv->u16_BlkPageCntMask);
	nand_debug(0, 1, "  BlkSectorCntMask       : 0x%X\n", pNandDrv->u16_BlkSectorCntMask);
	nand_debug(0, 1, "  PageByteCntMask        : 0x%X\n", pNandDrv->u16_PageByteCntMask);

	nand_debug(0, 1, "  PageSectorCnt          : 0x%X\n", pNandDrv->u16_PageSectorCnt);
	nand_debug(0, 1, "  SectorByteCnt          : 0x%X\n", pNandDrv->u16_SectorByteCnt);
	nand_debug(0, 1, "  SectorSpareByteCnt     : 0x%X\n", pNandDrv->u16_SectorSpareByteCnt);
	nand_debug(0, 1, "  ECCCodeByteCnt         : 0x%X\n\n", pNandDrv->u16_ECCCodeByteCnt);

	nand_debug(0, 1, "  PageSectorCntBits      : 0x%X\n", pNandDrv->u8_PageSectorCntBits);
	nand_debug(0, 1, "  SectorByteCntBits      : 0x%X\n", pNandDrv->u8_SectorByteCntBits);
	nand_debug(0, 1, "  PageSectorCntMask      : 0x%X\n", pNandDrv->u16_PageSectorCntMask);
	nand_debug(0, 1, "  SectorByteCntMask      : 0x%X\n\n", pNandDrv->u16_SectorByteCntMask);

	nand_debug(0, 1, "  CellType               : %s\n", pNandDrv->u8_CellType==NAND_CellType_MLC?"MLC":"SLC");
	nand_debug(0, 1, "  RequireRandomizer      : %s\n", pNandDrv->u8_RequireRandomizer?"Yes":"No");
	nand_debug(0, 1, "  MaxCENum               : 0x%X\n", pNandDrv->u8_MaxCENum);
    
	nand_debug(0, 1, "  OpCode_Erase_AdrCycle  : 0x%X\n", pNandDrv->u8_OpCode_Erase_AdrCycle);
	nand_debug(0, 1, "  OpCode_RW_AdrCycle     : 0x%X\n", pNandDrv->u8_OpCode_RW_AdrCycle);
	nand_debug(0, 1, "  AddrCycleIdx           : 0x%X\n\n", pNandDrv->u8_AddrCycleIdx);

	nand_debug(0, 1, "  u16_tRC;    : 0x%X\n", pNandDrv->u16_tRC);
	nand_debug(0, 1, "  u8_tRP      : 0x%X\n", pNandDrv->u8_tRP);
	nand_debug(0, 1, "  u8_tREH     : 0x%X\n", pNandDrv->u8_tREH);
	nand_debug(0, 1, "  u8_tREA     : 0x%X\n", pNandDrv->u8_tREA);
	nand_debug(0, 1, "  u8_tRR      : 0x%X\n", pNandDrv->u8_tRR);
	nand_debug(0, 1, "  u16_tADL    : 0x%X\n", pNandDrv->u16_tADL);
	nand_debug(0, 1, "  u16_tRHW    : 0x%X\n", pNandDrv->u16_tRHW);
	nand_debug(0, 1, "  u16_tWHR    : 0x%X\n", pNandDrv->u16_tWHR);
	nand_debug(0, 1, "  u16_tCCS    : 0x%X\n", pNandDrv->u16_tCCS);
	nand_debug(0, 1, "  u8_tCS      : 0x%X\n", pNandDrv->u8_tCS);
	nand_debug(0, 1, "  u16_tWC     : 0x%X\n", pNandDrv->u16_tWC);
	nand_debug(0, 1, "  u8_tWP      : 0x%X\n", pNandDrv->u8_tWP);
	nand_debug(0, 1, "  u8_tWH      : 0x%X\n", pNandDrv->u8_tWH);
	nand_debug(0, 1, "  u16_tCWAW   : 0x%X\n", pNandDrv->u16_tCWAW);
	nand_debug(0, 1, "  u8_tCLHZ    : 0x%X\n", pNandDrv->u8_tCLHZ);
	nand_debug(0, 1, "  u16_tWW     : 0x%X\n", pNandDrv->u16_tWW);

	nand_debug(0, 1, "  BBtPageCheckCount : 0x%X\n", pNandDrv->u16_BBtPageCheckCount);
	for (u16_i=0; u16_i<pNandDrv->u16_BBtPageCheckCount; u16_i++) {
		nand_debug(0, 1, "  Page:%02Xh  Mark:%02Xh \n", 
			pNandDrv->u16_BBtPageIdx[pNandDrv->u16_BBtPageCheckCount], 
			pNandDrv->u16_BBtMarker[pNandDrv->u16_BBtPageCheckCount]);
	}
	nand_debug(0, 1, "  PhySpareByteCnt           : 0x%X\n\n", pNandDrv->u16_PhySpareByteCnt);		
}

void dump_nand_PPM(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_i;
    
	nand_debug(0,1,"\n");

	if (pNandDrv->u8_CellType == NAND_CellType_SLC) {
		nand_debug(0,1," SLC has no PPM \n");
		return;
	}
    
	nand_debug(0,1,"    LP      HP\n");
	for (u16_i=0; u16_i<pNandDrv->u16_BlkPageCnt; u16_i++) {
		nand_debug(0,1,"  %4u    %4u\n", 
			ga_tPairedPageMap[u16_i].u16_LSB, ga_tPairedPageMap[u16_i].u16_MSB);

		if (ga_tPairedPageMap[u16_i].u16_LSB == pNandDrv->u16_BlkPageCnt-1 ||
		    ga_tPairedPageMap[u16_i].u16_MSB == pNandDrv->u16_BlkPageCnt-1)
			break;
	}

	nand_debug(0,1,"\n");
}

U8 drvNAND_CountBits(U32 u32_x)
{
	U8 u8_i = 0;

	if (u32_x==0) return u8_i;
    
	while (u32_x) {
		u8_i++;
		u32_x >>= 1;
	}

	return u8_i-1;
}

U32 drvNAND_CheckSum(U8 *pu8_Data, U32 u32_ByteCnt)
{
	U32 u32_Sum = 0;

	while (u32_ByteCnt--)
		u32_Sum += *pu8_Data++;

	return u32_Sum;
}

/* return 0: same, 1: different */
U32 drvNAND_CompareCISTag(U8 *tag)
{
	const char *str = "MSTARSEMIUNFDCIS";
	int i = 0;
	
	for (i = 0; i < NAND_TAG_BYTE_CNT; i++) {
		if (tag[i] != str[i])
			return 1;
	}

	return 0;
}

/* Search for partition of type @u16_PartType, begin from @pRecord */
PARTITION_RECORD_t *drvNAND_SearchPartition(PARTITION_RECORD_t *pRecord,  U16 u16_PartType)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	PARTITION_INFO_t *pPartInfo = pNandDrv->pPartInfo;

	while (pRecord - pPartInfo->records < pPartInfo->u16_PartCnt) {
		if (pRecord->u16_PartType == u16_PartType)
			return pRecord;

		pRecord++;
	}

	return (void*)0;
}

/* return 1: Good block, 0: Bad block */
U32 drvNAND_CheckSpareSpacePattern(U8 *au8_SpareBuf, U32 u32_PageIdx)
{
	U16 u16_PageIdxInBlk;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	u16_PageIdxInBlk = u32_PageIdx & pNandDrv->u16_BlkPageCntMask;
	if (au8_SpareBuf[pNandDrv->u16_BBtMarker[u16_PageIdxInBlk]] != MAX_U8_VALUE)
		return 0;
	return 1;
}

/* return 1: Good block, 0: Bad block */
U32 drvNAND_IsGoodBlk(U16 u16_PBA)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *au8_DataBuf = (U8*)pNandDrv->pu8_PageDataBuf;
	U8 *au8_SpareBuf = (U8*)pNandDrv->pu8_PageSpareBuf;
	U32 u32_PageIdx;
	U32 u32_Err;
	U32 i;
	BLK_INFO_t *pBlkInfo = (BLK_INFO_t *)au8_SpareBuf;

	// check by nni
	if (pNandDrv->u16_BBtPageCheckCount > 0 &&
	    pNandDrv->u16_BBtPageCheckCount <= NAND_BBT_PAGE_COUNT) {
		for (i=0; i<pNandDrv->u16_BBtPageCheckCount; i++) {
			u32_PageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits)
				+ pNandDrv->u16_BBtPageIdx[i];
			u32_Err = NC_ReadPages(u32_PageIdx, au8_DataBuf, au8_SpareBuf, 1);
			if (u32_Err != UNFD_ST_SUCCESS) {
				nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1,
					"NC_ReadPage(Row:0x%X), Err:0x%X \n",
					(unsigned int)u32_PageIdx, (unsigned int)u32_Err);
			}
			if (!drvNAND_CheckSpareSpacePattern(au8_SpareBuf, u32_PageIdx))
				return 0;
		}
		return 1;
	}


	u32_PageIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;
	u32_Err = NC_ReadSectors_Retry(u32_PageIdx, 0, au8_DataBuf, au8_SpareBuf, 1);
	if (u32_Err != UNFD_ST_SUCCESS)
		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "NC_ReadSectors(0x%X)=0x%X\n",
			(unsigned int)u32_PageIdx, (unsigned int)u32_Err);

	if (pBlkInfo->u8_BadBlkMark != MAX_U8_VALUE)
		return 0; // bad blk

	return 1; // good blk
}

U32 drvNAND_MarkBadBlk(U16 u16_PBA)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *au8_PageBuf = (U8*)pNandDrv->pu8_PageDataBuf;
	U8 *au8_SpareBuf = (U8*)pNandDrv->pu8_PageSpareBuf;
	U32 u32_Err;
	U32 u32_PageIdx;
	U8  u8_LFSR=0;

	u32_Err = NC_EraseBlk (u16_PBA << pNandDrv->u8_BlkPageCntBits);

	//memset(au8_PageBuf, '\0', pNandDrv->u16_PageByteCnt);
	memset(au8_SpareBuf, '\0', pNandDrv->u16_SpareByteCnt);

	if (IF_LFSR_ENABLED())
	{u8_LFSR = 1;  NC_DisableLFSR();}
                    
	/* Clear the 1st page (main + spare) of the block */
	u32_PageIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;
	u32_Err = NC_WritePages(u32_PageIdx, au8_PageBuf, au8_SpareBuf, 1);
	if (u32_Err != UNFD_ST_SUCCESS)
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Mark Blk 0x%04x failed with ErrCode: 0x%08x\n",
		(unsigned int)u16_PBA, (unsigned int)u32_Err);
	/* Clear the last page (main + spare) of the block */
	u32_PageIdx += pNandDrv->u16_BlkPageCnt - 1;
	u32_Err = NC_WritePages(u32_PageIdx, au8_PageBuf, au8_SpareBuf, 1);
	if (u32_Err != UNFD_ST_SUCCESS) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, 
			"Mark Blk 0x%04x failed with ErrCode: 0x%08x\n",
			(unsigned int)u16_PBA, (unsigned int)u32_Err);
	}

	if (u8_LFSR)
		NC_EnableLFSR();

	return u32_Err;
}

U32 drvNAND_ParseAddrToADMA(U8 * pu8_DMADataBuf,  U32 u32_ByteCnt, U8 u8_Direction,
	ADMA_DESC_t* ADMADesc, U8 u8_IsEnd)
{
	#define BIT_MIUSEL_MASK		3
	#define BIT_PAGECNT_MASK	0x3FF
	#define BIT_SECTORCNT_MASK	0x3FF

	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (u32_ByteCnt & pNandDrv->u16_PageByteCntMask)
		return UNFD_ST_ERR_INVALID_PARAM;

	ADMADesc->u32_EndFlag = (u8_IsEnd & 1);
	ADMADesc->u32_MIUSel =
		(nand_translate_DMA_MIUSel((unsigned long)pu8_DMADataBuf, u32_ByteCnt)
		& BIT_MIUSEL_MASK);
	ADMADesc->u32_PageCnt =
		cpu_to_le32((u32_ByteCnt >> pNandDrv->u8_PageByteCntBits) & BIT_PAGECNT_MASK);
	ADMADesc->u32_SectorCnt = cpu_to_le32(((u32_ByteCnt >> pNandDrv->u8_PageByteCntBits)
		& BIT_SECTORCNT_MASK)<<pNandDrv->u8_PageSectorCntBits);
	ADMADesc->u32_ExtAddress = 0;
	ADMADesc->u32_Address =
		cpu_to_le32(nand_translate_DMA_address_Ex((unsigned long)pu8_DMADataBuf,
		u32_ByteCnt, u8_Direction));

	//nand_debug(0, 1, "Main Desc:");
	//dump_mem(ADMADesc, sizeof(ADMA_DESC_t_));
	//nand_debug(0, 0, "\n");
	return UNFD_ST_SUCCESS;
}

