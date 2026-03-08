// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

typedef struct
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned high     : 4;
    unsigned reserve  : 4;
#else /* __BIG_ENDIAN */
    unsigned reserve  : 4;
    unsigned high     : 4;
#endif
    unsigned char low;

} __attribute__((packed)) AUO_BIN_Panel_Level;


typedef struct
{
    MS_U8           PARAMETER_CRC_H;
    MS_U8           PARAMETER_CRC_L;
    MS_U8           dummy0;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U8           DEMURA_TBL_H_H;
    MS_U8           DEMURA_TBL_H_L;
    MS_U8           DEMURA_TBL_V_H;
    MS_U8           DEMURA_TBL_V_L;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U8           dummy1[10];
    MS_U8           LOWER_BOUND_H;
    MS_U8           LOWER_BOUND_L;
    MS_U8           UPPER_BOUND_H;
    MS_U8           UPPER_BOUND_L;
    MS_U8           dummy2[12];
    MS_U8           PLANE00_LV_H;
    MS_U8           PLANE00_LV_L;
    MS_U8           PLANE01_LV_H;
    MS_U8           PLANE01_LV_L;
    MS_U8           PLANE02_LV_H;
    MS_U8           PLANE02_LV_L;
    MS_U8           PLANE03_LV_H;
    MS_U8           PLANE03_LV_L;
    MS_U8           PLANE04_LV_H;
    MS_U8           PLANE04_LV_L;
    MS_U8           PLANE05_LV_H;
    MS_U8           PLANE05_LV_L;
    MS_U8           PLANE06_LV_H;
    MS_U8           PLANE06_LV_L;
    MS_U8           PLANE07_LV_H;
    MS_U8           PLANE07_LV_L;
    MS_U8           dummy3[14];
} __attribute__((packed)) CHOT_BIN_Header;

#endif  // _PARSE_HEADER_H_
