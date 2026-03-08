#ifndef _VENDOR_H_
#define _VENDOR_H_

#include "vendor_magic.h"
// Start INX
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
// End INX

// Start CSOT_Himax
#define CSOT_HIMAX_HEADER_START   0
#define CSOT_HIMAX_HEADER_LEN     0x200
#define CSOT_HIMAX_LUT_START      0x200
#define CSOT_HIMAX_LUT_LEN        590238

#define CSOT_HIMAX_LUT_CHECKSUM_H        0x99
#define CSOT_HIMAX_LUT_CHECKSUM_L        0x9A
#define CSOT_HIMAX_HEADER_CHECKSUM_H        0x19B
#define CSOT_HIMAX_HEADER_CHECKSUM_L        0x19C

#define CSOT_HIMAX_USE_PLANE_NUM           3
#define CSOT_HIMAX_MAX_PLANE_NUM           4
// End

#define CSOT_CSOT_HEADER_START   0
#define CSOT_CSOT_HEADER_LEN     0x110
#define CSOT_CSOT_LUT_START      0x110
#define CSOT_CSOT_DATA_BIT_NUM 12

// Start SDC
#define FORCE_TO_4_LAYER 1
#define TEST_BIN       0
// Test.bin offset = 0, real flash data offset 0x50000
#if(TEST_BIN)
#define DATA_OFFSET    0x0
#else
#define DATA_OFFSET    0x50000
#endif
#define SDC_HEADER_START   DATA_OFFSET
#define SDC_HEADER_LEN     128
#define SDC_LUT_START      DATA_OFFSET + 0x1000
#define SDC_LUT_LEN        912464

#define SDC_HEADER_CHECKSUM     SDC_HEADER_START + SDC_HEADER_LEN
#define SDC_LUT_CHECKSUM        SDC_LUT_START + SDC_LUT_LEN

#define SDC_MAX_PLANE_NUM           7
//End

// Start CSOT_hisilicon
#define CSOT_HISI_HEADER_START   0
#define CSOT_HISI_HEADER_LEN     0x24
#define CSOT_HISI_LUT_START      0x24
#define CSOT_HISI_LUT_LEN        0x9B9D0

#define CSOT_HISI_DMC_LUT_CHECKSUM        CSOT_HISI_HEADER_START + 0x20

#define CSOT_HISI_MAX_PLANE_NUM           5
// End

// Start CSOT vendor.h
#define HEADER_START   (CHOT_HEADER_CHECKSUM)
#define HEADER_LEN     (CHOT_HEADER_END - CHOT_HEADER_CHECKSUM + 1)

#define CHOT_HEADER_CHECKSUM     0x4
#define CHOT_HEADER_START        0x6
#define CHOT_HEADER_END          0x3F

#define CHOT_LUT_CHECKSUM        0x40
#define CHOT_LUT_START           0x42

#define MAX_PLANE_NUM           8
// End

// Start LGD vendor.h
#define FCIC_CHECKSUM_SIZE       4
#define FCIC_CRC_SIZE            4
#define FCIC_EXTRA_SIZE          (FCIC_CHECKSUM_SIZE + FCIC_CRC_SIZE)

#define LGD_MURA_START_ADR       0x69000      // Just for filename test
#define LGD_MURA_DAT_OFFSET      0x00004
#define LGD_CHECKSUM_ADR         0x2662f4

#define ID_CUS_LGD               0x0800

#define SAMPLE_DAT_START         1024     // Byte
#define SAMPLE_DAT_LEN           4096     // Byte
// End

#define DMC_HEADER_CHECKSUM     65536
#define DMC_HEADER_START        65538
#define DMC_HEADER_END          65694

#define DMC_LUT_CHECKSUM        131072
#define DMC_LUT_START           131074

#if CONFIG_TARGET_MT5896

typedef struct
{   //Align to machli_reg_sc_0406.xls
    demura_reg reg_dmc_black_limit;            // 12bit
    demura_reg reg_dmc_plane_level1;           // 12bit
    demura_reg reg_dmc_plane_level2;           // 12bit
    demura_reg reg_dmc_plane_level3;           // 12bit
    demura_reg reg_dmc_plane_level4;           // 12bit
    demura_reg reg_dmc_plane_level5;           // 12bit
    demura_reg reg_dmc_plane_level6;           // 12bit
    demura_reg reg_dmc_plane_level7;           // 12bit
    demura_reg reg_dmc_plane_level8;           // 12bit
    demura_reg reg_dmc_white_limit;            // 12bit
    demura_reg reg_dmc_data_r_mag4;            // 3 bit
    demura_reg reg_dmc_data_r_mag3;            // 3 bit
    demura_reg reg_dmc_data_r_mag2;            // 3 bit
    demura_reg reg_dmc_data_r_mag1;            // 3 bit
    demura_reg reg_dmc_data_r_mag8;            // 3 bit
    demura_reg reg_dmc_data_r_mag7;            // 3 bit
    demura_reg reg_dmc_data_r_mag6;            // 3 bit
    demura_reg reg_dmc_data_r_mag5;            // 3 bit
    demura_reg reg_dmc_data_g_mag4;            // 3 bit
    demura_reg reg_dmc_data_g_mag3;            // 3 bit
    demura_reg reg_dmc_data_g_mag2;            // 3 bit
    demura_reg reg_dmc_data_g_mag1;            // 3 bit
    demura_reg reg_dmc_data_g_mag8;            // 3 bit
    demura_reg reg_dmc_data_g_mag7;            // 3 bit
    demura_reg reg_dmc_data_g_mag6;            // 3 bit
    demura_reg reg_dmc_data_g_mag5;            // 3 bit
    demura_reg reg_dmc_data_b_mag4;            // 3 bit
    demura_reg reg_dmc_data_b_mag3;            // 3 bit
    demura_reg reg_dmc_data_b_mag2;            // 3 bit
    demura_reg reg_dmc_data_b_mag1;            // 3 bit
    demura_reg reg_dmc_data_b_mag8;            // 3 bit
    demura_reg reg_dmc_data_b_mag7;            // 3 bit
    demura_reg reg_dmc_data_b_mag6;            // 3 bit
    demura_reg reg_dmc_data_b_mag5;            // 3 bit
    demura_reg reg_dmc_plane_b1_coef;          // 14bit
    demura_reg reg_dmc_plane_12_coef;          // 14bit
    demura_reg reg_dmc_plane_23_coef;          // 14bit
    demura_reg reg_dmc_plane_34_coef;          // 14bit
    demura_reg reg_dmc_plane_45_coef;          // 14bit
    demura_reg reg_dmc_plane_56_coef;          // 14bit
    demura_reg reg_dmc_plane_67_coef;          // 14bit
    demura_reg reg_dmc_plane_78_coef;          // 14bit
    demura_reg reg_dmc_plane_9w_coef;          // 14bit
    demura_reg reg_dmc_plane_9w_ks22;          // 1 bit
    demura_reg reg_dmc_plane_89_ks22;          // 1 bit
    demura_reg reg_dmc_plane_78_ks22;          // 1 bit
    demura_reg reg_dmc_plane_67_ks22;          // 1 bit
    demura_reg reg_dmc_plane_56_ks22;          // 1 bit
    demura_reg reg_dmc_plane_45_ks22;          // 1 bit
    demura_reg reg_dmc_plane_34_ks22;          // 1 bit
    demura_reg reg_dmc_plane_23_ks22;          // 1 bit
    demura_reg reg_dmc_plane_12_ks22;          // 1 bit
    demura_reg reg_dmc_plane_b1_ks22;          // 1 bit
    demura_reg reg_dmc_h_block;                // 3 bit
    demura_reg reg_dmc_v_block;                // 3 bit
    demura_reg reg_dmc_rgb_mode;               // 1 bit
    demura_reg reg_dmc_block_size;             // 2 bit
    demura_reg reg_dmc_plane_num;              // 4 bit
    demura_reg reg_dmc_h_lut_num;              // 11bit
    demura_reg reg_dmc_v_lut_num;              // 11bit
    demura_reg reg_dmc_dummy1;                 // 16bit
    demura_reg reg_dmc_dummy2;                 // 16bit
    demura_reg reg_dmc_dither_rst_e_4_frame;   // 1 bit
    demura_reg reg_dmc_dither_pse_rst_num;     // 2 bit
    demura_reg reg_dmc_pos_af_gamma;           // 1 bit
    demura_reg reg_dmc_bypass;                 // 1 bit
    demura_reg reg_dmc_dither_en;              // 2 bit
    demura_reg reg_dmc_db_en;                  // 1 bit
    demura_reg reg_dmc_r_en;                   // 1 bit
    demura_reg reg_dmc_g_en;                   // 1 bit
    demura_reg reg_dmc_b_en;                   // 1 bit
    demura_reg reg_dmc_data_r_offset1;         // 16bit
    demura_reg reg_dmc_data_r_offset2;         // 16bit
    demura_reg reg_dmc_data_r_offset3;         // 16bit
    demura_reg reg_dmc_data_r_offset4;         // 16bit
    demura_reg reg_dmc_data_r_offset5;         // 16bit
    demura_reg reg_dmc_data_r_offset6;         // 16bit
    demura_reg reg_dmc_data_r_offset7;         // 16bit
    demura_reg reg_dmc_data_r_offset8;         // 16bit
    demura_reg reg_dmc_data_g_offset1;         // 16bit
    demura_reg reg_dmc_data_g_offset2;         // 16bit
    demura_reg reg_dmc_data_g_offset3;         // 16bit
    demura_reg reg_dmc_data_g_offset4;         // 16bit
    demura_reg reg_dmc_data_g_offset5;         // 16bit
    demura_reg reg_dmc_data_g_offset6;         // 16bit
    demura_reg reg_dmc_data_g_offset7;         // 16bit
    demura_reg reg_dmc_data_g_offset8;         // 16bit
    demura_reg reg_dmc_data_b_offset1;         // 16bit
    demura_reg reg_dmc_data_b_offset2;         // 16bit
    demura_reg reg_dmc_data_b_offset3;         // 16bit
    demura_reg reg_dmc_data_b_offset4;         // 16bit
    demura_reg reg_dmc_data_b_offset5;         // 16bit
    demura_reg reg_dmc_data_b_offset6;         // 16bit
    demura_reg reg_dmc_data_b_offset7;         // 16bit
    demura_reg reg_dmc_data_b_offset8;         // 16bit
    demura_reg reg_dmc_panel_h_size;           // 13bit
    demura_reg reg_dmc_preload_line;           // 13bit
//    demura_reg reg_dmc_r_h_r;                  // 5 bit
//    demura_reg reg_dmc_q_h_r;                  // 8 bit
//    demura_reg reg_dmc_grid_line_en;           // 1 bit
//    demura_reg reg_dmc_grid_line_size;         // 3 bit
//    demura_reg reg_dmc_grid_line_color;        // 8 bit
    demura_reg reg_dmc_flow_ctrl_en;           // 1 bit
    demura_reg reg_dmc_flow_len;               // 11bit
 // demura_reg reg_dmc_bist_fail;              // 16bit
 // demura_reg reg_adl_checksum_xp;            // 3 bit
 // demura_reg reg_adl_checksum_lr;            // 1 bit
 // demura_reg reg_adl_checksum_en;            // 1 bit
  demura_reg reg_dmc_guard_band_size;        // 8 bit
    demura_reg reg_dmc_plane_level9;           // 12bit
    demura_reg reg_dmc_data_r_mag9;            // 3 bit
    demura_reg reg_dmc_data_g_mag9;            // 3 bit
    demura_reg reg_dmc_data_b_mag9;            // 3 bit
    demura_reg reg_dmc_plane_89_coef;          // 14bit
    demura_reg reg_dmc_data_r_offset9;         // 16bit
    demura_reg reg_dmc_data_g_offset9;         // 16bit
    demura_reg reg_dmc_data_b_offset9;         // 16bit
    demura_reg reg_dmc_delta_low_limit;        // 16bit
    demura_reg reg_dmc_delta_high_limit;       // 16bit
    demura_reg reg_dmc_pos_v_offset;           // 4 bit
    demura_reg reg_dmc_pos_h_offset;           // 4 bit
    demura_reg reg_dmc_l1_eq_0_en;             // 1 bit
    demura_reg reg_dmc_flow_rest;              // 11bit
//  demura_reg reg_dmc_bist_fail_0;            // 7 bit
//  demura_reg reg_dmc_bist_fail_1;            // 7 bit
//  demura_reg reg_dmc_bist_fail_2;            // 7 bit
}dmc_registers;
#else
typedef struct
{
    _08bits reg_dmc_rgb_mode;        //  1 bit
    _08bits reg_dmc_block_size;      //  2 bit
    _08bits reg_dmc_plane_num;       //  4 bit
    _08bits reg_dmc_h_block;         //  3 bit
    _08bits reg_dmc_v_block;         //  3 bit
    _16bits reg_dmc_h_lut_num;       // 11 bit
    _16bits reg_dmc_v_lut_num;       // 10 bit
    _08bits reg_dmc_plane_b1_ks22;   //  1 bit
    _08bits reg_dmc_plane_12_ks22;   //  1 bit
    _08bits reg_dmc_plane_23_ks22;   //  1 bit
    _08bits reg_dmc_plane_34_ks22;   //  1 bit
    _08bits reg_dmc_plane_45_ks22;   //  1 bit
    _08bits reg_dmc_plane_56_ks22;   //  1 bit
    _08bits reg_dmc_plane_67_ks22;   //  1 bit
    _08bits reg_dmc_plane_78_ks22;   //  1 bit
    _08bits reg_dmc_plane_8w_ks22;   //  1 bit
    _16bits reg_dmc_black_limit;     // 12 bit
    _16bits reg_dmc_plane_level1;    // 12 bit
    _16bits reg_dmc_plane_level2;    // 12 bit
    _16bits reg_dmc_plane_level3;    // 12 bit
    _16bits reg_dmc_plane_level4;    // 12 bit
    _16bits reg_dmc_plane_level5;    // 12 bit
    _16bits reg_dmc_plane_level6;    // 12 bit
    _16bits reg_dmc_plane_level7;    // 12 bit
    _16bits reg_dmc_plane_level8;    // 12 bit
    _16bits reg_dmc_white_limit;     // 12 bit
    _16bits reg_dmc_plane_b1_coef;   // 14 bit
    _16bits reg_dmc_plane_12_coef;   // 14 bit
    _16bits reg_dmc_plane_23_coef;   // 14 bit
    _16bits reg_dmc_plane_34_coef;   // 14 bit
    _16bits reg_dmc_plane_45_coef;   // 14 bit
    _16bits reg_dmc_plane_56_coef;   // 14 bit
    _16bits reg_dmc_plane_67_coef;   // 14 bit
    _16bits reg_dmc_plane_78_coef;   // 14 bit
    _16bits reg_dmc_plane_8w_coef;   // 14 bit
    _08bits reg_dmc_data_r_mag1;     //  3 bit
    _08bits reg_dmc_data_r_mag2;     //  3 bit
    _08bits reg_dmc_data_r_mag3;     //  3 bit
    _08bits reg_dmc_data_r_mag4;     //  3 bit
    _08bits reg_dmc_data_r_mag5;     //  3 bit
    _08bits reg_dmc_data_r_mag6;     //  3 bit
    _08bits reg_dmc_data_r_mag7;     //  3 bit
    _08bits reg_dmc_data_r_mag8;     //  3 bit
    _08bits reg_dmc_data_g_mag1;     //  3 bit
    _08bits reg_dmc_data_g_mag2;     //  3 bit
    _08bits reg_dmc_data_g_mag3;     //  3 bit
    _08bits reg_dmc_data_g_mag4;     //  3 bit
    _08bits reg_dmc_data_g_mag5;     //  3 bit
    _08bits reg_dmc_data_g_mag6;     //  3 bit
    _08bits reg_dmc_data_g_mag7;     //  3 bit
    _08bits reg_dmc_data_g_mag8;     //  3 bit
    _08bits reg_dmc_data_b_mag1;     //  3 bit
    _08bits reg_dmc_data_b_mag2;     //  3 bit
    _08bits reg_dmc_data_b_mag3;     //  3 bit
    _08bits reg_dmc_data_b_mag4;     //  3 bit
    _08bits reg_dmc_data_b_mag5;     //  3 bit
    _08bits reg_dmc_data_b_mag6;     //  3 bit
    _08bits reg_dmc_data_b_mag7;     //  3 bit
    _08bits reg_dmc_data_b_mag8;     //  3 bit
    _16bits reg_dmc_data_r_offset1;   // 14 bit
    _16bits reg_dmc_data_r_offset2;   // 14 bit
    _16bits reg_dmc_data_r_offset3;   // 14 bit
    _16bits reg_dmc_data_r_offset4;   // 14 bit
    _16bits reg_dmc_data_r_offset5;   // 14 bit
    _16bits reg_dmc_data_r_offset6;   // 14 bit
    _16bits reg_dmc_data_r_offset7;   // 14 bit
    _16bits reg_dmc_data_r_offset8;   // 14 bit
    _16bits reg_dmc_data_g_offset1;   // 14 bit
    _16bits reg_dmc_data_g_offset2;   // 14 bit
    _16bits reg_dmc_data_g_offset3;   // 14 bit
    _16bits reg_dmc_data_g_offset4;   // 14 bit
    _16bits reg_dmc_data_g_offset5;   // 14 bit
    _16bits reg_dmc_data_g_offset6;   // 14 bit
    _16bits reg_dmc_data_g_offset7;   // 14 bit
    _16bits reg_dmc_data_g_offset8;   // 14 bit
    _16bits reg_dmc_data_b_offset1;   // 14 bit
    _16bits reg_dmc_data_b_offset2;   // 14 bit
    _16bits reg_dmc_data_b_offset3;   // 14 bit
    _16bits reg_dmc_data_b_offset4;   // 14 bit
    _16bits reg_dmc_data_b_offset5;   // 14 bit
    _16bits reg_dmc_data_b_offset6;   // 14 bit
    _16bits reg_dmc_data_b_offset7;   // 14 bit
    _16bits reg_dmc_data_b_offset8;   // 14 bit

} dmc_registers;
#endif

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
    MS_U16        hdr_checksum;       // Only use for auo binary header
    E_COLOR_MODE  rgb_mode;
    MS_U32        dmc_block_size;     // 64, 128, 256, unit : bits
    MS_U32        dmc_panel_num;
    MS_U32        dmc_h_block_size;   //  4,   8,  16, unit : pixs
    MS_U32        dmc_v_block_size;   //  4,   8,  16, unit : pixs
    MS_U32        dmc_h_lut_num;
    MS_U32        dmc_v_lut_num;
    MS_U32        dmc_black_limit;
    MS_U16        panel_levels[8];
    MS_U32        dmc_white_limit;

    // Header Buffer
    MS_U8         *hdr_buffer;
    MS_U32        hdr_offset;
    MS_U32        hdr_length;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
    MS_U32        lut_checksum;
} AUO_Demura_Header;

typedef struct
{
    MS_U16          PARAMETER_CRC;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U16          LOWER_BOUND;
    MS_U16          UPPER_BOUND;
    MS_U16          PLANE[MAX_PLANE_NUM];
    MS_U32          TBL_END_ADDR;
    MS_U16          TABLE_CRC;
    MS_U32          DEMURA_TABLE_LENGTH;

    // Header Buffer
    MS_U8         *hdr_buffer;
    MS_U32        hdr_offset;
    MS_U32        hdr_length;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}CHOT_Demura_Header;

typedef struct
{
    MS_U8           DEMURA_PLANE_NUM;
    MS_BOOL         DEMURA_MODE;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U32          TABLE_CRC;

    // Header Buffer
    MS_U8         *hdr_buffer;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}CSOT_Hisilicon_Demura_Header;

typedef struct
{
    MS_U16          PARAMETER_CRC;
    MS_U8           DEMURA_PLANE_NUM;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U16          PLANE[MAX_PLANE_NUM];
    MS_U8           Value_1[MAX_PLANE_NUM];
    MS_U16          TABLE_CRC;

    // Header Buffer
    MS_U8         *hdr_buffer;
    MS_U32        hdr_offset;
    MS_U32        hdr_length;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}SDC_Demura_Header;

typedef struct
{
    MS_U16          LOW_BOUND;
    MS_U16          PLANE0_LV;
    MS_U16          PLANE1_LV;
    MS_U16          PLANE2_LV;
    MS_U16          PLANE3_LV;
    MS_U16          HIGH_BOUND;
    MS_U16          TABLE_CRC;
    MS_U16          HEADER_CRC;

    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;

    // Header Buffer
    MS_U8         *hdr_buffer;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}CSOT_Himax_Demura_Header;

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
    MS_U8           DEMURA_PLANE_NUM;
    MS_BOOL         DEMURA_MODE;
    MS_U16          DEMURA_TBL_H;
    MS_U16          DEMURA_TBL_V;
    MS_U8           DEMURA_BLK_H;
    MS_U8           DEMURA_BLK_V;
    MS_U32          TABLE_CRC[2];
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
    MS_U8           *file_header;
    MS_U8           *section_header;
    MS_U8           *demura_header;
    // Lut Buffer
    MS_U8           *lut_buffer[2];
    MS_U32          lut_offset[2];
    MS_U32          lut_length[2];
}CSOT_CSOT_Demura_Header;

typedef struct{
    MS_U16 mode;
    MS_U16 bitWidth;
    MS_U16 hLutNum;
    MS_U16 vLutNum;
    MS_U16 planeNum;
    MS_U16 planeLevel[6];

    // TBD
    MS_U16 hBlkNum;
    MS_U16 vBlkNum;
    MS_U16 blackLimit;
    MS_U16 whiteLimit;

    // record the lut pointer
    MS_U8 *lut_buffer;
}TCL_Demura_Header;

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


// { Start BOE ESWIN
#define BOE_ESWIN_HEADER_START      (0)
#define BOE_ESWIN_HEADER_LEN        (0x70) // 0 to 0x6f
#define BOE_ESWIN_LUT_START         (0x70)
#define BOE_ESWIN_LUT_LEN           (590238)
#define BOE_ESWIN_LUT_CHECKSUM_H    (0x99)
#define BOE_ESWIN_LUT_CHECKSUM_L    (0x9A)
#define BOE_ESWIN_HEADER_CHECKSUM_H (0x19B)
#define BOE_ESWIN_HEADER_CHECKSUM_L (0x19C)
#define BOE_ESWIN_USE_PLANE_NUM     (3)
#define BOE_ESWIN_MAX_PLANE_NUM     (9)
#define BOE_ESWIN_ALIGN_BYTES       (128/8)
#define BOE_ESWIN_REPEATED_BYTES    (20) /*((128+32)/8)*/
#define BOE_ESWIN_REPEATED_MAX_IDX  (16)
#define BOE_ESWIN_DATA_BASE         (512)

#define BOE_ONE_LINE_BYTES(hnodes, layer) ((((((hnodes * layer) * 10) + 127) / 128) * 128) / 8)
#define BOE_ESWIN_GET_LAYER(p, pinfo) \
({\
    int nLayer = 0;\
    switch (p)\
    {\
        case 0: nLayer = pinfo->reg_dmc_plane_level1; break;\
        case 1: nLayer = pinfo->reg_dmc_plane_level2; break;\
        case 2: nLayer = pinfo->reg_dmc_plane_level3; break;\
        case 3: nLayer = pinfo->reg_dmc_plane_level4; break;\
        case 4: nLayer = pinfo->reg_dmc_plane_level5; break;\
        case 5: nLayer = pinfo->reg_dmc_plane_level6; break;\
        case 6: nLayer = pinfo->reg_dmc_plane_level7; break;\
        case 7: nLayer = pinfo->reg_dmc_plane_level8; break;\
        default: break;\
    }\
    nLayer = nLayer / 4;\
    nLayer;\
})


#define BOE_ESWIN_HEADER_U32(ret, buffer, idx) \
    ret = (((MS_U32)buffer[idx]) << 24)\
        | (((MS_U32)buffer[idx + 1]) << 16)\
        | (((MS_U32)buffer[idx + 2]) << 8)\
        | ((MS_U32)buffer[idx + 3]);

#define BOE_ESWIN_HEADER_U16(ret, buffer, idx) \
    ret = (((MS_U16)buffer[idx]) << 8) | ((MS_U16)buffer[idx + 1]);

#define BOE_ESWIN_GET_LUT_DATA(idx, pLut) \
({  MS_U16 ret = 0;\
    if (idx == 0) {ret = (((MS_U16)pLut[2] & 0x03) << 8) | (MS_U16)pLut[3];}\
    else if (idx == 1) {ret =  ((((MS_U16)pLut[1] & 0x0F) << 6) | ((MS_U16)pLut[2] >> 2));}\
    else if (idx == 2) {ret =  ((((MS_U16)pLut[0] & 0x3F) << 4) | ((MS_U16)pLut[1] >> 4));}\
    else if (idx == 3) {ret =  ((((MS_U16)pLut[7] & 0xFF) << 2) | ((MS_U16)pLut[0] >> 6));}\
    else if (idx == 4) {ret =  ((((MS_U16)pLut[5] & 0x03) << 8) | ((MS_U16)pLut[6]));}\
    else if (idx == 5) {ret =  ((((MS_U16)pLut[4] & 0x0F) << 6) | ((MS_U16)pLut[5] >> 2));}\
    else if (idx == 6) {ret =  ((((MS_U16)pLut[11] & 0x03F) << 4) | ((MS_U16)pLut[4] >> 4));}\
    else if (idx == 7) {ret =  ((((MS_U16)pLut[10] & 0xFF) << 2) | ((MS_U16)pLut[11] >> 6));}\
    else if (idx == 8) {ret =  ((((MS_U16)pLut[8] & 0x03) << 8) | ((MS_U16)pLut[9] & 0xFF));}\
    else if (idx == 9) {ret =  ((((MS_U16)pLut[15] & 0x0F) << 6) | ((MS_U16)pLut[8] >> 2));}\
    else if (idx == 10) {ret = ((((MS_U16)pLut[14] & 0x3F) << 4) | ((MS_U16)pLut[15] >> 4));}\
    else if (idx == 11) {ret = ((((MS_U16)pLut[13] & 0xFF) << 2) | ((MS_U16)pLut[14] >> 6));}\
    else if (idx == 12) {ret = ((((MS_U16)pLut[19] & 0x03) << 8) | ((MS_U16)pLut[12]));}\
    else if (idx == 13) {ret = ((((MS_U16)pLut[18] & 0x0F) << 6) | ((MS_U16)pLut[19] >> 2));}\
    else if (idx == 14) {ret = ((((MS_U16)pLut[17] & 0x3F) << 4) | ((MS_U16)pLut[18] >> 4));}\
    else if (idx == 15) {ret = ((((MS_U16)pLut[16] & 0xFF) << 2) | ((MS_U16)pLut[17] >> 6));}\
    ret;})

typedef struct {
    MS_U32 type;
    MS_U32 offset;
    MS_U32 size;
    MS_U32 crc;
}BOE_ESWIN_Section_Header;

typedef struct {
    MS_U32 u32MagicNum;
    char name[14];
    MS_U16 version;
    MS_U16 sectionNum;
    MS_U16 sectionCrc;
    MS_U16 headerCrc;
    BOE_ESWIN_Section_Header header[2];
    MS_U8 enDemuraMod;
    MS_U8 u8PlaneNum;
    MS_U8 u8HblockSize;
    MS_U8 u8VblockSize;
    MS_U16 u16HLutNum;
    MS_U16 u16VLutNum;
    MS_U8 u8twoChipEn;
    MS_U8 u8DataFormate;
    MS_U16 u16BlackLimit;
    MS_U16 u16WhiteLimit;
    MS_U16 u16PlaneLevel[9];

    // extra info
    MS_U32 u32SegNum;
    MS_U32 u32SegsPerLine;
    MS_U32 u32SegAlignedBytes;
    MS_U32 u32BytesPerLine;

    // header buffer
    MS_U8 *hdr_buffer;

    // Lut Buffer
    MS_U8         *lut_buffer;
    MS_U32        lut_offset;
    MS_U32        lut_length;
}BOE_ESWIN_Demura_Header;

// End BOE ESWIN }

MS_U8 inx_crc8(MS_U8 *data, MS_U8 seed, MS_U32 crcStart, MS_U32 crcLength);
MS_BOOL load_vendor_header(AUO_Demura_Header *phdr);
MS_BOOL load_vendor_header_chot(CHOT_Demura_Header *phdr, MS_U32 flash_offset);
MS_BOOL load_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr);
MS_BOOL load_vendor_header_csot_himax(CSOT_Himax_Demura_Header *phdr);
MS_BOOL load_vendor_header_sdc(SDC_Demura_Header *phdr);
MS_BOOL fetch_decode_info_inx(INX_Demura_Header *phdr, Vendor2Mstar_Info *pv2m);
MS_BOOL parse_vendor_header(AUO_Demura_Header *phdr);
MS_BOOL parse_vendor_header_chot(CHOT_Demura_Header *phdr);
MS_BOOL parse_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr);
MS_BOOL parse_vendor_header_csot_himax(CSOT_Himax_Demura_Header *phdr);
MS_BOOL parse_vendor_header_sdc(SDC_Demura_Header *phdr);
MS_BOOL parse_vendor_header_inx(MS_U8 *buf, INX_Demura_Header *phdr);
void dump_vendor_header(AUO_Demura_Header *phdr);
void dump_vendor_header_chot(CHOT_Demura_Header *phdr);
void dump_vendor_header_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr);
void dump_vendor_header_csot_himax(CSOT_Himax_Demura_Header *phdr);
MS_BOOL load_vendor_lut(AUO_Demura_Header *phdr);
MS_BOOL load_vendor_lut_chot(CHOT_Demura_Header *phdr,MS_U32 flash_offset);
MS_BOOL load_vendor_lut_csot_hisilicon(CSOT_Hisilicon_Demura_Header *phdr);
MS_BOOL load_vendor_lut_csot_himax(CSOT_Himax_Demura_Header *phdr);
MS_BOOL load_vendor_lut_sdc(SDC_Demura_Header *phdr);
MS_U16 get_lut_checksum(void);
MS_U16 get_lut_checksum_chot(MS_U32 flash_offset);
MS_U16 get_lut_checksum_csot_hisilicon(void);
MS_U16 get_lut_checksum_csot_himax(void);
MS_U16 get_lut_checksum_sdc(void);

MS_U16 get_CSOT_CSOT_CRC(E_FILE_CRC eCRCType);
MS_BOOL load_vendor_header_csot_csot(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL parse_vendor_header_csot_csot(CSOT_CSOT_Demura_Header *phdr);
MS_BOOL load_vendor_lut_csot_csot(CSOT_CSOT_Demura_Header *phdr);

MS_BOOL load_vendor_header_boe_eswin(BOE_ESWIN_Demura_Header *phdr);
MS_BOOL parse_vendor_header_boe_eswin(BOE_ESWIN_Demura_Header *phdr);
MS_BOOL load_vendor_lut_boe_eswin(BOE_ESWIN_Demura_Header *phdr);
#endif
