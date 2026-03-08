// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#ifndef NAND_DRV_H_
#define NAND_DRV_H_

//=====================================================================================
#include "drvNAND_config.h" // [CAUTION]: edit drvNAND_config.h for your platform
//=====================================================================================
#include "drvNAND_err_codes.h"

#define NAND_DRIVER_VERSION	1 // used to sync with other SW stages (e.g. linux kernel)

#define MAX_U4_VALUE		0x0F
#define MAX_U8_VALUE		0xFF
#define MAX_U16_VALUE		0xFFFF
#define MAX_U32_VALUE		0xFFFFFFFF
#define LEFT_SHIFT4(x)		(x<<4)
#define RIGHT_SHIFT4(x)		(x>>4)
#define LEFT_SHIFT8(x)		(x<<8)
#define RIGHT_SHIFT8(x)		(x>>8)
#define LEFT_SHIFT16(x)		(x<<16)
#define RIGHT_SHIFT16(x)	(x>>16)
#define LEFT_SHIFT32(x)		(x<<32)
#define RIGHT_SHIFT32(x)	(x>>32)

//===========================================================
// driver parameters
//===========================================================
#define NAND_ID_BYTE_CNT	15
#define NAND_VENDOR_BYTE_CNT	16
#define NAND_PART_BYTE_CNT	16
#define NAND_TAG_BYTE_CNT	16
#define NAND_HASH_TYPE		3
#define NAND_HASH_BLK_CNT	2
#define NAND_BBT_PAGE_COUNT	8

#define R_SEQUENCE_003A		1
#define R_SEQUENCE_004A		2
#define R_SEQUENCE_004A30	3
#define R_SEQUENCE_005A30	4

#define NAND_CellType_SLC	0
#define NAND_CellType_MLC	1

#define DEFAULT_BLK_CNT			0x400
#define DEFAULT_BLK_PAGE_CNT		0x40
#define DEFAULT_PAGE_BYTE_CNT		0x800
#define DEFAULT_SPARE_BYTE_CNT		0x100
#define DEFAULT_SPARE_BYTE_CNT72	0x180

//===========================================================
// Partition Info parameters
//===========================================================
#define UNFD_LOGI_PART		0x8000 // if the partition needs Wear-Leveling
#define UNFD_HIDDEN_PART	0x4000 // if this partition is hidden, normally it is for LOGI PARTs.

//#define UNFD_PART_UNUSED1	0x01
#define UNFD_PART_ENV		0x02
//#define UNFD_PART_UNUSED3	0x03
//#define UNFD_PART_UNUSED4	0x04
//#define UNFD_PART_UNUSED5	0x05
#define UNFD_PART_MBOOT		0x06
//#define UNFD_PART_UNUSED7	0x07
//#define UNFD_PART_UNUSED8	0x08
//#define UNFD_PART_UNUSED9	0x09
//#define UNFD_PART_UNUSED10	0x0A
//#define UNFD_PART_UNUSED11	0x0B
//#define UNFD_PART_UNUSED12	0x0C
//#define UNFD_PART_UNUSED13	0x0D
//#define UNFD_PART_UNUSED14	0x0E
//#define UNFD_PART_UNUSED15	0x0F
//#define UNFD_PART_UNUSED16	0x10
//#define UNFD_PART_UNUSED17	0x11
//#define UNFD_PART_UNUSED18	0x12
//#define UNFD_PART_UNUSED19	0x13
#define UNFD_PART_OPTEE		0x14
#define UNFD_PART_ARMFW		0x15
#define UNFD_PART_TEEKB		0x16

typedef struct _PARTITION_RECORD {
	U16 u16_StartBlk;	// the start block index, reserved for UNFD internal use.
	U16 u16_BlkCnt;		// project team defined
	U16 u16_PartType;	// project team defined, e.g. UNFD_PART_XXX_0
	U16 u16_BackupBlkCnt;	// reserved good blocks count for backup, UNFD internal use.
				// e.g. u16BackupBlkCnt  = u16BlkCnt * 0.03 + 2
} __attribute__((__packed__)) PARTITION_RECORD_t, *P_PARTITION_RECORD_t;

typedef struct _PARTITION_INFO {
	U32 u32_ChkSum;
	U16 u16_SpareByteCnt;
	U16 u16_PageByteCnt;
	U16 u16_BlkPageCnt;
	U16 u16_BlkCnt;
	U16 u16_PartCnt;
	U16 u16_UnitByteCnt;
	PARTITION_RECORD_t records[];
} __attribute__((__packed__)) PARTITION_INFO_t;

typedef struct _DDR_TIMING_GROUP {
	U8 u8_ClkIdx;
	U8 u8_DqsMode;
	U8 u8_DelayCell;
	U8 u8_DdrTiming;
} __attribute__((__packed__)) DDR_TIMING_GROUP_t;

typedef struct _NAND_FLASH_INFO {
	U8 au8_Tag[NAND_TAG_BYTE_CNT];
	U8 u8_IDByteCnt;
	U8 au8_ID[NAND_ID_BYTE_CNT];
	U32 u32_ChkSum;
	U16 u16_SpareByteCnt;
	U16 u16_PageByteCnt;
	U16 u16_BlkPageCnt;
	U16 u16_BlkCnt;
	U32 u32_Config;
	U16 u16_ECCType;
	U16 u16_tRC;
	U8 u8_tRP;
	U8 u8_tREH;
	U8 u8_tREA;
	U8 u8_tRR;
	U16 u16_tADL;
	U16 u16_tRHW;
	U16 u16_tWHR;
	U16 u16_InitBadBlkCnt;
	U8 u8_Vendor[NAND_VENDOR_BYTE_CNT];
	U8 u8_PartNumber[NAND_PART_BYTE_CNT];
	U8 u8_PairPageMapLoc;
	U8 u8_PairPageMapType;
	U16 u16_tCCS;
	U8 u8_tCS;
	//for TV ROM Code
	U8 u8_BL0PBA;
	U8 u8_BL1PBA;
	U8 u8_UBOOTPBA;
	U16 u16_tWC;
	U8 u8_tWP;
	U8 u8_tWH;
	U16 u16_tCWAW;
	U8 u8_tCLHZ;
	U8 u8_AddrCycleIdx;
	DDR_TIMING_GROUP_t tDefaultDDR;
	DDR_TIMING_GROUP_t tMaxDDR;
	DDR_TIMING_GROUP_t tMinDDR;
	U8 u8_HashPBA[NAND_HASH_TYPE][NAND_HASH_BLK_CNT];
	U16 u16_tWW;
	U8 u8_ReadRetryType;
	U8 u8_BitflipThreshold;
	U8 u8_Hash0PageIdx;
	U8 u8_Hash1PageIdx;
	U32 u32_BootSize;
	U16 u16_BBtPageCheckCount;
	U16 u16_BBtPageIdx[NAND_BBT_PAGE_COUNT];
	U16 u16_BBtMarker[NAND_BBT_PAGE_COUNT];
	U16 u16_PhySpareByteCnt;
} __attribute__((__packed__)) NAND_FLASH_INFO_t;

typedef struct _PAIRED_PAGE_MAP {
    U16 u16_LSB;
    U16 u16_MSB;
} PAIRED_PAGE_MAP_t, *P_PAIRED_PAGE_MAP_t;

typedef struct _BlK_INFO {
	U8 u8_BadBlkMark;
	U8 u8_PartType;
	U16 u16_BlkAddr;
} __attribute__((__packed__)) BLK_INFO_t;

/*
BIT :[63:32]   |   [31: 13]   |  [12: 3]   |   [2:1]   |   [0]
     MIU_          Reserved      Page_         MIU_        End_
     Address       Count         SEL           Flag
*/
typedef  struct  _ADMA_DESC {
	U32 u32_EndFlag		: 1;
	U32 u32_MIUSel		: 2;
	U32 u32_PageCnt		: 10;
	U32 u32_ExtAddress	: 3;
	U32 u32_SectorCnt	: 16;
	U32 u32_Address;
} ADMA_DESC_t;

typedef struct _NAND_DRIVER {
	U8 au8_ID[NAND_ID_BYTE_CNT];
	U8 u8_IDByteCnt;
	U8 u8_CISBlk;
	//-----------------------------
	// HAL parameters
	//-----------------------------
	volatile U16 u16_Reg40_Signal;
	volatile U16 u16_Reg48_Spare;
	volatile U16 u16_Reg49_SpareSize;
	volatile U16 u16_Reg50_EccCtrl;
	volatile U16 u16_Reg57_RELatch;

	volatile U16 u16_Reg58_DDRCtrl;
	volatile U16 u16_Reg5A_tWHR_tCLHZ;
	volatile U16 u16_Reg5D_tCWAW_tADL;

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	volatile U16 u16_Reg56_Rand_W_Cmd;
	#endif

	#if (defined(FCIE_LFSR) && FCIE_LFSR) || (defined(NC_TRR_TCS) && NC_TRR_TCS)
	volatile U16 u16_Reg59_LFSRCtrl;
	#endif

	U8 u8_PadMode;
	U8 u8_HasPNI;
	U32 u32_Clk;
	//-----------------------------
	// NAND Info (listed by their init_sequence)
	//-----------------------------
	U8 u8_WordMode;
	U8 u8_OpCode_RW_AdrCycle;
	U8 u8_OpCode_Erase_AdrCycle;
	U8 u8_Flag_004A30;
	U16 u16_One_Col_Addr;

	U16 u16_BlkCnt;
	U16 u16_BlkPageCnt;
	U16 u16_PageByteCnt;
	U16 u16_SpareByteCnt;
	U16 u16_BitflipThreshold;
	U16 u16_ECCCorretableBit;
	U16 u16_ECCType;
	U32 u32_Config;
	U16 u16_tRC;
	U8 u8_tRP;
	U8 u8_tREH;
	U8 u8_tREA;
	U8 u8_tRR;
	U16 u16_tADL;
	U16 u16_tRHW;
	U16 u16_tWHR;
	U16 u16_tCCS;
	U8 u8_tCS;
	U16 u16_tWC;
	U8 u8_tWP;
	U8 u8_tWH;
	U16 u16_tCWAW;
	U8 u8_tCLHZ;
	U8  u8_AddrCycleIdx;
	U16 u16_tWW;
	U8 u8_Vendor[NAND_VENDOR_BYTE_CNT];
	U8 u8_PartNumber[NAND_PART_BYTE_CNT];

	// bit0 of u32_Config, 0:SLC, 1:MLC
	U8 u8_CellType;
	// bit3:1 of u32_Config, 0:Column0, ...
	U8 u8_BadBlkMarker;
	// bit6:4 of u32_Config, 0:P1, 1:P2, ...
	U8 u8_PlaneCnt;
	// bit8 of u32_Config, 0: disable, 1: enable
	U8 u8_RequireRandomizer;
	// bit10:9 of u32_Config, 0:SDR 1:Toggle 2: ONFI
	U8 u8_NANDInterface;
	// bit12:11 of u32_Config, 0 normal, 1 cache, 2 multi-plane, 3 multi-plane cache
	U8 u8_CacheProgram;
	// bit14:13 of u32_Config, 0 normal, 1 cache, 2 multi-plane, 3 multi-plane cache
	U8 u8_CacheRead;
	// bit15 of u32_Config, 0: no retry 1: read retry
	// bit 19:18 of u32_Config, 0:CE 0, 1:CE 0/1, 2:CE 0/1/2, 3:CE 0/1/2/3
	U8 u8_MaxCENum;
	U8 u8_PairPageMapLoc;
	U16 u16_BlkSectorCnt;
	U16 u16_PageSectorCnt;
	U16 u16_SectorByteCnt;
	U16 u16_SectorSpareByteCnt;
	U16 u16_ECCCodeByteCnt;

	U8 u8_BlkPageCntBits;
	U8 u8_BlkSectorCntBits;
	U8 u8_PageByteCntBits;
	//U8 u8_SpareByteCntBits;
	U8 u8_PageSectorCntBits;
	U8 u8_SectorByteCntBits;
	//U8 u8_SectorSpareByteCntBits;

	U16 u16_BlkPageCntMask;
	U16 u16_BlkSectorCntMask;
	U16 u16_PageByteCntMask;
	//U16 u16_SpareByteCntMask;
	U16 u16_PageSectorCntMask;
	U16 u16_SectorByteCntMask;

	U8 u8_BL0PBA;
	U8 u8_BL1PBA;
	U8 u8_UBOOTPBA;
	U8 u8_HashPBA[NAND_HASH_TYPE][NAND_HASH_BLK_CNT];
	U8 u8_Hash0PageIdx;
	U8 u8_Hash1PageIdx;
	U32 u32_BootSize;
	U16 u16_BBtPageCheckCount;
	U16 u16_BBtPageIdx[NAND_BBT_PAGE_COUNT];
	U16 u16_BBtMarker[NAND_BBT_PAGE_COUNT];
	U16 u16_PhySpareByteCnt;

	U16 u16_BlkLowPCnt;
	U8  u8_SlcMode;

	U8  u8_SwEccCorr;
	U16 u16_NandInfoECCType;

	U8 *pu8_PageSpareBuf;
	U8 *pu8_PageDataBuf;
	//-----------------------------
	// Partition Info
	//-----------------------------
	PARTITION_INFO_t *pPartInfo;

	//-----------------------------
	#if IF_IP_VERIFY
	U8 u8_IfECCTesting;
	#endif

	//-----------------------------
	struct mtk_fcie_nand_host *priv;
} NAND_DRIVER, *P_NAND_DRIVER;

/*=============================================================*/
// Global function definition
/*=============================================================*/
extern U32  drvNAND_Init(void *priv);
extern U32 drvNAND_FindMatchCIS(U8 *nniaddr, U8 *pniaddr,
	U16 *nniidx, U16 *pniidx, U16 *ppmidx);
extern U32 drvNAND_WriteCIS(U8 *nniaddr, U8 *pniaddr, U8 *ppmaddr);
extern U32  drvNAND_IPVerify_Main(void);

//===========================================================
// internal used functions
//===========================================================
#include "drvNAND_utl.h"
extern void *drvNAND_get_DrvContext_address(void);
extern void *drvNAND_get_DrvContext_PartInfo(void);

//---------------------------------------
// HAL functions
//---------------------------------------
extern U32  NC_Init(void);
extern U32  NC_RegInit(void);
extern void NC_ReConfig(void);
extern U32  NC_EnableLFSR(void);
extern U32  NC_DisableLFSR(void);
extern U32  NC_PlatformResetPre(void);
extern U32  NC_PlatformResetPost(void);
extern U32  NC_PlatformInit(void);
extern U32  NC_ConfigNandFlashContext(void);
extern U32  NC_ResetFCIE(void);
extern U32  NC_ReadPages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt);
extern U32  NC_WritePages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt);
extern U32  NC_PageCopy(U32 u32_SrcPhyRowIdx, U32 u32_DstPhyRowIdx, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_PageCnt, U32 *pu32_DonePageCnt);
extern U32  NC_ReadSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_SectorCnt);
extern U32  NC_ReadSectors_Retry(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_SectorCnt);
extern U32  NC_WriteSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_SectorCnt);
extern U32  NC_ReadID(void);
extern U32  NC_ProbeReadSeq(void);
extern U32  NC_EraseBlk(U32 u32PhyRowIdx);
extern U32  NC_ResetNandFlash(void);
extern U32  NC_Read_RandomIn(U32 u32PhyRow, U32 u32Col, U8 *pu8DataBuf,
	U32 u32DataByteCnt);
extern U32  NC_Write_RandomOut(U32 u32_PhyRow, U32 u32_Col, U8 *pu8_DataBuf,
	U32 u32_DataByteCnt);
extern U32  NC_Read_RandomIn_MIU(U32 u32PhyRow, U32 u32Col, U8 *pu8DataBuf,
	U32 u32DataByteCnt);
extern U32  NC_Write_RandomOut_MIU(U32 u32_PhyRow, U32 u32_Col, U8 *pu8_DataBuf,
	U32 u32_DataByteCnt);
extern U32  NC_GetECCBits(void);
extern U32  NC_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec);
extern U32  NC_ReadStatus(void);
extern void NC_DumpRegisters(void);
extern void NC_DumpDebugBus(void);

extern void NC_Config(void);
extern void NC_SetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt);
extern void NC_GetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt);
extern void NC_SetCIFD_Const(U8 u8_Data, U32 u32_CIFDPos, U32 u32_ByteCnt);
extern void NC_GetRBufCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt);

extern U32  NC_WritePage_RIUMode(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf);
extern U32  NC_ReadSector_RIUMode(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf);

//Read Retry
extern void NC_SetupReadRetryCmd(void);
extern U32  NC_GetRegDefaultValue(void);

extern U32  NC_ADMAReadPages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt, U8 u8_Zdec);
extern U32  NC_ADMAWritePages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt);

extern void nand_CheckPowerCut(void);
extern void nand_Prepare_Power_Saving_Mode_Queue(void);

#endif // NAND_DRV_H_

