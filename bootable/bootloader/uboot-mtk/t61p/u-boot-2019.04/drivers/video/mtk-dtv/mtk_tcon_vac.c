// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
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
#include "coda/PAFRC_BKA324_V004.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_tcon_vac.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_autodownload.h"

// Debug related
#define PNL_RGB_LUT_CHECK(R, G, B) ((R) == NULL || (G) == NULL || (B) == NULL)

//EVA Table
#define EVA_MAX_VALUE_OFFSET  (0x4)
#define EVA_MAX_VALUE_CNANNEL  (6)
#define EVA_MAINHEADER_OFFSET  (0x18)
#define EVA_DATA_START         (0x1B)
#define EVA_TABLE_MATRIX_SIZE_17X2    (17)
#define EVA_TABLE_MATRIX_SIZE_257X2   (257)
#define EVA_LUT_SIZE_16    (16)
#define EVA_LUT_SIZE_256   (256)
#define EVA_TABLE_RGB_CNANNEL  (3)
#define EVA_TABLE_UNIT_SIZE_8  (0)
#define EVA_TABLE_UNIT_SIZE_12 (1)

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

//VAC ADL
#define XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET   (0x200) //16 entry * 32 byte
#define XC_AUTODOWNLOAD_VAC_MEM_SIZE         (0xC00) // (48+48)*32 bytes

#define TCON_REGISTERTYPE_1_BYTES   1
#define TCON_REGISTERTYPE_6_BYTES   6
#define TCON_REGISTERTYPE_4_BYTES   4
#define TCON_REGISTERTYPE_POWERSEQENCE_9_BYTES   9
#define TCON_REGISTERTYPE_POWERSEQENCE_7_BYTES   7
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_15_BYTES   15
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_11_BYTES   11
#define TCON_REGISTERTYPE_POWERSEQENCE_WITHSUB_14_BYTES   14

#define TCON_BIN_VERSION_RGBW (1)
#define TCON_BIN_HEADER_BYTE_NUBMER(byte_n) (byte_n)

typedef struct __attribute__((packed))
{
    //uint16_t u16Version;              //[15:8]: major, [7:0]: minor version
    //uint8_t u8Eva_en;                 //[0]: eva enable
    uint8_t u8SeparateFlag;             //[0]: channel separate flag [1]: table separate flag (unused)
    //uint32_t u32YearMonthDayMinute;   //[31:25]: reserved, [24:20]: year, [19:16]: month, [15:11]: day,
                                        //[10:0]: minute
    uint8_t u8TableMatrix;              //0: 17x2, 1: 257x2
    uint8_t u8TableBit;                 //0: 8bit 1:12bit
    //uint8_t u8Reserved1;
    //uint8_t u8Reserved2;
    //uint8_t u8Reserved3;
} ST_PNL_EVA_SUB_HEADER, *pST_PNL_EVA_SUB_HEADER;

typedef struct DLL_PACKED
{
    uint16_t  u16TblSize;                /// IN: size
    uint16_t* pu16RChannel0;               /// IN/OUT: R channel 0 data
#if !defined (__aarch64__)
    void *pDummy0;                     /// Dummy parameter
#endif
    uint16_t* pu16GChannel0;               /// IN/OUT: G channel 0 data
#if !defined (__aarch64__)
    void *pDummy1;                     /// Dummy parameter
#endif
    uint16_t* pu16BChannel0;               /// IN/OUT:: B channel 0 data
#if !defined (__aarch64__)
    void *pDummy2;                     /// Dummy parameter
#endif
    uint16_t* pu16RChannel1;               /// IN/OUT:: R channel 1 data
#if !defined (__aarch64__)
    void *pDummy3;                     /// Dummy parameter
#endif
    uint16_t* pu16GChannel1;               /// IN/OUT:: G channel 1 data
#if !defined (__aarch64__)
    void *pDummy4;                     /// Dummy parameter
#endif
    uint16_t* pu16BChannel1;               /// IN/OUT:: B channel 1 data
#if !defined (__aarch64__)
    void *pDummy5;                     /// Dummy parameter
#endif
    uint16_t* pu16RChannel0_17;             /// IN: R channel 0 data
#if !defined (__aarch64__)
    void *pDummy6;                     /// Dummy parameter
#endif
    uint16_t* pu16GChannel0_17;             /// IN: G channel 0 data
#if !defined (__aarch64__)
    void *pDummy7;                     /// Dummy parameter
#endif
    uint16_t* pu16BChannel0_17;             /// IN: B channel 0 data
#if !defined (__aarch64__)
    void *pDummy8;                     /// Dummy parameter
#endif
    uint16_t* pu16RChannel1_17;             /// IN: R channel 1 data
#if !defined (__aarch64__)
    void *pDummy9;                     /// Dummy parameter
#endif
    uint16_t* pu16GChannel1_17;             /// IN: G channel 1 data
#if !defined (__aarch64__)
    void *pDummy10;                     /// Dummy parameter
#endif
    uint16_t* pu16BChannel1_17;             /// IN: B channel 1 data
#if !defined (__aarch64__)
    void *pDummy11;                     /// Dummy parameter
#endif
}ST_PNL_EVA_TBL;

//Dump EVA Table
void _set_vac_bypass(bool bEnable)
{
#if SUPPORT_VAC_256
    W2BYTEMSK(REG_001C_PAFRC_BKA324_V004, (bEnable? 1:0), REG_001C_PAFRC_BKA324_V004_REG_ADVAC_BYPASS_001C);
#endif
}

bool _get_vac_bypass(void)
{
#if SUPPORT_VAC_256
    if (R2BYTEMSK(REG_001C_PAFRC_BKA324_V004, REG_001C_PAFRC_BKA324_V004_REG_ADVAC_BYPASS_001C) >> 15)
    {
        return TRUE;
    }
    else
#endif
    {
        return FALSE;
    }
}

void _set_vac_enable(bool bEnable)
{
    W2BYTEMSK(REG_003C_PAFRC_BKA324_V004,
                (bEnable? 1:0),
                REG_003C_PAFRC_BKA324_V004_REG_VAC_CURVE_FIT_EN_003C);//bit[0]:vac_en
}

bool _get_vac_enable(void)
{
    return (R2BYTEMSK(REG_003C_PAFRC_BKA324_V004, REG_003C_PAFRC_BKA324_V004_REG_VAC_CURVE_FIT_EN_003C));
}

bool _set_eva_tbl(ST_PNL_EVA_TBL *pstEvaTbl)
{
#if SUPPORT_VAC_256
    uint8_t u8Channel =0;
    uint16_t u16Index = 0;
    bool bVACBypass = 0;
    bool bVACEnable = 0;
    uint16_t u16Timeout = 0;
    uint16_t* p16TmpTbl0 = NULL;
    uint16_t* p16TmpTbl1 = NULL;

    // Store VAC Bypass and enable flag
    bVACBypass = _get_vac_bypass();
    bVACEnable = _get_vac_enable();

    TCON_DEBUG("bVACBypass=%d bVACEnable=%d\n.", bVACBypass, bVACEnable);

    _set_vac_bypass(FALSE);
    _set_vac_enable(FALSE);

    p16TmpTbl0 = pstEvaTbl->pu16RChannel0;
    p16TmpTbl1 = pstEvaTbl->pu16RChannel1;

    for (u8Channel = 0; u8Channel < EVA_TABLE_RGB_CNANNEL; u8Channel ++)
    {
        //1.  channel select
        W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                    u8Channel,
                    REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_CH_SEL_0140);//bit[6:5]: 00=R, 01=B, 10=B

        if (u8Channel == 1)//G
        {
            p16TmpTbl0 = pstEvaTbl->pu16GChannel0;
            p16TmpTbl1 = pstEvaTbl->pu16GChannel1;
        }
        else if (u8Channel == 2)//B
        {
            p16TmpTbl0 = pstEvaTbl->pu16BChannel0;
            p16TmpTbl1 = pstEvaTbl->pu16BChannel1;
        }

        for (u16Index = 0; u16Index < EVA_LUT_SIZE_256; u16Index ++)//0~255 for lut0, 256~511 for lut1
        {
        //lut0
            //2. Addr
            W2BYTEMSK(REG_0144_PAFRC_BKA324_V004,
                        u16Index,
                        REG_0144_PAFRC_BKA324_V004_REG_VAC_GAMMA_ADR_0144);//bit[9:0]
            //3. Data
            W2BYTEMSK(REG_0148_PAFRC_BKA324_V004,
                        p16TmpTbl0[u16Index],
                        REG_0148_PAFRC_BKA324_V004_REG_VAC_GAMMA_DATA_0148);//bit[11:0]
            //4. toggle LUT_W_FLAG
            W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                        1,
                        REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_WR_EN_0140);//bit[3]
            // after setting toggle bit, we need to make sure the bit is already toggled.
            while(R2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                                REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_WR_EN_0140) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }

        //lut1
            //2. Addr
            W2BYTEMSK(REG_0144_PAFRC_BKA324_V004,
                        u16Index+EVA_LUT_SIZE_256,
                        REG_0144_PAFRC_BKA324_V004_REG_VAC_GAMMA_ADR_0144);//bit[9:0]
            //3. Data
            W2BYTEMSK(REG_0148_PAFRC_BKA324_V004,
                        p16TmpTbl1[u16Index],
                        REG_0148_PAFRC_BKA324_V004_REG_VAC_GAMMA_DATA_0148);//bit[11:0]

            //4. toggle LUT_W_FLAG
            W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                        1,
                        REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_WR_EN_0140);//bit[3]

            // after setting toggle bit, we need to make sure the bit is already toggled.
            while(R2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                                REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_WR_EN_0140) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
        }
    }

    //Restore VAC status
    if (bVACEnable)
    {
        _set_vac_enable(TRUE);
    }

#endif
    return TRUE;
}

bool _get_eva_from_reg(ST_PNL_EVA_TBL *pstEvaTbl)
{
#if SUPPORT_VAC_256
    uint8_t u8Channel =0;
    uint16_t u16Index = 0;
    bool bVACBypass = 0;
    bool bVACEnable = 0;
    uint16_t u16Timeout = 0;
    uint16_t* p16TmpTbl0 = NULL;
    uint16_t* p16TmpTbl1 = NULL;

    bVACBypass = _get_vac_bypass();
    bVACEnable = _get_vac_enable();

    TCON_DEBUG("bVACBypass=%d bVACEnable=%d\n.", bVACBypass, bVACEnable);

    if (PNL_RGB_LUT_CHECK(pstEvaTbl->pu16RChannel0, pstEvaTbl->pu16GChannel0, pstEvaTbl->pu16BChannel0) ||
       PNL_RGB_LUT_CHECK(pstEvaTbl->pu16RChannel1, pstEvaTbl->pu16GChannel1, pstEvaTbl->pu16BChannel1))
    {
        TCON_DEBUG("Table error \n.");
        return FALSE;
    }

    _set_vac_bypass(FALSE); // VSC is ON

    p16TmpTbl0 = pstEvaTbl->pu16RChannel0;
    p16TmpTbl1 = pstEvaTbl->pu16RChannel1;

    for (u8Channel = 0; u8Channel < EVA_TABLE_RGB_CNANNEL; u8Channel ++)
    {
        //1.  channel select
        W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                    u8Channel,
                    REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_CH_SEL_0140);//bit[6:5]: 00=R, 01=B, 10=B

        if (u8Channel == 1)//G
        {
            p16TmpTbl0 = pstEvaTbl->pu16GChannel0;
            p16TmpTbl1 = pstEvaTbl->pu16GChannel1;
        }
        else if (u8Channel == 2)//B
        {
            p16TmpTbl0 = pstEvaTbl->pu16BChannel0;
            p16TmpTbl1 = pstEvaTbl->pu16BChannel1;
        }

        for (u16Index = 0; u16Index < EVA_LUT_SIZE_256; u16Index ++)//0~255 for lut0, 256~511 for lut1
        {
        //lut0
            //2. Addr
            W2BYTEMSK(REG_0144_PAFRC_BKA324_V004,
                        u16Index,
                        REG_0144_PAFRC_BKA324_V004_REG_VAC_GAMMA_ADR_0144);//bit[9:0]

            //3. toggle LUT_R_FLAG
            W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                        1,
                        REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_EN_0140);//bit[4]

            // after setting toggle bit, we need to make sure the bit is already toggled.
            while (R2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                                REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_EN_0140) &&
                                (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            //4. Data
            p16TmpTbl0[u16Index] = R2BYTEMSK(REG_0150_PAFRC_BKA324_V004,
                                        REG_0150_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_DATA_0150);//bit[11:0]
         //lut1
            //2. Addr
            W2BYTEMSK(REG_0144_PAFRC_BKA324_V004,
                        u16Index+EVA_LUT_SIZE_256,
                        REG_0144_PAFRC_BKA324_V004_REG_VAC_GAMMA_ADR_0144);//bit[9:0]

            //3. toggle LUT_R_FLAG
            W2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                        1,
                        REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_EN_0140);//bit[4]

            // after setting toggle bit, we need to make sure the bit is already toggled.
            while(R2BYTEMSK(REG_0140_PAFRC_BKA324_V004,
                            REG_0140_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_EN_0140) &&
                            (u16Timeout < MAX_DELAY_TIME))
            {
                mdelay(1);
                u16Timeout++;
            }
            //4. Data
            p16TmpTbl1[u16Index] = R2BYTEMSK(REG_0150_PAFRC_BKA324_V004,
                                        REG_0150_PAFRC_BKA324_V004_REG_VAC_GAMMA_RD_DATA_0150);//bit[11:0]
        }
    }

    //Restore VAC status
    if(bVACEnable)
    {
        _set_vac_enable(TRUE);
    }

#endif
    return TRUE;
}

bool _use_adl_seteva_tbl(ST_PNL_EVA_TBL *pstEvaTbl)
{
#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_VAC_AUTODOWNLOAD == TRUE)
    uint32_t u32index = 0;
    uint8_t *pu8EvaTbl;
    bool bRet = TRUE;
    uint8_t *pu8R0 = NULL, *pu8G0 = NULL, *pu8B0 = NULL, *pu8R1 = NULL, *pu8G1 = NULL, *pu8B1 = NULL;

    PNL_MALLOC_MEM(pu8EvaTbl, XC_AUTODOWNLOAD_VAC_MEM_SIZE, bRet);

    if (bRet)
    {
       pu8R0 = pu8EvaTbl;
       pu8G0 = pu8R0 + XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET;  // 16entry  * 32 byte
       pu8B0 = pu8G0 + XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET;
       pu8R1 = pu8B0 + XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET;
       pu8G1 = pu8R1 + XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET;
       pu8B1 = pu8G1 + XC_AUTODOWNLOAD_VAC_CHANNEL_OFFSET;

       for(u32index = 0 ; u32index < EVA_LUT_SIZE_256; u32index++)
       {
           //LUT 0
           *((uint16_t *)pu8R0 + u32index) = pstEvaTbl->pu16RChannel0[u32index];
           *((uint16_t *)pu8G0 + u32index) = pstEvaTbl->pu16GChannel0[u32index];
           *((uint16_t *)pu8B0 + u32index) = pstEvaTbl->pu16BChannel0[u32index];
           //LUT 1
           *((uint16_t *)pu8R1 + u32index) = pstEvaTbl->pu16RChannel1[u32index];
           *((uint16_t *)pu8G1 + u32index) = pstEvaTbl->pu16GChannel1[u32index];
           *((uint16_t *)pu8B1 + u32index) = pstEvaTbl->pu16BChannel1[u32index];
       }

        // Write ADL
        if(KHal_XC_WriteAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC,
                                    pu8EvaTbl, XC_AUTODOWNLOAD_VAC_MEM_SIZE, NULL)!= TRUE)
        {
            UBOOT_ERROR("VAC,Write ADL Fail!!!!!!!!!!\n");
            PNL_FREE_MEM(pu8EvaTbl);
            return FALSE;
        }

        //Fire ADL
        if(KHal_XC_FireAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_VAC) != TRUE)
        {
            UBOOT_ERROR("VAC,Fire ADL Fail!!!!!!!!!!\n");
            PNL_FREE_MEM(pu8EvaTbl);
            return FALSE;
        }
        PNL_FREE_MEM(pu8EvaTbl);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
#else
    return FALSE;
#endif
}

//EVA Table
static bool _evatbl_mem_allocate(ST_PNL_EVA_TBL *pstEvaTbl)
{
    bool bRet = TRUE;

    PNL_MALLOC_MEM(pstEvaTbl->pu16RChannel0, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);
    PNL_MALLOC_MEM(pstEvaTbl->pu16GChannel0, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);
    PNL_MALLOC_MEM(pstEvaTbl->pu16BChannel0, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);

    PNL_MALLOC_MEM(pstEvaTbl->pu16RChannel1, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);
    PNL_MALLOC_MEM(pstEvaTbl->pu16GChannel1, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);
    PNL_MALLOC_MEM(pstEvaTbl->pu16BChannel1, sizeof(uint16_t)*EVA_TABLE_MATRIX_SIZE_257X2, bRet);

    if (!bRet)
    {
        TCON_ERROR("EVA_MALLOC_MEM Fail!!!!!!!!!!\n");

        PNL_FREE_MEM(pstEvaTbl->pu16RChannel0);
        PNL_FREE_MEM(pstEvaTbl->pu16GChannel0);
        PNL_FREE_MEM(pstEvaTbl->pu16BChannel0);
        PNL_FREE_MEM(pstEvaTbl->pu16RChannel1);
        PNL_FREE_MEM(pstEvaTbl->pu16GChannel1);
        PNL_FREE_MEM(pstEvaTbl->pu16BChannel1);
        return bRet;
    }

    if (pstEvaTbl->u16TblSize == EVA_TABLE_MATRIX_SIZE_17X2)
    {
        PNL_MALLOC_MEM(pstEvaTbl->pu16RChannel0_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);
        PNL_MALLOC_MEM(pstEvaTbl->pu16GChannel0_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);
        PNL_MALLOC_MEM(pstEvaTbl->pu16BChannel0_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);

        PNL_MALLOC_MEM(pstEvaTbl->pu16RChannel1_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);
        PNL_MALLOC_MEM(pstEvaTbl->pu16GChannel1_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);
        PNL_MALLOC_MEM(pstEvaTbl->pu16BChannel1_17, sizeof(uint16_t)*pstEvaTbl->u16TblSize, bRet);

        if (!bRet)
        {
            TCON_ERROR("EVA_MALLOC_MEM Fail!!!!!!!!!!\n");

            PNL_FREE_MEM(pstEvaTbl->pu16RChannel0_17);
            PNL_FREE_MEM(pstEvaTbl->pu16GChannel0_17);
            PNL_FREE_MEM(pstEvaTbl->pu16BChannel0_17);

            PNL_FREE_MEM(pstEvaTbl->pu16RChannel1_17);
            PNL_FREE_MEM(pstEvaTbl->pu16GChannel1_17);
            PNL_FREE_MEM(pstEvaTbl->pu16BChannel1_17);
            return bRet;
        }
    }
    return bRet;
}

void _convert_eva_17to257_tbl(ST_PNL_EVA_TBL *pstEvaTbl)
{
#define EVA_CONVERT_UNIT (16)
    uint16_t u16Index = 0;
    uint16_t i = 0;

    if (PNL_RGB_LUT_CHECK(pstEvaTbl->pu16RChannel0, pstEvaTbl->pu16GChannel0, pstEvaTbl->pu16BChannel0) ||
        PNL_RGB_LUT_CHECK(pstEvaTbl->pu16RChannel1, pstEvaTbl->pu16GChannel1, pstEvaTbl->pu16BChannel1) ||
        PNL_RGB_LUT_CHECK(
                pstEvaTbl->pu16RChannel0_17, pstEvaTbl->pu16GChannel0_17, pstEvaTbl->pu16BChannel0_17) ||
        PNL_RGB_LUT_CHECK(
                pstEvaTbl->pu16RChannel1_17, pstEvaTbl->pu16GChannel1_17, pstEvaTbl->pu16BChannel1_17))
    {
        TCON_ERROR("RGB Table is null\n.");
        return;
    }

    if (pstEvaTbl->u16TblSize == EVA_TABLE_MATRIX_SIZE_257X2)
    {
        return;
    }

    for (u16Index = 0; u16Index < EVA_TABLE_MATRIX_SIZE_257X2; u16Index ++) //0~256
    {
        i = u16Index/EVA_CONVERT_UNIT;
        if (i+1 < pstEvaTbl->u16TblSize)
        {
            //llut0
            pstEvaTbl->pu16RChannel0[u16Index] = ((pstEvaTbl->pu16RChannel0_17[i+1]-
                                                pstEvaTbl->pu16RChannel0_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16RChannel0_17[i])*EVA_CONVERT_UNIT);

            pstEvaTbl->pu16GChannel0[u16Index] = ((pstEvaTbl->pu16GChannel0_17[i+1]-
                                                pstEvaTbl->pu16GChannel0_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16GChannel0_17[i])*EVA_CONVERT_UNIT);

            pstEvaTbl->pu16BChannel0[u16Index] = ((pstEvaTbl->pu16BChannel0_17[i+1]-
                                                pstEvaTbl->pu16BChannel0_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16BChannel0_17[i])*EVA_CONVERT_UNIT);

            //llut1
            pstEvaTbl->pu16RChannel1[u16Index] = ((pstEvaTbl->pu16RChannel1_17[i+1]-
                                                pstEvaTbl->pu16RChannel1_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16RChannel1_17[i])*EVA_CONVERT_UNIT);

            pstEvaTbl->pu16GChannel1[u16Index] = ((pstEvaTbl->pu16GChannel1_17[i+1]-
                                                pstEvaTbl->pu16GChannel1_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16GChannel1_17[i])*EVA_CONVERT_UNIT);

            pstEvaTbl->pu16BChannel1[u16Index] = ((pstEvaTbl->pu16BChannel1_17[i+1]-
                                                pstEvaTbl->pu16BChannel1_17[i])*(u16Index%EVA_CONVERT_UNIT))+
                                                  ((pstEvaTbl->pu16BChannel1_17[i])*EVA_CONVERT_UNIT);
        }
        else // i=16 max value
        {
            pstEvaTbl->pu16RChannel0[u16Index]= pstEvaTbl->pu16RChannel0_17[i]*EVA_CONVERT_UNIT;
            pstEvaTbl->pu16GChannel0[u16Index]= pstEvaTbl->pu16GChannel0_17[i]*EVA_CONVERT_UNIT;
            pstEvaTbl->pu16BChannel0[u16Index]= pstEvaTbl->pu16BChannel0_17[i]*EVA_CONVERT_UNIT;

            pstEvaTbl->pu16RChannel1[u16Index]= pstEvaTbl->pu16RChannel1_17[i]*EVA_CONVERT_UNIT;
            pstEvaTbl->pu16GChannel1[u16Index]= pstEvaTbl->pu16GChannel1_17[i]*EVA_CONVERT_UNIT;
            pstEvaTbl->pu16BChannel1[u16Index]= pstEvaTbl->pu16BChannel1_17[i]*EVA_CONVERT_UNIT;
        }
    }
}

//Dump EVA Reg
bool mtk_tcon_vac_reg_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr = 0;
    uint8_t u8Mask = 0;
    uint8_t u8Value = 0;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = (((uint32_t)pu8TconTab[u32tabIdx]<<24) + ((uint32_t)pu8TconTab[(u32tabIdx +1)]<<16) +
                    ((uint32_t)pu8TconTab[(u32tabIdx +2)]<<8) + pu8TconTab[(u32tabIdx +3)]) & 0x7FFFFFFF;
        u8Mask  = pu8TconTab[(u32tabIdx +4)] & 0xFF;
        u8Value = pu8TconTab[(u32tabIdx +5)] & 0xFF;

        TCON_INFO("addr=%04tx, msk=%02x, val=%02x\n", (ptrdiff_t)u32Addr, u8Mask, u8Value);
        TCON_W1BYTEMSK(u32Addr, u8Value, u8Mask);

        u32tabIdx = u32tabIdx + 6;
    }

    TCON_FUNC_EXIT(TRUE);
    return TRUE;
}

bool mtk_tcon_vac_setting(struct udevice *dev)
{
#if SUPPORT_VAC_256
    uint16_t u16TargetIndex = 0;
    uint16_t pu16max_value[EVA_MAX_VALUE_CNANNEL];
    pST_PNL_EVA_SUB_HEADER pstEvaSubHdr;
    bool bRetLut = FALSE, bChannelSeparate = FALSE;
    ST_PNL_EVA_TBL stEvaTbl;
    uint16_t u16Index;
    unsigned char *pdata_buf = NULL;
    bool data_exist = FALSE;
    loff_t data_len = 0;
    uint8_t *TconDumpAll;
    st_tcon_tab_info stInfo;
    bool bRet = TRUE;
    int i;

    TCON_CHECK_PARAMETER_NULL(dev);

    data_exist = is_tcon_data_exist(&pdata_buf, &data_len);
    if (!data_exist)
    {
        TCON_DEBUG("VAC data does not exist.\n.");
        return FALSE;
    }
    else
    {
            TCON_DEBUG("VAC load success!\n.");
    }

    memset(&stInfo, 0, sizeof(st_tcon_tab_info));
    stInfo.pu8Table = pdata_buf;
    stInfo.u8TconType = E_TCON_TAB_TYPE_VAC_TABLE;

    if (get_tcon_dump_table(&stInfo) != TRUE)
    {
        return FALSE;
    }

    memcpy(pu16max_value, (stInfo.pu8Table+ EVA_MAX_VALUE_OFFSET), sizeof(uint16_t)*EVA_MAX_VALUE_CNANNEL);
    pstEvaSubHdr = (pST_PNL_EVA_SUB_HEADER)(stInfo.pu8Table + EVA_MAINHEADER_OFFSET);
    bChannelSeparate = pstEvaSubHdr->u8SeparateFlag & _BIT(0);

    TCON_DEBUG("channel seperate=%d table matrix=%d table bit=%d\n",
            bChannelSeparate, pstEvaSubHdr->u8TableMatrix, pstEvaSubHdr->u8TableBit);

    for (u16Index = 0; u16Index < EVA_MAX_VALUE_CNANNEL; u16Index++)
    {
        TCON_INFO("channel=%d max value=%d\n", u16Index, pu16max_value[u16Index]);
    }
    memset(&stEvaTbl, 0, sizeof(ST_PNL_EVA_TBL));
    memset(&TconDumpAll, 0, sizeof(TconDumpAll));
    stEvaTbl.u16TblSize = (pstEvaSubHdr->u8TableMatrix ? EVA_TABLE_MATRIX_SIZE_257X2 : EVA_TABLE_MATRIX_SIZE_17X2);

    if (_evatbl_mem_allocate(&stEvaTbl) == FALSE)
    {
        goto finally;
    }

    u16TargetIndex = EVA_DATA_START;
    if (stEvaTbl.u16TblSize == EVA_TABLE_MATRIX_SIZE_17X2)
    {
        // get table from bin
        if (bChannelSeparate)
        {
            PNL_MALLOC_MEM(TconDumpAll, sizeof(uint8_t)*stInfo.u32ReglistSize, bRet);

            if (!bRet)
            {
                TCON_ERROR("TconDumpAll allocate mem fail\n");
                goto finally;
            }
            memcpy(TconDumpAll, (stInfo.pu8Table+u16TargetIndex), sizeof(uint8_t)*stInfo.u32ReglistSize);

            for (i = 0; i < stEvaTbl.u16TblSize; i++){
                stEvaTbl.pu16RChannel0_17[i] = (uint8_t)TconDumpAll[3*i]; //R lut0
                stEvaTbl.pu16GChannel0_17[i] = (uint8_t)TconDumpAll[3*i+1]; //G lut0
                stEvaTbl.pu16BChannel0_17[i] = (uint8_t)TconDumpAll[3*i+2]; //B lut0
                stEvaTbl.pu16RChannel1_17[i] = (uint8_t)TconDumpAll[3*i+(uint32_t)stEvaTbl.u16TblSize*3]; //R lut1
                stEvaTbl.pu16GChannel1_17[i] = (uint8_t)TconDumpAll[3*i+1+(uint32_t)stEvaTbl.u16TblSize*3]; //G lut1
                stEvaTbl.pu16BChannel1_17[i] = (uint8_t)TconDumpAll[3*i+2+(uint32_t)stEvaTbl.u16TblSize*3]; //B lut1
            }

            TCON_INFO("====================RGB LUT0====================\n");

            for (i = 0; i < (uint32_t)stEvaTbl.u16TblSize; i++)
                TCON_INFO("%d  %d  %d\n.", stEvaTbl.pu16RChannel0_17[i], stEvaTbl.pu16GChannel0_17[i], stEvaTbl.pu16BChannel0_17[i]);

            TCON_INFO("\n====================RGB LUT1====================\n");
            for (i = 0; i < (uint32_t)stEvaTbl.u16TblSize; i++)
                TCON_INFO("%d  %d  %d\n.", stEvaTbl.pu16RChannel1_17[i], stEvaTbl.pu16GChannel1_17[i], stEvaTbl.pu16BChannel1_17[i]);
        }
        else
        {
            memcpy(stEvaTbl.pu16RChannel0_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut0
            memcpy(stEvaTbl.pu16GChannel0_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut0
            memcpy(stEvaTbl.pu16BChannel0_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut0
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16RChannel1_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut1
            memcpy(stEvaTbl.pu16GChannel1_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut1
            memcpy(stEvaTbl.pu16BChannel1_17, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut1
        }

        // replace last with max
        stEvaTbl.pu16RChannel0_17[stEvaTbl.u16TblSize-1] = pu16max_value[0]/16;
        stEvaTbl.pu16GChannel0_17[stEvaTbl.u16TblSize-1] = pu16max_value[1]/16;
        stEvaTbl.pu16BChannel0_17[stEvaTbl.u16TblSize-1] = pu16max_value[2]/16;
        stEvaTbl.pu16RChannel1_17[stEvaTbl.u16TblSize-1] = pu16max_value[3]/16;
        stEvaTbl.pu16GChannel1_17[stEvaTbl.u16TblSize-1] = pu16max_value[4]/16;
        stEvaTbl.pu16BChannel1_17[stEvaTbl.u16TblSize-1] = pu16max_value[5]/16;
        //convert to 257
        _convert_eva_17to257_tbl(&stEvaTbl);
    }
    else
    {
        // get table from bin
        if (bChannelSeparate)
        {
            memcpy(stEvaTbl.pu16RChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut0
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16RChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut1
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16GChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut0
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16GChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut1
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16BChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut0
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16BChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut1
        }
        else
        {
            memcpy(stEvaTbl.pu16RChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut0
            memcpy(stEvaTbl.pu16GChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut0
            memcpy(stEvaTbl.pu16BChannel0, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut0
            u16TargetIndex += stEvaTbl.u16TblSize*2;
            memcpy(stEvaTbl.pu16RChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //R lut1
            memcpy(stEvaTbl.pu16GChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //G lut1
            memcpy(stEvaTbl.pu16BChannel1, (stInfo.pu8Table+u16TargetIndex), sizeof(uint16_t)*stEvaTbl.u16TblSize); //B lut1
        }
    }

    bRetLut = _use_adl_seteva_tbl(&stEvaTbl);
    if (bRetLut != TRUE)
    {
        // riu
        bRetLut = _set_eva_tbl(&stEvaTbl);
    }

    if (bRetLut != TRUE)
    {
        TCON_DEBUG(" Eva table fail \n.");
    }

finally:
    PNL_FREE_MEM(stEvaTbl.pu16RChannel0);
    PNL_FREE_MEM(stEvaTbl.pu16GChannel0);
    PNL_FREE_MEM(stEvaTbl.pu16BChannel0);
    PNL_FREE_MEM(stEvaTbl.pu16RChannel1);
    PNL_FREE_MEM(stEvaTbl.pu16GChannel1);
    PNL_FREE_MEM(stEvaTbl.pu16BChannel1);

    if (stEvaTbl.u16TblSize == EVA_TABLE_MATRIX_SIZE_17X2)
    {
        PNL_FREE_MEM(stEvaTbl.pu16RChannel0_17);
        PNL_FREE_MEM(stEvaTbl.pu16GChannel0_17);
        PNL_FREE_MEM(stEvaTbl.pu16BChannel0_17);
        PNL_FREE_MEM(stEvaTbl.pu16RChannel1_17);
        PNL_FREE_MEM(stEvaTbl.pu16GChannel1_17);
        PNL_FREE_MEM(stEvaTbl.pu16BChannel1_17);
        PNL_FREE_MEM(TconDumpAll);
    }
    TCON_DEBUG("VAC setting done\n.");
    return bRetLut;
#else
    TCON_DEBUG("Not support VAC 256 setting\n.");
    return FALSE;
#endif
}
