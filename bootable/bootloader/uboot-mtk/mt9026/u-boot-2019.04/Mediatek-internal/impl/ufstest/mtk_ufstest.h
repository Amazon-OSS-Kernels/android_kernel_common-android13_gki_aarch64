/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MTK_UFSTEST_H
#define _MTK_UFSTEST_H

u32 ufstest_speed(u8* u8_DataBuf, u32 u32_BlkAddr);
u32 ufstest_pwrcut_initdata(u8* u8_DataBuf, u32 u32_BlkStartAddr);
u32 ufstest_pwrcut_test(u8* u8_DataBuf, u32 u32_BlkStartAddr, u8* u8_ReadDataBuf);

#endif