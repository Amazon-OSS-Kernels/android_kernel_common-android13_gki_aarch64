// SPDX-License-Identifier: GPL-2.0+
/*
 * MTK DTV NAND Flash controller driver.
 * Copyright (C) 2021 MediaTek Inc.
 * Authors:	Edward-CH Lee		<edward-ch.lee@mediatek.com>
 */

#ifndef __UNFD_CONFIG_H__
#define __UNFD_CONFIG_H__

#include "drvNAND_uboot.h"

//========================================================================
// Time Dalay, do NOT edit the following content, for NC_WaitComplete use.
//========================================================================
#define DELAY_100us_in_us	100
#define DELAY_500us_in_us	500
#define DELAY_1ms_in_us		1000
#define DELAY_10ms_in_us	10000
#define DELAY_100ms_in_us	100000
#define DELAY_500ms_in_us	500000
#define DELAY_1s_in_us		1000000

#define WAIT_ERASE_TIME		(DELAY_1s_in_us<<2)
#define WAIT_WRITE_TIME		(DELAY_1s_in_us<<2)
#define WAIT_READ_TIME		(DELAY_1ms_in_us<<10)
#define WAIT_PROBE_TIME		(DELAY_100us_in_us<<2)
#define WAIT_RESET_TIME		(WAIT_ERASE_TIME)

//for system cache direction calls
#define WRITE_TO_NAND		0
#define READ_FROM_NAND		1
#define BIDIRECTIONAL		2

#define NANDINFO_BYTE_CNT	512
#define PARTINFO_BYTE_CNT	512
#define MAX_PARTINFO_CNT	128
#define CIS_AREA_BLK_CNT	10
#define CIS_BLK_CNT		2

extern U32 nand_clock_setting(U32 u32ClkParam);
extern U32 nand_config_clock(void);
extern dma_addr_t nand_translate_DMA_address_Ex(unsigned long ulong_DMAAddr, U32 u32_ByteCnt,
	int Mode);
extern dma_addr_t nand_translate_Spare_DMA_address_Ex(unsigned long ulong_DMAAddr,
	U32 u32_ByteCnt, int Mode);
extern dma_addr_t nand_translate_DMA_MIUSel(unsigned long ulong_DMAAddr, U32 u32_ByteCnt);
extern void nand_read_dma_post_flush(unsigned long ulong_DMAAddr, U32 u32_ByteCnt);

#endif /* __UNFD_CONFIG_H__ */
