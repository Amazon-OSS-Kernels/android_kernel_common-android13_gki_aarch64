// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#define HEADER_START   0
#define HEADER_LEN     0x110
#define LUT_START      0x110
#define CSOT_CSOT_DATA_BIT_NUM 12

typedef enum
{
    E_MONO_MODE  = 0,
    E_RGB_MODE   = 1,
} E_COLOR_MODE;

typedef enum
{
    E_FILE_TATOL_CRC = 0,
    E_FILE_HEADER_CRC,
    E_DEMURA_PARA_CRC,
    E_DEMURA_LUT_CRC,
} E_FILE_CRC;

typedef struct
{
    MS_U8           DEMURA_PLANE_NUM;
    MS_BOOL         DEMURA_MODE;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U32          TABLE_CRC;
    MS_U16          DEMURA_BLACK_LIMIT;
    MS_U16          DEMURA_PLANE_LEVEL1;
    MS_U16          DEMURA_PLANE_LEVEL2;
    MS_U16          DEMURA_PLANE_LEVEL3;
    MS_U16          DEMURA_PLANE_LEVEL4;
    MS_U16          DEMURA_PLANE_LEVEL5;
    MS_U16          DEMURA_PLANE_LEVEL6;
    MS_U16          DEMURA_PLANE_LEVEL7;
    MS_U16          DEMURA_PLANE_LEVEL8;
    MS_U16          DEMURA_WHITE_LIMIT;
    MS_U16          DEMURA_GAIN_R;
    MS_U16          DEMURA_GAIN_G;
    MS_U16          DEMURA_GAIN_B;
    MS_U16          DEMURA_OFFSET_R;
    MS_U16          DEMURA_OFFSET_G;
    MS_U16          DEMURA_OFFSET_B;
    MS_U8           DEMURA_INT_BIT_WIDTH;
    MS_U8           DEMURA_DEC_BIT_WIDTH;
    // Header Buffer
    MS_U8           *hdr_buffer;
    // Lut Buffer
    MS_U8           *lut_buffer;
    MS_U32          lut_offset;
    MS_U32          lut_length;
}CSOT_CSOT_Demura_Header;

MS_U16 get_CSOT_CSOT_CRC(E_FILE_CRC eCRCType);
MS_BOOL load_vendor_header(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL parse_vendor_header(CSOT_CSOT_Demura_Header *phdr);
void dump_vendor_header(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL load_vendor_lut(CSOT_CSOT_Demura_Header *phdr);

#endif
