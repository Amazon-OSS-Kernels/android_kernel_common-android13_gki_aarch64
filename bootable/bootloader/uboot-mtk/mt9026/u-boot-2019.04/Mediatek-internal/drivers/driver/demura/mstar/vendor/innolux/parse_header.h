// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

MS_U8 inx_crc8(MS_U8 *data, MS_U8 seed, MS_U32 crcStart, MS_U32 crcLength);
MS_BOOL parse_vendor_header(MS_U8 *buf, INX_Demura_Header *phdr);
MS_BOOL fetch_decode_info(INX_Demura_Header *phdr, Vendor2Mstar_Info *pv2m);

#endif
