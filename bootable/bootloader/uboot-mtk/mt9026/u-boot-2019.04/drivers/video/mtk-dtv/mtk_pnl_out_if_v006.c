// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */
//#define DEBUG
#include <common.h>
#include <dm.h>
#include <utility.h>
#include <debug_impl.h>
#include <system_impl.h>
#include <mtk_panel.h>
#include <dts_parser.h>
#include "coda/TGEN_MAIN_BKA3A0.h"
#include "coda/TGEN_FRAMELOCK_BKA3A2.h"
#include "coda/TGEN_VRR_BKA39F.h"
#include "coda/TRIGGER_GEN_BKA3A5.h"
#include "coda/DISP_LB_BKA326.h"
#include "coda/TGEN_REGEN_BKA39C.h"
#include "coda/DELTA_PATH_BKA35B.h"
#include "coda/PATGEN_SCALER_BKA404.h"
#include "coda/PATGEN_SCALER_BKA407.h"
#include "coda/PATGEN_SCALER_BKA409.h"
#include "coda/GOPG_HVSP4_BKA4DA.h"
#include "coda/ckgen00.h"
#include "coda/ckgen01.h"
#include "coda/CKGEN00_V004.h"
#include "coda/CKGEN01_V004.h"
#include "coda/modv11.h"
#include "coda/modv12.h"
#include "coda/modv13.h"
#include "coda/modv21.h"
#include "coda/modv22.h"
#include "coda/modd1.h"
#include "coda/moda1.h"
#include "coda/moda2.h"
#include "coda/moda3.h"
#include "coda/moda4.h"
#include "coda/modosd1.h"
#include "coda/modosd2.h"
#include "coda/lpll.h"
#include "coda/SCAID_BIND_BKA3AE.h"
#include "coda/PATGEN_SCALER_BKA401.h"
#include "coda/TRIGGER_GEN_BKA3A4.h"
#include "coda/GOP0_BKA4DE.h"
#include "coda/GOPG_BKA4D9.h"
#include "coda/IP2_M_BKA330.h"
#include "coda/SCTCON_MISC_BKA3E0.h"
#include "coda/TGEN_OUTPUT_BKA3A3.h"
#include "coda/reg_META_DATA_TRANSFER_BKA35A.h"
#include "coda/PAFRC_BKA324.h"
#include "coda/PAFRC_BKA324_V004.h"
#include "coda/DEMURA_BKA377_V004.h"
#include "coda/LINEOD_BKA3A8_V004.h"
#include "coda/PAT_DET_BKA37F_V004.h"
#include "coda/OD_1ST_BKA336_V004.h"
#include "coda/OD_2ND_BKA337_V004.h"
#include "coda/OD_3RD_BKA338_V004.h"
#include "coda/SCTCON_BKA4F7_V004.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_pnl.h"
#include "mtk_pnl_out_if_v006.h"
#include "coda/MODV11_V004.h"
#include "coda/MODV12_V004.h"
#include "coda/MODV13_V004.h"
#include "coda/MODD1_V004.h"
#include "coda/MODA1_V004.h"
#include "coda/MODA2_V004.h"
#include "coda/MODA3_V004.h"
#include "coda/MODA4_V004.h"
#include "coda/MODOSD1_V004.h"
#include "coda/MODOSD2_V004.h"
#include "coda/LPLL_V004.h"
#include "coda/DISP_MISC_V004.h"
#include "coda/DET_V004.h"
#include "coda/SCDISP_BKA4FA.h"
#include "mtk_pnl_autodownload.h"
#include "coda/CKGEN00_V005.h"
#include "coda/CKGEN01_V005.h"
#include "coda/MODV11_V005.h"
#include "coda/MODV12_V005.h"
#include "coda/MODD1_V005.h"
#include "coda/MODOSD1_V005.h"
#include "coda/MODOSD2_V005.h"
#include "coda/MODA1_V005.h"
#include "coda/MODA4_V005.h"
#include "coda/LPLL_V005.h"
#include "coda/disp_misc.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_out_if.h"
#include "mtk_tcon_out_if.h"
//#include "mtk_pnl_clk_ctrl.h"
#include "coda/FRCPLL.h"
#include "coda/SCTCON_MISC_BKA3E0_V005.h"
//#include "coda/XCPLL_REG_V005.h"
#include "coda/CKGEN00_V006.h"
#include "coda/CKGEN01_V006.h"
#include "coda/XCPLL_V006.h"
#include "coda/BLEND_TOP_BKA36B.h"
#include "coda/TCON_VCOM_PAT_V005.h"
#include "coda/OSDB_COLOV_BKA3ED.h"

//Only for HAPS
#define REG_HFRC_HAPS_BASE	(0x147F400)
#define REG_HFRC_HVSP_IP_BASE	(0x147F600)
#define REG_HFRC_HVSP_OP_BASE	(0x147E800)

#define TGEN_2P (2)
#define TGEN_4P (4)

#define MFT_P (8)

#define MOD_VER2 (2)
#define MOD_VER3 (3)
#define REG_ALL (0xFFFF)

#define FULL_BIT (16)
#define VBO_USE_LANE_NUM_64 64
#define VBO_USE_LANE_NUM_40 40
#define VBO_USE_LANE_NUM_32 32
#define VBO_USE_LANE_NUM_16 16
#define VBO_USE_LANE_NUM_8 8
#define VBO_USE_LANE_NUM_4 4

#define OUT_CONFIG_OFF (0x0000)
#define OUT_CONFIG_4K60_1_VER1 (0x5545)
#define OUT_CONFIG_4K60_2_VER1 (0x0010)
#define OUT_CONFIG_4K60_1_VER2 (0x5555)
#define OUT_CONFIG_4K60_2_VER2 (0x0000)

#define FRCPLL_CONFIG_SET_0_0014 (0xe6cc)
#define FRCPLL_CONFIG_SET_1_0018 (0x1f)
#define FRCPLL_CONFIG_STEP_0_001C (0x001e)
#define FRCPLL_CONFIG_SPAN_0_0024 (0x02a6)
#define FRCPLL_CONFIG_TEST_0000 (0x0012)
#define FRCPLL_CONFIG_LOOP_DIV_SECOND_000C (0x03)
#define FRCPLL_CONFIG_ICP_ICTRL_0010 (0x02)
#define FRCPLL_CONFIG_OUTPUT_DIV_000C (0x04)

#define FRCPLL_CONFIG_UDELAY (500)

#define VBY1_LOCK_PROTECT_EN true

//efuse for mod
#define EFUSE_BANK (0x252A0)	//bank 0x129_50(8bit) to switch sub-bank.
#define MOD_ATOP_SUBBANK (0x12)	// MOD_ATOP subbank is 0x12, addr[8:2]
#define MOD_ATOP_SUBBANK_VER004 (0x15)	// MOD_ATOP subbank is 0x12, addr[8:2]
#define MOD_ATOP_SUBBANK_VER005 (0x12)	// MOD_ATOP subbank is 0x12, addr[8:2]
#define MOD_ATOP_SUBBANK_0C (0x0C)
#define MOD_ATOP_SUBBANK_0D (0x0D)
#define MOD_ATOP_SUBBANK_13_VER005 (0x13)
#define MOD_ATOP_SUBBANK_14_VER005 (0x14)
#define EFUSE_PPM_REG_TABLE_1 (0x2E)	//bank 0x129_A0 (8bit)
#define EFUSE_PPM_REG_TABLE_1_TCON_DISABLE_BIT (1<<25)	//Bit25


#define EFUSE_VAL_LSB	(0x252B0)
#define EFUSE_VAL_MSB	(0x252B4)
#define TIME_OUT_CNT	(10)
#define RINT_THRESHOLD	(0x200)	// 12M:0x200, 24M:0x100
#define MOD_VOL_VX1_ACT_BIT_V006	(16)
#define MOD_CUR_SINGLE_LSB	(7)
#define MOD_CUR_SINGLE_MASK (0x7F)
#define MOD_CUR_SINGLE_ACT_BIT	(14)
#define MOD_CUR_SINGLE_ACT_BIT_VER2	(24)

#define MOD_CUR_DOUBLE_MASK_V006 (0x7F)
#define MOD_CUR_DOUBLE_MASK_VER5 (0x3F)
#define MOD_CUR_DOUBLE_LSB_V006	(17)
#define MOD_CUR_DOUBLE_LSB_VER5	(7)
#define MOD_CUR_DOUBLE_OFFSET	(15)
#define MOD_CUR_DOUBLE_ACT_BIT	(22)
#define MOD_CUR_DOUBLE_ACT_BIT_V006	(24)
#define MOD_RINT_OFFSET		(23)
#define MOD_RINT_ACT_BIT	(30)
#define LPLL_RETIME_CTRL	(25)
#define LPLL_DELAY_TIME_CTRL	(26)

//max/min/default for RCON setting for swing calibration
#define RCON_MAX_VAL	(25)
#define RCON_MIN_VAL	(6)
#define RCON_DEF_VAL	(16)

//max/min/default for BiasCon
#define BIASCON_SINGLE_MAX_VAL	(85)
#define BIASCON_SINGLE_MIN_VAL	(55)
#define BIASCON_DOUBLE_MAX_VAL	(85)
#define BIASCON_DOUBLE_MIN_VAL	(55)
#define BIASCON_DEF_VAL	(70)
#define DIV_2	(2)
#define DIV_4	(4)
#define DIV_8	(8)

#define CHIP_VERSION2 (2)
#define CHIP_VERSION3 (3)
#define CHIP_VERSION4 (4)

#define DEC_2 (2)
#define DEC_4 (4)
#define DEC_5 (5)
#define DEC_100 (100)

//MT5873 XCPLL PD_CLK
#define XCPLL_REG_0004_SET (0x0010)

#ifdef MSOS_TYPE_LINUX_KERNEL
#define mst_atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0);
#else
#define mst_atoi(str) strtoul(((str != NULL) ? str : ""), NULL, 0);
#endif

//for QMS
#define QMS_TFR_MAX 14
#define QMS_OUT_MAX 2

#ifndef MAX
#define MAX(a, b)            (((a) > (b)) ? (a) : (b))
#endif
#define SHIFT_LEFT_8(n)     ((n) << 8)

static const struct mtk_qms_tfr_mapping_info qms_info[QMS_OUT_MAX][QMS_TFR_MAX] = {
	{
		//Tfr  vfreq ratio
		{0, 0, 0, 0},
		{1, 2397, 2, 4795},
		{2, 2400, 2, 4800},
		{3, 2500, 2, 5000},
		{4, 2997, 2, 5994},
		{5, 3000, 2, 6000},
		{6, 4795, 1, 4795},
		{7, 4800, 1, 4800},
		{8, 5000, 1, 5000},
		{9, 5994, 1, 5994},
		{10, 6000, 1, 6000},
		{11, 10000, 0, 0},
		{12, 11988, 0, 0},
		{13, 12000, 0, 0},
	},
	{
		{0, 0, 0, 0},
		{1, 2397, 5, 11989},
		{2, 2400, 5, 12000},
		{3, 2500, 4, 10000},
		{4, 2997, 4, 11989},
		{5, 3000, 4, 12000},
		{6, 4795, 2, 9589},
		{7, 4800, 2, 9600},
		{8, 5000, 2, 10000},
		{9, 5994, 2, 11989},
		{10, 6000, 2, 12000},
		{11, 10000, 1, 10000},
		{12, 11988, 1, 11988},
		{13, 12000, 1, 12000},
	},

};

#if (VBY1_LOCK_PROTECT_EN)
void _mtk_pnl_checkLockProtect_v006(void)
{
#define COUNTMAX 50
#define TOGGLE_DELAY 50		//us
#define CHECK_DELAY 2		//ms
	bool bLocknEn = true;
	uint8_t u8uLockCount = 0;
	uint8_t u8ResetCount = 0;

	bLocknEn = R2BYTEMSK(REG_0184_MODV12, REG_0184_MODV12_REG_VBY1_LOCK_VIDEO);
	u8uLockCount = R2BYTEMSK(REG_01F8_MODV12, REG_01F8_MODV12_REG_UNLOCK_CNT);

	while ((bLocknEn == true) || (u8uLockCount != 0)) {
		if (bLocknEn) {
			if (u8uLockCount == 0) {
				// toggle vby1 test en
				W2BYTEMSK(REG_018C_MODV12, true, REG_018C_MODV12_REG_VBY1_TEST_EN);
				udelay(TOGGLE_DELAY);
				W2BYTEMSK(REG_018C_MODV12, false, REG_018C_MODV12_REG_VBY1_TEST_EN);
				// clear unlock cnt before training
				W2BYTEMSK(REG_01F8_MODV12, true,
					  REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
				udelay(TOGGLE_DELAY);
				W2BYTEMSK(REG_01F8_MODV12, false,
					  REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
				mdelay(CHECK_DELAY);
			} else {
				// clear unlock cnt before training
				W2BYTEMSK(REG_01F8_MODV12, true,
					  REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
				udelay(TOGGLE_DELAY);
				W2BYTEMSK(REG_01F8_MODV12, false,
					  REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
				mdelay(CHECK_DELAY);
			}
		} else {
			// clear unlock cnt before training
			W2BYTEMSK(REG_01F8_MODV12, true, REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
			udelay(TOGGLE_DELAY);
			W2BYTEMSK(REG_01F8_MODV12, false, REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
			mdelay(CHECK_DELAY);
		}

		bLocknEn = R2BYTEMSK(REG_0184_MODV12, REG_0184_MODV12_REG_VBY1_LOCK_VIDEO);
		u8uLockCount = R2BYTEMSK(REG_01F8_MODV12, REG_01F8_MODV12_REG_UNLOCK_CNT);

		if ((u8ResetCount > COUNTMAX) || ((bLocknEn == false) && (u8uLockCount == 0))) {
			if (u8ResetCount > COUNTMAX)
				debug("[%s] u8ResetCount=%d \n", __func__, u8ResetCount);
			break;
		}
		u8ResetCount++;
	}
}
#endif

void mtk_pnl_out_en_v006(struct udevice *dev, bool en)
{
#define TOGGLE_DELAY 50		//us
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint8_t u8laneCount;
	bool bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE_MAX] = { false };
	en_boot_pnl_output_lane_config eOutputLaneEnNumber = E_BOOT_PNL_OUTPUT_LANE0;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}
	debug("[%s] enable panel output :%d\n", __func__, en);

	if (priv->tcon_info.bUsingTCON) {
		if (is_tcon_sti_flow()) {
			//The order required by the general panel should be load pmic first
			//and then load tcon bin power on
			mtk_tcon_enable(dev, en);
			TCON_DEBUG("Tcon related output config settings are set in tcon bin.\n");
		} else {
			MHal_PNL_EnableTcon(dev);
		}
		return;
	}
	// clear unlock cnt before training
	W2BYTEMSK(REG_01F8_MODV12, true, REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);
	udelay(TOGGLE_DELAY);
	W2BYTEMSK(REG_01F8_MODV12, false, REG_01F8_MODV12_REG_UNLOCK_CNT_CLR);

	debug("[%s] enable panel output :%d\n", __func__, en);

	if (en == true && priv->linktype == E_LINK_VB1) {
		W2BYTEMSK(REG_0184_MODV12, 0xF, REG_0184_MODV12_REG_VBY1_ALN_DE_CNT);
		W2BYTEMSK(REG_0184_MODV12, 0x3F, REG_0184_MODV12_REG_VBY1_ALN_PIX_CNT);
		//enable vby1 hw training.
		W2BYTEMSK(REG_0180_MODV12, 0xAAE, Fld(16, 0, AC_FULLW10));
	}
	//video part. if use tcon bin, don't set outen here
	if (priv->linktype < E_LINK_MINILVDS_2BLK_6PAIR_8BIT) {

		switch (priv->out_timing) {
		case E_HD_60HZ:
			{
				//temp test, use mask [15:0] instead of CODA defined mask.
				if (priv->linktype == E_LINK_LVDS) {
					// ch2 - ch13
					W2BYTEMSK(REG_0060_MODA1, en ? 0x5550 : 0x0, Fld(16, 0, AC_FULLW10));
					W2BYTEMSK(REG_0064_MODA1, en ? 0x0555 : 0x0, Fld(16, 0, AC_FULLW10));
				} else {
					W2BYTEMSK(REG_0060_MODA1, en ? 0x0440 : 0x0, Fld(16, 0, AC_FULLW10));
					W2BYTEMSK(REG_0064_MODA1, 0x0, Fld(16, 0, AC_FULLW10));
				}
			}
			break;
		case E_FHD_60HZ:
		case E_HD_120HZ:
			{
				//temp test, use mask [15:0] instead of CODA defined mask.
				if (priv->linktype == E_LINK_LVDS) {
					// ch2 - ch13
					W2BYTEMSK(REG_0060_MODA1, en ? 0x5550 : 0x0,
						  Fld(16, 0, AC_FULLW10));
					W2BYTEMSK(REG_0064_MODA1, en ? 0x0555 : 0x0,
						  Fld(16, 0, AC_FULLW10));
				} else {
					W2BYTEMSK(REG_0060_MODA1, en ? 0x0440 : 0x0,
						  Fld(16, 0, AC_FULLW10));
					W2BYTEMSK(REG_0064_MODA1, 0x0, Fld(16, 0, AC_FULLW10));
				}
			}
			break;
		case E_FHD_120HZ:
			{
				for (u8laneCount = E_BOOT_PNL_OUTPUT_LANE0;
				     u8laneCount < E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
					if ((priv->lane_info.def_layout[u8laneCount] <
					     E_BOOT_PNL_OUTPUT_LANE4) && (en == true))
						bOutputConfigEn[u8laneCount] = true;
					else
						bOutputConfigEn[u8laneCount] = false;
				}

				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE0],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH00);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE1],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH01);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE2],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH02);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE3],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH03);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE4],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH04);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE5],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH05);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE6],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH06);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE7],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH07);

				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE8],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH08);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE9],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH09);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE10],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH10);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE11],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH11);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE12],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH12);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE13],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH13);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE14],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH14);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE15],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH15);
			}
			break;
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
			{
				if (priv->lane_duplicate_en)
					eOutputLaneEnNumber = E_BOOT_PNL_OUTPUT_LANE16;
				else
					eOutputLaneEnNumber = E_BOOT_PNL_OUTPUT_LANE8;

				for (u8laneCount = E_BOOT_PNL_OUTPUT_LANE0;
				     u8laneCount < E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
					if ((priv->lane_info.def_layout[u8laneCount] <
					     eOutputLaneEnNumber) && (en == true))
						bOutputConfigEn[u8laneCount] = true;
					else
						bOutputConfigEn[u8laneCount] = false;
				}

				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE0],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH00);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE1],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH01);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE2],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH02);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE3],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH03);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE4],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH04);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE5],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH05);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE6],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH06);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE7],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH07);

				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE8],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH08);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE9],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH09);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE10],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH10);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE11],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH11);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE12],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH12);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE13],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH13);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE14],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH14);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE15],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH15);
			}
			break;
		case E_4K1K_144HZ:
			{
				for (u8laneCount = E_BOOT_PNL_OUTPUT_LANE0;
				     u8laneCount < E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
					if ((priv->lane_info.def_layout[u8laneCount] <
					     E_BOOT_PNL_OUTPUT_LANE8) && (en == true))
						bOutputConfigEn[u8laneCount] = true;
					else
						bOutputConfigEn[u8laneCount] = false;
				}
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE0],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH00);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE1],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH01);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE2],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH02);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE3],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH03);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE4],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH04);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE5],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH05);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE6],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH06);
				W2BYTEMSK(REG_0060_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE7],
					  REG_0060_MODA1_REG_OUTPUT_CONF_CH07);

				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE8],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH08);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE9],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH09);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE10],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH10);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE11],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH11);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE12],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH12);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE13],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH13);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE14],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH14);
				W2BYTEMSK(REG_0064_MODA1, bOutputConfigEn[E_BOOT_PNL_OUTPUT_LANE15],
					  REG_0064_MODA1_REG_OUTPUT_CONF_CH15);
			}
			break;
		default:
			debug("[%s][%d] OUTPUT TIMING Not Support\n", __func__, __LINE__);
			break;
		}

#if (VBY1_LOCK_PROTECT_EN)
		if (en == true)
			_mtk_pnl_checkLockProtect_v006();
#endif
	}
}

void mtk_pnl_mute_en_v006(struct udevice *dev, bool en)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv) {
		printf("%s: priv is NULL\n", __func__);
		return;
	}

	UBOOT_TRACE("(%s): panel version: %d\n",
			en ? "MUTE" : "UNMUTE",
			priv->pnl_lib_version);

	// MOD pattern
	if (en) {
		W2BYTEMSK(REG_0198_MODV11_V005, 0x00, REG_0198_MODV11_V005_REG_TEST_COLOR_R_0198);
		W2BYTEMSK(REG_019C_MODV11_V005, 0x00, REG_019C_MODV11_V005_REG_TEST_COLOR_G_019C);
		W2BYTEMSK(REG_01A0_MODV11_V005, 0x00, REG_01A0_MODV11_V005_REG_TEST_COLOR_B_01A0);

		W2BYTEMSK(REG_0184_MODV11_V005, 0xFFFF, REG_0184_MODV11_V005_REG_TEST_H_LINE_ST_0184);
		W2BYTEMSK(REG_0188_MODV11_V005, 0xFFFF, REG_0188_MODV11_V005_REG_TEST_H_LINE_END_0188);
		W2BYTEMSK(REG_0180_MODV11_V005, 0x01, REG_0180_MODV11_V005_REG_TEST_H_LINE_MODE_0180);
		mdelay(10);
		W2BYTEMSK(REG_0180_MODV11_V005, 0x01, REG_0180_MODV11_V005_REG_TEST_MODE_0180);
	} else {
		W2BYTEMSK(REG_0180_MODV11_V005, 0x00, REG_0180_MODV11_V005_REG_TEST_MODE_0180);
	}
}

void mtk_pnl_set_vby1_mft_hmirror_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv->cus_info.hmirror_en == true) {
		W2BYTEMSK(REG_0004_MODV11, 1, REG_0004_MODV11_REG_MFT_H_MIRROR);
		W2BYTEMSK(REG_0004_MODV21, 1, REG_0004_MODV21_REG_MFT_H_MIRROR);
		W2BYTEMSK(REG_0004_MODOSD1, 1, REG_0004_MODOSD1_REG_MFT_H_MIRROR);
	} else {
		W2BYTEMSK(REG_0004_MODV11, 0, REG_0004_MODV11_REG_MFT_H_MIRROR);
		W2BYTEMSK(REG_0004_MODV21, 0, REG_0004_MODV21_REG_MFT_H_MIRROR);
		W2BYTEMSK(REG_0004_MODOSD1, 0, REG_0004_MODOSD1_REG_MFT_H_MIRROR);
	}
}

void vby1_set_pn_swap_v006(uint32_t pnl_lib_version)
{
	W2BYTEMSK(REG_0008_MODA1, 0xFFF, REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
}

void mtk_swing_level_setting_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	if ((priv->linktype == E_LINK_VB1) && (priv->pnl_lib_version == BOOT_PNL_VERSION0100))
		vby1_set_pn_swap_v006(priv->pnl_lib_version);	//PN swap setting

	if ((priv->swing_info.usr_swing_level != 0) && (priv->linktype == E_LINK_VB1)) {
		//usr_swing_level : True , 0 is flase , >= 1 is True
		uint16_t swing_val = 0;

		if (priv->swing_info.common_swing != 0) {
			//common_swing: True , 0 is flase , >= 1 is True
			swing_val = priv->swing_info.swing_level[0] & 0xF;
			swing_val = swing_val << 12 | swing_val << 8 | swing_val << 4 | swing_val;

			UBOOT_DEBUG("[pnl]common swing value = %d\n", swing_val);

			W2BYTEMSK(REG_0140_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch3
			W2BYTEMSK(REG_0140_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch3
			W2BYTEMSK(REG_0144_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch4-ch7
			W2BYTEMSK(REG_0148_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch8-ch11
			W2BYTEMSK(REG_014C_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch12-ch15
			W2BYTEMSK(REG_0150_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch16-ch19
			W2BYTEMSK(REG_0140_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch20-ch23
			W2BYTEMSK(REG_0144_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch24-ch27
			W2BYTEMSK(REG_0148_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch28-ch31
			W2BYTEMSK(REG_014C_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch32-ch35
			W2BYTEMSK(REG_0150_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch36-ch39
			W2BYTEMSK(REG_0140_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch40-ch43
			W2BYTEMSK(REG_0144_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch44-ch47
			W2BYTEMSK(REG_0148_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch48-ch51
			W2BYTEMSK(REG_014C_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch52-ch55
			W2BYTEMSK(REG_0150_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch56-ch59
			W2BYTEMSK(REG_0140_MODA4, swing_val, Fld(16, 0, AC_FULLW10));	//ch60-ch63
		} else {
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			    priv->pnl_lib_version == BOOT_PNL_VERSION0200) {
				swing_val = (priv->swing_info.swing_level[3] & 0xF) << 12 |
				    (priv->swing_info.swing_level[2] & 0xF) << 8 |
				    (priv->swing_info.swing_level[1] & 0xF) << 4 |
				    (priv->swing_info.swing_level[0] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch3~0 = %d\n", swing_val);
				W2BYTEMSK(REG_0140_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch3

				swing_val = (priv->swing_info.swing_level[7] & 0xF) << 12 |
				    (priv->swing_info.swing_level[6] & 0xF) << 8 |
				    (priv->swing_info.swing_level[5] & 0xF) << 4 |
				    (priv->swing_info.swing_level[4] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch7~4 = %d\n", swing_val);
				W2BYTEMSK(REG_0144_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch4-ch7

				swing_val = (priv->swing_info.swing_level[11] & 0xF) << 12 |
				    (priv->swing_info.swing_level[10] & 0xF) << 8 |
				    (priv->swing_info.swing_level[9] & 0xF) << 4 |
				    (priv->swing_info.swing_level[8] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch11~8 = %d\n", swing_val);
				W2BYTEMSK(REG_0148_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch8-ch11

				swing_val = (priv->swing_info.swing_level[15] & 0xF) << 12 |
				    (priv->swing_info.swing_level[14] & 0xF) << 8 |
				    (priv->swing_info.swing_level[13] & 0xF) << 4 |
				    (priv->swing_info.swing_level[12] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch15~12 = %d\n", swing_val);
				W2BYTEMSK(REG_014C_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch12-ch15

				swing_val = (priv->swing_info.swing_level[19] & 0xF) << 12 |
				    (priv->swing_info.swing_level[18] & 0xF) << 8 |
				    (priv->swing_info.swing_level[17] & 0xF) << 4 |
				    (priv->swing_info.swing_level[16] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch35~32 = %d\n", swing_val);
				W2BYTEMSK(REG_014C_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch32-ch35 GFX

				swing_val = (priv->swing_info.swing_level[23] & 0xF) << 12 |
				    (priv->swing_info.swing_level[22] & 0xF) << 8 |
				    (priv->swing_info.swing_level[21] & 0xF) << 4 |
				    (priv->swing_info.swing_level[20] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch39~36 = %d\n", swing_val);
				W2BYTEMSK(REG_0150_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch36-ch39 GFX
			} else {
				swing_val = (priv->swing_info.swing_level[3] & 0xF) << 12 |
				    (priv->swing_info.swing_level[2] & 0xF) << 8 |
				    (priv->swing_info.swing_level[1] & 0xF) << 4 |
				    (priv->swing_info.swing_level[0] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch3~0 = %d\n", swing_val);
				W2BYTEMSK(REG_0140_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch3

				swing_val = (priv->swing_info.swing_level[7] & 0xF) << 12 |
				    (priv->swing_info.swing_level[6] & 0xF) << 8 |
				    (priv->swing_info.swing_level[5] & 0xF) << 4 |
				    (priv->swing_info.swing_level[4] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch7~4 = %d\n", swing_val);
				W2BYTEMSK(REG_0144_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch4-ch7

				swing_val = (priv->swing_info.swing_level[11] & 0xF) << 12 |
				    (priv->swing_info.swing_level[10] & 0xF) << 8 |
				    (priv->swing_info.swing_level[9] & 0xF) << 4 |
				    (priv->swing_info.swing_level[8] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch11~8 = %d\n", swing_val);
				W2BYTEMSK(REG_0148_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch8-ch11

				swing_val = (priv->swing_info.swing_level[15] & 0xF) << 12 |
				    (priv->swing_info.swing_level[14] & 0xF) << 8 |
				    (priv->swing_info.swing_level[13] & 0xF) << 4 |
				    (priv->swing_info.swing_level[12] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch15~12 = %d\n", swing_val);
				W2BYTEMSK(REG_014C_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch12-ch15

				swing_val = (priv->swing_info.swing_level[19] & 0xF) << 12 |
				    (priv->swing_info.swing_level[18] & 0xF) << 8 |
				    (priv->swing_info.swing_level[17] & 0xF) << 4 |
				    (priv->swing_info.swing_level[16] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch19~16 = %d\n", swing_val);
				W2BYTEMSK(REG_0150_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch16-ch19

				swing_val = (priv->swing_info.swing_level[23] & 0xF) << 12 |
				    (priv->swing_info.swing_level[22] & 0xF) << 8 |
				    (priv->swing_info.swing_level[21] & 0xF) << 4 |
				    (priv->swing_info.swing_level[20] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch23~20 = %d\n", swing_val);
				W2BYTEMSK(REG_0140_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch20-ch23

				swing_val = (priv->swing_info.swing_level[27] & 0xF) << 12 |
				    (priv->swing_info.swing_level[26] & 0xF) << 8 |
				    (priv->swing_info.swing_level[25] & 0xF) << 4 |
				    (priv->swing_info.swing_level[24] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch27~24 = %d\n", swing_val);
				W2BYTEMSK(REG_0144_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch24-ch27

				swing_val = (priv->swing_info.swing_level[31] & 0xF) << 12 |
				    (priv->swing_info.swing_level[30] & 0xF) << 8 |
				    (priv->swing_info.swing_level[29] & 0xF) << 4 |
				    (priv->swing_info.swing_level[28] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch31~28 = %d\n", swing_val);
				W2BYTEMSK(REG_0148_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch28-ch31

				swing_val = (priv->swing_info.swing_level[35] & 0xF) << 12 |
				    (priv->swing_info.swing_level[34] & 0xF) << 8 |
				    (priv->swing_info.swing_level[33] & 0xF) << 4 |
				    (priv->swing_info.swing_level[32] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch35~32 = %d\n", swing_val);
				W2BYTEMSK(REG_014C_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch32-ch35

				swing_val = (priv->swing_info.swing_level[39] & 0xF) << 12 |
				    (priv->swing_info.swing_level[38] & 0xF) << 8 |
				    (priv->swing_info.swing_level[37] & 0xF) << 4 |
				    (priv->swing_info.swing_level[36] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch39~36 = %d\n", swing_val);
				W2BYTEMSK(REG_0150_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch36-ch39
				swing_val = (priv->swing_info.swing_level[43] & 0xF) << 12 |
				    (priv->swing_info.swing_level[42] & 0xF) << 8 |
				    (priv->swing_info.swing_level[41] & 0xF) << 4 |
				    (priv->swing_info.swing_level[40] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch43~40 = %d\n", swing_val);
				W2BYTEMSK(REG_0140_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch40-ch43
				swing_val = (priv->swing_info.swing_level[47] & 0xF) << 12 |
				    (priv->swing_info.swing_level[46] & 0xF) << 8 |
				    (priv->swing_info.swing_level[45] & 0xF) << 4 |
				    (priv->swing_info.swing_level[44] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch47~44 = %d\n", swing_val);
				W2BYTEMSK(REG_0144_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch44-ch47
				swing_val = (priv->swing_info.swing_level[51] & 0xF) << 12 |
				    (priv->swing_info.swing_level[50] & 0xF) << 8 |
				    (priv->swing_info.swing_level[49] & 0xF) << 4 |
				    (priv->swing_info.swing_level[48] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch51~48 = %d\n", swing_val);
				W2BYTEMSK(REG_0148_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch48-ch51
				swing_val = (priv->swing_info.swing_level[55] & 0xF) << 12 |
				    (priv->swing_info.swing_level[54] & 0xF) << 8 |
				    (priv->swing_info.swing_level[53] & 0xF) << 4 |
				    (priv->swing_info.swing_level[52] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch55~52 = %d\n", swing_val);
				W2BYTEMSK(REG_014C_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch52-ch55
				swing_val = (priv->swing_info.swing_level[59] & 0xF) << 12 |
				    (priv->swing_info.swing_level[58] & 0xF) << 8 |
				    (priv->swing_info.swing_level[57] & 0xF) << 4 |
				    (priv->swing_info.swing_level[56] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch59~56 = %d\n", swing_val);
				W2BYTEMSK(REG_0150_MODA3, swing_val, Fld(16, 0, AC_FULLW10));	//ch56-ch59
				swing_val = (priv->swing_info.swing_level[63] & 0xF) << 12 |
				    (priv->swing_info.swing_level[62] & 0xF) << 8 |
				    (priv->swing_info.swing_level[61] & 0xF) << 4 |
				    (priv->swing_info.swing_level[60] & 0xF);
				UBOOT_DEBUG("[pnl]swing value ch63~60 = %d\n", swing_val);
				W2BYTEMSK(REG_0140_MODA4, swing_val, Fld(16, 0, AC_FULLW10));	//ch60-ch63


			}
		}
	}

	if ((priv->swing_info.usr_swing_level != 0) && (priv->linktype == E_LINK_LVDS)) {

		uint16_t swing_val = 0;

		if (priv->swing_info.common_swing != 0) {
			//common_swing : True , 0 is flase , >= 1 is True
			swing_val = priv->swing_info.swing_level[0] & 0xFF;
			swing_val = swing_val << 8 | swing_val;

			UBOOT_DEBUG("[pnl]LVDS icon common swing value= %d\n", swing_val);

			W2BYTEMSK(REG_00B0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch1
			W2BYTEMSK(REG_00B4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch2-ch3
			W2BYTEMSK(REG_00B8_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch4-ch5
			W2BYTEMSK(REG_00BC_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch6-ch7
			W2BYTEMSK(REG_00C0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch8-ch9
			W2BYTEMSK(REG_00C4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch10-ch11
			W2BYTEMSK(REG_00C8_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch12-ch13
			W2BYTEMSK(REG_00CC_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch14-ch15
			W2BYTEMSK(REG_00D0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch16-ch17
			W2BYTEMSK(REG_00D4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch18-ch19

			W2BYTEMSK(REG_00B0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch20-ch21
			W2BYTEMSK(REG_00B4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch22-ch23
			W2BYTEMSK(REG_00B8_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch24-ch25
			W2BYTEMSK(REG_00BC_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch26-ch27
			W2BYTEMSK(REG_00C0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch28-ch29
			W2BYTEMSK(REG_00C4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch30-ch31
			W2BYTEMSK(REG_00C8_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch32-ch33
			W2BYTEMSK(REG_00CC_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch34-ch35
			W2BYTEMSK(REG_00D0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch36-ch37
			W2BYTEMSK(REG_00D4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch38-ch39
		} else {

			swing_val = (priv->swing_info.swing_level[1] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[0] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch1~0 = %d\n", swing_val);
			W2BYTEMSK(REG_00B0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch0-ch1

			swing_val = (priv->swing_info.swing_level[3] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[2] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch3~2 = %d\n", swing_val);
			W2BYTEMSK(REG_00B4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch2-ch3

			swing_val = (priv->swing_info.swing_level[5] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[4] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch5~4 = %d\n", swing_val);
			W2BYTEMSK(REG_00B8_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch4-ch5

			swing_val = (priv->swing_info.swing_level[7] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[6] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch7~6 = %d\n", swing_val);
			W2BYTEMSK(REG_00BC_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch6-ch7

			swing_val = (priv->swing_info.swing_level[9] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[8] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch9~8 = %d\n", swing_val);
			W2BYTEMSK(REG_00C0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch8-ch9

			swing_val = (priv->swing_info.swing_level[11] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[10] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch11~10 = %d\n", swing_val);
			W2BYTEMSK(REG_00C4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch10-ch11

			swing_val = (priv->swing_info.swing_level[13] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[12] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch13~12 = %d\n", swing_val);
			W2BYTEMSK(REG_00C8_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch12-ch13

			swing_val = (priv->swing_info.swing_level[15] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[14] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch15~14 = %d\n", swing_val);
			W2BYTEMSK(REG_00CC_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch14-ch15

			swing_val = (priv->swing_info.swing_level[17] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[16] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch17~16 = %d\n", swing_val);
			W2BYTEMSK(REG_00D0_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch16-ch17

			swing_val = (priv->swing_info.swing_level[19] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[18] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch19~18 = %d\n", swing_val);
			W2BYTEMSK(REG_00D4_MODA1, swing_val, Fld(16, 0, AC_FULLW10));	//ch18-ch19

			swing_val = (priv->swing_info.swing_level[21] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[20] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch21~20 = %d\n", swing_val);
			W2BYTEMSK(REG_00B0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch20-ch21

			swing_val = (priv->swing_info.swing_level[23] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[22] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch23~22 = %d\n", swing_val);
			W2BYTEMSK(REG_00B4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch22-ch23

			swing_val = (priv->swing_info.swing_level[25] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[24] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch25~24 = %d\n", swing_val);
			W2BYTEMSK(REG_00B8_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch24-ch25

			swing_val = (priv->swing_info.swing_level[27] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[26] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch27~26 = %d\n", swing_val);
			W2BYTEMSK(REG_00BC_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch26-ch27

			swing_val = (priv->swing_info.swing_level[29] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[28] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch29~28 = %d\n", swing_val);
			W2BYTEMSK(REG_00C0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch28-ch29

			swing_val = (priv->swing_info.swing_level[31] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[30] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch31~30 = %d\n", swing_val);
			W2BYTEMSK(REG_00C4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch30-ch31

			swing_val = (priv->swing_info.swing_level[33] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[32] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch33~32 = %d\n", swing_val);
			W2BYTEMSK(REG_00C8_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch32-ch33

			swing_val = (priv->swing_info.swing_level[35] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[34] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch35~34 = %d\n", swing_val);
			W2BYTEMSK(REG_00CC_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch34-ch35

			swing_val = (priv->swing_info.swing_level[37] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[36] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch37~36 = %d\n", swing_val);
			W2BYTEMSK(REG_00D0_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch36-ch37

			swing_val = (priv->swing_info.swing_level[39] & 0xFF) << 8 |
			    (priv->swing_info.swing_level[38] & 0xFF);
			UBOOT_DEBUG("[pnl]swing value ch39~38 = %d\n", swing_val);
			W2BYTEMSK(REG_00D4_MODA2, swing_val, Fld(16, 0, AC_FULLW10));	//ch38-ch39

		}
	}
}

static void _setAll_pe_value_v006(uint16_t value)
{
	value = value & 0xFFFF;

	W2BYTEMSK(REG_0080_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch0-ch1
	W2BYTEMSK(REG_0084_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch2-ch3
	W2BYTEMSK(REG_0088_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch4-ch5
	W2BYTEMSK(REG_008C_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch6-ch7
	W2BYTEMSK(REG_0090_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch8-ch9
	W2BYTEMSK(REG_0094_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch10-ch11
	W2BYTEMSK(REG_0098_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch12-ch13
	W2BYTEMSK(REG_009C_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch14-ch15
	W2BYTEMSK(REG_00A0_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch16-ch17
	W2BYTEMSK(REG_00A4_MODA1, value, Fld(16, 0, AC_FULLW10));	//ch18-ch19
	W2BYTEMSK(REG_0080_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch20-ch21
	W2BYTEMSK(REG_0084_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch22-ch23
	W2BYTEMSK(REG_0088_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch24-ch25
	W2BYTEMSK(REG_008C_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch26-ch27
	W2BYTEMSK(REG_0090_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch28-ch29
	W2BYTEMSK(REG_0094_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch30-ch31

	W2BYTEMSK(REG_0098_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch32-ch33
	W2BYTEMSK(REG_009C_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch34-ch35
	W2BYTEMSK(REG_00A0_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch36-ch37
	W2BYTEMSK(REG_00A4_MODA2, value, Fld(16, 0, AC_FULLW10));	//ch38-ch39

	//W2BYTEMSK(REG_0080_MODA3, value, Fld(16,0,AC_FULLW10));//ch40-ch41
	//W2BYTEMSK(REG_0084_MODA3, value, Fld(16,0,AC_FULLW10));//ch42-ch43
	//W2BYTEMSK(REG_0088_MODA3, value, Fld(16,0,AC_FULLW10));//ch44-ch45
	//W2BYTEMSK(REG_008C_MODA3, value, Fld(16,0,AC_FULLW10));//ch46-ch47
	//W2BYTEMSK(REG_0090_MODA3, value, Fld(16,0,AC_FULLW10));//ch48-ch49
	//W2BYTEMSK(REG_0094_MODA3, value, Fld(16,0,AC_FULLW10));//ch50-ch51
	//W2BYTEMSK(REG_0098_MODA3, value, Fld(16,0,AC_FULLW10));//ch52-ch53
	//W2BYTEMSK(REG_009C_MODA3, value, Fld(16,0,AC_FULLW10));//ch54-ch55
	//W2BYTEMSK(REG_00A0_MODA3, value, Fld(16,0,AC_FULLW10));//ch56-ch57
	//W2BYTEMSK(REG_00A4_MODA3, value, Fld(16,0,AC_FULLW10));//ch58-ch59
	//W2BYTEMSK(REG_0080_MODA4, value, Fld(16,0,AC_FULLW10));//ch60-ch61
	//W2BYTEMSK(REG_0084_MODA4, value, Fld(16,0,AC_FULLW10));//ch62-ch63
}

void mtk_pre_emphasis_setting_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	//PE enable for all channels.
	W2BYTEMSK(REG_0010_MODA1, 0xFFFF, REG_0010_MODA1_REG_GCR_PE_EN_CH_19_00_0);
	W2BYTEMSK(REG_0014_MODA1, 0xF, REG_0014_MODA1_REG_GCR_PE_EN_CH_19_00_1);
	W2BYTEMSK(REG_0010_MODA2, 0xFFFF, REG_0010_MODA2_REG_GCR_PE_EN_CH_39_20_0);
	W2BYTEMSK(REG_0014_MODA2, 0xF, REG_0014_MODA2_REG_GCR_PE_EN_CH_39_20_1);
	W2BYTEMSK(REG_0010_MODA3, 0xFFFF, REG_0010_MODA3_REG_GCR_PE_EN_CH_59_40_0);
	W2BYTEMSK(REG_0014_MODA3, 0xF, REG_0014_MODA3_REG_GCR_PE_EN_CH_59_40_1);
	W2BYTEMSK(REG_000C_MODA4, 0xF, REG_000C_MODA4_REG_GCR_PE_EN_CH_63_60);


	if (priv->pe_info.pe_usr != 0) {
		if (priv->pe_info.common_pe != 0) {
			//pe_usr:True and common_pe: True , 0 is flase , >= 1 is True
			_setAll_pe_value_v006((priv->pe_info.pe_level[0] << 8 | priv->pe_info.
					       pe_level[0]));
		} else {
			switch (priv->pnl_lib_version) {
			case BOOT_PNL_VERSION0200:	//0x2
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[0],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH00_0080);
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[1],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH01_0080);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[2],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH02_0084);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[3],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH03_0084);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[4],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH04_0088);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[5],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH05_0088);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[6],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH06_008C);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[7],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH07_008C);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[8],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH08_0090);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[9],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH09_0090);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[10],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH10_0094);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[11],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH11_0094);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[12],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH12_0098);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[13],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH13_0098);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[14],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH14_009C);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[15],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH15_009C);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[16],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH32_0098);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[17],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH33_0098);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[18],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH34_009C);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[19],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH35_009C);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[20],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH36_00A0);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[21],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH37_00A0);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[22],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH38_00A4);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[23],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH39_00A4);
				break;
			case BOOT_PNL_VERSION0203:	//0x0203
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[0],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH00_0080);
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[1],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH01_0080);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[2],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH02_0084);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[3],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH03_0084);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[4],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH04_0088);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[5],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH05_0088);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[6],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH06_008C);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[7],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH07_008C);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[8],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH08_0090);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[9],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH09_0090);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[10],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH10_0094);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[11],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH11_0094);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[12],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH12_0098);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[13],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH13_0098);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[14],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH14_009C);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[15],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH15_009C);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[16],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH32_0098);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[17],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH33_0098);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[18],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH34_009C);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[19],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH35_009C);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[20],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH36_00A0);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[21],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH37_00A0);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[22],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH38_00A4);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[23],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH39_00A4);
				break;
			case BOOT_PNL_VERSION0400:
			case BOOT_PNL_VERSION0600:
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[0],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH00_0080);
				W2BYTEMSK(REG_0080_MODA1_V004, priv->pe_info.pe_level[1],
					  REG_0080_MODA1_V004_REG_GCR_PE_ADJ_CH01_0080);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[2],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH02_0084);
				W2BYTEMSK(REG_0084_MODA1_V004, priv->pe_info.pe_level[3],
					  REG_0084_MODA1_V004_REG_GCR_PE_ADJ_CH03_0084);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[4],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH04_0088);
				W2BYTEMSK(REG_0088_MODA1_V004, priv->pe_info.pe_level[5],
					  REG_0088_MODA1_V004_REG_GCR_PE_ADJ_CH05_0088);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[6],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH06_008C);
				W2BYTEMSK(REG_008C_MODA1_V004, priv->pe_info.pe_level[7],
					  REG_008C_MODA1_V004_REG_GCR_PE_ADJ_CH07_008C);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[8],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH08_0090);
				W2BYTEMSK(REG_0090_MODA1_V004, priv->pe_info.pe_level[9],
					  REG_0090_MODA1_V004_REG_GCR_PE_ADJ_CH09_0090);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[10],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH10_0094);
				W2BYTEMSK(REG_0094_MODA1_V004, priv->pe_info.pe_level[11],
					  REG_0094_MODA1_V004_REG_GCR_PE_ADJ_CH11_0094);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[12],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH12_0098);
				W2BYTEMSK(REG_0098_MODA1_V004, priv->pe_info.pe_level[13],
					  REG_0098_MODA1_V004_REG_GCR_PE_ADJ_CH13_0098);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[14],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH14_009C);
				W2BYTEMSK(REG_009C_MODA1_V004, priv->pe_info.pe_level[15],
					  REG_009C_MODA1_V004_REG_GCR_PE_ADJ_CH15_009C);
				W2BYTEMSK(REG_00A0_MODA1_V004, priv->pe_info.pe_level[16],
					  REG_00A0_MODA1_V004_REG_GCR_PE_ADJ_CH16_00A0);
				W2BYTEMSK(REG_00A0_MODA1_V004, priv->pe_info.pe_level[17],
					  REG_00A0_MODA1_V004_REG_GCR_PE_ADJ_CH17_00A0);
				W2BYTEMSK(REG_00A4_MODA1_V004, priv->pe_info.pe_level[18],
					  REG_00A4_MODA1_V004_REG_GCR_PE_ADJ_CH18_00A4);
				W2BYTEMSK(REG_00A4_MODA1_V004, priv->pe_info.pe_level[19],
					  REG_00A4_MODA1_V004_REG_GCR_PE_ADJ_CH19_00A4);
				W2BYTEMSK(REG_0080_MODA2_V004, priv->pe_info.pe_level[20],
					  REG_0080_MODA2_V004_REG_GCR_PE_ADJ_CH20_0080);
				W2BYTEMSK(REG_0080_MODA2_V004, priv->pe_info.pe_level[21],
					  REG_0080_MODA2_V004_REG_GCR_PE_ADJ_CH21_0080);
				W2BYTEMSK(REG_0084_MODA2_V004, priv->pe_info.pe_level[22],
					  REG_0084_MODA2_V004_REG_GCR_PE_ADJ_CH22_0084);
				W2BYTEMSK(REG_0084_MODA2_V004, priv->pe_info.pe_level[23],
					  REG_0084_MODA2_V004_REG_GCR_PE_ADJ_CH23_0084);
				W2BYTEMSK(REG_0088_MODA2_V004, priv->pe_info.pe_level[24],
					  REG_0088_MODA2_V004_REG_GCR_PE_ADJ_CH24_0088);
				W2BYTEMSK(REG_0088_MODA2_V004, priv->pe_info.pe_level[25],
					  REG_0088_MODA2_V004_REG_GCR_PE_ADJ_CH25_0088);
				W2BYTEMSK(REG_008C_MODA2_V004, priv->pe_info.pe_level[26],
					  REG_008C_MODA2_V004_REG_GCR_PE_ADJ_CH26_008C);
				W2BYTEMSK(REG_008C_MODA2_V004, priv->pe_info.pe_level[27],
					  REG_008C_MODA2_V004_REG_GCR_PE_ADJ_CH27_008C);
				W2BYTEMSK(REG_0090_MODA2_V004, priv->pe_info.pe_level[28],
					  REG_0090_MODA2_V004_REG_GCR_PE_ADJ_CH28_0090);
				W2BYTEMSK(REG_0090_MODA2_V004, priv->pe_info.pe_level[29],
					  REG_0090_MODA2_V004_REG_GCR_PE_ADJ_CH29_0090);
				W2BYTEMSK(REG_0094_MODA2_V004, priv->pe_info.pe_level[30],
					  REG_0094_MODA2_V004_REG_GCR_PE_ADJ_CH30_0094);
				W2BYTEMSK(REG_0094_MODA2_V004, priv->pe_info.pe_level[31],
					  REG_0094_MODA2_V004_REG_GCR_PE_ADJ_CH31_0094);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[32],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH32_0098);
				W2BYTEMSK(REG_0098_MODA2_V004, priv->pe_info.pe_level[33],
					  REG_0098_MODA2_V004_REG_GCR_PE_ADJ_CH33_0098);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[34],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH34_009C);
				W2BYTEMSK(REG_009C_MODA2_V004, priv->pe_info.pe_level[35],
					  REG_009C_MODA2_V004_REG_GCR_PE_ADJ_CH35_009C);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[36],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH36_00A0);
				W2BYTEMSK(REG_00A0_MODA2_V004, priv->pe_info.pe_level[37],
					  REG_00A0_MODA2_V004_REG_GCR_PE_ADJ_CH37_00A0);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[38],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH38_00A4);
				W2BYTEMSK(REG_00A4_MODA2_V004, priv->pe_info.pe_level[39],
					  REG_00A4_MODA2_V004_REG_GCR_PE_ADJ_CH39_00A4);
				//W2BYTEMSK(REG_0080_MODA3, priv->pe_info.pe_level[40], REG_0080_MODA3_REG_GCR_PE_ADJ_CH40);
				//W2BYTEMSK(REG_0080_MODA3, priv->pe_info.pe_level[41], REG_0080_MODA3_REG_GCR_PE_ADJ_CH41);
				//W2BYTEMSK(REG_0084_MODA3, priv->pe_info.pe_level[42], REG_0084_MODA3_REG_GCR_PE_ADJ_CH42);
				//W2BYTEMSK(REG_0084_MODA3, priv->pe_info.pe_level[43], REG_0084_MODA3_REG_GCR_PE_ADJ_CH43);
				//W2BYTEMSK(REG_0088_MODA3, priv->pe_info.pe_level[44], REG_0088_MODA3_REG_GCR_PE_ADJ_CH44);
				//W2BYTEMSK(REG_0088_MODA3, priv->pe_info.pe_level[45], REG_0088_MODA3_REG_GCR_PE_ADJ_CH45);
				//W2BYTEMSK(REG_008C_MODA3, priv->pe_info.pe_level[46], REG_008C_MODA3_REG_GCR_PE_ADJ_CH46);
				//W2BYTEMSK(REG_008C_MODA3, priv->pe_info.pe_level[47], REG_008C_MODA3_REG_GCR_PE_ADJ_CH47);
				//W2BYTEMSK(REG_0090_MODA3, priv->pe_info.pe_level[48], REG_0090_MODA3_REG_GCR_PE_ADJ_CH48);
				//W2BYTEMSK(REG_0090_MODA3, priv->pe_info.pe_level[49], REG_0090_MODA3_REG_GCR_PE_ADJ_CH49);
				//W2BYTEMSK(REG_0094_MODA3, priv->pe_info.pe_level[50], REG_0094_MODA3_REG_GCR_PE_ADJ_CH50);
				//W2BYTEMSK(REG_0094_MODA3, priv->pe_info.pe_level[51], REG_0094_MODA3_REG_GCR_PE_ADJ_CH51);
				//W2BYTEMSK(REG_0098_MODA3, priv->pe_info.pe_level[52], REG_0098_MODA3_REG_GCR_PE_ADJ_CH52);
				//W2BYTEMSK(REG_0098_MODA3, priv->pe_info.pe_level[53], REG_0098_MODA3_REG_GCR_PE_ADJ_CH53);
				//W2BYTEMSK(REG_009C_MODA3, priv->pe_info.pe_level[54], REG_009C_MODA3_REG_GCR_PE_ADJ_CH54);
				//W2BYTEMSK(REG_009C_MODA3, priv->pe_info.pe_level[55], REG_009C_MODA3_REG_GCR_PE_ADJ_CH55);
				//W2BYTEMSK(REG_00A0_MODA3, priv->pe_info.pe_level[56], REG_00A0_MODA3_REG_GCR_PE_ADJ_CH56);
				//W2BYTEMSK(REG_00A0_MODA3, priv->pe_info.pe_level[57], REG_00A0_MODA3_REG_GCR_PE_ADJ_CH57);
				//W2BYTEMSK(REG_00A4_MODA3, priv->pe_info.pe_level[58], REG_00A4_MODA3_REG_GCR_PE_ADJ_CH58);
				//W2BYTEMSK(REG_00A4_MODA3, priv->pe_info.pe_level[59], REG_00A4_MODA3_REG_GCR_PE_ADJ_CH59);
				//W2BYTEMSK(REG_0080_MODA4, priv->pe_info.pe_level[60], REG_0080_MODA4_REG_GCR_PE_ADJ_CH60);
				//W2BYTEMSK(REG_0080_MODA4, priv->pe_info.pe_level[61], REG_0080_MODA4_REG_GCR_PE_ADJ_CH61);
				//W2BYTEMSK(REG_0084_MODA4, priv->pe_info.pe_level[62], REG_0084_MODA4_REG_GCR_PE_ADJ_CH62);
				//W2BYTEMSK(REG_0084_MODA4, priv->pe_info.pe_level[63], REG_0084_MODA4_REG_GCR_PE_ADJ_CH63);
				break;

			case BOOT_PNL_VERSION0500:
				W2BYTEMSK(REG_0080_MODA1_V005, priv->pe_info.pe_level[0],
					  REG_0080_MODA1_V005_REG_GCR_PE_ADJ_CH00_0080);
				W2BYTEMSK(REG_0080_MODA1_V005, priv->pe_info.pe_level[1],
					  REG_0080_MODA1_V005_REG_GCR_PE_ADJ_CH01_0080);
				W2BYTEMSK(REG_0084_MODA1_V005, priv->pe_info.pe_level[2],
					  REG_0084_MODA1_V005_REG_GCR_PE_ADJ_CH02_0084);
				W2BYTEMSK(REG_0084_MODA1_V005, priv->pe_info.pe_level[3],
					  REG_0084_MODA1_V005_REG_GCR_PE_ADJ_CH03_0084);
				W2BYTEMSK(REG_0088_MODA1_V005, priv->pe_info.pe_level[4],
					  REG_0088_MODA1_V005_REG_GCR_PE_ADJ_CH04_0088);
				W2BYTEMSK(REG_0088_MODA1_V005, priv->pe_info.pe_level[5],
					  REG_0088_MODA1_V005_REG_GCR_PE_ADJ_CH05_0088);
				W2BYTEMSK(REG_008C_MODA1_V005, priv->pe_info.pe_level[6],
					  REG_008C_MODA1_V005_REG_GCR_PE_ADJ_CH06_008C);
				W2BYTEMSK(REG_008C_MODA1_V005, priv->pe_info.pe_level[7],
					  REG_008C_MODA1_V005_REG_GCR_PE_ADJ_CH07_008C);
				W2BYTEMSK(REG_0090_MODA1_V005, priv->pe_info.pe_level[8],
					  REG_0090_MODA1_V005_REG_GCR_PE_ADJ_CH08_0090);
				W2BYTEMSK(REG_0090_MODA1_V005, priv->pe_info.pe_level[9],
					  REG_0090_MODA1_V005_REG_GCR_PE_ADJ_CH09_0090);
				W2BYTEMSK(REG_0094_MODA1_V005, priv->pe_info.pe_level[10],
					  REG_0094_MODA1_V005_REG_GCR_PE_ADJ_CH10_0094);
				W2BYTEMSK(REG_0094_MODA1_V005, priv->pe_info.pe_level[11],
					  REG_0094_MODA1_V005_REG_GCR_PE_ADJ_CH11_0094);
				W2BYTEMSK(REG_0098_MODA1_V005, priv->pe_info.pe_level[12],
					  REG_0098_MODA1_V005_REG_GCR_PE_ADJ_CH12_0098);
				W2BYTEMSK(REG_0098_MODA1_V005, priv->pe_info.pe_level[13],
					  REG_0098_MODA1_V005_REG_GCR_PE_ADJ_CH13_0098);
				W2BYTEMSK(REG_009C_MODA1_V005, priv->pe_info.pe_level[14],
					  REG_009C_MODA1_V005_REG_GCR_PE_ADJ_CH14_009C);
				W2BYTEMSK(REG_009C_MODA1_V005, priv->pe_info.pe_level[15],
					  REG_009C_MODA1_V005_REG_GCR_PE_ADJ_CH15_009C);
				W2BYTEMSK(REG_00A0_MODA1_V005, priv->pe_info.pe_level[16],
					  REG_00A0_MODA1_V005_REG_GCR_PE_ADJ_CH16_00A0);
				W2BYTEMSK(REG_00A0_MODA1_V005, priv->pe_info.pe_level[17],
					  REG_00A0_MODA1_V005_REG_GCR_PE_ADJ_CH17_00A0);
				W2BYTEMSK(REG_00A4_MODA1_V005, priv->pe_info.pe_level[18],
					  REG_00A4_MODA1_V005_REG_GCR_PE_ADJ_CH18_00A4);
				W2BYTEMSK(REG_00A4_MODA1_V005, priv->pe_info.pe_level[19],
					  REG_00A4_MODA1_V005_REG_GCR_PE_ADJ_CH19_00A4);
				break;
			default:
				W2BYTEMSK(REG_0080_MODA1, priv->pe_info.pe_level[0],
					  REG_0080_MODA1_REG_GCR_PE_ADJ_CH00);
				W2BYTEMSK(REG_0080_MODA1, priv->pe_info.pe_level[1],
					  REG_0080_MODA1_REG_GCR_PE_ADJ_CH01);
				W2BYTEMSK(REG_0084_MODA1, priv->pe_info.pe_level[2],
					  REG_0084_MODA1_REG_GCR_PE_ADJ_CH02);
				W2BYTEMSK(REG_0084_MODA1, priv->pe_info.pe_level[3],
					  REG_0084_MODA1_REG_GCR_PE_ADJ_CH03);
				W2BYTEMSK(REG_0088_MODA1, priv->pe_info.pe_level[4],
					  REG_0088_MODA1_REG_GCR_PE_ADJ_CH04);
				W2BYTEMSK(REG_0088_MODA1, priv->pe_info.pe_level[5],
					  REG_0088_MODA1_REG_GCR_PE_ADJ_CH05);
				W2BYTEMSK(REG_008C_MODA1, priv->pe_info.pe_level[6],
					  REG_008C_MODA1_REG_GCR_PE_ADJ_CH06);
				W2BYTEMSK(REG_008C_MODA1, priv->pe_info.pe_level[7],
					  REG_008C_MODA1_REG_GCR_PE_ADJ_CH07);
				W2BYTEMSK(REG_0090_MODA1, priv->pe_info.pe_level[8],
					  REG_0090_MODA1_REG_GCR_PE_ADJ_CH08);
				W2BYTEMSK(REG_0090_MODA1, priv->pe_info.pe_level[9],
					  REG_0090_MODA1_REG_GCR_PE_ADJ_CH09);
				W2BYTEMSK(REG_0094_MODA1, priv->pe_info.pe_level[10],
					  REG_0094_MODA1_REG_GCR_PE_ADJ_CH10);
				W2BYTEMSK(REG_0094_MODA1, priv->pe_info.pe_level[11],
					  REG_0094_MODA1_REG_GCR_PE_ADJ_CH11);
				W2BYTEMSK(REG_0098_MODA1, priv->pe_info.pe_level[12],
					  REG_0098_MODA1_REG_GCR_PE_ADJ_CH12);
				W2BYTEMSK(REG_0098_MODA1, priv->pe_info.pe_level[13],
					  REG_0098_MODA1_REG_GCR_PE_ADJ_CH13);
				W2BYTEMSK(REG_009C_MODA1, priv->pe_info.pe_level[14],
					  REG_009C_MODA1_REG_GCR_PE_ADJ_CH14);
				W2BYTEMSK(REG_009C_MODA1, priv->pe_info.pe_level[15],
					  REG_009C_MODA1_REG_GCR_PE_ADJ_CH15);
				W2BYTEMSK(REG_00A0_MODA1, priv->pe_info.pe_level[16],
					  REG_00A0_MODA1_REG_GCR_PE_ADJ_CH16);
				W2BYTEMSK(REG_00A0_MODA1, priv->pe_info.pe_level[17],
					  REG_00A0_MODA1_REG_GCR_PE_ADJ_CH17);
				W2BYTEMSK(REG_00A4_MODA1, priv->pe_info.pe_level[18],
					  REG_00A4_MODA1_REG_GCR_PE_ADJ_CH18);
				W2BYTEMSK(REG_00A4_MODA1, priv->pe_info.pe_level[19],
					  REG_00A4_MODA1_REG_GCR_PE_ADJ_CH19);
				W2BYTEMSK(REG_0080_MODA2, priv->pe_info.pe_level[20],
					  REG_0080_MODA2_REG_GCR_PE_ADJ_CH20);
				W2BYTEMSK(REG_0080_MODA2, priv->pe_info.pe_level[21],
					  REG_0080_MODA2_REG_GCR_PE_ADJ_CH21);
				W2BYTEMSK(REG_0084_MODA2, priv->pe_info.pe_level[22],
					  REG_0084_MODA2_REG_GCR_PE_ADJ_CH22);
				W2BYTEMSK(REG_0084_MODA2, priv->pe_info.pe_level[23],
					  REG_0084_MODA2_REG_GCR_PE_ADJ_CH23);
				W2BYTEMSK(REG_0088_MODA2, priv->pe_info.pe_level[24],
					  REG_0088_MODA2_REG_GCR_PE_ADJ_CH24);
				W2BYTEMSK(REG_0088_MODA2, priv->pe_info.pe_level[25],
					  REG_0088_MODA2_REG_GCR_PE_ADJ_CH25);
				W2BYTEMSK(REG_008C_MODA2, priv->pe_info.pe_level[26],
					  REG_008C_MODA2_REG_GCR_PE_ADJ_CH26);
				W2BYTEMSK(REG_008C_MODA2, priv->pe_info.pe_level[27],
					  REG_008C_MODA2_REG_GCR_PE_ADJ_CH27);
				W2BYTEMSK(REG_0090_MODA2, priv->pe_info.pe_level[28],
					  REG_0090_MODA2_REG_GCR_PE_ADJ_CH28);
				W2BYTEMSK(REG_0090_MODA2, priv->pe_info.pe_level[29],
					  REG_0090_MODA2_REG_GCR_PE_ADJ_CH29);
				W2BYTEMSK(REG_0094_MODA2, priv->pe_info.pe_level[30],
					  REG_0094_MODA2_REG_GCR_PE_ADJ_CH30);
				W2BYTEMSK(REG_0094_MODA2, priv->pe_info.pe_level[31],
					  REG_0094_MODA2_REG_GCR_PE_ADJ_CH31);
				W2BYTEMSK(REG_0098_MODA2, priv->pe_info.pe_level[32],
					  REG_0098_MODA2_REG_GCR_PE_ADJ_CH32);
				W2BYTEMSK(REG_0098_MODA2, priv->pe_info.pe_level[33],
					  REG_0098_MODA2_REG_GCR_PE_ADJ_CH33);
				W2BYTEMSK(REG_009C_MODA2, priv->pe_info.pe_level[34],
					  REG_009C_MODA2_REG_GCR_PE_ADJ_CH34);
				W2BYTEMSK(REG_009C_MODA2, priv->pe_info.pe_level[35],
					  REG_009C_MODA2_REG_GCR_PE_ADJ_CH35);
				W2BYTEMSK(REG_00A0_MODA2, priv->pe_info.pe_level[36],
					  REG_00A0_MODA2_REG_GCR_PE_ADJ_CH36);
				W2BYTEMSK(REG_00A0_MODA2, priv->pe_info.pe_level[37],
					  REG_00A0_MODA2_REG_GCR_PE_ADJ_CH37);
				W2BYTEMSK(REG_00A4_MODA2, priv->pe_info.pe_level[38],
					  REG_00A4_MODA2_REG_GCR_PE_ADJ_CH38);
				W2BYTEMSK(REG_00A4_MODA2, priv->pe_info.pe_level[39],
					  REG_00A4_MODA2_REG_GCR_PE_ADJ_CH39);
				W2BYTEMSK(REG_0080_MODA3, priv->pe_info.pe_level[40],
					  REG_0080_MODA3_REG_GCR_PE_ADJ_CH40);
				W2BYTEMSK(REG_0080_MODA3, priv->pe_info.pe_level[41],
					  REG_0080_MODA3_REG_GCR_PE_ADJ_CH41);
				W2BYTEMSK(REG_0084_MODA3, priv->pe_info.pe_level[42],
					  REG_0084_MODA3_REG_GCR_PE_ADJ_CH42);
				W2BYTEMSK(REG_0084_MODA3, priv->pe_info.pe_level[43],
					  REG_0084_MODA3_REG_GCR_PE_ADJ_CH43);
				W2BYTEMSK(REG_0088_MODA3, priv->pe_info.pe_level[44],
					  REG_0088_MODA3_REG_GCR_PE_ADJ_CH44);
				W2BYTEMSK(REG_0088_MODA3, priv->pe_info.pe_level[45],
					  REG_0088_MODA3_REG_GCR_PE_ADJ_CH45);
				W2BYTEMSK(REG_008C_MODA3, priv->pe_info.pe_level[46],
					  REG_008C_MODA3_REG_GCR_PE_ADJ_CH46);
				W2BYTEMSK(REG_008C_MODA3, priv->pe_info.pe_level[47],
					  REG_008C_MODA3_REG_GCR_PE_ADJ_CH47);
				W2BYTEMSK(REG_0090_MODA3, priv->pe_info.pe_level[48],
					  REG_0090_MODA3_REG_GCR_PE_ADJ_CH48);
				W2BYTEMSK(REG_0090_MODA3, priv->pe_info.pe_level[49],
					  REG_0090_MODA3_REG_GCR_PE_ADJ_CH49);
				W2BYTEMSK(REG_0094_MODA3, priv->pe_info.pe_level[50],
					  REG_0094_MODA3_REG_GCR_PE_ADJ_CH50);
				W2BYTEMSK(REG_0094_MODA3, priv->pe_info.pe_level[51],
					  REG_0094_MODA3_REG_GCR_PE_ADJ_CH51);
				W2BYTEMSK(REG_0098_MODA3, priv->pe_info.pe_level[52],
					  REG_0098_MODA3_REG_GCR_PE_ADJ_CH52);
				W2BYTEMSK(REG_0098_MODA3, priv->pe_info.pe_level[53],
					  REG_0098_MODA3_REG_GCR_PE_ADJ_CH53);
				W2BYTEMSK(REG_009C_MODA3, priv->pe_info.pe_level[54],
					  REG_009C_MODA3_REG_GCR_PE_ADJ_CH54);
				W2BYTEMSK(REG_009C_MODA3, priv->pe_info.pe_level[55],
					  REG_009C_MODA3_REG_GCR_PE_ADJ_CH55);
				W2BYTEMSK(REG_00A0_MODA3, priv->pe_info.pe_level[56],
					  REG_00A0_MODA3_REG_GCR_PE_ADJ_CH56);
				W2BYTEMSK(REG_00A0_MODA3, priv->pe_info.pe_level[57],
					  REG_00A0_MODA3_REG_GCR_PE_ADJ_CH57);
				W2BYTEMSK(REG_00A4_MODA3, priv->pe_info.pe_level[58],
					  REG_00A4_MODA3_REG_GCR_PE_ADJ_CH58);
				W2BYTEMSK(REG_00A4_MODA3, priv->pe_info.pe_level[59],
					  REG_00A4_MODA3_REG_GCR_PE_ADJ_CH59);
				W2BYTEMSK(REG_0080_MODA4, priv->pe_info.pe_level[60],
					  REG_0080_MODA4_REG_GCR_PE_ADJ_CH60);
				W2BYTEMSK(REG_0080_MODA4, priv->pe_info.pe_level[61],
					  REG_0080_MODA4_REG_GCR_PE_ADJ_CH61);
				W2BYTEMSK(REG_0084_MODA4, priv->pe_info.pe_level[62],
					  REG_0084_MODA4_REG_GCR_PE_ADJ_CH62);
				W2BYTEMSK(REG_0084_MODA4, priv->pe_info.pe_level[63],
					  REG_0084_MODA4_REG_GCR_PE_ADJ_CH63);
				break;
			}
		}
	} else {
		//default value
		_setAll_pe_value_v006(0x1010);
	}
}

void mtk_SSC_control_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint64_t tmp = 0;
	uint64_t lpll_set = 0;
	uint32_t ssc_span = 0;
	uint32_t ssc_step = 0;
	const uint32_t Khz = 1000;
	const uint32_t Precision_unit = 10;
	uint32_t unit_align_vby1 = 1;
	st_out_ssc_ctrl stCtrl;

	if (!priv) {
		UBOOT_ERROR("get device private fail\n");
		return;
	}

	if (priv->tcon_info.bUsingTCON && priv->tcon_info.stPanelInfo.u32SSC_TconBinCtrl == 0) {
		unit_align_vby1 = 10;
		stCtrl.ssc_en = priv->tcon_info.stPanelInfo.u32SSCEnable;
		stCtrl.ssc_modulation = priv->tcon_info.stPanelInfo.u32SSC_Fmodulation;
		stCtrl.ssc_deviation =
		    priv->tcon_info.stPanelInfo.u32SSC_Percentage * unit_align_vby1;
		UBOOT_DEBUG("[%s,%5d] TCON control SSC value\n", __func__, __LINE__);
	} else {
		unit_align_vby1 = 1;
		stCtrl.ssc_en = priv->ssc_info.ssc_en;
		stCtrl.ssc_modulation = priv->ssc_info.ssc_modulation;
		stCtrl.ssc_deviation = priv->ssc_info.ssc_deviation;
	}

	lpll_set =
	    (uint64_t) (((uint64_t)
			 (R2BYTEMSK(REG_0040_LPLL, REG_0040_LPLL_REG_PLL_SET_1) & 0xFFFF)) << 16);
	lpll_set =
	    (uint64_t) (((uint64_t) (lpll_set)) |
			((R2BYTEMSK(REG_003C_LPLL, REG_003C_LPLL_REG_PLL_SET_0) & 0xFFFF)));
	UBOOT_DEBUG("\n  SSC LPLL set value : %x\n", (unsigned int)lpll_set);
	if (lpll_set == 0) {
		printf("[%s]LPLL_SET is 0. Check NG\n", __func__);
		return;
	}
	//Cal modulation (ssc span)
	if (stCtrl.ssc_modulation != 0) {
		tmp =
		    (uint64_t) ((uint64_t) MPLL_CLK_864MHZ * LPLL_SPAN_FACTOR_V6 *
				LPLL_CAL_PARAM);
		UBOOT_DEBUG("\n  SSC tmp : %llx\n", tmp);
		tmp = (uint64_t) ((uint64_t) (tmp) * Precision_unit);	//for precision
		debug("\n  SSC tmp : %llx\n", tmp);
		tmp = (uint64_t) ((uint64_t) (tmp) * Khz);
		debug("\n  SSC tmp : %llx\n", tmp);
		tmp = ((uint64_t) (tmp * unit_align_vby1) / (stCtrl.ssc_modulation));
		UBOOT_DEBUG("\n  SSC tmp : %llx\n", tmp);
		UBOOT_DEBUG("\n  ssc_info.ssc_modulation : %x\n", stCtrl.ssc_modulation);
		tmp = (uint64_t) ((uint64_t) (tmp) / lpll_set);
		debug("\n  SSC tmp : %llx\n", tmp);
		tmp = (uint32_t) (tmp / Precision_unit);
		ssc_span = (uint32_t) (tmp / SSC_SPAN_CAL_PARAM);
		UBOOT_DEBUG("\n  SSC Span value : %x\n", ssc_span);

		W2BYTEMSK(REG_0060_LPLL, (ssc_span & 0x3FFF), REG_0060_LPLL_REG_LPLL_SPAN);
	}
	//ssc step (deviation)
	if (ssc_span != 0) {
		tmp = (uint64_t) (lpll_set);
		debug("\n  SSC tmp : %llx\n", tmp);
		tmp = (uint64_t) ((uint64_t) (tmp) * stCtrl.ssc_deviation);
		UBOOT_DEBUG("\n  SSC tmp : %llx\n", tmp);
		UBOOT_DEBUG("\n  ssc_info.ssc_deviation : %x\n", stCtrl.ssc_deviation);
		tmp = (uint64_t) ((uint64_t) tmp / (Khz * Precision_unit * ssc_span));
		UBOOT_DEBUG("\n  SSC tmp : %llx, ssc_span :%x\n", tmp, ssc_span);
		ssc_step = (uint32_t) tmp;
		UBOOT_DEBUG("\n  SSC Step value : %x\n", ssc_step);

		W2BYTEMSK(REG_005C_LPLL, (ssc_step & 0xFFFF), REG_005C_LPLL_REG_LPLL_STEP);
	}
	//ssc enable
	W2BYTEMSK(REG_0034_LPLL, stCtrl.ssc_en, REG_0034_LPLL_REG_SSC_EN);
}

void mtk_pnl_controlbit_init_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	UBOOT_DEBUG("%s V006 not support\n", __func__);
	return;
}

void mtk_out_if_hbkproch_protect_init_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	UBOOT_DEBUG("%s: Ver%d not use hbkproch_protect\n", __func__,
		    priv->pnl_lib_version);
}

void mtk_pnl_set_panel_SCDISP_Path_Sel_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	UBOOT_DEBUG("%s: priv->cus_info.scdisp_path_sel=%d\n", __func__,
		    priv->cus_info.scdisp_path_sel);
	W2BYTEMSK(REG_0100_SCDISP_BKA4FA, priv->cus_info.scdisp_path_sel,
		  REG_0100_SCDISP_BKA4FA_REG_SCDISP_PATH_SEL);
}

static uint32_t _mtk_get_efuse_val_v006(uint32_t hwVersion, uint16_t subBank)
{
	uint32_t efuse_val = 0;
	uint8_t count = 0;

	W2BYTEMSK(EFUSE_BANK, (subBank << 2), Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(EFUSE_BANK, 1, Fld(1, 13, AC_MSKB1));

	while (R2BYTEMSK(EFUSE_BANK, Fld(1, 13, AC_MSKB1)) != 0) {
		mdelay(1);
		count++;
		if (count > TIME_OUT_CNT)
			break;
	}

	efuse_val =
	    (R2BYTEMSK(EFUSE_VAL_MSB, Fld(16, 0, AC_FULLW10)) << 16) | R2BYTEMSK(EFUSE_VAL_LSB,
										 Fld(16, 0,
										     AC_FULLW10));

	return efuse_val;
}

void efuse_set_lpllsetting_v006(uint32_t hwVersion)
{
	uint32_t efuse_sub0D_val = 0;

	if (hwVersion == BOOT_PNL_VERSION0203) {
		//sub bank 0D
		efuse_sub0D_val = _mtk_get_efuse_val_v006(hwVersion, MOD_ATOP_SUBBANK_0D);
		if ((efuse_sub0D_val & _BIT(LPLL_RETIME_CTRL)) == _BIT(LPLL_RETIME_CTRL)) {
			W2BYTEMSK(REG_00D8_LPLL, 1, Fld(1, 0, AC_MSKB0));
			W2BYTEMSK(REG_0110_LPLL, 1, Fld(1, 0, AC_MSKB0));
		} else {
			W2BYTEMSK(REG_00D8_LPLL, 0, Fld(1, 0, AC_MSKB0));
			W2BYTEMSK(REG_0110_LPLL, 0, Fld(1, 0, AC_MSKB0));
		}

		if ((efuse_sub0D_val & _BIT(LPLL_DELAY_TIME_CTRL)) == _BIT(LPLL_DELAY_TIME_CTRL)) {
			W2BYTEMSK(REG_00DC_LPLL, 1, Fld(1, 3, AC_MSKB0));
			W2BYTEMSK(REG_0114_LPLL, 1, Fld(1, 3, AC_MSKB0));
		} else {
			W2BYTEMSK(REG_00DC_LPLL, 0, Fld(1, 3, AC_MSKB0));
			W2BYTEMSK(REG_0114_LPLL, 0, Fld(1, 3, AC_MSKB0));
		}
	}
}

void _PATCH_INIT_XC_CLK_v006(void)
{
	u16 u16XCPLL_CLK = 0;
	//ckgen 00

	//ckgen 01
	W2BYTEMSK(REG_0B18_CKGEN01_V006, 0x1, REG_0B18_CKGEN01_V006_REG_CKG_S_XC_DBG2MIU_SCIP_0B18); // hdmi
	W2BYTEMSK(REG_1578_CKGEN01_V006, 0x1, REG_1578_CKGEN01_V006_REG_SW_EN_FRC_FCLK_2X2FRCCORE1_1578); //h55e[0]
	W2BYTEMSK(REG_1578_CKGEN01_V006, 0x1, REG_1578_CKGEN01_V006_REG_SW_EN_FRC_FCLK2FRCCORE1_1578); //h55e[9]
	W2BYTEMSK(REG_157C_CKGEN01_V006, 0x1, REG_157C_CKGEN01_V006_REG_SW_EN_FRC_FCLK_2XPLUS2FRCCORE1_157C); //h55f[0]
	W2BYTEMSK(REG_15B4_CKGEN01_V006, 0x1, REG_15B4_CKGEN01_V006_REG_SW_EN_XTAL_12M2FRCMISC_15B4); //h56d[0]
	W2BYTEMSK(REG_15C0_CKGEN01_V006, 0x1, REG_15C0_CKGEN01_V006_REG_SW_EN_XTAL_24M2FRCCORE1_15C0); //h570[0]
	W2BYTEMSK(REG_1994_CKGEN01_V006, 0x1, REG_1994_CKGEN01_V006_REG_SW_EN_XC_FN2XC_DC0_CG_SCIP_1994); //h665[13]

	W2BYTEMSK(REG_01E8_CKGEN01_V006, 0x0, REG_01E8_CKGEN01_V006_REG_CKG_S_FRC_FCLK_01E8); //h7a[3:2], frc fclk select
	W2BYTEMSK(REG_01F0_CKGEN01_V006, 0x1, REG_01F0_CKGEN01_V006_REG_CKG_S_FRC_FCLK_2X_01F0); //h7c[3:2], frc fclk_2x select
	W2BYTEMSK(REG_01F8_CKGEN01_V006, 0x1, REG_01F8_CKGEN01_V006_REG_CKG_S_FRC_FCLK_2XPLUS_01F8); //h7e[3:2], frc fclk_2xplus select
	W2BYTEMSK(REG_0AD0_CKGEN01_V006, 0x1, REG_0AD0_CKGEN01_V006_REG_CKG_S_XC_FN_0AD0); //h2b4[3:2], fn clk select
	W2BYTEMSK(REG_0AF0_CKGEN01_V006, 0x1, REG_0AF0_CKGEN01_V006_REG_CKG_S_XC_SRS_0AF0); //h2bc[2], srs clk select
	W2BYTEMSK(REG_0E84_CKGEN01_V006, 0x0, REG_0E84_CKGEN01_V006_REG_GATE_XC_FN_0E84); //h3a1[0], fn clk gate

	u16XCPLL_CLK = R2BYTEMSK(REG_0004_XCPLL_V006, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_0004)
					| (R2BYTEMSK(REG_0004_XCPLL_V006, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV1_0004) << 4)
					| (R2BYTEMSK(REG_0004_XCPLL_V006, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV2_0004) << 8);

	if (u16XCPLL_CLK != XCPLL_REG_0004_SET) {
		//XCPLL Setting
		W2BYTEMSK(REG_0000_XCPLL_V006, 0x3, REG_0000_XCPLL_V006_REG_XCPLL_V006_TEST_0000); //BK100D_00[15:0] = 0x03
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x0, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_0004); //BK100D_01[0] = b'0
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x1, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV1_0004); //BK100D_01[4] = b'1
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x1, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV2_0004); //BK100D_01[8] = b'1
		W2BYTEMSK(REG_0008_XCPLL_V006, 0x0, REG_0008_XCPLL_V006_REG_XCPLL_V006_RST_0008); //BK100D_02[0] = b'0
		W2BYTEMSK(REG_0008_XCPLL_V006, 0x0, REG_0008_XCPLL_V006_REG_XCPLL_V006_INPUT_DIV_FIRST_0008); //BK100D_02[5:4] = b'00
		W2BYTEMSK(REG_0008_XCPLL_V006, 0x1, REG_0008_XCPLL_V006_REG_XCPLL_V006_LOOP_DIV_FIRST_0008); //BK100D_02[9:8] = b'01
		W2BYTEMSK(REG_000C_XCPLL_V006, 0x4, REG_000C_XCPLL_V006_REG_XCPLL_V006_LOOP_DIV_SECOND_000C); //BK100D_03[7:0] = 0x04
		W2BYTEMSK(REG_000C_XCPLL_V006, 0x4, REG_000C_XCPLL_V006_REG_XCPLL_V006_OUTPUT_DIV_000C); //BK100D_03[10:8] = b'100
		W2BYTEMSK(REG_000C_XCPLL_V006, 0x0, REG_000C_XCPLL_V006_REG_XCPLL_V006_OUTPUT_DIV_FIRST_000C); //BK100D_03[13:12] = b'00
		//XCPLL SSC 1% 30khz
		W2BYTEMSK(REG_0014_XCPLL_V006, 0x2A1D, REG_0014_XCPLL_V006_REG_XCPLL_V006_SET_0_0014); //BK100D_05[15:0] = 0x2A1D
		W2BYTEMSK(REG_0018_XCPLL_V006, 0x16, REG_0018_XCPLL_V006_REG_XCPLL_V006_SET_1_0018); //BK100D_06[7:0] = 0x16
		W2BYTEMSK(REG_001C_XCPLL_V006, 0x000B, REG_001C_XCPLL_V006_REG_XCPLL_V006_STEP_0_001C); //BK100D_07[15:0] = 0x000B
		W2BYTEMSK(REG_0020_XCPLL_V006, 0x00, REG_0020_XCPLL_V006_REG_XCPLL_V006_STEP_1_0020); //BK100D_08[7:0] = 0x00
		W2BYTEMSK(REG_0024_XCPLL_V006, 0x0514, REG_0024_XCPLL_V006_REG_XCPLL_V006_SPAN_0_0024); //BK100D_09[15:0] = 0x0514
		W2BYTEMSK(REG_0028_XCPLL_V006, 0x00, REG_0028_XCPLL_V006_REG_XCPLL_V006_SPAN_1_0028); //BK100D_0A[7;0] = 0x00
		W2BYTEMSK(REG_0034_XCPLL_V006, 0x0, REG_0034_XCPLL_V006_REG_XCPLL_V006_SSC_SYNC_0034); //BK100D_0D[0] = b'0
		W2BYTEMSK(REG_0034_XCPLL_V006, 0x1, REG_0034_XCPLL_V006_REG_XCPLL_V006_SSC_MODE_0034); //BK100D_0D[1] = b'1
		W2BYTEMSK(REG_0050_XCPLL_V006, 0x1, REG_0050_XCPLL_V006_REG_XCPLL_V006_SLD_0050); //BK100D_14[0] = b'1
		W2BYTEMSK(REG_0054_XCPLL_V006, 0x0, REG_0054_XCPLL_V006_REG_XCPLL_V006_PORST_0054); //BK100D_15[0] = b'0
		//Delay 500us
		udelay(500);
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x0, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_0004); //BK100D_01[0] = b'0
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x1, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV1_0004); //BK100D_01[4] = b'1
		W2BYTEMSK(REG_0004_XCPLL_V006, 0x0, REG_0004_XCPLL_V006_REG_XCPLL_V006_PD_CLK_DIV2_0004); //BK100D_01[8] = b'0
	}
}


static void _PATCH_TRIGGER_GEN_v006(struct mtk_panel_priv *priv)
{
	#define TG_GEN_SW_HCOUNT (20)
	uint32_t u32VTT = 0;
	uint32_t u32HTT = 0;
	uint16_t u16VFreq = 0;
	uint64_t u64OneLineOutputUsx10000 = 0;
	uint64_t u64OneLineTrigUsx10000 = 0;
	uint32_t u32Value = 0;

	if (!priv)
		return;

	u32VTT = priv->typ_vtt;
	u32HTT = priv->typ_htt;

	if (u32HTT ==0)
		u32HTT = 1;
	if (u32VTT ==0)
		u32VTT = 1;

	u16VFreq = priv->typ_dclk/u32HTT/u32VTT;

	u64OneLineOutputUsx10000 = (uint64_t)1000000*10000/u16VFreq/u32VTT;
	u64OneLineTrigUsx10000 = 10000*TG_GEN_SW_HCOUNT/24;

	//patch for Trigger Gen
	W2BYTEMSK(REG_0018_TRIGGER_GEN_BKA3A4, 0x55,
		  REG_0018_TRIGGER_GEN_BKA3A4_REG_SRC0_DMA_RD_TRIG_DLY_V);

	W2BYTEMSK((0x1474C00 + 0x0A8), 0x5, Fld(13, 0, AC_MSKW10));	//reg_tgen_dummy0_trig_dly_v=5
	W2BYTEMSK((0x1474C00 + 0x0AC), 0x5, Fld(13, 0, AC_MSKW10));	//reg_tgen_dummy1_trig_dly_v=5
	W2BYTEMSK(REG_0004_TRIGGER_GEN_BKA3A4, 0,
		  REG_0004_TRIGGER_GEN_BKA3A4_REG_SRC0_INPUT_SRC_SEL);

	W2BYTEMSK(REG_0084_TRIGGER_GEN_BKA3A4, 0x1,
		  REG_0084_TRIGGER_GEN_BKA3A4_REG_SRC1_INPUT_SRC_SEL);

	W2BYTEMSK(REG_0104_TRIGGER_GEN_BKA3A4, 0x0,
		  REG_0104_TRIGGER_GEN_BKA3A4_REG_B2R_INPUT_SRC_SEL);
	W2BYTEMSK(REG_0004_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0004_TRIGGER_GEN_BKA3A5_REG_OP1_INPUT_SRC_SEL);
	W2BYTEMSK(REG_0084_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0084_TRIGGER_GEN_BKA3A5_REG_OP2_INPUT_SRC_SEL);
	W2BYTEMSK(REG_0104_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0104_TRIGGER_GEN_BKA3A5_REG_DISP_INPUT_SRC_SEL);
	W2BYTEMSK(REG_0184_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0184_TRIGGER_GEN_BKA3A5_REG_FRC1_INPUT_SRC_SEL);
	W2BYTEMSK((0x1474C00 + 0x04), 0x4, Fld(3, 0, AC_MSKB0));	//reg_frc2_input_src_sel=4
	W2BYTEMSK((0x1474C00 + 0x084), 0x2, Fld(3, 0, AC_MSKB0));	//reg_tgen_input_src_sel=0
	W2BYTEMSK((0x10A0800 + 0x004), 0x4, Fld(3, 0, AC_MSKB0));	//reg_frc14=4
	W2BYTEMSK((0x10A0400 + 0x184), 0x4, Fld(3, 0, AC_MSKB0));	//reg_frc15=4
	W2BYTEMSK((0x10A0600 + 0x084), 0x4, Fld(3, 0, AC_MSKB0));	//frc_stage_src_sel=4

	W2BYTEMSK(REG_0018_TRIGGER_GEN_BKA3A4, 0x9,
		  REG_0018_TRIGGER_GEN_BKA3A4_REG_SRC0_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0098_TRIGGER_GEN_BKA3A4, 0x9,
		  REG_0098_TRIGGER_GEN_BKA3A4_REG_SRC1_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0118_TRIGGER_GEN_BKA3A4, 0x9,
		  REG_0118_TRIGGER_GEN_BKA3A4_REG_B2R_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0198_TRIGGER_GEN_BKA3A4, 0x4,
		  REG_0198_TRIGGER_GEN_BKA3A4_REG_IP_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0018_TRIGGER_GEN_BKA3A5, 0x9,
		  REG_0018_TRIGGER_GEN_BKA3A5_REG_OP1_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0098_TRIGGER_GEN_BKA3A5, 0x9,
		  REG_0098_TRIGGER_GEN_BKA3A5_REG_OP2_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0198_TRIGGER_GEN_BKA3A5, 0x9,
		  REG_0198_TRIGGER_GEN_BKA3A5_REG_FRC1_DMA_RD_TRIG_DLY_V);
	// patch for gop trigger gen setting
	u32Value = 0;
	W2BYTEMSK(REG_0108_TRIGGER_GEN_BKA3A5, u32Value, REG_0108_TRIGGER_GEN_BKA3A5_REG_DISP_RW_BK_UP_TRIG_DLY_V);
	u32Value = (uint32_t)(1*u64OneLineOutputUsx10000/u64OneLineTrigUsx10000);
	W2BYTEMSK(REG_010C_TRIGGER_GEN_BKA3A5, u32Value, REG_010C_TRIGGER_GEN_BKA3A5_REG_DISP_DS_TRIG_DLY_V);
	W2BYTEMSK(REG_0110_TRIGGER_GEN_BKA3A5, u32Value, REG_0110_TRIGGER_GEN_BKA3A5_REG_DISP_ML_TRIG_DLY_V);
	u32Value = (uint32_t)(0x0D*u64OneLineOutputUsx10000/u64OneLineTrigUsx10000);
	W2BYTEMSK(REG_0114_TRIGGER_GEN_BKA3A5, u32Value, REG_0114_TRIGGER_GEN_BKA3A5_REG_DISP_VS_TRIG_DLY_V);
	u32Value = (uint32_t)(0x13*u64OneLineOutputUsx10000/u64OneLineTrigUsx10000);
	W2BYTEMSK(REG_0118_TRIGGER_GEN_BKA3A5, u32Value, REG_0118_TRIGGER_GEN_BKA3A5_REG_DISP_DMA_RD_TRIG_DLY_V);
	W2BYTEMSK(REG_0170_TRIGGER_GEN_BKA3A5, (TG_GEN_SW_HCOUNT-1), REG_0170_TRIGGER_GEN_BKA3A5_REG_DISP_SW_HTT_SIZE);
	W2BYTEMSK(REG_0174_TRIGGER_GEN_BKA3A5, 1, REG_0174_TRIGGER_GEN_BKA3A5_REG_DISP_SW_H_USR_MD);
	W2BYTEMSK(REG_0174_TRIGGER_GEN_BKA3A5, 0, REG_0174_TRIGGER_GEN_BKA3A5_REG_DISP_VS_SW_TRIG);
	W2BYTEMSK(REG_0174_TRIGGER_GEN_BKA3A5, 0, REG_0174_TRIGGER_GEN_BKA3A5_REG_DISP_SW_V_USR_MD);
	W2BYTEMSK(REG_0164_TRIGGER_GEN_BKA3A5, 1, REG_0164_TRIGGER_GEN_BKA3A5_REG_DISP_VCNT_OF_KEEP_EN);
	W2BYTEMSK(REG_0164_TRIGGER_GEN_BKA3A5, 8, REG_0164_TRIGGER_GEN_BKA3A5_REG_DISP_VCNT_UPD_MASK_RANGE);

	W2BYTEMSK((0x1474C00 + 0x018), 0x9, Fld(13, 0, AC_MSKW10));	//reg_frc2_dma_rd_trig_dly_v=9
	W2BYTEMSK((0x1474C00 + 0x098), 0x9, Fld(13, 0, AC_MSKW10));	//reg_tgen_dma_rd_trig_dly_v=9

	W2BYTEMSK(REG_0038_TRIGGER_GEN_BKA3A4, 0x1,
		  REG_0038_TRIGGER_GEN_BKA3A4_REG_SRC0_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_00B8_TRIGGER_GEN_BKA3A4, 0x1,
		  REG_00B8_TRIGGER_GEN_BKA3A4_REG_SRC1_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_0138_TRIGGER_GEN_BKA3A4, 0x1,
		  REG_0138_TRIGGER_GEN_BKA3A4_REG_B2R_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_01B8_TRIGGER_GEN_BKA3A4, 2,
		  REG_01B8_TRIGGER_GEN_BKA3A4_REG_IP_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_0038_TRIGGER_GEN_BKA3A5, 12,
		  REG_0038_TRIGGER_GEN_BKA3A5_REG_OP1_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_00B8_TRIGGER_GEN_BKA3A5, 19,
		  REG_00B8_TRIGGER_GEN_BKA3A5_REG_OP2_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_0138_TRIGGER_GEN_BKA3A5, 14,
		  REG_0138_TRIGGER_GEN_BKA3A5_REG_DISP_RW_BK_UPD_TRIG_DLY_H);
	W2BYTEMSK(REG_01B8_TRIGGER_GEN_BKA3A5, 1,
		  REG_01B8_TRIGGER_GEN_BKA3A5_REG_FRC1_RW_BK_UPD_TRIG_DLY_H);

	W2BYTEMSK((0x1474C00 + 0x038), 0x1, Fld(3, 0, AC_MSKB0));	//reg_frc2_rw_bk_upd_trig_dly_h=1
	W2BYTEMSK((0x1474C00 + 0x0B8), 0x1, Fld(3, 0, AC_MSKB0));	//reg_tgen_rw_bk_upd_trig_dly_h=1
	W2BYTEMSK((0x1474C00 + 0x138), 0x1, Fld(3, 0, AC_MSKB0));	//reg_b2r_lite0_rw_bk_upd_trig_dly_h=1
	W2BYTEMSK((0x1474C00 + 0x1B8), 0x1, Fld(3, 0, AC_MSKB0));	//reg_b2r_lite1_rw_bk_upd_trig_dly_h=1
	W2BYTEMSK(REG_0058_TRIGGER_GEN_BKA3A5, 1,
		  REG_0058_TRIGGER_GEN_BKA3A5_REG_OP1_RW_BK_UPD_TRIG_REP_INTERVAL);
	// patch for ldm usage
	u32Value = (uint32_t)(0x12*u64OneLineOutputUsx10000/u64OneLineTrigUsx10000);
	W2BYTEMSK(REG_0124_TRIGGER_GEN_BKA3A5, u32Value, REG_0124_TRIGGER_GEN_BKA3A5_REG_DISP_PQ_IRQ_TRIG_DLY_V);
}

uint32_t _align_floor_v006(uint32_t input, uint8_t alignNum)
{
	if (alignNum > 0)
		input = (input / alignNum) * alignNum;
	return input;
}

uint32_t _tgen_target_vde_v006(uint32_t u32DtsVde)
{
	uint32_t u32TargetVTT = TGEN_VDE_ST_2160;

	if (u32DtsVde == TGEN_VDE_4320)
		u32TargetVTT = TGEN_VDE_ST_4320;
	else if (u32DtsVde == TGEN_VDE_2160)
		u32TargetVTT = TGEN_VDE_ST_2160;
	else if (u32DtsVde <= TGEN_VDE_1080)
		u32TargetVTT = u32DtsVde * 100 * TGEN_VDE_ST_1080 / 1080 / 100;
	else
		u32TargetVTT = TGEN_VDE_ST_2160;

	return u32TargetVTT;
}

static uint32_t _tgen_target_VTTVBuf_v006(uint32_t u32DtsVde)
{
	uint32_t u32TargetVTTBuf = TGEN_VDE_ST_2160;

	if (u32DtsVde == TGEN_VDE_4320)
		u32TargetVTTBuf = TGEN_VTTV_BUF_4320;
	else if (u32DtsVde == TGEN_VDE_2160)
		u32TargetVTTBuf = TGEN_VTTV_BUF_2160;
	else if (u32DtsVde <= TGEN_VDE_1080)
		u32TargetVTTBuf = TGEN_VTTV_BUF_1080;
	else
		u32TargetVTTBuf = TGEN_VTTV_BUF_2160;

	return u32TargetVTTBuf;
}

int _tgen_qms_related_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	u8 qms_table_idx = 0;
	u8 idx = 0;
	u16 vfrq = 0;
	u16 lock_vsync_msk_num = 1;
	u16 tgen_input_vtt[QMS_TFR_MAX];
	u32 qms_output_vfreq = 0;

	if (!priv) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	//QMS setting: TFR from delay sync
	W2BYTEMSK(REG_0080_TGEN_VRR_BKA39F, 0x1, REG_0080_TGEN_VRR_BKA39F_REG_TGEN_NEXT_TFR_SEL);

	vfrq = priv->typ_dclk / priv->typ_htt / priv->typ_vtt;
	if (!IS_VFREQ_60HZ_GROUP(vfrq))
		qms_table_idx = 1;

	for (idx = 0; idx < QMS_TFR_MAX; idx++) {
		qms_output_vfreq = qms_info[qms_table_idx][idx].output_vfrq;
		if (qms_output_vfreq == 0)
			tgen_input_vtt[idx] = 0;
		else
			tgen_input_vtt[idx] = priv->typ_vtt * vfrq * DEC_100 / qms_info[qms_table_idx][idx].input_vfrq;
	}

	idx = 1;
	W2BYTEMSK(REG_0140_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0140_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_1);
	W2BYTEMSK(REG_0140_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0140_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_1);
	W2BYTEMSK(REG_00F0_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_00F0_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_1);
	idx++;

	W2BYTEMSK(REG_0140_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_0140_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_2);
	W2BYTEMSK(REG_0140_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0140_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_2);
	W2BYTEMSK(REG_00F4_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_00F4_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_2);
	idx++;

	W2BYTEMSK(REG_0144_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0144_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_3);
	W2BYTEMSK(REG_0144_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0144_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_3);
	W2BYTEMSK(REG_00F8_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_00F8_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_3);
	idx++;

	W2BYTEMSK(REG_0144_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_0144_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_4);
	W2BYTEMSK(REG_0144_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0144_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_4);
	W2BYTEMSK(REG_00FC_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_00FC_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_4);
	idx++;

	W2BYTEMSK(REG_0148_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0148_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_5);
	W2BYTEMSK(REG_0148_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0148_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_5);
	W2BYTEMSK(REG_0100_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0100_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_5);
	idx++;

	W2BYTEMSK(REG_0148_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_0148_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_6);
	W2BYTEMSK(REG_0148_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0148_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_6);
	W2BYTEMSK(REG_0104_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0104_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_6);
	idx++;

	W2BYTEMSK(REG_014C_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_014C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_7);
	W2BYTEMSK(REG_014C_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_014C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_7);
	W2BYTEMSK(REG_0108_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0108_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_7);
	idx++;

	W2BYTEMSK(REG_014C_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_014C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_8);
	W2BYTEMSK(REG_014C_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_014C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_8);
	W2BYTEMSK(REG_010C_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_010C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_8);
	idx++;

	W2BYTEMSK(REG_0150_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0150_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_9);
	W2BYTEMSK(REG_0150_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0150_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_9);
	W2BYTEMSK(REG_0110_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0110_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_9);
	idx++;

	W2BYTEMSK(REG_0150_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_0150_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_10);
	W2BYTEMSK(REG_0150_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0150_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_10);
	W2BYTEMSK(REG_0114_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0114_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_10);
	idx++;

	W2BYTEMSK(REG_0154_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0154_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_11);
	W2BYTEMSK(REG_0154_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0154_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_11);
	W2BYTEMSK(REG_0118_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0118_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_11);
	idx++;

	W2BYTEMSK(REG_0154_TGEN_FRAMELOCK_BKA3A2, SHIFT_LEFT_8((MAX(lock_vsync_msk_num, 1) - 1)),
		  REG_0154_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_12);
	W2BYTEMSK(REG_0154_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0154_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_12);
	W2BYTEMSK(REG_011C_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_011C_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_12);
	idx++;

	W2BYTEMSK(REG_0158_TGEN_FRAMELOCK_BKA3A2, (MAX(lock_vsync_msk_num, 1) - 1),
		  REG_0158_TGEN_FRAMELOCK_BKA3A2_REG_QMS_IP_LOCK_VSYNC_MASK_NUM_13);
	W2BYTEMSK(REG_0158_TGEN_FRAMELOCK_BKA3A2, (MAX(qms_info[qms_table_idx][idx].ratio, 1) - 1),
		  REG_0158_TGEN_FRAMELOCK_BKA3A2_REG_QMS_LOCK_FREERUN_SWITCH_NUM_13);
	W2BYTEMSK(REG_0120_TGEN_FRAMELOCK_BKA3A2, (MAX(tgen_input_vtt[idx], 0)),
		  REG_0120_TGEN_FRAMELOCK_BKA3A2_REG_QMS_HW_SRC_LOCK_VTT_CHECK_13);

	return 0;
}

int mtk_tgen_init_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint8_t alignNum = 0;
	uint32_t u32TargetVdeStart = 0;
	uint32_t u32TargetVTTVBuf = 0;
	uint32_t u32DtsVbp = 0;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}
	//HAPS test, need to enable this bit first.
	//Let tool can switch ch5 to read scaler bank.
	W2BYTEMSK(0x60041C, 0x4, Fld(4, 0, AC_MSKB0));

	// for video mute, enable render blending
	W2BYTEMSK(REG_0004_BLEND_TOP_BKA36B, 1, REG_0004_BLEND_TOP_BKA36B_REG_LAYER0_OFF);
	W2BYTEMSK(REG_000C_OSDB_COLOV_BKA3ED, 1, REG_000C_OSDB_COLOV_BKA3ED_REG_COLOV_SET_BLACK_DBF_EN);
	W2BYTEMSK(REG_0010_OSDB_COLOV_BKA3ED, 1, REG_0010_OSDB_COLOV_BKA3ED_REG_COLOV_SET_BLACK_MAIN_EN);


	//Patch for enable xc mcu clk.
	_PATCH_INIT_XC_CLK_v006();

//#ifdef CONFIG_HAPS
#if 0
	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_TS_MD);
	W2BYTEMSK(REG_004C_TGEN_MAIN_BKA3A0, 0x112F, REG_004C_TGEN_MAIN_BKA3A0_REG_HTT);
	W2BYTEMSK(REG_008C_TGEN_MAIN_BKA3A0, 0x8C9, REG_008C_TGEN_MAIN_BKA3A0_REG_VTT);
	W2BYTEMSK(REG_0050_TGEN_MAIN_BKA3A0, 0, REG_0050_TGEN_MAIN_BKA3A0_REG_HSYNC_ST);
	W2BYTEMSK(REG_0054_TGEN_MAIN_BKA3A0, 0x3, REG_0054_TGEN_MAIN_BKA3A0_REG_HSYNC_END);
	W2BYTEMSK(REG_0058_TGEN_MAIN_BKA3A0, 0x30, REG_0058_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_ST);
	W2BYTEMSK(REG_005C_TGEN_MAIN_BKA3A0, 0xF2F, REG_005C_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_END);
	W2BYTEMSK(REG_0060_TGEN_MAIN_BKA3A0, 0x30, REG_0060_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_V_ST);
	W2BYTEMSK(REG_0064_TGEN_MAIN_BKA3A0, 0xF2F, REG_0064_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_V_END);
	W2BYTEMSK(REG_0068_TGEN_MAIN_BKA3A0, 0x30, REG_0068_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_G_ST);
	W2BYTEMSK(REG_006C_TGEN_MAIN_BKA3A0, 0xF2f, REG_006C_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_G_END);
	W2BYTEMSK(REG_0090_TGEN_MAIN_BKA3A0, 0, REG_0090_TGEN_MAIN_BKA3A0_REG_VSYNC_ST);
	W2BYTEMSK(REG_0094_TGEN_MAIN_BKA3A0, 0x0, REG_0094_TGEN_MAIN_BKA3A0_REG_VSYNC_END);
	W2BYTEMSK(REG_0098_TGEN_MAIN_BKA3A0, 0x1E, REG_0098_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_ST);
	W2BYTEMSK(REG_009C_TGEN_MAIN_BKA3A0, 0x88D, REG_009C_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_END);
	W2BYTEMSK(REG_00A0_TGEN_MAIN_BKA3A0, 0x1E, REG_00A0_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_V_ST);
	W2BYTEMSK(REG_00A4_TGEN_MAIN_BKA3A0, 0x88D, REG_00A4_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_V_END);
	W2BYTEMSK(REG_00A8_TGEN_MAIN_BKA3A0, 0x1E, REG_00A8_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_G_ST);
	W2BYTEMSK(REG_00AC_TGEN_MAIN_BKA3A0, 0x88D, REG_00AC_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_G_END);
	W2BYTEMSK(REG_01F4_TGEN_MAIN_BKA3A0, 0, REG_01F4_TGEN_MAIN_BKA3A0_REG_M_DELTA);
	W2BYTEMSK(REG_0008_TGEN_FRAMELOCK_BKA3A2, 0,
		  REG_0008_TGEN_FRAMELOCK_BKA3A2_REG_PROTECT_V_END);
	W2BYTEMSK(REG_01F8_TGEN_MAIN_BKA3A0, 0, REG_01F8_TGEN_MAIN_BKA3A0_REG_PANEL_LOWER_BOUND);
	W2BYTEMSK(REG_01FC_TGEN_MAIN_BKA3A0, 0xFFFF,
		  REG_01FC_TGEN_MAIN_BKA3A0_REG_PANEL_UPPER_BOUND);
	W2BYTEMSK(REG_0084_TGEN_MAIN_BKA3A0, 1, REG_0084_TGEN_MAIN_BKA3A0_REG_EVEN_VTT_VCNT_EN);
	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_SW_RST_ODCLK);
	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_SW_RST_XTALI);
	W2BYTEMSK(REG_0008_TGEN_MAIN_BKA3A0, 1, REG_0008_TGEN_MAIN_BKA3A0_REG_TGEN_VIDEO_EN);
	W2BYTEMSK(REG_00C0_TGEN_MAIN_BKA3A0, 0x30, REG_00C0_TGEN_MAIN_BKA3A0_REG_HDE_VIDEO0_ST);
	W2BYTEMSK(REG_00C4_TGEN_MAIN_BKA3A0, 0xF2F, REG_00C4_TGEN_MAIN_BKA3A0_REG_HDE_VIDEO0_END);
	W2BYTEMSK(REG_0100_TGEN_MAIN_BKA3A0, 0x30, REG_0100_TGEN_MAIN_BKA3A0_REG_VDE_VIDEO0_ST);
	W2BYTEMSK(REG_0104_TGEN_MAIN_BKA3A0, 0x88D, REG_0104_TGEN_MAIN_BKA3A0_REG_VDE_VIDEO0_END);

	//set disp domain trigger gen setting, ref to tgen sync.
	W2BYTEMSK(REG_0104_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0104_TRIGGER_GEN_BKA3A5_REG_DISP_INPUT_SRC_SEL);
#else
	//Vde start protect
	u32TargetVdeStart = _tgen_target_vde_v006(priv->de_height);
	u32TargetVTTVBuf = _tgen_target_VTTVBuf_v006(priv->de_height);

	if ((priv->de_width == 3840 && priv->de_height == 2160) &&
	    ((priv->pnl_lib_version == BOOT_PNL_VERSION0400)
	     || (priv->pnl_lib_version == BOOT_PNL_VERSION0500)))
		u32TargetVdeStart = u32TargetVdeStart - TGEN_PIXELSHIFT_BUF;

	if (u32TargetVdeStart > priv->de_vstart) {
		u32DtsVbp = priv->de_vstart - (priv->vsync_st + priv->vsync_w);
		priv->de_vstart = u32TargetVdeStart;
		priv->vsync_st = u32TargetVdeStart - priv->vsync_w - u32DtsVbp;
	}

	if (priv->de_vstart > (priv->typ_vtt - priv->de_height - u32TargetVTTVBuf)) {
		u32DtsVbp = priv->de_vstart - (priv->vsync_st + priv->vsync_w);
		priv->de_vstart = priv->typ_vtt - priv->de_height - u32TargetVTTVBuf;
		if (priv->de_vstart > (priv->vsync_w + u32DtsVbp))
			priv->vsync_st = priv->de_vstart - priv->vsync_w - u32DtsVbp;
		else
			priv->vsync_st = 0;
	}

	UBOOT_DEBUG("Vde start protect\n");
	UBOOT_DEBUG("u32TargetVdeStart %d\n", u32TargetVdeStart);
	UBOOT_DEBUG("u32DtsVbp %d\n", u32DtsVbp);
	UBOOT_DEBUG("priv->de_vstart %d\n", priv->de_vstart);
	UBOOT_DEBUG("priv->vsync_st %d\n", priv->vsync_st);

	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_TS_MD);	//legacy mode.

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
		// 1p aligned
		alignNum = 1;
		priv->de_hstart = _align_floor_v006(priv->de_hstart, alignNum);
		priv->de_width = _align_floor_v006(priv->de_width, alignNum);
	}

	W2BYTEMSK(REG_004C_TGEN_MAIN_BKA3A0, priv->typ_htt - 1, REG_004C_TGEN_MAIN_BKA3A0_REG_HTT);
	W2BYTEMSK(REG_008C_TGEN_MAIN_BKA3A0, priv->typ_vtt - 1, REG_008C_TGEN_MAIN_BKA3A0_REG_VTT);

	W2BYTEMSK(REG_0050_TGEN_MAIN_BKA3A0, 0, REG_0050_TGEN_MAIN_BKA3A0_REG_HSYNC_ST);
	W2BYTEMSK(REG_0054_TGEN_MAIN_BKA3A0, priv->hsync_w - 1,
		  REG_0054_TGEN_MAIN_BKA3A0_REG_HSYNC_END);

	W2BYTEMSK(REG_0058_TGEN_MAIN_BKA3A0, priv->de_hstart,
		  REG_0058_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_ST);
	W2BYTEMSK(REG_005C_TGEN_MAIN_BKA3A0, priv->de_hstart + priv->de_width - 1,
		  REG_005C_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_END);
	W2BYTEMSK(REG_0060_TGEN_MAIN_BKA3A0, priv->de_hstart,
		  REG_0060_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_V_ST);
	W2BYTEMSK(REG_0064_TGEN_MAIN_BKA3A0, priv->de_hstart + priv->de_width - 1,
		  REG_0064_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_V_END);

	//set tcon de for s company justscan orbit case
	//the value is equal to panel DE
	W2BYTEMSK(REG_0070_TGEN_MAIN_BKA3A0, priv->de_hstart,
		  REG_0070_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_TCON_ST);
	W2BYTEMSK(REG_0074_TGEN_MAIN_BKA3A0, priv->de_hstart + priv->de_width - 1,
		  REG_0074_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_TCON_END);

	//GFX?
	W2BYTEMSK(REG_0068_TGEN_MAIN_BKA3A0, priv->de_hstart,
		  REG_0068_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_G_ST);
	W2BYTEMSK(REG_006C_TGEN_MAIN_BKA3A0, priv->de_hstart + priv->de_width - 1,
		  REG_006C_TGEN_MAIN_BKA3A0_REG_HDE_FRAME_G_END);

	//disp lb Hsize
	W2BYTEMSK(REG_0018_DISP_LB_BKA326, priv->de_width, REG_0018_DISP_LB_BKA326_REG_DISP_HSIZE);

	W2BYTEMSK(REG_0090_TGEN_MAIN_BKA3A0, priv->vsync_st,
		  REG_0090_TGEN_MAIN_BKA3A0_REG_VSYNC_ST);
	W2BYTEMSK(REG_0014_TGEN_OUTPUT_BKA3A3, 0, REG_0014_TGEN_OUTPUT_BKA3A3_REG_TGEN_TO_TRIG_GEN_VSYNC_VCNT);	// to trigger gen vsync : same as vsync start

	W2BYTEMSK(REG_0094_TGEN_MAIN_BKA3A0, priv->vsync_st + priv->vsync_w - 1,
		  REG_0094_TGEN_MAIN_BKA3A0_REG_VSYNC_END);
	W2BYTEMSK(REG_0098_TGEN_MAIN_BKA3A0, priv->de_vstart,
		  REG_0098_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_ST);
	W2BYTEMSK(REG_009C_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height - 1,
		  REG_009C_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_END);
	W2BYTEMSK(REG_00A0_TGEN_MAIN_BKA3A0, priv->de_vstart,
		  REG_00A0_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_V_ST);
	W2BYTEMSK(REG_00A4_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height - 1,
		  REG_00A4_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_V_END);

	//set tcon de for s company justscan orbit case
	//the value is equal to panel DE
	W2BYTEMSK(REG_00B0_TGEN_MAIN_BKA3A0, priv->de_vstart,
		  REG_00B0_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_TCON_ST);
	W2BYTEMSK(REG_00B4_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height - 1,
		  REG_00B4_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_TCON_END);

	//GFX?
	W2BYTEMSK(REG_00A8_TGEN_MAIN_BKA3A0, priv->de_vstart,
		  REG_00A8_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_G_ST);
	W2BYTEMSK(REG_00AC_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height - 1,
		  REG_00AC_TGEN_MAIN_BKA3A0_REG_VDE_FRAME_G_END);

	//disp lb Vsize
	W2BYTEMSK(REG_001C_DISP_LB_BKA326, priv->de_height, REG_001C_DISP_LB_BKA326_REG_DISP_VSIZE);

	W2BYTEMSK(REG_01F4_TGEN_MAIN_BKA3A0, 0xFFFF, REG_01F4_TGEN_MAIN_BKA3A0_REG_M_DELTA);	// m delta
	W2BYTEMSK(REG_0008_TGEN_FRAMELOCK_BKA3A2, priv->min_vtt_panelprotect, REG_0008_TGEN_FRAMELOCK_BKA3A2_REG_PROTECT_V_END);	// protect v endi

	if ((priv->de_vstart + priv->de_height) > priv->min_vtt_panelprotect) {	//if Vde_end is after Vmin then use Vde_end as lower bound
		W2BYTEMSK(REG_0008_TGEN_FRAMELOCK_BKA3A2, priv->de_vstart + priv->de_height, REG_0008_TGEN_FRAMELOCK_BKA3A2_REG_PROTECT_V_END);	// protect v end
		W2BYTEMSK(REG_01F8_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height,
			  REG_01F8_TGEN_MAIN_BKA3A0_REG_PANEL_LOWER_BOUND);
	} else {
		W2BYTEMSK(REG_0008_TGEN_FRAMELOCK_BKA3A2, priv->min_vtt_panelprotect, REG_0008_TGEN_FRAMELOCK_BKA3A2_REG_PROTECT_V_END);	// protect v end
		W2BYTEMSK(REG_01F8_TGEN_MAIN_BKA3A0, priv->min_vtt_panelprotect,
			  REG_01F8_TGEN_MAIN_BKA3A0_REG_PANEL_LOWER_BOUND);
	}
	W2BYTEMSK(REG_01FC_TGEN_MAIN_BKA3A0, priv->max_vtt_panelprotect,
		  REG_01FC_TGEN_MAIN_BKA3A0_REG_PANEL_UPPER_BOUND);

	W2BYTEMSK(REG_0084_TGEN_MAIN_BKA3A0, 0, REG_0084_TGEN_MAIN_BKA3A0_REG_EVEN_VTT_VCNT_EN);

	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_SW_RST_XTALI);

	W2BYTEMSK(REG_0008_TGEN_MAIN_BKA3A0, 1, REG_0008_TGEN_MAIN_BKA3A0_REG_TGEN_VIDEO_EN);	// only enable window 0

	W2BYTEMSK(REG_0004_TGEN_MAIN_BKA3A0, 0, REG_0004_TGEN_MAIN_BKA3A0_REG_TGEN_SW_RST_ODCLK);

	W2BYTEMSK(REG_00C0_TGEN_MAIN_BKA3A0, priv->de_hstart,
		  REG_00C0_TGEN_MAIN_BKA3A0_REG_HDE_VIDEO0_ST);
	W2BYTEMSK(REG_00C4_TGEN_MAIN_BKA3A0, priv->de_hstart + priv->de_width - 1,
		  REG_00C4_TGEN_MAIN_BKA3A0_REG_HDE_VIDEO0_END);
	W2BYTEMSK(REG_0100_TGEN_MAIN_BKA3A0, priv->de_vstart,
		  REG_0100_TGEN_MAIN_BKA3A0_REG_VDE_VIDEO0_ST);
	W2BYTEMSK(REG_0104_TGEN_MAIN_BKA3A0, priv->de_vstart + priv->de_height - 1,
		  REG_0104_TGEN_MAIN_BKA3A0_REG_VDE_VIDEO0_END);

	//set disp domain trigger gen setting, ref to tgen sync.
	W2BYTEMSK(REG_0104_TRIGGER_GEN_BKA3A5, 0x4,
		  REG_0104_TRIGGER_GEN_BKA3A5_REG_DISP_INPUT_SRC_SEL);

	//For main video_out 120hz, GFX 60HZ output case,
	//need to reset gfx every 2 vsync
	if ((priv->out_timing == E_4K2K_120HZ) || (priv->out_timing == E_4K2K_144HZ)
	    || (priv->out_timing == E_4K1K_240HZ))
		W2BYTEMSK(REG_004C_TGEN_OUTPUT_BKA3A3, 0x1,
			  REG_004C_TGEN_OUTPUT_BKA3A3_REG_TGEN_TO_GOP_REF_MASK_EN);

	//sw patch for reset gop-sc aid binding

	_PATCH_TRIGGER_GEN_v006(priv);

	_tgen_qms_related_setting(dev);

#endif
	return 0;

}

static void _lvds_v_hd_60HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}
	// LVDS always 2p
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_1PTO2P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x3, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0x780, REG_0040_MODV11_REG_HSIZE); // move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_015C_MODD1, 0x0, REG_015C_MODD1_REG_SERI_FORMAT_V1);
	//REG_0038_MODA1_REG_GCR_BANK_CLK_SRC_SEL bit[0:0]
	W2BYTEMSK(REG_0038_MODA1, 0x0, Fld(1, 0, AC_MSKB0));
	W2BYTEMSK(REG_003C_MODV11, 0x10, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_RSCLK_TESTMD);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_lvds_ti_mode, REG_00E0_MODD1_REG_LVDS_TI);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_pdp_10bit, REG_00E0_MODD1_REG_PDP_10BIT);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_PLASMA);
	W2BYTEMSK(REG_00E0_MODD1, (priv->lvds_info.panel_swap_lvds_pol), REG_00E0_MODD1_REG_CH_POLARITY);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_swap_lvds_ch, REG_00E0_MODD1_REG_PA_SWAP);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_EN);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.ti_bit_mode, REG_00E0_MODD1_REG_TI_BITMODE);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_TESTERMD);
}

static void _lvds_v_hd_120HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}
	// LVDS always 2p
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_1PTO2P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x3, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0x780, REG_0040_MODV11_REG_HSIZE); // move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_015C_MODD1, 0x0, REG_015C_MODD1_REG_SERI_FORMAT_V1);
	//REG_0038_MODA1_REG_GCR_BANK_CLK_SRC_SEL bit[0:0]
	W2BYTEMSK(REG_0038_MODA1, 0x0, Fld(1, 0, AC_MSKB0));
	W2BYTEMSK(REG_003C_MODV11, 0x10, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_RSCLK_TESTMD);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_lvds_ti_mode, REG_00E0_MODD1_REG_LVDS_TI);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_pdp_10bit, REG_00E0_MODD1_REG_PDP_10BIT);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_PLASMA);
	W2BYTEMSK(REG_00E0_MODD1, (priv->lvds_info.panel_swap_lvds_pol), REG_00E0_MODD1_REG_CH_POLARITY);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_swap_lvds_ch, REG_00E0_MODD1_REG_PA_SWAP);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_EN);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.ti_bit_mode, REG_00E0_MODD1_REG_TI_BITMODE);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_TESTERMD);
}

static void _lvds_v_fhd_60HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	// LVDS always 2p
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_1PTO2P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x2, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0x780, REG_0040_MODV11_REG_HSIZE); // move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_015C_MODD1, 0x0, REG_015C_MODD1_REG_SERI_FORMAT_V1);
	//REG_0038_MODA1_REG_GCR_BANK_CLK_SRC_SEL bit[0:0]
	W2BYTEMSK(REG_0038_MODA1, 0x0, Fld(1, 0, AC_MSKB0));
	W2BYTEMSK(REG_003C_MODV11, 0x10, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_RSCLK_TESTMD);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_lvds_ti_mode, REG_00E0_MODD1_REG_LVDS_TI);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_pdp_10bit, REG_00E0_MODD1_REG_PDP_10BIT);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_PLASMA);
	W2BYTEMSK(REG_00E0_MODD1, (priv->lvds_info.panel_swap_lvds_pol), REG_00E0_MODD1_REG_CH_POLARITY);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.panel_swap_lvds_ch, REG_00E0_MODD1_REG_PA_SWAP);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH4_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_CH3_EN);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_SET);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_PDP_MASK_EN);
	W2BYTEMSK(REG_00E0_MODD1, priv->lvds_info.ti_bit_mode, REG_00E0_MODD1_REG_TI_BITMODE);
	W2BYTEMSK(REG_00E0_MODD1, 0x0, REG_00E0_MODD1_REG_LVDS_TESTERMD);
}

static void _vby1_v_fhd_60HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x2, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0x780, REG_0040_MODV11_REG_HSIZE); // move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_003C_MODV11, 0x10, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
}

static void _vby1_v_fhd_120HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x1, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0x780, REG_0040_MODV11_REG_HSIZE); //move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_003C_MODV11, 0x22, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
}

static void _vby1_v_4k_60HZ_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x1, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
	W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_YUV_422_MODE);
	W2BYTEMSK(REG_0004_MODV11, 0x0, REG_0004_MODV11_REG_DIV_PIX_MODE);
	//W2BYTEMSK(REG_0040_MODV11, 0xF00, REG_0040_MODV11_REG_HSIZE); // move into _vby1_set_mft()
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_8PTO16P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x0, REG_0004_MODV12_REG_VBY1_16PTO32P_EN);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, 0x1, REG_0004_MODV12_REG_VBY1_VS_INV);
	W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_FORMAT_MAP);
	W2BYTEMSK(REG_003C_MODV11, 0x22, REG_003C_MODV11_REG_MFT_READ_HDE_ST);
}

static void _vby1_set_byte_mode_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	//en_vbo_bytemode vbo_byte = E_VBO_NO_LINK;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->linktype != E_LINK_VB1)
		return;

	//vbo_byte = priv->vbo_byte;

	// 4 byte mode
	W2BYTEMSK(REG_0188_MODV12, 2, REG_0188_MODV12_REG_VBY1_BYTE_MODE);
	W2BYTEMSK(REG_015C_MODD1, 0x1, REG_015C_MODD1_REG_SERI_FORMAT_V1);
	W2BYTEMSK(REG_0038_MODA1, 0x0, Fld(1, 0, AC_MSKB0));
}

static void _vby1_set_mft_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	//mirror type , mirror_en
	uint32_t mod_x = 0;
	uint32_t de_align4Num = 0;

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	uint8_t htt_slot = 0;

	if (priv->out_format == E_OUTPUT_YUV422)
		W2BYTEMSK(REG_0040_MODV11, priv->de_width / 2, REG_0040_MODV11_REG_HSIZE);
	else
		W2BYTEMSK(REG_0040_MODV11, priv->de_width, REG_0040_MODV11_REG_HSIZE);

	if (priv->linktype == E_LINK_VB1) {
		switch (priv->div_sec) {
		case 1:
			W2BYTEMSK(REG_0004_MODV11, 0x1, REG_0004_MODV11_REG_MFT_MODE);
			if (priv->out_format == E_OUTPUT_YUV422)
				W2BYTEMSK(REG_0044_MODV11, priv->de_width / 2,
					  REG_0044_MODV11_REG_DIV_LEN);
			else
				W2BYTEMSK(REG_0044_MODV11, priv->de_width,
					  REG_0044_MODV11_REG_DIV_LEN);

			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			break;

		case 2:
			W2BYTEMSK(REG_0004_MODV11, 0x2, REG_0004_MODV11_REG_MFT_MODE);
			if (priv->out_format == E_OUTPUT_YUV422)
				W2BYTEMSK(REG_0044_MODV11, priv->de_width / 4,
					  REG_0044_MODV11_REG_DIV_LEN);
			else
				W2BYTEMSK(REG_0044_MODV11, priv->de_width / 2,
					  REG_0044_MODV11_REG_DIV_LEN);

			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			W2BYTEMSK(REG_004C_MODV11, priv->de_width / 2,
				  REG_004C_MODV11_REG_BASE1_ADDR);
			break;

		case 4:
			W2BYTEMSK(REG_0004_MODV11, 0x3, REG_0004_MODV11_REG_MFT_MODE);
			W2BYTEMSK(REG_0044_MODV11, priv->de_width / 4, REG_0044_MODV11_REG_DIV_LEN);
			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			W2BYTEMSK(REG_004C_MODV11, priv->de_width / 4,
				  REG_004C_MODV11_REG_BASE1_ADDR);
			W2BYTEMSK(REG_0050_MODV11, priv->de_width / 2,
				  REG_0050_MODV11_REG_BASE2_ADDR);
			W2BYTEMSK(REG_0054_MODV11, priv->de_width * 3 / 4,
				  REG_0054_MODV11_REG_BASE3_ADDR);
			break;

		case 8:
			W2BYTEMSK(REG_0004_MODV11, 0x4, REG_0004_MODV11_REG_MFT_MODE);
			W2BYTEMSK(REG_0044_MODV11, priv->de_width / 8, REG_0044_MODV11_REG_DIV_LEN);
			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			W2BYTEMSK(REG_004C_MODV11, priv->de_width / 8,
				  REG_004C_MODV11_REG_BASE1_ADDR);
			W2BYTEMSK(REG_0050_MODV11, priv->de_width / 4,
				  REG_0050_MODV11_REG_BASE2_ADDR);
			W2BYTEMSK(REG_0054_MODV11, priv->de_width * 3 / 8,
				  REG_0054_MODV11_REG_BASE3_ADDR);
			W2BYTEMSK(REG_0058_MODV11, priv->de_width / 2,
				  REG_0058_MODV11_REG_BASE4_ADDR);
			W2BYTEMSK(REG_005C_MODV11, priv->de_width * 5 / 8,
				  REG_005C_MODV11_REG_BASE5_ADDR);
			W2BYTEMSK(REG_0060_MODV11, priv->de_width * 6 / 8,
				  REG_0060_MODV11_REG_BASE6_ADDR);
			W2BYTEMSK(REG_0064_MODV11, priv->de_width * 7 / 8,
				  REG_0064_MODV11_REG_BASE7_ADDR);
			break;

		default:
			debug("UNSUPPORT MFT Division\n");
			break;

		}

		if (priv->lanes / MFT_P > 0)
			htt_slot = priv->lanes / MFT_P - 1;
		else
			htt_slot = 0;

		W2BYTEMSK(REG_000C_MODV11, htt_slot, REG_000C_MODV11_REG_MFT_VLD_WRAP_FIX_HTT);
	}

	if (priv->linktype == E_LINK_LVDS) {
		//prepare for h mirror case
		mod_x = (priv->de_width)%4;
		//de_width allign to 4
		de_align4Num = (priv->de_width + 3) & ~3;

		//MFT DE Filter En = 1
		W2BYTEMSK(REG_0118_MODV11, 1, REG_0118_MODV11_REG_MFT_DE_FILTER_EN);

		if (priv->cus_info.hmirror_en == true) {
			UBOOT_DEBUG("H Mirror CASE\n");
			W2BYTEMSK(REG_0004_MODV11, 0x1, REG_0004_MODV11_REG_MFT_MODE);
			W2BYTEMSK(REG_0040_MODV11, 0x558, REG_0040_MODV11_REG_HSIZE);
			W2BYTEMSK(REG_0044_MODV11, 0x558, REG_0044_MODV11_REG_DIV_LEN);
			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			//mirror case start = x
			W2BYTEMSK(REG_0118_MODV11, mod_x, REG_0118_MODV11_REG_MFT_DE_FILTER_ST);
			//mirror case end = de_align4Num
			W2BYTEMSK(REG_011C_MODV11, de_align4Num, REG_011C_MODV11_REG_MFT_DE_FILTER_END);
		} else {
			UBOOT_DEBUG("H non Mirror CASE\n");
			//prepare for h non-mirror case
			W2BYTEMSK(REG_0004_MODV11, 0x1, REG_0004_MODV11_REG_MFT_MODE);
			W2BYTEMSK(REG_0040_MODV11, priv->de_width, REG_0040_MODV11_REG_HSIZE);
			W2BYTEMSK(REG_0044_MODV11, priv->de_width, REG_0044_MODV11_REG_DIV_LEN);
			W2BYTEMSK(REG_0048_MODV11, 0x0, REG_0048_MODV11_REG_BASE0_ADDR);
			//mirror case start = 0
			W2BYTEMSK(REG_0118_MODV11, mod_x, REG_0118_MODV11_REG_MFT_DE_FILTER_ST);
			//mirror case end = de_align4Num -mod_x
			W2BYTEMSK(REG_011C_MODV11, (de_align4Num - mod_x), REG_011C_MODV11_REG_MFT_DE_FILTER_END);
		}
	}
#ifdef CONFIG_HAPS
	//if (priv->pnl_lib_version == BOOT_PNL_VERSION0500)
	{
		// HAPS case, bypass MFT
		W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_1PTO2P_EN);
		W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_2PTO4P_EN);
		W2BYTEMSK(REG_0140_MODV11, 0x0, REG_0140_MODV11_REG_MOD_4PTO8P_EN);
		W2BYTEMSK(REG_0004_MODV11, 0x0, REG_0004_MODV11_REG_DIV_PIX_MODE);
		W2BYTEMSK(REG_0004_MODV11, 0x0, REG_0004_MODV11_REG_MFT_MODE);
	}
#endif				//#ifdef CONFIG
}

static void _vby1_set_SCTCON_MISC_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->out_format == E_OUTPUT_YUV444) {
		W2BYTEMSK(REG_0004_SCTCON_MISC_BKA3E0, 0,
			  REG_0004_SCTCON_MISC_BKA3E0_REG_SCTCON_MISC_BYPASS_ALL);
		W2BYTEMSK(REG_0040_SCTCON_MISC_BKA3E0, 1,
			  REG_0040_SCTCON_MISC_BKA3E0_REG_444TO422_BYPASS);
		W2BYTEMSK(REG_0040_SCTCON_MISC_BKA3E0, 0,
			  REG_0040_SCTCON_MISC_BKA3E0_REG_MOD_REORDER);
	} else if (priv->out_format == E_OUTPUT_YUV422) {
		W2BYTEMSK(REG_0004_SCTCON_MISC_BKA3E0, 0, REG_0004_SCTCON_MISC_BKA3E0_REG_SCTCON_MISC_BYPASS_ALL);	//csc out bypass
		W2BYTEMSK(REG_0040_SCTCON_MISC_BKA3E0, 0,
			  REG_0040_SCTCON_MISC_BKA3E0_REG_444TO422_BYPASS);
		W2BYTEMSK(REG_0040_SCTCON_MISC_BKA3E0, 1,
			  REG_0040_SCTCON_MISC_BKA3E0_REG_MOD_REORDER);
	} else {
		W2BYTEMSK(REG_0004_SCTCON_MISC_BKA3E0, 1, REG_0004_SCTCON_MISC_BKA3E0_REG_SCTCON_MISC_BYPASS_ALL);	//csc out bypass
	}
}

static void _vby1_seri_data_sel_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->lanes == 32)
		W2BYTEMSK(REG_00C0_MODD1, 0x1, REG_00C0_MODD1_REG_DATAX_SEL);

	if (priv->lanes == 16 || priv->lanes == 8)
		W2BYTEMSK(REG_00C0_MODD1, 0x2, REG_00C0_MODD1_REG_DATAX_SEL);

	if (priv->lanes == 64)
		W2BYTEMSK(REG_00C0_MODD1, 0x3, REG_00C0_MODD1_REG_DATAX_SEL);

	if (priv->linktype == E_LINK_LVDS)
		W2BYTEMSK(REG_00C0_MODD1, 0x0, REG_00C0_MODD1_REG_DATAX_SEL);

}

static void _pin_mapping_free_swap_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}
	//ping mapping free swap
	if (priv->linktype == E_LINK_LVDS) {
		W2BYTEMSK(REG_0044_MODD1, 0x00, REG_0044_MODD1_REG_FREE_CH02_SWAP);
		W2BYTEMSK(REG_0044_MODD1, 0x01, REG_0044_MODD1_REG_FREE_CH03_SWAP);
		W2BYTEMSK(REG_0048_MODD1, 0x02, REG_0048_MODD1_REG_FREE_CH04_SWAP);
		W2BYTEMSK(REG_0048_MODD1, 0x03, REG_0048_MODD1_REG_FREE_CH05_SWAP);
		W2BYTEMSK(REG_004C_MODD1, 0x04, REG_004C_MODD1_REG_FREE_CH06_SWAP);
		W2BYTEMSK(REG_004C_MODD1, 0x05, REG_004C_MODD1_REG_FREE_CH07_SWAP);
		W2BYTEMSK(REG_0050_MODD1, 0x06, REG_0050_MODD1_REG_FREE_CH08_SWAP);
		W2BYTEMSK(REG_0050_MODD1, 0x07, REG_0050_MODD1_REG_FREE_CH09_SWAP);
		W2BYTEMSK(REG_0054_MODD1, 0x08, REG_0054_MODD1_REG_FREE_CH10_SWAP);
		W2BYTEMSK(REG_0054_MODD1, 0x09, REG_0054_MODD1_REG_FREE_CH11_SWAP);
		W2BYTEMSK(REG_0058_MODD1, 0x0A, REG_0058_MODD1_REG_FREE_CH12_SWAP);
		W2BYTEMSK(REG_0058_MODD1, 0x0B, REG_0058_MODD1_REG_FREE_CH13_SWAP);
	}

	if (priv->linktype == E_LINK_VB1) {
		if (priv->lane_info.sup_lanes == 20 || priv->lane_info.sup_lanes == 40
		    || priv->lane_info.sup_lanes == 64) {
			W2BYTEMSK(REG_0040_MODD1, priv->lane_info.def_layout[0],
				  REG_0040_MODD1_REG_FREE_CH00_SWAP);
			W2BYTEMSK(REG_0040_MODD1, priv->lane_info.def_layout[1],
				  REG_0040_MODD1_REG_FREE_CH01_SWAP);
			W2BYTEMSK(REG_0044_MODD1, priv->lane_info.def_layout[2],
				  REG_0044_MODD1_REG_FREE_CH02_SWAP);
			W2BYTEMSK(REG_0044_MODD1, priv->lane_info.def_layout[3],
				  REG_0044_MODD1_REG_FREE_CH03_SWAP);
			W2BYTEMSK(REG_0048_MODD1, priv->lane_info.def_layout[4],
				  REG_0048_MODD1_REG_FREE_CH04_SWAP);
			W2BYTEMSK(REG_0048_MODD1, priv->lane_info.def_layout[5],
				  REG_0048_MODD1_REG_FREE_CH05_SWAP);
			W2BYTEMSK(REG_004C_MODD1, priv->lane_info.def_layout[6],
				  REG_004C_MODD1_REG_FREE_CH06_SWAP);
			W2BYTEMSK(REG_004C_MODD1, priv->lane_info.def_layout[7],
				  REG_004C_MODD1_REG_FREE_CH07_SWAP);
			W2BYTEMSK(REG_0050_MODD1, priv->lane_info.def_layout[8],
				  REG_0050_MODD1_REG_FREE_CH08_SWAP);
			W2BYTEMSK(REG_0050_MODD1, priv->lane_info.def_layout[9],
				  REG_0050_MODD1_REG_FREE_CH09_SWAP);
			W2BYTEMSK(REG_0054_MODD1, priv->lane_info.def_layout[10],
				  REG_0054_MODD1_REG_FREE_CH10_SWAP);
			W2BYTEMSK(REG_0054_MODD1, priv->lane_info.def_layout[11],
				  REG_0054_MODD1_REG_FREE_CH11_SWAP);
			W2BYTEMSK(REG_0058_MODD1, priv->lane_info.def_layout[12],
				  REG_0058_MODD1_REG_FREE_CH12_SWAP);
			W2BYTEMSK(REG_0058_MODD1, priv->lane_info.def_layout[13],
				  REG_0058_MODD1_REG_FREE_CH13_SWAP);
			W2BYTEMSK(REG_005C_MODD1, priv->lane_info.def_layout[14],
				  REG_005C_MODD1_REG_FREE_CH14_SWAP);
			W2BYTEMSK(REG_005C_MODD1, priv->lane_info.def_layout[15],
				  REG_005C_MODD1_REG_FREE_CH15_SWAP);
			W2BYTEMSK(REG_0060_MODD1, priv->lane_info.def_layout[16],
				  REG_0060_MODD1_REG_FREE_CH16_SWAP);
			W2BYTEMSK(REG_0060_MODD1, priv->lane_info.def_layout[17],
				  REG_0060_MODD1_REG_FREE_CH17_SWAP);
			W2BYTEMSK(REG_0064_MODD1, priv->lane_info.def_layout[18],
				  REG_0064_MODD1_REG_FREE_CH18_SWAP);
			W2BYTEMSK(REG_0064_MODD1, priv->lane_info.def_layout[19],
				  REG_0064_MODD1_REG_FREE_CH19_SWAP);
			if ((priv->lane_info.sup_lanes == 40) || (priv->lane_info.sup_lanes == 64)) {
				W2BYTEMSK(REG_0068_MODD1, priv->lane_info.def_layout[20],
					  REG_0068_MODD1_REG_FREE_CH20_SWAP);
				W2BYTEMSK(REG_0068_MODD1, priv->lane_info.def_layout[21],
					  REG_0068_MODD1_REG_FREE_CH21_SWAP);
				W2BYTEMSK(REG_006C_MODD1, priv->lane_info.def_layout[22],
					  REG_006C_MODD1_REG_FREE_CH22_SWAP);
				W2BYTEMSK(REG_006C_MODD1, priv->lane_info.def_layout[23],
					  REG_006C_MODD1_REG_FREE_CH23_SWAP);
				W2BYTEMSK(REG_0070_MODD1, priv->lane_info.def_layout[24],
					  REG_0070_MODD1_REG_FREE_CH24_SWAP);
				W2BYTEMSK(REG_0070_MODD1, priv->lane_info.def_layout[25],
					  REG_0070_MODD1_REG_FREE_CH25_SWAP);
				W2BYTEMSK(REG_0074_MODD1, priv->lane_info.def_layout[26],
					  REG_0074_MODD1_REG_FREE_CH26_SWAP);
				W2BYTEMSK(REG_0074_MODD1, priv->lane_info.def_layout[27],
					  REG_0074_MODD1_REG_FREE_CH27_SWAP);
				W2BYTEMSK(REG_0078_MODD1, priv->lane_info.def_layout[28],
					  REG_0078_MODD1_REG_FREE_CH28_SWAP);
				W2BYTEMSK(REG_0078_MODD1, priv->lane_info.def_layout[29],
					  REG_0078_MODD1_REG_FREE_CH29_SWAP);
				W2BYTEMSK(REG_007C_MODD1, priv->lane_info.def_layout[30],
					  REG_007C_MODD1_REG_FREE_CH30_SWAP);
				W2BYTEMSK(REG_007C_MODD1, priv->lane_info.def_layout[31],
					  REG_007C_MODD1_REG_FREE_CH31_SWAP);
				W2BYTEMSK(REG_0080_MODD1, priv->lane_info.def_layout[32],
					  REG_0080_MODD1_REG_FREE_CH32_SWAP);
				W2BYTEMSK(REG_0080_MODD1, priv->lane_info.def_layout[33],
					  REG_0080_MODD1_REG_FREE_CH33_SWAP);
				W2BYTEMSK(REG_0084_MODD1, priv->lane_info.def_layout[34],
					  REG_0084_MODD1_REG_FREE_CH34_SWAP);
				W2BYTEMSK(REG_0084_MODD1, priv->lane_info.def_layout[35],
					  REG_0084_MODD1_REG_FREE_CH35_SWAP);
				W2BYTEMSK(REG_0088_MODD1, priv->lane_info.def_layout[36],
					  REG_0088_MODD1_REG_FREE_CH36_SWAP);
				W2BYTEMSK(REG_0088_MODD1, priv->lane_info.def_layout[37],
					  REG_0088_MODD1_REG_FREE_CH37_SWAP);
				W2BYTEMSK(REG_008C_MODD1, priv->lane_info.def_layout[38],
					  REG_008C_MODD1_REG_FREE_CH38_SWAP);
				W2BYTEMSK(REG_008C_MODD1, priv->lane_info.def_layout[39],
					  REG_008C_MODD1_REG_FREE_CH39_SWAP);
			}
			if (priv->lane_info.sup_lanes == 64) {
				W2BYTEMSK(REG_0090_MODD1, priv->lane_info.def_layout[40],
					  REG_0090_MODD1_REG_FREE_CH40_SWAP);
				W2BYTEMSK(REG_0090_MODD1, priv->lane_info.def_layout[41],
					  REG_0090_MODD1_REG_FREE_CH41_SWAP);
				W2BYTEMSK(REG_0094_MODD1, priv->lane_info.def_layout[42],
					  REG_0094_MODD1_REG_FREE_CH42_SWAP);
				W2BYTEMSK(REG_0094_MODD1, priv->lane_info.def_layout[43],
					  REG_0094_MODD1_REG_FREE_CH43_SWAP);
				W2BYTEMSK(REG_0098_MODD1, priv->lane_info.def_layout[44],
					  REG_0098_MODD1_REG_FREE_CH44_SWAP);
				W2BYTEMSK(REG_0098_MODD1, priv->lane_info.def_layout[45],
					  REG_0098_MODD1_REG_FREE_CH45_SWAP);
				W2BYTEMSK(REG_009C_MODD1, priv->lane_info.def_layout[46],
					  REG_009C_MODD1_REG_FREE_CH46_SWAP);
				W2BYTEMSK(REG_009C_MODD1, priv->lane_info.def_layout[47],
					  REG_009C_MODD1_REG_FREE_CH47_SWAP);
				W2BYTEMSK(REG_00A0_MODD1, priv->lane_info.def_layout[48],
					  REG_00A0_MODD1_REG_FREE_CH48_SWAP);
				W2BYTEMSK(REG_00A0_MODD1, priv->lane_info.def_layout[49],
					  REG_00A0_MODD1_REG_FREE_CH49_SWAP);
				W2BYTEMSK(REG_00A4_MODD1, priv->lane_info.def_layout[50],
					  REG_00A4_MODD1_REG_FREE_CH50_SWAP);
				W2BYTEMSK(REG_00A4_MODD1, priv->lane_info.def_layout[51],
					  REG_00A4_MODD1_REG_FREE_CH51_SWAP);
				W2BYTEMSK(REG_00A8_MODD1, priv->lane_info.def_layout[52],
					  REG_00A8_MODD1_REG_FREE_CH52_SWAP);
				W2BYTEMSK(REG_00A8_MODD1, priv->lane_info.def_layout[53],
					  REG_00A8_MODD1_REG_FREE_CH53_SWAP);
				W2BYTEMSK(REG_00AC_MODD1, priv->lane_info.def_layout[54],
					  REG_00AC_MODD1_REG_FREE_CH54_SWAP);
				W2BYTEMSK(REG_00AC_MODD1, priv->lane_info.def_layout[55],
					  REG_00AC_MODD1_REG_FREE_CH55_SWAP);
				W2BYTEMSK(REG_00B0_MODD1, priv->lane_info.def_layout[56],
					  REG_00B0_MODD1_REG_FREE_CH56_SWAP);
				W2BYTEMSK(REG_00B0_MODD1, priv->lane_info.def_layout[57],
					  REG_00B0_MODD1_REG_FREE_CH57_SWAP);
				W2BYTEMSK(REG_00B4_MODD1, priv->lane_info.def_layout[58],
					  REG_00B4_MODD1_REG_FREE_CH58_SWAP);
				W2BYTEMSK(REG_00B4_MODD1, priv->lane_info.def_layout[59],
					  REG_00B4_MODD1_REG_FREE_CH59_SWAP);
				W2BYTEMSK(REG_00B8_MODD1, priv->lane_info.def_layout[60],
					  REG_00B8_MODD1_REG_FREE_CH60_SWAP);
				W2BYTEMSK(REG_00B8_MODD1, priv->lane_info.def_layout[61],
					  REG_00B8_MODD1_REG_FREE_CH61_SWAP);
				W2BYTEMSK(REG_00BC_MODD1, priv->lane_info.def_layout[62],
					  REG_00BC_MODD1_REG_FREE_CH62_SWAP);
				W2BYTEMSK(REG_00BC_MODD1, priv->lane_info.def_layout[63],
					  REG_00BC_MODD1_REG_FREE_CH63_SWAP);
			}
		}
	}
}

static void _usr_def_free_swap_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->linktype == E_LINK_VB1) {
		W2BYTEMSK(REG_01A0_MODV12, priv->lane_info.lane_order[0],
			  REG_01A0_MODV12_REG_VBY1_SWAP_CH00);
		W2BYTEMSK(REG_01A0_MODV12, priv->lane_info.lane_order[1],
			  REG_01A0_MODV12_REG_VBY1_SWAP_CH01);

		W2BYTEMSK(REG_01A4_MODV12, priv->lane_info.lane_order[2],
			  REG_01A4_MODV12_REG_VBY1_SWAP_CH02);
		W2BYTEMSK(REG_01A4_MODV12, priv->lane_info.lane_order[3],
			  REG_01A4_MODV12_REG_VBY1_SWAP_CH03);

		W2BYTEMSK(REG_01A8_MODV12, priv->lane_info.lane_order[4],
			  REG_01A8_MODV12_REG_VBY1_SWAP_CH04);
		W2BYTEMSK(REG_01A8_MODV12, priv->lane_info.lane_order[5],
			  REG_01A8_MODV12_REG_VBY1_SWAP_CH05);

		W2BYTEMSK(REG_01AC_MODV12, priv->lane_info.lane_order[6],
			  REG_01AC_MODV12_REG_VBY1_SWAP_CH06);
		W2BYTEMSK(REG_01AC_MODV12, priv->lane_info.lane_order[7],
			  REG_01AC_MODV12_REG_VBY1_SWAP_CH07);

		W2BYTEMSK(REG_01B0_MODV12, priv->lane_info.lane_order[8],
			  REG_01B0_MODV12_REG_VBY1_SWAP_CH08);
		W2BYTEMSK(REG_01B0_MODV12, priv->lane_info.lane_order[9],
			  REG_01B0_MODV12_REG_VBY1_SWAP_CH09);

		W2BYTEMSK(REG_01B4_MODV12, priv->lane_info.lane_order[10],
			  REG_01B4_MODV12_REG_VBY1_SWAP_CH10);
		W2BYTEMSK(REG_01B4_MODV12, priv->lane_info.lane_order[11],
			  REG_01B4_MODV12_REG_VBY1_SWAP_CH11);
		W2BYTEMSK(REG_01B8_MODV12, priv->lane_info.lane_order[12],
			  REG_01B8_MODV12_REG_VBY1_SWAP_CH12);
		W2BYTEMSK(REG_01B8_MODV12, priv->lane_info.lane_order[13],
			  REG_01B8_MODV12_REG_VBY1_SWAP_CH13);
		W2BYTEMSK(REG_01BC_MODV12, priv->lane_info.lane_order[14],
			  REG_01BC_MODV12_REG_VBY1_SWAP_CH14);
		W2BYTEMSK(REG_01BC_MODV12, priv->lane_info.lane_order[15],
			  REG_01BC_MODV12_REG_VBY1_SWAP_CH15);
		W2BYTEMSK(REG_01C0_MODV12, priv->lane_info.lane_order[16],
			  REG_01C0_MODV12_REG_VBY1_SWAP_CH16);
		W2BYTEMSK(REG_01C0_MODV12, priv->lane_info.lane_order[17],
			  REG_01C0_MODV12_REG_VBY1_SWAP_CH17);
		W2BYTEMSK(REG_01C4_MODV12, priv->lane_info.lane_order[18],
			  REG_01C4_MODV12_REG_VBY1_SWAP_CH18);
		W2BYTEMSK(REG_01C4_MODV12, priv->lane_info.lane_order[19],
			  REG_01C4_MODV12_REG_VBY1_SWAP_CH19);

		W2BYTEMSK(REG_01C8_MODV12, priv->lane_info.lane_order[20],
			  REG_01C8_MODV12_REG_VBY1_SWAP_CH20);
		W2BYTEMSK(REG_01C8_MODV12, priv->lane_info.lane_order[21],
			  REG_01C8_MODV12_REG_VBY1_SWAP_CH21);
		W2BYTEMSK(REG_01CC_MODV12, priv->lane_info.lane_order[22],
			  REG_01CC_MODV12_REG_VBY1_SWAP_CH22);
		W2BYTEMSK(REG_01CC_MODV12, priv->lane_info.lane_order[23],
			  REG_01CC_MODV12_REG_VBY1_SWAP_CH23);

		W2BYTEMSK(REG_01D0_MODV12, priv->lane_info.lane_order[24],
			  REG_01D0_MODV12_REG_VBY1_SWAP_CH24);
		W2BYTEMSK(REG_01D0_MODV12, priv->lane_info.lane_order[25],
			  REG_01D0_MODV12_REG_VBY1_SWAP_CH25);
		W2BYTEMSK(REG_01D4_MODV12, priv->lane_info.lane_order[26],
			  REG_01D4_MODV12_REG_VBY1_SWAP_CH26);
		W2BYTEMSK(REG_01D4_MODV12, priv->lane_info.lane_order[27],
			  REG_01D4_MODV12_REG_VBY1_SWAP_CH27);
		W2BYTEMSK(REG_01D8_MODV12, priv->lane_info.lane_order[28],
			  REG_01D8_MODV12_REG_VBY1_SWAP_CH28);
		W2BYTEMSK(REG_01D8_MODV12, priv->lane_info.lane_order[29],
			  REG_01D8_MODV12_REG_VBY1_SWAP_CH29);
		W2BYTEMSK(REG_01DC_MODV12, priv->lane_info.lane_order[30],
			  REG_01DC_MODV12_REG_VBY1_SWAP_CH30);
		W2BYTEMSK(REG_01DC_MODV12, priv->lane_info.lane_order[31],
			  REG_01DC_MODV12_REG_VBY1_SWAP_CH31);

		W2BYTEMSK(REG_01A0_MODOSD2, priv->lane_info.lane_order[32],
			  REG_01A0_MODOSD2_REG_VBY1_SWAP_CH00);
		W2BYTEMSK(REG_01A0_MODOSD2, priv->lane_info.lane_order[33],
			  REG_01A0_MODOSD2_REG_VBY1_SWAP_CH01);
		W2BYTEMSK(REG_01A4_MODOSD2, priv->lane_info.lane_order[34],
			  REG_01A4_MODOSD2_REG_VBY1_SWAP_CH02);
		W2BYTEMSK(REG_01A4_MODOSD2, priv->lane_info.lane_order[35],
			  REG_01A4_MODOSD2_REG_VBY1_SWAP_CH03);
		W2BYTEMSK(REG_01A8_MODOSD2, priv->lane_info.lane_order[36],
			  REG_01A8_MODOSD2_REG_VBY1_SWAP_CH04);
		W2BYTEMSK(REG_01A8_MODOSD2, priv->lane_info.lane_order[37],
			  REG_01A8_MODOSD2_REG_VBY1_SWAP_CH05);
		W2BYTEMSK(REG_01AC_MODOSD2, priv->lane_info.lane_order[38],
			  REG_01AC_MODOSD2_REG_VBY1_SWAP_CH06);
		W2BYTEMSK(REG_01AC_MODOSD2, priv->lane_info.lane_order[39],
			  REG_01AC_MODOSD2_REG_VBY1_SWAP_CH07);


	}
}

static void _vby1_8lane_def_free_swap_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	// 4K60Hz 8lane no need to change laneorder based on div_sec
	if (priv->div_sec) {
		W2BYTEMSK(REG_01A0_MODV12, 0x0100, Fld(16, 0, AC_FULLW10));	//ch0-ch1
		W2BYTEMSK(REG_01A4_MODV12, 0x0302, Fld(16, 0, AC_FULLW10));	//ch2-ch3
		W2BYTEMSK(REG_01A8_MODV12, 0x0504, Fld(16, 0, AC_FULLW10));	//ch4-ch5
		W2BYTEMSK(REG_01AC_MODV12, 0x0706, Fld(16, 0, AC_FULLW10));	//ch6-ch7
	}
}

static void _default_free_swap_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->linktype != E_LINK_VB1) {
		printf("[%s] only support E_LINK_VB1\n", __func__);
		return;
	}
	_vby1_8lane_def_free_swap_v006(dev);
}

void _hfrc_setting_for_HAPS_v006(void)
{
	// Bank_a02f_fa: frc_haps
	//wriu -w 0xa02f02 0x0898 // tg_htt      = 16'h898, 16'd2200
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x4, 0x898, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f04 0x0000 // tg_hs_st    = 16'h000
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x8, 0x0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f06 0x000a // tg_hs_end   = 16'h00a
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xC, 0xA, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f08 0x0070 // tg_hfde_st  = 16'h070, 16'd112
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x10, 0x70, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f0a 0x042F // tg_hfde_end = 16'h42F, 16'd1071
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x14, 0x42F, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f0c 0x0070 // tg_hde_st   = 16'h070, 16'd112
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x18, 0x70, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f0e 0x042F // tg_hde_end  = 16'h42F, 16'd1071
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x1C, 0x42F, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f10 0x0465 // tg_vtt      = 16'h465, 16'd1125
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x20, 0x465, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f12 0x0000 // tg_vs_st    = 16'h000
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x24, 0x0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f14 0x0006 // tg_vs_end   = 16'h006
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x28, 0x6, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f16 0x0009 // tg_vfde_st  = 16'h9,   16'd9
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x2C, 0x9, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f18 0x0440 // tg_vfde_end = 16'h440, 16'd1088
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x30, 0x440, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f1a 0x0009 // tg_vde_st   = 16'h9,   16'd9
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x34, 0x9, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f1c 0x0440 // tg_vde_end  = 16'h440, 16'd1088
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x38, 0x440, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f24 0x0002 // tg_frc_vs_st   = 16'h002
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x48, 0x2, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f26 0x000a // tg_frc_vs_end  = 16'h00a
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0x4C, 0xA, Fld(16, 0, AC_FULLW10));

	//wriu -w 0xa02fAE 0x0001 // pure_color_en[12], ip_patgen_en[4], op_patgen_en[0]

	//wriu -w 0xa02f64 0x0800 // scmi_size  = 20'h 70800 (1280*720/2)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xC8, 0x800, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f66 0x0007 // scmi_size  = 20'h 70800 (1280*720/2)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xCC, 0x7, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f68 0x03c0 // lb_size    = 12'h 3c0   (960)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xD0, 0x3C0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f6a 0x0800 // reg_2pto1p, reg1pto2p, fake444, 42to44_mode[1:0] = 16'h0800
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xD4, 0x800, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f76 0x0280 // scmi_hsize = 12'h 280   (640)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xEC, 0x280, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f78 0x0780 // ip_hsize   = 12'h 780   (1920)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xF0, 0x780, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f7a 0x03c0 // op_hsize   = 12'h 3c0   (960)
	W2BYTEMSK(REG_HFRC_HAPS_BASE + 0xF4, 0x3C0, Fld(16, 0, AC_FULLW10));


	//Bank_a02f_fb: frc_haps_hvsp_ip
	//wriu -w 0xa02f0e 0x0000 // reg_scale_factor_ho        = 24'h300000
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x1C, 0x0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f10 0x0130 // reg_scale_ho_en=1, reg_scale_factor_ho  = 24'h300000
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x20, 0x130, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f12 0x0000 // reg_scale_factor_ve        = 24'h300000
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x24, 0x0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f14 0x0130 // reg_scale_ve_en=1, reg_scale_factor_ve  = 24'h300000
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x28, 0x130, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f16 0x42c2 // op_mode
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x2C, 0x42C2, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f50 0x8F00 // reg_h_size_usr_md[15] = 1'd1 , 1'd0, reg_h_size[13:0] = 14'hF00(3840)
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0xA0, 0x8F00, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f52 0x8870 // reg_v_size_usr_md[15] = 1'd1 , 1'd0, reg_v_size[13:0] = 14'h870(2160)
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0xA4, 0x8870, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f54 0x0500 // reg_scl_h_size = 13'h0500 (1280)
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0xA8, 0x500, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f56 0x02d0 // reg_scl_v_size = 13'h02d0 (720)
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0xAC, 0x2D0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02fc0 0x0000 // reg_bypass = 0
	W2BYTEMSK(REG_HFRC_HVSP_IP_BASE + 0x180, 0x0, Fld(16, 0, AC_FULLW10));


	//Bank_a02f_f4: frc_haps_hvsp_op
	//wriu -w 0xa02f0e 0xaaaa // reg_scale_factor_ho        = 24'h0AAAAA
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x1C, 0xAAAA, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f10 0x010a // reg_scale_ho_en=1, reg_scale_factor_ho  = 24'h0AAAAA
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x20, 0x10A, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f12 0xaaaa // reg_scale_factor_ve        = 24'h0AAAAA
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x24, 0xAAAA, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f14 0x010a // reg_scale_ve_en=1, reg_scale_factor_ve  = 24'h0AAAAA
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x28, 0x10A, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f16 0x42c2 // op_mode
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x2C, 0x42C2, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f50 0x8500 // reg_h_size_usr_md[15] = 1'd1 , 1'd0, reg_h_size[13:0] = 14'h500(1280)
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0xA0, 0x8500, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f52 0x82d0 // reg_v_size_usr_md[15] = 1'd1 , 1'd0, reg_v_size[13:0] = 14'h2d0(720)
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0xA4, 0x82D0, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f54 0x0780 // reg_scl_h_size = 13'h0780 (1920)
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0xA8, 0x780, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02f56 0x0438 // reg_scl_v_size = 13'h0438 (1080)
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0xAC, 0x438, Fld(16, 0, AC_FULLW10));
	//wriu -w 0xa02fc0 0x0000 // reg_bypass = 0utt
	W2BYTEMSK(REG_HFRC_HVSP_OP_BASE + 0x180, 0x0, Fld(16, 0, AC_FULLW10));

}

void _vby1_lane_order_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	//ping mapping free swap
	_pin_mapping_free_swap_v006(dev);

	//read lane order from device tree
	if (priv->linktype != E_LINK_NONE) {
		if (priv->out_timing == E_FHD_60HZ ||
			priv->out_timing == E_HD_60HZ ||
			priv->out_timing == E_HD_120HZ)
			W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		else if (priv->out_timing == E_FHD_120HZ)
			W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		else if (priv->out_timing == E_4K2K_60HZ || priv->out_timing == E_4K1K_120HZ)
			W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		else if (priv->out_timing == E_4K2K_120HZ ||
			 priv->out_timing == E_4K2K_144HZ || priv->out_timing == E_4K1K_240HZ)
			W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		else if (priv->out_timing == E_8K4K_60HZ && priv->out_format == E_OUTPUT_YUV422)
			W2BYTEMSK(REG_0008_MODV12, 0x0, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		else if (priv->out_timing == E_8K4K_60HZ) {
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_47_32);
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		} else if (priv->out_timing == E_8K4K_120HZ) {
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_63_48);
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_47_32);
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		} else if (priv->out_timing == E_8K4K_144HZ) {
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_63_48);
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_47_32);
			W2BYTEMSK(REG_0008_MODV12, 0x1, REG_0008_MODV12_REG_VBY1_16CH_CKEN_31_16);
		} else {
			debug("[%s] OUTPUT TIMING Not Support\n", __func__);
		}
		debug("[%s] User defined lane order = %d\n", __func__, priv->lane_info.usr_defined);

		if (priv->lane_info.usr_defined == 1)
			_usr_def_free_swap_v006(dev);
		else
			_default_free_swap_v006(dev);

	}
#ifdef CONFIG_HAPS
	//polarity , ti mode
	W2BYTEMSK(REG_00E0_MODD1, 0x2800, Fld(16, 0, AC_FULLW10));

	//free swap for a/b swap
	W2BYTEMSK(REG_0040_MODD1, 0x0706, Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(REG_0044_MODD1, 0x0908, Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(REG_0048_MODD1, 0x0B0A, Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(REG_004C_MODD1, 0x0100, Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(REG_0050_MODD1, 0x0302, Fld(16, 0, AC_FULLW10));
	W2BYTEMSK(REG_0054_MODD1, 0x0504, Fld(16, 0, AC_FULLW10));
#endif

}

int mtk_out_if_init_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}
	//clk setting

	if (priv->linktype == E_LINK_VB1) {
		switch (priv->out_timing) {

		case E_FHD_60HZ:
			_vby1_v_fhd_60HZ_v006(dev);
			break;
		case E_FHD_120HZ:
			_vby1_v_fhd_120HZ_v006(dev);
			break;
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
			_vby1_v_4k_60HZ_v006(dev);
			break;
		default:
			UBOOT_DEBUG("Video OUTPUT TIMING Not Support\n");
			return -EINVAL;
			break;
		}

		//ToDo:Add Fixed H-backporch code.
		//VBY1 byte-mode
		_vby1_set_byte_mode_v006(dev);
	}

	if (priv->linktype == E_LINK_LVDS) {
		switch (priv->out_timing) {
		case E_HD_60HZ:
			_lvds_v_hd_60HZ_v006(dev);
		break;
		case E_HD_120HZ:
			_lvds_v_hd_120HZ_v006(dev);
		break;
		case E_FHD_60HZ:
			_lvds_v_fhd_60HZ_v006(dev);
		break;
		default:
			UBOOT_DEBUG("Video OUTPUT TIMING Not Support\n");
			return -EINVAL;
		break;
		}
	}
	_vby1_set_mft_v006(dev);
	_vby1_set_SCTCON_MISC_v006(dev);

	//h/v sync polarity
	W2BYTEMSK(REG_0004_MODV12, priv->hsync_pol, REG_0004_MODV12_REG_VBY1_HS_INV);
	W2BYTEMSK(REG_0004_MODV12, priv->vsync_pol, REG_0004_MODV12_REG_VBY1_VS_INV);

	_vby1_seri_data_sel_v006(dev);

	//lane order setting
	_vby1_lane_order_v006(dev);

	//fifo reset (video path)
	W2BYTEMSK(REG_0150_MODD1, 0x1, REG_0150_MODD1_REG_V1_SW_FORCE_FIX_TRIG);
	W2BYTEMSK(REG_0150_MODD1, 0x0, REG_0150_MODD1_REG_V1_SW_FORCE_FIX_TRIG);

#ifdef CONFIG_HAPS
	UBOOT_DEBUG("!!!! HAPS case !!!!!!\n");
	_hfrc_setting_for_HAPS_v006();
#endif

	return 0;
}

static void _efuse_set_rcon_v006(struct udevice *dev, uint32_t value)
{
	uint16_t rcon_val = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	bool rcon_en = 0;
	uint32_t rcon_max = 0;
	uint32_t rcon_min = 0;
	uint32_t rcon_default_value = 0;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	rcon_en = priv->hw_info.rcon_enable;
	rcon_max = priv->hw_info.rcon_max;
	rcon_min = priv->hw_info.rcon_min;
	rcon_default_value = priv->hw_info.rcon_value;

	//check act bit is enabled
	if (value & _BIT(MOD_VOL_VX1_ACT_BIT_V006)) {

		rcon_val = value & 0x3F;	//bit[5:0]

		UBOOT_DEBUG("rcon value read from efuse is %d\n", rcon_val);

		//check if out of boundary
		if (rcon_en) {
			if (rcon_val > rcon_max || rcon_val < rcon_min)
				rcon_val = rcon_default_value;
		} else {
			rcon_val = rcon_default_value;
		}
	} else {
		rcon_val = rcon_default_value;
		UBOOT_DEBUG("RCON act value is set to default value [%d]\n", RCON_DEF_VAL);
	}

	W2BYTEMSK(REG_00E0_MODA1, rcon_val, REG_00E0_MODA1_REG_GCR_RCON_CH00);
	W2BYTEMSK(REG_00E0_MODA1, rcon_val, REG_00E0_MODA1_REG_GCR_RCON_CH01);
	W2BYTEMSK(REG_00E4_MODA1, rcon_val, REG_00E4_MODA1_REG_GCR_RCON_CH02);
	W2BYTEMSK(REG_00E4_MODA1, rcon_val, REG_00E4_MODA1_REG_GCR_RCON_CH03);
	W2BYTEMSK(REG_00E8_MODA1, rcon_val, REG_00E8_MODA1_REG_GCR_RCON_CH04);
	W2BYTEMSK(REG_00E8_MODA1, rcon_val, REG_00E8_MODA1_REG_GCR_RCON_CH05);
	W2BYTEMSK(REG_00EC_MODA1, rcon_val, REG_00EC_MODA1_REG_GCR_RCON_CH06);
	W2BYTEMSK(REG_00EC_MODA1, rcon_val, REG_00EC_MODA1_REG_GCR_RCON_CH07);
	W2BYTEMSK(REG_00F0_MODA1, rcon_val, REG_00F0_MODA1_REG_GCR_RCON_CH08);
	W2BYTEMSK(REG_00F0_MODA1, rcon_val, REG_00F0_MODA1_REG_GCR_RCON_CH09);
	W2BYTEMSK(REG_00F4_MODA1, rcon_val, REG_00F4_MODA1_REG_GCR_RCON_CH10);
	W2BYTEMSK(REG_00F4_MODA1, rcon_val, REG_00F4_MODA1_REG_GCR_RCON_CH11);
	W2BYTEMSK(REG_00F8_MODA1, rcon_val, REG_00F8_MODA1_REG_GCR_RCON_CH12);
	W2BYTEMSK(REG_00F8_MODA1, rcon_val, REG_00F8_MODA1_REG_GCR_RCON_CH13);
	W2BYTEMSK(REG_00FC_MODA1, rcon_val, REG_00FC_MODA1_REG_GCR_RCON_CH14);
	W2BYTEMSK(REG_00FC_MODA1, rcon_val, REG_00FC_MODA1_REG_GCR_RCON_CH15);
	W2BYTEMSK(REG_0100_MODA1, rcon_val, REG_0100_MODA1_REG_GCR_RCON_CH16);
	W2BYTEMSK(REG_0100_MODA1, rcon_val, REG_0100_MODA1_REG_GCR_RCON_CH17);
	W2BYTEMSK(REG_0104_MODA1, rcon_val, REG_0104_MODA1_REG_GCR_RCON_CH18);
	W2BYTEMSK(REG_0104_MODA1, rcon_val, REG_0104_MODA1_REG_GCR_RCON_CH19);
	W2BYTEMSK(REG_00E0_MODA2, rcon_val, REG_00E0_MODA2_REG_GCR_RCON_CH20);
	W2BYTEMSK(REG_00E0_MODA2, rcon_val, REG_00E0_MODA2_REG_GCR_RCON_CH21);
	W2BYTEMSK(REG_00E4_MODA2, rcon_val, REG_00E4_MODA2_REG_GCR_RCON_CH22);
	W2BYTEMSK(REG_00E4_MODA2, rcon_val, REG_00E4_MODA2_REG_GCR_RCON_CH23);
	W2BYTEMSK(REG_00E8_MODA2, rcon_val, REG_00E8_MODA2_REG_GCR_RCON_CH24);
	W2BYTEMSK(REG_00E8_MODA2, rcon_val, REG_00E8_MODA2_REG_GCR_RCON_CH25);
	W2BYTEMSK(REG_00EC_MODA2, rcon_val, REG_00EC_MODA2_REG_GCR_RCON_CH26);
	W2BYTEMSK(REG_00EC_MODA2, rcon_val, REG_00EC_MODA2_REG_GCR_RCON_CH27);
	W2BYTEMSK(REG_00F0_MODA2, rcon_val, REG_00F0_MODA2_REG_GCR_RCON_CH28);
	W2BYTEMSK(REG_00F0_MODA2, rcon_val, REG_00F0_MODA2_REG_GCR_RCON_CH29);
	W2BYTEMSK(REG_00F4_MODA2, rcon_val, REG_00F4_MODA2_REG_GCR_RCON_CH30);
	W2BYTEMSK(REG_00F4_MODA2, rcon_val, REG_00F4_MODA2_REG_GCR_RCON_CH31);
	W2BYTEMSK(REG_00F8_MODA2, rcon_val, REG_00F8_MODA2_REG_GCR_RCON_CH32);
	W2BYTEMSK(REG_00F8_MODA2, rcon_val, REG_00F8_MODA2_REG_GCR_RCON_CH33);
	W2BYTEMSK(REG_00FC_MODA2, rcon_val, REG_00FC_MODA2_REG_GCR_RCON_CH34);
	W2BYTEMSK(REG_00FC_MODA2, rcon_val, REG_00FC_MODA2_REG_GCR_RCON_CH35);
	W2BYTEMSK(REG_0100_MODA2, rcon_val, REG_0100_MODA2_REG_GCR_RCON_CH36);
	W2BYTEMSK(REG_0100_MODA2, rcon_val, REG_0100_MODA2_REG_GCR_RCON_CH37);
	W2BYTEMSK(REG_0104_MODA2, rcon_val, REG_0104_MODA2_REG_GCR_RCON_CH38);
	W2BYTEMSK(REG_0104_MODA2, rcon_val, REG_0104_MODA2_REG_GCR_RCON_CH39);
}

static void _efuse_set_biascon_v006(struct udevice *dev, uint32_t value, uint32_t value1,
				    uint32_t value2)
{
	//for vby1 case, default read double term

	bool bSingleterm = false;
	//uint16_t SingleActBitPosition = 0;
	//uint16_t SingleBiasConMask = 0;
	uint16_t DoubleActBitPosition = 0;
	//uint16_t DoubleBiasConMask = 0;
	uint16_t cur_val = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	//uint32_t bias_single_max = 0;
	//uint32_t bias_single_min = 0;
	//uint32_t bias_single_default_val = 0;
	uint32_t bias_double_max = 0;
	uint32_t bias_double_min = 0;
	uint32_t bias_double_default_val = 0;
	bool efuseActive = false;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	//bias_single_max = priv->hw_info.biascon_single_max;
	//bias_single_min = priv->hw_info.biascon_single_min;
	//bias_single_default_val = priv->hw_info.biascon_single_value;

	bias_double_max = priv->hw_info.biascon_double_max;
	bias_double_min = priv->hw_info.biascon_double_min;
	bias_double_default_val = priv->hw_info.biascon_double_value;

	bSingleterm = false;

	UBOOT_DEBUG("Current mode is %s\n", bSingleterm ? "Single Term" : "Double Term");

	DoubleActBitPosition = MOD_CUR_DOUBLE_ACT_BIT_V006;	//bit[16]
	//DoubleBiasConMask = MOD_CUR_DOUBLE_MASK_VER5;
	if ((value & _BIT(DoubleActBitPosition)) == _BIT(DoubleActBitPosition))
		efuseActive = true;
	else
		efuseActive = false;

	UBOOT_DEBUG("Active=%d BitPosition=%d bias_max=%d bias_min=%d bias_default=%d\n",
		    efuseActive, DoubleActBitPosition, bias_double_max, bias_double_min,
		    bias_double_default_val);

	if (efuseActive) {
		cur_val = (value >> MOD_CUR_DOUBLE_LSB_V006) & MOD_CUR_DOUBLE_MASK_V006;	//bit[23:17]
		UBOOT_DEBUG("Dump double term biasCon = %d from efuse\n", cur_val);
		if (cur_val > bias_double_max || cur_val < bias_double_min)
			cur_val = bias_double_default_val;
	} else {
		cur_val = bias_double_default_val;
		UBOOT_DEBUG("MOD CUR DOUBLE ACT is 0, set default value\n");
	}

	W2BYTEMSK(REG_0044_MODA1, cur_val, REG_0044_MODA1_REG_GCR_BIAS_CON);

	//set biascon offset
	//_efuse_set_biascon_offset(dev, value, value1, value2);
}

void mtk_dump_mod_efuse_v006(struct udevice *dev)
{
	uint32_t efuse_val = 0;
	uint32_t efuse_sub0C_val = 0;
	uint32_t efuse_sub0D_val = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	efuse_val = _mtk_get_efuse_val_v006(priv->pnl_lib_version, MOD_ATOP_SUBBANK);

	UBOOT_DEBUG("Efuse value = 0x%x\n", efuse_val);

	//dump setting from efuse
	_efuse_set_rcon_v006(dev, efuse_val);
	_efuse_set_biascon_v006(dev, efuse_val, efuse_sub0C_val, efuse_sub0D_val);

	//_efuse_set_rint(dev, efuse_val); //v006 without RINT
}

static void _mtk_dither_depth_setting_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}
	if (priv->dither_info.dither_capability == 0)	//input 10bit case
		switch (priv->dither_info.dither_depth) {
		case E_DITHER_DEPTH_6:
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_DITH_BITS);	//dither 4 bit (10to6 case)
			break;
		case E_DITHER_DEPTH_8:
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_DITH_BITS);	//dither 2 bit (10to8 case)
			break;
		case E_DITHER_DEPTH_10:
			//means by pass(10to10 case)
			priv->dither_info.dither_pattern = E_DITHER_BYPASS;
			break;
		case E_DITHER_DEPTH_12:
		default:
			// To avoid dither pattern works in not support case, force bypass mode
			priv->dither_info.dither_pattern = E_DITHER_BYPASS;
			debug("[%s] dither depth not support\n", __func__);
			break;
	} else if (priv->dither_info.dither_capability == 1) {	//input 12bit case
		switch (priv->dither_info.dither_depth) {
		case E_DITHER_DEPTH_6:
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_DITH_BITS);	//dither 4 bit (12to6 case)
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0,
				  REG_00FC_PAFRC_BKA324_REG_PAFRC_INPUT_IS_12B);
			break;
		case E_DITHER_DEPTH_8:
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_DITH_BITS);	//dither 4 bit (12to8 case)
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1,
				  REG_00FC_PAFRC_BKA324_REG_PAFRC_INPUT_IS_12B);
			break;
		case E_DITHER_DEPTH_10:
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_DITH_BITS);	//dither 2 bit (12to10 case)
			W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1,
				  REG_00FC_PAFRC_BKA324_REG_PAFRC_INPUT_IS_12B);
			break;
		case E_DITHER_DEPTH_12:
			//means by pass(12to12 case)
			priv->dither_info.dither_pattern = E_DITHER_BYPASS;
			break;
		default:
			// To avoid dither pattern works in not support case, force bypass mode
			priv->dither_info.dither_pattern = E_DITHER_BYPASS;
			debug("[%s] dither depth not support\n", __func__);
			break;
		}

	} else {
		debug("[%s] dither capability not support\n", __func__);
	}

}

static void _mtk_dither_12bit_setting_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_BOOST_EN);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_DITHER_BIT_1);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_LSB_ALIGNMENT_SEL);

	if ((priv->dither_info.dither_depth == E_DITHER_DEPTH_12)
	    || (priv->dither_info.dither_pattern == E_DITHER_BYPASS)) {
		W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_BYPASS_MODE);
	} else {
		W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_BYPASS_MODE);
	}

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_PAFRC_INPUT_IS_13B);

	W2BYTEMSK(REG_012C_PAFRC_BKA324, 0x0, REG_012C_PAFRC_BKA324_REG_13B_CLAMP_MODE);

}

static void _mtk_dither_common_setting_v006(void)
{
	//setting here means different dither pattern use the same setting

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_POL_TYPE);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_G_V_SWAP);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_G_H_SWAP);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_B_D_SWAP);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FR_SW);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX4X4_FR_SW);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX8X8_ROT_UNIT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_ROTATE_EN);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_TOP_BOX_UNIT_FLAG);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_SHRINK);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_FR_C2_BIT);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_C2X2_ROT_B_DIR_S);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_D2X2_ROT_B_DIR_S);

	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_A2X2_ROT_R_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_B2X2_ROT_R_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_C2X2_ROT_R_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_D2X2_ROT_R_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_C2X2_ROT_R_DIR_S);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_D2X2_ROT_R_DIR_S);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_A2X2_ROT_G_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_B2X2_ROT_G_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_C2X2_ROT_G_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_D2X2_ROT_G_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_C2X2_ROT_G_DIR_S);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_D2X2_ROT_G_DIR_S);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_A2X2_ROT_B_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x0, REG_0104_PAFRC_BKA324_REG_B2X2_ROT_B_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_C2X2_ROT_B_DIR);
	W2BYTEMSK(REG_0104_PAFRC_BKA324, 0x1, REG_0104_PAFRC_BKA324_REG_D2X2_ROT_B_DIR);

	W2BYTEMSK(REG_0108_PAFRC_BKA324, 0x9c, REG_0108_PAFRC_BKA324_REG_TOP_BOX_FR_SEQ1);
	W2BYTEMSK(REG_0108_PAFRC_BKA324, 0xc9, REG_0108_PAFRC_BKA324_REG_TOP_BOX_FR_SEQ2);

	W2BYTEMSK(REG_010C_PAFRC_BKA324, 0x9c, REG_010C_PAFRC_BKA324_REG_TOP_BOX_FR_SEQ3);
	W2BYTEMSK(REG_010C_PAFRC_BKA324, 0xc9, REG_010C_PAFRC_BKA324_REG_TOP_BOX_FR_SEQ4);

	W2BYTEMSK(REG_0110_PAFRC_BKA324, 0xaa, REG_0110_PAFRC_BKA324_REG_TOP_BOX_FR_C2_SEQ12);
	W2BYTEMSK(REG_0110_PAFRC_BKA324, 0xaa, REG_0110_PAFRC_BKA324_REG_TOP_BOX_FR_C2_SEQ34);

	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x0, REG_0114_PAFRC_BKA324_REG_BOX8X8_ROT_00);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x2, REG_0114_PAFRC_BKA324_REG_BOX8X8_ROT_01);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x2, REG_0114_PAFRC_BKA324_REG_BOX8X8_ROT_10);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x0, REG_0114_PAFRC_BKA324_REG_BOX8X8_ROT_11);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x0, REG_0114_PAFRC_BKA324_REG_BOX_A_ROT_DIR);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x1, REG_0114_PAFRC_BKA324_REG_BOX_B_ROT_DIR);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x0, REG_0114_PAFRC_BKA324_REG_BOX_C_ROT_DIR);
	W2BYTEMSK(REG_0114_PAFRC_BKA324, 0x1, REG_0114_PAFRC_BKA324_REG_BOX_D_ROT_DIR);
}

static void _mtk_dither_entity_same_setting_v006(void)
{
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_A_LU_00);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_A_RU_01);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_A_RD_11);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_A_LD_10);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_B_LU_00);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_B_RU_01);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_B_RD_11);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_B_LD_10);

	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_C_LU_00);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_C_RU_01);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_C_RD_11);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_C_LD_10);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_D_LU_00);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_D_RU_01);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_D_RD_11);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_D_LD_10);

	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_C_LU_00_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_C_RU_01_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_C_RD_11_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_C_LD_10_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_D_LU_00_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_D_RU_01_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_D_RD_11_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_D_LD_10_S);

	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_A_LU_00);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_A_RU_01);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_A_RD_11);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_A_LD_10);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_B_LU_00);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_B_RU_01);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_B_RD_11);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_B_LD_10);

	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_C_LU_00);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_C_RU_01);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_C_RD_11);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_C_LD_10);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_D_LU_00);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_D_RU_01);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_D_RD_11);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_D_LD_10);
}

static void _mtk_dither_bypass_setting_v006(void)
{

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);

	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_same_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_FRC_ON);

}

static void _mtk_dither_truncate_setting_v006(void)
{

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);	//only diff with bypass

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);

	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_same_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_FRC_ON);


}

static void _mtk_dither_round_setting_v006(void)
{

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);

	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_same_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_FRC_ON);	//only diff with truncate

}


static void _mtk_dither_entity_diff_setting_v006(void)
{
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x3, REG_0118_PAFRC_BKA324_REG_A_LU_00);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x1, REG_0118_PAFRC_BKA324_REG_A_RU_01);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_A_RD_11);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x0, REG_0118_PAFRC_BKA324_REG_A_LD_10);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x3, REG_0118_PAFRC_BKA324_REG_B_LU_00);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x1, REG_0118_PAFRC_BKA324_REG_B_RU_01);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x2, REG_0118_PAFRC_BKA324_REG_B_RD_11);
	W2BYTEMSK(REG_0118_PAFRC_BKA324, 0x0, REG_0118_PAFRC_BKA324_REG_B_LD_10);

	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x1, REG_011C_PAFRC_BKA324_REG_C_LU_00);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x3, REG_011C_PAFRC_BKA324_REG_C_RU_01);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x0, REG_011C_PAFRC_BKA324_REG_C_RD_11);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_C_LD_10);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x1, REG_011C_PAFRC_BKA324_REG_D_LU_00);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x3, REG_011C_PAFRC_BKA324_REG_D_RU_01);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x0, REG_011C_PAFRC_BKA324_REG_D_RD_11);
	W2BYTEMSK(REG_011C_PAFRC_BKA324, 0x2, REG_011C_PAFRC_BKA324_REG_D_LD_10);

	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_C_LU_00_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x0, REG_0120_PAFRC_BKA324_REG_C_RU_01_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x3, REG_0120_PAFRC_BKA324_REG_C_RD_11_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x1, REG_0120_PAFRC_BKA324_REG_C_LD_10_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x2, REG_0120_PAFRC_BKA324_REG_D_LU_00_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x0, REG_0120_PAFRC_BKA324_REG_D_RU_01_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x3, REG_0120_PAFRC_BKA324_REG_D_RD_11_S);
	W2BYTEMSK(REG_0120_PAFRC_BKA324, 0x1, REG_0120_PAFRC_BKA324_REG_D_LD_10_S);

	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x3, REG_0124_PAFRC_BKA324_REG_BOX_A_LU_00);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x1, REG_0124_PAFRC_BKA324_REG_BOX_A_RU_01);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_A_RD_11);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x0, REG_0124_PAFRC_BKA324_REG_BOX_A_LD_10);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x0, REG_0124_PAFRC_BKA324_REG_BOX_B_LU_00);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x2, REG_0124_PAFRC_BKA324_REG_BOX_B_RU_01);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x1, REG_0124_PAFRC_BKA324_REG_BOX_B_RD_11);
	W2BYTEMSK(REG_0124_PAFRC_BKA324, 0x3, REG_0124_PAFRC_BKA324_REG_BOX_B_LD_10);

	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_C_LU_00);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x0, REG_0128_PAFRC_BKA324_REG_BOX_C_RU_01);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x3, REG_0128_PAFRC_BKA324_REG_BOX_C_RD_11);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x1, REG_0128_PAFRC_BKA324_REG_BOX_C_LD_10);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x1, REG_0128_PAFRC_BKA324_REG_BOX_D_LU_00);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x3, REG_0128_PAFRC_BKA324_REG_BOX_D_RU_01);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x0, REG_0128_PAFRC_BKA324_REG_BOX_D_RD_11);
	W2BYTEMSK(REG_0128_PAFRC_BKA324, 0x2, REG_0128_PAFRC_BKA324_REG_BOX_D_LD_10);


}

static void _mtk_dither_random_static_setting_v006(void)
{
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);

	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_diff_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_FRC_ON);


}

static void _mtk_dither_random_dynamic_setting_v006(void)
{
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x0, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);	//diff with random static

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);	//diff with random static

	_mtk_dither_common_setting_v006();

	_mtk_dither_entity_diff_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_FRC_ON);


}

static void _mtk_dither_pattern_table_static_setting_v006(void)
{
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);	//diff with random
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x1, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);

	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_diff_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_FRC_ON);


}

static void _mtk_dither_pattern_table_dynamic_setting_v006(void)
{
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_NOISE_DITH_DISABLE);	//diff with random
	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_PAFRC_TAIL_CUT);

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_BOX_FREEZE);	//diff with random static

	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_FREEZE);	//diff with random static
	W2BYTEMSK(REG_0100_PAFRC_BKA324, 0x0, REG_0100_PAFRC_BKA324_REG_TOP_BOX_SHRINK);
	_mtk_dither_common_setting_v006();
	_mtk_dither_entity_diff_setting_v006();

	W2BYTEMSK(REG_00FC_PAFRC_BKA324, 0x1, REG_00FC_PAFRC_BKA324_REG_FRC_ON);
}

void mtk_dither_setting_v006(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	_mtk_dither_depth_setting_v006(dev);

	if (priv->dither_info.dither_capability == 1) {
		_mtk_dither_12bit_setting_v006(dev);
	}

	switch (priv->dither_info.dither_pattern) {
	case E_DITHER_BYPASS:
		_mtk_dither_bypass_setting_v006();
		break;
	case E_DITHER_TRUNCATION:
		_mtk_dither_truncate_setting_v006();
		break;
	case E_DITHER_ROUND:
		_mtk_dither_round_setting_v006();
		break;
	case E_DITHER_PATTERN_TABLE_DYNAMIC:
		_mtk_dither_pattern_table_dynamic_setting_v006();
		break;
	case E_DITHER_PATTERN_TABLE_STATIC:
		_mtk_dither_pattern_table_static_setting_v006();
		break;
	case E_DITHER_RANDOM_DYNAMIC:
		_mtk_dither_random_dynamic_setting_v006();
		break;
	case E_DITHER_RANDOM_STATIC:
		_mtk_dither_random_static_setting_v006();
		break;
	default:
		UBOOT_DEBUG("dither pattern fail\n");
		break;

	}
}
