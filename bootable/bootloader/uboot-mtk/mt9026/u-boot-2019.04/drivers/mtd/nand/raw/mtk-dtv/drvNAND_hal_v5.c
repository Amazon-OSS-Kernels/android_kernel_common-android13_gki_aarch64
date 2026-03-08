// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

//#include <linux/string.h>
#include "drvNAND.h"
#include "drvNAND_utl.h"

#define CHECK_HAL_PARAM		1 // normal can be 0

#define CIFD_BYTE_CNT		64
#define CIFD_BYTE_SHIFT		6
#define ECC_LOC_CNT		8
#define ECC_LOC_CNT_SHIFT	3

//========================================================
// HAL pre-processors
//========================================================
#if IF_FCIE_SHARE_PINS
#define NC_PAD_SWITCH(enable)	nand_pads_switch(enable);
#else
#define NC_PAD_SWITCH(enable)
#endif

#if IF_FCIE_SHARE_IP
// re-config FCIE3 for NFIE mode
#define NC_RECONFIG()		{NC_ReConfig(); nand_clock_setting(pNandDrv->u32_Clk);}
#else
// NULL to save CPU a JMP/RET time
#define NC_RECONFIG()
#endif

#define NAND_TIMEOUT_RETRY_CNT	5

#if defined(NEED_FLUSH_MIU_PIPE) && NEED_FLUSH_MIU_PIPE
#define FLUSH_MIU_PIPE()	nand_flush_miu_pipe()
#else
#define FLUSH_MIU_PIPE()
#endif

//========================================================
// HAL misc. function definitions
//========================================================
void NC_DumpRegisters(void);
void NC_DumpDebugBus(void);
U32 NC_CheckEWStatus(U8 u8_OpType);
U32 NC_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec);
U32 NC_WaitCIFD_Event(U16 u16_WaitEvent, U32 u32_MicroSec);

U32 NC_WaitGetCIFD(U8 *pu8_DataBuf, U32 u32_ByteCnt, U32 u32_MicroSec);
U32 NC_WaitSetCIFD(U8 *pu8_DataBuf, U32 u32_ByteCnt);

#define OPTYPE_ERASE	1
#define OPTYPE_WRITE	2

int Disable_Err_log = 0;

void NC_DumpDebugBus(void)
{
	#define BIT_FCIE_DEBUG_DBUS_SHIFT	8
	#define FCIE_DEBUG_DBUS_CNT		16
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16_Reg, u16_i;

	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "\n");
	for (u16_i=0; u16_i<FCIE_DEBUG_DBUS_CNT; u16_i++) {
		REG_CLR_BITS_UINT16(NC_DEBUG_DBUS1, BIT_FCIE_DEBUG_MODE);
		REG_SET_BITS_UINT16(NC_DEBUG_DBUS1, u16_i << BIT_FCIE_DEBUG_DBUS_SHIFT);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "Reg debug mode: 0x%X\n", u16_i);
		REG_READ_UINT16(NC_DEBUG_DBUS0, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_DEBUG_DBUS0: %X\n", u16_Reg);
		REG_READ_UINT16(NC_DEBUG_DBUS1, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_DEBUG_DBUS1: %X\n", u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	}

	REG_CLR_BITS_UINT16(NC_DEBUG_DBUS1, BIT_FCIE_DEBUG_MODE);
	REG_SET_BITS_UINT16(NC_DEBUG_DBUS1, BIT0 << BIT_FCIE_DEBUG_DBUS_SHIFT);

	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	for (u16_i=0; u16_i<FCIE_DEBUG_DBUS_CNT; u16_i++) {
		REG_CLR_BITS_UINT16(NC_WIDTH, BIT_NC_DEB_SEL);
		REG_SET_BITS_UINT16(NC_WIDTH, u16_i<<BIT_NC_DEB_SEL_SHIFT);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "REG_NC_DEB_SEL: %X\n", u16_i);

		REG_READ_UINT16(NC_WIDTH, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_WIDTH: %X\n", u16_Reg);

		REG_READ_UINT16(NC_DEBUG_DBUS0, u16_Reg);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_DEBUG_DBUS0: %X\n", u16_Reg);

		REG_READ_UINT16(NC_DEBUG_DBUS1, u16_Reg);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "NC_DEBUG_DBUS1: %X\n", u16_Reg);

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "\n");
	}
	REG_CLR_BITS_UINT16(NC_WIDTH, BIT_NC_DEB_SEL);
}

void NC_DumpRegisters(void)
{
	#define DUMP_REG_START	0x30
	#define DUMP_REG_STOP	0x80
	#define DUMP_ROW_SIZE	8
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16_Reg;
	U16 u16_idx = 0;

	nand_DumpPadClk();
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Dump FCIE Register\n");

	REG_READ_UINT16(NC_RPT_CNT, u16_Reg);
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_RPT_CNT(0x0C) = %04X\n", u16_Reg);

	REG_READ_UINT16(NC_TR_RPT_CNT, u16_Reg);
	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_TR_RPT_CNT(0x0C) = %04X\n", u16_Reg);

	nand_debug(UNFD_DEBUG_LEVEL,0,"FCIE Reg: ");
	for (u16_idx=DUMP_REG_START ; u16_idx<DUMP_REG_STOP; u16_idx++) {
		if (0 == u16_idx%DUMP_ROW_SIZE)
			nand_debug(UNFD_DEBUG_LEVEL,0, "\n%02Xh:| ", u16_idx);

		REG_READ_UINT16(GET_REG_ADDR(FCIE_REG_BASE_ADDR, u16_idx), u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL, 0, "%04Xh ", u16_Reg);

		if ((DUMP_ROW_SIZE-1) == u16_idx%DUMP_ROW_SIZE)
			nand_debug(UNFD_DEBUG_LEVEL,0,"| %02Xh ", u16_idx);
	}
	nand_debug(UNFD_DEBUG_LEVEL, 0,"\n");

}

//========================================================
// HAL function definitions
//========================================================
U32 NC_ResetFCIE(void)
{
	#define RETRY_CNT	0x1000
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16Reg, u16Cnt;

	REG_WRITE_UINT16(NC_RESERVED_FOR_SW, NAND_DRIVER_VERSION);
	//nand_debug(1, 1, "1: %Xh\n", REG(NC_RESERVED_FOR_SW));

	NC_PlatformResetPre();

	// soft reset
	REG_CLR_BITS_UINT16(NC_FCIE_RST, BIT_FCIE_SOFT_RST); /* active low */

	//As reset is active, Check Reset Status from 0 -> 1
	u16Cnt=0;
	do {
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	  	if (RETRY_CNT == u16Cnt++) {
			nand_DumpPadClk();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"ERROR: NC_ResetFCIE Err1, ErrCode: 0x%03X\n", UNFD_ST_ERR_NO_NFIE);
			return UNFD_ST_ERR_NO_NFIE;
		}

		REG_READ_UINT16(NC_FCIE_RST, u16Reg);

	} while (BIT_RST_STS_MASK != (u16Reg & BIT_RST_STS_MASK));

	//nand_hw_timer_delay(HW_TIMER_DELAY_1ms);
	REG_SET_BITS_UINT16(NC_FCIE_RST, BIT_FCIE_SOFT_RST);
	//nand_hw_timer_delay(HW_TIMER_DELAY_1ms);

	//Restore reset bit, check reset status from 1 -> 0
	u16Cnt=0;
	do {
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	  	if (RETRY_CNT == u16Cnt++) {
			nand_DumpPadClk();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"ERROR: NC_ResetFCIE Err2, ErrCode: 0x%03X\n", UNFD_ST_ERR_NO_NFIE);
			return UNFD_ST_ERR_NO_NFIE;
		}

		REG_READ_UINT16(NC_FCIE_RST, u16Reg);

	} while (0 != (u16Reg & BIT_RST_STS_MASK));

	NC_PlatformResetPost();

	//nand_debug(1, 1, "2: %Xh\n", REG(NC_RESERVED_FOR_SW));

	return UNFD_ST_SUCCESS; // ok
}

U32 NC_ConfigNandFlashContext(void)
{
	#define SECTOR_BYTE_CNT_512	0x200
	#define SECTOR_BYTE_CNT_1K	0x400
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (pNandDrv->u16_ECCType < ECC_TYPE_24BIT1KB ||
		pNandDrv->u16_ECCType == ECC_TYPE_32BIT ||
		pNandDrv->u16_ECCType == ECC_TYPE_40BIT ||
		pNandDrv->u16_ECCType == ECC_TYPE_60BIT ||
		pNandDrv->u16_ECCType == ECC_TYPE_72BIT)
		pNandDrv->u16_SectorByteCnt = SECTOR_BYTE_CNT_512;
	else
		pNandDrv->u16_SectorByteCnt = SECTOR_BYTE_CNT_1K;

	pNandDrv->u8_BlkPageCntBits = drvNAND_CountBits(pNandDrv->u16_BlkPageCnt);
	pNandDrv->u8_PageByteCntBits = drvNAND_CountBits(pNandDrv->u16_PageByteCnt);
	//pNandDrv->u8_SpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SpareByteCnt);
	pNandDrv->u8_SectorByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorByteCnt);

	pNandDrv->u16_BlkPageCntMask = (1<<pNandDrv->u8_BlkPageCntBits)-1;
	pNandDrv->u16_PageByteCntMask = (1<<pNandDrv->u8_PageByteCntBits)-1;
	pNandDrv->u16_SectorByteCntMask = (1<<pNandDrv->u8_SectorByteCntBits)-1;
	//pNandDrv->u16_SpareByteCntMask = (1<<pNandDrv->u8_SpareByteCntBits)-1;

	pNandDrv->u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> pNandDrv->u8_SectorByteCntBits;
	pNandDrv->u8_PageSectorCntBits = drvNAND_CountBits(pNandDrv->u16_PageSectorCnt);
	pNandDrv->u16_PageSectorCntMask = (1<<pNandDrv->u8_PageSectorCntBits)-1;
	pNandDrv->u16_SectorSpareByteCnt =
		pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_SectorSpareByteCnt &= ~1;
	pNandDrv->u16_SpareByteCnt
		= pNandDrv->u16_SectorSpareByteCnt << pNandDrv->u8_PageSectorCntBits;
	//pNandDrv->u8_SectorSpareByteCntBits = drvNAND_CountBits(pNandDrv->u16_SectorSpareByteCnt);

	pNandDrv->u8_BlkSectorCntBits =
		pNandDrv->u8_BlkPageCntBits + pNandDrv->u8_PageSectorCntBits;
	pNandDrv->u16_BlkSectorCnt = 1<<pNandDrv->u8_BlkSectorCntBits;
	pNandDrv->u16_BlkSectorCntMask = pNandDrv->u16_BlkSectorCnt-1;

	return UNFD_ST_SUCCESS;
}

U32 NC_Init(void)
{
	U32 u32_RetVal;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	// disable NC
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_NC_EN);
	REG_WRITE_UINT16(NC_CTRL , 0);
	// reset NC
	u32_RetVal = NC_ResetFCIE();
	if (UNFD_ST_SUCCESS != u32_RetVal) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_Init, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_NO_NFIE);
		return u32_RetVal;
	}

	// disable interupts
	//REG_CLR_BITS_UINT16(NC_MIE_INT_EN, BIT_MMA_DATA_END | BIT_NC_JOB_END);
	// clean int events
	REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	// enable NC
	REG_WRITE_UINT16(NC_FUN_CTL, BIT_NC_EN);

	REG_WRITE_UINT16(NC_SIGNAL, pNandDrv->u16_Reg40_Signal);
	NC_RegInit();

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	REG_READ_UINT16(NC_RAND_W_CMD, pNandDrv->u16_Reg56_Rand_W_Cmd);
	#endif

	#if defined(NC_TRR_TCS) && NC_TRR_TCS
	REG_READ_UINT16(NC_LFSR_CTRL, pNandDrv->u16_Reg59_LFSRCtrl);
	#endif

	pNandDrv->u8_PadMode =NAND_PAD_BYPASS_MODE;

	// ===================================================
	NC_Config();
	nand_debug(UNFD_DEBUG_LEVEL_LOW,1, "NC_ResetNandFlash\n");
	u32_RetVal = NC_ResetNandFlash();
	if (UNFD_ST_SUCCESS != u32_RetVal) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"ERROR, NAND reset flash, Err Code:%08Xh \n",
			u32_RetVal);
		return u32_RetVal;
	}
	nand_debug(UNFD_DEBUG_LEVEL_HIGH,1, "NC_ReadID\n");

	u32_RetVal = NC_ReadID();
	if (u32_RetVal != UNFD_ST_SUCCESS) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Failed to read ID with EC: 0x%08X\n",
			u32_RetVal);
		u32_RetVal = UNFD_ST_ERR_UNKNOWN_ID;
		return u32_RetVal;
	}

	NC_Config();
	return UNFD_ST_SUCCESS;
}

U32 NC_EnableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16_RegVal;

	if (0 == IF_LFSR_ENABLED()) {
		// tune timing: RE_LATCH
		if (0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM)) {
			REG_READ_UINT16(NC_LATCH_DATA, u16_RegVal);
			u16_RegVal = u16_RegVal & BIT_NC_LATCH_DATA_MASK;
			if(u16_RegVal > BIT_NC_LATCH_DATA_6_0_T) {
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
					"Error, RD_LATCH already > 1T, can NOT use LFSR.\n");
				return UNFD_ST_ERR_LFSR_RD_LATCH;
			}
			pNandDrv->u16_Reg57_RELatch &= ~BIT_NC_LATCH_DATA_MASK;
			pNandDrv->u16_Reg57_RELatch += u16_RegVal + BIT_NC_LATCH_DATA_1_0_T;
		}

		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// set LFST
		pNandDrv->u16_Reg59_LFSRCtrl |= BIT_LFSR_ENABLE;
		REG_SET_BITS_UINT16(NC_LFSR_CTRL, BIT_LFSR_ENABLE);
	}

	return UNFD_ST_SUCCESS;
}

U32 NC_DisableLFSR(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (IF_LFSR_ENABLED()) {
		if (0 == (pNandDrv->u16_Reg58_DDRCtrl & BIT_DDR_MASM))
			pNandDrv->u16_Reg57_RELatch -= BIT_NC_LATCH_DATA_1_0_T;

		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

		// clear LFST
		pNandDrv->u16_Reg59_LFSRCtrl &= ~BIT_LFSR_ENABLE;
		REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_LFSR_ENABLE);
	}
	return UNFD_ST_SUCCESS;
}

U32 NC_RegInit(void)
{
	#define SPARE_SIZE_ALIGN	0x3
	#define PAGE_SIZE_512		0x200
	#define PAGE_SIZE_2KB		0x800
	#define PAGE_SIZE_4KB		0x1000
	#define PAGE_SIZE_8KB		0x2000
	#define PAGE_SIZE_16KB		0x4000
	#define PAGE_SIZE_32KB		0x8000
	#define ECC_CORRECTABLE_8BIT	8
	#define ECC_CORRECTABLE_16BIT	16
	#define ECC_CORRECTABLE_24BIT	24
	#define ECC_CORRECTABLE_32BIT	32
	#define ECC_CORRECTABLE_40BIT	40
	#define ECC_CORRECTABLE_60BIT	60
	#define ECC_CORRECTABLE_72BIT	72
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	// config NC
	pNandDrv->u16_Reg48_Spare = (pNandDrv->u16_SpareByteCnt >> pNandDrv->u8_PageSectorCntBits);
	if (NC_MAX_SECTOR_SPARE_BYTE_CNT < pNandDrv->u16_Reg48_Spare ||
		(pNandDrv->u16_Reg48_Spare & SPARE_SIZE_ALIGN)) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: invalid Sector Spare Size: %Xh bytes!\r\n",
			pNandDrv->u16_Reg48_Spare);
		nand_die();
	}

	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	pNandDrv->u16_Reg48_Spare |= BIT_SPARE_ADR_AUTO_INC;
	#endif

	pNandDrv->u16_Reg49_SpareSize = pNandDrv->u16_SpareByteCnt;
	if (NC_MAX_TOTAL_SPARE_BYTE_CNT < pNandDrv->u16_Reg49_SpareSize
		|| (pNandDrv->u16_Reg49_SpareSize & SPARE_SIZE_ALIGN)) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: invalid Total Spare Size: %Xh bytes!\r\n",
			pNandDrv->u16_Reg49_SpareSize);
		nand_die();
	}

	pNandDrv->u16_Reg50_EccCtrl = REG50_ECC_CTRL_INIT_VALUE;
	if (pNandDrv->u8_WordMode)
		pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_WORD_MODE;
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1 ,"UNFD, WordMode:%X\r\n", pNandDrv->u8_WordMode);

	switch (pNandDrv->u16_PageByteCnt) {
		case PAGE_SIZE_512:
			pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_PAGE_SIZE_512Bn;
			pNandDrv->u16_Reg48_Spare |= BIT_NC_ONE_COL_ADDR;
			break;
		case PAGE_SIZE_2KB:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_2KB;  break;
		case PAGE_SIZE_4KB:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_4KB;  break;
		case PAGE_SIZE_8KB:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_8KB;  break;
		case PAGE_SIZE_16KB:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_16KB;  break;
		case PAGE_SIZE_32KB:  pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_PAGE_SIZE_32KB;  break;
		default:
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"ERROR: invalid Page Size: %Xh bytes!\r\n",
				pNandDrv->u16_PageByteCnt);
			nand_die();
	}

	switch (pNandDrv->u16_ECCType) {
		case ECC_TYPE_4BIT:
			//FCIE5 not support 4bit ecc -> using 8 bit ecc instead. for NNI compatible.
		case ECC_TYPE_8BIT:
			pNandDrv->u16_Reg50_EccCtrl &= ~BIT_NC_ECC_TYPE_8b512Bn;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_8BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_8BIT;
			break;
		case ECC_TYPE_16BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_16b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_16BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_16BIT;
			break;
		case ECC_TYPE_24BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_24BIT;
			break;
		case ECC_TYPE_32BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_32b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_32BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_32BIT;
			break;
		case ECC_TYPE_40BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_40b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_40BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_40BIT;
			break;
		case ECC_TYPE_60BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_60b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_60BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_60BIT;
			break;
		case ECC_TYPE_72BIT:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_72b512B;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_72BIT;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_72BIT;
			break;
		case ECC_TYPE_8BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_8b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_8BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_8BIT;
			break;
		case ECC_TYPE_16BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_16b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_16BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_16BIT;
			break;
		case ECC_TYPE_24BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_24b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_24BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_24BIT;
			break;
		case ECC_TYPE_32BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_32b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_32BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_32BIT;
			break;
		case ECC_TYPE_40BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_40b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_40BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_40BIT;
			break;
		case ECC_TYPE_60BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_60b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_60BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_60BIT;
			break;
		case ECC_TYPE_72BIT1KB:
			pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECC_TYPE_72b1KB;
			pNandDrv->u16_ECCCodeByteCnt = ECC_CODE_BYTECNT_72BIT1KB;
			pNandDrv->u16_ECCCorretableBit = ECC_CORRECTABLE_72BIT;
			break;
		default:
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"ERROR: invalid ECC Type: %Xh \r\n", pNandDrv->u16_ECCType);
			nand_die();
	}

	if (!pNandDrv->u16_BitflipThreshold)
		pNandDrv->u16_BitflipThreshold = pNandDrv->u16_ECCCorretableBit>>1;

	pNandDrv->u16_Reg50_EccCtrl |= BIT_NC_ECCERR_NSTOP;
	return UNFD_ST_SUCCESS;
}

void NC_Config(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	REG_WRITE_UINT16(NC_SIGNAL, pNandDrv->u16_Reg40_Signal);
	/*sector spare size*/
	REG_WRITE_UINT16(NC_SPARE, pNandDrv->u16_Reg48_Spare);
	/* page spare size*/
	REG_WRITE_UINT16(NC_SPARE_SIZE, (U16)pNandDrv->u16_Reg49_SpareSize);
	/* page size and ECC type*/
	REG_WRITE_UINT16(NC_ECC_CTRL, pNandDrv->u16_Reg50_EccCtrl);

	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);

	#if defined(NC_TWHR_TCLHZ) && NC_TWHR_TCLHZ
	/*set timing for tCLHZ*/
	REG_WRITE_UINT16(NC_NAND_TIMING, pNandDrv->u16_Reg5A_tWHR_tCLHZ);
	#endif

	#if defined(NC_TCWAW_TADL) && NC_TCWAW_TADL
	/*set timing for tADL and tCWAW*/
	REG_WRITE_UINT16(NC_NAND_TIMING1,pNandDrv->u16_Reg5D_tCWAW_tADL);
	#endif

	#if (defined(FCIE_LFSR) && FCIE_LFSR) || (defined(NC_TRR_TCS) && NC_TRR_TCS)
	REG_WRITE_UINT16(NC_LFSR_CTRL, pNandDrv->u16_Reg59_LFSRCtrl);
	#endif

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	REG_WRITE_UINT16(NC_RAND_W_CMD, pNandDrv->u16_Reg56_Rand_W_Cmd);
	#endif
}

void NC_ReConfig(void) // re-config FCIE3 for NFIE mode
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	// enable interrupts if system allows
	//nand_enable_intr_mode();
	#else
	// disable interupts
	REG_CLR_BITS_UINT16(NC_MIE_INT_EN,  BIT_NC_JOB_END);
	#endif
	// clean int events
	REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_FUN_CTL, BIT_NC_EN);

	NC_ResetFCIE();
	NC_Config();
}

U32 NC_SelectCE(U32 u32_PhyRowIdx)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_ChipRowIdx;
	U32 u32_ChipPageCnt;
	U16 u16_CESel;

	//nand_debug(UNFD_DEBUG_LEVEL, 1, "Logical row %lXh", u32_PhyRowIdx);

	u16_CESel = 0;
	u32_ChipRowIdx = u32_PhyRowIdx;
	u32_ChipPageCnt =
		(pNandDrv->u16_BlkCnt/(pNandDrv->u8_MaxCENum+1))<<pNandDrv->u8_BlkPageCntBits;

	while (u32_ChipRowIdx >= u32_ChipPageCnt) {
		u16_CESel++;
		u32_ChipRowIdx -= u32_ChipPageCnt;
	}

	REG_CLR_BITS_UINT16(NC_SIGNAL,  BIT_NC_CE_SEL_MASK);
	REG_SET_BITS_UINT16(NC_SIGNAL,  u16_CESel);

	//nand_debug(UNFD_DEBUG_LEVEL, 0, " -> CE%d row %lXh\n", u16_CESel, u32_ChipRowIdx);

	return u32_ChipRowIdx;
}

// can not cross block
U32 NC_WritePages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_PageCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr=0;
	U32 u32_Ret;

	#if CHECK_HAL_PARAM
	// can not cross block
	if ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) + u32_PageCnt
		> pNandDrv->u16_BlkPageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_WritePages, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_HAL_W_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_W_INVALID_PARAM;
	}
	#endif
	//nand_debug(0,0,"w b:%lXh p:%lXh cnt:%lXh\n", u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
	//    u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask, u32_PageCnt);
	//nand_debug(0,0,"T: %X %X\n", FCIE_REG41_VAL, pNandDrv->u16_Reg57_RELatch);

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE(); // to turn on ONFI clk

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
	  	((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
	  	& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt * u32_PageCnt, WRITE_TO_NAND);

	if (pu8_SpareBuf ==NULL) {
		memset(pNandDrv->pu8_PageSpareBuf, MAX_U8_VALUE, pNandDrv->u16_SpareByteCnt);
		SpareDMAAddr =
		nand_translate_Spare_DMA_address_Ex((unsigned long)pNandDrv->pu8_PageSpareBuf,
		pNandDrv->u16_SpareByteCnt, WRITE_TO_NAND);
	}
	else
		SpareDMAAddr =
			nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
			pNandDrv->u16_SpareByteCnt, WRITE_TO_NAND);

	if ((REG(NC_FUN_CTL) & BIT_BIST_MODE) == BIT_BIST_MODE) { //for bist mode test
		SpareDMAAddr = 0;
		DataDMAAddr = 0;
	}

	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT,
		LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | CMD_0x80);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70) | ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_REPEAT) | ACT_CHKSTATUS);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);
	//nand_debug(1, 1, "W Rpt Cnt: %Xh \r\n", u16_PageCnt-1);

	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif
	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME*u32_PageCnt)
		== WAIT_WRITE_TIME*u32_PageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_WritePages Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	#if defined(CONFIG_NAND_SECURITY_GEN) && CONFIG_NAND_SECURITY_GEN
	if ((REG(NC_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT) {
		nand_debug(0, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		NC_ReConfig();
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_SECURE_ALERT);
		NC_CLR_DDR_MODE();	
		return UNFD_ST_ERR_W_PROTECTED;
	}
	#endif

	u32_Ret = NC_CheckEWStatus(OPTYPE_WRITE);
	if (UNFD_ST_SUCCESS != u32_Ret)
        	nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"write blk:%Xh p:%Xh fail:%Xh \n",
            		u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
            		u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask, u32_Ret);

	NC_CLR_DDR_MODE();
	return u32_Ret;
}

void NC_SlowdownClk(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	nand_clock_setting(FCIE_SW_SLOWEST_CLK);
	REG_WRITE_UINT16(NC_WIDTH, 0);
	REG_WRITE_UINT16(NC_LATCH_DATA, REG57_ECO_FIX_INIT_VALUE);
}

void NC_RestoreClk(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	nand_clock_setting(pNandDrv->u32_Clk);
	REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
}

// do the ReadPages operation, retry for timeout error.
static U32 NC_ReadPages_Ex(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr = 0;
	U16 u16_SpareByteCnt;
	U8  u8_RetryCnt_Events=0;

	if(pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

nand_redo_read:
	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	// select seeds, while NC_EnableLFSR/NC_DisableLFSR is called outside.
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
		((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
		& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt * u32_PageCnt, READ_FROM_NAND );

	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	u16_SpareByteCnt = pNandDrv->u16_SpareByteCnt * u32_PageCnt;
	#else
	u16_SpareByteCnt = pNandDrv->u16_SpareByteCnt;
	#endif
	SpareDMAAddr = nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
		u16_SpareByteCnt, READ_FROM_NAND);

	if ((REG(NC_FUN_CTL) & BIT_BIST_MODE) == BIT_BIST_MODE){ // for bist mode test
		SpareDMAAddr = 0;
		DataDMAAddr = 0;
	}

	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));

	if (pNandDrv->u16_Reg48_Spare& BIT_NC_ONE_COL_ADDR) { // if a page 512B
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_REPEAT);
	} else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_REPEAT) | ACT_SER_DIN);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	}
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);

	REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, (u32_PageCnt<<pNandDrv->u8_PageSectorCntBits)-1);

	// check & set - patch for MIU busy case: 
	// slow down FCIE clock to wait for MIU service.
	if (0!=u8_RetryCnt_Events)
		NC_SlowdownClk();

	REG_WRITE_UINT16(NC_CTRL,  BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME * u32_PageCnt) ==
		(WAIT_READ_TIME * u32_PageCnt )) {
		#if (0==IF_IP_VERIFY)
		NC_ReConfig();
		NC_ResetNandFlash();
		if (++u8_RetryCnt_Events < NAND_TIMEOUT_RETRY_CNT) {
            		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Warn: NC_ReadPages Timeout, ErrCode:%Xh \r\n",
				UNFD_ST_ERR_R_TIMEOUT);
			goto nand_redo_read;
		} else {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: reset & retry over %d times, stop!\n",
				NAND_TIMEOUT_RETRY_CNT);

			// restore the normal setting before return
			NC_RestoreClk();
			NC_CLR_DDR_MODE();
			nand_die(); // fatal error
			return UNFD_ST_ERR_R_TIMEOUT;
		}
		#else
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: NC_ReadPages Timeout, ErrCode:%Xh \r\n",
				UNFD_ST_ERR_R_TIMEOUT);
		//nand_stop();
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT;
		#endif		
	}

	// restore - patch for MIU busy case
	// for retry ok, restore the normal setting
	if (0!=u8_RetryCnt_Events) {
		NC_RestoreClk();
		//nand_debug(UNFD_DEBUG_LEVEL,1,"restore clk \n");
	}

    return UNFD_ST_SUCCESS;
}

U32 NC_ReadPages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf, U32 u32_PageCnt)
{
	U16 u16_Tmp = 0, u16_Cnt, u16_ByteLoc, u16_i, u16_j, u16_k;
	U16 i = 0, k = 0;
	U32 u32_Err=UNFD_ST_SUCCESS;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8  u8_RetryCnt_ECCFail = 0;
	U8  u8_MaxReadRetryCount = 1; // 1 time for slow clk
	U32 u32_UECCRow, u32_UECCRowTmp, u32_UECCPageCnt;
 	U8  *pu8_UECCDataBuf, *pu8_UECCSpareBuf;

	#if CHECK_HAL_PARAM
	// can not cross block
	if ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) + u32_PageCnt >
		pNandDrv->u16_BlkPageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_ReadPages, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_HAL_R_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_R_INVALID_PARAM;
	}
	#endif

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();

	//nand_debug(UNFD_DEBUG_LEVEL, 1, "Blk: %lXh Page: %lXh Cnt: %lXh \r\n",
	//u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask,
	//u32_PageCnt);
	if (pu8_SpareBuf == NULL)
		pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	if (!pNandDrv->u8_SwEccCorr) {

		u32_Err = NC_ReadPages_Ex(u32_PhyRowIdx, pu8_DataBuf, pu8_SpareBuf, u32_PageCnt);
		if (u32_Err == UNFD_ST_ERR_R_TIMEOUT)
			goto EXIT;

		// =====================================
		// Check ECC
		if (0 == (REG(NC_ECC_STAT0) & BIT_NC_ECC_FAIL))
			goto EXIT;
		else {
			//nand_debug(UNFD_DEBUG_LEVEL,1,"\n====================================\n");
			u32_UECCRowTmp = MAX_U32_VALUE; // to toggle LFSR
		}

		// debug
		if (pNandDrv->u8_RequireRandomizer==0 && IF_LFSR_ENABLED()) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"NAND Err: no need LFSR but enabled !!! \n\r");
			NC_DumpRegisters();
			dump_nand_driver(pNandDrv);
			nand_die();
		}
		u32_UECCRow = u32_PhyRowIdx;
		u32_UECCPageCnt = u32_PageCnt;
		pu8_UECCDataBuf = pu8_DataBuf;
		pu8_UECCSpareBuf = pu8_SpareBuf;

		while (REG(NC_ECC_STAT0) & BIT_NC_ECC_FAIL) {
			#if (0 == IF_IP_VERIFY)
			//----------------------------------
			// 0. get the error address
			u16_Tmp = REG(NC_1ST_UECC_PAGE)&BIT_1ST_UECC_PAGE_MASK;
			u32_UECCRow += u16_Tmp;

			nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,
				"--------\nNAND Warn: UECC @ Blk: %Xh, Page: %Xh, PageCnt: %Xh  \n",
			   	u32_UECCRow>>pNandDrv->u8_BlkPageCntBits,
			   	u32_UECCRow&pNandDrv->u16_BlkPageCntMask, u32_UECCPageCnt);
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "LFSR Require: %Xh, En: %Xh \n",
				   	pNandDrv->u8_RequireRandomizer, IF_LFSR_ENABLED()?1:0);
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "R51:%04Xh R52:%04Xh R53:%04Xh \n",
					REG(NC_ECC_STAT0), REG(NC_ECC_STAT1), REG(NC_ECC_STAT2));

			u32_UECCPageCnt -= u16_Tmp;
			pu8_UECCDataBuf += u16_Tmp << pNandDrv->u8_PageByteCntBits;
			#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
			pu8_UECCSpareBuf += u16_Tmp * pNandDrv->u16_SpareByteCnt;
			#endif

			//----------------------------------
			// 1. check empty pages
			NC_ReadPages_Ex(u32_UECCRow, pu8_UECCDataBuf, pu8_UECCSpareBuf, 1);

			// FCIE5 check ZERO_COUNT register which records max zero count of a sector
			if (pNandDrv->u8_RequireRandomizer && IF_LFSR_ENABLED())
				REG_READ_UINT16(NC_NAND_ZERO_BITCNT,u16_Tmp);
			else
				REG_READ_UINT16(NC_FCIE_ZERO_BITCNT,u16_Tmp);

			if (u16_Tmp <= pNandDrv->u16_ECCCorretableBit) {
			// when meet 1 empty page, all rest of pages in the same block are empty.
				memset(pu8_UECCDataBuf, MAX_U8_VALUE,
					pNandDrv->u16_PageByteCnt*u32_UECCPageCnt);
				#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
				memset(pu8_UECCSpareBuf, MAX_U8_VALUE,
					pNandDrv->u16_SpareByteCnt*u32_UECCPageCnt);
				#else
				memset(pu8_UECCSpareBuf, MAX_U8_VALUE, pNandDrv->u16_SpareByteCnt);
				#endif

				// No need to reset read retry status
				// Clean False Alarm Status
				NC_ReConfig();
				NC_CLR_DDR_MODE();
				nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "NAND: Ok, Bit Flip-Flop \n");
				goto EXIT;
			}

			//----------------------------------
			// 2. try slow clk
			if (0 == u8_RetryCnt_ECCFail) {
				nand_debug(UNFD_DEBUG_LEVEL, 1,
					"NAND: SlowClk, Retry times: %Xh, Max: %Xh \n", 
					u8_RetryCnt_ECCFail+1, u8_MaxReadRetryCount);           

				NC_SlowdownClk();
				NC_ReadPages_Ex(u32_UECCRow, pu8_UECCDataBuf, pu8_UECCSpareBuf,
					u32_UECCPageCnt);
				NC_RestoreClk();
				u8_RetryCnt_ECCFail++;
			}
			//----------------------------------
			// 3. read-retry

			//----------------------------------
			// 4. toggle en/disable LFSR
			else {
				if (pNandDrv->u8_RequireRandomizer && u32_UECCRowTmp!=u32_UECCRow) {
					u32_UECCRowTmp = u32_UECCRow;

					u8_RetryCnt_ECCFail = 1; // no need to slow clk again
					nand_debug(UNFD_DEBUG_LEVEL, 1, "NAND: switch LFSR: ");

					if(IF_LFSR_ENABLED()) {
						NC_DisableLFSR();
						nand_debug(UNFD_DEBUG_LEVEL, 1, "OFF \n");
					} else {
						NC_EnableLFSR();
						nand_debug(UNFD_DEBUG_LEVEL, 1, "ON \n");
					}

					NC_ReadPages_Ex(u32_UECCRow, pu8_UECCDataBuf,
						pu8_UECCSpareBuf, u32_UECCPageCnt);
				} else {
		                	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
						"NAND Error: NC_ReadPages ECC Fail \n");
					nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
						"Blk: %Xh  Page: %X \n\n",
						u32_UECCRow>>pNandDrv->u8_BlkPageCntBits,
						u32_UECCRow&pNandDrv->u16_BlkPageCntMask);

					NC_DumpRegisters();
					dump_nand_driver(pNandDrv);

					NC_CLR_DDR_MODE();
					u32_Err = UNFD_ST_ERR_ECC_FAIL;
					goto EXIT;
				}
			}
			#else // IF_IP_VERIFY==1
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ECC Fail \r\n");
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Blk: %Xh  Page: %X \r\n",
				u32_UECCRow>>pNandDrv->u8_BlkPageCntBits,
				u32_UECCRow&pNandDrv->u16_BlkPageCntMask);

			NC_CLR_DDR_MODE();
			if (0==pNandDrv->u8_IfECCTesting)
				u32_Err = UNFD_ST_ERR_ECC_FAIL;

			goto EXIT;
			#endif // IF_IP_VERIFY
		}
	}
	else {
		while (i < u32_PageCnt) {
			u32_UECCRow = u32_PhyRowIdx + i;
			pu8_UECCDataBuf = pu8_DataBuf + i * + pNandDrv->u16_PageByteCnt;
			#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
			pu8_UECCSpareBuf = pu8_SpareBuf + i * pNandDrv->u16_SpareByteCnt;
			#else
			pu8_UECCSpareBuf = pu8_SpareBuf;
			#endif

			u32_Err = NC_ReadPages_Ex(u32_UECCRow, pu8_UECCDataBuf,
				pu8_UECCSpareBuf, 1);
			if (u32_Err == UNFD_ST_ERR_R_TIMEOUT)
				goto EXIT;

			u16_Tmp = REG(NC_ECC_STAT0);
			if ((u16_Tmp & BIT_NC_ECC_FAIL)) {
				#if (0 == IF_IP_VERIFY)
				if (pNandDrv->u8_RequireRandomizer && IF_LFSR_ENABLED())
					REG_READ_UINT16(NC_NAND_ZERO_BITCNT,u16_Tmp);
				else
					REG_READ_UINT16(NC_FCIE_ZERO_BITCNT,u16_Tmp);

				if (u16_Tmp <= pNandDrv->u16_ECCCorretableBit) {
					memset(pu8_UECCDataBuf, MAX_U8_VALUE,
						pNandDrv->u16_PageByteCnt*u32_UECCPageCnt);
				#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
					memset(pu8_UECCSpareBuf, MAX_U8_VALUE,
						pNandDrv->u16_SpareByteCnt);
				#else
					memset(pu8_UECCSpareBuf, MAX_U8_VALUE,
						pNandDrv->u16_SpareByteCnt);
				#endif

					// No need to reset read retry status
					// Clean False Alarm Status
					NC_ReConfig();
					NC_CLR_DDR_MODE();
					nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,
						"NAND: Ok, Bit Flip-Flop \n");
				}
				//----------------------------------
				// 2. try slow clk
				else if (0 == u8_RetryCnt_ECCFail) {
					nand_debug(UNFD_DEBUG_LEVEL, 1,
						"NAND: SlowClk, Retry times: %Xh, Max: %Xh \n",
						u8_RetryCnt_ECCFail+1, u8_MaxReadRetryCount);

					NC_SlowdownClk();
					u8_RetryCnt_ECCFail++;
					continue;
				}
				//----------------------------------
				// 3. read-retry

				//----------------------------------
				// 4. toggle en/disable LFSR
				else {
					NC_RestoreClk();
					if (pNandDrv->u8_RequireRandomizer &&
						u8_RetryCnt_ECCFail == 1) {
						nand_debug(UNFD_DEBUG_LEVEL, 1,
							"NAND: switch LFSR: ");

						if(IF_LFSR_ENABLED()) {
							NC_DisableLFSR();
							nand_debug(UNFD_DEBUG_LEVEL, 1, "OFF \n");
						} else {
							NC_EnableLFSR();
							nand_debug(UNFD_DEBUG_LEVEL, 1, "ON \n");
						}
						u8_RetryCnt_ECCFail++;
						continue;
					} else {
			                	nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
							"NAND Error: NC_ReadPages ECC Fail \n");
						nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
							"Blk: %Xh  Page: %X \n\n",
							u32_UECCRow>>pNandDrv->u8_BlkPageCntBits,
							u32_UECCRow&pNandDrv->u16_BlkPageCntMask);

						NC_DumpRegisters();
						dump_nand_driver(pNandDrv);

						NC_CLR_DDR_MODE();
						u32_Err = UNFD_ST_ERR_ECC_FAIL;
						goto EXIT;
					}
				}
				#else // IF_IP_VERIFY==1
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ECC Fail \r\n");
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Blk: %Xh  Page: %X \r\n",
					u32_UECCRow>>pNandDrv->u8_BlkPageCntBits,
					u32_UECCRow&pNandDrv->u16_BlkPageCntMask);

				NC_CLR_DDR_MODE();
				if (0==pNandDrv->u8_IfECCTesting)
					u32_Err = UNFD_ST_ERR_ECC_FAIL;

				goto EXIT;
				#endif // IF_IP_VERIFY
			}
			else if ((u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK)) {
				for (k = 0 ; k < pNandDrv->u16_PageSectorCnt ; k++) {
					u32_Err = NC_ReadSectors_Retry(u32_UECCRow, k,
					pu8_UECCDataBuf + k * pNandDrv->u16_SectorByteCnt,
					pu8_UECCSpareBuf + k * pNandDrv->u16_SectorSpareByteCnt, 1);
					if (u32_Err)
						goto EXIT;
				}
			}

			u8_RetryCnt_ECCFail = 0;
			i++;
		}
	}

	// =====================================
	// ending jobs
	EXIT:
	if (u8_RetryCnt_ECCFail) {
		if(UNFD_ST_SUCCESS == u32_Err)
			nand_debug(UNFD_DEBUG_LEVEL,1,"NAND: retry ok. \n");
	}

	NC_CLR_DDR_MODE(); // for ONFI, stop clk to save power.

	#if defined(CONFIG_MIPS32)
	Chip_Read_Memory();
	#endif

	nand_read_dma_post_flush((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt * u32_PageCnt);
	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	nand_read_dma_post_flush((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SpareByteCnt * u32_PageCnt);
	#else
	nand_read_dma_post_flush((unsigned long)pu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
	#endif

	if (pNandDrv->u8_RequireRandomizer)
		NC_EnableLFSR();

	return u32_Err; 
}

// ====================================
// since UECC would not stop, so don't need internal-retry
// ====================================
U32 NC_PageCopy(U32 u32_SrcPhyRowIdx, U32 u32_DstPhyRowIdx, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_PageCnt, U32 *pu32_DonePageCnt)
{
	#define IDLE_CNT	3
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_Tmp = 0;
	dma_addr_t DataDMAAddr, SpareDMAAddr;
	U32 u32_Ret = 0, u32_i;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE(); // to turn on ONFI clk

	if(pNandDrv->u8_MaxCENum)
		return UNFD_ST_ERR_INVALID_PARAM;

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	// select seeds, while NC_EnableLFSR/NC_DisableLFSR is called outside.
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
		((u32_SrcPhyRowIdx & pNandDrv->u16_BlkPageCntMask) &
		BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	REG_CLR_BITS_UINT16(NC_SPARE, BIT_SPARE_ADR_AUTO_INC);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt,
		BIDIRECTIONAL );
	SpareDMAAddr = nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SpareByteCnt, BIDIRECTIONAL );
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_IDLE_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, IDLE_CNT);

	for (u32_i=0; u32_i<u32_PageCnt; u32_i++) {
		REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_SrcPhyRowIdx & MAX_U16_VALUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_SrcPhyRowIdx));
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_DstPhyRowIdx & MAX_U16_VALUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_DstPhyRowIdx));
		REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		//REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_PAGECOPY_US) | ACT_WAIT_IDLE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAIT_IDLE) | ACT_WAIT_IDLE);
		REG_WRITE_UINT16(NC_AUXREG_DAT,
			LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | CMD_0x00);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAIT_MMA) | ACT_SER_DIN);
		u16_Tmp = (pNandDrv->u8_OpCode_RW_AdrCycle &~ OP_ADR_SET_0) | OP_ADR_SET_1;
		//REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_PAGECOPY_DS) | ACT_WAIT_IDLE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x80) | ACT_WAIT_IDLE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DOUT) | u16_Tmp);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x10);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_CHKSTATUS) | CMD_0x70);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_BREAK);
		//REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_BREAK);
		// ----------------------------------------------------
		//[CAUTION]: data buffer will be increased (and used beyond the a PageByteCnt)
		//REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
		//REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);
		// ----------------------------------------------------
		REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, (1<<pNandDrv->u8_PageSectorCntBits)-1);

		#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
 		nand_CheckPowerCut();
		#endif

		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

		u32_SrcPhyRowIdx++;
		u32_DstPhyRowIdx++;
		if (NC_WaitComplete(BIT_NC_JOB_END, (WAIT_WRITE_TIME + WAIT_READ_TIME)) ==
			((WAIT_WRITE_TIME + WAIT_READ_TIME))) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, 
				"NAND Err: NC_PageCopy TO, ErrCode:%Xh \n", UNFD_ST_ERR_R_TIMEOUT);
			#if 0==IF_IP_VERIFY
			NC_ReConfig();
			NC_ResetNandFlash();
			#else
			//nand_stop();
			#endif

			u32_Ret = UNFD_ST_ERR_R_TIMEOUT;
			goto LABEL_END;
		}

		/* Check ECC */
		REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
		if (u16_Tmp & BIT_NC_ECC_FAIL) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NAND Err: ECC Fail, Reg51: 0x%04X ",
				u16_Tmp);
			REG_READ_UINT16(NC_ECC_STAT1, u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "Reg52: 0x%04X ", u16_Tmp);
			REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reg53: 0x%04X\n", u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Blk: %Xh  Page: %Xh\r\n",
			    u32_SrcPhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
			    u32_SrcPhyRowIdx&pNandDrv->u16_BlkPageCntMask);

			#if IF_IP_VERIFY
			if (pNandDrv->u8_IfECCTesting)
				u32_Ret = UNFD_ST_SUCCESS;
			else
			#endif
			u32_Ret = UNFD_ST_ERR_ECC_FAIL;
			goto LABEL_END;
		}

		u32_Ret=NC_CheckEWStatus(OPTYPE_WRITE);
		if (UNFD_ST_SUCCESS != u32_Ret) {
		    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "write fail::%Xh @ blk:%Xh p:%Xh \n",
		    	u32_Ret, u32_DstPhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
		        u32_DstPhyRowIdx&pNandDrv->u16_BlkPageCntMask);
		    goto LABEL_END;
		}
	}

	// --------------------------
	LABEL_END:
	#if defined(SPARE_DMA_ADDR_AUTO_INC) && SPARE_DMA_ADDR_AUTO_INC
	REG_SET_BITS_UINT16(NC_SPARE, BIT_SPARE_ADR_AUTO_INC);
	#endif
	NC_CLR_DDR_MODE(); // to turn off ONFI clk
	*pu32_DonePageCnt = u32_i;
	return u32_Ret;
}

/*--------------------------------------------------
  get ECC corrected bits count

  return: 0xFFFFFFFF -> ECC uncorrectable error,
  other: max ECC corrected bits (within the readed sectors).
  --------------------------------------------------*/
U32 NC_GetECCBits(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_Tmp;

	REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
	if (u16_Tmp & BIT_NC_ECC_FAIL)
		return 0;

	return ((u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK) >> 1);
}

U32 NC_ReadSectors_Retry(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf, U32 u32_SectorCnt)
{
	U16 u16_Tmp, u16_Cnt, u16_ByteLoc, u16_i, u16_j, u16_k;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr;
	U32 u32_Err=UNFD_ST_SUCCESS;
	 // 1 time for slow clk
	U8  u8_RetryCnt_Events=0, u8_RetryCnt_ECCFail=0, u8_MaxReadRetryCount = 1;
	U16 u16_ToggleLFSR = 0;

	#if CHECK_HAL_PARAM
	if (u8_SectorInPage+u32_SectorCnt > pNandDrv->u16_PageSectorCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: ErrCode:%Xh. Row:%Xh SectorInPage:%Xh SectorCnt:%Xh  \n",
			UNFD_ST_ERR_HAL_R_INVALID_PARAM, u32_PhyRowIdx, u8_SectorInPage,
			u32_SectorCnt);
		return UNFD_ST_ERR_HAL_R_INVALID_PARAM;
	}
	#endif

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL, ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) &
		BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_SectorByteCnt*u32_SectorCnt, READ_FROM_NAND );

	if (pu8_SpareBuf == NULL)
		SpareDMAAddr =
		nand_translate_Spare_DMA_address_Ex((unsigned long)pNandDrv->pu8_PageSpareBuf,
		pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt, READ_FROM_NAND );
	else
		SpareDMAAddr =
			nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
			pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt, READ_FROM_NAND );

	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	// if not 512B/page, set Partial Mode
	REG_READ_UINT16(NC_ECC_CTRL, u16_Tmp);
	if (u16_Tmp & BIT_NC_PAGE_SIZE_MASK) {
		REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, (u32_SectorCnt-1)<<BIT_START_SECTOR_CNT_SHIFT);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT);
	}
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

LABEL_READ_SECTOR:
	NC_SET_DDR_MODE(); // to turn on ONFI clk
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if(0 == pNandDrv->u8_WordMode)
		REG_WRITE_UINT16(NC_AUXREG_DAT,
			u8_SectorInPage << pNandDrv->u8_SectorByteCntBits);
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT,
			u8_SectorInPage << (pNandDrv->u8_SectorByteCntBits-1));
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));
	if (pNandDrv->u16_Reg48_Spare& BIT_NC_ONE_COL_ADDR) { // if a page 512B
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_BREAK);
	} else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_SER_DIN);
	}

	REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, u32_SectorCnt-1);

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == (WAIT_READ_TIME)) {
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_ResetNandFlash();
		if (++u8_RetryCnt_Events < NAND_TIMEOUT_RETRY_CNT) {
			NC_SlowdownClk();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Warn: Timeout, ErrCode:%Xh \n",
				UNFD_ST_ERR_R_TIMEOUT);
			goto LABEL_READ_SECTOR;
		} else {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: reset & retry over %d times, stop!\n",
				NAND_TIMEOUT_RETRY_CNT);
			// restore the normal setting before return
			NC_RestoreClk();
			NC_CLR_DDR_MODE();
			nand_die(); // fatal error
		}
		#else
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Timeout, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_R_TIMEOUT);
		//nand_stop();
		NC_CLR_DDR_MODE();
		#endif		

		REG_WRITE_UINT16(NC_PART_MODE, 0);
		u32_Err = UNFD_ST_ERR_R_TIMEOUT;
		goto LABEL_EXIT;
		//return UNFD_ST_ERR_R_TIMEOUT;
	}

	if (u8_RetryCnt_Events || u8_RetryCnt_ECCFail==1)
		NC_RestoreClk();

	// =====================================
	// Check ECC
	REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
	if (u16_Tmp & BIT_NC_ECC_FAIL) {
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,
			"--------\nNAND Warn: UECC @ Blk: %Xh, Page: %Xh \n",
		   	u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
		   	u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask);
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "LFSR Require: %Xh, En: %Xh \n",
			pNandDrv->u8_RequireRandomizer, IF_LFSR_ENABLED()?1:0);
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "R51:%04Xh R52:%04Xh R53:%04Xh \n", 
			u16_Tmp, REG(NC_ECC_STAT1), REG(NC_ECC_STAT2));

		#if !(defined(IF_IP_VERIFY) && IF_IP_VERIFY)
		//----------------------------------
		// 1. check empty pages
		if (pNandDrv->u8_RequireRandomizer && IF_LFSR_ENABLED())
			REG_READ_UINT16(NC_NAND_ZERO_BITCNT,u16_Tmp);
		else
			REG_READ_UINT16(NC_FCIE_ZERO_BITCNT,u16_Tmp);
			
		if (u16_Tmp <= pNandDrv->u16_ECCCorretableBit) {
			// when meet 1 empty page, all rest of pages in the same block are empty.
			memset(pu8_DataBuf, MAX_U8_VALUE,
				pNandDrv->u16_SectorByteCnt*u32_SectorCnt);
			if(pu8_SpareBuf) {
			    memset(pu8_SpareBuf, MAX_U8_VALUE,
			    	pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt);
			}
			// No need to reset read retry status
			// Clean False Alarm Status
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "NAND: Ok, Bit Flip-Flop \n");
			goto LABEL_EXIT;
		}
		//----------------------------------
		// 2. try slow clk
		if (0 == u8_RetryCnt_ECCFail) {
			nand_debug(UNFD_DEBUG_LEVEL, 0,
				"NAND: SlowClk, Retry times: %Xh, Max: %Xh \n", 
				u8_RetryCnt_ECCFail+1, u8_MaxReadRetryCount);           

			NC_SlowdownClk();
			u8_RetryCnt_ECCFail++;
			goto LABEL_READ_SECTOR;
		}
		//----------------------------------
		// 3. read-retry

		//----------------------------------
		// 4. toggle en/disable LFSR
		else {
			if (pNandDrv->u8_RequireRandomizer && 0==u16_ToggleLFSR) {
				u16_ToggleLFSR = 1; // toggle once

				u8_RetryCnt_ECCFail = 1; // no need to slow clk again
				nand_debug(UNFD_DEBUG_LEVEL, 0, "NAND: switch LFSR: ");

				if (IF_LFSR_ENABLED()) {
					NC_DisableLFSR();
					nand_debug(UNFD_DEBUG_LEVEL, 0, "OFF \n");
				} else {
					NC_EnableLFSR();
					nand_debug(UNFD_DEBUG_LEVEL, 0, "ON \n");
				}
	        		goto LABEL_READ_SECTOR;
			} else {
				nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "ECC Fail \n");
					nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0,
						"Blk: %Xh  Page: %X \n\n",
						u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
						u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask);

				NC_DumpRegisters();
				dump_nand_driver(pNandDrv);

				NC_CLR_DDR_MODE();
			}
		}
		#endif

		NC_CLR_DDR_MODE();
		REG_WRITE_UINT16(NC_PART_MODE, 0);

		#if IF_IP_VERIFY
		if (pNandDrv->u8_IfECCTesting)
			u32_Err = UNFD_ST_SUCCESS;
		#else
		u32_Err = UNFD_ST_ERR_ECC_FAIL;
		#endif
		goto LABEL_EXIT;
	}
	else if (pNandDrv->u8_SwEccCorr && (u32_SectorCnt == 1)
		&& (u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK)) {
		u16_Cnt = (u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK) >> BIT_NC_ECC_MAX_BITS_SHIFT;
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "%xh bit flip\r\n", u16_Cnt);

		for (u16_i=0; u16_i<u16_Cnt; u16_i++) {
			REG_CLR_BITS_UINT16(NC_ECC_STAT2, BIT_NC_ECC_SEL_LOC_MASK);
			REG_SET_BITS_UINT16(NC_ECC_STAT2, u16_i<<BIT_NC_ECC_SEL_LOC_SHIFT);
			LABEL_ECC_LOC_READ:
			REG_READ_UINT16(NC_ECC_LOC, u16_j);
			REG_READ_UINT16(NC_ECC_LOC, u16_k);
			if (u16_j != u16_k)
				goto LABEL_ECC_LOC_READ;
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "bit loc: %02Xh \r\n", u16_j);
			u16_ByteLoc = u16_j >> ECC_LOC_CNT_SHIFT;
			if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt)
				pu8_DataBuf[u16_ByteLoc] ^= 1<<(u16_j%ECC_LOC_CNT);
			else {
				if (pu8_SpareBuf) {
					u16_ByteLoc -= pNandDrv->u16_SectorByteCnt;
					pu8_SpareBuf[u16_ByteLoc] ^= 1<<(u16_j%ECC_LOC_CNT);
				}
			}
		}
	}

	// =====================================
	LABEL_EXIT:
	if (u8_RetryCnt_Events || u8_RetryCnt_ECCFail) {
		if(UNFD_ST_SUCCESS == u32_Err)
			nand_debug(UNFD_DEBUG_LEVEL,0,"NAND: retry ok. \n");

		//nand_debug(UNFD_DEBUG_LEVEL,1,"restore clk \n");
	}
	FLUSH_MIU_PIPE(); // Only used in U4 now
	nand_read_dma_post_flush((unsigned long)pu8_DataBuf,
		pNandDrv->u16_SectorByteCnt*u32_SectorCnt);
	nand_read_dma_post_flush((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt);

	REG_WRITE_UINT16(NC_PART_MODE, 0);
	NC_CLR_DDR_MODE();
	if (pNandDrv->u8_RequireRandomizer)
		NC_EnableLFSR();
	return u32_Err;
}

U32 NC_ReadSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_SectorCnt)
{
	U32 u32_Tmp, u32_SectorCnt_Tmp;
	U16 u16_PageCnt, u16_Tmp, u16_Cnt, u16_ByteLoc, u16_i, u16_j, u16_k;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr;

	#if CHECK_HAL_PARAM
	u32_Tmp = ((u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask) << pNandDrv->u8_PageSectorCntBits)
		+ u8_SectorInPage + u32_SectorCnt;
	if(u32_Tmp > ((U32)pNandDrv->u16_BlkPageCnt << pNandDrv->u8_PageSectorCntBits)) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_ReadSectors, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_HAL_R_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_R_INVALID_PARAM;
	}
	#endif

	// HW setting
	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE(); // to turn on ONFI clk
	
	if(pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	//-----------------------------------------------
	// read sectors
	u32_SectorCnt_Tmp = pNandDrv->u16_PageSectorCnt - u8_SectorInPage;
	u32_SectorCnt_Tmp = (u32_SectorCnt_Tmp > u32_SectorCnt) ? u32_SectorCnt : u32_SectorCnt_Tmp;

	if (u32_SectorCnt_Tmp) {
		//nand_debug(1, 1, "NC_ReadSectors read sectors: %u \r\n", u32_SectorCnt_Tmp);
		//nand_debug(1, 1, "%X %X %X %X %X \r\n",
		//  u32_PhyRowIdx, u8_SectorInPage, pu8_DataBuf, pu8_SpareBuf, u32_SectorCnt_Tmp);

		#if defined(FCIE_LFSR) && FCIE_LFSR
		REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
		REG_SET_BITS_UINT16(NC_LFSR_CTRL, ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
			& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
		#endif

		DataDMAAddr =
			nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
			pNandDrv->u16_SectorByteCnt*u32_SectorCnt_Tmp, READ_FROM_NAND );

		if (pu8_SpareBuf == NULL) {
			SpareDMAAddr = nand_translate_Spare_DMA_address_Ex(
				(unsigned long)pNandDrv->pu8_PageSpareBuf,
				pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt_Tmp,
				READ_FROM_NAND);
		}
		else
			SpareDMAAddr = nand_translate_Spare_DMA_address_Ex(
				(unsigned long)pu8_SpareBuf,
				pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt_Tmp,
				READ_FROM_NAND);

		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
		if (sizeof(dma_addr_t) == sizeof(unsigned long long))
			REG_WRITE_UINT16(NC_RSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

		// if not 512B/page, set Partial Mode
		REG_READ_UINT16(NC_ECC_CTRL, u16_Tmp);
		if (u16_Tmp & BIT_NC_PAGE_SIZE_MASK) {
			REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
			REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);
			REG_SET_BITS_UINT16(NC_PART_MODE, (u32_SectorCnt_Tmp-1)
				<<BIT_START_SECTOR_CNT_SHIFT);
			REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
			REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage
				<<BIT_START_SECTOR_IDX_SHIFT);
		}
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
		if (sizeof(dma_addr_t) == sizeof(unsigned long long))
			REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
		if (0 == pNandDrv->u8_WordMode)
			REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage
				<< pNandDrv->u8_SectorByteCntBits);
		else
			REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage
				<< (pNandDrv->u8_SectorByteCntBits-1));
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));
		REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle)
			| (CMD_0x00));
		if (pNandDrv->u16_Reg48_Spare& BIT_NC_ONE_COL_ADDR) { // if a page 512B
			REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DIN) | ACT_WAITRB);
			REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_BREAK);
		} else {
			REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
			REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_SER_DIN);
		}

		REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, u32_SectorCnt_Tmp-1);

		REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == (WAIT_READ_TIME)) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: NC_ReadSectors Timeout, ErrCode:%Xh \r\n",
				UNFD_ST_ERR_R_TIMEOUT);
			#if 0==IF_IP_VERIFY
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			NC_ResetNandFlash();
			#else
			//nand_stop();
			#endif
			REG_WRITE_UINT16(NC_PART_MODE, 0);
			return UNFD_ST_ERR_R_TIMEOUT;
		}

		FLUSH_MIU_PIPE(); // Only used in U4 now
		nand_read_dma_post_flush((unsigned long)pu8_DataBuf,
			pNandDrv->u16_SectorByteCnt*u32_SectorCnt_Tmp);
		nand_read_dma_post_flush((unsigned long)pu8_SpareBuf,
			pNandDrv->u16_SectorSpareByteCnt*u32_SectorCnt_Tmp);
		// Check ECC
		REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
		if (u16_Tmp & BIT_NC_ECC_FAIL) {
			/*  prevent ecc error print in Search CIS in TV platform
			    and no longer call this function after Search CIS. */
			#if defined(IF_IP_VERIFY) && IF_IP_VERIFY
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: NC_ReadSectors ECC Fail, Reg51:%04Xh ", u16_Tmp);
			REG_READ_UINT16(NC_ECC_STAT1, u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "Reg52:%04Xh ", u16_Tmp);
			REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 0, "Reg53:%04Xh \r\n", u16_Tmp);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Blk: %Xh  Page: %X\r\n",
				u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
				u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask);
			#endif

			NC_CLR_DDR_MODE();
			REG_WRITE_UINT16(NC_PART_MODE, 0);

			#if 0 == IF_IP_VERIFY
			return UNFD_ST_ERR_ECC_FAIL;
			#else
			if (pNandDrv->u8_IfECCTesting)
				return UNFD_ST_SUCCESS;
			else
				return UNFD_ST_ERR_ECC_FAIL;
			#endif
		}
		else if (pNandDrv->u8_SwEccCorr && (u32_SectorCnt == 1)
			&& (u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK)) {
			u16_Cnt = (u16_Tmp & BIT_NC_ECC_MAX_BITS_MASK) >> BIT_NC_ECC_MAX_BITS_SHIFT;
			nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "%xh bit flip\r\n", u16_Cnt);

			for (u16_i=0; u16_i<u16_Cnt; u16_i++) {
				REG_CLR_BITS_UINT16(NC_ECC_STAT2, BIT_NC_ECC_SEL_LOC_MASK);
				REG_SET_BITS_UINT16(NC_ECC_STAT2, u16_i<<BIT_NC_ECC_SEL_LOC_SHIFT);
				LABEL_ECC_LOC_READ:
				REG_READ_UINT16(NC_ECC_LOC, u16_j);
				REG_READ_UINT16(NC_ECC_LOC, u16_k);
				if (u16_j != u16_k)
					goto LABEL_ECC_LOC_READ;
				nand_debug(UNFD_DEBUG_LEVEL_LOW, 1, "bit loc: %02Xh \r\n", u16_j);
				u16_ByteLoc = u16_j >> ECC_LOC_CNT_SHIFT;
				if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt)
					pu8_DataBuf[u16_ByteLoc] ^= 1<<(u16_j%ECC_LOC_CNT);
				else {
					if (pu8_SpareBuf) {
						u16_ByteLoc -= pNandDrv->u16_SectorByteCnt;
						pu8_SpareBuf[u16_ByteLoc] ^= 1<<(u16_j%ECC_LOC_CNT);
					}
				}
			}
		}

		REG_WRITE_UINT16(NC_PART_MODE, 0);
	}

	//-----------------------------------------------
	// read pages
	u32_SectorCnt -= u32_SectorCnt_Tmp;
	if (0 == u32_SectorCnt) {
		NC_CLR_DDR_MODE();
		return UNFD_ST_SUCCESS;
	}

	u32_PhyRowIdx += 1;
	u16_PageCnt = u32_SectorCnt >> pNandDrv->u8_PageSectorCntBits;

	pu8_DataBuf += u32_SectorCnt_Tmp << pNandDrv->u8_SectorByteCntBits;
	if (pu8_SpareBuf)
		pu8_SpareBuf += u32_SectorCnt_Tmp * pNandDrv->u16_SectorSpareByteCnt;

	if (u16_PageCnt) {
		//nand_debug(1, 1, "NC_ReadSectors read pages %X %X %X %X \r\n",
		//  u32_PhyRowIdx, pu8_DataBuf, pu8_SpareBuf, u16_PageCnt);

		u32_Tmp = NC_ReadPages(u32_PhyRowIdx, pu8_DataBuf, pu8_SpareBuf, u16_PageCnt);
		if (UNFD_ST_SUCCESS != u32_Tmp) {
			NC_CLR_DDR_MODE();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: NC_ReadSectors read pages fail, ErrCode:%Xh \r\n", u32_Tmp);
			return u32_Tmp;
		}
	}

	//-----------------------------------------------
	// read sectors
	u32_SectorCnt_Tmp = u16_PageCnt << pNandDrv->u8_PageSectorCntBits;
	pu8_DataBuf += u32_SectorCnt_Tmp << pNandDrv->u8_SectorByteCntBits;
	if (pu8_SpareBuf)
		pu8_SpareBuf += u32_SectorCnt_Tmp * pNandDrv->u16_SectorSpareByteCnt;

	u32_SectorCnt -= u32_SectorCnt_Tmp;
	u32_PhyRowIdx += u16_PageCnt;
	if (u32_SectorCnt) {
		//nand_debug(1, 1, "NC_ReadSectors read last sectors %X %X %X %X \r\n",
		//  u32_PhyRowIdx, pu8_DataBuf, pu8_SpareBuf, u32_SectorCnt);

		u32_Tmp = NC_ReadSectors(u32_PhyRowIdx, 0, pu8_DataBuf, pu8_SpareBuf,
			u32_SectorCnt);
		if (UNFD_ST_SUCCESS != u32_Tmp) {
			NC_CLR_DDR_MODE();
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: NC_ReadSectors read last sectors fail, ErrCode:%Xh \r\n",
				u32_Tmp);
			return u32_Tmp;
		}
	}

	NC_CLR_DDR_MODE();
	return UNFD_ST_SUCCESS;
}

U32 NC_WriteSectors(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_SectorCnt )
{
	U16 u16_Tmp;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr = 0, SpareDMAAddr;
	U32 u32_Ret;

	#if CHECK_HAL_PARAM
	if (u8_SectorInPage + u32_SectorCnt > pNandDrv->u16_PageSectorCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR, NAND, over one page sector cnt\n");
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "%X %X %lX %lX %X\n",
			u32_PhyRowIdx, u8_SectorInPage, (unsigned long)pu8_DataBuf,
			(unsigned long)pu8_SpareBuf, u32_SectorCnt);
		return UNFD_ST_ERR_INVALID_PARAM;
	}
	if (0 == u32_SectorCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR, NAND, zero sector cnt\n");
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "%X %X %lX %lX %X\n",
			u32_PhyRowIdx, u8_SectorInPage, (unsigned long)pu8_DataBuf,
			(unsigned long)pu8_SpareBuf, u32_SectorCnt);
		return UNFD_ST_ERR_INVALID_PARAM;
	}
	#endif
	//nand_debug(0,0,"ws b:%Xh p:%Xh \n", u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
	//    u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask);

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL, ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
		& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_SectorByteCnt, WRITE_TO_NAND);
	if (pu8_SpareBuf == NULL) {
		memset(pNandDrv->pu8_PageSpareBuf, MAX_U8_VALUE,  pNandDrv->u16_SpareByteCnt);
		SpareDMAAddr =
		nand_translate_Spare_DMA_address_Ex((unsigned long)pNandDrv->pu8_PageSpareBuf,
		pNandDrv->u16_SectorSpareByteCnt, WRITE_TO_NAND);
	} else {
		SpareDMAAddr =
		nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SectorSpareByteCnt, WRITE_TO_NAND);
	}

	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	// if Partial Mode (if not 512B/page, set Partial Mode)
	REG_READ_UINT16(NC_ECC_CTRL, u16_Tmp);
	if (u16_Tmp & BIT_NC_PAGE_SIZE_MASK) {
		REG_WRITE_UINT16(NC_PART_MODE, BIT_PARTIAL_MODE_EN);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, (u32_SectorCnt-1)<<BIT_START_SECTOR_CNT_SHIFT);
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT);
	}
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if (0 == pNandDrv->u8_WordMode)
		REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage << pNandDrv->u8_SectorByteCntBits);
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u8_SectorInPage
		<< (pNandDrv->u8_SectorByteCntBits-1));
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70)| ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK)| ACT_CHKSTATUS);

	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	//while(1)  nand_reset_WatchDog();
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_WriteSectors Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	NC_CLR_DDR_MODE();
	REG_WRITE_UINT16(NC_PART_MODE, 0);

	#if defined(CONFIG_NAND_SECURITY_GEN) && CONFIG_NAND_SECURITY_GEN
	if ((REG(NC_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT) {
		//U32 u32_SecureRow;
		nand_debug(0, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		#if 0
		u32_SecureRow = LEFT_SHIFT16(REG(NC_SECURE_ERR_ADDR_H))|(REG(NC_SECURE_ERR_ADDR_L)
			& MAX_U16_VALUE);
		if (u32_SecureRow != u32_PhyRowIdx) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n",
				u32_SecureRow, u32_PhyRowIdx);
		}
		#endif
		NC_ReConfig();
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_SECURE_ALERT);
		return UNFD_ST_ERR_W_PROTECTED;
	}
	#endif

	u32_Ret = NC_CheckEWStatus(OPTYPE_WRITE);
	if (UNFD_ST_SUCCESS != u32_Ret)
		nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"write blk:%Xh p:%Xh fail:%Xh \n", 
			u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
			u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask, u32_Ret);

	return u32_Ret;
}

U32 NC_ReadSector_RIUMode(U32 u32_PhyRowIdx, U8 u8_SectorInPage, U8 *pu8_DataBuf,
	U8 *pu8_SpareBuf)
{
	volatile U16 u16_Tmp, u16_i, u16_j, u16_k, u16_Cnt, u16_ByteLoc;
	U16 u16_Col;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_Tmp, u32_Err;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN); // enable RIU Mode
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	if (pu8_SpareBuf == NULL)
		pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	// if Partial Mode (if not 512B/page, set Partial Mode)	
	REG_READ_UINT16(NC_ECC_CTRL, u16_Tmp);
	if (u16_Tmp & BIT_NC_PAGE_SIZE_MASK) {
		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_CNT_MASK);

		REG_CLR_BITS_UINT16(NC_PART_MODE, BIT_START_SECTOR_IDX_MASK);
		REG_SET_BITS_UINT16(NC_PART_MODE, (u8_SectorInPage<<BIT_START_SECTOR_IDX_SHIFT));
		REG_SET_BITS_UINT16(NC_PART_MODE,  BIT_PARTIAL_MODE_EN);
	}

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if (0 == pNandDrv->u8_WordMode)
		u16_Col = u8_SectorInPage << pNandDrv->u8_SectorByteCntBits;
	else
		u16_Col = u8_SectorInPage << (pNandDrv->u8_SectorByteCntBits-1);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u16_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR) {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	} else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_SER_DIN);
	}

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	//read for data
	u16_Cnt = pNandDrv->u16_SectorByteCnt >> CIFD_BYTE_SHIFT; // how many 64B per sector
	u16_Cnt = (pNandDrv->u16_SectorByteCnt & (CIFD_BYTE_CNT -1)) ? u16_Cnt + 1 : u16_Cnt;
	for (u16_j=0; u16_j<u16_Cnt; u16_j++) {   // read data
		u16_Tmp = ((pNandDrv->u16_SectorByteCnt - (u16_j << CIFD_BYTE_SHIFT))
			>= CIFD_BYTE_CNT) ? CIFD_BYTE_CNT
			: (pNandDrv->u16_SectorByteCnt - (u16_j << CIFD_BYTE_SHIFT) );
		u32_Err = NC_WaitGetCIFD(
				(U8*)((unsigned long)pu8_DataBuf + (u16_j << CIFD_BYTE_SHIFT)),
				u16_Tmp, WAIT_READ_TIME);
		if (u32_Err != UNFD_ST_SUCCESS) {
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			return u32_Err;
		}		
	}
	
	//read for spare
	u16_Cnt = pNandDrv->u16_SectorSpareByteCnt >> CIFD_BYTE_SHIFT; // how many 64B per sector
	u16_Cnt = (pNandDrv->u16_SectorSpareByteCnt & (CIFD_BYTE_CNT -1)) ? u16_Cnt + 1 : u16_Cnt;
	for (u16_j=0; u16_j<u16_Cnt; u16_j++) {   // read data
		u16_Tmp = ((pNandDrv->u16_SectorSpareByteCnt - (u16_j << CIFD_BYTE_SHIFT))
			>= CIFD_BYTE_CNT) ? CIFD_BYTE_CNT
			: (pNandDrv->u16_SectorSpareByteCnt - (u16_j << CIFD_BYTE_SHIFT));
		u32_Err = NC_WaitGetCIFD(
				(U8*)((unsigned long)pu8_SpareBuf + (u16_j << CIFD_BYTE_SHIFT)),
				u16_Tmp, WAIT_READ_TIME);
		if (u32_Err != UNFD_ST_SUCCESS) {
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			return u32_Err;
		}
	}

	// wait for events
	u32_Tmp = 0;
	do {
		u16_Tmp = 0; // do not remove this line
		REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
		u32_Tmp++;
		if (u32_Tmp > WAIT_READ_TIME) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: Read_RM timeout 2, ErrCode:%X\n", UNFD_ST_ERR_R_TIMEOUT_RM);
			NC_DumpDebugBus();
			NC_DumpRegisters();
			REG_WRITE_UINT16(NC_PART_MODE, 0);						
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			return UNFD_ST_ERR_R_TIMEOUT_RM;
		}
	} while (((u16_Tmp & BIT_NC_R2N_ECCCOR)==0) &&
		((u16_Tmp & (BIT_NC_JOB_END )) != (BIT_NC_JOB_END )));

	// if ECC Fail
	REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
	if ((u16_Tmp & BIT_NC_ECC_FLGA_MASK) == BIT_NC_ECC_FLGA_FAIL) {
		REG_WRITE_UINT16(NC_PART_MODE, 0);				
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);	
		NC_ReConfig();
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Read_RM ECC Fail, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_ECC_FAIL_RM);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Blk: %Xh  Page: %X\r\n",
		u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
		u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask);
		return UNFD_ST_ERR_ECC_FAIL_RM;
	}

	REG_READ_UINT16(NC_MIE_EVENT, u16_Tmp);
	if (u16_Tmp & BIT_NC_R2N_ECCCOR) {
//		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "RIU Mode, ECC happens: ");
		// make CIFD available for SW
		// [CAUTION]: before clear BIT_NC_R2N_ECC, JOB_END would not set.
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_R2N_ECCCOR);
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME) {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: Read_RM timeout 3, ErrCode:%X\n", UNFD_ST_ERR_R_TIMEOUT_RM);
			REG_WRITE_UINT16(NC_PART_MODE, 0);			
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);			
			NC_ReConfig();
			NC_CLR_DDR_MODE();
		}

		REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
		u16_Cnt = (u16_Tmp & BIT_NC_ECC_CNT_MASK) >> BIT_NC_ECC_CNT_SHIFT;
//		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "%u bits\r\n", u16_Cnt);
		for (u16_i=0; u16_i<u16_Cnt; u16_i++) {
			REG_CLR_BITS_UINT16(NC_ECC_STAT2, BIT_NC_ECC_SEL_LOC_MASK);
			REG_SET_BITS_UINT16(NC_ECC_STAT2, u16_i<<BIT_NC_ECC_SEL_LOC_SHIFT);
			LABEL_ECC_LOC_READ:
			//REG_READ_UINT16(NC_ECC_LOC, u16_Tmp);
			REG_READ_UINT16(NC_ECC_LOC, u16_j);
			REG_READ_UINT16(NC_ECC_LOC, u16_k);
			if (u16_j != u16_k)
				goto LABEL_ECC_LOC_READ;
//			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "bit loc: %02Xh \r\n", u16_j);
			u16_ByteLoc = u16_j >> ECC_LOC_CNT_SHIFT;
			if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt) {
				pu8_DataBuf[u16_ByteLoc] ^= 1<<(u16_j%ECC_LOC_CNT);
			} else if (u16_ByteLoc < pNandDrv->u16_SectorByteCnt +
			       pNandDrv->u16_SectorSpareByteCnt - pNandDrv->u16_ECCCodeByteCnt) {
				pu8_SpareBuf[u16_ByteLoc-pNandDrv->u16_SectorByteCnt]
					^= 1<<(u16_j%ECC_LOC_CNT);
			}
		}

		u16_Tmp = BIT_NC_R2N_ECCCOR;
	} else if (0 == (u16_Tmp & BIT_NC_R2N_ECCCOR)) {
		if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME) {
			U16 u16_Reg;
			REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"Error: Read_RM timeout 2, ErrCode:%Xh, Reg00h:%Xh\r\n",
			       UNFD_ST_ERR_R_TIMEOUT_RM, u16_Reg);

			REG_WRITE_UINT16(NC_PART_MODE, 0);
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

			#if 0 == IF_IP_VERIFY
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			//NC_ResetNandFlash(); // save code size
			#else
			//nand_stop();
			#endif
			return UNFD_ST_ERR_R_TIMEOUT_RM; // timeout
		}
	}

	NC_CLR_DDR_MODE();
  	REG_WRITE_UINT16(NC_PART_MODE, 0);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	return UNFD_ST_SUCCESS;
}

U32  NC_WritePage_RIUMode(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf )
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_i, u16_j, u16_DataCnt, u16_SpareCnt, u16_Tmp;
	U32 u32_Err;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);

	if (pu8_SpareBuf == NULL) {
		pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;
		memset(pu8_SpareBuf, MAX_U8_VALUE, pNandDrv->u16_SpareByteCnt);
	}

	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70)| ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK)| ACT_CHKSTATUS);

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	u16_DataCnt = pNandDrv->u16_SectorByteCnt >> CIFD_BYTE_SHIFT; // how many 64B per sector
	u16_DataCnt = (pNandDrv->u16_SectorByteCnt & (CIFD_BYTE_CNT -1))
		? u16_DataCnt + 1 : u16_DataCnt;

	// how many 64B per sector spare
	u16_SpareCnt = (pNandDrv->u16_SectorSpareByteCnt -  pNandDrv->u16_ECCCodeByteCnt)
		>> CIFD_BYTE_SHIFT;
	u16_SpareCnt = ((pNandDrv->u16_SectorSpareByteCnt -  pNandDrv->u16_ECCCodeByteCnt)
		& (CIFD_BYTE_CNT -1)) ? u16_SpareCnt + 1 : u16_SpareCnt;

	//---------------------------------------------------
	// fill in DO
	// do filling data for each sector, data and spare are filled separately
	for (u16_i = 0; u16_i <  pNandDrv->u16_PageSectorCnt; u16_i ++) {
		//fill for data 
		for (u16_j=0; u16_j<u16_DataCnt; u16_j++) {	
			u16_Tmp = ((pNandDrv->u16_SectorByteCnt - (u16_j << CIFD_BYTE_SHIFT))
				> CIFD_BYTE_CNT) ? CIFD_BYTE_CNT
				: pNandDrv->u16_SectorByteCnt - (u16_j << CIFD_BYTE_SHIFT);
			
			u32_Err = NC_WaitSetCIFD(
				(U8*)((unsigned long) pu8_DataBuf +
				(u16_i << pNandDrv->u8_SectorByteCntBits) +
				(u16_j << CIFD_BYTE_SHIFT)), u16_Tmp);
			if (u32_Err != UNFD_ST_SUCCESS) {
				nand_debug(0, 1, "Transfer Byte %X\n",
					(u16_i << pNandDrv->u8_SectorByteCntBits)
					+ (u16_j << CIFD_BYTE_SHIFT));
				REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
				NC_ReConfig();
				NC_CLR_DDR_MODE();
				return u32_Err;
			}
		}
		//fill for spare
		for (u16_j=0; u16_j<u16_SpareCnt; u16_j++) {	
			if ((REG(NC_ECC_CTRL) & BIT_NC_BYPASS_ECC) ==BIT_NC_BYPASS_ECC) {
				u16_Tmp = (((pNandDrv->u16_SectorSpareByteCnt)
					- (u16_j << CIFD_BYTE_SHIFT)) > CIFD_BYTE_CNT)
					? CIFD_BYTE_CNT
					: (pNandDrv->u16_SectorSpareByteCnt)
					- (u16_j << CIFD_BYTE_SHIFT);				
			} else {
				u16_Tmp = (((pNandDrv->u16_SectorSpareByteCnt
					- pNandDrv->u16_ECCCodeByteCnt)
					- (u16_j << CIFD_BYTE_SHIFT)) > CIFD_BYTE_CNT)
					? CIFD_BYTE_CNT
					: (pNandDrv->u16_SectorSpareByteCnt
					- pNandDrv->u16_ECCCodeByteCnt)
					- (u16_j << CIFD_BYTE_SHIFT);
			}

			u32_Err = NC_WaitSetCIFD(
				(U8*)((unsigned long)pu8_SpareBuf
				+ (u16_i *  pNandDrv->u16_SectorSpareByteCnt)
				+ (u16_j << CIFD_BYTE_SHIFT)),
				u16_Tmp);
			if (u32_Err != UNFD_ST_SUCCESS) {
				nand_debug(0, 1, "Transfer Spare Byte %X\n",
					(u16_i *  pNandDrv->u16_SectorSpareByteCnt)
					+ (u16_j << CIFD_BYTE_SHIFT));
				REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
				NC_ReConfig();
				NC_CLR_DDR_MODE();
				return u32_Err;
			}			
		}
	}

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME) {
		U16 u16_Reg;
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: Write_RM timeout 3, ErrCode:%Xh, Reg00h:%Xh\r\n",
		       UNFD_ST_ERR_W_TIMEOUT_RM, u16_Reg);
		#if 0 == IF_IP_VERIFY
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		//NC_ResetNandFlash(); // save code size
		#else
		//nand_stop();
		#endif
		return UNFD_ST_ERR_W_TIMEOUT_RM; // timeout
	}

	NC_CLR_DDR_MODE();
  	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
	return NC_CheckEWStatus(OPTYPE_WRITE);
}

U32 NC_Read_RandomIn (U32 u32_PhyRow, U32 u32_Col, U8 *pu8DataBuf, U32 u32DataByteCnt)
{
	U32 u32_Tmp;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_Err;
	U16 u16_Cnt, u16_j, u16_Tmp;
	
	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();
	u32_Tmp = u32DataByteCnt + (u32DataByteCnt & 0x01); // byet-count needs to be word-alignment

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRow = NC_SelectCE(u32_PhyRow);

	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if(pNandDrv->u8_WordMode)
		u32_Col >>= 1;
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR)
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(u32_Col));
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRow & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRow));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | CMD_0x00);
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR) {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_RAN_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	} else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_RAN_DIN);
	}

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Tmp);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	/* get data from CIFD */

	u16_Cnt = u32_Tmp >> CIFD_BYTE_SHIFT; // how many 64B of random count
	u16_Cnt = (u32_Tmp & (CIFD_BYTE_CNT -1)) ? u16_Cnt + 1 : u16_Cnt;
	for (u16_j=0; u16_j < u16_Cnt; u16_j++) { // read data
		u16_Tmp =( (u32_Tmp - (u16_j << CIFD_BYTE_SHIFT)) > CIFD_BYTE_CNT)
			? CIFD_BYTE_CNT : u32_Tmp - (u16_j << CIFD_BYTE_SHIFT);
		u32_Err = NC_WaitGetCIFD(pu8DataBuf + (u16_j << CIFD_BYTE_SHIFT),
			u16_Tmp, WAIT_READ_TIME);
		if (u32_Err != UNFD_ST_SUCCESS) {
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			return u32_Err;
		}		
	}
	
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Err: drvNAND_ReadRandomData_Ex - timeout \r\n");
		#if 0==IF_IP_VERIFY
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);		
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		
		#else
		//nand_stop();
		#endif
		return UNFD_ST_ERR_R_TIMEOUT; // timeout
	}

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	NC_CLR_DDR_MODE();
	
	return UNFD_ST_SUCCESS;
}

U32 NC_Write_RandomOut (U32 u32_PhyRow, U32 u32_Col, U8 *pu8_DataBuf, U32 u32_DataByteCnt )
{
	U32 u32_Tmp;
	U32 u32_Err;
	U16 u16_j, u16_Tmp, u16_Cnt;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	u32_Tmp = u32_DataByteCnt + (u32_DataByteCnt & 0x01); // byet-count needs to be word-alignment

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRow = NC_SelectCE(u32_PhyRow);
 
	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if (pNandDrv->u8_WordMode)
		u32_Col >>= 1;
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR)
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(u32_Col));
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRow & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRow));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_RAN_DOUT);

	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70)| ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK)| ACT_CHKSTATUS);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Tmp);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	u16_Cnt = u32_Tmp >> CIFD_BYTE_SHIFT; // how many 64B of random count
	u16_Cnt = (u32_Tmp & (CIFD_BYTE_CNT -1)) ? u16_Cnt + 1 : u16_Cnt;	
	for (u16_j=0; u16_j<u16_Cnt; u16_j++) {	
		u16_Tmp =( (u32_Tmp - (u16_j << CIFD_BYTE_SHIFT)) > CIFD_BYTE_CNT) ? 
			CIFD_BYTE_CNT: u32_Tmp - (u16_j << CIFD_BYTE_SHIFT);
		
		u32_Err = NC_WaitSetCIFD(
			(U8*)((unsigned long) pu8_DataBuf + (u16_j << CIFD_BYTE_SHIFT)),  u16_Tmp);
		if (u32_Err != UNFD_ST_SUCCESS) {
			nand_debug(0, 1, "Transfer Spare Byte %X\n", (u16_j << CIFD_BYTE_SHIFT));
			REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
			NC_ReConfig();
			NC_CLR_DDR_MODE();
			return u32_Err;
		}			
	}

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_Write_RandomOut Timeout, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif

		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	NC_CLR_DDR_MODE();
	return NC_CheckEWStatus(OPTYPE_WRITE);
}

U32 NC_Read_RandomIn_MIU(U32 u32_PhyRow, U32 u32_Col, U8 *pu8DataBuf, U32 u32DataByteCnt)
{
	U32 u32_Tmp;
	dma_addr_t DataDMAAddr;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();
	u32_Tmp = u32DataByteCnt + (u32DataByteCnt & 0x01); // byet-count needs to be word-alignment

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRow = NC_SelectCE(u32_PhyRow);

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8DataBuf, u32_Tmp,
		READ_FROM_NAND);

	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if (pNandDrv->u8_WordMode)
		u32_Col >>= 1;
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR)
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(u32_Col));
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRow & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRow));
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR) {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_RAN_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	} else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_RAN_DIN);
	}

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Tmp);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	/* get data from CIFD */

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME) == WAIT_READ_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Err: drvNAND_ReadRandomData_Ex - timeout \r\n");
		#if 0==IF_IP_VERIFY
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);		
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		
		#else
		//nand_stop();
		#endif
		return UNFD_ST_ERR_R_TIMEOUT; // timeout
	}

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	NC_CLR_DDR_MODE();

	return UNFD_ST_SUCCESS;
}

U32 NC_Write_RandomOut_MIU(U32 u32_PhyRow, U32 u32_Col, U8 *pu8_DataBuf,
	U32 u32_DataByteCnt )
{
	U32 u32_Tmp;
	dma_addr_t DataDMAAddr;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	u32_Tmp = u32_DataByteCnt + (u32_DataByteCnt & 0x01); // byet-count needs to be word-alignment

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRow = NC_SelectCE(u32_PhyRow);

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf, u32_Tmp,
		WRITE_TO_NAND);

	REG_WRITE_UINT16(NC_WDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	if (pNandDrv->u8_WordMode)
		u32_Col >>= 1;
	if (pNandDrv->u16_Reg48_Spare & BIT_NC_ONE_COL_ADDR)
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(u32_Col));
	else
		REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Col);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRow & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRow));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_RAN_DOUT);

	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70)| ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK)| ACT_CHKSTATUS);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_Tmp);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); // offset;

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME) == WAIT_WRITE_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_Write_RandomOut Timeout, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif

		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	NC_CLR_DDR_MODE();
	return NC_CheckEWStatus(OPTYPE_WRITE);
}

U32 NC_ReadID(void)
{
	volatile U16 u16_Reg;
	U16 u16_i;
	U16 u16_IDByteCnt;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	// data go through CIFD
	REG_SET_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ADR_C2T1S0) | CMD_0x90);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_RAN_DIN);

	if ( pNandDrv->u8_WordMode)
		u16_IDByteCnt = NAND_ID_BYTE_CNT<<1;
	else
		u16_IDByteCnt = NAND_ID_BYTE_CNT+1;

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RAN_CNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u16_IDByteCnt);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0); /*offset 0*/

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	//polling for wbuf is full, means wbuffer is full or random data is done
	if (NC_WaitCIFD_Event(BIT_WBUF_FULL, DELAY_100ms_in_us) == DELAY_100ms_in_us) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error! CIFD Event Timeout!\r\n");	
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_MIE_EVENT: %Xh \r\n", u16_Reg);
		REG_READ_UINT16(NC_CTRL, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_CTRL: %Xh \r\n", u16_Reg);
		NC_ReConfig();
		NC_ResetNandFlash();
		NC_CLR_DDR_MODE();
		REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	if ( pNandDrv->u8_WordMode) {
		for (u16_i=0 ; u16_i<NAND_ID_BYTE_CNT ; u16_i++)
			NC_GetCIFD(&pNandDrv->au8_ID[u16_i], u16_i<<1, 1);
	}
	else
		NC_GetCIFD(pNandDrv->au8_ID, 0, NAND_ID_BYTE_CNT);

	REG_W1C_BITS_UINT16(NC_CIFD_EVENT, BIT_WBUF_FULL);
	REG_SET_BITS_UINT16(NC_CIFD_EVENT, BIT_WBUF_EMPTY_TRI);

	if (NC_WaitComplete(BIT_NC_JOB_END, DELAY_100ms_in_us) == DELAY_100ms_in_us) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error! Read ID timeout!\r\n");

		#if 0==IF_IP_VERIFY
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_MIE_EVENT: %Xh \r\n", u16_Reg);
		REG_READ_UINT16(NC_CTRL, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "NC_CTRL: %Xh \r\n", u16_Reg);
		#else
		//nand_stop();
		#endif
		NC_ReConfig();
		NC_ResetNandFlash();
		NC_CLR_DDR_MODE();
		REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
		return UNFD_ST_ERR_R_TIMEOUT; // timeout
	}

	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "ID: %02X ", pNandDrv->au8_ID[0]);

	if (0 == pNandDrv->u8_IDByteCnt) {
		for (u16_i=1; u16_i<(NAND_ID_BYTE_CNT-1); u16_i++)
			if(pNandDrv->au8_ID[u16_i] == pNandDrv->au8_ID[0] && 
				pNandDrv->au8_ID[u16_i+1] == pNandDrv->au8_ID[1])
				break;
		pNandDrv->u8_IDByteCnt = u16_i;
	}

	for (u16_i=1; u16_i<pNandDrv->u8_IDByteCnt; u16_i++)
		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 0, "%02X ", pNandDrv->au8_ID[u16_i]);

	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 0, ", total %u bytes ID.\r\n", pNandDrv->u8_IDByteCnt);

	NC_CLR_DDR_MODE();
	REG_CLR_BITS_UINT16(NC_FUN_CTL,BIT_R2N_MODE_EN);
	return UNFD_ST_SUCCESS;
}

U32 NC_ProbeReadSeq(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16_Reg;
	volatile U32 u32_Count;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	// ProbeReadSeq uses polling mode, so disable interupts here
	REG_CLR_BITS_UINT16(NC_MIE_INT_EN, BIT_NC_JOB_END);
	#endif
	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));

	if (pNandDrv->u16_Reg48_Spare& BIT_NC_ONE_COL_ADDR) // if a page 512B
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_WAITRB);
	else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	}

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	for (u32_Count=0; u32_Count < WAIT_PROBE_TIME; u32_Count++) {
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		if ((u16_Reg & BIT_NC_JOB_END) == BIT_NC_JOB_END)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}

	if (u32_Count < WAIT_PROBE_TIME)
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END); /*clear events*/
	else {
		nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1,
			"Error: NC_ProbeReadSeq Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_R_TIMEOUT);
		NC_ReConfig();
		NC_ResetNandFlash();
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	return UNFD_ST_SUCCESS;
}

U32 NC_EraseBlk(U32 u32_PhyRowIdx)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_Ret;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	//nand_debug(0,0,"e b:%lXh \n", u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits);

	if (pNandDrv->u8_MaxCENum)
		u32_PhyRowIdx = NC_SelectCE(u32_PhyRowIdx);
    
	REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_Erase_AdrCycle) | CMD_0x60);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0xD0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_CHKSTATUS) | CMD_0x70);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);
	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_ERASE_TIME) == WAIT_ERASE_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_EraseBlk Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_E_TIMEOUT);

		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif

		NC_CLR_DDR_MODE();

		return UNFD_ST_ERR_E_TIMEOUT;
	}

	#if defined(CONFIG_NAND_SECURITY_GEN) && CONFIG_NAND_SECURITY_GEN
	if ((REG(NC_MIE_EVENT) & BIT_NC_SECURE_ALERT) == BIT_NC_SECURE_ALERT) {
		//U32 u32_SecureRow;
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Hit Secure range 0x%X\n", u32_PhyRowIdx);
		#if 0
		u32_SecureRow = LEFT_SHIFT16(REG(NC_SECURE_ERR_ADDR_H))|(REG(NC_SECURE_ERR_ADDR_L)
			& MAX_U16_VALUE);
		if(u32_SecureRow != u32_PhyRowIdx)
		{
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: Alert Address 0x%X!= 0x%X\n",
				u32_SecureRow, u32_PhyRowIdx);
		}			
		#endif
		NC_ReConfig();
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_SECURE_ALERT);
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_PROTECTED;
	}
	#endif

	u32_Ret = NC_CheckEWStatus(OPTYPE_ERASE);
	if(UNFD_ST_SUCCESS != u32_Ret)
		nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"erase blk:%Xh fail:%Xh \n", 
			u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, u32_Ret);

	NC_CLR_DDR_MODE();
	return u32_Ret;
}

U32 NC_CheckEWStatus(U8 u8_OpType)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U16 u16_Tmp;
	U32 u32_ErrCode = UNFD_ST_SUCCESS;

	REG_READ_UINT16(NC_ST_READ, u16_Tmp);

	if ((u16_Tmp & BIT_ST_READ_FAIL) == 1) { // if fail
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_FAIL;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_FAIL;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: NC_CheckEWStatus Fail, Nand St:%Xh, ErrCode:%Xh \r\n",
			REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_BUSYn) == 0) { // if busy
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_BUSY;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_BUSY;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: NC_CheckEWStatus Busy, Nand St:%Xh, ErrCode:%Xh \r\n",
		       REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	} else if ((u16_Tmp & BIT_ST_READ_PROTECTn) == 0) { // if protected
		if (OPTYPE_ERASE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_E_PROTECTED;
		else if (OPTYPE_WRITE == u8_OpType)
			u32_ErrCode = UNFD_ST_ERR_W_PROTECTED;

		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: NC_CheckEWStatus Protected, Nand St:%Xh, ErrCode:%Xh \r\n",
			REG(NC_ST_READ), u32_ErrCode);
		return u32_ErrCode;
	}

	return u32_ErrCode;
}

U32 NC_ResetNandFlash(void)
{
	NAND_DRIVER *pNandDrv=(NAND_DRIVER *)drvNAND_get_DrvContext_address();
	pNandDrv = pNandDrv;
	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

  	REG_W1C_BITS_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);

  	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0xFF);
  	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

  	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

  	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_RESET_TIME) == WAIT_RESET_TIME) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"ERROR: NC_ResetNandFlash, ErrCode:%Xh \r\n", UNFD_ST_ERR_RST_TIMEOUT);
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_RST_TIMEOUT;
  	}

	NC_CLR_DDR_MODE();
  	return UNFD_ST_SUCCESS;
}

U32 NC_WaitCIFD_Event(U16 u16_WaitEvent, U32  u32_MicroSec)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U32 u32_Count;
	volatile U16 u16_Reg;

	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++) {
		REG_READ_UINT16(NC_CIFD_EVENT, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}
	if (u32_Count >= u32_MicroSec) {
		REG_READ_UINT16(NC_CIFD_EVENT, u16_Reg);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "CIFD Event Timout %X\n", u16_Reg);
		if(Disable_Err_log == 0) {
			NC_DumpDebugBus();
			NC_DumpRegisters();
		}
	}
	return u32_Count;
}

U32 NC_WaitGetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt, U32 u32_MicroSec)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (u32_ByteCnt > NC_CIFD_WBUF_BYTE_CNT)
		return UNFD_ST_ERR_INVAL_CIFD_CNT;

	if (NC_WaitCIFD_Event(BIT_WBUF_FULL, u32_MicroSec)== (u32_MicroSec )) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: CIFD timeout 0, ErrCode:%Xh\r\n", UNFD_ST_ERR_R_TIMEOUT_RM);
		REG_WRITE_UINT16(NC_PART_MODE, 0);
		REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT_RM; // timeout
	}
	NC_GetCIFD(pu8_DataBuf , 0, u32_ByteCnt);

	REG_WRITE_UINT16(NC_CIFD_EVENT,BIT_WBUF_FULL);

	REG_WRITE_UINT16(NC_CIFD_EVENT, BIT_WBUF_EMPTY_TRI);

	return UNFD_ST_SUCCESS;
}

U32 NC_WaitSetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	if (u32_ByteCnt > NC_CIFD_RBUF_BYTE_CNT)
		return UNFD_ST_ERR_INVAL_CIFD_CNT;

	NC_SetCIFD(pu8_DataBuf, 0, u32_ByteCnt);

	//hw bug of 8bit macro ddr mode, if no delay cifd event would timeout
	nand_hw_timer_delay(DELAY_100us_in_us);

	REG_SET_BITS_UINT16(NC_CIFD_EVENT, BIT_RBUF_FULL_TRI);

	if (NC_WaitCIFD_Event(BIT_RBUF_EMPTY, WAIT_READ_TIME)== (WAIT_READ_TIME )) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: CIFD timeout 0, ErrCode:%Xh\r\n", UNFD_ST_ERR_R_TIMEOUT_RM);
		return UNFD_ST_ERR_R_TIMEOUT_RM; // timeout
	}

	REG_W1C_BITS_UINT16(NC_CIFD_EVENT, BIT_RBUF_EMPTY);

	return UNFD_ST_SUCCESS;
}

U32 NC_WaitComplete(U16 u16_WaitEvent, U32 u32_MicroSec)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	volatile U32 u32_Count;
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	volatile U32 u32_Err;
	#endif
	volatile U16 u16_Reg;
 
	#if defined(ENABLE_NAND_INTERRUPT_MODE) && ENABLE_NAND_INTERRUPT_MODE
	REG_READ_UINT16(NC_MIE_INT_EN, u16_Reg);
	if (u16_Reg & u16_WaitEvent) {
		u32_Err = nand_WaitCompleteIntr(u16_WaitEvent, u32_MicroSec);
		if ( u32_Err == UNFD_ST_SUCCESS )
			return 0;
		else {
			NC_DumpDebugBus();
			NC_DumpRegisters();

			nand_die();
			return u32_MicroSec;
		}
	}
	#endif

	for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++) {
		REG_READ_UINT16(NC_MIE_EVENT, u16_Reg);
		if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
			break;

		nand_hw_timer_delay(HW_TIMER_DELAY_1us);
	}

	if (u32_Count < u32_MicroSec)
		REG_W1C_BITS_UINT16(NC_MIE_EVENT, u16_WaitEvent); /*clear events*/
	else {
		if(Disable_Err_log == 0) {
			NC_DumpDebugBus();
			NC_DumpRegisters();
		}
	}

	return u32_Count;
}

void NC_SetCIFD_Const(U8 u8_Data, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_i;
	volatile U16 u16_Tmp;

	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		u16_Tmp &= MAX_U8_VALUE;
		u16_Tmp += LEFT_SHIFT8(u8_Data);
		//nand_debug(1, 1, "0: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
	}

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		u16_Tmp = u8_Data + LEFT_SHIFT8(u8_Data);
		//nand_debug(1, 1, "1: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
	}

	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		u16_Tmp = (u16_Tmp & LEFT_SHIFT8(MAX_U8_VALUE)) + u8_Data;
		//nand_debug(1, 1, "2: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
	}
}

void NC_SetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_i, u32_BufPos;
	volatile U16 u16_Tmp;

	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		u16_Tmp &= MAX_U8_VALUE;
		u16_Tmp += LEFT_SHIFT8(pu8_Buf[0]);
		//nand_debug(1, 1, "0: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
		u32_BufPos = 1;
	} else
		u32_BufPos = 0;

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		u16_Tmp = pu8_Buf[(u32_i<<1)+u32_BufPos] +
		      LEFT_SHIFT8(pu8_Buf[(u32_i<<1)+u32_BufPos+1]);
		//nand_debug(1, 1, "1: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
	}

	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		u16_Tmp = (u16_Tmp & LEFT_SHIFT8(MAX_U8_VALUE)) + pu8_Buf[(u32_i<<1)+u32_BufPos];
		//nand_debug(1, 1, "2: %X \r\n", u16_Tmp);
		REG_WRITE_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
	}
}

void NC_GetRBufCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_i, u32_BufPos;
	U16 u16_Tmp;

	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		pu8_Buf[0] = (U8)RIGHT_SHIFT8(u16_Tmp);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
		u32_BufPos = 1;
	} else
		u32_BufPos = 0;

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
		pu8_Buf[(u32_i<<1)+u32_BufPos+1] = (U8)RIGHT_SHIFT8(u16_Tmp);
	}

	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_RBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
	}

}

void NC_GetCIFD(U8 *pu8_Buf, U32 u32_CIFDPos, U32 u32_ByteCnt)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U32 u32_i, u32_BufPos;
	U16 u16_Tmp;

	if (u32_CIFDPos & 1) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_CIFDPos>>1), u16_Tmp);
		pu8_Buf[0] = (U8)RIGHT_SHIFT8(u16_Tmp);
		u32_CIFDPos += 1;
		u32_ByteCnt -= 1;
		u32_BufPos = 1;
	} else
		u32_BufPos = 0;

	for (u32_i=0; u32_i<u32_ByteCnt>>1; u32_i++) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
		pu8_Buf[(u32_i<<1)+u32_BufPos+1] = (U8)RIGHT_SHIFT8(u16_Tmp);
	}

	if (u32_ByteCnt - (u32_i<<1)) {
		REG_READ_UINT16(NC_WBUF_CIFD_ADDR(u32_i+(u32_CIFDPos>>1)), u16_Tmp);
		pu8_Buf[(u32_i<<1)+u32_BufPos] = (U8)u16_Tmp;
	}

}

U32 NC_ReadStatus(void)
{
	U8 u8Status = 0;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 *pu8_SpareBuf = pNandDrv->pu8_PageSpareBuf;

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_CHKSTATUS) | CMD_0x70);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START);

	if (NC_WaitComplete(BIT_NC_JOB_END, DELAY_100ms_in_us) == DELAY_100ms_in_us) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_ReadStatus Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_R_TIMEOUT);
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	u8Status = (U8)REG(NC_ST_READ);
	pu8_SpareBuf[0] = u8Status;

	NC_CLR_DDR_MODE();

	return UNFD_ST_SUCCESS;
}

// can not cross block
U32 NC_ADMAWritePages(    U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt )
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr=0;
	U32 u32_Ret;

	if(pNandDrv->u8_MaxCENum)
		return UNFD_ST_ERR_INVALID_PARAM;

	#if CHECK_HAL_PARAM
	// can not cross block
	if ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)+u32_PageCnt > pNandDrv->u16_BlkPageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_WritePages, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_HAL_W_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_W_INVALID_PARAM;
	}
	#endif

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();
	NC_SET_DDR_MODE(); // to turn on ONFI clk

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL, ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
		& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	/*
		Main Data DMA Address is the descriptor table buffer address
		FCIE read DMA descriptor from this address, the direction must be read.
	*/
	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt * u32_PageCnt, READ_FROM_NAND);
	SpareDMAAddr = nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SpareByteCnt, WRITE_TO_NAND);

	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_WSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_WSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x80));
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x10) | ACT_SER_DOUT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(CMD_0x70) | ACT_WAITRB);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_REPEAT) | ACT_CHKSTATUS);
	REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);
	//nand_debug(1, 1, "W Rpt Cnt: %Xh \r\n", u16_PageCnt-1);

	#if defined(ENABLE_NAND_POWER_SAVING_MODE) && ENABLE_NAND_POWER_SAVING_MODE
	nand_CheckPowerCut();
	#endif

	REG_WRITE_UINT16(NC_CTRL, BIT_NC_JOB_START|BIT_NC_ADMA_EN);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_WRITE_TIME*u32_PageCnt)
		== WAIT_WRITE_TIME*u32_PageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_WritePages Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_W_TIMEOUT);
		#if 0==IF_IP_VERIFY
		NC_ReConfig();
		NC_CLR_DDR_MODE();
		NC_ResetNandFlash();
		#else
		//nand_stop();
		#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_W_TIMEOUT; // timeout
	}

	u32_Ret = NC_CheckEWStatus(OPTYPE_WRITE);
	if (UNFD_ST_SUCCESS != u32_Ret) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR,1,"write blk:%Xh p:%Xh fail:%Xh \n", 
			u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits, 
			u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask, u32_Ret);
	}

	NC_CLR_DDR_MODE();

	return u32_Ret;
}

U32 NC_ADMAReadPages(U32 u32_PhyRowIdx, U8 *pu8_DataBuf, U8 *pu8_SpareBuf,
	U32 u32_PageCnt, U8 u8_Zdec)
{
	U16 u16_Tmp=0;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	dma_addr_t DataDMAAddr, SpareDMAAddr = 0;
	U8  u8_RetryCnt_Events=0, u8_RetryCnt_ECCFail=0;
	#if (0 == IF_IP_VERIFY)
	U8  u8_MaxReadRetryCount = NAND_TIMEOUT_RETRY_CNT;
	#endif

	#if CHECK_HAL_PARAM
	// can not cross block
	if ((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask) + u32_PageCnt >
	  pNandDrv->u16_BlkPageCnt) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "ERROR: NC_ReadPages, ErrCode:%Xh \r\n",
			UNFD_ST_ERR_HAL_R_INVALID_PARAM);
		return UNFD_ST_ERR_HAL_R_INVALID_PARAM;
	}
	#endif

	NC_PAD_SWITCH(pNandDrv->u8_PadMode);
	NC_RECONFIG();

	#if 0==IF_IP_VERIFY
	nand_redo_read:
	#endif

	NC_SET_DDR_MODE();

	REG_WRITE_UINT16(NC_MIE_EVENT, BIT_NC_JOB_END);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_R2N_MODE_EN);

	#if defined(FCIE_LFSR) && FCIE_LFSR
	REG_CLR_BITS_UINT16(NC_LFSR_CTRL, BIT_SEL_PAGE_MASK);
	REG_SET_BITS_UINT16(NC_LFSR_CTRL,
		((u32_PhyRowIdx & pNandDrv->u16_BlkPageCntMask)
		& BIT_SEL_PAGE_MASK>>BIT_SEL_PAGE_SHIFT)<<BIT_SEL_PAGE_SHIFT);
	#endif

	DataDMAAddr = nand_translate_DMA_address_Ex((unsigned long)pu8_DataBuf,
		pNandDrv->u16_PageByteCnt * u32_PageCnt, READ_FROM_NAND );
	SpareDMAAddr = nand_translate_Spare_DMA_address_Ex((unsigned long)pu8_SpareBuf,
		pNandDrv->u16_SpareByteCnt, READ_FROM_NAND );

	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR0, SpareDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RSPARE_DMA_ADR1, RIGHT_SHIFT16(SpareDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RSPARE_DMA_ADR2, RIGHT_SHIFT32(SpareDMAAddr));

	REG_WRITE_UINT16(NC_RDATA_DMA_ADR0, DataDMAAddr & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_RDATA_DMA_ADR1, RIGHT_SHIFT16(DataDMAAddr));
	if (sizeof(dma_addr_t) == sizeof(unsigned long long))
		REG_WRITE_UINT16(NC_RDATA_DMA_ADR2, RIGHT_SHIFT32(DataDMAAddr));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_ADRSET);
	REG_WRITE_UINT16(NC_AUXREG_DAT, 0);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PhyRowIdx & MAX_U16_VALUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, RIGHT_SHIFT16(u32_PhyRowIdx));

	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_INSTQUE);
	REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(pNandDrv->u8_OpCode_RW_AdrCycle) | (CMD_0x00));

	if (pNandDrv->u16_Reg48_Spare& BIT_NC_ONE_COL_ADDR) { // if a page 512B
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_SER_DIN) | ACT_WAITRB);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_BREAK) | ACT_REPEAT);
	}
	else {
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_WAITRB) | CMD_0x30);
		REG_WRITE_UINT16(NC_AUXREG_DAT, LEFT_SHIFT8(ACT_REPEAT) | ACT_SER_DIN);
		REG_WRITE_UINT16(NC_AUXREG_DAT, ACT_BREAK);
	}
	REG_WRITE_UINT16(NC_AUXREG_ADR, AUXADR_RPTCNT);
	REG_WRITE_UINT16(NC_AUXREG_DAT, u32_PageCnt - 1);

	REG_WRITE_UINT16(NC_TOTAL_SEC_NUM, (u32_PageCnt<<pNandDrv->u8_PageSectorCntBits)-1);

	// check & set - patch for MIU busy case
	// slow down FCIE clock to wait for MIU service,
	// enhance the retry (more reliable) for error handling.
	if (0!=u8_RetryCnt_Events || 0!=u8_RetryCnt_ECCFail) {
		if (pNandDrv->u8_RequireRandomizer == 0) {
			nand_clock_setting(FCIE_SW_SLOWEST_CLK);
			REG_WRITE_UINT16(NC_WIDTH, 0);
			REG_WRITE_UINT16(NC_LATCH_DATA,
				(REG(NC_LATCH_DATA) & ~BIT_NC_LATCH_DATA_MASK));
		}
		//nand_printf("check & set - patch for MIU busy case \n");
	}

	if (u8_Zdec)
		REG_WRITE_UINT16(NC_CTRL,  BIT_NC_JOB_START|BIT_NC_ADMA_EN|BIT_NC_ZDEC_EN);
	else
		REG_WRITE_UINT16(NC_CTRL,  BIT_NC_JOB_START|BIT_NC_ADMA_EN);

	if (NC_WaitComplete(BIT_NC_JOB_END, WAIT_READ_TIME * u32_PageCnt) ==
	  	(WAIT_READ_TIME * u32_PageCnt )) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
			"Error: NC_ReadPages Timeout, ErrCode:%Xh \r\n", UNFD_ST_ERR_R_TIMEOUT);
		#if (0==IF_IP_VERIFY)
		NC_ReConfig();
		NC_ResetNandFlash();
		if ( ++u8_RetryCnt_Events < NAND_TIMEOUT_RETRY_CNT )
		      goto nand_redo_read;
		else {
			nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1,
				"\033[31mSoft reset over %d times\n, stop!\033[m\n",
				NAND_TIMEOUT_RETRY_CNT);

			//If soft reset still can not solve this problem, show an alert
			//and return a error, restore the normal setting before return
			nand_clock_setting(pNandDrv->u32_Clk);
			REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
			REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
			NC_CLR_DDR_MODE();
			
			return UNFD_ST_ERR_R_TIMEOUT;
		}
		#else
		//nand_stop();
		#endif
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_R_TIMEOUT;
	}

	// restore - patch for MIU busy case
	// for retry ok, restore the normal setting
	if (0!=u8_RetryCnt_Events || 0!=u8_RetryCnt_ECCFail) {
		nand_clock_setting(pNandDrv->u32_Clk);
		REG_WRITE_UINT16(NC_WIDTH, FCIE_REG41_VAL);
		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
		//nand_printf("restore - patch for MIU busy case \n");
	}

	//-----------------------------------

	#if defined(CONFIG_MIPS32)
	Chip_Read_Memory();
	#endif

	// Check ECC
	REG_READ_UINT16(NC_ECC_STAT0, u16_Tmp);
	if (u16_Tmp & BIT_NC_ECC_FAIL) {	
		#if (0 == IF_IP_VERIFY)
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: NC_ReadPages ECC Fail, Reg51:%04Xh ",
			u16_Tmp);
		REG_READ_UINT16(NC_ECC_STAT1, u16_Tmp);
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "Reg52:%04Xh ", u16_Tmp);
		REG_READ_UINT16(NC_ECC_STAT2, u16_Tmp);
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 0, "Reg53:%04Xh \r\n", u16_Tmp);
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Error: Blk: %Xh  Page: %X\r\n",
		    	u32_PhyRowIdx>>pNandDrv->u8_BlkPageCntBits,
		    	u32_PhyRowIdx&pNandDrv->u16_BlkPageCntMask);
		nand_debug(UNFD_DEBUG_LEVEL_LOW, 1,
			"ERROR, NAND, RETRY_READ_ECC_FAIL over %d times \n", u8_MaxReadRetryCount);
		NC_CLR_DDR_MODE();
		return UNFD_ST_ERR_ECC_FAIL;
		#else
		NC_CLR_DDR_MODE();
		if (pNandDrv->u8_IfECCTesting)
			return UNFD_ST_SUCCESS;
		else
			return UNFD_ST_ERR_ECC_FAIL;
		#endif
	}

	NC_CLR_DDR_MODE();

	return UNFD_ST_SUCCESS;
}

