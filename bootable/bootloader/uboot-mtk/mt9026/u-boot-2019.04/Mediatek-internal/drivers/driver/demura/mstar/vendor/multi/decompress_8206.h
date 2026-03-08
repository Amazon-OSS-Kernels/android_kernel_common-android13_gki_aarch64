// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _DECOMPRESS_H_
#define _DECOMPRESS_H_

typedef struct
{
    int  Height;
    int  Width;
    int  Width16;
    int  Width256;
    int  CodeBookSize;

    MS_U32 *ByteCmpxIn;   // For efficiency, Define in 4Byte Align
    MS_U32 CmpxSize;
    int  **LutOut[5];
} DeCmpxInfo;

MS_BOOL DeCmpx(DeCmpxInfo *pDeCmpx);

#endif
