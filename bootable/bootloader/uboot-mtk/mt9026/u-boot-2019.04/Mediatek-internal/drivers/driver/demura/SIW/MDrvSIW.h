// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MDRV_SIW_H_
#define _MDRV_SIW_H_

#include <stdio.h>
#include <common.h>
#include <debug_impl.h>

#define DBG_SIW_DEMURA (0)
#define TEST_SIW_DEMURA (0)

#if (DBG_SIW_DEMURA == 1)
#define DEBUG_SIW_DEMURA(msg...) UBOOT_TRACE(msg)
#else
#define DEBUG_SIW_DEMURA(msg...)
#endif

void MDrv_SIW_Demura_AutoDownload_Setup(MS_PHYADDR phy_addr, MS_BOOL bEnable);
void MDrv_SIW_Demura_SPOC_Reg(MS_U8 *pReg);
MS_BOOL MDrv_SIW_Demura_Checksum_CRC16(MS_U8 *lut_buf, MS_U32 lut_len);
void MDrv_SIW_Demura_RD_Done(MS_BOOL bDone);

#endif  //_MDRV_SIW_H_
