// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon panel gamma driver
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
#include <linux/delay.h>

#include "coda/hwreg_common.h"
#include "coda/PAFRC_BKA324.h"
#include "coda/PAFRC_BKA324_V004.h"
#include "coda/SCTCON_MISC_BKA3E0_V005.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_tcon_dga.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_autodownload.h"

#define ENABLE_PNL_GAMMA_AUTODOWNLOAD   (TRUE)
#define PANEL_GAMMATBL_VERSION          (1)

#define SUPPORT_PANEL_GAMMA_W_CHANNEL   (FALSE)
#if SUPPORT_PANEL_GAMMA_W_CHANNEL
#define PANEL_GAMMA_1LUT_MAX_CHANNEL    (3) //RGBW
#else
#define PANEL_GAMMA_1LUT_MAX_CHANNEL    (4) //RGBW
#endif
#define PANEL_GAMMA_2LUT_MAX_CHANNEL    (6) //RGBRGB

// Deafine Panel gamma ADL entry size
#define PNLGAMMA_ADL_CMD_LENGTH         (32) //byte
#define PNLGAMMA_ADL_CMD_ENTRY          (512)
#define PNLGAMMA_ADL_CMD_192_ENTRY      (192) //RGB
#define PNLGAMMA_ADL_CMD_256_ENTRY      (256) //RGBW
#define PNLGAMMA_ADL_CMD_384_ENTRY      (384) //RGBRGB

#define PANEL_GAMMA_MAINHEADER_LEN      (32)
//Each channel of R/G/B/W has 64 entry separately, each entry contains 32bytes command
#define PANEL_GAMMA_CHANNEL_OFFSET      (64*32)
//Each table contains R/G/B/W 4 channels, accouts for 64*32*4 = 0x2000 bytes
#define PANEL_GAMMA_TBL_OFFSET          (0x2000)
//Panel gamma curve has 1024 entries
#define PANEL_GAMMA_ENTRY               (1024)

#define MAX_VERIFY_READ_NUM             (5)         //Maximun number of read verification using RIU

/// Define the panel gamma precision type
typedef enum
{
    E_PNL_GAMMA_10BIT = 0,              ///< Gamma Type of 10bit
    E_PNL_GAMMA_12BIT,                  ///< Gamma Type of 12bit
    E_PNL_GAMMA_ALL                     ///< The library can support all mapping mode
} EN_PNL_GAMMA_TYPE;

/// Define the panel gamma bin entry
typedef enum
{
    E_PNL_GAMMA_256_ENTRIES = 0,              ///< Indicate PNL Gamma is 256 entrise
    E_PNL_GAMMA_1024_ENTRIES,                 ///< Indicate PNL Gamma is 1024 entries
    E_PNL_GAMMA_MAX_ENTRIES                   ///< Indicate PNL Gamma is MAX entries
} EN_PNL_GAMMA_ENTRIES;

typedef enum
{
    E_PNL_GAMMA_TBL_FROM_BIN,
    E_PNL_GAMMA_TBL_FROM_REG,
    E_PNL_GAMMA_TBL_MAX,
}EN_PNL_GAMMA_TBL;

typedef struct __attribute__((packed))
{
    uint8_t u8GammaType;
    uint8_t u8Table2DNumber;
    uint8_t u8Table3DNumber;
    uint8_t u8TableFormat;
    uint16_t u16PreTableOrignalSize;
    uint16_t u16PreTableAlignedSize;
    uint32_t u32Table2DStartAddress;
    uint32_t u32Table3DStartAddress;
    uint8_t u8BinVersion_Minor2;
    uint8_t u8BinVersion_Minor1;
    uint8_t u8BinVersion_Major;
    uint8_t u8PanelSize;
    uint16_t u16Minute : 11;
    uint8_t u8Day : 5;
    uint8_t u8Month : 4;
    uint8_t u8Year : 5;
    uint8_t u8Reserved0 : 7;
    bool bGamma2LutEnable : 1;
    uint8_t u8Gamma2LutMode : 2;
    uint32_t u32Reserved1 : 29;
    uint32_t u32Reserved2;
}ST_PNL_GAMMA_CONFIG;

typedef struct  __attribute__((packed))
{
    uint32_t u32Version;                // Struct version
    uint32_t u32Length;                 // Sturct length
    EN_PNL_GAMMA_TBL  enPnlGammaTbl;    // IN: select Get gamma value from
    uint8_t*	pu8GammaTbl;            // IN: Bin file address
#if !defined (__aarch64__)
    void *pDummy0;                      // Dummy parameter
#endif
    uint32_t	u32TblSize;             // IN: PNL Gamma bin size
    uint16_t* pu16RChannel0;            // IN/OUT: R channel 0 data
#if !defined (__aarch64__)
    void *pDummy1;                      // Dummy parameter
#endif
    uint16_t* pu16GChannel0;            // IN/OUT: G channel 0 data
#if !defined (__aarch64__)
    void *pDumm2;                       // Dummy parameter
#endif
    uint16_t* pu16BChannel0;            // IN/OUT: B channel 0 data
#if !defined (__aarch64__)
    void *pDummy3;                      // Dummy parameter
#endif
    uint16_t* pu16WChannel0;            // IN/OUT: W channel 0 data
#if !defined (__aarch64__)
    void *pDummy4;                      // Dummy parameter
#endif
    uint16_t* pu16RChannel1;            // IN/OUT: R channel 1 data
#if !defined (__aarch64__)
    void *pDummy5;                      // Dummy parameter
#endif
    uint16_t* pu16GChannel1;            // IN/OUT: G channel 1 data
#if !defined (__aarch64__)
    void *pDummy6;                      // Dummy parameter
#endif
    uint16_t* pu16BChannel1;            // IN/OUT: B channel 1 data
#if !defined (__aarch64__)
    void *pDummy7;                      // Dummy parameter
#endif
    uint16_t* pu16WChannel1;            // IN/OUT: W channel 1 data
#if !defined (__aarch64__)
    void *pDummy8;                      // Dummy parameter
#endif
    uint16_t u16Index;                  // IN: the index of the data want to get
    uint16_t u16Size;                   // IN: every channel data number
    bool bSupportChannel1;              // OUT: Support PNL gamma Channel 1
    EN_PNL_GAMMA_ENTRIES enGammaEntries;// OUT: PNL gamma bin entries
    EN_PNL_GAMMA_TYPE enGammaType;      // OUT: PNL gamma bin type
    uint16_t u162DTableNum;             // OUT: PNL gamma bin 2D table number
    uint16_t u163DTableNum;             // OUT: PNL gamma bin 3D table number
}ST_PNL_PNLGAMMATBL;

//Panel Gamma
void _set_panelgamma_bypass(bool bEnable)
{
    W2BYTEMSK(REG_01B0_PAFRC_BKA324_V004,
                bEnable ? 0x1 : 0x0,REG_01B0_PAFRC_BKA324_V004_REG_GAMMA_BYPASS_01B0);
}

bool _get_panelgamma_bypass(void)
{
    return R2BYTEMSK(REG_01B0_PAFRC_BKA324_V004,REG_01B0_PAFRC_BKA324_V004_REG_GAMMA_BYPASS_01B0);
}

void _set_panelgamma_enable(bool bEnable)
{
    W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                bEnable ? 0x1 : 0x0,REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_EN_01B4);
}

bool _get_panelgamma_enable(void)
{
    return R2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_EN_01B4);
}

bool _set_to_lut_mode(struct udevice *dev, ST_PNL_GAMMA_CONFIG *pstPNLGammaConfig)
{
    bool bRet = TRUE;
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(priv);

    if(priv->pnl_lib_version != VERSION4)
    {
        W2BYTEMSK(REG_01A8_PAFRC_BKA324,
                (pstPNLGammaConfig->u8Gamma2LutMode & 0x03),
                REG_01A8_PAFRC_BKA324_REG_GAMMA_2LUT_MODE_SEL);

        W2BYTEMSK(REG_01A4_PAFRC_BKA324,
                (pstPNLGammaConfig->bGamma2LutEnable? 0x1:0x0),
                REG_01A4_PAFRC_BKA324_REG_GAMMA_2LUT_EN);
    }
    return bRet;
}

bool _get_to_lut_enable(struct udevice *dev)
{
    bool bRet = FALSE;
    uint16_t u16Value = 0;
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(priv);

    if (priv->pnl_lib_version != VERSION4)
    {
        u16Value = R2BYTEMSK(REG_01A4_PAFRC_BKA324, REG_01A4_PAFRC_BKA324_REG_GAMMA_2LUT_EN);
        bRet = (bool)u16Value;
    }

    return bRet;
}

uint16_t _read_channel_value(uint16_t u16Addr)
{
    uint8_t u8ResIdx = 0, u8Count = 0;
    uint16_t u16ChValue[MAX_VERIFY_READ_NUM] = {0};
    uint16_t u16Timeout = 0;

    for (uint8_t u8Idx = 0; u8Idx < MAX_VERIFY_READ_NUM; u8Idx++)
    {
        //toggle LUT_READ_EN (0 -> 1 -> 0)
        W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004, 1, REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_RD_EN_01B4);

        while (R2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                            REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_RD_EN_01B4) &&
                            (u16Timeout < MAX_DELAY_TIME))
        {
           mdelay(1);
           u16Timeout++;
        }
        u16Timeout = 0;

        u16ChValue[u8Idx] = R2BYTEMSK(REG_01C4_PAFRC_BKA324_V004,
                                        REG_01C4_PAFRC_BKA324_V004_REG_GAMMA_RD_DATA_01C4);

        //if (u16Addr < 30)
        //    TCON_DEBUG("[%d] u16ChValue[%d]=%d\n", u16Addr, u8Idx, u16ChValue[u8Idx]);
    }

    //most frequent element process (using moore's voting algoritm)
    u8ResIdx = 0;
    u8Count = 1;
    for (uint8_t u8Idx = 0; u8Idx < MAX_VERIFY_READ_NUM; u8Idx++)
    {
        if (u16ChValue[u8Idx] == u16ChValue[u8ResIdx])
            u8Count++;
        else
            u8Count--;

        if (u8Count == 0)
        {
            u8ResIdx = u8Idx;
            u8Count = 1;
        }
    }

    return u16ChValue[u8ResIdx];
}

bool _fill_channel0_from_reg(ST_PNL_PNLGAMMATBL *pstTbl, const uint16_t u16Addr,
                                        const uint8_t u8Ch, uint16_t u16Value)
{
    TCON_CHECK_PARAMETER_NULL(pstTbl);

    if (u8Ch == 0) //R channel
    {
        pstTbl->pu16RChannel0[u16Addr] = u16Value;
    }
    else if (u8Ch == 1) //G channel
    {
        pstTbl->pu16GChannel0[u16Addr] = u16Value;
    }
    else if (u8Ch == 2) //B channel
    {
        pstTbl->pu16BChannel0[u16Addr] = u16Value;
    }
    else if (u8Ch == 3) //W channel
    {
        pstTbl->pu16WChannel0[u16Addr] = u16Value;
    }
    return TRUE;
}

bool _fill_channel1_from_reg(ST_PNL_PNLGAMMATBL *pstTbl, const uint16_t u16Addr,
                                        const uint8_t u8Ch, uint16_t u16Value)
{
    TCON_CHECK_PARAMETER_NULL(pstTbl);

    if (u8Ch == 0) //R channel
    {
        pstTbl->pu16RChannel1[u16Addr] = u16Value;
    }
    else if (u8Ch == 1) //G channel
    {
        pstTbl->pu16GChannel1[u16Addr] = u16Value;
    }
    else if (u8Ch == 2) //B channel
    {
        pstTbl->pu16BChannel1[u16Addr] = u16Value;
    }
    else if (u8Ch == 3) //W channel
    {
        pstTbl->pu16WChannel1[u16Addr] = u16Value;
    }
    return TRUE;
}

bool _is_support_w_channel(void)
{
    return SUPPORT_PANEL_GAMMA_W_CHANNEL;
}

bool _get_tbl_from_reg(struct udevice *dev, ST_PNL_PNLGAMMATBL *pstGetPNLGammaTbl)
{
    uint16_t u16NumOfLevel = /*PANEL_GAMMA_ENTRY*/30;
    uint16_t u16Addr = 0;
    uint8_t u8Channel =0;
    bool bGammaByPass = _get_panelgamma_bypass();
    bool bSupportWChannel = _is_support_w_channel();
    uint16_t u16Timeout = 0;
    uint16_t u16Value = 0;
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(priv);

    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16RChannel0);
    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16GChannel0);
    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16BChannel0);
    if(bSupportWChannel)
    {
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16WChannel0);
    }

    if (bGammaByPass)
    {
        _set_panelgamma_bypass(FALSE);
    }

    //R/G/B/W
    for (u8Channel = 0; u8Channel < 4; u8Channel++)
    {
        //W channel
        if (u8Channel == 3 && !bSupportWChannel)
        {
            TCON_DEBUG("Not support W channel\n");
            break;
        }

        //1. set Read W/R channel (R:00 G:01 B:10)
        W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                    u8Channel,
                    REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_CH_SEL_01B4);

        for(u16Addr = 0; u16Addr < u16NumOfLevel; u16Addr++)
        {
            //2. set LUT_ADDR 0 ~ u16NumOfLevel
            W2BYTEMSK(REG_01B0_PAFRC_BKA324_V004,
                        u16Addr,
                        REG_01B0_PAFRC_BKA324_V004_REG_GAMMA_ADR_01B0);

            u16Value = _read_channel_value(u16Addr);

            _fill_channel0_from_reg(pstGetPNLGammaTbl, u16Addr, u8Channel, u16Value);
        }
    }

    if (priv->pnl_lib_version != VERSION4)
    {
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16RChannel1);
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16GChannel1);
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16BChannel1);
        if (bSupportWChannel)
        {
            TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16WChannel1);
        }

        // Set Channel1
        // Enable REG_GAMMA_2LUT_EN
        W2BYTEMSK(REG_01A4_PAFRC_BKA324, 1, REG_01A4_PAFRC_BKA324_REG_GAMMA_2LUT_EN);
        // Set REG_GAMMA_LUT_SEL 1
        W2BYTEMSK(REG_01A4_PAFRC_BKA324, 1, REG_01A4_PAFRC_BKA324_REG_GAMMA_LUT_SEL);

        for (u8Channel = 0; u8Channel < 4; u8Channel++)
        {
            //W channel
            if (u8Channel == 3 && !bSupportWChannel)
            {
                TCON_DEBUG("Not support W channel\n");
                break;
            }

            //1. set Read channel
            W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                        u8Channel,
                        REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_CH_SEL_01B4);

            for (u16Addr = 0; u16Addr < u16NumOfLevel; u16Addr++)
            {
                //2. set LUT_ADDR 0
                W2BYTEMSK(REG_01B0_PAFRC_BKA324_V004,
                            u16Addr,
                            REG_01B0_PAFRC_BKA324_V004_REG_GAMMA_ADR_01B0);

                //3. toggle LUT_R_FLAG
                W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                            1,
                            REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_RD_EN_01B4);

                u16Timeout = 0;
                while(R2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                        REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_RD_EN_01B4) &&
                        (u16Timeout < MAX_DELAY_TIME))
                {
                    udelay(10);
                    u16Timeout++;
                }

                u16Value = R2BYTEMSK(REG_01C4_PAFRC_BKA324_V004,
                                        REG_01C4_PAFRC_BKA324_V004_REG_GAMMA_RD_DATA_01C4);

                _fill_channel1_from_reg(pstGetPNLGammaTbl, u16Addr, u8Channel, u16Value);
            }

        }

        // Disable REG_GAMMA_2LUT_EN
        W2BYTEMSK(REG_01A4_PAFRC_BKA324, 0, REG_01A4_PAFRC_BKA324_REG_GAMMA_2LUT_EN);
        // Set REG_GAMMA_LUT_SEL 0
        W2BYTEMSK(REG_01A4_PAFRC_BKA324, 0, REG_01A4_PAFRC_BKA324_REG_GAMMA_LUT_SEL);
    }

    if (bGammaByPass == TRUE)
    {
        //restore setting
        _set_panelgamma_bypass(bGammaByPass);
    }
    return TRUE;
}

bool _get_panelgamma_tbl_from_bin(ST_PNL_PNLGAMMATBL *pstGetPNLGammaTbl)
{
    uint32_t u32index = 0, u32StartAddress = 0;
    ST_PNL_GAMMA_CONFIG stPNLGammaConfig;
    uint8_t *u8ptr = NULL;
    bool bSupportWChannel = _is_support_w_channel();
    memset(&stPNLGammaConfig, 0, sizeof(ST_PNL_GAMMA_CONFIG));
    memcpy(&stPNLGammaConfig,
            (pstGetPNLGammaTbl->pu8GammaTbl + PANEL_GAMMA_MAINHEADER_LEN),
            sizeof(ST_PNL_GAMMA_CONFIG));

    u32StartAddress = stPNLGammaConfig.u32Table2DStartAddress +
                        stPNLGammaConfig.u16PreTableAlignedSize * pstGetPNLGammaTbl->u16Index;

    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu8GammaTbl);
    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16RChannel0);
    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16GChannel0);
    TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16BChannel0);

    if (bSupportWChannel)
    {
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16WChannel0);
    }

    if (stPNLGammaConfig.bGamma2LutEnable)
    {
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16RChannel1);
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16GChannel1);
        TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16BChannel1);

        if (bSupportWChannel)
        {
            TCON_CHECK_PARAMETER_NULL(pstGetPNLGammaTbl->pu16WChannel1);
        }
    }

    u8ptr = u32StartAddress + pstGetPNLGammaTbl->pu8GammaTbl;

    //[7:4]: 0x01 10BITS GAMMA
    //       0x02 12BITS GAMMA
    if ((stPNLGammaConfig.u8GammaType & _BIT(4)))
    {
        pstGetPNLGammaTbl->enGammaType = E_PNL_GAMMA_10BIT;
    }
    else if ((stPNLGammaConfig.u8GammaType & _BIT(5)))
    {
        pstGetPNLGammaTbl->enGammaType = E_PNL_GAMMA_12BIT;
    }

    //[3:0]: 0x01 256 entries
    //       0x02 1024 entries
    if ((stPNLGammaConfig.u8GammaType & _BIT(0)))
    {
        pstGetPNLGammaTbl->enGammaEntries = E_PNL_GAMMA_256_ENTRIES;
    }
    else
    {
        pstGetPNLGammaTbl->enGammaEntries = E_PNL_GAMMA_1024_ENTRIES;
    }
    /* data format
    //each cmd:
    //|  d_15  |   d_14  |   d_13  |  d_12   |   d_11  |   d_10  |   d_9   |   d_8   |   d_7   |...
    //|251:240 | 235:224 | 219:208 | 203:192 | 187:176 | 171:160 | 155:144 | 139:128 | 123:112 |...
    //...|   d_6  |  d_5  |  d_4  |  d_3  |  d_2  |  d_1  | d_0  |
    //...| 107:96 | 91:80 | 75:64 | 59:48 | 43:32 | 27:16 | 11:0 |
    //
    //R/G/B/W cmd order in DRAM
    //            |END | START |
    //  R_LUT0     63      0
    //  G_LUT0     127     64
    //  B_LUT0     191     128
    //  W_LUT0     255     192
    //  R_LUT1     319     256
    //  G_LUT1     383     320
    //  B_LUT1     447     384
    //  W_LUT1     511     448
    */

    uint8_t* pu8ptr_R0 = u8ptr;
    uint8_t* pu8ptr_G0 = pu8ptr_R0 + PANEL_GAMMA_CHANNEL_OFFSET;  // 64entry  * 32 byte =  2048
    uint8_t* pu8ptr_B0 = pu8ptr_G0 + PANEL_GAMMA_CHANNEL_OFFSET;
    uint8_t* pu8ptr_W0 = pu8ptr_B0 + PANEL_GAMMA_CHANNEL_OFFSET;

    uint8_t* pu8ptr_R1 = pu8ptr_R0 + PANEL_GAMMA_TBL_OFFSET;
    uint8_t* pu8ptr_G1 = pu8ptr_R1 + PANEL_GAMMA_CHANNEL_OFFSET;
    uint8_t* pu8ptr_B1 = pu8ptr_G1 + PANEL_GAMMA_CHANNEL_OFFSET;
    uint8_t* pu8ptr_W1 = pu8ptr_B1 + PANEL_GAMMA_CHANNEL_OFFSET;

    for (u32index = 0; u32index < PANEL_GAMMA_ENTRY; u32index++)
    {
        //LUT 0
        pstGetPNLGammaTbl->pu16RChannel0[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_R0 + u32index));
        pstGetPNLGammaTbl->pu16GChannel0[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_G0 + u32index));
        pstGetPNLGammaTbl->pu16BChannel0[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_B0 + u32index));
        if (bSupportWChannel)
        {
            pstGetPNLGammaTbl->pu16WChannel0[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_W0 + u32index));
        }

        if (stPNLGammaConfig.bGamma2LutEnable)
        {
            //LUT 1
            pstGetPNLGammaTbl->pu16RChannel1[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_R1 + u32index));
            pstGetPNLGammaTbl->pu16GChannel1[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_G1 + u32index));
            pstGetPNLGammaTbl->pu16BChannel1[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_B1 + u32index));
            if (bSupportWChannel)
            {
                pstGetPNLGammaTbl->pu16WChannel1[u32index] =
                                                ((uint16_t)*((uint16_t *)pu8ptr_W1 + u32index));
            }
        }
    }
    return TRUE;
}

bool _use_adl_set_panelgamma_tbl(struct udevice *dev, ST_PNL_PNLGAMMATBL *pstTbl)
{
#if (defined(UFO_XC_AUTO_DOWNLOAD) && ENABLE_PNL_GAMMA_AUTODOWNLOAD == TRUE)
    uint8_t *pu8GammaTbl = NULL;
    bool bRet = TRUE;
    uint32_t u32index = 0, u32Size = 0;
    bool b2LutEn = _get_to_lut_enable(dev);
    uint32_t u32PnlGammaADLCmdSize;
    uint8_t *pu8R0 = NULL, *pu8G0 = NULL, *pu8B0 = NULL, *pu8W0 = NULL;
    uint8_t *pu8R1 = NULL, *pu8G1 = NULL, *pu8B1 = NULL, *pu8W1 = NULL;
    bool bGammaByPass = _get_panelgamma_bypass();
    bool bSupportWChannel = _is_support_w_channel();
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_DEBUG("LutEn=%d SupportWChannel=%d\n", b2LutEn, bSupportWChannel);

    if (!b2LutEn)
    {
        if (bSupportWChannel)
        {
            u32PnlGammaADLCmdSize = PNLGAMMA_ADL_CMD_256_ENTRY;
        }
        else
        {
            u32PnlGammaADLCmdSize = PNLGAMMA_ADL_CMD_192_ENTRY;
        }
    }
    else
    {
        u32PnlGammaADLCmdSize = PNLGAMMA_ADL_CMD_384_ENTRY;
    }

    u32Size = PNLGAMMA_ADL_CMD_LENGTH*u32PnlGammaADLCmdSize;

    PNL_MALLOC_MEM(pu8GammaTbl, u32Size, bRet);

    if (bRet)
    {
        //LUT 0
        pu8R0 = pu8GammaTbl;
        pu8G0 = pu8R0 + PANEL_GAMMA_CHANNEL_OFFSET;  // 64entry  * 32 byte =  2048
        pu8B0 = pu8G0 + PANEL_GAMMA_CHANNEL_OFFSET;

        if (bSupportWChannel)
        {
            pu8W0 = pu8B0 + PANEL_GAMMA_CHANNEL_OFFSET;
        }

        if (b2LutEn)
        {
            //LUT 1
            pu8R1 = pu8B0 + PANEL_GAMMA_CHANNEL_OFFSET;
            pu8G1 = pu8R1 + PANEL_GAMMA_CHANNEL_OFFSET;
            pu8B1 = pu8G1 + PANEL_GAMMA_CHANNEL_OFFSET;

            if (bSupportWChannel)
            {
                pu8W1 = pu8B1 + PANEL_GAMMA_CHANNEL_OFFSET;
            }
        }

        for (u32index = 0; u32index < PANEL_GAMMA_ENTRY; u32index++)
        {
            //LUT 0
            *((uint16_t *)pu8R0 + u32index) = pstTbl->pu16RChannel0[u32index];
            *((uint16_t *)pu8G0 + u32index) = pstTbl->pu16GChannel0[u32index];
            *((uint16_t *)pu8B0 + u32index) = pstTbl->pu16BChannel0[u32index];

            if (bSupportWChannel)
            {
                *((uint16_t *)pu8W0 + u32index) = pstTbl->pu16WChannel0[u32index];
            }

            if (b2LutEn)
            {
                //LUT 1
                *((uint16_t *)pu8R1 + u32index) = pstTbl->pu16RChannel1[u32index];
                *((uint16_t *)pu8G1 + u32index) = pstTbl->pu16GChannel1[u32index];
                *((uint16_t *)pu8B1 + u32index) = pstTbl->pu16BChannel1[u32index];

                if (bSupportWChannel)
                {
                    *((uint16_t *)pu8W1 + u32index) = pstTbl->pu16WChannel1[u32index];
                }
            }
        }

        if (bGammaByPass)
        {
            _set_panelgamma_bypass(FALSE);
        }

	if ((priv->pnl_lib_version == VERSION5) || (priv->pnl_lib_version == VERSION6))
	{
		//adl index user mode en have to be set 1 when boot time
		W2BYTEMSK(REG_0080_SCTCON_MISC_BKA3E0_V005, 0x1, REG_0080_SCTCON_MISC_BKA3E0_V005_REG_ADL_IDX_PGA_SEL_0080);
	}
        // Write ADL
        if (KHal_XC_WriteAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA,
                                        pu8GammaTbl, u32Size, NULL) != TRUE)
        {
            TCON_ERROR(" Write ADL fail \n.");
            PNL_FREE_MEM(pu8GammaTbl);
            return FALSE;
        }

        //Fire ADL
        if (KHal_XC_FireAutoDownload(E_PNL_XC_AUTODOWNLOAD_CLIENT_PANEL_GAMMA) != TRUE)
        {
            TCON_ERROR(" Fire ADL fail \n.");
            PNL_FREE_MEM(pu8GammaTbl);
            return FALSE;
        }
        PNL_FREE_MEM(pu8GammaTbl);
    }
    else
    {
        TCON_ERROR("Allocate buffer return fail\n.");
        return FALSE;
    }
    return TRUE;
#else
    TCON_DEBUG("Auto download config disable.\n");
    return FALSE;
#endif
}

bool _write_1lut_to_reg(ST_PNL_PNLGAMMATBL *pstTbl, const uint16_t u16DataIdx,
                                        const uint8_t u8Ch)
{
    bool bSupportWChannel = _is_support_w_channel();
    uint32_t u32Addr = REG_01B8_PAFRC_BKA324_V004;
    uint32_t u32Fld = REG_01B8_PAFRC_BKA324_V004_REG_GAMMA_DATA_01B8;

    TCON_CHECK_PARAMETER_NULL(pstTbl);

    if(u8Ch == 0) //R channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16RChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 1) //G channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16GChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 2) //B channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16BChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 3) //W channel
    {
        if (bSupportWChannel == TRUE)
        {
            W2BYTEMSK(u32Addr, pstTbl->pu16WChannel0[u16DataIdx], u32Fld);
        }
    }

    return TRUE;
}

bool _write_2lut_to_reg(ST_PNL_PNLGAMMATBL *pstTbl, const uint16_t u16DataIdx,
                                        const uint8_t u8Ch)
{
    uint32_t u32Addr = REG_01B8_PAFRC_BKA324_V004;
    uint32_t u32Fld = REG_01B8_PAFRC_BKA324_V004_REG_GAMMA_DATA_01B8;

    TCON_CHECK_PARAMETER_NULL(pstTbl);

    if(u8Ch == 0) //R channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16RChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 1) //G channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16GChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 2) //B channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16BChannel0[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 3) //R1 channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16RChannel1[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 4) //G1 channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16GChannel1[u16DataIdx], u32Fld);
    }
    else if (u8Ch == 5) //B1 channel
    {
        W2BYTEMSK(u32Addr, pstTbl->pu16BChannel1[u16DataIdx], u32Fld);
    }

    return TRUE;
}

bool _set_panelgamma_tbl(struct udevice *dev, ST_PNL_PNLGAMMATBL *pstTbl)
{
    uint16_t u16NumOfLevel = PANEL_GAMMA_ENTRY;
    uint16_t u16DataIdx = 0;
    uint8_t u8Channel =0;
    uint16_t u16Timeout = 0;
    bool bGammaEnable = 0;
    bool b2LutEn = _get_to_lut_enable(dev);
    // 1: RGBRGB, 0: RGBW
    uint8_t u8MaxChannel = b2LutEn ? PANEL_GAMMA_2LUT_MAX_CHANNEL : PANEL_GAMMA_1LUT_MAX_CHANNEL;
    bool bGammaByPass = _get_panelgamma_bypass();
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    TCON_CHECK_PARAMETER_NULL(priv);

    if (_use_adl_set_panelgamma_tbl(dev, pstTbl))
    {
        //use ADL to set panel gamma table, use ML to set panel gamma enable
        //but ML trigger early ADL so that it shows garbage in AC on
        //so delay 1 vsync for ML
        mdelay(20);
        TCON_DEBUG("Use auto download to set panel gamma successfully.\n");
        return TRUE;
    }
    else
    {
        TCON_DEBUG("Non-use auto download to set panel gamma.\n");
    }

    if (bGammaByPass)
    {
        _set_panelgamma_bypass(FALSE);
    }

    // Store Panel Gamma enable flag
    bGammaEnable = _get_panelgamma_enable();
    //Disbale Panel Gamma
    if (bGammaEnable)
    {
        _set_panelgamma_enable(FALSE);
    }

    //Due to odclk and mcu clock not the same, the set value not stable.
    //Need add 4 NOP for burst wirte to guarantee that set value correct.
    //So use normal write to save time.

    for (u8Channel = 0; u8Channel < u8MaxChannel; u8Channel++)
    {
        W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                    0x1,
                    REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_BW_EN_01B4); //BW enable
        if (b2LutEn == FALSE)
        {
            // set Write channel
            W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                        u8Channel,
                        REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_CH_SEL_01B4);
            if (priv->pnl_lib_version != VERSION4)
            {
                //reg_gamma_lut_sel, select lut0 load table
                W2BYTEMSK(REG_01A4_PAFRC_BKA324,
                            0x0,
                            REG_01A4_PAFRC_BKA324_REG_GAMMA_LUT_SEL);
            }
        }
        else
        {
            // set Write channel
            W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                        (u8Channel%3),
                        REG_01B4_PAFRC_BKA324_REG_GAMMA_CH_SEL);
            if (priv->pnl_lib_version != VERSION4)
            {
                //reg_gamma_lut_sel, select lut0 or lut1 to load table
                W2BYTEMSK(REG_01A4_PAFRC_BKA324,
                            (u8Channel >= 3) ? 0x1 : 0x0,REG_01A4_PAFRC_BKA324_REG_GAMMA_LUT_SEL);
            }
        }

        W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                    0x1,
                    REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_BW_EN_01B4); //BW enable

        for (u16DataIdx = 0; u16DataIdx < u16NumOfLevel; u16DataIdx++)
        {
            //2. set LUT_ADDR
            W2BYTEMSK(REG_01B0_PAFRC_BKA324_V004,
                        u16DataIdx,
                        REG_01B0_PAFRC_BKA324_V004_REG_GAMMA_ADR_01B0);
            //3. set write data
            if (b2LutEn == FALSE)
            {
                _write_1lut_to_reg(pstTbl, u16DataIdx, u8Channel);
            }
            else
            {
                //lut0 & lut1
                _write_2lut_to_reg(pstTbl, u16DataIdx, u8Channel);
            }

            //4. toggle LUT_W_FLAG_R
            W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                        0x1,
                        REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_WR_EN_01B4);
            // after setting toggle bit, we need to make sure the bit is already toggled.
            while(R2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                            REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_WR_EN_01B4) &&
                            (u16Timeout < MAX_DELAY_TIME))
            {
                udelay(10);
                u16Timeout++;
            }
        }

        //Afetr writing one channel, disable BW.
        W2BYTEMSK(REG_01B4_PAFRC_BKA324_V004,
                    0x0,
                    REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_BW_EN_01B4);
    }

    // restore setting
    if (bGammaEnable)
    {
        _set_panelgamma_enable(TRUE);
    }
    return TRUE;
}

bool _mem_free(ST_PNL_PNLGAMMATBL *stPNLGammaTbl)
{
    PNL_FREE_MEM(stPNLGammaTbl->pu16RChannel0);
    PNL_FREE_MEM(stPNLGammaTbl->pu16GChannel0);
    PNL_FREE_MEM(stPNLGammaTbl->pu16BChannel0);
    PNL_FREE_MEM(stPNLGammaTbl->pu16WChannel0);

    PNL_FREE_MEM(stPNLGammaTbl->pu16RChannel1);
    PNL_FREE_MEM(stPNLGammaTbl->pu16GChannel1);
    PNL_FREE_MEM(stPNLGammaTbl->pu16BChannel1);
    PNL_FREE_MEM(stPNLGammaTbl->pu16WChannel1);

    return TRUE;
}

bool _mem_allocate(ST_PNL_PNLGAMMATBL *stPNLGammaTbl, bool bGamma2LutEnable)
{
    bool bRet = TRUE;
    bool bSupportWChannel = _is_support_w_channel();
    const uint16_t u16ChannelLen = PANEL_GAMMA_ENTRY*sizeof(uint16_t);

    if (!bGamma2LutEnable)
    {
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16RChannel0, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16GChannel0, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16BChannel0, u16ChannelLen, bRet);
        if (bSupportWChannel)
        {
            PNL_MALLOC_MEM(stPNLGammaTbl->pu16WChannel0, u16ChannelLen, bRet);
        }
    }
    else
    {
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16RChannel0, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16GChannel0, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16BChannel0, u16ChannelLen, bRet);

        PNL_MALLOC_MEM(stPNLGammaTbl->pu16RChannel1, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16GChannel1, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16BChannel1, u16ChannelLen, bRet);

        PNL_MALLOC_MEM(stPNLGammaTbl->pu16WChannel0, u16ChannelLen, bRet);
        PNL_MALLOC_MEM(stPNLGammaTbl->pu16WChannel1, u16ChannelLen, bRet);
    }

    if (!bRet)
    {
        _mem_free(stPNLGammaTbl);
    }

    return bRet;
}

//--------------------------------------------------------------------------------------------------------------------
// Panel Gamma Auto Download Format
// Lut0                         2Lut case
// R/G/B/W cmd format in DRAM   R/G/B cmd format in DRAM
//            |END | START |              |END | START |
//  R_LUT0     63      0        R_LUT0     63      0
//  G_LUT0     127     64       G_LUT0     127     64
//  B_LUT0     191     128      B_LUT0     191     128
//  W_LUT0     255     192      R_LUT1     255     192
//                              G_LUT1     319     256
//                              B_LUT1     383     320
// Panel Gamma Bin File Format
// Lut0                         2Lut case
//  R_LUT0     63      0        R_LUT0     63      0
//  G_LUT0     127     64       G_LUT0     127     64
//  B_LUT0     191     128      B_LUT0     191     128
//  W_LUT0     255     192      W_LUT0     255     192
//                              R_LUT1     319     256
//                              G_LUT1     383     320
//                              B_LUT1     447     384
//                              W_LUT1     511     448
//--------------------------------------------------------------------------------------------------------------------
bool _set_panelgamma_proc(struct udevice *dev, uint8_t *pu8GammaTbl,
                                    uint32_t u32Size, uint8_t u8Index)
{
    bool bRet = TRUE;
    ST_PNL_PNLGAMMATBL stPnlGammaTbl;
    ST_PNL_GAMMA_CONFIG stPnlGammaConfig;

    TCON_CHECK_PARAMETER_NULL(pu8GammaTbl);

    if (PANEL_GAMMA_MAINHEADER_LEN > u32Size)
    {
        TCON_ERROR("Table Size(%u) is too small.\n", u32Size); \
        return FALSE;
    }

    //get panel gamma sub header
    memset(&stPnlGammaConfig, 0, sizeof(ST_PNL_GAMMA_CONFIG));
    memcpy(&stPnlGammaConfig,
            (pu8GammaTbl + PANEL_GAMMA_MAINHEADER_LEN),
            sizeof(ST_PNL_GAMMA_CONFIG));

    //get panel gamma RGBW curve table from BIN
    memset(&stPnlGammaTbl, 0, sizeof(ST_PNL_PNLGAMMATBL));
    stPnlGammaTbl.pu8GammaTbl = pu8GammaTbl;
    stPnlGammaTbl.enPnlGammaTbl = E_PNL_GAMMA_TBL_FROM_BIN;
    stPnlGammaTbl.u16Index = u8Index;

    //allocate memory
    bRet &= _mem_allocate(&stPnlGammaTbl, stPnlGammaConfig.bGamma2LutEnable);
    if (!bRet)
    {
        TCON_DEBUG("\033[1;32m _mem_allocate retuen fail\033[0m \n");
        return bRet;
    }

    bRet &= _get_panelgamma_tbl_from_bin(&stPnlGammaTbl);
    if (bRet)
    {
        //Set VAC 2Lut mode
        _set_to_lut_mode(dev, &stPnlGammaConfig);
        _set_panelgamma_tbl(dev, &stPnlGammaTbl);

        //Enable Panel gamma
        _set_panelgamma_enable(TRUE);
    }
    else
    {
        TCON_ERROR("Get panel gamma table from bin return failed.\n");
    }

    //free memory
    _mem_free(&stPnlGammaTbl);

    return bRet;
}

bool _get_panelgamma_tbl(struct udevice *dev, ST_PNL_PNLGAMMATBL *pstTbl)
{
    bool bRet = FALSE;
    TCON_CHECK_PARAMETER_NULL(pstTbl);

    switch (pstTbl->enPnlGammaTbl)
    {
        case E_PNL_GAMMA_TBL_FROM_BIN:
        {
            bRet = _get_panelgamma_tbl_from_bin(pstTbl);
            break;
        }
        case E_PNL_GAMMA_TBL_FROM_REG:
        {
            for (uint8_t u8Idx = 0; u8Idx < 2; u8Idx++)
            {
                bRet = _get_tbl_from_reg(dev, pstTbl);
            }
            break;
        }
        default:
        {
            TCON_ERROR("SET WRONG type for read Panel gamma!!!!!\n");
            break;
        }

    }
    return bRet;
}

bool _verify_panelgamma(struct udevice *dev, unsigned char *pdata_buf, loff_t data_len)
{
    bool bRet = TRUE;
    const uint16_t u16ChannelLen = PANEL_GAMMA_ENTRY;

    TCON_CHECK_PARAMETER_NULL(dev);

    // Read the panel gamma value from Bin
    ST_PNL_PNLGAMMATBL pstBin;
    memset(&pstBin, 0, sizeof(ST_PNL_PNLGAMMATBL));
    pstBin.enPnlGammaTbl = E_PNL_GAMMA_TBL_FROM_BIN;
    pstBin.pu8GammaTbl = pdata_buf;
    pstBin.u32TblSize = (uint32_t)data_len;//64*32*8;
    pstBin.u16Index = 0;
    pstBin.u16Size = u16ChannelLen;
    pstBin.u32Version = PANEL_GAMMATBL_VERSION;
    pstBin.u32Length = sizeof(ST_PNL_PNLGAMMATBL);

    PNL_MALLOC_MEM(pstBin.pu16RChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16GChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16BChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16WChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);

    PNL_MALLOC_MEM(pstBin.pu16RChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16GChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16BChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstBin.pu16WChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);

    if (!bRet)
    {
        TCON_ERROR("Allcate buffer fail!!\n");
        goto finally;
    }

    bRet &= _get_panelgamma_tbl(dev, &pstBin);

    // read the panel gamma value from reg.
    ST_PNL_PNLGAMMATBL pstReg;
    memset(&pstReg, 0, sizeof(ST_PNL_PNLGAMMATBL));
    pstReg.enPnlGammaTbl = E_PNL_GAMMA_TBL_FROM_REG;
    pstReg.pu8GammaTbl = pdata_buf;
    pstReg.u32TblSize = (uint32_t)data_len;//64*32*8;
    pstReg.u16Index = 0;
    pstReg.u16Size = u16ChannelLen;
    pstReg.u32Version = PANEL_GAMMATBL_VERSION;
    pstReg.u32Length = sizeof(ST_PNL_PNLGAMMATBL);

    PNL_MALLOC_MEM(pstReg.pu16RChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16GChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16BChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16WChannel0, sizeof(uint16_t)*u16ChannelLen, bRet);

    PNL_MALLOC_MEM(pstReg.pu16RChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16GChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16BChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);
    PNL_MALLOC_MEM(pstReg.pu16WChannel1, sizeof(uint16_t)*u16ChannelLen, bRet);

    if (!bRet)
    {
        TCON_ERROR("Allcate buffer fail!!\n");
        goto finally;
    }

    bRet &= _get_panelgamma_tbl(dev, &pstReg);

    // compare the data
    for (uint32_t u32Idx = 0; u32Idx < u16ChannelLen; u32Idx++)
    {
        if(*(pstReg.pu16RChannel0 + u32Idx) != *(pstBin.pu16RChannel0 + u32Idx))
        {
            TCON_DEBUG("\033[1;32m not same idx=%u, bin:%x, reg:%x\033[0m\n", \
                        u32Idx, *(pstBin.pu16RChannel0 + u32Idx),*(pstReg.pu16RChannel0 + u32Idx));
            bRet = FALSE;
            break;
        }
        if(*(pstReg.pu16GChannel0 + u32Idx) != *(pstBin.pu16GChannel0 + u32Idx))
        {
            TCON_DEBUG("\033[1;32m not same idx=%u, bin:%x, reg:%x\033[0m\n", \
                        u32Idx, *(pstBin.pu16GChannel0 + u32Idx),*(pstReg.pu16GChannel0 + u32Idx));
            bRet = FALSE;
            break;
        }
        if(*(pstReg.pu16BChannel0 + u32Idx) != *(pstBin.pu16BChannel0 + u32Idx))
        {
            TCON_DEBUG("\033[1;32m not same idx=%u, bin:%x, reg:%x\033[0m\n", \
                        u32Idx, *(pstBin.pu16BChannel0 + u32Idx),*(pstReg.pu16BChannel0 + u32Idx));
            bRet = FALSE;
            break;
        }

    }

finally:
    PNL_FREE_MEM(pstBin.pu16RChannel0);
    PNL_FREE_MEM(pstBin.pu16GChannel0);
    PNL_FREE_MEM(pstBin.pu16BChannel0);
    PNL_FREE_MEM(pstBin.pu16WChannel0);

    PNL_FREE_MEM(pstBin.pu16RChannel1);
    PNL_FREE_MEM(pstBin.pu16GChannel1);
    PNL_FREE_MEM(pstBin.pu16BChannel1);
    PNL_FREE_MEM(pstBin.pu16WChannel1);

    PNL_FREE_MEM(pstReg.pu16RChannel0);
    PNL_FREE_MEM(pstReg.pu16GChannel0);
    PNL_FREE_MEM(pstReg.pu16BChannel0);
    PNL_FREE_MEM(pstReg.pu16WChannel0);

    PNL_FREE_MEM(pstReg.pu16RChannel1);
    PNL_FREE_MEM(pstReg.pu16GChannel1);
    PNL_FREE_MEM(pstReg.pu16BChannel1);
    PNL_FREE_MEM(pstReg.pu16WChannel1);

    TCON_DEBUG("\033[1;32m Verify [%s]\033[0m\n", bRet ? "OK" : "NG");

    return bRet;
}

bool mtk_tcon_panelgamma_setting(struct udevice *dev)
{
    bool bRet = TRUE;
    bool data_exist = FALSE;
    loff_t data_len = 0;
    unsigned char *pdata_buf = NULL;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(dev);

    data_exist = is_pnlgamma_data_exist(&pdata_buf, &data_len);

    if (data_exist && data_len > 1)
    {
        TCON_DEBUG("Data len=%lld [0]=0x%X [1]=0x%X\n", data_len, pdata_buf[0], pdata_buf[1]);

        //initial auto download
        bRet &= mtk_pnl_autodownload_init(dev);
        if (!bRet)
        {
            TCON_ERROR("autodownload init failed\n");
        }

        //set panel gamma process
        bRet &= _set_panelgamma_proc(dev, pdata_buf, (uint32_t)data_len, 0);
        if (!bRet)
        {
            TCON_ERROR("_set_panelgamma_proc fail\n");
        }

        //verity panel gamma value
        //_verify_panelgamma(dev, pdata_buf, (uint32_t)data_len);
    }
    else
    {
        TCON_DEBUG("Not support panel gamma function. Data len=%lld\n", data_len);
    }

    TCON_FUNC_EXIT(bRet);
    return bRet;
}

