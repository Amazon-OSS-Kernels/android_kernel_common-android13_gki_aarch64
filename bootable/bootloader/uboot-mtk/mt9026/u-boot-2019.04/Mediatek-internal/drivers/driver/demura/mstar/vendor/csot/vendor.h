// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#define CSOT_DEMURA_DAT_BIT_LEN  8

typedef enum
{
    E_BLOCK_SIZE_8_8     = 0,
    E_BLOCK_SIZE_16_16   = 1,
    E_BLOCK_SIZE_UNKOWN  = 2,
}DEMURA_BLOCK_SIZE;

typedef enum
{
    E_SHARE_RGB      = 0,
    E_SEPERATE_RGB   = 1,
}DEMURA_SEPERATE_TYPE;

typedef struct
{
    MS_U32    blk_type;
    MS_U32    VNode;
    MS_U32    HNode;
    MS_U32    sep_type;
    MS_U32    LevelCount;
    MS_U32    Layer[6];
}CSOT_Demura_Header;

#endif