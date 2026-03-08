// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#include "drvNAND.h"
#include "memalign.h"
#ifdef CONFIG_ROM_OVERLAY
#include <mtk_romtblo.h>
#endif

static NAND_DRIVER sg_NandDrv;
static U8 gau8_PartInfo[PARTINFO_BYTE_CNT];
PAIRED_PAGE_MAP_t ga_tPairedPageMap[PAIRED_PAGE_COUNT] = {{0,0}};

U32 nand_hw_timer_delay(U32 u32_us)
{
	volatile U32 u32_i = u32_us;

	while (u32_i > HW_TIMER_DELAY_1ms) {
		udelay(HW_TIMER_DELAY_1ms);
		u32_i -= HW_TIMER_DELAY_1ms;
	}

	udelay(u32_i);

	return u32_us;
}

U32 nand_pads_init(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	pNandDrv->u16_Reg40_Signal =
		(BIT_NC_WP_AUTO | BIT_NC_WP_H | BIT_NC_CE_AUTO | BIT_NC_CE_H) &
		~(BIT_NC_CHK_RB_EDGEn | BIT_NC_CE_SEL_MASK);

	return UNFD_ST_SUCCESS;
}

U32 nand_pads_switch(U32 u32EnableFCIE)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	pNandDrv->u16_Reg58_DDRCtrl &= ~(BIT_DDR_MASM|BIT_SDR_DIN_FROM_MACRO|BIT_NC_32B_MODE);

 	REG_WRITE_UINT16(NC_DDR_CTRL, pNandDrv->u16_Reg58_DDRCtrl);

	return UNFD_ST_SUCCESS;
}

U32 nand_clock_setting(U32 u32ClkParam)
{
	#define CLK_SRC_SHIFT	2
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_Tmp;
	U16 u16_ClkMask;
	U16 u16_ClkShift;

	u16_ClkShift = pNandDrv->priv->clk_info.clk_shift;
	u16_ClkMask = (((1<<pNandDrv->priv->clk_info.clk_bit_cnt)-1)<<CLK_SRC_SHIFT);
	u16_Tmp = (FCIE_CLK_GATING|FCIE_CLK_INVERSE|u16_ClkMask) << u16_ClkShift;
	REG_CLR_BITS_UINT16(reg_ckg_fcie, u16_Tmp);

	u16_Tmp = (pNandDrv->priv->clk_info.clk_table[u32ClkParam].value)
		<< (u16_ClkShift+CLK_SRC_SHIFT);
	REG_SET_BITS_UINT16(reg_ckg_fcie, u16_Tmp);

	return UNFD_ST_SUCCESS;
}

void nand_DumpPadClk(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	nand_debug(0, 1, "clk setting: \n");
	nand_debug(0, 1, "  reg_ckg_fcie(0x%lX):0x%X\n", (unsigned long)reg_ckg_fcie,
		REG(reg_ckg_fcie));
  	nand_debug(0, 0, "\n\n");
}

#if defined(DECIDE_CLOCK_BY_NAND) && DECIDE_CLOCK_BY_NAND
#define MAX_UNFD(a,b) ((a) > (b) ? (a) : (b))

U32 nand_config_timing(U16 u16_1T)
{
	#define NC_DEFAULT_DELAY	2
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_DefaultTRR;
	U16 u16_DefaultTCS;
	U16 u16_DefaultTWW;
	U16 u16_DefaultRX40Cmd;
	U16 u16_DefaultRX40Adr;
	U16 u16_DefaultRX56;
	U16 u16_DefaultTADL;
	U16 u16_DefaultTCWAW;
	#if defined(NC_TWHR_TCLHZ) && NC_TWHR_TCLHZ
	U16 u16_DefaultTCLHZ = NC_SDR_DEFAULT_TCLHZ;
	#endif
	U16 u16_DefaultTWHR;
	U16 u16_Tmp, u16_Cnt;
	U16 u16_Tmp2, u16_Cnt2;

	u16_DefaultTRR = NC_SDR_DEFAULT_TRR;
	u16_DefaultTCS = NC_SDR_DEFAULT_TCS;
	u16_DefaultTWW = NC_SDR_DEFAULT_TWW;
	u16_DefaultRX40Cmd = NC_SDR_DEFAULT_RX40CMD;
	u16_DefaultRX40Adr = NC_SDR_DEFAULT_RX40ADR;
	u16_DefaultRX56 = NC_SDR_DEFAULT_RX56;
	u16_DefaultTADL = NC_SDR_DEFAULT_TADL;
	u16_DefaultTCWAW = NC_SDR_DEFAULT_TCWAW;
	u16_DefaultTWHR = NC_SDR_DEFAULT_TWHR;

	#if defined(NC_INST_DELAY) && NC_INST_DELAY
	// Check CMD_END
	u16_Tmp = MAX_UNFD(pNandDrv->u16_tWHR, pNandDrv->u16_tCWAW);
	u16_Cnt = (u16_Tmp + u16_1T - 1) / u16_1T;

	if(u16_DefaultRX40Cmd >= u16_Cnt)
		u16_Cnt = 0;
	else if(u16_Cnt-u16_DefaultRX40Cmd > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt -= u16_DefaultRX40Cmd;

	// Check ADR_END
	u16_Tmp2 = MAX_UNFD(MAX_UNFD(pNandDrv->u16_tWHR, pNandDrv->u16_tADL), pNandDrv->u16_tCCS);
	u16_Cnt2 = (u16_Tmp2 + u16_1T - 1) / u16_1T;

	if(u16_DefaultRX40Adr >= u16_Cnt2)
		u16_Cnt2 = 0;
	else if(u16_Cnt2-u16_DefaultRX40Adr > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt2 -= u16_DefaultRX40Adr;

	// get the max cnt
	u16_Cnt = MAX_UNFD(u16_Cnt, u16_Cnt2);

	pNandDrv->u16_Reg40_Signal &= ~(LEFT_SHIFT8(MAX_U8_VALUE));
	pNandDrv->u16_Reg40_Signal |= LEFT_SHIFT8(u16_Cnt);
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_Reg40_Signal =  %X\n",
		pNandDrv->u16_Reg40_Signal);
	#endif

	#if defined(NC_HWCMD_DELAY) && NC_HWCMD_DELAY
	u16_Cnt = ((pNandDrv->u16_tRHW + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultRX56 >= u16_Cnt)
		u16_Cnt = 0;
	else if(u16_Cnt-u16_DefaultRX56 > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt -= u16_DefaultRX56;

	pNandDrv->u16_Reg56_Rand_W_Cmd &= ~(LEFT_SHIFT8(MAX_U8_VALUE));
	pNandDrv->u16_Reg56_Rand_W_Cmd |= LEFT_SHIFT8(u16_Cnt);
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_Reg56_Rand_W_Cmd =  %X\n",
		pNandDrv->u16_Reg56_Rand_W_Cmd);
	#endif

	#if defined(NC_TRR_TCS) && NC_TRR_TCS
	u16_Cnt = ((pNandDrv->u8_tRR + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTRR >= u16_Cnt)
		u16_Cnt = 0;
	else if(u16_Cnt-u16_DefaultTRR > MAX_U4_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt -= u16_DefaultTRR;

	u16_Tmp = ((pNandDrv->u8_tCS + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTCS >= u16_Tmp)
		u16_Tmp = 0;
	else if(u16_Tmp-u16_DefaultTCS > MAX_U4_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Tmp -= u16_DefaultTCS;

	u16_Tmp2 = ((pNandDrv->u16_tWW + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTWW >= u16_Tmp2)
		u16_Tmp2 = 0;
	else if(u16_Tmp2-u16_DefaultTWW > MAX_U4_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Tmp2 -= u16_DefaultTWW;

	u16_Cnt2 = MAX_UNFD(u16_Tmp, u16_Tmp2);

	pNandDrv->u16_Reg59_LFSRCtrl &= ~((U16)MAX_U8_VALUE);
	pNandDrv->u16_Reg59_LFSRCtrl |= (u16_Cnt | LEFT_SHIFT4(u16_Cnt2));
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_Reg59_LFSRCtrl =  %X\n",
		pNandDrv->u16_Reg59_LFSRCtrl);
	#endif

	#if defined(NC_TCWAW_TADL) && NC_TCWAW_TADL
	u16_Cnt = ((pNandDrv->u16_tADL + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTADL > u16_Cnt)
		u16_Cnt = 0;
	else if(u16_Cnt-u16_DefaultTADL > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt -= u16_DefaultTADL;

	u16_Cnt2 = ((pNandDrv->u16_tCWAW + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTCWAW > u16_Cnt2)
		u16_Cnt2 = 0;
	else if(u16_Cnt2-u16_DefaultTCWAW > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt2 -= u16_DefaultTCWAW;

	pNandDrv->u16_Reg5D_tCWAW_tADL &= ~(MAX_U16_VALUE);
	pNandDrv->u16_Reg5D_tCWAW_tADL |= (u16_Cnt|LEFT_SHIFT8(u16_Cnt2));
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_Reg5D_tCWAW_tADL =  %X\n",
		pNandDrv->u16_Reg5D_tCWAW_tADL);
	#endif

	#if defined(NC_TWHR_TCLHZ) && NC_TWHR_TCLHZ
	u16_Cnt = ((pNandDrv->u8_tCLHZ + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTCLHZ > u16_Cnt)
		u16_Cnt = 0;
	else if(u16_Cnt-u16_DefaultTCLHZ > MAX_U4_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt -= u16_DefaultTCLHZ;

	u16_Cnt2 = ((pNandDrv->u16_tWHR + u16_1T - 1) / u16_1T) + NC_DEFAULT_DELAY;

	if(u16_DefaultTWHR > u16_Cnt2)
		u16_Cnt2 = 0;
	else if(u16_Cnt2-u16_DefaultTWHR > MAX_U8_VALUE)
		return UNFD_ST_ERR_INVALID_PARAM;
	else
		u16_Cnt2 -= u16_DefaultTWHR;

	pNandDrv->u16_Reg5A_tWHR_tCLHZ &= ~(MAX_U16_VALUE);
	pNandDrv->u16_Reg5A_tWHR_tCLHZ |= ((u16_Cnt & MAX_U4_VALUE) | LEFT_SHIFT8(u16_Cnt2));
	nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_Reg5A_tWHR_tCLHZ =  %X\n",
		pNandDrv->u16_Reg5A_tWHR_tCLHZ);
	#endif

	NC_Config();
	return UNFD_ST_SUCCESS;
}

U32 nand_find_timing(U8 *pu8_ClkIdx, U8 u8_find_DDR_timg)
{
	#define NC_LATCH_DATA_CNT	16
	#define VALUE_1G		1000000000
	#define BIT_NC_LATCH_STS_SHIFT	5
	#define NC_LATCH_STS_CNT	8
	U32 u32_Err;
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U16 u16_i, u16_j, u16_pass_begin, u16_pass_cnt;
	U16 u16_status;
	U8 au8_ID[NAND_ID_BYTE_CNT];
	U32 u32_Clk;
	U16 u16_SeqAccessTime, u16_Tmp, u16_Tmp2, u16_1T;
	S8 s8_ClkIdx;

	if (u8_find_DDR_timg)
		return UNFD_ST_ERR_PAD_UNSUPPORT_DDR_NAND;

	u16_Tmp = MAX_UNFD(MAX_UNFD(pNandDrv->u8_tRP, pNandDrv->u8_tREH), (pNandDrv->u16_tRC+1)>>1);
	u16_Tmp2 = MAX_UNFD(MAX_UNFD(pNandDrv->u8_tWP, pNandDrv->u8_tWH), (pNandDrv->u16_tWC+1)>>1);
	u16_SeqAccessTime = MAX_UNFD(u16_Tmp, u16_Tmp2);

	u16_Tmp = (pNandDrv->u8_tREA + NAND_SEQ_ACC_TIME_TOL)>>1;
	u16_Tmp2 = u16_SeqAccessTime;
	u16_SeqAccessTime = MAX_UNFD(u16_Tmp, u16_Tmp2);

	u32_Clk = VALUE_1G/((U32)u16_SeqAccessTime);
	for (s8_ClkIdx =  0 ; s8_ClkIdx < pNandDrv->priv->clk_info.clk_cnt ; s8_ClkIdx ++) {
		if (u32_Clk < pNandDrv->priv->clk_info.clk_table[s8_ClkIdx].clk_hz)
			break;
	}
	if ((--s8_ClkIdx) < 0)
		return UNFD_ST_ERR_INVALID_PARAM;

	while (s8_ClkIdx >= 0) {
		u16_1T = VALUE_1G / pNandDrv->priv->clk_info.clk_table[s8_ClkIdx].clk_hz; //us
		if (nand_config_timing(u16_1T) != UNFD_ST_SUCCESS)
			s8_ClkIdx--;
		else
			break;
	}
	if (s8_ClkIdx < 0)
		return UNFD_ST_ERR_INVALID_PARAM;

	u32_Err = NC_ReadStatus();
	if (u32_Err != UNFD_ST_SUCCESS)
		return u32_Err;
	REG_READ_UINT16(NC_ST_READ, u16_status);

	pNandDrv->u32_Clk = s8_ClkIdx;
	nand_clock_setting(pNandDrv->u32_Clk);

	//using read id to detect relatch
	memcpy(au8_ID, pNandDrv->au8_ID, NAND_ID_BYTE_CNT);
	u16_pass_begin = MAX_U8_VALUE;
	u16_pass_cnt = 0;

	for (u16_i = 0 ; u16_i < NC_LATCH_DATA_CNT ; u16_i++) {
		pNandDrv->u16_Reg57_RELatch &= ~(BIT_NC_LATCH_DATA_MASK|BIT_NC_PAD_SEL_FAILLING);
		if (u16_i & 1) //select falling edge otherwise rising edge is selected
			pNandDrv->u16_Reg57_RELatch |= BIT_NC_PAD_SEL_FAILLING;
		pNandDrv->u16_Reg57_RELatch |= ((u16_i >> 1) << 1) & BIT_NC_LATCH_DATA_MASK;

		NC_Config();
		u32_Err = NC_ReadID();
		if (u32_Err != UNFD_ST_SUCCESS) {
			nand_debug(0, 1, "ReadID Error with ErrCode 0x%X\n", u32_Err);
			//nand_die();
			return u32_Err;
		}
		for (u16_j = 0 ; u16_j < NAND_ID_BYTE_CNT ; u16_j++) {
			if (pNandDrv->au8_ID[u16_j] != au8_ID[u16_j])
				break;
		}
		if (u16_j == NAND_ID_BYTE_CNT) {
			if (u16_pass_begin == MAX_U8_VALUE)
				u16_pass_begin = u16_i;
			u16_pass_cnt ++;
		}
		//break;
	}

	if (u16_pass_cnt == 0) {
		nand_debug(0, 1, "Read ID detect timing fails\n");
		pNandDrv->u16_Reg57_RELatch = REG57_ECO_FIX_INIT_VALUE;
		NC_Config();
		s8_ClkIdx = 0;
		*pu8_ClkIdx = (U8)s8_ClkIdx;
		memcpy(pNandDrv->au8_ID, au8_ID, NAND_ID_BYTE_CNT);
		return UNFD_ST_SUCCESS;
	} else {
		u16_i = u16_pass_begin + (u16_pass_cnt >> 1);
		pNandDrv->u16_Reg57_RELatch &= ~(BIT_NC_LATCH_DATA_MASK|BIT0);
		if (u16_i & 1) //select falling edge otherwise rising edge is selected
			pNandDrv->u16_Reg57_RELatch |= BIT0;
		pNandDrv->u16_Reg57_RELatch |= ((u16_i >> 1) << 1) & BIT_NC_LATCH_DATA_MASK;
		memcpy(pNandDrv->au8_ID, au8_ID, NAND_ID_BYTE_CNT);
	}

	//detect read status
	u16_pass_begin = MAX_U8_VALUE;
	u16_pass_cnt = 0;

	for (u16_i = 0 ; u16_i < NC_LATCH_STS_CNT ; u16_i ++) {
		U16 u16_tmpStatus;
		pNandDrv->u16_Reg57_RELatch &= ~(BIT_NC_LATCH_STS_MASK);
		pNandDrv->u16_Reg57_RELatch |=
			((u16_i) << BIT_NC_LATCH_STS_SHIFT) & BIT_NC_LATCH_STS_MASK;

		NC_Config();
		NC_ReadStatus();
		REG_READ_UINT16(NC_ST_READ, u16_tmpStatus);

		if (u16_tmpStatus == u16_status) {
			if(u16_pass_begin == MAX_U8_VALUE)
				u16_pass_begin = u16_i;
			u16_pass_cnt ++;
		}
	}

	if (u16_pass_cnt == 0) {
		nand_debug(0, 1, "Read status detect timing fails\n");
		pNandDrv->u16_Reg57_RELatch = REG57_ECO_FIX_INIT_VALUE;
		NC_Config();
		s8_ClkIdx = 0;
		*pu8_ClkIdx = (U8)s8_ClkIdx;

		return UNFD_ST_SUCCESS;
	} else {
		u16_i = u16_pass_begin + (u16_pass_cnt >> 1);
		pNandDrv->u16_Reg57_RELatch &= ~(BIT_NC_LATCH_STS_MASK);
		pNandDrv->u16_Reg57_RELatch |=
			((u16_i) << BIT_NC_LATCH_STS_SHIFT) & BIT_NC_LATCH_STS_MASK;
	}

	*pu8_ClkIdx = (U8)s8_ClkIdx;

	return UNFD_ST_SUCCESS;
}
#endif

// [nand_config_clock] the entry for clock auto-config (by DECIDE_CLOCK_BY_NAND)
// [nand_find_timing]   find a FCIE clock
// [nand_config_timing] set other auxiliary parameters (cycle count)
// if timing parameters in nni are 0, would use default cycle count.
U32 nand_config_clock(void)
{
	#define MHZ	1000000
	U32 u32_Err = UNFD_ST_SUCCESS;
	#if defined(DECIDE_CLOCK_BY_NAND) && DECIDE_CLOCK_BY_NAND
	NAND_DRIVER * pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	U8 u8_ClkIdx;

	#if defined(FCIE_LFSR) && FCIE_LFSR
	if (pNandDrv->u8_RequireRandomizer == 1)
		NC_DisableLFSR();
	#endif

	u32_Err = nand_find_timing(&u8_ClkIdx, 0);
	if (u32_Err != UNFD_ST_SUCCESS) {
		nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Err, NAND, Cannot config nand timing\n");
		//nand_die();
	} else {
		pNandDrv->u32_Clk = u8_ClkIdx;
		printf("FCIE is set to %dMHz\n",
			pNandDrv->priv->clk_info.clk_table[u8_ClkIdx].clk_hz/MHZ);

		nand_clock_setting(pNandDrv->u32_Clk);
		REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
	}

	#if defined(FCIE_LFSR) && FCIE_LFSR
	if (pNandDrv->u8_RequireRandomizer == 1)
		NC_EnableLFSR();
	#endif
	#endif

	return u32_Err;
}

void nand_CheckPowerCut(void)
{
}

void nand_Prepare_Power_Saving_Mode_Queue(void)
{
}

dma_addr_t nand_translate_DMA_address_Ex(unsigned long ulong_DMAAddr, U32 u32_ByteCnt, int mode)
{
	flush_cache(ulong_DMAAddr, u32_ByteCnt);

	return (virt_to_phys((void*)ulong_DMAAddr) - CONFIG_BUSADDR_START);
}

dma_addr_t nand_translate_Spare_DMA_address_Ex(unsigned long ulong_DMAAddr, U32 u32_ByteCnt, int mode)
{
	flush_cache(ulong_DMAAddr, u32_ByteCnt);

	return (virt_to_phys((void*)ulong_DMAAddr) - CONFIG_BUSADDR_START);
}

dma_addr_t nand_translate_DMA_MIUSel(unsigned long ulong_DMAAddr, U32 u32_ByteCnt)
{
	return 0;
}

void nand_read_dma_post_flush(unsigned long ulong_DMAAddr, U32 u32_ByteCnt)
{
	invalidate_dcache_range(ulong_DMAAddr, (ulong_DMAAddr + (unsigned long)u32_ByteCnt));
}

void *drvNAND_get_DrvContext_address(void)
{
	return ((void *)&sg_NandDrv);
}

void *drvNAND_get_DrvContext_PartInfo(void)
{
	return ((void*)gau8_PartInfo);
}

U32 NC_PlatformResetPre(void)
{

	return UNFD_ST_SUCCESS;
}

U32 NC_PlatformResetPost(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();

	REG_WRITE_UINT16(NC_FUN_CTL, BIT_NC_EN);

	return UNFD_ST_SUCCESS;
}

U32 NC_PlatformInit(void)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER *)drvNAND_get_DrvContext_address();
	unsigned int chip = 0;
	#define MT5876_CHIP_ID	5876

	nand_pads_init();
	pNandDrv->u8_WordMode = 0; // TV/Set-Top Box projects did not support x16 NAND flash
	nand_pads_switch(NAND_PAD_BYPASS_MODE);
	pNandDrv->u8_PadMode = NAND_PAD_BYPASS_MODE;

	pNandDrv->u32_Clk = FCIE_SW_SLOWEST_CLK;
	nand_clock_setting(FCIE_SW_SLOWEST_CLK);
	pNandDrv->u16_Reg57_RELatch = REG57_ECO_FIX_INIT_VALUE;
	REG_WRITE_UINT16(NC_LATCH_DATA, pNandDrv->u16_Reg57_RELatch);
	pNandDrv->u16_Reg59_LFSRCtrl = 0;
	REG_WRITE_UINT16(NC_LFSR_CTRL, 0);

	pNandDrv->pu8_PageDataBuf = (U8 *)malloc_cache_aligned(PAGE_DATA_BUF_SIZE);
	pNandDrv->pu8_PageSpareBuf = (U8 *)malloc_cache_aligned(PAGE_SPARE_BUF_SIZE);
	if (!pNandDrv->pu8_PageDataBuf || !pNandDrv->pu8_PageSpareBuf) {
		nand_debug(0, 1, "malloc_cache_aligned fail\n");
		return UNFD_ST_ERR_NULL_PTR;
	}

	if (REG(REG_SW_ECC) == TAG_SW_ECC)
		pNandDrv->u8_SwEccCorr = 1;
	else
		pNandDrv->u8_SwEccCorr = 0;

	#ifdef CONFIG_ROM_OVERLAY
	chip = romtbl_get_chip_id_info_dec();
	if (chip == MT5876_CHIP_ID)
		pNandDrv->u16_NandInfoECCType = ECC_TYPE_60BIT1KB;
	else
		pNandDrv->u16_NandInfoECCType = ECC_TYPE_40BIT1KB;
	#else
	pNandDrv->u16_NandInfoECCType = ECC_TYPE_40BIT1KB;
	#endif

	return UNFD_ST_SUCCESS;
}

