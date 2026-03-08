// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon overdrive driver
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
#include <linux/math64.h>

#include "coda/hwreg_common.h"
#include "coda/PAFRC_BKA324_V004.h"
#include "coda/OD_1ST_BKA336_V004.h"
#include "coda/OD_2ND_BKA337_V004.h"
#include "coda/OD_3RD_BKA338_V004.h"
#include "coda/SCTCON_BKA4F7_V004.h"
#include "coda/SCTCON_MISC_BKA3E0_V005.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_tcon_od.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_autodownload.h"

// Debug related

#define PNL_RGB_LUT_CHECK(R, G, B) ((R) == NULL || (G) == NULL || (B) == NULL)

#define BYTE_PER_WORD      (32)
#define OD_BYTE_PER_WORD   (32)

#define TCON_BIN_VERSION_RGBW (1)
#define TCON_BIN_HEADER_BYTE_NUBMER(byte_n) (byte_n)

#define TCON_PATH_DEMURA_OD_PGA  (0)
#define TCON_PATH_DEMURA_PGA_OD  (1)
#define TCON_PATH_OD_DEMURA_PGA  (2)
#define TCON_PATH_OD_PGA_DEMURA  (3)
#define TCON_PATH_PGA_DEMURA_OD  (4)
#define TCON_PATH_PGA_OD_DEMURA  (5)

#define TCON_REGISTERTYPE_1_BYTES   1
#define TCON_REGISTERTYPE_6_BYTES   6
#define TCON_REGISTERTYPE_4_BYTES   4
#define TCON_REGISTERTYPE_POWERSEQENCE_9_BYTES   9
#define TCON_REGISTERTYPE_POWERSEQENCE_7_BYTES   7
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES   15
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES   11
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_14_BYTES   14

//Polarity mode
//0: 1 pixel
//     reg_eva_polarity_lut0 = 0xE38
//     reg_eva_polarity_lut1 = 0x1C7
//     reg_eva_polarity_lut2 = 0xE38
//     reg_eva_polarity_lut3 = 0x1C7
//1: 1 subpixel
//     reg_eva_polarity_lut0 = 0xAAA
//     reg_eva_polarity_lut1 = 0x555
//     reg_eva_polarity_lut2 = 0xAAA
//     reg_eva_polarity_lut3 = 0x555
//2: 2 pixel (RBG)
//     reg_eva_polarity_lut0 = 0xCCC
//     reg_eva_polarity_lut1 = 0x333
//     reg_eva_polarity_lut2 = 0xCCC
//     reg_eva_polarity_lut3 = 0x333
//3: 2 subpixel (BGR) = 2 pixel (RBG) + RBswap enable
//     reg_eva_polarity_lut0 = 0x7A1
//     reg_eva_polarity_lut1 = 0x85E
//     reg_eva_polarity_lut2 = 0x7A1
//     reg_eva_polarity_lut3 = 0x85E
#define EVA_POLARITY_LUT0_MODE_0 (0xE38)
#define EVA_POLARITY_LUT0_MODE_1 (0xAAA)
#define EVA_POLARITY_LUT0_MODE_2 (0xCCC)
#define EVA_POLARITY_LUT0_MODE_3 (0x7A1)

// OD
#define MAX_OVERDRIVER_TABLE 1089
#define OVERDRIVER_TABLE_SIZE_33X32    1056
#define OVERDRIVER_TABLE_SIZE_33X33    1089
//old od table
#define OVERDRIVER_TABLE_SIZE_33X32    1056
//new od table
#define OVERDRIVER_TABLE_SIZE_33X33    1089
#define MAX_OVERDRIVER_TABLE 1089
#define OVERDRIVER_SRAM1_SIZE (289)
#define OVERDRIVER_SRAM2_SIZE (272)
#define OVERDRIVER_SRAM3_SIZE (272)
#define OVERDRIVER_SRAM4_SIZE (256)
#define OD_DECRYPTION_IDX_1 (9)
#define OD_DECRYPTION_IDX_2 (10)
#define XC_AUTODOWNLOAD_OVERDRIVER_DEPTH  (289)

//Define OVERDRIVE
#define OVERDRIVE_WEIGHT                        (0x40)
#define OVERDRIVE_ACTIVE_THRESHOLD              (0x08)
#define OVERDRIVE_RGB_CHANNEL                   (3)
#define OVERDRIVE_RGBW_CHANNEL                  (4)
#define OVERDRIVE_MIU_BUS                       (256)
#define OVERDRIVE_PG_LENGTH                     (64)
#define OVERDRIVE_PG_ENTRY_PRE_REQ              (128)
#define OVERDRIVE_ENABLE_HSCALING               (TRUE)
#define OVERDRIVE_ENABLE_HSD_4X                 (FALSE)
#define OVERDRIVE_ENABLE_VSCALING               (TRUE)
#define OVERDRIVE_ENABLE_VAC_HSCALING           (TRUE)
#define OVERDRIVE_ENABLE_RGBW                   (FALSE) //If HW supprt OD RGBW
#define OVERDRIVE_RGB_444_MODE                  (12) //pix-bit
#define OVERDRIVE_RGB_565_MODE                  (16) //pix-bit
#define OVERDRIVE_RGB_666_MODE                  (18) //pix-bit
#define OVERDRIVE_COMPRESS_MODE                 (12) //pix-bit
#define OVERDRIVE_RGB_555_MODE                  (15) //pix-bit
#define OVERDRIVE_RGB_888_MODE                  (24) //pix-bit
#define OVERDRIVE_MODE                          (OVERDRIVE_COMPRESS_MODE)
#define OVERDRIVE_PIXEL                         (8)
#define OVERDRIVE_PANEL_EPI                     (5)
#define OVERDRIVE_PANEL_CEDS                    (8)
#define OVERDRIVE_EXTRA_MEM_SIZE_FACTOR         (14)//extra DRAM size to prevent page table over run
#define OVERDRIVE_ODW_6_BITMODE                 (6)
#define OVERDRIVE_ODW_8_BITMODE                 (8)
#define OVERDRIVE_ODW_10_BITMODE                (10)
#define OVERDRIVE_ODW_BITMODE                   (OVERDRIVE_ODW_6_BITMODE)
//Define OVERDRIVE CUSTOMIZED default value: it should reference register table
#define OVERDRIVE_WEIGHT_DEFAULT                (0x40)
#define OVERDRIVE_ACTIVE_THRESHOLD_DEFAULT      (0x04)
#define OVERDRIVE_STRENGTH_SLOP_DEFAULT        (0x2E)

#define DEC_3 (3)
#define DEC_4 (4)
#define DEC_7 (7)
#define DEC_10 (10)
#define DEC_64 (64)

typedef enum
{
    E_PNL_OVERDRIVER_SEPARATE_R_ENABLE = 0x08,
    E_PNL_OVERDRIVER_SEPARATE_G_ENABLE = 0x04,
    E_PNL_OVERDRIVER_SEPARATE_B_ENABLE = 0x02,
    E_PNL_OVERDRIVER_SEPARATE_W_ENABLE = 0x01,
} EN_PNL_OVERDRIVER_SEPARATE;

typedef struct __attribute__((packed))
{
    uint32_t u32SRAM_B1 : 8; //[7:0]
    uint32_t u32SRAM_B2 : 8; //[15:8]
    uint32_t u32SRAM_B3 : 8; //[23:16]
    uint32_t u32SRAM_B4 : 8; //[31:24]
    //uint32_t u32SRAM_B : 32;
    uint32_t u32SRAM_G1 : 8; //[7:0]
    uint32_t u32SRAM_G2 : 8; //[15:8]
    uint32_t u32SRAM_G3 : 8; //[23:16]
    uint32_t u32SRAM_G4 : 8; //[31:24]
    //uint32_t u32SRAM_G : 32;
    uint32_t u32SRAM_R1 : 8; //[7:0]
    uint32_t u32SRAM_R2 : 8; //[15:8]
    uint32_t u32SRAM_R3 : 8; //[23:16]
    uint32_t u32SRAM_R4 : 8; //[31:24]
    //uint32_t u32SRAM_R : 32;
    uint32_t u32OD_B_WriteEn : 1;
    uint32_t u32OD_G_WriteEn : 1;
    uint32_t u32OD_R_WriteEn : 1;
    uint32_t u32SecondOD_B_WriteEn : 1;
    uint32_t u32SecondOD_G_WriteEn : 1;
    uint32_t u32SecondOD_R_WriteEn : 1;
    uint32_t u32Dummy0 : 2;
    uint32_t u32Table_WriteAddr : 9;
    uint32_t u32Dummy1 : 15;
    uint32_t u32SRAM_W1 : 8; //[7:0]
    uint32_t u32SRAM_W2 : 8; //[15:8]
    uint32_t u32SRAM_W3 : 8; //[23:16]
    uint32_t u32SRAM_W4 : 8; //[31:24]
    //uint32_t u32SRAM_W : 32;
    uint32_t u32OD_W_WriteEn : 1;
    uint32_t u32SecondOD_W_WriteEn : 1;
    uint32_t u32Dummy2 : 30; //[191:162]
    uint32_t u32Dummy3 : 32; //[223:192]
    uint32_t u32Dummy4 : 32; //[255:224]
}ST_PNL_OD_NEW_MODE_DRAM_FORMAT;

struct ST_PNL_OD_NEW_MODE_DRAM_FORMAT_VERSION6 {
	uint32_t u32SRAM_B1 : 8; //[7:0]
	uint32_t u32SRAM_B2 : 8; //[15:8]
	uint32_t u32SRAM_B3 : 8; //[23:16]
	uint32_t u32SRAM_B4 : 8; //[31:24]
	//uint32_t u32SRAM_B : 32;
	uint32_t u32SRAM_G1 : 8; //[7:0]
	uint32_t u32SRAM_G2 : 8; //[15:8]
	uint32_t u32SRAM_G3 : 8; //[23:16]
	uint32_t u32SRAM_G4 : 8; //[31:24]
	//uint32_t u32SRAM_G : 32;
	uint32_t u32SRAM_R1 : 8; //[7:0]
	uint32_t u32SRAM_R2 : 8; //[15:8]
	uint32_t u32SRAM_R3 : 8; //[23:16]
	uint32_t u32SRAM_R4 : 8; //[31:24]
	//uint32_t u32SRAM_R : 32;
	uint32_t u32OD_B_WriteEn : 1;
	uint32_t u32OD_G_WriteEn : 1;
	uint32_t u32OD_R_WriteEn : 1;
	uint32_t u32SecondOD_B_WriteEn : 1;
	uint32_t u32SecondOD_G_WriteEn : 1;
	uint32_t u32SecondOD_R_WriteEn : 1;
	uint32_t u32Dummy0 : 2;
	uint32_t u32Table_WriteAddr : 9;
	uint32_t u32Dummy1 : 15;
};

typedef struct __attribute__((packed))
{
    uint16_t u16Version;            //[15:8]: major, [7:0]: minor version
    uint8_t u8LineOD_en;            //[0]: line od enable
    uint8_t u8SeparateFlag;         //[0]: channel separate flag [1]: table separate flag (unused)
    uint32_t u32YearMonthDayMinute; //[31:25]: reserved, [24:20]: year, [19:16]: month, [15:11]: day,
                                    //[10:0]: minute
    uint8_t u8TableMatrix;          //0: 19x19, 1: 17x17
    uint8_t u8RegionNumber;         //(Version>1 0: 1x1, 1: 2x8, 2:4X4) (Version<=1 16: 4x4 or 2x8, 1: 1x1)
    uint8_t u8GainMatrix;           //0: 4x4, 1: 13x9
    uint8_t u8GainRegion;           //0: 1x1
    uint8_t u8TableBit;                //(Version>2 0: 8bit 1:12bit) (Version<=2 no this member)
} ST_PNL_LINE_OD_SUB_HEADER, *pST_PNL_LINE_OD_SUB_HEADER;

ST_PNL_OD_NEW_MODE_DRAM_FORMAT *g_pstOD_ADL_DATA = NULL;
struct ST_PNL_OD_NEW_MODE_DRAM_FORMAT_VERSION6 *g_pstOD_ADL_DATA_V6;

void _overdriver_vac_hscaling_setting(struct udevice *dev)
{
#if OVERDRIVE_ENABLE_VAC_HSCALING
    uint16_t u16TconPath = 0;
    uint16_t u16VACPath = 0;
    bool bIsVacBeforeOD = FALSE;
    bool bIsVacEnable = FALSE;
    u16VACPath = R2BYTEMSK(REG_003C_PAFRC_BKA324_V004, Fld(2,8,AC_MSKB1));//bit[9:8]
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    bIsVacEnable = R2BYTEMSK(REG_003C_PAFRC_BKA324_V004, REG_003C_PAFRC_BKA324_V004_REG_VAC_CURVE_FIT_EN_003C);
    //Only VER4 and VER6 support vac hscaling
    if ((priv->pnl_lib_version == VERSION4) || (priv->pnl_lib_version == VERSION6))
    {
        if (bIsVacEnable)
        {
            if (u16VACPath == 0x1) // VAC move to WM output
            {
                bIsVacBeforeOD = TRUE;
            }
            else if (u16VACPath == 0x2) //VAC mode to HSE output
            {
                bIsVacBeforeOD = FALSE;
            }
            else //VAC in the middle of TCON
            {
                u16TconPath = R2BYTEMSK(REG_0180_SCTCON_BKA4F7_V004,
                    REG_0180_SCTCON_BKA4F7_V004_REG_DMC_OD_PGA_PATH_SEL_0180);
                if ((u16TconPath == TCON_PATH_DEMURA_PGA_OD) ||
                    (u16TconPath == TCON_PATH_PGA_DEMURA_OD) ||
                    (u16TconPath == TCON_PATH_PGA_OD_DEMURA))
                {
                    bIsVacBeforeOD = TRUE;
                }
                else
                {
                    bIsVacBeforeOD = FALSE;
                }
            }
        }
        else
        {
            bIsVacBeforeOD = FALSE;
        }
    }
    else
    {
        bIsVacBeforeOD = FALSE;
    }

    if(bIsVacBeforeOD) // enable vac hscaling
    {
        uint16_t u16Case = 0;
        uint16_t u16VacPol = R2BYTEMSK(REG_0020_PAFRC_BKA324_V004,
                                        REG_0020_PAFRC_BKA324_V004_REG_VAC_POLARITY_LUT0_0020);
        bool bRBSwap = FALSE;
        // set mode
        W2BYTEMSK(REG_01B0_OD_3RD_BKA338_V004, 1, REG_01B0_OD_3RD_BKA338_V004_REG_VAC_SCALING_EN_01B0);
        if ((u16VacPol == EVA_POLARITY_LUT0_MODE_3) ||
            ((u16VacPol == EVA_POLARITY_LUT0_MODE_2) && (bRBSwap == TRUE)))
        {
            u16Case = 3;
        }
        else if(u16VacPol == EVA_POLARITY_LUT0_MODE_2 && (bRBSwap == FALSE))
        {
            u16Case = 2;
        }
        else if(u16VacPol == EVA_POLARITY_LUT0_MODE_1)
        {
            u16Case = 1;
        }
        else
        {
            u16Case = 0;
        }
        W2BYTEMSK(REG_01B0_OD_3RD_BKA338_V004, u16Case, REG_01B0_OD_3RD_BKA338_V004_REG_VAC_CASE_01B0);
    }
    else //disable vac hscaling
    {
        W2BYTEMSK(REG_01B0_OD_3RD_BKA338_V004, 0, REG_01B0_OD_3RD_BKA338_V004_REG_VAC_SCALING_EN_01B0);
    }
#endif
}

bool _overdriver_is_enable_rgbw(void)
{
#if SUPPORT_OVERDRIVE
    return (R2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004,REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC));
#else
    return FALSE;
#endif
}

void _overdriver_enable_wchannel(bool bEn)
{
#if SUPPORT_OVERDRIVE
    //use dummy register to store tcon w channel enable status
    W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004, bEn, REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);
#endif
}

bool _enable_hvscaing_setting(u16 *u16HSize, u16 *u16VSize, bool bHscaling2X,
			      bool bHscaling4X, bool bVscaling)
{
    bool bRet =TRUE;
	u16 u16hsize = *u16HSize;
	u16 u16vsize = *u16VSize;

	if (bHscaling2X) {
		if (bHscaling4X) {
            W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,
                        0x1,
                        REG_01B0_OD_1ST_BKA336_V004_REG_HSD_2X4X_SEL_01B0); // HScaling_4X
			W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,
				  0x1,
				  REG_01B0_OD_1ST_BKA336_V004_REG_DE_ALIGN8_EN_01B0); // HScaling_4X
/*
            W2BYTEMSK(REG_01A0_OD_1ST_BKA336_V004, 0xE60D, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_01A4_OD_1ST_BKA336_V004, 0xD, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00D0_OD_2ND_BKA337_V004, 0x1804, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00D4_OD_2ND_BKA337_V004, 0x503C, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00D8_OD_2ND_BKA337_V004, 0x183C, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00DC_OD_2ND_BKA337_V004, 0x4, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00E0_OD_2ND_BKA337_V004, 0x8819, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_00E4_OD_2ND_BKA337_V004, 0xFA65, Fld(16,0,AC_FULLW10));
*/
            u16hsize = u16hsize/4;
        }
        else
        {
            W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,
                        0x0,
                        REG_01B0_OD_1ST_BKA336_V004_REG_HSD_2X4X_SEL_01B0); // HScaling_2X
            W2BYTEMSK(REG_0198_OD_1ST_BKA336_V004, 0x843E, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_019C_OD_1ST_BKA336_V004, 0x3E, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_01A0_OD_1ST_BKA336_V004, 0xE60D, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_01A4_OD_1ST_BKA336_V004, 0xD, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_01A8_OD_1ST_BKA336_V004, 0x95EB, Fld(16,0,AC_FULLW10));
            W2BYTEMSK(REG_01AC_OD_1ST_BKA336_V004, 0xEB95, Fld(16,0,AC_FULLW10));
            u16hsize = u16hsize/2;
        }
    }

	if (bVscaling) {
        W2BYTEMSK(REG_003C_OD_2ND_BKA337_V004, u16vsize, Fld(16,0,AC_FULLW10));//V_size
        W2BYTEMSK(REG_0160_OD_1ST_BKA336_V004, u16vsize, Fld(16,0,AC_FULLW10));//vertical size of a frame
        //vertical scaling 2x
        W2BYTEMSK(REG_00E8_OD_2ND_BKA337_V004, 0x8040, Fld(16,0,AC_FULLW10));
        W2BYTEMSK(REG_00EC_OD_2ND_BKA337_V004, 0x40, Fld(16,0,AC_FULLW10));
        u16vsize = u16vsize/2;
    }

    *u16HSize = u16hsize;
    *u16VSize = u16vsize;

    return bRet;
}

bool _set_od_weight_default(void)
{
    if (R2BYTEMSK(REG_0044_OD_1ST_BKA336_V004,
                    REG_0044_OD_1ST_BKA336_V004_REG_OD_USER_WEIGHT_0044) == OVERDRIVE_WEIGHT_DEFAULT)
    {
        // If equal to hardware default value, it means TCON bin do not write this value,
        // software should write this value instead.
        W2BYTEMSK(REG_0044_OD_1ST_BKA336_V004,
                    OVERDRIVE_WEIGHT,
                    REG_0044_OD_1ST_BKA336_V004_REG_OD_USER_WEIGHT_0044); //OD Weight
    }

    return TRUE;
}

bool _set_od_active_theshold_default(void)
{
    if (R2BYTEMSK(REG_0048_OD_1ST_BKA336_V004,
                    REG_0048_OD_1ST_BKA336_V004_REG_OD_ACT_THRD_0048) == OVERDRIVE_ACTIVE_THRESHOLD_DEFAULT)
    {
        // If equal to hardware default value, it means TCON bin do not write this value,
        // software should write this value instead.
        W2BYTEMSK(REG_0048_OD_1ST_BKA336_V004,
                    OVERDRIVE_ACTIVE_THRESHOLD,
                    REG_0048_OD_1ST_BKA336_V004_REG_OD_ACT_THRD_0048); //OD Active threshold
    }

    return TRUE;
}


// Turn OD function
u8 mtk_tcon_OverDriverGetPixelBitNum(u8 u8OdModeType)
{
	u8 u8PixBit = 0;

	if (u8OdModeType <= E_HALPNL_OD_MODE_NONE || u8OdModeType >= E_HALPNL_OD_MODE_MAX) {
		u8PixBit = OVERDRIVE_MODE;
		TCON_DEBUG("[%s,%5d].default u8PixBit=%d\033[0m\n", __func__, __LINE__, u8PixBit);
		return u8PixBit;
	}

	switch (u8OdModeType) {
	case E_HALPNL_OD_MODE_RGB_444:
	case E_HALPNL_OD_MODE_RGB_444_HSCALING:
	case E_HALPNL_OD_MODE_RGB_444_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X:
		u8PixBit = OVERDRIVE_RGB_444_MODE;
		break;
	case E_HALPNL_OD_MODE_RGB_565:
	case E_HALPNL_OD_MODE_RGB_565_HSCALING:
	case E_HALPNL_OD_MODE_RGB_565_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X:
		u8PixBit = OVERDRIVE_RGB_565_MODE;
		break;
	case E_HALPNL_OD_MODE_RGB_666:
	case E_HALPNL_OD_MODE_RGB_666_HSCALING:
	case E_HALPNL_OD_MODE_RGB_666_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X:
		u8PixBit = OVERDRIVE_RGB_666_MODE;
		break;
	case E_HALPNL_OD_MODE_RGB_555:
	case E_HALPNL_OD_MODE_RGB_555_HSCALING:
	case E_HALPNL_OD_MODE_RGB_555_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X:
		u8PixBit = OVERDRIVE_RGB_555_MODE;
		break;
	case E_HALPNL_OD_MODE_RGB_888:
	case E_HALPNL_OD_MODE_RGB_888_HSCALING:
	case E_HALPNL_OD_MODE_RGB_888_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X:
		u8PixBit = OVERDRIVE_RGB_888_MODE;
		break;
	case E_HALPNL_OD_MODE_COMPRESS_VLC:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HVSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_CE:
	case E_HALPNL_OD_MODE_COMPRESS_CE_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD_HCALING:
		u8PixBit = OVERDRIVE_COMPRESS_MODE;
		break;
	default:
		u8PixBit = OVERDRIVE_MODE;
	break;
	}
	TCON_DEBUG("[%s,%5d].u8PixBit=%d\033[0m\n", __func__, __LINE__, u8PixBit);
	return u8PixBit;
}

bool mtk_tcon_OverDriverGetHscaling2X(u8 u8OdModeType)
{
	u8 bHscaling = FALSE;

	if (u8OdModeType <= E_HALPNL_OD_MODE_NONE || u8OdModeType >= E_HALPNL_OD_MODE_MAX) {
		bHscaling = OVERDRIVE_ENABLE_HSCALING;
		TCON_DEBUG("[%s,%5d].default bHscaling=%d\033[0m\n", __func__, __LINE__, bHscaling);
		return bHscaling;
	}

	switch (u8OdModeType) {
	case E_HALPNL_OD_MODE_RGB_444:
	case E_HALPNL_OD_MODE_RGB_565:
	case E_HALPNL_OD_MODE_RGB_666:
	case E_HALPNL_OD_MODE_RGB_555:
	case E_HALPNL_OD_MODE_RGB_888:
	case E_HALPNL_OD_MODE_COMPRESS_VLC:
	case E_HALPNL_OD_MODE_COMPRESS_CE:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD:
		bHscaling = FALSE;
		break;
	case E_HALPNL_OD_MODE_RGB_444_HSCALING:
	case E_HALPNL_OD_MODE_RGB_565_HSCALING:
	case E_HALPNL_OD_MODE_RGB_666_HSCALING:
	case E_HALPNL_OD_MODE_RGB_555_HSCALING:
	case E_HALPNL_OD_MODE_RGB_888_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HVSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_CE_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD_HCALING:
	case E_HALPNL_OD_MODE_RGB_444_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_565_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_666_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_555_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_888_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X:
		bHscaling = TRUE;
		break;
	default:
		bHscaling = OVERDRIVE_ENABLE_HSCALING;
		break;
	}
	TCON_DEBUG("[%s,%5d].bHscaling=%d\033[0m\n", __func__, __LINE__, bHscaling);
	return bHscaling;
}

u8 mtk_tcon_OverDriverGetHscaling4X(u8 u8OdModeType)
{
	u8 bHscaling4X = FALSE;

	if (u8OdModeType <= E_HALPNL_OD_MODE_NONE || u8OdModeType >= E_HALPNL_OD_MODE_MAX) {
		bHscaling4X = OVERDRIVE_ENABLE_HSD_4X;
		TCON_DEBUG("[%s,%5d].default bHscaling4X=%d\033[0m\n", __func__, __LINE__, bHscaling4X);
		return bHscaling4X;
	}

	switch (u8OdModeType) {
	case E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X:
		bHscaling4X = TRUE;
		break;
	default:
		bHscaling4X = FALSE;
		break;
	}
	TCON_DEBUG("[%s,%5d].bHscaling4X=%d\033[0m\n", __func__, __LINE__, bHscaling4X);
	return bHscaling4X;
}

bool mtk_tcon_OverDriverGetVscaling(u8 u8OdModeType)
{
	bool bVscaling = FALSE;

	if (u8OdModeType <= E_HALPNL_OD_MODE_NONE || u8OdModeType >= E_HALPNL_OD_MODE_MAX) {
		bVscaling = OVERDRIVE_ENABLE_VSCALING;
		TCON_DEBUG("[%s,%5d].default bVscaling=%d\033[0m\n", __func__, __LINE__, bVscaling);
		return bVscaling;
	}

	switch (u8OdModeType) {
	case E_HALPNL_OD_MODE_RGB_444:
	case E_HALPNL_OD_MODE_RGB_444_HSCALING:
	case E_HALPNL_OD_MODE_RGB_565:
	case E_HALPNL_OD_MODE_RGB_565_HSCALING:
	case E_HALPNL_OD_MODE_RGB_666:
	case E_HALPNL_OD_MODE_RGB_666_HSCALING:
	case E_HALPNL_OD_MODE_RGB_555:
	case E_HALPNL_OD_MODE_RGB_555_HSCALING:
	case E_HALPNL_OD_MODE_RGB_888:
	case E_HALPNL_OD_MODE_RGB_888_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_VLC:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_CE:
	case E_HALPNL_OD_MODE_COMPRESS_CE_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD_HCALING:
		bVscaling = FALSE;
		break;
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_444_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_565_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_666_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_555_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_888_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X:
	case E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X:
		bVscaling = TRUE;
		break;
	default:
		bVscaling = OVERDRIVE_ENABLE_VSCALING;
		break;
	}

	TCON_DEBUG("[%s,%5d].bVscaling=%d\033[0m\n", __func__, __LINE__, bVscaling);
	return bVscaling;
}

#define HSIZE_4K 3840
#define HSIZE_2K 1920
#define VSIZE_2K 2160
#define VSIZE_1K 1080

void _get_od_page_length(u16 u16hsize,
	u16 u16vsize, u16 *od_page_length)
{
	if (u16hsize < HSIZE_2K && u16vsize < VSIZE_1K) {
		// < 2K1K
		*od_page_length = 0x1;
	} else if (u16hsize == HSIZE_2K && u16vsize == VSIZE_1K) {
		// 2k1k
		*od_page_length = DEC_3;
	} else if (u16hsize <= HSIZE_4K && u16vsize <= VSIZE_1K) {
		// 4k1k
		*od_page_length = DEC_7;
	} else if (u16hsize <= HSIZE_4K && u16vsize <= VSIZE_2K) {
		// 4k2k
		*od_page_length = 0xf;
	} else {
		// > 4k2k
		*od_page_length = 0xf;
	}
	TCON_DEBUG("%s = %d, Hde= %d, Vde = %d\n",
		__func__, *od_page_length, u16hsize, u16vsize);
}

void _overdriver_init(
                struct udevice *dev, u64 u64OD_MSB_Addr, u64 u64OD_LSB_Addr)
{
#if SUPPORT_OVERDRIVE
    u64 phypg_adde_base = 0;
    u64 phypg_addr_limit = 0;
    u64 phydram_addr_offset = 0;
    u64 wadr_max_limit = 0,radr_max_limit = 0;
    //uint32_t u32panel_pixels = 0;
	u16 u16hsize = 0;
	u16 u16vsize = 0;
#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
	u16 u16Timeout = 0;
#endif
    u64 u64OD_MSB_limit = 0;
	u8 u8PixBitNum = 0;
	bool bVscaling = 0;
	bool bHscaling2X = 0;
	bool bHscaling4X = 0;
	u8 u8OdModeType = 0;
	u16 od_page_length = 1;

    struct mtk_panel_priv *priv = dev_get_priv(dev);
    if (priv == NULL) {
        TCON_DEBUG("get device private fail\n");
        return;
    }

	u8OdModeType = (uint8_t)priv->tcon_info.stPanelInfo.u32OdModeType;
	u8PixBitNum = mtk_tcon_OverDriverGetPixelBitNum(u8OdModeType);
	bVscaling = mtk_tcon_OverDriverGetVscaling(u8OdModeType);
	bHscaling2X = mtk_tcon_OverDriverGetHscaling2X(u8OdModeType);
	bHscaling4X = mtk_tcon_OverDriverGetHscaling4X(u8OdModeType);
    // RD suggest to set pg address limit enable at begining
    W2BYTEMSK(
        REG_009C_OD_1ST_BKA336_V004,
        1,
        REG_009C_OD_1ST_BKA336_V004_REG_PG_LIMIT_EN_009C); //OD PG Address Limit Enable

#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if (_overdriver_is_enable_rgbw())
    {
        //If LPLL is powerdown, Double buffer can't update register value.
        if (R2BYTEMSK(L_BK_LPLL(0x03),BIT(5))!= 0)
        {
            MHal_PNL_PowerDownLPLL(FALSE);
        }
        //Disable Overdriver RGBW to prevent miu hit.
        SC_W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004,0,REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);

        while(SC_R2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004,
                REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC) &&
                (u16Timeout < MAX_DELAY_TIME))
        {
            mdelay(10);
            u16Timeout++;
        }
    }
#endif
    u16hsize = ALIGN_8(priv->de_width);//panel H, need align 8
    u16vsize = priv->de_height;//panel V
	_get_od_page_length(u16hsize, u16vsize, &od_page_length);
#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if (_overdriver_is_enable_rgbw())
    {
        u16hsize = (u16hsize * OVERDRIVE_RGB_CHANNEL) / OVERDRIVE_RGBW_CHANNEL;// RGBW mode h*3/4
    }
#endif

    //u32panel_pixels =u16hsize*u16vsize;

    //SC_W2BYTEMSK(REG_SC_BK16_61_L,u8MIUSel<<8,BIT(8)|BIT(9)); // OD MIU select
    W2BYTEMSK(REG_0054_OD_1ST_BKA336_V004,
                (uint16_t)(u64OD_MSB_Addr & 0xFFFF),
                REG_0054_OD_1ST_BKA336_V004_REG_OD_BASE_ADR_0_0054); // OD MSB request base address

    W2BYTEMSK(REG_0058_OD_1ST_BKA336_V004,
                (uint16_t)((u64OD_MSB_Addr >> 16) & 0xFFFF),
                REG_0058_OD_1ST_BKA336_V004_REG_OD_BASE_ADR_1_0058); // OD MSB request base address

    /*
    W2BYTEMSK(REG_00E4_OD_1ST_BKA336_V004,
                ((OVERDRIVE_MODE*u32panel_pixels+4096)/32)&0xFFFF,
                REG_00E4_OD_1ST_BKA336_V004_REG_OD_WADR_MAX_LIMIT_0_00E4); //low byte

    W2BYTEMSK(REG_00E8_OD_1ST_BKA336_V004,
                (((OVERDRIVE_MODE*u32panel_pixels+4096)/32)>>16),
                REG_00E8_OD_1ST_BKA336_V004_REG_OD_WADR_MAX_LIMIT_1_00E8); //high byte

    W2BYTEMSK(REG_00EC_OD_1ST_BKA336_V004,
                (u32panel_pixels)&0xFFFF,
                REG_00EC_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_0_00EC); //low byte

    W2BYTEMSK(REG_00F0_OD_1ST_BKA336_V004,
                (u32panel_pixels)>>16,
                REG_00F0_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_1_00F0); //high byte

    W2BYTEMSK(REG_00EC_OD_1ST_BKA336_V004,
                (u32panel_pixels)&0xFFFF,
                REG_00EC_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_0_00EC); //low byte

    W2BYTEMSK(REG_00F0_OD_1ST_BKA336_V004,
                (u32panel_pixels)>>16,
                REG_00F0_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_1_00F0); //high byte
    */
	_enable_hvscaing_setting(&u16hsize, &u16vsize, bHscaling2X, bHscaling4X, bVscaling);

    _set_od_weight_default();
    _set_od_active_theshold_default();

    // OD horizontal size of a frame
    W2BYTEMSK(REG_00BC_OD_1ST_BKA336_V004, u16hsize, REG_00BC_OD_1ST_BKA336_V004_REG_VDE_H_SIZE_00BC);

    if ((OVERDRIVE_ENABLE_RGBW == TRUE) && _overdriver_is_enable_rgbw())
    {
        W2BYTEMSK(REG_01BC_OD_1ST_BKA336_V004,
                    ((priv->de_width*OVERDRIVE_RGB_CHANNEL)/OVERDRIVE_RGBW_CHANNEL),
                    REG_01BC_OD_1ST_BKA336_V004_REG_H_SIZE_01BC); //set reg_htotal*3/4
    }
    else
    {
        W2BYTEMSK(REG_01BC_OD_1ST_BKA336_V004,
                    priv->de_width,
                    REG_01BC_OD_1ST_BKA336_V004_REG_H_SIZE_01BC); //set reg_htotal
    }

	phypg_addr_limit = ((u64)(u16hsize * u16vsize) * (u64)u8PixBitNum *
		(u64)OVERDRIVE_EXTRA_MEM_SIZE_FACTOR) /
		(u64)(DEC_10 * OVERDRIVE_MIU_BUS *
		OVERDRIVE_PG_ENTRY_PRE_REQ * (od_page_length + 1) * DEC_4);//254,128,64

	phydram_addr_offset = (phypg_addr_limit + 1) * OVERDRIVE_PG_ENTRY_PRE_REQ *
		(od_page_length + 1) * DEC_4;

    TCON_DEBUG("pg_addr_limit=%td dram_addr_offset=%td \n",
                    (ptrdiff_t)phypg_addr_limit, (ptrdiff_t)phydram_addr_offset);

    u64OD_MSB_limit = u64OD_MSB_Addr + phydram_addr_offset + phypg_addr_limit;

    W2BYTEMSK(REG_005C_OD_1ST_BKA336_V004,
                (uint16_t)(u64OD_MSB_limit & 0xFFFF),
                Fld(16,0,AC_FULLW10)); // OD MSB request address limit
    W2BYTEMSK(REG_0060_OD_1ST_BKA336_V004,
                (uint16_t)((u64OD_MSB_limit >> 16) & 0xFFFF),
                REG_0060_OD_1ST_BKA336_V004_REG_OD_ADR_LIMIT_1_0060); // OD MSB request address limit

    phypg_adde_base = u64OD_MSB_Addr + phydram_addr_offset;

    W2BYTEMSK(REG_011C_OD_1ST_BKA336_V004,
                (uint16_t)(phypg_adde_base & 0xFFFF),
                Fld(16,0,AC_FULLW10)); // OD PG Base request base address

    W2BYTEMSK(REG_0120_OD_1ST_BKA336_V004,
                (uint16_t)((phypg_adde_base >> 16) & 0xFFFF),
                REG_0120_OD_1ST_BKA336_V004_REG_PG_BASE_ADR_1_0120); //OD PG Base request base address

    W2BYTEMSK(REG_009C_OD_1ST_BKA336_V004,
                phypg_addr_limit,
                REG_009C_OD_1ST_BKA336_V004_REG_PG_ADR_LIMIT_009C); //OD PG Address Limit

	W2BYTEMSK(REG_00A4_OD_1ST_BKA336_V004,
		od_page_length,
		REG_00A4_OD_1ST_BKA336_V004_REG_PG_LENGTH_00A4); //OD PG length

    // read/write address = mit setting
	wadr_max_limit = div_u64((u64)u8PixBitNum * (u64)(u16hsize * u16vsize) +
                                (phypg_addr_limit+1)*OVERDRIVE_MIU_BUS, BYTE_PER_WORD);

    W2BYTEMSK(REG_00E4_OD_1ST_BKA336_V004,
                (uint16_t)(wadr_max_limit & 0xFFFF),
                REG_00E4_OD_1ST_BKA336_V004_REG_OD_WADR_MAX_LIMIT_0_00E4);

    W2BYTEMSK(REG_00E8_OD_1ST_BKA336_V004,
                (uint16_t)((wadr_max_limit >> 16) & 0xFFFF),
                REG_00E8_OD_1ST_BKA336_V004_REG_OD_WADR_MAX_LIMIT_1_00E8);

    radr_max_limit = (u64)(u16hsize*u16vsize);

    W2BYTEMSK(REG_00EC_OD_1ST_BKA336_V004,
                (uint16_t)(radr_max_limit & 0xFFFF),
                REG_00EC_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_0_00EC);

    W2BYTEMSK(REG_00F0_OD_1ST_BKA336_V004,
                (uint16_t)((radr_max_limit >> 16) & 0xFFFF),
                REG_00F0_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_1_00F0);

    TCON_DEBUG("wadr_max_limit=%llx, radr_max_limit=%llx\n", wadr_max_limit, radr_max_limit);
    //SC_W2BYTE(REG_SC_BK16_6F_L,0x0F00);//for merlin definition different
    // Need Check
    //SC_W2BYTE(REG_SC_BK16_29_L,0x0F);

    //OD one index containts
    //SC_W2BYTEMSK(REG_SC_BK16_29_L,BIT(8)|BIT(9)|BIT(10)|BIT(11),BIT(8)|BIT(9)|BIT(10)|BIT(11));

    W2BYTEMSK(REG_0064_OD_1ST_BKA336_V004, 0x10, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0068_OD_1ST_BKA336_V004, 0x3F, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0068_OD_1ST_BKA336_V004, 0x00, Fld(8,8,AC_FULLB1));
    W2BYTEMSK(REG_006C_OD_1ST_BKA336_V004, 0x0A, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0070_OD_1ST_BKA336_V004, 0x3F, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0070_OD_1ST_BKA336_V004, 0x00, Fld(8,8,AC_FULLB1));
    W2BYTEMSK(REG_00A0_OD_1ST_BKA336_V004, 0x3F, REG_00A0_OD_1ST_BKA336_V004_REG_ARB_WACK_LENGTH_00A0);

    W2BYTEMSK(REG_0064_OD_1ST_BKA336_V004, 1, REG_0064_OD_1ST_BKA336_V004_REG_OD_RREQ_BREAK_MODE_0064);

#ifdef STELLAR
    if((SC_R2BYTEMSK(REG_SC_BKF2_73_L, 0xF0) >> 4) == 5) //M+
    {
        W2BYTEMSK(REG_0154_OD_1ST_BKA336_V004, 0x2, REG_0154_OD_1ST_BKA336_V004_REG_VFEND_DELAY_0154);
    }
    else if((SC_R2BYTEMSK(REG_SC_BKF2_73_L, 0xF0) >> 4) == 8) //SiW RGBW
    {
        W2BYTEMSK(REG_0154_OD_1ST_BKA336_V004, 0x1, REG_0154_OD_1ST_BKA336_V004_REG_VFEND_DELAY_0154);
    }
    else
#endif
    {
	/* Solve the problem of bright lines at the bottom of the screen. Align mixed mode */
	W2BYTEMSK(REG_0154_OD_1ST_BKA336_V004,
		  0x03,
		  REG_0154_OD_1ST_BKA336_V004_REG_VSYNC_START_DELAY_0154);

	W2BYTEMSK(REG_0154_OD_1ST_BKA336_V004,
		  0x01,
		  REG_0154_OD_1ST_BKA336_V004_REG_VSYNC_WIDTH_DELAY_0154);

	W2BYTEMSK(REG_0154_OD_1ST_BKA336_V004,
		  0x01,
		  REG_0154_OD_1ST_BKA336_V004_REG_VFEND_DELAY_0154);
    }

    W2BYTEMSK(REG_00C8_OD_1ST_BKA336_V004, 0, REG_00C8_OD_1ST_BKA336_V004_REG_LN_RATE_CTRL_EN_ODW_00C8);
    W2BYTEMSK(REG_00C8_OD_1ST_BKA336_V004, 0, REG_00C8_OD_1ST_BKA336_V004_REG_LN_RATE_CTRL_EN_ODR_00C8);

    if ((R2BYTEMSK(REG_00E8_OD_1ST_BKA336_V004, Fld(6,8,AC_MSKB1))) == OVERDRIVE_STRENGTH_SLOP_DEFAULT)
    {
        // If equal to hardware default value, it means TCON bin do not write this value,
        // software should write this value instead.
        W2BYTEMSK(
                REG_00E8_OD_1ST_BKA336_V004,
                0x2E,
                REG_00E8_OD_1ST_BKA336_V004_REG_OD_STRENGTH_SLOP_00E8);
        W2BYTEMSK(
                REG_00E8_OD_1ST_BKA336_V004,
                0,
                REG_00E8_OD_1ST_BKA336_V004_REG_OD_STRENGTH_SLOP_BYPASS_00E8);
    }
    W2BYTEMSK(REG_00D8_OD_1ST_BKA336_V004, 0x0007, Fld(16,0,AC_FULLW10));
    //W2BYTEMSK(REG_00E4_OD_1ST_BKA336_V004, 0xFFFF, Fld(16,0,AC_FULLW10));
    //W2BYTEMSK(REG_00F0_OD_1ST_BKA336_V004, 0xFF,REG_00F0_OD_1ST_BKA336_V004_REG_OD_RADR_MAX_LIMIT_1_00F0);
    //W2BYTEMSK(REG_00EC_OD_1ST_BKA336_V004, 0xFFFF, Fld(16,0,AC_FULLW10));

    //OD setting
	//enable hw config of dram priority
	W2BYTEMSK(REG_01E0_OD_3RD_BKA338_V004,
		    0,
		    REG_01E0_OD_3RD_BKA338_V004_REG_W_PREULTRA_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    0,
		    REG_01E0_OD_3RD_BKA338_V004_REG_W_ULTRA_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    1,
		    REG_01E0_OD_3RD_BKA338_V004_REG_W_URGENT_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    1,
		    REG_01E0_OD_3RD_BKA338_V004_REG_W_URGENT_BLANKING_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    0,
		    REG_01E0_OD_3RD_BKA338_V004_REG_R_PREULTRA_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    0,
		    REG_01E0_OD_3RD_BKA338_V004_REG_R_ULTRA_MASK_01E0);
	W2BYTEMSK(
		    REG_01E0_OD_3RD_BKA338_V004,
		    1,
		    REG_01E0_OD_3RD_BKA338_V004_REG_R_URGENT_MASK_01E0);
	W2BYTEMSK(REG_01E0_OD_3RD_BKA338_V004,
		    1,
		    REG_01E0_OD_3RD_BKA338_V004_REG_R_URGENT_BLANKING_MASK_01E0);
    //od request vsync reset wait all request end enable,RD suggest for prevent miu lock problem
    W2BYTEMSK(REG_0080_OD_1ST_BKA336_V004, 1, REG_0080_OD_1ST_BKA336_V004_REG_REQ_STATE_JUMP_SEL_0080);

    //random_active_threshold
    W2BYTEMSK(REG_0020_OD_2ND_BKA337_V004, 0x0302, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0024_OD_2ND_BKA337_V004, 0x0905, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0028_OD_2ND_BKA337_V004, 0x0302, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_002C_OD_2ND_BKA337_V004, 0x0905, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0030_OD_2ND_BKA337_V004, 0x0302, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0034_OD_2ND_BKA337_V004, 0x0905, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0038_OD_2ND_BKA337_V004, 1, REG_0038_OD_2ND_BKA337_V004_REG_RANDOM_ACT_THRD_EN_0038);

    //compression setting
    W2BYTEMSK(REG_0084_OD_1ST_BKA336_V004, 0x1218, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0088_OD_1ST_BKA336_V004, 0x060C, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_008C_OD_1ST_BKA336_V004, 0x1417, Fld(16,0,AC_FULLW10));
    W2BYTEMSK(REG_0090_OD_1ST_BKA336_V004, 0x8E11, Fld(16,0,AC_FULLW10));

    //compression decode round enable
    W2BYTEMSK(REG_01B8_OD_1ST_BKA336_V004, 1, REG_01B8_OD_1ST_BKA336_V004_REG_DEC_ROUND_EN_01B8);

#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if(_overdriver_is_enable_rgbw())
    {
        W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004, 0xE4, 0x0FFF);//ch_loc_w,ch_loc_b,ch_loc_g,ch_loc_r
    }
#endif
#endif
}

void _overdriver_set_od_mode(u8 u8OdModeType)
{
	// set OD mode
	switch (u8OdModeType) {
	case E_HALPNL_OD_MODE_RGB_444:
	case E_HALPNL_OD_MODE_RGB_444_HSCALING:
	case E_HALPNL_OD_MODE_RGB_444_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_444_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	case E_HALPNL_OD_MODE_RGB_565:
	case E_HALPNL_OD_MODE_RGB_565_HSCALING:
	case E_HALPNL_OD_MODE_RGB_565_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_444_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	case E_HALPNL_OD_MODE_RGB_666:
	case E_HALPNL_OD_MODE_RGB_666_HSCALING:
	case E_HALPNL_OD_MODE_RGB_666_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_444_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	case E_HALPNL_OD_MODE_RGB_555:
	case E_HALPNL_OD_MODE_RGB_555_HSCALING:
	case E_HALPNL_OD_MODE_RGB_555_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_555_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	case E_HALPNL_OD_MODE_RGB_888:
	case E_HALPNL_OD_MODE_RGB_888_HSCALING:
	case E_HALPNL_OD_MODE_RGB_888_HVSCALING:
	case E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_888_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	case E_HALPNL_OD_MODE_COMPRESS_VLC:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_VLC_HVSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_CE:
	case E_HALPNL_OD_MODE_COMPRESS_CE_HSCALING:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD:
	case E_HALPNL_OD_MODE_COMPRESS_ONE3RD_HCALING:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_COMPRESS_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	default:
		W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, OVERDRIVE_COMPRESS_EN_VALUE, Fld(DEC_4, 0, AC_MSKB0));
		break;
	}
}

void _set_od_enable(struct udevice *dev, bool bEnable)
{
#if SUPPORT_OVERDRIVE
	u16 u16Taget_BR = 0; //Target bit rate
	u8 u8PixBitNum = 0;
	bool bVscaling = 0;
	bool bHscaling2X = 0;
	u8 u8OdModeType = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv) {
		TCON_ERROR("[%s] get device private fail\n", __func__);
		return;
	}

	u8OdModeType = (u8)priv->tcon_info.stPanelInfo.u32OdModeType;

	u8PixBitNum = mtk_tcon_OverDriverGetPixelBitNum(u8OdModeType);
	bVscaling = mtk_tcon_OverDriverGetVscaling(u8OdModeType);
	bHscaling2X = mtk_tcon_OverDriverGetHscaling2X(u8OdModeType);
    // OD mode
    // OD used user weight to output blending directly
    // OD Enable

		u16Taget_BR = (u8PixBitNum * DEC_64) + DEC_3;//Formula = 64*bits_per_pixel + 3
        //Target bit rate of compression engine output for 4 bit compression
        W2BYTEMSK(REG_00B4_OD_1ST_BKA336_V004, u16Taget_BR, REG_00B4_OD_1ST_BKA336_V004_REG_TARGET_BR_00B4);

#if (OVERDRIVE_ENABLE_RGBW == TRUE)
        if(_overdriver_is_enable_rgbw())//not support w channel
        {
            //RGBW input Enable
            W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004, 0x1, REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);
        }
#endif
		if (bVscaling || bHscaling2X)
        {
            //both need VScaling & HScaling
			if (bVscaling && bHscaling2X) {
                //Enable VScaling HScaling together
                W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,0x3,Fld(2,0,AC_MSKB0));
			} else if (bVscaling) {//only VScaling
                //Enable VScaling
                W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,1,REG_01B0_OD_1ST_BKA336_V004_REG_ENABLE_VSCALING_01B0);
            }
            else //only HScaling
            {
                //Enable HScaling
                W2BYTEMSK(REG_01B0_OD_1ST_BKA336_V004,1,REG_01B0_OD_1ST_BKA336_V004_REG_ENABLE_HSCALING_01B0);
            }
        }
#if OVERDRIVE_ENABLE_VAC_HSCALING
        _overdriver_vac_hscaling_setting(dev);
#endif
		_overdriver_set_od_mode(u8OdModeType);
        // rd suggest enable od at last
    if (bEnable)
    {
        //OD enable
        W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, 0x1, REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);
    }
    else
    {
        //OD disable
        W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, 0x0, REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);
    }

#endif
}

bool _set_od_rgbw_enable(bool bEnable)
{
    bool bRet = TRUE;

#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if(_overdriver_is_enable_rgbw())
    {
        if(R2BYTEMSK(L_BK_LPLL(0x03),BIT(5))!= 0)
        {
            MHal_PNL_PowerDownLPLL(FALSE);
        }
        W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004,
                    (bEnable ? 1 : 0),
                    REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);
    }
#endif

    return bRet;
}

bool _prepare_od_tbl_to_sram(
                                uint16_t *pu16ODTbl,
                                uint8_t *pu8OD_SRAM1, uint8_t *pu8OD_SRAM2,
                                uint8_t *pu8OD_SRAM3, uint8_t *pu8OD_SRAM4)
{
    bool bRet = TRUE;
    uint16_t u16TableIndex = 0;
    uint16_t u16CurrentTableIndex = 0;
    uint16_t u16TargetIndex = 0;
    uint16_t u16Target = 0;

    TCON_CHECK_PARAMETER_NULL(pu16ODTbl);
    TCON_CHECK_PARAMETER_NULL(pu8OD_SRAM1);
    TCON_CHECK_PARAMETER_NULL(pu8OD_SRAM2);
    TCON_CHECK_PARAMETER_NULL(pu8OD_SRAM3);
    TCON_CHECK_PARAMETER_NULL(pu8OD_SRAM4);

    //sram1
    u16TargetIndex = OD_DECRYPTION_IDX_1;
    u16Target = pu16ODTbl[u16TargetIndex];

    for (u16TableIndex = 0; u16TableIndex < OVERDRIVER_SRAM1_SIZE; u16TableIndex++)
    {
        pu8OD_SRAM1[u16TableIndex] = (u16TableIndex == u16TargetIndex) ?
                    (uint8_t)u16Target :
                    (uint8_t)(pu16ODTbl[u16TableIndex]^u16Target);
    }

    //sram2
    u16CurrentTableIndex += OVERDRIVER_SRAM1_SIZE;
    u16TargetIndex += OD_DECRYPTION_IDX_2;
    u16Target = pu16ODTbl[(u16CurrentTableIndex+u16TargetIndex)];

    for (u16TableIndex = 0; u16TableIndex < OVERDRIVER_SRAM2_SIZE; u16TableIndex++)
    {
        pu8OD_SRAM2[u16TableIndex] = (u16TableIndex == u16TargetIndex) ?
                (uint8_t)u16Target :
                (uint8_t)(pu16ODTbl[(u16CurrentTableIndex+u16TableIndex)]^u16Target);
    }

    //sram3
    u16CurrentTableIndex += OVERDRIVER_SRAM2_SIZE;
    u16TargetIndex += OD_DECRYPTION_IDX_2;
    u16Target = pu16ODTbl[(u16CurrentTableIndex+u16TargetIndex)];

    for (u16TableIndex = 0; u16TableIndex < OVERDRIVER_SRAM3_SIZE; u16TableIndex++)
    {
        pu8OD_SRAM3[u16TableIndex] = (u16TableIndex == u16TargetIndex) ?
                (uint8_t)u16Target :
                (uint8_t)(pu16ODTbl[(u16CurrentTableIndex+u16TableIndex)]^u16Target);
    }

    //sram4
    u16CurrentTableIndex += OVERDRIVER_SRAM3_SIZE;
    u16TargetIndex += OD_DECRYPTION_IDX_2;
    u16Target = pu16ODTbl[(u16CurrentTableIndex+u16TargetIndex)];

    for (u16TableIndex = 0; u16TableIndex < OVERDRIVER_SRAM4_SIZE; u16TableIndex++)
    {
        pu8OD_SRAM4[u16TableIndex] = (u16TableIndex == u16TargetIndex) ?
                (uint8_t)u16Target :
                (uint8_t)(pu16ODTbl[(u16CurrentTableIndex+u16TableIndex)]^u16Target);
    }

    return bRet;
}

bool _overdriver_setting_without_adl(
                                    uint8_t *pu8OD_SRAM1, uint8_t *pu8OD_SRAM2,
                                    uint8_t *pu8OD_SRAM3, uint8_t *pu8OD_SRAM4, uint16_t u16SepatateMode)
{
    bool bRet = TRUE;
    uint16_t u16CodeTableIndex = 0;
    uint16_t u16Timeout = 0;

    // OD table SRAM enable, RGBW channel
    TCON_W2BYTEMSK(REG_0004_OD_1ST_BKA336_V004>>1, u16SepatateMode, 0x0F);

    for (u16CodeTableIndex = 0; u16CodeTableIndex < OVERDRIVER_SRAM1_SIZE; u16CodeTableIndex++)
    {
        TCON_W2BYTEMSK(REG_000C_OD_1ST_BKA336_V004>>1, pu8OD_SRAM1[u16CodeTableIndex], 0x00FF);
        TCON_W2BYTEMSK(REG_0008_OD_1ST_BKA336_V004>>1, (u16CodeTableIndex|0x8000), 0x81FF);
        while(R2BYTEMSK(REG_0008_OD_1ST_BKA336_V004,
                            REG_0008_OD_1ST_BKA336_V004_REG_OD_SRAM1_IOWE_READ_BACK_0008) &&
                            (u16Timeout < MAX_DELAY_TIME))
        {
            mdelay(1);
            u16Timeout++;
        }
        u16Timeout = 0;
    }

    for (u16CodeTableIndex = 0; u16CodeTableIndex < OVERDRIVER_SRAM2_SIZE; u16CodeTableIndex++)
    {
        TCON_W2BYTEMSK(REG_0018_OD_1ST_BKA336_V004>>1, pu8OD_SRAM2[u16CodeTableIndex], 0x00FF);
        TCON_W2BYTEMSK(REG_0014_OD_1ST_BKA336_V004>>1, (u16CodeTableIndex|0x8000), 0x81FF);
        while(R2BYTEMSK(REG_0014_OD_1ST_BKA336_V004,
                            REG_0014_OD_1ST_BKA336_V004_REG_OD_SRAM2_IOWE_READ_BACK_0014) &&
                            (u16Timeout < MAX_DELAY_TIME))
        {
            mdelay(1);
            u16Timeout++;
        }
        u16Timeout = 0;
    }

    for (u16CodeTableIndex = 0; u16CodeTableIndex<OVERDRIVER_SRAM3_SIZE; u16CodeTableIndex++)
    {
        TCON_W2BYTEMSK(REG_0024_OD_1ST_BKA336_V004>>1, pu8OD_SRAM3[u16CodeTableIndex], 0x00FF);
        TCON_W2BYTEMSK(REG_0020_OD_1ST_BKA336_V004>>1, (u16CodeTableIndex|0x8000), 0x81FF);
        while(R2BYTEMSK(REG_0020_OD_1ST_BKA336_V004,
                            REG_0020_OD_1ST_BKA336_V004_REG_OD_SRAM3_IOWE_READ_BACK_0020) &&
                            (u16Timeout < MAX_DELAY_TIME))
        {
            mdelay(1);
            u16Timeout++;
        }
        u16Timeout = 0;
    }

    for (u16CodeTableIndex = 0; u16CodeTableIndex < OVERDRIVER_SRAM4_SIZE; u16CodeTableIndex++)
    {
        TCON_W2BYTEMSK(REG_0030_OD_1ST_BKA336_V004>>1, pu8OD_SRAM4[u16CodeTableIndex], 0x00FF);
        TCON_W2BYTEMSK(REG_002C_OD_1ST_BKA336_V004>>1, (u16CodeTableIndex|0x8000), 0x81FF);
        while(R2BYTEMSK(REG_002C_OD_1ST_BKA336_V004,
                            REG_002C_OD_1ST_BKA336_V004_REG_OD_SRAM4_IOWE_READ_BACK_002C) &&
                            (u16Timeout < MAX_DELAY_TIME))
        {
            mdelay(1);
            u16Timeout++;
        }
        u16Timeout = 0;
    }

    TCON_W2BYTEMSK( REG_0004_OD_1ST_BKA336_V004>>1, 0x00, 0x0F); // OD table SRAM enable, RGBW channel

    return bRet;
}

bool _overdriver_adl_init(
                    struct udevice *dev, uint32_t u32ADLSize)
{
    bool bRet = TRUE;
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(priv);

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
    if ((priv->pnl_lib_version == VERSION5) || (priv->pnl_lib_version == VERSION6))
    {
        //adl index user mode en have to be set 1 when boot time
        W2BYTEMSK(
            REG_0084_SCTCON_MISC_BKA3E0_V005,
            0x1,
            REG_0084_SCTCON_MISC_BKA3E0_V005_REG_ADL_IDX_OD_SEL_0084);
    }

    //Always use new mode, support write independent R/G/B/W tables
    W2BYTEMSK(
            REG_0004_OD_1ST_BKA336_V004,
            1,
            REG_0004_OD_1ST_BKA336_V004_REG_AUTO_TABLE_LOAD_EN_0004);//[8]:reg_auto_table_load_en

    W2BYTEMSK(
            REG_0004_OD_1ST_BKA336_V004,
            1,
            REG_0004_OD_1ST_BKA336_V004_REG_OD_AUTODL_TRIG_EN_0004);//[10]:reg_od_autodl_trig_en

    W2BYTEMSK(
            REG_01D4_OD_1ST_BKA336_V004,
            0,
            REG_01D4_OD_1ST_BKA336_V004_REG_ADL_TRIG_SEL_01D4); //reg_adl_trig_sel

    W2BYTEMSK(
            REG_010C_OD_2ND_BKA337_V004,
            1,
            REG_010C_OD_2ND_BKA337_V004_REG_AUTO_TABLE_MODE_010C);

	if (priv->pnl_lib_version == VERSION6) {
		PNL_MALLOC_MEM(g_pstOD_ADL_DATA_V6, u32ADLSize, bRet);

		if (!bRet) {
			TCON_ERROR("OD Autodownload allocate mem fail\n");
			PNL_FREE_MEM(g_pstOD_ADL_DATA_V6);
			return FALSE;
		}
	} else {
		PNL_MALLOC_MEM(g_pstOD_ADL_DATA, u32ADLSize, bRet);

		if (!bRet) {
			TCON_ERROR("OD Autodownload allocate mem fail\n");
			PNL_FREE_MEM(g_pstOD_ADL_DATA);
			return FALSE;
		}
	}

    bRet = R2BYTEMSK(
                REG_0040_OD_1ST_BKA336_V004,
                REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);
 #endif

    W2BYTEMSK(
            REG_0040_OD_1ST_BKA336_V004,
            0x00,
            REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040); // OD enable

    return bRet;
}

bool _fill_to_adl_sram1_data(
				u8 *pu8Data,
				u16 u16TableIndex, u16 u16SepatateMode,
				u32 pnl_lib_version)
{
    bool bRet = TRUE;

	TCON_CHECK_PARAMETER_NULL(pu8Data);

	if (pnl_lib_version == VERSION6) {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA_V6);
		if (u16TableIndex < OVERDRIVER_SRAM1_SIZE) {
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32Table_WriteAddr = u16TableIndex;
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE) {
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32OD_B_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE) >> 1;
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B1 = pu8Data[u16TableIndex];
			}
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE) {
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32OD_G_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE) >> 2;
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G1 = pu8Data[u16TableIndex];
			}
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE) {
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32OD_R_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE) >> 3;
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R1 = pu8Data[u16TableIndex];
			}
		} else {
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B1 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G1 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R1 = 0;
		}
	} else {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA);
		if (u16TableIndex < OVERDRIVER_SRAM1_SIZE) {
			g_pstOD_ADL_DATA[u16TableIndex].u32Table_WriteAddr = u16TableIndex;
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE) {
				g_pstOD_ADL_DATA[u16TableIndex].u32OD_B_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE) >> 1;
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B1 = pu8Data[u16TableIndex];
			}
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE) {
				g_pstOD_ADL_DATA[u16TableIndex].u32OD_G_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE) >> 2;
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G1 = pu8Data[u16TableIndex];
			}
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE) {
				g_pstOD_ADL_DATA[u16TableIndex].u32OD_R_WriteEn =
					(u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE) >> 3;
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R1 = pu8Data[u16TableIndex];
			}
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_W_ENABLE) {
				g_pstOD_ADL_DATA[u16TableIndex].u32OD_W_WriteEn =
					u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_W_ENABLE;
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W1 = pu8Data[u16TableIndex];
			}
		} else {
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B1 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G1 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R1 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W1 = 0;
		}
	}

    return bRet;
}

bool _fill_to_adl_sram2_data(
				u8 *pu8Data,
				u16 u16TableIndex, u16 u16SepatateMode,
				u32 pnl_lib_version)
{
    bool bRet = TRUE;

	TCON_CHECK_PARAMETER_NULL(pu8Data);

	if (pnl_lib_version == VERSION6) {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA_V6);
		if (u16TableIndex < OVERDRIVER_SRAM2_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B2 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G2 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R2 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B2 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G2 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R2 = 0;
		}
	} else {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA);
		if (u16TableIndex < OVERDRIVER_SRAM2_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B2 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G2 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R2 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_W_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W2 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B2 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G2 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R2 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W2 = 0;
		}
	}

    return bRet;
}

bool _fill_to_adl_sram3_data(
				u8 *pu8Data,
				u16 u16TableIndex, u16 u16SepatateMode,
				u32 pnl_lib_version)
{
    bool bRet = TRUE;

	TCON_CHECK_PARAMETER_NULL(pu8Data);

	if (pnl_lib_version == VERSION6) {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA_V6);
		if (u16TableIndex < OVERDRIVER_SRAM3_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B3 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G3 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R3 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B3 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G3 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R3 = 0;
		}
	} else {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA);
		if (u16TableIndex < OVERDRIVER_SRAM3_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B3 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G3 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R3 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_W_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W3 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B3 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G3 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R3 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W3 = 0;
		}
	}

    return bRet;
}

bool _fill_to_adl_sram4_data(
				u8 *pu8Data,
				u16 u16TableIndex, u16 u16SepatateMode,
				u32 pnl_lib_version)
{
    bool bRet = TRUE;

	TCON_CHECK_PARAMETER_NULL(pu8Data);

	if (pnl_lib_version == VERSION6) {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA_V6);
		if (u16TableIndex < OVERDRIVER_SRAM4_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B4 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G4 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R4 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_B4 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_G4 = 0;
			g_pstOD_ADL_DATA_V6[u16TableIndex].u32SRAM_R4 = 0;
		}
	} else {
		TCON_CHECK_PARAMETER_NULL(g_pstOD_ADL_DATA);
		if (u16TableIndex < OVERDRIVER_SRAM4_SIZE) {
			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_B_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B4 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_G_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G4 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_R_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R4 = pu8Data[u16TableIndex];

			if (u16SepatateMode & E_PNL_OVERDRIVER_SEPARATE_W_ENABLE)
				g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W4 = pu8Data[u16TableIndex];
		} else {
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_B4 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_G4 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_R4 = 0;
			g_pstOD_ADL_DATA[u16TableIndex].u32SRAM_W4 = 0;
		}
	}

    return bRet;
}

bool _overdriver_adl_setting_tbl(
	struct udevice *dev, uint16_t *pu16ODTbl, uint16_t u16SepatateMode)
{
    uint16_t u16TableIndex = 0;
    uint8_t *pu8OD_SRAM1 = NULL;
    uint8_t *pu8OD_SRAM2 = NULL;
    uint8_t *pu8OD_SRAM3 = NULL;
    uint8_t *pu8OD_SRAM4 = NULL;
    bool bRet = TRUE;
	struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(pu16ODTbl);

    PNL_MALLOC_MEM(pu8OD_SRAM1, OVERDRIVER_SRAM1_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM2, OVERDRIVER_SRAM2_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM3, OVERDRIVER_SRAM3_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM4, OVERDRIVER_SRAM4_SIZE, bRet);

    if (!bRet)
    {
        TCON_ERROR("Error: malloc od sram failed\n");
        goto finally;
    }

    //Decrypt OD table.
    _prepare_od_tbl_to_sram(pu16ODTbl, pu8OD_SRAM1, pu8OD_SRAM2, pu8OD_SRAM3, pu8OD_SRAM4);

#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if(MHal_PNL_OverDriver_IsEnableRGBW())
    {
        if (R2BYTEMSK(L_BK_LPLL(0x03),BIT(5))!= 0)
        {
            MHal_PNL_PowerDownLPLL(FALSE);
        }
        //Disable Overdriver RGBW to prevent write od table fail
        W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004,
                    0x00,
                    REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);
    }
#endif

    for (u16TableIndex = 0; u16TableIndex < XC_AUTODOWNLOAD_OVERDRIVER_DEPTH; u16TableIndex++)
    {
		_fill_to_adl_sram1_data(pu8OD_SRAM1, u16TableIndex, u16SepatateMode, priv->pnl_lib_version);
		_fill_to_adl_sram2_data(pu8OD_SRAM2, u16TableIndex, u16SepatateMode, priv->pnl_lib_version);
		_fill_to_adl_sram3_data(pu8OD_SRAM3, u16TableIndex, u16SepatateMode, priv->pnl_lib_version);
		_fill_to_adl_sram4_data(pu8OD_SRAM4, u16TableIndex, u16SepatateMode, priv->pnl_lib_version);
    }

#if (OVERDRIVE_ENABLE_RGBW == TRUE)//not support w channel
    if (MHal_PNL_OverDriver_IsEnableRGBW())
    {
        if (R2BYTEMSK(L_BK_LPLL(0x03),BIT(5))!= 0)
        {
            MHal_PNL_PowerDownLPLL(FALSE);
        }
        W2BYTEMSK(REG_01DC_OD_1ST_BKA336_V004, 1, REG_01DC_OD_1ST_BKA336_V004_REG_OD_RGBW_EN_01DC);
    }
#endif

finally:
    PNL_FREE_MEM(pu8OD_SRAM1);
    PNL_FREE_MEM(pu8OD_SRAM2);
    PNL_FREE_MEM(pu8OD_SRAM3);
    PNL_FREE_MEM(pu8OD_SRAM4);

    return bRet;
}

void _overdriver_adl_fire(
	struct udevice *dev, uint32_t u32ADLSize, bool bEnableOD)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
	if (priv->pnl_lib_version == VERSION6) {
		if (!g_pstOD_ADL_DATA_V6) {
			TCON_ERROR("Auto download command is null\n");
			return;
		}
	} else {
		if (!g_pstOD_ADL_DATA) {
			TCON_ERROR("Auto download command is null\n");
			return;
		}
	}

    if (KHal_XC_WriteAutoDownload(
            E_PNL_XC_AUTODOWNLOAD_CLIENT_OD,
			(priv->pnl_lib_version == VERSION6) ?
			(u8 *)g_pstOD_ADL_DATA_V6 : (u8 *)g_pstOD_ADL_DATA,
            u32ADLSize, NULL))
    {
        if(KHal_XC_FireAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_OD) != TRUE)
            TCON_ERROR("OD, FIRE ADL Fail!!! \n");
    }
    else
    {
        TCON_ERROR("OD, Write ADL Fail!!! \n");
    }

#endif
    W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004,
                (bEnableOD ? 1 : 0),
                REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040); // OD enable
}

void _overdriver_setting_tbl(uint16_t *pu16ODTbl, uint32_t u32ODTabSize, uint16_t u16SepatateMode)
{
#if SUPPORT_OVERDRIVE
    if(!pu16ODTbl)
    {
        return;
    }
    bool bEnable = FALSE;
    uint8_t* pu8OD_SRAM1 = NULL;
    uint8_t* pu8OD_SRAM2 = NULL;
    uint8_t* pu8OD_SRAM3 = NULL;
    uint8_t* pu8OD_SRAM4 = NULL;
    bool bRet = TRUE;

    PNL_MALLOC_MEM(pu8OD_SRAM1, OVERDRIVER_SRAM1_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM2, OVERDRIVER_SRAM2_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM3, OVERDRIVER_SRAM3_SIZE, bRet);
    PNL_MALLOC_MEM(pu8OD_SRAM4, OVERDRIVER_SRAM4_SIZE, bRet);

    if (!bRet)
    {
        TCON_ERROR("Error: malloc od sram failed\n");
        goto finally;
    }

    //Decrypt OD table.
    _prepare_od_tbl_to_sram(pu16ODTbl, pu8OD_SRAM1, pu8OD_SRAM2, pu8OD_SRAM3, pu8OD_SRAM4);

    bEnable = R2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);

    //set od disable
    W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, 0x0, REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);

    //Disable Overdriver RGBW to prevent write od table fail
    _set_od_rgbw_enable(FALSE);

    _overdriver_setting_without_adl(pu8OD_SRAM1, pu8OD_SRAM2, pu8OD_SRAM3, pu8OD_SRAM4, u16SepatateMode);

    //recover od setting
    _set_od_rgbw_enable(bEnable);
    W2BYTEMSK(REG_0040_OD_1ST_BKA336_V004, (bEnable ? 1 : 0), REG_0040_OD_1ST_BKA336_V004_REG_OD_EN_0040);

finally:
    PNL_FREE_MEM(pu8OD_SRAM1);
    PNL_FREE_MEM(pu8OD_SRAM2);
    PNL_FREE_MEM(pu8OD_SRAM3);
    PNL_FREE_MEM(pu8OD_SRAM4);
#endif
}

void _overdriver_init_proc(
                           struct udevice *dev,
                           u64 u64OD_MSB_Addr, u64 u64OD_LSB_Addr,
                           uint16_t* pu16ODTbl, uint32_t u32ODTabSize)
{
    u64 u64Offset;
    uint8_t u8MIUSel = 0;
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    if (priv == NULL) {
        debug("[%s] get device private fail\n", __func__);
        return;
    }

    KHal_XC_GetMiuOffset(u8MIUSel, &u64Offset);
    u64OD_MSB_Addr = u64OD_MSB_Addr - u64Offset;
    u64OD_LSB_Addr = u64OD_LSB_Addr - u64Offset;
    u64OD_MSB_Addr = (u64OD_MSB_Addr / BYTE_PER_WORD) & 0xFFFFFFFF;
    u64OD_LSB_Addr = (u64OD_LSB_Addr / BYTE_PER_WORD) & 0xFFFFFFFF;
    if (SUPPORT_OVERDRIVE)
    {
        _overdriver_init(dev, u64OD_MSB_Addr, u64OD_LSB_Addr);
    }
}

void _overdriver_riu_setting_tbl(
                        struct udevice *dev,
                        u64 u64OD_MSB_Addr, u64 u64OD_LSB_Addr,
                        uint16_t* pu16ODTbl, uint32_t u32ODTabSize,uint16_t u16SepatateMode)
{
    u64 u64Offset;
    uint8_t u8MIUSel = 0;
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    if (priv == NULL)
    {
        TCON_DEBUG("get device private fail\n");
        return;
    }

    KHal_XC_GetMiuOffset(u8MIUSel, &u64Offset);
    u64OD_MSB_Addr = u64OD_MSB_Addr - u64Offset;
    u64OD_LSB_Addr = u64OD_LSB_Addr - u64Offset;
    u64OD_MSB_Addr = (u64OD_MSB_Addr / BYTE_PER_WORD) & 0xFFFFFFFF;
    u64OD_LSB_Addr = (u64OD_LSB_Addr / BYTE_PER_WORD) & 0xFFFFFFFF;

    if (SUPPORT_OVERDRIVE)
    {
        _overdriver_init(dev, u64OD_MSB_Addr, u64OD_LSB_Addr);
        _overdriver_setting_tbl(pu16ODTbl, u32ODTabSize, u16SepatateMode);
    }
}

bool _mtk_tcon_od_reg_setting(struct udevice *dev, uint8_t* pu8TconTab)
{
    bool bRet = TRUE;
    bool bSupportWChannel = FALSE;
    u64 _u32OD_MSB_Addr = 0;
    u64 _u32OD_LSB_Addr = 0;
    struct dts_mmap mmap;
    int ret = 0;
    uint8_t u8RGBSeparateFlag = 0;
    uint8_t u8MaxODSizeFlag = 0;
    uint8_t u8TConBinVersion = 0;
    uint16_t u16TargetIndex = 0;
    uint16_t u16SepatateMode = 0;
    uint16_t u16Index = 0;
    uint16_t* pu16OverDriveTable = NULL;
    uint32_t _ODTbl_Size;
    uint32_t u32ADLSize = XC_AUTODOWNLOAD_OVERDRIVER_DEPTH*BYTE_PER_WORD;
    bool bEnableOD = FALSE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    u8RGBSeparateFlag = (*((unsigned char*)(pu8TconTab + TCON_BIN_HEADER_BYTE_NUBMER(2)))) & BIT(0);
    u8MaxODSizeFlag = ((*((unsigned char*)(pu8TconTab + TCON_BIN_HEADER_BYTE_NUBMER(2)))) & BIT(2)) >> 2;
    u8TConBinVersion = (*((unsigned char*)(pu8TconTab + TCON_BIN_HEADER_BYTE_NUBMER(3))));

    if (u8TConBinVersion >= TCON_BIN_VERSION_RGBW)
    {
        bSupportWChannel =
                    ((*((unsigned char*)(pu8TconTab + TCON_BIN_HEADER_BYTE_NUBMER(2)))) & BIT(4)) >> 4;
        u16SepatateMode = E_PNL_OVERDRIVER_SEPARATE_R_ENABLE|E_PNL_OVERDRIVER_SEPARATE_G_ENABLE|
                            E_PNL_OVERDRIVER_SEPARATE_B_ENABLE|E_PNL_OVERDRIVER_SEPARATE_W_ENABLE;
        u16TargetIndex = TCON_BIN_HEADER_BYTE_NUBMER(5);
    }
    else
    {
        u16SepatateMode = E_PNL_OVERDRIVER_SEPARATE_R_ENABLE|E_PNL_OVERDRIVER_SEPARATE_G_ENABLE|
                            E_PNL_OVERDRIVER_SEPARATE_B_ENABLE;
        u16TargetIndex = TCON_BIN_HEADER_BYTE_NUBMER(4);
    }
    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_DISP_OD_BUF", mmap_dt_parser, (void*)&mmap,"reg");
    if (ret < 0 || mmap.address == 0)
    {
        TCON_ERROR("Error: Get MI_DISP_OD_BUF buffer from DTS mmap failure\n");
        return FALSE;
    }
    else
    {
        _u32OD_MSB_Addr = mmap.address;//BA address
        _u32OD_LSB_Addr = mmap.address + mmap.size;
        TCON_DEBUG("od address = %llx, size = %llx \n", mmap.address, mmap.size);
    }

    _overdriver_enable_wchannel(bSupportWChannel);

    TCON_DEBUG("u8RGBSeparateFlag=%d u8MaxODSizeFlag=%d u8TConBinVersion=%d bSupportWChannel=%d\n", \
                u8RGBSeparateFlag, u8MaxODSizeFlag, u8TConBinVersion, bSupportWChannel);

    if (!u8MaxODSizeFlag)
    {
        _ODTbl_Size = OVERDRIVER_TABLE_SIZE_33X32;
    }
    else
    {
        _ODTbl_Size = OVERDRIVER_TABLE_SIZE_33X33;
    }

    PNL_MALLOC_MEM(pu16OverDriveTable, sizeof(uint16_t) * MAX_OVERDRIVER_TABLE, bRet);

    if (u8RGBSeparateFlag)//RGB sepatate
    {
#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
        _overdriver_init_proc(
                    dev, _u32OD_MSB_Addr, _u32OD_LSB_Addr, pu16OverDriveTable, _ODTbl_Size);
        bEnableOD = _overdriver_adl_init(dev, u32ADLSize);
#endif
        for (u16Index = 0; u16Index < _ODTbl_Size; u16Index++)
        {
            pu16OverDriveTable[u16Index] = pu8TconTab[u16Index + u16TargetIndex];
        }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
        _overdriver_adl_setting_tbl(
			dev, pu16OverDriveTable, E_PNL_OVERDRIVER_SEPARATE_R_ENABLE);
#else
        _overdriver_riu_setting_tbl(dev,
                            _u32OD_MSB_Addr,
                            _u32OD_LSB_Addr,
                            pu16OverDriveTable,
                            _ODTbl_Size,
                            E_PNL_OVERDRIVER_SEPARATE_R_ENABLE);
#endif

        u16TargetIndex = u16TargetIndex + u16Index;
        for (u16Index = 0; u16Index < _ODTbl_Size; u16Index++)
        {
            pu16OverDriveTable[u16Index] = pu8TconTab[u16Index + u16TargetIndex];
        }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
        _overdriver_adl_setting_tbl(
			dev, pu16OverDriveTable, E_PNL_OVERDRIVER_SEPARATE_G_ENABLE);
#else
        _overdriver_riu_setting_tbl(dev,
                            _u32OD_MSB_Addr,
                            _u32OD_LSB_Addr,
                            pu16OverDriveTable,
                            _ODTbl_Size,
                            E_PNL_OVERDRIVER_SEPARATE_G_ENABLE);
#endif

        u16TargetIndex = u16TargetIndex + u16Index;
        for (u16Index = 0; u16Index < _ODTbl_Size; u16Index++)
        {
            pu16OverDriveTable[u16Index] = pu8TconTab[u16Index + u16TargetIndex];
        }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
        _overdriver_adl_setting_tbl(
			dev, pu16OverDriveTable, E_PNL_OVERDRIVER_SEPARATE_B_ENABLE);
#else
        _overdriver_riu_setting_tbl(dev,
                            _u32OD_MSB_Addr,
                            _u32OD_LSB_Addr,
                            pu16OverDriveTable,
                            _ODTbl_Size,
                            E_PNL_OVERDRIVER_SEPARATE_B_ENABLE);
#endif

        if (bSupportWChannel)
        {
            u16TargetIndex = u16TargetIndex + u16Index;
            for (u16Index = 0; u16Index < _ODTbl_Size; u16Index++)
            {
                pu16OverDriveTable[u16Index] = pu8TconTab[u16Index+u16TargetIndex];
            }

#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_OD_AUTODOWNLOAD == TRUE)
            _overdriver_adl_setting_tbl(
				dev, pu16OverDriveTable, E_PNL_OVERDRIVER_SEPARATE_W_ENABLE);
#else
            _overdriver_riu_setting_tbl(dev,
                                _u32OD_MSB_Addr,
                                _u32OD_LSB_Addr,
                                pu16OverDriveTable,
                                _ODTbl_Size,
                                E_PNL_OVERDRIVER_SEPARATE_W_ENABLE);
#endif

        }

		_overdriver_adl_fire(dev, u32ADLSize, bEnableOD);
    }
    else
    {
        for (u16Index = 0; u16Index < _ODTbl_Size; u16Index++)
        {
            pu16OverDriveTable[u16Index] = pu8TconTab[u16Index + u16TargetIndex];
        }

        _overdriver_riu_setting_tbl(dev,
                            _u32OD_MSB_Addr,
                            _u32OD_LSB_Addr,
                            pu16OverDriveTable,
                            _ODTbl_Size,
                            u16SepatateMode);

    }

    PNL_FREE_MEM(pu16OverDriveTable);
    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool mtk_tcon_od_setting(struct udevice *dev)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    bool data_exist = FALSE;
    loff_t data_len = 0 ;
    unsigned char *pdata_buf = NULL;
    st_tcon_tab_info stInfo;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(priv);

    if (priv->tcon_info.bOverDriveEn)
    {
        data_exist = is_tcon_data_exist(&pdata_buf, &data_len);

        if (data_exist)
        {
            memset(&stInfo, 0, sizeof(st_tcon_tab_info));
            stInfo.pu8Table = pdata_buf;
            stInfo.u8TconType = E_TCON_TAB_TYPE_OVERDRIVER;
            if (get_tcon_dump_table(&stInfo) == TRUE)
            {
                if(stInfo.u8Version > TCON20_VERSION)
                {
                    _mtk_tcon_od_reg_setting(dev, stInfo.pu8Table);
                }
			_set_od_enable(dev, priv->tcon_info.bvrr_od_en ? FALSE : TRUE);
            }
            else
            {
                _set_od_enable(dev,FALSE);
            }
        }
    }
    else
    {
        TCON_DEBUG("OD is off...\n");
        _set_od_enable(dev,FALSE);
    }
    TCON_FUNC_EXIT(bRet);

    return bRet;
}

