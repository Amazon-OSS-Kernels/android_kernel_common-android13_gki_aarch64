// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _VENDOR_H_
#define _VENDOR_H_

#define INX_INDENTIFICATION      0x99
#define INX_BYTE_ORDER          __BIG_ENDIAN
#define INX_CRC8_SEED           0x0FF
#define INX_DEMURA_DL_FLOW 1

#if (INX_BYTE_ORDER == __BIG_ENDIAN)
#define inx32_to_cpu(x)         __be32_to_cpu(x)
#define inx16_to_cpu(x)         __be16_to_cpu(x)
#elif (INX_BYTE_ORDER == __LITTLE_ENDIAN)
#define inx32_to_cpu(x)         __le32_to_cpu(x)
#define inx16_to_cpu(x)         __le16_to_cpu(x)
#endif

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

typedef struct
{
    MS_U8  type[2];         // Mstar Only care : LUT1
    MS_U16 data_checksum;   // INX_CRC8
    MS_U32 data_start;
    MS_U32 data_length;
} __attribute__((packed)) INX_Block_INFO;


typedef struct
{
    // (0x80 ~ 0xE0) Block9 - Block16
    INX_Block_INFO block[8];

    // (0xE0 ~ 0xFE)
    MS_U8  reserved[30];

    // (0xFE ~ 0x100) The Sum of whole Header data (0x80~0xFD)
    MS_U16 header_checksum;

} __attribute__((packed)) _INX_Extend_Header;


typedef struct
{
    // (0x00 ~ 0x01) 0x99
    MS_U8  magic_id;

    // (0x01 ~ 0x06)
    MS_U8  asic[5];

    // (0x06 ~ 0x07) Header Version
    MS_U8  version;

    // (0x07 ~ 0x08) INX : 0x01, NVT : 0x02, Mstar : 0x03
    MS_U8  algorithm;

    // (0x08 ~ 0x0C) 0x20140225 ==> 2014/02/25
    MS_U32 date;

    // (0x0C ~ 0x0D) How many Chip for this system?  1-chip: 0x01, 2-chip: 0x02
    MS_U8  chip_st;

    // (0x0D ~ 0x0E) BIT0 ==> 1: 2D Demura On; BIT1 ==> 1: 3D Demura On; BIT2~BIT7 ==> Reserved for future using
    MS_U8  function_define;

    // (0x0E ~ 0x0F) How many Blocks for Demura Data? 1 block: 0x01, 2 block: 0x02
    MS_U8  block_st;

    // (0x0F ~ 0x10) 0x00 ==> no Header Extension Block, Total Size = 128B
    //               0x01 ==> 1 Header Extension Block, Total Size = 256B
    MS_U8  extension_block;

    // (0x10 ~ 0x70) Block1 - Block8
    INX_Block_INFO block[8];

    // (0x70 ~ 0x71) 0~255, Compensation lower boundary.
    MS_U8  bot_limit;

    // (0x71 ~ 0x72) 0~255, Compensation upper boundary.
    MS_U8  top_limit;

    // (0x72 ~ 0x77) 0~255, Optimized compensated gray 1-5
    //  If it is unused, the value will be the same as Upper Limit
    MS_U8  gray_level[5];

    // (0x77 ~ 0x7B) Reserved for Information Block at Header
    MS_U8  header_info[5];

    // (0x7B ~ 0x7E) The Sum of whole LUT data
    MS_U16 lut_checksum;

    // (0x7E ~ 0x80) The Sum of whole Header data (0x00~0x7D)
    MS_U16 header_checksum;

    // (0x80 ~ 0x100) Extension Block header
    _INX_Extend_Header extend;

} __attribute__((packed)) INX_Demura_Header;


typedef struct
{
    MS_U32 layer_count;
    MS_U32 layer_levels[8];
    MS_U32 top_limit;
    MS_U32 bot_limit;
    MS_U32 hblock_size;
    MS_U32 vblock_size;
    MS_U32 hblock_num;
    MS_U32 vblock_num;

    MS_U32 data_start;
    MS_U32 data_length;
    MS_U32 data_checksum;
    int    **LutIn[8];
    MS_U32 *input_buf;   // Using to load compress data, align to 4Byte

    MS_U32 build_date;

    #if(INX_DEMURA_DL_FLOW)
    MS_U8  function_define;
    #endif
} Vendor2Mstar_Info;

#endif
