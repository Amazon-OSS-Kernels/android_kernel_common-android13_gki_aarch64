// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek tcon output driver
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
#include <time.h>

#include "coda/hwreg_common.h"
#include "coda/CKGEN00_V004.h"
#include "coda/CKGEN01_V004.h"
#include "coda/SCDISP_BKA4FA.h"
#include "coda/PAFRC_BKA324_V004.h"
#include "coda/DEMURA_BKA377_V004.h"
#include "coda/SCTCON_BKA4F7_V004.h"
#include "coda/CKGEN01_V005.h"
#include "coda/TCON_4K.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_tcon_common.h"
#include "mtk_tcon_out_if.h"
#include "mtk_tcon_dga.h"
#include "mtk_tcon_od.h"
#include "mtk_tcon_lineod.h"
#include "mtk_tcon_vac.h"
#include "mtk_pnl_autodownload.h"

#define READ_1BYTE_REG(u32Addr) \
        do { \
            TCON_DEBUG("[Read] addr=%06x, val=%02x\n", \
                        u32Addr, \
                        RIU_READ_BYTE((((u32Addr) <<1) - ((u32Addr) & 1))));\
        } while (0)

unsigned long lTconEanbleTiming = 0;

//for panel tab
typedef enum
{
    E_PNL_TCON_PANELINFO_PANELLINKTYPE = 0,
    E_PNL_TCON_PANELINFO_PANELLINKEXTTYPE,
    E_PNL_TCON_PANELINFO_FIXEDVABCKPORCH,
    E_PNL_TCON_PANELINFO_FIXEDHBACKPORCH,

    E_PNL_TCON_PANELINFO_VSYNC_START = 11,
    E_PNL_TCON_PANELINFO_VSYNC_END,
    E_PNL_TCON_PANELINFO_VDESTART,
    E_PNL_TCON_PANELINFO_VDEND,
    E_PNL_TCON_PANELINFO_PANELMAXVTOTAL,
    E_PNL_TCON_PANELINFO_PANELVTOTAL,
    E_PNL_TCON_PANELINFO_MINVTOTAL,
    E_PNL_TCON_PANELINFO_VDEHEADDUMMY,
    E_PNL_TCON_PANELINFO_VDETAILDUMMY,

    E_PNL_TCON_PANELINFO_HSYNC_START = 31,
    E_PNL_TCON_PANELINFO_HSYNC_END,
    E_PNL_TCON_PANELINFO_HDESTART,
    E_PNL_TCON_PANELINFO_HDEEND,
    E_PNL_TCON_PANELINFO_PANELMAXHTOTAL,
    E_PNL_TCON_PANELINFO_PANELHTOTAL,
    E_PNL_TCON_PANELINFO_PANELMINHTOTAL,
    E_PNL_TCON_PANELINFO_HDEHEADDUMMY,
    E_PNL_TCON_PANELINFO_HDETAILDUMMY,

    E_PNL_TCON_PANELINFO_PANELMAXDCLK = 51,
    E_PNL_TCON_PANELINFO_PANELDCLK,
    E_PNL_TCON_PANELINFO_PANELMINDCLK,
    E_PNL_TCON_PANELINFO_PANELMAXSET,
    E_PNL_TCON_PANELINFO_MINSET,
    E_PNL_TCON_PANELINFO_OUTTIMINGMODE,
    E_PNL_TCON_PANELINFO_TOTALPAIR,
    E_PNL_TCON_PANELINFO_PAIREVEN,
    E_PNL_TCON_PANELINFO_PAIRODD,
    E_PNL_TCON_PANELINFO_DUALPORT,

    E_PNL_TCON_PANELINFO_PANELHSYNCWIDTH = 71,
    E_PNL_TCON_PANELINFO_PANELHSYNCBACKPORCH,
    E_PNL_TCON_PANELINFO_PANELVSYNCWIDTH,
    E_PNL_TCON_PANELINFO_PANELVBACKPORCH,

    E_PNL_TCON_PANELINFO_SSC_ENABLE = 80,
    E_PNL_TCON_PANELINFO_SSC_FMODULATION,
    E_PNL_TCON_PANELINFO_SSC_PERCENTAGE,
    E_PNL_TCON_PANELINFO_SSC_BIN_CTRL,
    E_PNL_TCON_PANELINFO_DEMURA_SEL,
    E_PNL_TCON_PANELINFO_DATA_PATH_SEL,
    E_PNL_TCON_PANELINFO_VCOM_SEL,

    E_PNL_TCON_PANELINFO_DEMURA_ENABLE = 91,
	E_PNL_TCON_PANELINFO_OD_ENABLE,
	E_PNL_TCON_PANELINFO_OD_MODE_TYPE,
} E_PNL_TCON_PANELINFO;

#define TCON_PATH_REG_COUNT (18)
#define TCON_PATH_V005_REG_COUNT (13)

typedef enum
{
    E_PNL_TCON_PATH_MODE_0,     //control by TCON bin
    E_PNL_TCON_PATH_MODE_1,     //MTK Mode 4k2k@60
    E_PNL_TCON_PATH_MODE_2,     //Game Mode for CSOT, INX, AUO 4k1k@144
    E_PNL_TCON_PATH_MODE_3,     //L_G Mode 4k2k@60
    E_PNL_TCON_PATH_MODE_4,     //SO_EM Mode 4k2k@60
    E_PNL_TCON_PATH_MODE_5,     //SEC Mode1 4k2k@60
    E_PNL_TCON_PATH_MODE_6,     //SEC Mode2 4k2k@60
    E_PNL_TCON_PATH_MODE_7,     //SiW Mode 4k2k@60
    E_PNL_TCON_PATH_MODE_8,     //L_G OLED Mode 4k2k@60
    E_PNL_TCON_PATH_MODE_9,     //MTK Mode 4k2k@120
    E_PNL_TCON_PATH_MODE_10,    //L_G Mode 4k2k@120
    E_PNL_TCON_PATH_MODE_11,    //SiW Mode 4k2k@120
    E_PNL_TCON_PATH_MODE_12,    //L_G OLED Mode 4k2k@120
	E_PNL_TCON_PATH_MODE_MAX,
} E_PNL_TCON_PATH_MODE;

u32 u32_tcon_path_reg[TCON_PATH_REG_COUNT] = {
	//data path setting(v1)
	REG_0100_SCDISP_BKA4FA,
	REG_0180_SCTCON_BKA4F7_V004,
	REG_0180_SCTCON_BKA4F7_V004,
	REG_0140_DEMURA_BKA377_V004,
	REG_012C_PAFRC_BKA324_V004,
	REG_003C_PAFRC_BKA324_V004,
	REG_003C_PAFRC_BKA324_V004,
	//clock setting(v1)
	REG_114C_CKGEN01_V004,
	REG_1148_CKGEN01_V004,
	REG_1144_CKGEN01_V004,
	REG_1158_CKGEN01_V004,
	REG_1154_CKGEN01_V004,
	REG_1150_CKGEN01_V004,
	//PAFRC setting
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_01B4_PAFRC_BKA324_V004,
};

u32 u32_tcon_path_mask[TCON_PATH_REG_COUNT] = {
	//data path setting(v1)
	Fld(3, 0, AC_MSKB0), //bit[3:0]
	REG_0180_SCTCON_BKA4F7_V004_REG_DMC_OD_PGA_PATH_SEL_0180, //bit[14:12]
	REG_0180_SCTCON_BKA4F7_V004_REG_LINEOD_PCLRC_PATH_SEL_0180, //bit[0]
	REG_0140_DEMURA_BKA377_V004_REG_PCID_PIXELOD_MOD_EN_0140, //bit[3]
	REG_012C_PAFRC_BKA324_V004_REG_PAFRC_SWITCH_TO_MOD_EN_012C, //bit[6]
	Fld(2, 8, AC_MSKB1), //bit[9:8]
	REG_003C_PAFRC_BKA324_V004_REG_VAC_BF_PANEL_GAMMA_EN_003C, //bit[3]
	//clock setting(v1)
	Fld(3, 3, AC_MSKB0), //bit[5:3]
	Fld(3, 3, AC_MSKB0),
	Fld(3, 3, AC_MSKB0),
	Fld(3, 3, AC_MSKB0),
	Fld(3, 3, AC_MSKB0),
	Fld(3, 3, AC_MSKB0),
	//PAFRC setting
	REG_00FC_PAFRC_BKA324_V004_REG_FRC_ON_00FC,
	REG_00FC_PAFRC_BKA324_V004_REG_DITH_BITS_00FC,
	REG_00FC_PAFRC_BKA324_V004_REG_NOISE_DITH_DISABLE_00FC,
	REG_00FC_PAFRC_BKA324_V004_REG_PAFRC_TAIL_CUT_00FC,
	REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_DITHER_EN_01B4,
};

u16 u16_tcon_path_val[TCON_PATH_REG_COUNT][E_PNL_TCON_PATH_MODE_MAX - 1] = {
    //mode1 mode2   mode3   mode4   mode5   mode6
	//data path setting(v1)
    {0x0,   0x0,   0x0,   0x3,   0x0,   0x0,},
    {0x0,   0x0,   0x0,   0x0,   0x1,   0x1,},
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
    {0x1,   0x1,   0x0,   0x1,   0x1,   0x1,},
    {0x1,   0x1,   0x0,   0x1,   0x1,   0x1,},
    {0x0,   0x2,   0x0,   0x2,   0x1,   0x0,},
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x1,},
	//clock setting(v1)
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
    {0x1,   0x0,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x0,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x0,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x0,   0x1,   0x1,   0x1,   0x1,},
	//PAFRC setting
    {0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
    {0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
    {0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
    {0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
};

u32 u32_tcon_path_reg_v005[TCON_PATH_V005_REG_COUNT] = {
	//data path setting(v3.1)
	REG_0100_SCDISP_BKA4FA,
	REG_0180_SCTCON_BKA4F7_V004,
	REG_0180_SCTCON_BKA4F7_V004,
	REG_0140_DEMURA_BKA377_V004,
	REG_012C_PAFRC_BKA324_V004,
	REG_003C_PAFRC_BKA324_V004,
	REG_003C_PAFRC_BKA324_V004,
	//clock setting(v3.1)
	REG_1148_CKGEN01_V005,
	//PAFRC setting
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_00FC_PAFRC_BKA324_V004,
	REG_01B4_PAFRC_BKA324_V004,
};

u32 u32_tcon_path_mask_v005[TCON_PATH_V005_REG_COUNT] = {
	//data path setting(v3.1)
	Fld(3, 0, AC_MSKB0),//bit[2:0]
	REG_0180_SCTCON_BKA4F7_V004_REG_DMC_OD_PGA_PATH_SEL_0180, //bit[14:12]
	REG_0180_SCTCON_BKA4F7_V004_REG_LINEOD_PCLRC_PATH_SEL_0180, //bit[0]
	REG_0140_DEMURA_BKA377_V004_REG_PCID_PIXELOD_MOD_EN_0140, //bit[3]
	REG_012C_PAFRC_BKA324_V004_REG_PAFRC_SWITCH_TO_MOD_EN_012C, //bit[6]
	Fld(2, 8, AC_MSKB1),//bit[9:8]
	REG_003C_PAFRC_BKA324_V004_REG_VAC_BF_PANEL_GAMMA_EN_003C, //bit[3]
	//clock setting(v3.1)
	REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148, //bit[5:2]
	//PAFRC setting
	REG_00FC_PAFRC_BKA324_V004_REG_FRC_ON_00FC, //bit[0:0]
	REG_00FC_PAFRC_BKA324_V004_REG_DITH_BITS_00FC, //bit[2:2]
	REG_00FC_PAFRC_BKA324_V004_REG_NOISE_DITH_DISABLE_00FC, //bit[3:3]
	REG_00FC_PAFRC_BKA324_V004_REG_PAFRC_TAIL_CUT_00FC, //bit[4:4]
	REG_01B4_PAFRC_BKA324_V004_REG_GAMMA_DITHER_EN_01B4, //bit[9:9]
};

u16 u16_tcon_path_val_v005[TCON_PATH_V005_REG_COUNT][E_PNL_TCON_PATH_MODE_MAX - 1] = {
	//mode1 mode2  mode3  mode4  mode5  mode6  mode7  mode8  mode9  mode10 mode11 mode12
	//data path setting(v3.1)
	{0x0,   0x0,   0x0,   0x3,   0x0,   0x0,   0x0,   0x1,   0x0,   0x0,   0x0,   0x1,},
	{0x0,   0x0,   0x0,   0x0,   0x1,   0x1,   0x1,   0x0,   0x0,   0x0,   0x1,   0x0,},
	{0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
	{0x1,   0x1,   0x0,   0x1,   0x1,   0x1,   0x0,   0x0,   0x1,   0x0,   0x0,   0x0,},
	{0x1,   0x1,   0x0,   0x1,   0x1,   0x1,   0x0,   0x0,   0x1,   0x0,   0x0,   0x0,},
	{0x0,   0x2,   0x0,   0x2,   0x1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
	{0x0,   0x0,   0x0,   0x0,   0x0,   0x1,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
	//clock setting(v3.1)
	{0x3,   0x0,   0x3,   0x1,   0x1,   0x1,   0x3,   0x3,   0x1,   0x1,   0x1,   0x1,},
	//PAFRC setting
	{0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x0,   0x1,   0x0,   0x1,   0x0,},
	{0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,   0x0,},
	{0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
	{0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x0,   0x1,   0x0,   0x1,   0x0,},
	{0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,   0x1,},
};

bool _tcon_path_patch(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	bool ret = TRUE;
	u8 mode_idx = priv->tcon_info.stPanelInfo.u32DataPath_sel - 1;
	u8 reg_count;
	u8 u8PAFRC_SWITCH_TO_MOD_EN = 0;

	TCON_CHECK_PARAMETER_NULL(priv);

	if (priv->tcon_info.stPanelInfo.u32DataPath_sel < E_PNL_TCON_PATH_MODE_1 &&
	    priv->tcon_info.stPanelInfo.u32DataPath_sel >= E_PNL_TCON_PATH_MODE_MAX) {
		TCON_ERROR("Ivalid data path sel=%d\n", priv->tcon_info.stPanelInfo.u32DataPath_sel);
		return FALSE;
	}
	TCON_DEBUG("path_enum=%d\n", priv->tcon_info.stPanelInfo.u32DataPath_sel);
	TCON_DEBUG("lib_version=%d\n", priv->pnl_lib_version);

	if (priv->pnl_lib_version == VERSION5) {
		for (reg_count = 0; reg_count < TCON_PATH_V005_REG_COUNT; reg_count++)
			W2BYTEMSK(u32_tcon_path_reg_v005[reg_count],
				  u16_tcon_path_val_v005[reg_count][mode_idx],
				  u32_tcon_path_mask_v005[reg_count]);

		//This REG_0198_CKGEN01_V004_REG_CKG_S_GPO_0198 setting is already in
		//_mtk_pnl_tcon_ckg_setting()
		//Set 0: LPLL frequency no division for 4K1K144,4k2k144,4k2k120,4k1k240
		//Set 1: LPLL frequency divide by 2 for 4K2K60
	} else {
		for (reg_count = 0; reg_count < TCON_PATH_REG_COUNT; reg_count++) {
			W2BYTEMSK(u32_tcon_path_reg[reg_count],
				  u16_tcon_path_val[reg_count][mode_idx],
				  u32_tcon_path_mask[reg_count]);
		}

		//Set 0: MOD CLK equal 600M → for SEC plsc case
		//Set 1: MOD CLK equal 300M ( matching Input CLK of 4K ) → 60hz use
		if (priv->tcon_info.stPanelInfo.u32DataPath_sel == E_PNL_TCON_PATH_MODE_2) { // if 144, divided by 1
			//W2BYTEMSK((0x206000+0x198), 0x0, Fld(1,8,AC_MSKB2));
			W2BYTEMSK(REG_0198_CKGEN01_V004, 0x0, REG_0198_CKGEN01_V004_REG_CKG_S_GPO_0198);
		} else {
			//W2BYTEMSK((0x206000+0x198), 0x1, Fld(1,8,AC_MSKB2));
			W2BYTEMSK(REG_0198_CKGEN01_V004, 0x1, REG_0198_CKGEN01_V004_REG_CKG_S_GPO_0198);
		}
	}
	u8PAFRC_SWITCH_TO_MOD_EN =
		R2BYTEMSK(REG_012C_PAFRC_BKA324_V004,
			  REG_012C_PAFRC_BKA324_V004_REG_PAFRC_SWITCH_TO_MOD_EN_012C);
	//When use PAFRC in MOD, need set OD mode to 1
	if (u8PAFRC_SWITCH_TO_MOD_EN)
		W2BYTEMSK(REG_01E0_TCON_4K, 0x1, REG_01E0_TCON_4K_REG_OD_MODE_01E0);

	return ret;
}

bool _tcon_general_setting(struct udevice *dev, uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr;
    uint8_t u8Mask;
    uint8_t u8Value;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = ((pu8TconTab[u32tabIdx]<<24) + (pu8TconTab[(u32tabIdx+1)]<<16) +
                    (pu8TconTab[(u32tabIdx+2)]<<8) + pu8TconTab[(u32tabIdx+3)]) & 0xFFFFFFFF;
        u8Mask  = pu8TconTab[(u32tabIdx+4)] & 0xFF;
        u8Value = pu8TconTab[(u32tabIdx+5)] & 0xFF;

        TCON_INFO("[addr=%06x, msk=%02x, val=%02x]\n", u32Addr, u8Mask, u8Value);

        //Update to support ext register.
        TCON_W1BYTEMSK(u32Addr, u8Value, u8Mask);
        u32tabIdx = u32tabIdx + 6;
    }

    bRet = _tcon_path_patch(dev);
    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool _tcon_gpio_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr;
    uint8_t u8Mask;
    uint8_t u8Value;
    //uint16_t u16TconSubBank;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = ((pu8TconTab[u32tabIdx]<<24) + (pu8TconTab[(u32tabIdx+1)]<<16) +
                    (pu8TconTab[(u32tabIdx+2)]<<8) + pu8TconTab[(u32tabIdx+3)]) & 0xFFFFFFFF;
        u8Mask  = pu8TconTab[(u32tabIdx+4)] & 0xFF;
        u8Value = pu8TconTab[(u32tabIdx+5)] & 0xFF;

        TCON_INFO("[addr=%06x, msk=%02x, val=%02x] \n", u32Addr, u8Mask, u8Value);

        //Update to support ext register.
        TCON_W1BYTEMSK(u32Addr, u8Value, u8Mask);
        u32tabIdx = u32tabIdx + 6;
    }

    TCON_W1BYTEMSK(0x2444A0, BIT(1), BIT(1));
    mdelay(30); //need to use mdelay , udelay didn't work
    TCON_W1BYTEMSK(0x2444A0, 0, BIT(1));

    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool _tcon_scaler_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr;
    uint16_t u16Mask;
    uint16_t u16Value;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = ((pu8TconTab[u32tabIdx]<<24) + (pu8TconTab[(u32tabIdx+1)]<<16) +
                    (pu8TconTab[(u32tabIdx+2)]<<8) + pu8TconTab[(u32tabIdx+3)]) & 0xFFFFFFFF;
        u16Mask  = pu8TconTab[(u32tabIdx+4)] & 0xFF;
        u16Value = pu8TconTab[(u32tabIdx+5)] & 0xFF;

        TCON_INFO("[addr=%06x, msk=%02x, val=%02x] \n", u32Addr, u16Mask, u16Value);

        TCON_W1BYTEMSK(u32Addr, u16Value, u16Mask);
        u32tabIdx = u32tabIdx + 6;
    }

    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool _tcon_mod_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32Addr;
    uint16_t u16Mask;
    uint16_t u16Value;
    bool bRet = TRUE;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32Addr = ((pu8TconTab[u32tabIdx]<<24) + (pu8TconTab[(u32tabIdx+1)]<<16) +
                    (pu8TconTab[(u32tabIdx+2)]<<8) + pu8TconTab[(u32tabIdx+3)]) & 0xFFFFFFFF;
        u16Mask  = pu8TconTab[(u32tabIdx+4)] & 0xFF;
        u16Value = pu8TconTab[(u32tabIdx+5)] & 0xFF;

        TCON_INFO("[addr=%06x, msk=%02x, val=%02x] \n", u32Addr, u16Mask, u16Value);

        //Update to support ext register.
        TCON_W1BYTEMSK(u32Addr, u16Value, u16Mask);
        u32tabIdx = u32tabIdx + 6;
    }

    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool _tcon_show_panelinfo(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	TCON_CHECK_PARAMETER_NULL(priv);

	TCON_DEBUG("\nlinktype=%u hsync st=%u w=%u vsync st=%u w=%u\n"
		"de_hstart=%u de_vstart=%u de_width=%u de_height=%u\n"
		"typ_htt=%u typ_vtt=%u typ_dclk=%llu\n"
		"ssc_en=%u ssc_modulation=%u ssc_deviation=%u\n",
		priv->linktype,
		priv->hsync_st, priv->hsync_w,
		priv->vsync_st, priv->vsync_w,
		priv->de_hstart, priv->de_vstart,
		priv->de_width, priv->de_height,
		priv->typ_htt, priv->typ_vtt, priv->typ_dclk,
		priv->ssc_info.ssc_en,
		priv->ssc_info.ssc_modulation, priv->ssc_info.ssc_deviation);

	return TRUE;
}

bool _tcon_panelinfo_setting(struct udevice *dev, uint8_t* pu8TconTab, uint32_t u32RegisterlistSize)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    uint16_t u16PanelInfoID;
    uint32_t u32PanelInfoValue;
    uint16_t u16Count = 0;
    uint16_t u16MaxRegisterlistSize = u32RegisterlistSize/6;
    bool bRet = TRUE;

    TCON_CHECK_PARAMETER_NULL(priv);

    //ST_PNL_TCON_PANELINFO stPanelInfo;
    memset(&priv->tcon_info.stPanelInfo, 0, sizeof(ST_PNL_TCON_PANELINFO));

    //demura enable default set to true
    priv->tcon_info.stPanelInfo.u32DemuraEnable = TRUE;

    for (u16Count = 0; u16Count < u16MaxRegisterlistSize; u16Count++)
    {
        u16PanelInfoID = (pu8TconTab[1+6*u16Count]<<8) + (pu8TconTab[6*u16Count]);
        u32PanelInfoValue = (pu8TconTab[5+6*u16Count]<<24) + (pu8TconTab[4+6*u16Count]<<16) + (pu8TconTab[3+6*u16Count]<<8) + (pu8TconTab[2+6*u16Count]);
        TCON_INFO("u16PanelInfoID=%d u32PanelInfoValue=%d\n", u16PanelInfoID, u32PanelInfoValue);

        switch (u16PanelInfoID)
        {
           case E_PNL_TCON_PANELINFO_PANELLINKTYPE:
               priv->tcon_info.stPanelInfo.u32PanelLinkType = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELLINKEXTTYPE:
               priv->tcon_info.stPanelInfo.u32PanelLinkExtType = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_FIXEDVABCKPORCH:
               priv->tcon_info.stPanelInfo.u32FixedVBackPorch = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_FIXEDHBACKPORCH:
               priv->tcon_info.stPanelInfo.u32FixedHBackPorch = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VSYNC_START:
               priv->tcon_info.stPanelInfo.u32VsyncStart = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VSYNC_END:
               priv->tcon_info.stPanelInfo.u32VsyncEnd = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VDESTART:
               priv->tcon_info.stPanelInfo.u32VDEStart = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VDEND:
               priv->tcon_info.stPanelInfo.u32VDEEnd = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMAXVTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelMaxVTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELVTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelVTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_MINVTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelMinVTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VDEHEADDUMMY:
               priv->tcon_info.stPanelInfo.u32VDEHeadDummy = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VDETAILDUMMY:
               priv->tcon_info.stPanelInfo.u32VDETailDummy = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HSYNC_START:
               priv->tcon_info.stPanelInfo.u32HsyncStart = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HSYNC_END:
               priv->tcon_info.stPanelInfo.u32HsyncEnd = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HDESTART:
               priv->tcon_info.stPanelInfo.u32HDEStart = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HDEEND:
               priv->tcon_info.stPanelInfo.u32HDEEnd = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMAXHTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelMaxHTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELHTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelHTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMINHTOTAL:
               priv->tcon_info.stPanelInfo.u32PanelMinHTotal = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HDEHEADDUMMY:
               priv->tcon_info.stPanelInfo.u32HDEHeadDummy = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_HDETAILDUMMY:
               priv->tcon_info.stPanelInfo.u32HDETailDummy = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMAXDCLK:
               priv->tcon_info.stPanelInfo.u32PanelMaxDCLK = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELDCLK:
               priv->tcon_info.stPanelInfo.u32PanelDCLK = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMINDCLK:
               priv->tcon_info.stPanelInfo.u32PanelMinDCLK = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELMAXSET:
               priv->tcon_info.stPanelInfo.u32PanelMaxSET = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_MINSET:
               priv->tcon_info.stPanelInfo.u32PanelMinSET = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_OUTTIMINGMODE:
               priv->tcon_info.stPanelInfo.u32OutTimingMode = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_TOTALPAIR:
               priv->tcon_info.stPanelInfo.u32HTotalPair = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PAIREVEN:
               priv->tcon_info.stPanelInfo.u32VTotalPair_Even = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PAIRODD:
               priv->tcon_info.stPanelInfo.u32VTotalPair_Odd = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_DUALPORT:
               priv->tcon_info.stPanelInfo.u32DaulPort = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELHSYNCWIDTH:
               priv->tcon_info.stPanelInfo.u32PanelHsyncWidth = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELHSYNCBACKPORCH:
               priv->tcon_info.stPanelInfo.u32PanelHsyncBackPorch = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELVSYNCWIDTH:
               priv->tcon_info.stPanelInfo.u32PanelVsyncWidth = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_PANELVBACKPORCH:
               priv->tcon_info.stPanelInfo.u32PanelVsyncBackPorch = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_SSC_ENABLE:
               priv->tcon_info.stPanelInfo.u32SSCEnable = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_SSC_FMODULATION:
               priv->tcon_info.stPanelInfo.u32SSC_Fmodulation = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_SSC_PERCENTAGE:
               priv->tcon_info.stPanelInfo.u32SSC_Percentage = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_SSC_BIN_CTRL:
               priv->tcon_info.stPanelInfo.u32SSC_TconBinCtrl = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_DEMURA_SEL:
               priv->tcon_info.stPanelInfo.u32Demura_sel = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_DATA_PATH_SEL:
               priv->tcon_info.stPanelInfo.u32DataPath_sel = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_VCOM_SEL:
               priv->tcon_info.stPanelInfo.u32Vcom_sel = u32PanelInfoValue;
               break;
           case E_PNL_TCON_PANELINFO_DEMURA_ENABLE:
               priv->tcon_info.stPanelInfo.u32DemuraEnable = u32PanelInfoValue;
               break;
			case E_PNL_TCON_PANELINFO_OD_ENABLE:
				priv->tcon_info.stPanelInfo.u32OdEnable = u32PanelInfoValue;
				break;
			case E_PNL_TCON_PANELINFO_OD_MODE_TYPE:
				priv->tcon_info.stPanelInfo.u32OdModeType = u32PanelInfoValue;
				break;
        }
    }

    //u32PanelLinkType: for STI and uboot
    //u32PanelLinkExtType: for mixed mode utopia
    TCON_DEBUG("\nPanelLinkType=%u PanelLinkExtType=%u Demura_sel=%u DataPath_sel=%u "
               "Vcom_sel=%u DemuraEnable=%u\n",
                priv->tcon_info.stPanelInfo.u32PanelLinkType,
                priv->tcon_info.stPanelInfo.u32PanelLinkExtType,
                priv->tcon_info.stPanelInfo.u32Demura_sel,
                priv->tcon_info.stPanelInfo.u32DataPath_sel,
                priv->tcon_info.stPanelInfo.u32Vcom_sel,
                priv->tcon_info.stPanelInfo.u32DemuraEnable);

    TCON_CHECK_EQUAL_AND_ASSIGN(priv->linktype, priv->tcon_info.stPanelInfo.u32PanelLinkType);

    TCON_CHECK_EQUAL_AND_ASSIGN(priv->hsync_st, priv->tcon_info.stPanelInfo.u32HsyncStart);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->hsync_w, (priv->tcon_info.stPanelInfo.u32HsyncEnd - priv->tcon_info.stPanelInfo.u32HsyncStart + 1));
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->vsync_st, priv->tcon_info.stPanelInfo.u32VsyncStart);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->vsync_w, (priv->tcon_info.stPanelInfo.u32VsyncEnd - priv->tcon_info.stPanelInfo.u32VsyncStart + 1));

    TCON_CHECK_EQUAL_AND_ASSIGN(priv->de_hstart, priv->tcon_info.stPanelInfo.u32HDEStart);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->de_vstart, priv->tcon_info.stPanelInfo.u32VDEStart);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->de_width, (priv->tcon_info.stPanelInfo.u32HDEEnd - priv->tcon_info.stPanelInfo.u32HDEStart + 1));
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->de_height, (priv->tcon_info.stPanelInfo.u32VDEEnd - priv->tcon_info.stPanelInfo.u32VDEStart + 1));

    // max_htt / min_htt / max_vtt / min_vtt follow dtsi value.
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->typ_htt, priv->tcon_info.stPanelInfo.u32PanelHTotal);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->typ_vtt, priv->tcon_info.stPanelInfo.u32PanelVTotal);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->typ_dclk, priv->tcon_info.stPanelInfo.u32PanelDCLK);

    TCON_CHECK_EQUAL_AND_ASSIGN(priv->ssc_info.ssc_en, priv->tcon_info.stPanelInfo.u32SSCEnable);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->ssc_info.ssc_modulation, priv->tcon_info.stPanelInfo.u32SSC_Fmodulation);
    TCON_CHECK_EQUAL_AND_ASSIGN(priv->ssc_info.ssc_deviation, priv->tcon_info.stPanelInfo.u32SSC_Percentage);

    return bRet;
}

bool _tcon_powersequence_setting(uint8_t* pu8TconTab, uint16_t u16RegisterCount)
{
    uint32_t u32tabIdx = 0;
    uint32_t u32SubAddr = 0;
    uint8_t u8SubMask = 0;
    uint8_t u8SubNum = 0;
    uint32_t u32Addr = 0;
    uint16_t u16Mask = 0;
    uint16_t u16Value = 0;
    uint16_t u16Delay = 0;
    bool bRet = TRUE;
    //uint8_t u8Reserved = 0;

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(pu8TconTab);

    while (u16RegisterCount--)
    {
        u32SubAddr = ((pu8TconTab[u32tabIdx+2]<<16) + (pu8TconTab[(u32tabIdx+1)]<<8) + pu8TconTab[(u32tabIdx)]);
        u8SubMask = pu8TconTab[(u32tabIdx+3)];
        u8SubNum = pu8TconTab[(u32tabIdx+4)];
        u32Addr = ((pu8TconTab[u32tabIdx+7]<<16) + (pu8TconTab[(u32tabIdx+6)]<<8) + pu8TconTab[(u32tabIdx+5)]);
        u16Mask = ((pu8TconTab[u32tabIdx+9]<<8) + pu8TconTab[(u32tabIdx+8)]);
        u16Value = ((pu8TconTab[u32tabIdx+11]<<8) + pu8TconTab[(u32tabIdx+10)]);
        u16Delay = ((pu8TconTab[u32tabIdx+13]<<8) + pu8TconTab[(u32tabIdx+12)]);
        //u8Reserved = pu8TconTab[(u32tabIdx+14)];

        TCON_INFO("u32SubAddr=%06x u8SubMask=%02x u8SubNum=%02x\n",\
                    u32SubAddr, u8SubMask, u8SubNum);
        TCON_INFO("u32Addr=%06x u16Mask=%02x u16Value=%02x u8Delay=%d\n",\
                    u32Addr, u16Mask, u16Value, u16Delay);

        if (u32SubAddr != 0)//Change sub bank
        {
            TCON_W2BYTEMSK(u32SubAddr, u8SubNum, u8SubMask);
        }
        if (u32Addr != 0)
        {
            TCON_W2BYTEMSK(u32Addr,u16Value,u16Mask);
        }
        mdelay(u16Delay);

        u32tabIdx = u32tabIdx + 15;
    }

    TCON_FUNC_EXIT(bRet);
    return bRet;
}

bool _tcon_dump_table(struct udevice *dev, uint8_t *pu8Table, uint8_t u8TconType)
{
    bool bRet = TRUE;
    st_tcon_tab_info stInfo;
    TCON_CHECK_PARAMETER_NULL(pu8Table);

    memset(&stInfo, 0, sizeof(st_tcon_tab_info));
    stInfo.pu8Table = pu8Table;
    stInfo.u8TconType = u8TconType;

    bRet = get_tcon_dump_table(&stInfo);

    if (!bRet)
    {
        TCON_ERROR("get_tcon_dump_table return false\n");
        return bRet;
    }

    switch (u8TconType)
    {
        case E_TCON_TAB_TYPE_GENERAL:
            bRet = _tcon_general_setting(dev, stInfo.pu8Table, stInfo.u16RegCount);
            break;
        case E_TCON_TAB_TYPE_GPIO:
            bRet = _tcon_gpio_setting(stInfo.pu8Table, stInfo.u16RegCount);
            break;
        case E_TCON_TAB_TYPE_SCALER:
            bRet = _tcon_scaler_setting(stInfo.pu8Table, stInfo.u16RegCount);
            break;
        case E_TCON_TAB_TYPE_MOD:
            bRet = _tcon_mod_setting(stInfo.pu8Table, stInfo.u16RegCount);
            break;

        case E_TCON_TAB_TYPE_POWER_SEQUENCE_ON:
        case E_TCON_TAB_TYPE_POWER_SEQUENCE_OFF:
            if (stInfo.u8Version > TCON20_VERSION)
            {
                bRet = _tcon_powersequence_setting(stInfo.pu8Table, stInfo.u16RegCount);
            }
            break;
        case E_TCON_TAB_TYPE_GAMMA:
            break;
        case E_TCON_TAB_TYPE_PANEL_INFO:
            if (stInfo.u8Version > TCON20_VERSION)
            {
               /*if( (E_PNL_TCON20_REGISTERTYPE)u8RegisterType ==  EN_TCON20_REGISTERTYPE_PANEL_6_BYTES_NEWSSC)
               {
                   MDrv_PNL_Set_SSC_EnableHighAccurate(pInstance,1);
               }
               else
               {
                   MDrv_PNL_Set_SSC_EnableHighAccurate(pInstance,0);
               }*/
               bRet = _tcon_panelinfo_setting(dev, stInfo.pu8Table, stInfo.u32ReglistSize);
               //stTCONPanelConfig.enPanelInterface = u8PanelInterface;
            }
            break;
        case E_TCON_TAB_TYPE_OVERDRIVER:
            bRet = mtk_tcon_od_setting(dev);
            break;
        case E_TCON_TAB_TYPE_VAC_REG:
            if (stInfo.u8Version > TCON20_VERSION)
            {
                bRet = mtk_tcon_vac_reg_setting(stInfo.pu8Table, stInfo.u16RegCount);
            }
            break;
        default:
            TCON_DEBUG("GetTable: unknown tcon type=%u> \n", u8TconType);
            bRet = FALSE;
            break;
    }

    return bRet;
}

bool _mtk_tcon_setup(void)
{
    bool bRet = FALSE;

    //log level initialize
    bRet = init_log_level();

    return bRet;
}

bool mtk_tcon_preinit(struct udevice *dev)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    bool data_exist = FALSE, setup_status = FALSE;
    loff_t data_len = 0 ;
    unsigned char *pdata_buf = NULL;
    uint64_t u64Start = get_timer(0);

    TCON_CHECK_PARAMETER_NULL(priv);

    setup_status = _mtk_tcon_setup();

    load_tcon_files(dev);
    data_exist = is_tcon_data_exist(&pdata_buf, &data_len);

    if (data_exist)
    {
        _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_PANEL_INFO); // verison 2 new panel table
    }

    TCON_DEBUG("tcon preinit panel info done. Setup status=%d data_exist=%d\n", setup_status, data_exist);
    UBOOT_BOOTTIME("[tcon preinit][total time: %llu]\n", get_timer(0) - u64Start);

    return data_exist;
}

bool mtk_tcon_pq_init(struct udevice *dev, struct st_tcon_pq_force_en force_en)
{
	u8 tcon_version = 0;
	loff_t data_len = 0;
	bool data_exist = FALSE;
	unsigned char *pdata_buf = NULL;
	bool ret_val = TRUE;

	_mtk_tcon_setup();
	load_tcon_pq_files(dev, force_en);
	data_exist = is_tcon_data_exist(&pdata_buf, &data_len);
	if (data_exist) {
		if (!get_tcon_version(pdata_buf, &tcon_version)) {
			TCON_ERROR("get tcon version return failed\n");
			return FALSE;
		}
		if (mtk_pnl_autodownload_init(dev) == FALSE)
			TCON_ERROR("autodownload init failed\n");

		ret_val &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_VAC_REG);
		if (tcon_version > TCON20_VERSION)
			ret_val &= mtk_tcon_lineod_setting(dev);
	}

	ret_val &= mtk_tcon_od_setting(dev);
	ret_val &= mtk_tcon_vac_setting(dev);
	ret_val &= mtk_tcon_panelgamma_setting(dev);

	free_resource();
	return ret_val;
}

bool mtk_tcon_init(struct udevice *dev)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    bool data_exist = FALSE;
    bool bRet = TRUE;
    loff_t data_len = 0;
    unsigned char *pdata_buf = NULL;
    uint8_t u8Version = 0;
    uint64_t u64Start = get_timer(0);

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(priv);

    data_exist = is_tcon_data_exist(&pdata_buf, &data_len);


    if (data_exist)
    {
        if (!get_tcon_version(pdata_buf, &u8Version))
        {
            TCON_ERROR("get tcon version return failed\n");
            return FALSE;
        }

        bRet &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_GENERAL);
        bRet &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_GPIO);
        bRet &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_SCALER);
        bRet &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_MOD);

        if (mtk_pnl_autodownload_init(dev) == FALSE)
        {
            TCON_ERROR("autodownload init failed\n");
        }

        bRet &= _tcon_dump_table(dev, pdata_buf, E_TCON_TAB_TYPE_VAC_REG);

        if (u8Version > TCON20_VERSION)
        {
            bRet &= mtk_tcon_lineod_setting(dev);
        }

    }

    bRet &= mtk_tcon_od_setting(dev);
    bRet &= mtk_tcon_vac_setting(dev);
    bRet &= mtk_tcon_panelgamma_setting(dev);

	_tcon_show_panelinfo(dev);

    TCON_FUNC_EXIT(bRet);
    lTconEanbleTiming = get_timer(0);
    UBOOT_BOOTTIME("[tcon init][total time: %llu]\n", lTconEanbleTiming - u64Start);

    return bRet;
}

bool mtk_tcon_enable(struct udevice *dev, bool bEn)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);
    bool data_exist = FALSE;
    bool bRet = TRUE;
    loff_t data_len = 0;
    unsigned char *pdata_buf = NULL;
    uint64_t u64Start = get_timer(0);

    TCON_FUNC_ENTER();
    TCON_CHECK_PARAMETER_NULL(priv);

    data_exist = is_tcon_power_seq_data_exist(&pdata_buf, &data_len);

    if (data_exist) {
        TCON_DEBUG("Use separate power seq. on bin file setting.\n");
        bRet &= _tcon_dump_table(
                dev, pdata_buf, E_TCON_TAB_TYPE_POWER_SEQUENCE_ON);
    } else {
        TCON_DEBUG("Use power seq. on setting in the tcon bin.\n");
        data_exist = is_tcon_data_exist(&pdata_buf, &data_len);

        if (data_exist)
            bRet &= _tcon_dump_table(
                dev, pdata_buf, E_TCON_TAB_TYPE_POWER_SEQUENCE_ON);
    }

    //free tcon bin resource
    free_resource();

    UBOOT_BOOTTIME("[tcon enable][total time: %llu]\n" \
                   "[tcon init -> enable][cost time: %lu]\n",
                   get_timer(0) - u64Start, get_timer(0) - lTconEanbleTiming);

    return bRet;
}
