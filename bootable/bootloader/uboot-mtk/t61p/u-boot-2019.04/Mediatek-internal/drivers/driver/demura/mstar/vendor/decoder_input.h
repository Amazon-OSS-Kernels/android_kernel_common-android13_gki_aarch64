// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef  _DECODER_INPUT_H_
#define  _DECODER_INPUT_H_

#include "apiDemura.h"

#if (CONFIG_DEMURA_VENDOR_MULTI_EN)
MS_BOOL Decode_To_Mstar_Format_AUO(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_LGD(void *pDataInfo, BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor);
MS_BOOL Decode_To_Mstar_Format_NOVA(void *pDataInfo, BinOutputInfo *pbin_info, EN_DEMURA_MULTI_VENDOR multi_vendor, MS_U32 flash_offset);
MS_BOOL Decode_To_Mstar_Format_CSOT_HI_SILICON(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_CSOT_HIMAX(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_SDC(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_INX(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_CSOT_CSOT(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_BOE_ESWIN(void *pDataInfo, BinOutputInfo *pbin_info);
MS_BOOL Decode_To_Mstar_Format_SIO(void *pDataInfo, BinOutputInfo *pbin_info);
#else
MS_BOOL Decode_To_Mstar_Format(void *pDataInfo, BinOutputInfo *pbin_info);
#endif

#endif
