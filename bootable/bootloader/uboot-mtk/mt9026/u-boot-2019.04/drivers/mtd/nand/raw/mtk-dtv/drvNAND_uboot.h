// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#ifndef __UNFD_UBOOT_H__
#define __UNFD_UBOOT_H__

#include <common.h>
#include <linux/io.h>
#include "mtk-dtv-nand.h"

#define IF_IP_VERIFY				0 // [CAUTION]: to verify IP and HAL code, defaut 0
#if IF_IP_VERIFY
/* select a default NAND chip for IP_VERIFY or NAND_PROGRAMMER */
#define W29N04GV				1
//#define MT29F4G08ABAEA				1
#include "drvNAND_device.h"
#endif

#ifndef U64
#define U64	unsigned long long
#endif
#ifndef U32
#define U32	unsigned int
#endif
#ifndef U16
#define U16	unsigned short
#endif
#ifndef U8
#define U8	unsigned char
#endif
#ifndef S64
#define S64	signed long long
#endif
#ifndef S32
#define S32	signed long
#endif
#ifndef S16
#define S16	signed short
#endif
#ifndef S8
#define S8	signed char
#endif

#ifndef NULL
#define NULL (void*)0
#endif
#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#define REG_BANK_FCIE0				((unsigned long)pNandDrv->priv->fciebase)
#define REG_BANK_FCIE1				((unsigned long)pNandDrv->priv->fciebase + 0x200)
#define REG_BANK_FCIE2				((unsigned long)pNandDrv->priv->fciebase + 0x400)
#define REG_BANK_FCIE3				((unsigned long)pNandDrv->priv->fciebase + 0x600)
#define REG_BANK_CLK				((unsigned long)pNandDrv->priv->clk_info.clkbase)

#define REG(reg_addr)				readw(reg_addr)
#define REG_OFFSET_SHIFT_BITS			2
#define GET_REG_ADDR(x, y)			(x+((y)<<REG_OFFSET_SHIFT_BITS))

#define REG_WRITE_UINT16(reg_addr, val)		writew(val, reg_addr)
#define REG_READ_UINT16(reg_addr, val)		val = readw(reg_addr)
#define REG_SET_BITS_UINT16(reg_addr, val)	writew((readw(reg_addr)|(val)), reg_addr)
#define REG_CLR_BITS_UINT16(reg_addr, val)	writew((readw(reg_addr)&(~(val))), reg_addr)
#define REG_W1C_BITS_UINT16(reg_addr, val)	writew((readw(reg_addr)&(val)), reg_addr)

#define FCIE_REG_BASE_ADDR			GET_REG_ADDR(REG_BANK_FCIE0, 0x00)
#define FCIE_NC_CIFD_BASE			GET_REG_ADDR(REG_BANK_FCIE1, 0x00)
#define FCIE_NC_WBUF_CIFD_BASE			GET_REG_ADDR(REG_BANK_FCIE1, 0x00)
#define FCIE_NC_RBUF_CIFD_BASE			GET_REG_ADDR(REG_BANK_FCIE1, 0x20)
#define FCIE_POWEER_SAVE_MODE_BASE		GET_REG_ADDR(REG_BANK_FCIE2, 0x00)
#define FCIE_REG_TEE_BASE_ADDR			GET_REG_ADDR(REG_BANK_FCIE3, 0x00)
#define CONFIG_NAND_SECURITY_GEN 		2

#define REG_SW_ECC				NC_RX5F
#define TAG_SW_ECC				0xECC

#define REG50_ECC_CTRL_INIT_VALUE		0

#define UNFD_ST_PLAT				0x80000000

#include "drvNAND_reg_v5.h"

#define ENABLE_32BIT_MACRO			1
#define ENABLE_8BIT_MACRO			0

#define FCIE_LFSR				1

#define NC_SET_DDR_MODE()
#define NC_CLR_DDR_MODE()
#define NC_TEE_SET_DDR_MODE()
#define NC_TEE_CLR_DDR_MODE()

#define	NC_SDR_DEFAULT_TRR			7
#define	NC_SDR_DEFAULT_TCS			6
#define NC_SDR_DEFAULT_TWW			5
#define	NC_SDR_DEFAULT_TWHR			4
#define	NC_SDR_DEFAULT_TADL			8
#define	NC_SDR_DEFAULT_TCWAW			2
#define NC_SDR_DEFAULT_TCLHZ			4
#define	NC_SDR_DEFAULT_RX40CMD			4
#define	NC_SDR_DEFAULT_RX40ADR			5
#define	NC_SDR_DEFAULT_RX56			5

#define	NC_INST_DELAY				1
#define	NC_HWCMD_DELAY				1
#define	NC_TRR_TCS				1
#define	NC_TWHR_TCLHZ				1
#define	NC_TCWAW_TADL				1

#define NAND_PAD_BYPASS_MODE			1
#define NAND_PAD_TOGGLE_MODE			2
#define NAND_PAD_ONFI_SYNC_MODE			3

#define MACRO_TYPE_8BIT				1
#define MACRO_TYPE_32BIT			2

#define IF_FCIE_SHARE_PINS			0 // 1: need to nand_pads_switch at HAL's functions.
#define IF_FCIE_SHARE_CLK			0 // 1: need to nand_clock_setting at HAL's functions.
#define IF_FCIE_SHARE_IP			0

#define ENABLE_NAND_INTERRUPT_MODE		0
#define SPARE_DMA_ADDR_AUTO_INC			0 //spare would increase its address when dma

//=====================================================
#define ENABLE_NAND_POWER_SAVING_MODE		0

//=====================================================
// debug option
//=====================================================
#ifndef NAND_DEBUG_MSG
#define NAND_DEBUG_MSG				1
#endif

/* Define trace levels. */
#define UNFD_DEBUG_LEVEL_ERROR			(1)    /* Error condition debug messages. */
#define UNFD_DEBUG_LEVEL_WARNING		(2)    /* Warning condition debug messages. */
#define UNFD_DEBUG_LEVEL_HIGH			(3)    /* Debug messages (high debugging). */
#define UNFD_DEBUG_LEVEL_MEDIUM			(4)    /* Debug messages. */
#define UNFD_DEBUG_LEVEL_LOW			(5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef UNFD_DEBUG_LEVEL
#define UNFD_DEBUG_LEVEL			UNFD_DEBUG_LEVEL_WARNING
#endif

extern int Disable_Err_log;
#if defined(NAND_DEBUG_MSG) && NAND_DEBUG_MSG
#define nand_printf			printf
#define nand_debug(dbg_lv, tag, str, ...) \
	do { \
		if (dbg_lv > UNFD_DEBUG_LEVEL || Disable_Err_log == 1) \
 			break; \
		else { \
			if (dbg_lv == UNFD_DEBUG_LEVEL_ERROR) \
				nand_printf("NAND Err:"); \
			if (tag) \
				nand_printf("[%s]\t",__func__); \
			nand_printf(str, ##__VA_ARGS__); \
		} \
	} while(0)
#else /* NAND_DEBUG_MSG */
#define nand_printf(...)
#define nand_debug(enable, tag, str, ...) {}
#endif /* NAND_DEBUG_MSG */

#define nand_die() \
	do { \
		printf("\nUNFD Assert %s() Ln: %d", __FUNCTION__, __LINE__);	\
		while(1); \
	} while(0)

#define nand_stop() \
	while(1);

//=====================================================
// HW Timer for Delay
//=====================================================
#define HW_TIMER_DELAY_1us			1
#define HW_TIMER_DELAY_10us			10
#define HW_TIMER_DELAY_100us			100
#define HW_TIMER_DELAY_1ms			(1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms			(5    * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms			(10   * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms			(100  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms			(500  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s			(1000 * HW_TIMER_DELAY_1ms)

extern U32  nand_hw_timer_delay(U32 u32_us);

//=====================================================
// Pads Switch
//=====================================================
extern U32 nand_pads_switch(U32 u32EnableFCIE);

//================================================================================
// Set TEE Secure Range
//================================================================================
#define FCIE_WRITE_PROTECT_SUPPORT		0

#if defined(FCIE_WRITE_PROTECT_SUPPORT) && FCIE_WRITE_PROTECT_SUPPORT
#define RIU_PM_BASE				((unsigned long)0x1C000000)
#define REG_TZPC_FCIE_ADDR			(RIU_PM_BASE + (0x121900 << 1))

#define REG_FCIE_WP_START_ROW_ADDR_L		GET_REG_ADDR(REG_TZPC_FCIE_ADDR, 0x60)
#define REG_FCIE_WP_START_ROW_ADDR_H		GET_REG_ADDR(REG_TZPC_FCIE_ADDR, 0x61)
#define REG_FCIE_WP_END_ROW_ADDR_L		GET_REG_ADDR(REG_TZPC_FCIE_ADDR, 0x62)
#define REG_FCIE_WP_END_ROW_ADDR_H		GET_REG_ADDR(REG_TZPC_FCIE_ADDR, 0x63)

#define REG_FCIE_WP_ENABLE			GET_REG_ADDR(REG_TZPC_FCIE_ADDR, 0x64)
#define BIT_FCIE_WP_ENABLE			BIT0
#endif

//=====================================================
// set FCIE clock
//=====================================================
#define DECIDE_CLOCK_BY_NAND			1

#define DUTY_CYCLE_PATCH			0 // 1: to enlarge low width for tREA's worst case of 25ns
#if DUTY_CYCLE_PATCH
#define FCIE_REG41_VAL				((2<<9)|(2<<3)) // RE,WR pulse, Low:High=3:1
#define REG57_ECO_FIX_INIT_VALUE		0
#else
#define FCIE_REG41_VAL				0 // RE,WR pulse, Low:High=1:1
#ifdef CONFIG_HAPS
#define REG57_ECO_FIX_INIT_VALUE		(BIT_NC_LATCH_DATA_3_0_T|BIT_NC_LATCH_STS_2_0_T)
#else
#define REG57_ECO_FIX_INIT_VALUE		(BIT_NC_LATCH_DATA_2_0_T|BIT_NC_LATCH_STS_1_0_T)
#endif
#endif
#define FCIE_SW_SLOWEST_CLK			0

#define NAND_SEQ_ACC_TIME_TOL			16 //in unit of ns

#define reg_ckg_fcie				GET_REG_ADDR(REG_BANK_CLK, 0x00)
#define FCIE_CLK_GATING				(BIT0)
#define FCIE_CLK_INVERSE			(BIT1)

//=====================================================
// set ECC clock
//=====================================================
#define ECC_CLK_GATING				(BIT0)
#define ECC_CLK_INVERSE				(BIT1)
#define ECC_CLK_MASK				(BIT4|BIT3|BIT2)
#define ECC_CLK_12M				(0<<2)
#define ECC_CLK_54M				(1<<2)
#define ECC_CLK_108M				(2<<2)
#define ECC_CLK_160M				(3<<2)
#define ECC_CLK_216M				(4<<2)
#define ECC_SW_DEFAULT_CLK			ECC_CLK_216M

//=====================================================
// set fcie sync clock
//=====================================================
#define NFIE_CLK_SYNC_MASK			(BIT1|BIT0)
#define NFIE_CLK_SYNC_216M			(0<<0)
#define NFIE_CLK_SYNC_432M			(1<<0)
#define NFIE_CLK_SYNC_DEFAULT			NFIE_CLK_SYNC_432M

//=====================================================
extern U32  nand_clock_setting(U32 u32ClkParam);
extern void nand_DumpPadClk(void);

//=====================================================
// transfer DMA Address
//=====================================================
/*
 * Important:
 * The following buffers should be large enough for a whole NAND block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR				0x30000000
#define DMA_R_ADDR				0x30100000
#define DMA_W_SPARE_ADDR			0x30200000
#define DMA_R_SPARE_ADDR			0x30300000
#define DMA_BAD_BLK_BUF				0x30400000

#define DMA_W_SRAM_ADDR				0x1A000000
#define DMA_R_SRAM_ADDR				0x1A000800
#define DMA_W_SRAM_SPARE_ADDR			0x1A001000
#define DMA_R_SRAM_SPARE_ADDR			0x1A001800

//=====================================================
// misc
//=====================================================
#define UBOOT_TAG				0x82

#define PAGE_DATA_BUF_SIZE			(32*1024)
#define PAGE_SPARE_BUF_SIZE			(4*1024)

#endif //__UNFD_UBOOT_H__

