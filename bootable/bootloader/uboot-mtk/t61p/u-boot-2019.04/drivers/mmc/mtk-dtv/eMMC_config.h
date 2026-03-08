/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef __eMMC_CONFIG_H__
#define __eMMC_CONFIG_H__

#define UNIFIED_eMMC_DRIVER     1
#include <common.h>
//=====================================================
// select a HW platform:
//   - 1: enable, 0: disable.
//   - only one platform can be 1, others have to be 0.
//   - search and check all [FIXME] if need modify or not
//=====================================================

#ifndef U64
#define U64  unsigned long long
#endif
#ifndef U32
#define U32  unsigned int
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S64
#define S64  signed long long
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif


//=====================================================
// do NOT edit the following content.
//=====================================================
#include "eMMC_uboot.h"
#include "mtk-dtv-mmc.h"

//=====================================================
// misc. do NOT edit the following content.
//=====================================================
#define eMMC_DMA_RACING_PATCH           1
#define eMMC_DMA_PATCH_WAIT_TIME        HW_TIMER_DELAY_10ms
#define eMMC_DMA_RACING_PATTERN0        (((U32)'M'<<24)|((U32)0<<16)|((U32)'S'<<8)|(U32)1)
#define eMMC_DMA_RACING_PATTERN1        (((U32)'T'<<24)|((U32)6<<16)|((U32)'A'<<8)|(U32)8)

//===========================================================
// Time Dalay, do NOT edit the following content
//===========================================================
#if defined(EMMC_UPDATE_FIRMWARE) && (EMMC_UPDATE_FIRMWARE)
#define TIME_WAIT_DAT0_HIGH            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RESET           (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT  (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FIFOCLK_RDY          (HW_TIMER_DELAY_10ms*10)
#define TIME_WAIT_CMDRSP_END           (HW_TIMER_DELAY_10ms*10)
#define TIME_WAIT_1_BLK_END            (HW_TIMER_DELAY_1s*5)
#define TIME_WAIT_n_BLK_END            (HW_TIMER_DELAY_1s*10) // safe for 512 blocks
#else
#define TIME_WAIT_DAT0_HIGH            (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_ERASE_DAT0_HIGH      (HW_TIMER_DELAY_1s*10)
#define TIME_WAIT_FCIE_RESET           HW_TIMER_DELAY_500ms
#define TIME_WAIT_FCIE_RST_TOGGLE_CNT  HW_TIMER_DELAY_1us
#define TIME_WAIT_FIFOCLK_RDY          HW_TIMER_DELAY_10ms
#define TIME_WAIT_CMDRSP_END           HW_TIMER_DELAY_10ms
#define TIME_WAIT_R_1_BLK_END          HW_TIMER_DELAY_100ms
#define TIME_WAIT_1_BLK_END            (HW_TIMER_DELAY_1s*1)
#define TIME_WAIT_n_BLK_END            (HW_TIMER_DELAY_1s*2) // safe for 512 blocks
#endif

#endif /* __eMMC_CONFIG_H__ */
