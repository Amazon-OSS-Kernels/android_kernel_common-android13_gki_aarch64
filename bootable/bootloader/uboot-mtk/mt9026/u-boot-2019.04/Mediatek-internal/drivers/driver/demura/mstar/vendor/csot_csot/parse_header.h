// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _PARSE_HEADER_H_
#define _PARSE_HEADER_H_

typedef struct
{
    //File header
    MS_U8           u32MagicNum[4];
    MS_U8           name[14];
    MS_U8           version[2];
    MS_U8           sectionNum[2];
    MS_U8           u16Reseved_0[2];
    MS_U8           all_file_size[4];
    MS_U8           file_totoal_CRC[2];
    MS_U8           file_header_CRC[2]; //offset ox1e
    //section header1
    MS_U8           type_1[4];
    MS_U8           header_offser[4];
    MS_U8           header_size[4];
    MS_U8           header_section_CRC[2];
    MS_U8           u16Reseved_1[2]; //offset 0x2e
    //section header2
    MS_U8           type_2[4];
    MS_U8           lut_offse[4];
    MS_U8           lut_size[4];
    MS_U8           lut_section_CRC[2];
    MS_U8           u16Reseved_2[2]; //offset 0x3e
    //demura parameter
    MS_U8           enDemuraMode; //offset 0x40
    MS_U8           u8PlaneNum;
    MS_U8           u8HblockSize;
    MS_U8           u8VblockSize;
    MS_U8           u16HLutNum[2];
    MS_U8           u16VLutNum[2];
    MS_U8           u8IntBitWidth;
    MS_U8           u8DecimalsBitWidth;
    MS_U8           u8TwoChipEn;
    MS_U8           u8Reseved_3;
    MS_U8           u16CompValGainR[2];
    MS_U8           u16CompValGainG[2];
    MS_U8           u16CompValGainB[2];
    MS_U8           u8Reseved_4[2];
    MS_U8           u16CompValOffsetR[2];
    MS_U8           u16CompValOffsetG[2];
    MS_U8           u16CompValOffsetB[2];
    MS_U8           u16Reseved_5[2];
    MS_U8           u16BlackLimitR[2];
    MS_U8           u16BlackLimitG[2];
    MS_U8           u16BlackLimitB[2];
    MS_U8           u16Reseved_6[2];
    MS_U8           u16WhiteLimitR[2];
    MS_U8           u16WhiteLimitG[2];
    MS_U8           u16WhiteLimitB[2];
    MS_U8           u16Reseved_7[2]; //offset 0x6a
    MS_U8           u16Plane1LevelR[2]; //offset 0x6b
    MS_U8           u16Plane2LevelR[2];
    MS_U8           u16Plane3LevelR[2];
    MS_U8           u16Plane4LevelR[2];
    MS_U8           u16Plane5LevelR[2];
    MS_U8           u16Plane6LevelR[2];
    MS_U8           u16Plane7LevelR[2];
    MS_U8           u16Plane8LevelR[2];
    MS_U8           u16Plane9LevelR[2];
    MS_U8           u16Plane1LevelG[2];
    MS_U8           u16Plane2LevelG[2];
    MS_U8           u16Plane3LevelG[2];
    MS_U8           u16Plane4LevelG[2];
    MS_U8           u16Plane5LevelG[2];
    MS_U8           u16Plane6LevelG[2];
    MS_U8           u16Plane7LevelG[2];
    MS_U8           u16Plane8LevelG[2];
    MS_U8           u16Plane9LevelG[2];
    MS_U8           u16Plane1LevelB[2];
    MS_U8           u16Plane2LevelB[2];
    MS_U8           u16Plane3LevelB[2];
    MS_U8           u16Plane4LevelB[2];
    MS_U8           u16Plane5LevelB[2];
    MS_U8           u16Plane6LevelB[2];
    MS_U8           u16Plane7LevelB[2];
    MS_U8           u16Plane8LevelB[2];
    MS_U8           u16Plane9LevelB[2];
    MS_U8           u16Reseved_8[18];
    MS_U8           u16PlaneB1SlopeR[2]; //offset 0xb4 //65535/(u16Plane1LevelR - u16BlackLimitR)
    MS_U8           u16Plane12SlopeR[2];
    MS_U8           u16Plane23SlopeR[2];
    MS_U8           u16Plane34SlopeR[2];
    MS_U8           u16Plane45SlopeR[2];
    MS_U8           u16Plane56SlopeR[2];
    MS_U8           u16Plane67SlopeR[2];
    MS_U8           u16Plane78SlopeR[2];
    MS_U8           u16Plane89SlopeR[2];
    MS_U8           u16Plane9WSlopeR[2];
    MS_U8           u16PlaneB1SlopeG[2];
    MS_U8           u16Plane12SlopeG[2];
    MS_U8           u16Plane23SlopeG[2];
    MS_U8           u16Plane34SlopeG[2];
    MS_U8           u16Plane45SlopeG[2];
    MS_U8           u16Plane56SlopeG[2];
    MS_U8           u16Plane67SlopeG[2];
    MS_U8           u16Plane78SlopeG[2];
    MS_U8           u16Plane89SlopeG[2];
    MS_U8           u16Plane9WSlopeG[2];
    MS_U8           u16PlaneB1SlopeB[2];
    MS_U8           u16Plane12SlopeB[2];
    MS_U8           u16Plane23SlopeB[2];
    MS_U8           u16Plane34SlopeB[2];
    MS_U8           u16Plane45SlopeB[2];
    MS_U8           u16Plane56SlopeB[2];
    MS_U8           u16Plane67SlopeB[2];
    MS_U8           u16Plane78SlopeB[2];
    MS_U8           u16Plane89SlopeB[2];
    MS_U8           u16Plane9WSlopeB[2]; //offset 0xee
    MS_U8           u16Reseved_9[32];
    //demura lut offset 0x110 to end
}__attribute__((packed)) CSOT_CSOT_BIN_Header;

#endif  // _PARSE_HEADER_H_
