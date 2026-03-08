// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _BITS_STREAM_READER_H_
#define _BITS_STREAM_READER_H_

typedef struct _BitsStreamReader
{
    MS_U32  *pdata;
    MS_U32  total_bytes;
    MS_U32  total_bits;
    MS_U32  cur_bit_pos;
}BitsStreamReader;

int    Init_BSReader(BitsStreamReader * pBSR, MS_U32 *pdata, MS_U32 size_in_bytes);
MS_U32 BSR_ReadData (BitsStreamReader * pBSR, MS_U8  bit_num);
void   BSR_ShiftBits(BitsStreamReader * pBSR, MS_U32 bit_num);

#endif
