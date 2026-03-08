// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _CONVERT_ENTRY_H_
#define _CONVERT_ENTRY_H_

#include "apiDemura.h"

#if defined(CONFIG_DEMURA_VENDOR_MULTI)
int do_demura_convert(BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor);
MS_BOOL If_Need_Decode_AUO(void);
MS_BOOL If_Need_Decode_NOVA(MS_U32 flash_offset);
MS_BOOL If_Need_Decode_CSOT_HI_SILICON(void);
MS_BOOL If_Need_Decode_CSOT_HIMAX(void);
MS_BOOL If_Need_Decode_SDC(void);
MS_BOOL If_Need_Decode_LGD(EN_DEMURA_MULTI_VENDOR multi_vendor);
MS_BOOL If_Need_Decode_INX(void);
MS_BOOL If_Need_Decode_CSOT_CSOT(void);
MS_BOOL Check_MuraTable_INX(void);
#else
int do_demura_convert(BinOutputInfo *pbin_info);
MS_BOOL If_Need_Decode(void);
MS_BOOL Check_MuraTable(void);
#endif

int do_demura_merge(BinOutputInfo *bin_info, BinOutputInfo *bin_backlight_info, MS_U16 layer_bound);

MS_BOOL init_demura_heap(void);

MS_BOOL Alloc_LutIn_Space(void *pdat_info, BinOutputInfo *pbin_info);
void set_demura_version(int version);
void set_demura_pnl(MS_U16 width, MS_U16 height);


MS_BOOL If_Need_Decode_BOE_ESWIN(void);
MS_BOOL If_Need_Decode_SIO(void);

#endif
