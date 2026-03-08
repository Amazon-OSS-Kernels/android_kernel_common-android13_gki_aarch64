// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#include <common.h>
#include "drvNAND.h"

U32 drvNAND_Init(void *priv)
{
	U32 u32_Err = UNFD_ST_SUCCESS;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	memset((void *)pNandDrv, 0, sizeof(NAND_DRIVER));
	pNandDrv->priv = priv;

	NC_PlatformInit();

	pNandDrv->pPartInfo = (PARTITION_INFO_t *)drvNAND_get_DrvContext_PartInfo();
	memset((void *)pNandDrv->pPartInfo, 0, PARTINFO_BYTE_CNT);

	u32_Err = drvNAND_ProbeReadSeq();
	if(u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;

	u32_Err = drvNAND_SearchCIS();
	if(u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;

	u32_Err = nand_config_clock();
	if(u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;

	return u32_Err;
}

U32 drvNAND_FindMatchCIS(U8 *nniaddr, U8 *pniaddr, U16 *nniidx, U16 *pniidx, U16 *ppmidx)
{
 	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)drvNAND_get_DrvContext_address();
	NAND_FLASH_INFO_t *pNandInfo;
	PARTITION_INFO_t *pPartInfo;
	int i;

	i = 0;
	while (1) {
		pNandInfo = (NAND_FLASH_INFO_t *)(nniaddr + i * NANDINFO_BYTE_CNT);

		if (memcmp(pNandInfo->au8_Tag, "MSTARSEMIUNFDCIS", NAND_TAG_BYTE_CNT) != 0)
			return UNFD_ST_NO_NANDINFO;

		if (memcmp(pNandInfo->au8_ID, pNandDrv->au8_ID, pNandInfo->u8_IDByteCnt) == 0) {
			*nniidx = i;
			*ppmidx = pNandInfo->u8_PairPageMapType;
			printf("find match nandinfo @idx %d\n", i);
			if (pNandInfo->u8_PairPageMapType)
				printf("find match ppmap @idx %d\n", pNandInfo->u8_PairPageMapType);
			break;
		}

		i++;
	}
	printf("NAND ID : ");
	for(i = 0; i < pNandInfo->u8_IDByteCnt ; i++)
		printf("%02X ", pNandInfo->au8_ID[i]);
	printf("\n");

	for (i = 0; i < MAX_PARTINFO_CNT ; i++) {
		pPartInfo = (PARTITION_INFO_t *)(pniaddr + i * PARTINFO_BYTE_CNT);

		if (pPartInfo->u16_SpareByteCnt == pNandDrv->u16_SpareByteCnt &&
		    pPartInfo->u16_PageByteCnt == pNandDrv->u16_PageByteCnt &&
		    pPartInfo->u16_BlkPageCnt == pNandDrv->u16_BlkPageCnt &&
		    pPartInfo->u16_BlkCnt == pNandDrv->u16_BlkCnt) {
			*pniidx = i;
		    	pNandDrv->u8_HasPNI = 1;
			printf("find match partinfo @idx %d\n", i);
			break;
		}
	}
	if (i == MAX_PARTINFO_CNT)
		return UNFD_ST_NO_PARTINFO;

	return UNFD_ST_SUCCESS;
}

U32 drvNAND_WriteCIS(U8 *nniaddr, U8 *pniaddr, U8 *ppmaddr)
{
	U32 u32_Err = UNFD_ST_SUCCESS;
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	NAND_FLASH_INFO_t *pNandInfo = (NAND_FLASH_INFO_t *)nniaddr;
	U8 *au8_PageBuf = pNandDrv->pu8_PageDataBuf;
	U8 *au8_SpareBuf = pNandDrv->pu8_PageSpareBuf;
	U16 u16_PBA;
	U16 u16_PhyRowIdx;
	U8 u8_CisBlk = 0;

	for (u16_PBA = 0 ; u16_PBA < CIS_AREA_BLK_CNT && u8_CisBlk < CIS_BLK_CNT ; u16_PBA++) {
		/* Reset NAND driver and FCIE to the original settings */
		pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
		pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
		pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;
		NC_ConfigNandFlashContext();
		NC_RegInit();
		pNandDrv->u16_Reg48_Spare &= ~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
		NC_Config();

		u16_PhyRowIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;

		Disable_Err_log = 1;
		if (!drvNAND_IsGoodBlk(u16_PBA)) {
			printf("Skip bad block 0x%x\n", u16_PBA);
			Disable_Err_log = 0;
			continue;
		}
		Disable_Err_log = 0;

		u32_Err = NC_EraseBlk(u16_PhyRowIdx);
		if (u32_Err != UNFD_ST_SUCCESS) {
			printf("NAND erase to 0x%x failed %X\n", u16_PBA, u32_Err);
			#if 0
			u32_Err = drvNAND_MarkBadBlk(u16_PBA);
			if (u32_Err != UNFD_ST_SUCCESS) {
				printf("NAND markbad to 0x%x failed 0x%lX\n", u16_PBA, u32_Err);
				return u32_Err;
			}
			#endif
			continue;
		}

		pNandDrv->u16_PageByteCnt = DEFAULT_PAGE_BYTE_CNT;
		pNandDrv->u16_SpareByteCnt = DEFAULT_SPARE_BYTE_CNT;
		pNandDrv->u16_ECCType = pNandDrv->u16_NandInfoECCType;
		if(pNandDrv->u16_ECCType == ECC_TYPE_72BIT1KB)
			pNandDrv->u16_SpareByteCnt = DEFAULT_SPARE_BYTE_CNT72;
		NC_ConfigNandFlashContext();
		NC_RegInit();
		pNandDrv->u16_Reg48_Spare |= BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
		NC_Config();

		memset(au8_PageBuf, 0, pNandDrv->u16_SpareByteCnt);
		memcpy(au8_PageBuf, nniaddr, NANDINFO_BYTE_CNT);
		memset(au8_SpareBuf, MAX_U8_VALUE, pNandDrv->u16_SectorSpareByteCnt);

		u32_Err = NC_WriteSectors(u16_PhyRowIdx, 0, au8_PageBuf, au8_SpareBuf, 1);
		if (u32_Err != UNFD_ST_SUCCESS) {
			printf("NAND write to 0x%x failed %X\n", u16_PhyRowIdx, u32_Err);
			#if 0
			u32_Err = drvNAND_MarkBadBlk(u16_PBA);
			if (u32_Err != UNFD_ST_SUCCESS) {
				printf("NAND markbad to 0x%x failed 0x%lX\n", u16_PBA, u32_Err);
				return u32_Err;
			}
			#endif
			continue;
		}

		/* Reset NAND driver and FCIE to the original settings */
		pNandDrv->u16_SpareByteCnt = pNandInfo->u16_SpareByteCnt;
		pNandDrv->u16_PageByteCnt  = pNandInfo->u16_PageByteCnt;
		pNandDrv->u16_ECCType      = pNandInfo->u16_ECCType;
		NC_ConfigNandFlashContext();
		NC_RegInit();
		pNandDrv->u16_Reg48_Spare &= ~BIT_NC_HW_AUTO_RANDOM_CMD_DISABLE;
		NC_Config();

		if (pniaddr) {
			memset(au8_PageBuf, 0, pNandDrv->u16_SectorByteCnt);
			memcpy(au8_PageBuf, pniaddr, PARTINFO_BYTE_CNT);

			u32_Err = NC_WriteSectors(u16_PhyRowIdx+1, 0, au8_PageBuf, au8_SpareBuf, 1);
			if (u32_Err != UNFD_ST_SUCCESS) {
				printf("NAND write to 0x%x failed %X\n", u16_PhyRowIdx, u32_Err);
				#if 0
				u32_Err = drvNAND_MarkBadBlk(u16_PBA);
				if (u32_Err != UNFD_ST_SUCCESS) {
					printf("NAND markbad to 0x%x failed 0x%lX\n", u16_PBA,
						u32_Err);
					return u32_Err;
				}
				#endif
				continue;
			}
		}

		if (pNandDrv->u8_CellType == NAND_CellType_MLC) {
			if (pniaddr)
				printf("Not support MLC\n");
		}

		u8_CisBlk++;
		printf("CIS is written to blk %d\n", u16_PBA);
	}

	if (u8_CisBlk != CIS_BLK_CNT)
		return UNFD_ST_ERR_LACK_BLK;

	return UNFD_ST_SUCCESS;
}

