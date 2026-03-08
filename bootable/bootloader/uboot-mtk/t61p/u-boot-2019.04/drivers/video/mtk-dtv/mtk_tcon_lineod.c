// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon lineod driver
 *
 * Copyright (c) 2022 MediaTek Inc.
 */
#include <common.h>
#include <dm.h>
#include <utility.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <dts_parser.h>
#include <asm/io.h>

#include "coda/hwreg_common.h"
#include "coda/DEMURA_BKA377_V004.h"
#include "coda/LINEOD_BKA3A8_V004.h"
#include "coda/PAT_DET_BKA37F_V004.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_tcon_lineod.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_autodownload.h"

#define ENABLE_PCID_AUTODOWNLOAD        (TRUE)

// Debug related
#define RGB_LUT_CHECK(R, G, B) ((R) == NULL || (G) == NULL || (B) == NULL)

#define PCID_TABLE_SIZE_17X17           (289)
#define PCID_PIXEL_OD_POLARITY_LUT_SIZE (8)
#define PCID_SKIP_HEADER                (21)

#define PCID_RGB_CHANNEL                (3)
#define PCID_TABLE_COL_9                (9)
#define PCID_TABLE_COL_8                (8)
#define PCID_TABLE_ROW_9                (9)
#define PCID_TABLE_ROW_8                (8)

//PCID ADL
#define XC_AUTODOWNLOAD_PCID_19X19_OFFSET   (181) //10*10 + 9*9
#define XC_AUTODOWNLOAD_PCID_17X17_OFFSET   (145) //9*9 + 8*8

//LINE OD
#define LINE_OD_TOTAL_LUT_NUM           (10)
#define LINE_OD_TABLE_SIZE_19X19        (361)
#define LINE_OD_TABLE_SIZE_17X17        (289)
#define LINE_OD_GAIN_TABLE_SIZE_4X4     (16)
#define LINE_OD_GAIN_TABLE_SIZE_13X9    (117)
#define LINE_OD_REG_NUM_1X1             (1)
#define LINE_OD_REG_NUM_4X4             (16)
#define LINE_OD_REG_NUM_2X8             (16)
#define LINE_OD_REG_NUM_10X1            (10)
#define LINE_OD_TABLE_COL_10            (10)
#define LINE_OD_TABLE_COL_9             (9)
#define LINE_OD_TABLE_ROW_10            (10)
#define LINE_OD_TABLE_ROW_9             (9)
#define LINE_OD_GAIN_TABLE_ROW_4        (4)
#define LINE_OD_GAIN_TABLE_COL_4        (4)
#define LINE_OD_GAIN_TABLE_COL_13       (13)
#define LINE_OD_GAIN_TABLE_ROW_9        (9)
#define LINE_OD_TABLE_UNIT_SIZE_8       (0)
#define LINE_OD_TABLE_UNIT_SIZE_12      (1)
#define LINE_OD_HW_GAIN_SIZE_13X9       (117)
#define LINE_OD_HW_GAIN_COL_13          (13)
#define LINE_OD_HW_GAIN_ROW_9           (9)

#define MAX_DELAY_TIME                  (10)
#define TRIG_WRITE_SRAM                 (0x2000)
#define TRIG_READ_SRAM                  (0x4000)
#define SELECT_ALL_CHANNEL_RGB          (0x000C)
#define SELECT_SRAM1                    (0x0000)
#define SELECT_SRAM2                    (0x0100)
#define SELECT_SRAM3                    (0x0200)
#define SELECT_SRAM4                    (0x0300)
#define W_RGB_MASK                      (0x230C)
#define PNL_LINEOD_SHARE_SRAM           (TRUE)

typedef enum
{
    E_PCID_LUT0 = 0,
    E_PCID_LUT1 = 1,
} EN_PCID_LUT_TYPE;

typedef struct __attribute__((packed))
{
    uint16_t u16Verison;                //[15:8]: major, [7:0]: minor version
    uint8_t u8PcidEn;
    uint8_t u8PixelOdEn;                //0: do PCID, 1: do Pixel OD
    uint8_t u8PanelType;                //0: DRD, 1: V2LTD
    uint8_t u8SwapEn;                   //Pixel OD input/ouput R/B swap
    uint8_t u8PcidMode;
    uint8_t u8XTR_En;
    uint16_t u16XTR_thresh0SamePol;
    uint16_t u16XTR_thresh1DiffPol;
    uint16_t u16XTR_threshWhite;
    uint16_t u16XTR_threshBlack;
    uint32_t u32YearMonthDay;           //[31:25]: reserved, [24:20]: year, [19:16]: month,
                                        //[15:11]: day, [10:0]: minute
    uint8_t u8VacEn;
    uint8_t u8Reverse;
} ST_PCID_SUB_HEADER, *PST_PCID_SUB_HEADER;

typedef struct __attribute__((packed))
{
    uint16_t u16Version;                //[15:8]: major, [7:0]: minor version
    uint8_t u8LineOD_en;                //[0]: line od enable
    uint8_t u8SeparateFlag;             //[0]: channel separate flag [1]: table separate flag (unused)
    uint32_t u32YearMonthDayMinute;     //[31:25]: reserved, [24:20]: year, [19:16]: month,
                                        //[15:11]: day, [10:0]: minute
    uint8_t u8TableMatrix;              //0: 19x19, 1: 17x17
    uint8_t u8RegionNumber;             //(Version>1 0: 1x1, 1: 2x8, 2:4X4)(Version<=1 16: 4x4 or 2x8, 1: 1x1)
    uint8_t u8GainMatrix;               //0: 4x4, 1: 13x9
    uint8_t u8GainRegion;               //0: 1x1
    uint8_t u8TableBit;                 //(Version>2 0: 8bit 1:12bit) (Version<=2 no this member)
} ST_LINE_OD_SUB_HEADER, *PST_LINE_OD_SUB_HEADER;

typedef struct __attribute__((packed))
{
    uint16_t u16SRAM1_lut0 : 12; //[11:0]
    uint16_t u16SRAM1_lut1 : 12; //[23:12]
    uint16_t u16SRAM1_lut2 : 12; //[35:24]
    uint16_t u16SRAM1_lut3 : 12; //[47:36]
    uint16_t u16SRAM1_lut4 : 12; //[59:48]
    uint16_t u16SRAM1_lut5 : 12; //[71:60]
    uint16_t u16SRAM1_lut6 : 12; //[83:72]
    uint16_t u16SRAM1_lut7 : 12; //[95:84]
    uint16_t u16SRAM1_lut8 : 12; //[107:96]
    uint16_t u16SRAM1_lut9 : 12; //[119:108]
    uint16_t u16Dummy0 : 8;      //[127:120]
    uint16_t u16SRAM2_lut0 : 12; //[139:128]
    uint16_t u16SRAM2_lut1 : 12; //[151:140]
    uint16_t u16SRAM2_lut2 : 12; //[163:152]
    uint16_t u16SRAM2_lut3 : 12; //[175:164]
    uint16_t u16SRAM2_lut4 : 12; //[187:176]
    uint16_t u16SRAM2_lut5 : 12; //[199:88]
    uint16_t u16SRAM2_lut6 : 12; //[211:200]
    uint16_t u16SRAM2_lut7 : 12; //[223:212]
    uint16_t u16SRAM2_lut8 : 12; //[235:224]
    uint16_t u16SRAM2_lut9 : 12; //[247:236]
    uint16_t u16Dummy1 : 8;      //[255:248]
}ST_PCID_12BIT_DRAM_FORMAT;

typedef struct{
    ST_PCID_12BIT_DRAM_FORMAT *pstData;
    uint16_t u8TableIdx;
    uint16_t u16RIndex;
    uint16_t u16GIndex;
    uint16_t u16BIndex;
    uint16_t u16RVal;
    uint16_t u16GVal;
    uint16_t u16BVal;
} ST_SRAM_DATA_FORMAT_INFO, *PST_SRAM_DATA_FORMAT_INFO;

void _pcid_init(uint8_t* pu8Tbl,bool bPixelOverdriveEn,bool bXTREn)
{
    W2BYTEMSK(REG_0140_DEMURA_BKA377_V004,
                (bPixelOverdriveEn ? 1 : 0),
                REG_0140_DEMURA_BKA377_V004_REG_PCID_PIXELOD_EN_0140);
    W2BYTEMSK(REG_0140_DEMURA_BKA377_V004,
                (bXTREn ? 1 : 0),
                REG_0140_DEMURA_BKA377_V004_REG_PCID_PIXELOD_XTR_EN_0140);
    W2BYTEMSK(REG_0144_DEMURA_BKA377_V004, (pu8Tbl[0] | (pu8Tbl[1] << 8)), Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0148_DEMURA_BKA377_V004, (pu8Tbl[2] | (pu8Tbl[3] << 8)), Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_014C_DEMURA_BKA377_V004, (pu8Tbl[4] | (pu8Tbl[5] << 8)), Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0150_DEMURA_BKA377_V004, (pu8Tbl[6] | (pu8Tbl[7] << 8)), Fld(16,0,AC_FULLW10));
}

void _set_pcid_enable(bool bEnable)
{
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, (bEnable ? 1 : 0), REG_003C_DEMURA_BKA377_V004_REG_PCID_EN_003C);
}

bool _is_pcid_enable(void)
{
    return (bool)R2BYTEMSK(REG_003C_DEMURA_BKA377_V004, REG_003C_DEMURA_BKA377_V004_REG_PCID_EN_003C);
}

bool _write_pcid_tbl(uint8_t* pu8RTbl,uint8_t* pu8GTbl,uint8_t* pu8BTbl, EN_PCID_LUT_TYPE eLutType)
{
    bool bEnable = FALSE;
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0;
    uint16_t u16DataIdx = 0;

    TCON_CHECK_PARAMETER_NULL(pu8RTbl);
    TCON_CHECK_PARAMETER_NULL(pu8GTbl);
    TCON_CHECK_PARAMETER_NULL(pu8BTbl);

    bEnable = _is_pcid_enable();
    _set_pcid_enable(false);    // PCID Disable

    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, eLutType, Fld(1,15,AC_MSKB1)); //sel lut bit[15]

    for (u16CodeTableY = 0; u16CodeTableY < PCID_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < PCID_TABLE_COL_9; u16CodeTableX++)
        {
            u16DataIdx = u16CodeTableX * 2 + u16CodeTableY * (PCID_TABLE_COL_9+PCID_TABLE_COL_8) * 2;
            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004, pu8RTbl[u16DataIdx], 0xFF); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004, pu8GTbl[u16DataIdx], 0xFF); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004, pu8BTbl[u16DataIdx], 0xFF); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX + (u16CodeTableY*PCID_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address

            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM1|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    u16CodeTableX = 0;
    u16CodeTableY = 0;
    for (u16CodeTableY = 0; u16CodeTableY < PCID_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < PCID_TABLE_COL_8; u16CodeTableX++)
        {
            u16DataIdx = u16CodeTableX * 2 + u16CodeTableY * (PCID_TABLE_COL_9+PCID_TABLE_COL_8) * 2 + 1;
            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004, pu8RTbl[u16DataIdx], 0xFF); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004, pu8GTbl[u16DataIdx], 0xFF); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004, pu8BTbl[u16DataIdx], 0xFF); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*PCID_TABLE_COL_8),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address

            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM2|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    u16CodeTableX = 0;
    u16CodeTableY = 0;
    for (u16CodeTableY = 0; u16CodeTableY < PCID_TABLE_ROW_8; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < PCID_TABLE_COL_9; u16CodeTableX++)
        {
            u16DataIdx = u16CodeTableX * 2 +
                            (u16CodeTableY) * (PCID_TABLE_COL_9+PCID_TABLE_COL_8) * 2 +
                            (PCID_TABLE_COL_9+PCID_TABLE_COL_8);

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004, pu8RTbl[u16DataIdx], 0xFF); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004, pu8GTbl[u16DataIdx], 0xFF); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004, pu8BTbl[u16DataIdx], 0xFF); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*PCID_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address

            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM3|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    u16CodeTableX = 0;
    u16CodeTableY = 0;
    for (u16CodeTableY = 0; u16CodeTableY < PCID_TABLE_ROW_8; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < PCID_TABLE_COL_8; u16CodeTableX++)
        {
            u16DataIdx = u16CodeTableX * 2 +
                            u16CodeTableY * (PCID_TABLE_COL_9+PCID_TABLE_COL_8) * 2 +
                            (PCID_TABLE_COL_9+PCID_TABLE_COL_8) +1;

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004, pu8RTbl[u16DataIdx], 0xFF); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004, pu8GTbl[u16DataIdx], 0xFF); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004, pu8BTbl[u16DataIdx], 0xFF); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*PCID_TABLE_COL_8),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address

            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM4|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, 0, REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_CH_SEL_003C);
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, 0, REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_SEL_003C);
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, 0, Fld(1,15,AC_MSKB1));

    _set_pcid_enable(bEnable);    // PCID recover

    return TRUE;
}

bool _pcid_setting_proc(uint8_t* pu8TconTab)
{
    PST_PCID_SUB_HEADER pstHeader = NULL;
    uint8_t *pu8PCIDTabA1 = NULL;
    uint8_t *pu8PCIDTabA2 = NULL;
    uint8_t *pu8PCIDTabA3 = NULL;
    uint8_t *pu8PCIDTabB1 = NULL;
    uint8_t *pu8PCIDTabB2 = NULL;
    uint8_t *pu8PCIDTabB3 = NULL;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    PNL_MALLOC_MEM(pu8PCIDTabA1, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8PCIDTabA2, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8PCIDTabA3, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8PCIDTabB1, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8PCIDTabB2, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8PCIDTabB3, PCID_TABLE_SIZE_17X17 * sizeof(uint8_t), bRet);

    if (bRet)
    {
        uint8_t u8PixelOverDriveLutTab[PCID_PIXEL_OD_POLARITY_LUT_SIZE] = {0};
        uint16_t u16TargetIndex = 0;

        uint16_t u16Verison                   = *((unsigned char*) (pu8TconTab));
        uint8_t u8PCID_En                     = *((unsigned char*) (pu8TconTab+2));
        uint8_t u8Pixel_OD_En                 = *((unsigned char*) (pu8TconTab+3));
        uint8_t u8PanelType                   = *((unsigned char*) (pu8TconTab+4));
        uint8_t u8Swap_En                     = *((unsigned char*) (pu8TconTab+5));
        uint8_t u8PCIDMode                    = *((unsigned char*) (pu8TconTab+6));
        uint8_t u8XTR_En                      = *((unsigned char*) (pu8TconTab+7));
        uint16_t u16XTR_threshold0_polarity   = *((unsigned char*) (pu8TconTab+8));
        uint16_t u16XTR_threshold1_polarity   = *((unsigned char*) (pu8TconTab+10));

        uint16_t u16XTR_thresholdW            = *((unsigned char*) (pu8TconTab+12));
        uint16_t u16XTR_thresholdB            = *((unsigned char*) (pu8TconTab+14));
        //uint32_t u32YearMonthDay              = *((unsigned char*) (pu8TconTab+16));
        uint8_t u8Reverse                     = *((unsigned char*) (pu8TconTab+20));

        pstHeader = (PST_PCID_SUB_HEADER)pu8TconTab;

        TCON_DEBUG("u16Verison=%d -> %d\n", u16Verison, pstHeader->u16Verison);
        TCON_DEBUG("u8PCID_En=%d -> %d\n", u8PCID_En, pstHeader->u8PcidEn);
        TCON_DEBUG("u8Pixel_OD_En=%d -> %d\n", u8Pixel_OD_En, pstHeader->u8PixelOdEn);
        TCON_DEBUG("u8PanelType=%d -> %d\n", u8PanelType, pstHeader->u8PanelType);
        TCON_DEBUG("u8Swap_En=%d -> %d\n", u8Swap_En, pstHeader->u8SwapEn);
        TCON_DEBUG("u8PCIDMode=%d -> %d\n", u8PCIDMode, pstHeader->u8PcidMode);
        TCON_DEBUG("u8XTR_En=%d -> %d\n", u8XTR_En, pstHeader->u8XTR_En);

        TCON_DEBUG("u16XTR_threshold0_polarity=%d -> %d\n",
                    u16XTR_threshold0_polarity, pstHeader->u16XTR_thresh0SamePol);
        TCON_DEBUG("u16XTR_threshold1_polarity=%d -> %d\n",
                    u16XTR_threshold1_polarity, pstHeader->u16XTR_thresh1DiffPol);
        TCON_DEBUG("u16XTR_thresholdW=%d -> %d\n", u16XTR_thresholdW, pstHeader->u16XTR_threshWhite);
        TCON_DEBUG("u16XTR_thresholdB=%d -> %d\n", u16XTR_thresholdB, pstHeader->u16XTR_threshBlack);
        TCON_DEBUG("u8Reverse=%d -> %d\n", u8Reverse, pstHeader->u8Reverse);

        u16TargetIndex = PCID_SKIP_HEADER;
        memcpy(pu8PCIDTabA1, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID R table for lut0

        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(pu8PCIDTabA2, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID G table for lut0

        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(pu8PCIDTabA3, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID B table for lut0

        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(pu8PCIDTabB1, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID R table for lut1

        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(pu8PCIDTabB2, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID G table for lut1

        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(pu8PCIDTabB3, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_TABLE_SIZE_17X17); //Load PCID B table for lut1
        u16TargetIndex += PCID_TABLE_SIZE_17X17;
        memcpy(&u8PixelOverDriveLutTab, (pu8TconTab+u16TargetIndex),
                sizeof(uint8_t)*PCID_PIXEL_OD_POLARITY_LUT_SIZE);

        _pcid_init(u8PixelOverDriveLutTab, u8Pixel_OD_En,u8XTR_En);
        bRet &= _write_pcid_tbl(pu8PCIDTabA1, pu8PCIDTabA2, pu8PCIDTabA3, E_PCID_LUT0);
        bRet &= _write_pcid_tbl(pu8PCIDTabB1, pu8PCIDTabB2, pu8PCIDTabB3, E_PCID_LUT1);

        if (bRet)
        {
            _set_pcid_enable(u8PCID_En);
        }
    }
    else
    {
         TCON_ERROR("Allocate buffer return fail\n.");
    }

    PNL_FREE_MEM(pu8PCIDTabA1);
    PNL_FREE_MEM(pu8PCIDTabA2);
    PNL_FREE_MEM(pu8PCIDTabA3);
    PNL_FREE_MEM(pu8PCIDTabB1);
    PNL_FREE_MEM(pu8PCIDTabB2);
    PNL_FREE_MEM(pu8PCIDTabB3);

    return bRet;
}

void _set_lineod_enable(bool bEnable)
{
    W2BYTEMSK(REG_0180_PAT_DET_BKA37F_V004,
                (bEnable ? 1 : 0), REG_0180_PAT_DET_BKA37F_V004_REG_PCID_LINEOD_EN_0180);
}

bool _is_lineod_enable(void)
{
    return (bool)R2BYTEMSK(REG_0180_PAT_DET_BKA37F_V004,
                            REG_0180_PAT_DET_BKA37F_V004_REG_PCID_LINEOD_EN_0180);
}

bool _write_lineod_tbl_sram1(
                        uint8_t* pu8EvenRTbl, uint8_t* pu8EvenGTbl, uint8_t* pu8EvenBTbl,
                        uint8_t* pu8OddRTbl, uint8_t* pu8OddGTbl, uint8_t* pu8OddBTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram1
    for (u16CodeTableY = 0; u16CodeTableY<LINE_OD_TABLE_ROW_10; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX<LINE_OD_TABLE_COL_10; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error: %d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu8EvenRTbl[u16lutIndex] | pu8OddRTbl[u16lutIndex]<<8,
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu8EvenGTbl[u16lutIndex] | pu8OddGTbl[u16lutIndex]<<8,
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu8EvenBTbl[u16lutIndex] | pu8OddBTbl[u16lutIndex]<<8,
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_10),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                        TRIG_WRITE_SRAM|SELECT_SRAM1|SELECT_ALL_CHANNEL_RGB,
                        W_RGB_MASK); //sel RGB
            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_tbl_sram2(
                        uint8_t* pu8EvenRTbl, uint8_t* pu8EvenGTbl, uint8_t* pu8EvenBTbl,
                        uint8_t* pu8OddRTbl, uint8_t* pu8OddGTbl, uint8_t* pu8OddBTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram2
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_10; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_9; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                        u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 + 1;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error: %d\n", u16lutIndex);
                return FALSE;
            }
            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu8EvenRTbl[u16lutIndex] | pu8OddRTbl[u16lutIndex]<<8,
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu8EvenGTbl[u16lutIndex] | pu8OddGTbl[u16lutIndex]<<8,
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu8EvenBTbl[u16lutIndex] | pu8OddBTbl[u16lutIndex]<<8,
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM2|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB
            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                    REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                    (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_tbl_sram3(
                        uint8_t* pu8EvenRTbl, uint8_t* pu8EvenGTbl, uint8_t* pu8EvenBTbl,
                        uint8_t* pu8OddRTbl, uint8_t* pu8OddGTbl, uint8_t* pu8OddBTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram3
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_10; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            (u16CodeTableY) * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 +
                            (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9);

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu8EvenRTbl[u16lutIndex] | pu8OddRTbl[u16lutIndex]<<8,
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu8EvenGTbl[u16lutIndex] | pu8OddGTbl[u16lutIndex]<<8,
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu8EvenBTbl[u16lutIndex] | pu8OddBTbl[u16lutIndex]<<8,
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_10),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM3|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB
            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_tbl_sram4(
                        uint8_t* pu8EvenRTbl, uint8_t* pu8EvenGTbl, uint8_t* pu8EvenBTbl,
                        uint8_t* pu8OddRTbl, uint8_t* pu8OddGTbl, uint8_t* pu8OddBTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram4
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_9; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 +
                            (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) +1;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error: %d\n", u16lutIndex);
                return FALSE;
            }
            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu8EvenRTbl[u16lutIndex] | pu8OddRTbl[u16lutIndex]<<8,
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu8EvenGTbl[u16lutIndex] | pu8OddGTbl[u16lutIndex]<<8,
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu8EvenBTbl[u16lutIndex] | pu8OddBTbl[u16lutIndex]<<8,
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM4|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB
            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                    REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                    (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_tbl(
                        uint8_t* pu8EvenRTbl, uint8_t* pu8EvenGTbl, uint8_t* pu8EvenBTbl,
                        uint8_t* pu8OddRTbl, uint8_t* pu8OddGTbl, uint8_t* pu8OddBTbl,
                        uint8_t u8LutSramSeg)
{
    bool bPcidEnable = FALSE, bLineODEnable = FALSE;

    if (RGB_LUT_CHECK(pu8EvenRTbl, pu8EvenGTbl, pu8EvenBTbl) ||
        RGB_LUT_CHECK(pu8OddRTbl, pu8OddGTbl, pu8OddBTbl))
    {
        TCON_ERROR("Table error \n.");
        return FALSE;
    }

    bPcidEnable = _is_pcid_enable();
    bLineODEnable = _is_lineod_enable();

    _set_pcid_enable(false);        // PCID Disable
    _set_lineod_enable(false);      // LINE OD Disable

    W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                u8LutSramSeg,
                REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_SEG_0004); //sel PCID lut sram segment

    _write_lineod_tbl_sram1(
                        pu8EvenRTbl, pu8EvenGTbl, pu8EvenBTbl,
                        pu8OddRTbl, pu8OddGTbl, pu8OddBTbl);

    _write_lineod_tbl_sram2(
                        pu8EvenRTbl, pu8EvenGTbl, pu8EvenBTbl,
                        pu8OddRTbl, pu8OddGTbl, pu8OddBTbl);

    _write_lineod_tbl_sram3(
                        pu8EvenRTbl, pu8EvenGTbl, pu8EvenBTbl,
                        pu8OddRTbl, pu8OddGTbl, pu8OddBTbl);

    _write_lineod_tbl_sram4(
                        pu8EvenRTbl, pu8EvenGTbl, pu8EvenBTbl,
                        pu8OddRTbl, pu8OddGTbl, pu8OddBTbl);

    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                0,
                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_CH_SEL_003C);
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                0,
                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_SEL_003C);

    _set_pcid_enable(bPcidEnable);      // PCID recover
    _set_lineod_enable(bLineODEnable);  // LINE OD recover
    return TRUE;
}

bool _write_lineod_single_tbl_sram1(uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram1
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_10; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_10; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 + u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu16RTbl[u16lutIndex],
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu16GTbl[u16lutIndex],
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu16BTbl[u16lutIndex],
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_10),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM1|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    return TRUE;
}

bool _write_lineod_single_tbl_sram2(uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram2
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_10; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_9; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 + 1;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu16RTbl[u16lutIndex],
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R

            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu16GTbl[u16lutIndex],
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G

            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu16BTbl[u16lutIndex],
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM2|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_single_tbl_sram3(uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram3
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_10; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            (u16CodeTableY) * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 +
                            (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9);

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu16RTbl[u16lutIndex],
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu16GTbl[u16lutIndex],
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu16BTbl[u16lutIndex],
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_10),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM3|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                                REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    return TRUE;
}

bool _write_lineod_single_tbl_sram4(uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl)
{
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16Timeout = 0, u16lutIndex = 0;

    //sram4
    for (u16CodeTableY = 0; u16CodeTableY < LINE_OD_TABLE_ROW_9; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < LINE_OD_TABLE_COL_9; u16CodeTableX++)
        {
            u16lutIndex = u16CodeTableX * 2 +
                            u16CodeTableY * (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) * 2 +
                            (LINE_OD_TABLE_COL_10+LINE_OD_TABLE_COL_9) +1;

            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_DEBUG("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            W2BYTEMSK(REG_0008_DEMURA_BKA377_V004,
                        pu16RTbl[u16lutIndex],
                        REG_0008_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_R_DATA_0008); //R
            W2BYTEMSK(REG_000C_DEMURA_BKA377_V004,
                        pu16GTbl[u16lutIndex],
                        REG_000C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_G_DATA_000C); //G
            W2BYTEMSK(REG_0010_DEMURA_BKA377_V004,
                        pu16BTbl[u16lutIndex],
                        REG_0010_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_B_DATA_0010); //B

            W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                        u16CodeTableX+(u16CodeTableY*LINE_OD_TABLE_COL_9),
                        REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_ADDR_0004); //sram address
            TCON_W2BYTEMSK(REG_003C_DEMURA_BKA377_V004>>1,
                            TRIG_WRITE_SRAM|SELECT_SRAM4|SELECT_ALL_CHANNEL_RGB,
                            W_RGB_MASK); //sel RGB

            while (R2BYTEMSK(REG_003C_DEMURA_BKA377_V004,
                    REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_WR_003C) &&
                    (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }

    return TRUE;
}

bool _write_lineod_single_tbl(
                            uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl,
                            uint8_t u8LutSramSeg)
{
    bool bPcidEnable = FALSE, bLineODEnable = FALSE;

    TCON_CHECK_PARAMETER_NULL(pu16RTbl);
    TCON_CHECK_PARAMETER_NULL(pu16GTbl);
    TCON_CHECK_PARAMETER_NULL(pu16BTbl);

    bPcidEnable = _is_pcid_enable();
    bLineODEnable = _is_lineod_enable();

    if (!bPcidEnable || !bLineODEnable)
    {
        TCON_ERROR("Pcid enable=%d lineOD enable=%d\n", bPcidEnable, bLineODEnable);
        return FALSE;
    }

    _set_pcid_enable(false);        // PCID Disable
    _set_lineod_enable(false);      // LINE OD Disable

    if (PNL_LINEOD_SHARE_SRAM == TRUE)
    {
        TCON_W2BYTEMSK(0x1133A8, _BIT(2), _BIT(2));//bk1133_54 [2] set 1 for share SRAM with FCFC
        TCON_W2BYTEMSK(0x113308, _BIT(0), _BIT(0));//bk1133_04 [0] set 1 for share SRAM with LineOD
        TCON_W2BYTEMSK(0x1133AC, 0, _BIT(2));//bk1133_56 [2] set 0 for 1p path
    }

    W2BYTEMSK(REG_0004_DEMURA_BKA377_V004,
                u8LutSramSeg,
                REG_0004_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_SEG_0004); //sel PCID lut sram segment

    _write_lineod_single_tbl_sram1(pu16RTbl, pu16GTbl, pu16BTbl);
    _write_lineod_single_tbl_sram2(pu16RTbl, pu16GTbl, pu16BTbl);
    _write_lineod_single_tbl_sram3(pu16RTbl, pu16GTbl, pu16BTbl);
    _write_lineod_single_tbl_sram4(pu16RTbl, pu16GTbl, pu16BTbl);

    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, 0, REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_CH_SEL_003C);
    W2BYTEMSK(REG_003C_DEMURA_BKA377_V004, 0, REG_003C_DEMURA_BKA377_V004_REG_PCID_LUT_SRAM_SEL_003C);

    _set_pcid_enable(bPcidEnable);      // PCID recover
    _set_lineod_enable(bLineODEnable);  // LINE OD recover

    return TRUE;
}

bool _dump_lineod_gain_tbl(uint8_t* pu8ODGainTbl, uint16_t u16GainTableSize)
{
    bool bGainEnable = FALSE;
    uint16_t u16TableIndex = 0, u16Timeout = 0;
    uint16_t u16HWGainSize = LINE_OD_HW_GAIN_SIZE_13X9;//hw size
    uint16_t u16HWGainIndex = 0;

    TCON_CHECK_PARAMETER_NULL(pu8ODGainTbl);

    //Note:REG_SC_EXT_BK2D/332D/(Bank =104F Sub-Bank=2D)/0x302f00 0x2d --> Bank = A02F Sub-Bank=A8/0xA3A8

    bGainEnable = R2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                                REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_EN_0004);

    TCON_INFO("Gain table size=%d\n", u16GainTableSize);

    if (u16GainTableSize == LINE_OD_GAIN_TABLE_SIZE_13X9)
    {
        //fill the 13x9 data on 13x9 hw address
        for (u16TableIndex = 0; u16TableIndex < LINE_OD_GAIN_TABLE_SIZE_13X9; u16TableIndex++)
        {
            //gain disable
            W2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                        0,
                        REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_EN_0004);

            //[6:0]lineOD gain address
            W2BYTEMSK(REG_0008_LINEOD_BKA3A8_V004,
                        u16TableIndex,
                        REG_0008_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_ADR_0008);

            //[7:0]lineOD write data3.5
            W2BYTEMSK(REG_000C_LINEOD_BKA3A8_V004,
                        pu8ODGainTbl[u16TableIndex],
                        REG_000C_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_DATA_000C);

            //[3]wr enable
            W2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                        1,
                        REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_EN_0004);

            while(R2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                            REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_EN_0004) &&
                            (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    else if (u16GainTableSize == LINE_OD_GAIN_TABLE_SIZE_4X4)
    {
        // fill the 4x4 data on 13x9 hw address(other ignore)
        for (u16TableIndex = 0; u16TableIndex < LINE_OD_GAIN_TABLE_SIZE_4X4; u16TableIndex++)
        {
            u16HWGainIndex = ((u16TableIndex / LINE_OD_GAIN_TABLE_COL_4) * LINE_OD_GAIN_TABLE_COL_13) +
                                (u16TableIndex % LINE_OD_GAIN_TABLE_COL_4);

            //gain disable
            W2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                        0,
                        REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_EN_0004);

            if (u16HWGainIndex < u16HWGainSize)
            {
                //[6:0]lineOD gain address
                W2BYTEMSK(REG_0008_LINEOD_BKA3A8_V004,
                            u16HWGainIndex,
                            REG_0008_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_ADR_0008);

                //[7:0]lineOD write data3.5
                W2BYTEMSK(REG_000C_LINEOD_BKA3A8_V004,
                            pu8ODGainTbl[u16TableIndex],
                            REG_000C_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_DATA_000C);
            }
            W2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                        1,
                        REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_EN_0004); //[3]wr enable

            while(R2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                            REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_WR_EN_0004) &&
                            (u16Timeout < MAX_DELAY_TIME))
            {
               mdelay(1);
               u16Timeout++;
            }
            u16Timeout = 0;
        }
    }
    else
    {
        TCON_DEBUG("Table size=%d not correct\n", u16GainTableSize);
    }

    //gain enable status recovery
    W2BYTEMSK(REG_0004_LINEOD_BKA3A8_V004,
                (bGainEnable ? 1 : 0), REG_0004_LINEOD_BKA3A8_V004_REG_PCID_LINEOD_GAIN_EN_0004);
    return TRUE;
}

// PCID Auto Download Format
// 181 * 3 * 32 bytes = 0x43E0 (R/G/B channels)
//|<--------------32 bytes------------->|
//    SRAM1/SRAM4        SRAM2/SRAM3
//   lut0 .... lut9     lut0 .... lut9
//0   0   ....  0         1  ....  1
//.
//89  322 ....  322      359 .... 359
//                      ---------------
//90  358 ....  358       19 ....  19
//.
//99  360 ....  360       37 ....  37
//--------------------
//100 20  ....   20       57 ....  57
//.
//179 338 ....   338     341 ....  341
//                       ---------------
//180 340 ....   340       x ....  x
//--------------------------------------
bool _fill_to_adl_sram14_data(PST_SRAM_DATA_FORMAT_INFO pstInfo)
{
    bool bRet = TRUE;

    TCON_CHECK_PARAMETER_NULL(pstInfo);

    ST_PCID_12BIT_DRAM_FORMAT *pstData = pstInfo->pstData;
    uint16_t u8TableIdx = pstInfo->u8TableIdx;

    if (0 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut0 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut0 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut0 = pstInfo->u16BVal;
    }
    else if (1 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut1 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut1 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut1 = pstInfo->u16BVal;
    }
    else if (2 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut2 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut2 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut2 = pstInfo->u16BVal;
    }
    else if (3 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut3 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut3 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut3 = pstInfo->u16BVal;
    }
    else if (4 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut4 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut4 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut4 = pstInfo->u16BVal;
    }
    else if (5 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut5 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut5 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut5 = pstInfo->u16BVal;
    }
    else if (6 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut6 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut6 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut6 = pstInfo->u16BVal;
    }
    else if (7 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut7 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut7 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut7 = pstInfo->u16BVal;
    }
    else if (8 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut8 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut8 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut8 = pstInfo->u16BVal;
    }
    else if (9 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM1_lut9 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM1_lut9 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM1_lut9 = pstInfo->u16BVal;
    }

    return bRet;
}

bool _fill_to_adl_sram23_data(PST_SRAM_DATA_FORMAT_INFO pstInfo)
{
    bool bRet = TRUE;

    TCON_CHECK_PARAMETER_NULL(pstInfo);

    ST_PCID_12BIT_DRAM_FORMAT *pstData = pstInfo->pstData;
    uint16_t u8TableIdx = pstInfo->u8TableIdx;

    if (0 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut0 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut0 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut0 = pstInfo->u16BVal;
    }
    else if (1 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut1 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut1 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut1 = pstInfo->u16BVal;
    }
    else if (2 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut2 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut2 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut2 = pstInfo->u16BVal;
    }
    else if (3 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut3 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut3 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut3 = pstInfo->u16BVal;
    }
    else if (4 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut4 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut4 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut4 = pstInfo->u16BVal;
    }
    else if (5 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut5 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut5 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut5 = pstInfo->u16BVal;
    }
    else if (6 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut6 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut6 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut6 = pstInfo->u16BVal;
    }
    else if (7 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut7 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut7 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut7 = pstInfo->u16BVal;
    }
    else if (8 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut8 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut8 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut8 = pstInfo->u16BVal;
    }
    else if (9 == u8TableIdx)
    {
        pstData[pstInfo->u16RIndex].u16SRAM2_lut9 = pstInfo->u16RVal;
        pstData[pstInfo->u16GIndex].u16SRAM2_lut9 = pstInfo->u16GVal;
        pstData[pstInfo->u16BIndex].u16SRAM2_lut9 = pstInfo->u16BVal;
    }

    return bRet;
}

bool _arrange_adl_pcid_settbl(
                        ST_PCID_12BIT_DRAM_FORMAT* pstPCID_ADL_CMD,
                        uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl,
                        uint16_t u16TableSize, uint8_t u8TableIdx)
{
    bool bRet = TRUE;
    uint16_t u16CodeTableX = 0;
    uint16_t u16CodeTableY = 0;
    uint16_t u16lutIndex  = 0;
    uint16_t u16Sram1Idx = 0, u16Sram2Idx = 0, u16Sram3Idx = 0, u16Sram4Idx = 0;
    uint16_t u16ROW_MAX = 0, u16COL_MAX = 0;
    uint16_t u16SRAM3_Offset = 0, u16SRAM4_Offset = 0, u16Channel_Offset = 0;

    TCON_CHECK_PARAMETER_NULL(pstPCID_ADL_CMD);
    TCON_CHECK_PARAMETER_NULL(pu16RTbl);
    TCON_CHECK_PARAMETER_NULL(pu16GTbl);
    TCON_CHECK_PARAMETER_NULL(pu16BTbl);

    ST_SRAM_DATA_FORMAT_INFO stInfo;
    memset(&stInfo, 0, sizeof(ST_SRAM_DATA_FORMAT_INFO));
    stInfo.pstData = pstPCID_ADL_CMD;
    stInfo.u8TableIdx = u8TableIdx;

    u16ROW_MAX = (u16TableSize == LINE_OD_TABLE_SIZE_19X19) ? LINE_OD_TABLE_ROW_10 : LINE_OD_TABLE_ROW_9;
    u16COL_MAX = (u16TableSize == LINE_OD_TABLE_SIZE_19X19) ? LINE_OD_TABLE_COL_10 : LINE_OD_TABLE_COL_9;
    u16Channel_Offset = (u16TableSize == LINE_OD_TABLE_SIZE_19X19) ?
                                    XC_AUTODOWNLOAD_PCID_19X19_OFFSET : XC_AUTODOWNLOAD_PCID_17X17_OFFSET;
    u16SRAM3_Offset = u16ROW_MAX*(u16COL_MAX - 1);
    u16SRAM4_Offset = u16ROW_MAX*u16COL_MAX;

    for (u16CodeTableY = 0; u16CodeTableY < u16ROW_MAX; u16CodeTableY++)
    {
        for (u16CodeTableX = 0; u16CodeTableX < u16COL_MAX; u16CodeTableX++)
        {
            //sram1
            u16lutIndex = u16CodeTableX * 2 + u16CodeTableY * (u16COL_MAX*2-1) * 2;
            if (u16lutIndex >= LINE_OD_TABLE_SIZE_19X19)
            {
                TCON_ERROR("lutIndex error=%d\n", u16lutIndex);
                return FALSE;
            }

            stInfo.u16RIndex = u16Sram1Idx;
            stInfo.u16GIndex = u16Sram1Idx + u16Channel_Offset;
            stInfo.u16BIndex = u16Sram1Idx + u16Channel_Offset*2;
            stInfo.u16RVal = pu16RTbl[u16lutIndex];
            stInfo.u16GVal = pu16GTbl[u16lutIndex];
            stInfo.u16BVal = pu16BTbl[u16lutIndex];

            _fill_to_adl_sram14_data(&stInfo);
            u16Sram1Idx++;

            //sram2
            if (u16CodeTableX < u16COL_MAX-1)
            {
                u16lutIndex = u16CodeTableX * 2 + u16CodeTableY * (u16COL_MAX*2-1) * 2 + 1;

                stInfo.u16RIndex = u16Sram2Idx;
                stInfo.u16GIndex = u16Sram2Idx + u16Channel_Offset;
                stInfo.u16BIndex = u16Sram2Idx + u16Channel_Offset*2;
                stInfo.u16RVal = pu16RTbl[u16lutIndex];
                stInfo.u16GVal = pu16GTbl[u16lutIndex];
                stInfo.u16BVal = pu16BTbl[u16lutIndex];

                _fill_to_adl_sram23_data(&stInfo);
                u16Sram2Idx++;
            }

            //sram3
            if (u16CodeTableY < u16ROW_MAX-1)
            {
                u16lutIndex = u16CodeTableX * 2 + (u16CodeTableY) * (u16COL_MAX*2-1) * 2 + (u16COL_MAX*2-1);

                stInfo.u16RIndex = u16Sram3Idx + u16SRAM3_Offset;
                stInfo.u16GIndex = u16Sram3Idx + u16SRAM3_Offset + u16Channel_Offset;
                stInfo.u16BIndex = u16Sram3Idx + u16SRAM3_Offset + u16Channel_Offset*2;
                stInfo.u16RVal = pu16RTbl[u16lutIndex];
                stInfo.u16GVal = pu16GTbl[u16lutIndex];
                stInfo.u16BVal = pu16BTbl[u16lutIndex];

                _fill_to_adl_sram23_data(&stInfo);
                u16Sram3Idx++;
            }

            //sram4
            if (u16CodeTableX < u16COL_MAX-1 && u16CodeTableY < u16ROW_MAX-1)
            {
                u16lutIndex = u16CodeTableX * 2 + u16CodeTableY * (u16COL_MAX*2-1) * 2 + (u16COL_MAX*2-1) +1;

                stInfo.u16RIndex = u16Sram4Idx + u16SRAM4_Offset;
                stInfo.u16GIndex = u16Sram4Idx + u16SRAM4_Offset+ u16Channel_Offset;
                stInfo.u16BIndex = u16Sram4Idx + u16SRAM4_Offset+ u16Channel_Offset*2;
                stInfo.u16RVal = pu16RTbl[u16lutIndex];
                stInfo.u16GVal = pu16GTbl[u16lutIndex];
                stInfo.u16BVal = pu16BTbl[u16lutIndex];

                _fill_to_adl_sram14_data(&stInfo);
                u16Sram4Idx++;
            }

        }
    }

    return bRet;
}

bool _prepare_adl_data_format(
                                ST_PCID_12BIT_DRAM_FORMAT* pstPCID_ADL_CMD,
                                uint16_t* pu16RTbl, uint16_t* pu16GTbl, uint16_t* pu16BTbl,
                                uint16_t u16TableSize, uint8_t u8TableNum)
{
    bool bRet = TRUE;

    TCON_CHECK_PARAMETER_NULL(pstPCID_ADL_CMD);
    TCON_CHECK_PARAMETER_NULL(pu16RTbl);
    TCON_CHECK_PARAMETER_NULL(pu16GTbl);
    TCON_CHECK_PARAMETER_NULL(pu16BTbl);

    if (u8TableNum < LINE_OD_TOTAL_LUT_NUM)
    {
        _arrange_adl_pcid_settbl(pstPCID_ADL_CMD, pu16RTbl, pu16GTbl, pu16BTbl, u16TableSize, u8TableNum);
    }
    else
    {
        TCON_ERROR("Table Num=%d over the range(=%d)\n", u8TableNum, LINE_OD_TOTAL_LUT_NUM);
    }

    return bRet;
}

uint8_t _get_table_num(PST_LINE_OD_SUB_HEADER pstHeader, bool bTableSeparate)
{
    uint8_t u8TableNum = 0;

    if (!pstHeader)
    {
        TCON_ERROR("input pstHeader parameter is null\n");
        return u8TableNum;
    }

    if ((pstHeader->u16Version > 2) && (pstHeader->u8TableBit == LINE_OD_TABLE_UNIT_SIZE_12))
    {
        if (pstHeader->u8RegionNumber == 0)
        {
            u8TableNum = LINE_OD_REG_NUM_1X1;
        }
        else if (pstHeader->u8RegionNumber == 1)
        {
            u8TableNum = LINE_OD_REG_NUM_2X8;
        }
        else if (pstHeader->u8RegionNumber == 2)
        {
            u8TableNum  = LINE_OD_REG_NUM_4X4;
        }
        else if (pstHeader->u8RegionNumber == 3)
        {
            u8TableNum= LINE_OD_REG_NUM_10X1;
        }
        else
        {
            u8TableNum = LINE_OD_REG_NUM_1X1;
        }
    }
    else
    {
        if (pstHeader->u16Version > 1)//after verion 1 region number define different
        {
             if (pstHeader->u8RegionNumber == 0)
            {
                u8TableNum= LINE_OD_REG_NUM_1X1;
            }
            else if (pstHeader->u8RegionNumber == 1)
            {
                u8TableNum= LINE_OD_REG_NUM_2X8;
            }
            else if (pstHeader->u8RegionNumber == 2)
            {
                u8TableNum= LINE_OD_REG_NUM_4X4;
            }
            else
            {
                u8TableNum= LINE_OD_REG_NUM_1X1;
            }
        }
        else
        {
            u8TableNum = (bTableSeparate ? pstHeader->u8RegionNumber : 1);
        }
    }
    TCON_DEBUG("table num=%d\n", u8TableNum);

    return u8TableNum;
}

bool _gain_table_setting(
                        PST_LINE_OD_SUB_HEADER pstHeader, uint8_t* pu8TconTab, uint16_t u16TargetIndex)
{
    bool bRet = TRUE;
    uint8_t *pu8Gain = NULL;
    uint16_t u16GainTableSize = 0;

    TCON_CHECK_PARAMETER_NULL(pstHeader);
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    if (pstHeader->u16Version > 1)
    {
        if (pstHeader->u8GainMatrix == 0)
        {
            u16GainTableSize = LINE_OD_GAIN_TABLE_SIZE_4X4;
        }
        else if (pstHeader->u8GainMatrix == 1)
        {
            u16GainTableSize = LINE_OD_GAIN_TABLE_SIZE_13X9;
        }
        else
        {
            u16GainTableSize = LINE_OD_GAIN_TABLE_SIZE_13X9;
        }

        PNL_MALLOC_MEM(pu8Gain, u16GainTableSize*sizeof(uint8_t), bRet);
        if (bRet)
        {
            memcpy(pu8Gain, (pu8TconTab + u16TargetIndex), sizeof(uint8_t)*u16GainTableSize);
            bRet = _dump_lineod_gain_tbl(pu8Gain, u16GainTableSize);
        }
        PNL_FREE_MEM(pu8Gain);
    }

    return bRet;
}

bool _lineod_table_setting_ver2(
                            PST_LINE_OD_SUB_HEADER pstHeader,
                            uint8_t* pu8TconTab,
                            bool bChannelSeparate, bool bTableSeparate,
                            uint16_t u16TableSize)
{
    bool bRet = TRUE;
    uint16_t u16TargetIndex = 0;
    uint8_t u8TabIndex, u8TableNum;
    uint16_t *pu16RLut = NULL;
    uint16_t *pu16GLut = NULL;
    uint16_t *pu16BLut = NULL;
#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_PCID_AUTODOWNLOAD == TRUE)
    uint32_t u32AdlSize = 0;
    ST_PCID_12BIT_DRAM_FORMAT *pstPCID_ADL_CMD = NULL;
#else
    uint8_t u8LutSramSeg;
#endif

    TCON_FUNC_ENTER();

    PNL_MALLOC_MEM(pu16RLut, u16TableSize*sizeof(uint16_t), bRet);
    PNL_MALLOC_MEM(pu16GLut, u16TableSize*sizeof(uint16_t), bRet);
    PNL_MALLOC_MEM(pu16BLut, u16TableSize*sizeof(uint16_t), bRet);

    if (!bRet)
    {
        goto finally;
    }

    u16TargetIndex = sizeof(ST_LINE_OD_SUB_HEADER);
    u8TableNum = _get_table_num(pstHeader, bTableSeparate);

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_PCID_AUTODOWNLOAD == TRUE)
    if (u16TableSize == LINE_OD_TABLE_SIZE_17X17)
    {
        u32AdlSize = XC_AUTODOWNLOAD_PCID_17X17_OFFSET;// 32 bytes * 145 * 3
    }
    else
    {
        u32AdlSize = XC_AUTODOWNLOAD_PCID_19X19_OFFSET;// 32 bytes * 181 * 3
    }

    u32AdlSize = u32AdlSize * sizeof(ST_PCID_12BIT_DRAM_FORMAT) * PCID_RGB_CHANNEL;

    if (!pstPCID_ADL_CMD)
    {
        PNL_MALLOC_MEM(pstPCID_ADL_CMD, u32AdlSize, bRet);
        if (!bRet)
        {
            TCON_ERROR("Autodownload allocate mem fail\n");
            goto finally;
        }
    }
    memset(pstPCID_ADL_CMD, 0, u32AdlSize);
#endif

    u8TabIndex = 0;
    while (u8TabIndex < u8TableNum)
    {
        if (bChannelSeparate)
        {
            memcpy(pu16RLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //R
            u16TargetIndex += u16TableSize*2;
            memcpy(pu16GLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //G
            u16TargetIndex += u16TableSize*2;
            memcpy(pu16BLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //B
            u16TargetIndex += u16TableSize*2;
        }
        else
        {
            memcpy(pu16RLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //R
            memcpy(pu16GLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //G
            memcpy(pu16BLut, (pu8TconTab+u16TargetIndex), sizeof(uint16_t)*u16TableSize); //B
            u16TargetIndex += u16TableSize*2;
        }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_PCID_AUTODOWNLOAD == TRUE)
        bRet = _prepare_adl_data_format(pstPCID_ADL_CMD,
                                            pu16RLut, pu16GLut, pu16BLut,
                                            u16TableSize, u8TabIndex);
        if (!bRet)
        {
            TCON_ERROR("_prepare_adl_data_format u8TabIndex=%d \n.", u8TabIndex);
        }
#else
        u8LutSramSeg = u8TabIndex;
        bRet = _write_lineod_single_tbl(pu16RLut, pu16GLut, pu16BLut, u8LutSramSeg);
        if (!bRet)
        {
            TCON_ERROR("_dump_lineod_single_tbl return fail\n.");
        }
#endif
        u8TabIndex += 1;
    }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_PCID_AUTODOWNLOAD == TRUE)
    if (!KHal_XC_WriteAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID,
                                    (uint8_t *)pstPCID_ADL_CMD,
                                    u32AdlSize, NULL))
    {
        TCON_ERROR("PCID, Write ADL Fail!!! \n");
    }
    else
    {
        if (!KHal_XC_FireAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_PCID))
        {
            TCON_ERROR("PCID, Fire ADL Fail!!! \n");
        }
        else
        {
            TCON_DEBUG("Use auto download to set line OD successfully.\n");
        }
    }
    PNL_FREE_MEM(pstPCID_ADL_CMD);
#endif

    bRet &= _gain_table_setting(pstHeader, pu8TconTab, u16TargetIndex);

finally:
    //free allocated memory
    PNL_FREE_MEM(pu16RLut);
    PNL_FREE_MEM(pu16GLut);
    PNL_FREE_MEM(pu16BLut);

    TCON_FUNC_EXIT(bRet);

    return bRet;
}

bool _lineod_table_setting_ver1(
                            PST_LINE_OD_SUB_HEADER pstHeader,
                            uint8_t* pu8TconTab,
                            bool bChannelSeparate, bool bTableSeparate,
                            uint16_t u16TableSize)
{
    bool bRet = TRUE;
    uint8_t u8LutSramSeg;
    uint16_t u16TargetIndex = 0;
    uint8_t u8TabIndex, u8TableNum;
    uint8_t *pu8EvenRLut = NULL;
    uint8_t *pu8EvenGLut = NULL;
    uint8_t *pu8EvenBLut = NULL;
    uint8_t *pu8OddRLut = NULL;
    uint8_t *pu8OddGLut = NULL;
    uint8_t *pu8OddBLut = NULL;

    TCON_FUNC_ENTER();

    PNL_MALLOC_MEM(pu8EvenRLut, u16TableSize*sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8EvenGLut, u16TableSize*sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8EvenBLut, u16TableSize*sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8OddRLut, u16TableSize*sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8OddGLut, u16TableSize*sizeof(uint8_t), bRet);
    PNL_MALLOC_MEM(pu8OddBLut, u16TableSize*sizeof(uint8_t), bRet);

    if (!bRet)
    {
        goto finally;
    }

    u16TargetIndex = sizeof(ST_LINE_OD_SUB_HEADER);
    u8TableNum = _get_table_num(pstHeader, bTableSeparate);
    u8TabIndex = 0;

    while (u8TabIndex < u8TableNum)
    {
        if (bChannelSeparate)
        {
            //even lut
            memcpy(pu8EvenRLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //R
            u16TargetIndex += u16TableSize;
            memcpy(pu8EvenGLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //G
            u16TargetIndex += u16TableSize;
            memcpy(pu8EvenBLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //B
            u16TargetIndex += u16TableSize;

            if ((u8TabIndex + 1) != u8TableNum) //avoid to copy last odd lut table that doesn't exist
            {
                //odd lut
                memcpy(pu8OddRLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //R
                u16TargetIndex += u16TableSize;
                memcpy(pu8OddGLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //G
                u16TargetIndex += u16TableSize;
                memcpy(pu8OddBLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //B
                u16TargetIndex += u16TableSize;
            }
        }
        else
        {
            //even lut
            memcpy(pu8EvenRLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //R
            memcpy(pu8EvenGLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //G
            memcpy(pu8EvenBLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //B
            u16TargetIndex += u16TableSize;

            if ((u8TabIndex + 1) != u8TableNum) //avoid to copy last odd lut table that doesn't exist
            {
                //odd lut
                memcpy(pu8OddRLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //R
                memcpy(pu8OddGLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //G
                memcpy(pu8OddBLut, (pu8TconTab+u16TargetIndex), sizeof(uint8_t)*u16TableSize); //B
                u16TargetIndex += u16TableSize;
            }
        }

        u8LutSramSeg = u8TabIndex/2;
        bRet = _write_lineod_tbl(
                            pu8EvenRLut, pu8EvenGLut, pu8EvenBLut,
                            pu8OddRLut, pu8OddGLut, pu8OddBLut, u8LutSramSeg);

        u8TabIndex += 2;
    }

    bRet &= _gain_table_setting(pstHeader, pu8TconTab, u16TargetIndex);

finally:
    //free allocated memory
    PNL_FREE_MEM(pu8EvenRLut);
    PNL_FREE_MEM(pu8EvenGLut);
    PNL_FREE_MEM(pu8EvenBLut);
    PNL_FREE_MEM(pu8OddRLut);
    PNL_FREE_MEM(pu8OddGLut);
    PNL_FREE_MEM(pu8OddBLut);

    TCON_FUNC_EXIT(bRet);

    return bRet;
}

bool _lineod_table_setting_proc(uint8_t* pu8TconTab)
{
    PST_LINE_OD_SUB_HEADER pstLineODSubHdr = NULL;
    bool bRetLut = TRUE, bChannelSeparate = FALSE, bTableSeparate = FALSE;
    uint16_t u16TableSize;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    pstLineODSubHdr = (PST_LINE_OD_SUB_HEADER)pu8TconTab;
    bChannelSeparate = pstLineODSubHdr->u8SeparateFlag & BIT(0);
    bTableSeparate = (pstLineODSubHdr->u8SeparateFlag & BIT(1)) >> 1;

    TCON_DEBUG("\nVersion=%d LineOD_en=%d\n" \
            "channel seperate=%d, table separate=%d\n" \
            "year=%d month=%d day=%d minute=%d\n" \
            "table matrix=%d region number=%d\n",
            pstLineODSubHdr->u16Version, pstLineODSubHdr->u8LineOD_en,
            bChannelSeparate, bTableSeparate,
            (pstLineODSubHdr->u32YearMonthDayMinute & 0x1F00000)>>20,
            (pstLineODSubHdr->u32YearMonthDayMinute & 0xF0000)>>16,
            (pstLineODSubHdr->u32YearMonthDayMinute & 0xF800)>>11,
            (pstLineODSubHdr->u32YearMonthDayMinute & 0x7FF),
            pstLineODSubHdr->u8TableMatrix, pstLineODSubHdr->u8RegionNumber);

    if (pstLineODSubHdr->u16Version > 2)//after verion 2 add new member table unit
    {
        //Table Bit: 0: 8 bits, 1: 12 bits
        TCON_DEBUG("table bit=%d\n", pstLineODSubHdr->u8TableBit);
    }

    if (pstLineODSubHdr->u16Version > 1)
    {
        //Gain Matrix Size: 0: 4x4, 1: 13x9
        TCON_DEBUG("gain matrix=%d\n", pstLineODSubHdr->u8GainMatrix);
        TCON_DEBUG("gain region number=%d\n", pstLineODSubHdr->u8GainRegion);
    }
    u16TableSize = (pstLineODSubHdr->u8TableMatrix ? LINE_OD_TABLE_SIZE_17X17 : LINE_OD_TABLE_SIZE_19X19);
    TCON_DEBUG("table size=%d\n", u16TableSize);

    if ((pstLineODSubHdr->u16Version > 2) && (pstLineODSubHdr->u8TableBit == LINE_OD_TABLE_UNIT_SIZE_12))
    {
        bRetLut &= _lineod_table_setting_ver2(
                                    pstLineODSubHdr,
                                    pu8TconTab,
                                    bChannelSeparate, bTableSeparate, u16TableSize);
    }
    else
    {
        bRetLut &= _lineod_table_setting_ver1(
                                    pstLineODSubHdr,
                                    pu8TconTab,
                                    bChannelSeparate, bTableSeparate, u16TableSize);
    }

    TCON_FUNC_EXIT(bRetLut);

    return bRetLut;
}

bool _lineod_reg_setting_proc(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr = 0;
    uint8_t u8Mask = 0;
    uint8_t u8Value = 0;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = ((pu8TconTab[u32tabIdx]<<24) + (pu8TconTab[(u32tabIdx+1)]<<16) +
                    (pu8TconTab[(u32tabIdx +2)]<<8) + pu8TconTab[(u32tabIdx +3)]) & 0x7FFFFFFF;
        u8Mask  = pu8TconTab[(u32tabIdx+4)] & 0xFF;
        u8Value = pu8TconTab[(u32tabIdx+5)] & 0xFF;

        TCON_INFO("[addr=%04tx, msk=%02x, val=%02x] \n", (ptrdiff_t)u32Addr, u8Mask, u8Value);
        TCON_W1BYTEMSK(u32Addr, u8Value, u8Mask);

        u32tabIdx = u32tabIdx + 6;
    }

    TCON_FUNC_EXIT(TRUE);
    return TRUE;
}

bool mtk_tcon_lineod_setting(struct udevice *dev)
{
    bool bRet = TRUE;
    loff_t data_len = 0;
    unsigned char *pdata_buf = NULL;
    st_tcon_tab_info stInfo;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(dev);

    memset(&stInfo, 0, sizeof(st_tcon_tab_info));

    //do PCID setting process
    /***
      PCID mainly contains two major functions pixelOD / LineOD,
      which is mainly used to compensate for insufficient charging of CELL MAPPING,
      thereby reducing the crosstalk phenomenon
    ***/
    if (is_tcon_data_exist(&pdata_buf, &data_len))
    {
        stInfo.pu8Table = pdata_buf;
        stInfo.u8TconType = E_TCON_TAB_TYPE_PCID;

        if (get_tcon_dump_table(&stInfo))
        {
            bRet &= _pcid_setting_proc(stInfo.pu8Table);
        }
    }

    //do lineod reg. setting process
    if (is_tcon_data_exist(&pdata_buf, &data_len))
    {
        stInfo.pu8Table = pdata_buf;
        stInfo.u8TconType = E_TCON_TAB_TYPE_LINE_OD_REG;

        if (get_tcon_dump_table(&stInfo))
        {
            bRet &= _lineod_reg_setting_proc(stInfo.pu8Table, stInfo.u16RegCount);
        }
    }

    //do lineod table setting process
    if (is_tcon_data_exist(&pdata_buf, &data_len))
    {
        stInfo.pu8Table = pdata_buf;
        stInfo.u8TconType = E_TCON_TAB_TYPE_LINE_OD_TABLE;

        if (get_tcon_dump_table(&stInfo))
        {
            bRet &= _lineod_table_setting_proc(stInfo.pu8Table);
        }
    }

    TCON_FUNC_EXIT(bRet);
    return bRet;
}
