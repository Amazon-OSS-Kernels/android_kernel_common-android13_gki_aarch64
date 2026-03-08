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
#include <div64.h>
#include "coda/moda1.h"
#include "coda/moda2.h"
#include "coda/moda3.h"
#include "coda/moda4.h"
#include "coda/lpll.h"
#include "coda/ckgen01.h"
#include "coda/CKGEN01_V004.h"
#include "mtk_pnl_utility.h"
#include "mtk_tv_lpll_tbl.h"
#include "mtk_pnl_clk_ctrl.h"
#include "mtk_tv_pnl.h"
#include "mtk_pnl_out_if.h"
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
#include "coda/CKGEN01_V005.h"
#include "coda/MODV11_V005.h"
#include "coda/MODV12_V005.h"
#include "coda/MODD1_V005.h"
#include "coda/MODOSD1_V005.h"
#include "coda/MODOSD2_V005.h"
#include "coda/MODA1_V005.h"
#include "coda/MODA4_V005.h"
#include "coda/LPLL_V005.h"
#include "coda/TCON_GPO_COM_15_00.h"
#include "coda/TCON_GPO_COM_31_16.h"
#include <debug_impl.h>

#define MOD_IN_IF_VIDEO (4)	//means 4p engine input
#define MOD_IN_IF_DeltaV (1)	//means 1p engine input
#define MOD_IN_IF_GRAPH (1)	//means 1p engine input
#define LVDS_MPLL_CLOCK_1296MHZ	(1296)
#define LVDS_MPLL_CLOCK_MHZ	(864)	// For crystal 24Mhz
#define MOD_TOP_CLK_720MHZ (720)
#define MOD_TOP_CLK_360MHZ (360)
#define MOD_TOP_CLK (594)
#define MOD_VER2 (2)

#define REG_0FFF (0x0FFF)
#define REG_ALL (0xFFFF)
#define VBO_USE_LANE_NUM_64 (64)
#define VBO_USE_LANE_NUM_32 (32)
#define VBO_USE_LANE_NUM_16 (16)
#define VBO_USE_LANE_NUM_12 (12)
#define VBO_USE_LANE_NUM_8 (8)
#define VBO_USE_LANE_NUM_4 (4)
#define VBO_USE_LANE_NUM_2 (2)
#define P_250 (250)
#define P_500 (500)
#define P_1000 (1000)
#define P_2000 (2000)
#define P_4000 (4000)
#define P_8000 (8000)
#define REFRESH_144 (144)
#define MHZ (1000000ULL)

struct mtk_lpll_table_info {
	E_PNL_SUPPORTED_LPLL_TYPE table_start;
	E_PNL_SUPPORTED_LPLL_TYPE table_end;
	uint16_t panel_num;
	uint16_t lpll_reg_num;
	uint16_t mpll_reg_num;
	uint16_t moda_reg_num;
};

struct mtk_lpll_type{
	E_PNL_SUPPORTED_LPLL_TYPE lpll_type;
	E_PNL_SUPPORTED_LPLL_TYPE lpll_tbl_idx;
};

// function: check when bring up start
static void _get_table_info(struct udevice *dev,struct mtk_lpll_table_info* tbl_info)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	switch (priv->pnl_lib_version) {
	case BOOT_PNL_VERSION0100:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_Video_5BYTE_OSD_5BYTE_300to300MHz;
		tbl_info->panel_num = PNL_NUM_VER1;
		tbl_info->lpll_reg_num = LPLL_REG_NUM;
		tbl_info->mpll_reg_num = 0;
		tbl_info->moda_reg_num = 0;
		break;
	case BOOT_PNL_VERSION0200:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz_VER2;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_Video_5BYTE_OSD_5BYTE_Xtal_mode_0to583_2MHz_VER2;
		tbl_info->panel_num = PNL_NUM_VER0200;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER0200;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER0200;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER0200;
		break;
	case BOOT_PNL_VERSION0300:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz_VER0300;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_CEDS_TEST_0to600MHz_VER0300;
		tbl_info->panel_num = PNL_NUM_VER0300;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER0300;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER0300;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER0300;
		break;
	case BOOT_PNL_VERSION0203:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz_VER0203;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_Video_5BYTE_OSD_5BYTE_Xtal_mode_0to583_2MHz_VER0203;
		tbl_info->panel_num = PNL_NUM_VER0203;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER0203;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER0203;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER0203;
		break;
	case BOOT_PNL_VERSION0400:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_TTL_450to600MHz_VER004;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER004;
		tbl_info->panel_num = PNL_NUM_VER004;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER004;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER004;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER004;
		break;
	case BOOT_PNL_VERSION0500:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz_VER005;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_300to300MHz_VER005;
		tbl_info->panel_num = PNL_NUM_VER005;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER005;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER005;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER005;
		break;
	case BOOT_PNL_VERSION0600:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_TTL_450to600MHz_VER004;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER004;
		tbl_info->panel_num = PNL_NUM_VER004;
		tbl_info->lpll_reg_num = LPLL_REG_NUM_VER004;
		tbl_info->mpll_reg_num = MPLL_REG_NUM_VER004;
		tbl_info->moda_reg_num = MODA_REG_NUM_VER004;
		break;
	default:
		tbl_info->table_start = E_PNL_SUPPORTED_LPLL_LVDS_1ch_450to600MHz;
		tbl_info->table_end = E_PNL_SUPPORTED_LPLL_Vx1_Video_5BYTE_OSD_5BYTE_300to300MHz;
		tbl_info->panel_num = PNL_NUM_VER1;
		tbl_info->lpll_reg_num = LPLL_REG_NUM;
		tbl_info->mpll_reg_num = 0;
		tbl_info->moda_reg_num = 0;
		break;
	}
}

#define MHZ (1000000ULL)

static uint64_t _get_top_clk(struct mtk_panel_priv *priv)
{
	uint32_t pnl_version = priv->pnl_lib_version;
	uint64_t u64Odclk = priv->typ_dclk;
	uint8_t u8Factor = 1;
	uint8_t u8Div = 1;

	if (pnl_version != BOOT_PNL_VERSION0400 && pnl_version != BOOT_PNL_VERSION0500 &&
		pnl_version != BOOT_PNL_VERSION0600) {
		printf("%s: panel version %d sould not in here\n",
			__func__, pnl_version);
		return 0;
	}

	if (priv->linktype == E_LINK_VB1) {
		if(priv->lanes == 16) {
			u8Factor = 1;   // DCLK 1200MHz
			u8Div = 2;
		} else if(priv->lanes == 8) {
			u8Factor = 1;   // DCLK 600MHz
			u8Div = 1;

			if(priv->out_timing == E_4K1K_144HZ)
				u8Div *= 2; //4K1K_144 is special case. odclk should /2
		} else if(priv->lanes == 4) {
			u8Factor = 2;   // DCLK 300MHz
			u8Div = 1;
		} else if(priv->lanes == 2) {
			u8Factor = 4;   // DCLK 150MHz
			u8Div = 1;
		} else {
			u64Odclk = MOD_TOP_CLK * MHZ;
			u8Factor = 1;   // DCLK 600MHz
			u8Div = 1;
			printf("[%s] invalid VB1 lane num\n", __func__);
		}
	} else if (priv->linktype == E_LINK_LVDS) {
		if (priv->lanes == 2) {
			u8Factor = 4;   // DCLK 150MHz
			u8Div = 1;
		} else {
			u64Odclk = MOD_TOP_CLK  * MHZ;
			u8Factor = 1;   // DCLK 600MHz
			u8Div = 1;
			printf("[%s] invalid LVDS lane num\n", __func__);
		}
	} else { //TCON-less case
		// 4K60  TCON-less case: DCLK=600MHz,  TOP_CLOCK=600MHz, u8Div = 1
		// 4K120 TCON-less case: DCLK=1200MHz, TOP_CLOCK=600MHz, u8Div = 2
		// FHD   TCON-less case: not support now
		if (priv->out_timing == E_4K2K_120HZ || priv->out_timing == E_4K1K_144HZ ||
		    priv->out_timing == E_4K2K_144HZ ||
		    priv->out_timing == E_4K1K_240HZ) {
			u8Factor = 1;   // DCLK 1200MHz
			u8Div = 2;
		} else if ((priv->out_timing == E_4K2K_60HZ) ||
				(priv->out_timing == E_4K1K_120HZ)) {
			u8Factor = 1;   // DCLK 600MHz
			u8Div = 1;
		} else {
			u64Odclk = MOD_TOP_CLK * MHZ;
			u8Factor = 1;   // DCLK 600MHz
			u8Div = 1;
			printf("[%s] invalid TCON case\n", __func__);
		}
	}

	u64Odclk = (u64Odclk * u8Factor + u8Div / 2) / u8Div;
	debug("[%s] dclk=%llu, factor=%u, div=%u, top_clock=%llu\n", __func__, priv->typ_dclk, u8Factor, u8Div, u64Odclk);

	return u64Odclk;
}

static E_PNL_SUPPORTED_LPLL_TYPE __get_ver5_link_idx(struct mtk_panel_priv *priv)
{
	uint64_t ldHz = priv->typ_dclk;
	E_PNL_SUPPORTED_LPLL_TYPE idx = E_PNL_SUPPORTED_LPLL_MAX;
	ldHz = _get_top_clk(priv);

	switch (priv->linktype) {
	case E_LINK_HSLVDS_1CH :
		if (ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_450to600MHz_VER005;
		break;

	case E_LINK_HSLVDS_2CH :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_450to600MHz_VER005;
		break;

	case E_LINK_MINILVDS_2BLK_3PAIR_8BIT :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_300to300MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_300to700MHz_VER005;
		break;

	case E_LINK_EPI28_8BIT_6PAIR_4KCML :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_CML_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 360*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_CML_300to360MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_CML_360to600MHz_VER005;
		break;

	case E_LINK_EPI28_8BIT_8PAIR_4KCML :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_CML_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_CML_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_CML_450to600MHz_VER005;
		break;

	case E_LINK_EPI28_8BIT_12PAIR_4KCML :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K144_CML_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K144_CML_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_CML_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 360*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_CML_300to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_CML_360to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K60_CML_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K60_CML_300to600MHz_VER005;
		}
		break;

	case E_LINK_EPI28_8BIT_16PAIR_4KCML :
		if((priv->out_timing == E_4K2K_144HZ) || (priv->out_timing == E_4K1K_144HZ)){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_CML_360to360MHz_VER005;
			else if((ldHz >= 360*MHZ) && (ldHz < 480*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_CML_360to480MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_CML_480to720MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_CML_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_CML_300to450MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_CML_450to600MHz_VER005;
		}
		break;

	case E_LINK_EPI24_10BIT_12PAIR_4KCML :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K120_CML_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K120_CML_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K60_CML_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K60_CML_300to600MHz_VER005;
		}
		break;

	case E_LINK_EPI24_10BIT_16PAIR_4KCML :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K1K144_CML_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K1K144_CML_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_CML_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_CML_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_CML_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K60_CML_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K60_CML_300to600MHz_VER005;
		}
		break;

	case E_LINK_EPI28_8BIT_6PAIR_4KLVDS :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_LVDS_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 360*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_LVDS_300to360MHz_VER005;
		else
			idx =  E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K60_LVDS_360to600MHz_VER005;
		break;

	case E_LINK_EPI28_8BIT_8PAIR_4KLVDS :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_LVDS_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_LVDS_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K60_LVDS_450to600MHz_VER005;
		break;

	case E_LINK_EPI28_8BIT_12PAIR_4KLVDS :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K144_LVDS_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K144_LVDS_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_LVDS_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 360*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_LVDS_300to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K120_LVDS_360to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K60_LVDS_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K60_LVDS_300to600MHz_VER005;
		}
		break;

	case E_LINK_EPI28_8BIT_16PAIR_4KLVDS :
		if((priv->out_timing == E_4K2K_144HZ) || (priv->out_timing == E_4K1K_144HZ)){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_LVDS_360to360MHz_VER005;
			else if((ldHz >= 360*MHZ) && (ldHz < 480*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_LVDS_360to480MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K144_LVDS_480to720MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_LVDS_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_LVDS_300to450MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_16x1_4K120_LVDS_450to600MHz_VER005;
		}
		break;

	case E_LINK_EPI24_10BIT_12PAIR_4KLVDS :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K120_LVDS_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K120_LVDS_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K60_LVDS_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K60_LVDS_300to600MHz_VER005;
		}
		break;

	case E_LINK_EPI24_10BIT_16PAIR_4KLVDS :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K1K144_LVDS_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K1K144_LVDS_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_LVDS_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_LVDS_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K120_LVDS_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K60_LVDS_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_16x1_4K60_LVDS_300to600MHz_VER005;
		}
		break;

	case E_LINK_CMPI27_8BIT_6PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K60_300to400MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_4K60_400to600MHz_VER005;
		break;

	case E_LINK_CMPI27_8BIT_8PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K60_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_4K60_450to600MHz_VER005;
		break;

	case E_LINK_CMPI27_8BIT_12PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K144_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K144_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_8bit_12x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_8bit_12x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K120_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K120_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K120_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_CMPI27_10BIT_12PAIR :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_USIT_8BIT_6PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_4K60_300to400MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_4K60_400to600MHz_VER005;
		break;

	case E_LINK_USIT_8BIT_8PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_8x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_8x1_4K60_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_8x1_4K60_450to600MHz_VER005;
		break;

	case E_LINK_USIT_8BIT_12PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K144_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K144_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K120_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K120_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K120_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_USIT_8BIT_16PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K144_360to360MHz_VER005;
			else if((ldHz >= 360*MHZ) && (ldHz < 540*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K144_360to540MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K144_540to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K1K144_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K1K144_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K1K144_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K60_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K60_300to450MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_16x1_4K60_450to600MHz_VER005;
		}
		break;

	case E_LINK_USIT_10BIT_12PAIR :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_USIT_10BIT_16PAIR :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K120_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K120_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K120_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_16x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_ISP_8BIT_6PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K60_300to400MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_4K60_400to600MHz_VER005;
		break;

	case E_LINK_ISP_8BIT_6X2PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K60_300to300MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_4K60_300to600MHz_VER005;
		break;

	case E_LINK_ISP_8BIT_8PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K60_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_4K60_450to600MHz_VER005;
		break;

	case E_LINK_ISP_8BIT_12PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K144_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K144_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K120_300to300MHz_VER005;
			else if((ldHz >= 300*MHZ) && (ldHz < 400*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K120_300to400MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K120_400to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_ISP_10BIT_6X2PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K60_300to300MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_4K60_300to600MHz_VER005;
		break;

	case E_LINK_ISP_10BIT_12PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K144_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K144_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_CHPI_8BIT_6PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K60_300to300MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_4K60_300to600MHz_VER005;
		break;

	case E_LINK_CHPI_8BIT_6X2PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K60_300to300MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_4K60_300to600MHz_VER005;
		break;

	case E_LINK_CHPI_8BIT_8PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K60_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_4K60_450to600MHz_VER005;
		break;

	case E_LINK_CHPI_8BIT_12PAIR :
		if(priv->out_timing == E_4K2K_144HZ){
			if(ldHz < 360*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K144_360to360MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K144_360to720MHz_VER005;
		}
		else if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to180MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K60_300to600MHz_VER005;
		}
		break;

	case E_LINK_CHPI_10BIT_8PAIR :
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K60_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 360*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K60_300to360MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_4K60_360to600MHz_VER005;
		break;

	case E_LINK_CHPI_10BIT_12PAIR :
		if(priv->out_timing == E_4K1K_144HZ){
			if(ldHz < 180*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to180MHz_VER005;
			else if((ldHz >= 180*MHZ) && (ldHz < 270*MHZ))
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_180to270MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_270to360MHz_VER005;
		}
		else if((priv->out_timing == E_4K2K_120HZ)
			|| (priv->out_timing == E_4K1K_240HZ)){
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K120_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K120_300to600MHz_VER005;
		}
		else{
			if(ldHz < 300*MHZ)
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K60_300to300MHz_VER005;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K60_300to600MHz_VER005;
		}
		break;


	case E_LINK_LVDS:
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_300to300MHz_VER005;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_300to450MHz_VER005;
		else
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER005;
		break;

	case E_LINK_VB1:
		switch(priv->out_timing){
		// Vx1 10bit/8bit format is depend on Vx1 byte_mode, not dither_depth
		case E_4K2K_120HZ:
		case E_4K1K_240HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_16_lane_4K120_300to300MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_16_lane_4K120_300to600MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K120_300to300MHz_VER005;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K120_300to450MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K120_450to600MHz_VER005;
			}
			else{
				printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;

		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K60_300to300MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K60_300to600MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K60_300to300MHz_VER005;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K60_300to450MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K60_450to600MHz_VER005;
			}
			else{
				debug("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;

		case E_4K2K_30HZ:
		case E_FHD_120HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K30_300to300MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_4K30_300to600MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K30_300to300MHz_VER005;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K30_300to450MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_4K30_450to600MHz_VER005;
			}
			else{
				printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;
		case E_FHD_60HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_300to300MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_2K60_300to600MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_300to300MHz_VER005;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_300to450MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_2K60_450to600MHz_VER005;
			}
			else{
				printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;

		case E_4K2K_144HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 360*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_16_lane_4K144_360to360MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_16_lane_4K144_360to720MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 360*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K144_360to360MHz_VER005;
				else if((ldHz >= 360*MHZ) && (ldHz < 480*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K144_360to480MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_16_lane_4K144_480to720MHz_VER005;
			}
			else{
				printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;
		case E_4K1K_144HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 180*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1k144_180to180MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1k144_180to360MHz_VER005;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 180*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1k144_180to180MHz_VER005;
				else if((ldHz >= 180*MHZ) && (ldHz < 240*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1k144_180to240MHz_VER005;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1k144_240to360MHz_VER005;
			}
			else{
				printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}
			break;
		default:
			printf("[%s][%d] Not Support timming\n", __func__, __LINE__);
			break;
		}
		break;
	default:
		printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		break;
	}

	return idx;
}

static E_PNL_SUPPORTED_LPLL_TYPE __get_ver4_link_idx(struct mtk_panel_priv *priv)
{
	uint64_t ldHz = priv->typ_dclk;
	E_PNL_SUPPORTED_LPLL_TYPE idx = E_PNL_SUPPORTED_LPLL_MAX;
	ldHz = _get_top_clk(priv);

	switch (priv->linktype) {
	case E_LINK_TTL:
		if (ldHz < 75 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_TTL_75to75MHz_VER004;
		else if (ldHz >= 75 * MHZ && ldHz < 225 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_TTL_75to225MHz_VER004;
		else if (ldHz >= 225 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_TTL_225to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_TTL_450to600MHz_VER004;
		break;
	case E_LINK_HSLVDS_1CH:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_1ch_450to600MHz_VER004;
		break;
	case E_LINK_HSLVDS_2CH:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_HS_LVDS_2ch_450to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_1BLK_3PAIR_6BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_6bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_1BLK_6PAIR_6BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_6bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_2BLK_3PAIR_6BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_6bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_2BLK_6PAIR_6BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_6bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_1BLK_3PAIR_8BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_3pair_8bit_300to600MHz_VER004;
		break;

	case E_LINK_MINILVDS_1BLK_6PAIR_8BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_1bk_6pair_8bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_2BLK_3PAIR_8BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_3pair_8bit_300to600MHz_VER004;
		break;
	case E_LINK_MINILVDS_2BLK_6PAIR_8BIT:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_MINILVDS_2bk_6pair_8bit_300to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_2PAIR_2KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_CML_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_CML_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_CML_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_4PAIR_2KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_CML_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_CML_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_CML_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_6PAIR_2KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K_CML_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K_CML_300to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_8PAIR_2KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_CML_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_CML_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_CML_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_6PAIR_4KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_CML_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 350 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_CML_300to350MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_CML_350to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_8PAIR_4KCML:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_CML_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_CML_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_CML_450to600MHz_VER004;
		break;

	case E_LINK_EPI28_8BIT_12PAIR_4KCML :
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K_CML_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_CML_180to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K_CML_300to600MHz_VER004;
		}
		break;

	case E_LINK_EPI24_10BIT_12PAIR_4KCML:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K_CML_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
			E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_CML_250to360MHz_VER004;
			else
				idx =
				E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K_CML_300to600MHz_VER004;
		}
		break;
	case E_LINK_EPI28_8BIT_2PAIR_2KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_LVDS_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_LVDS_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_2pair_2K_LVDS_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_4PAIR_2KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_LVDS_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_LVDS_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_4pair_2K_LVDS_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_6PAIR_2KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K_LVDS_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6pair_2K_LVDS_300to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_8PAIR_2KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_LVDS_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_LVDS_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8pair_2K_LVDS_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_6PAIR_4KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_LVDS_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 350 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_LVDS_300to350MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_6x1_4K_LVDS_350to600MHz_VER004;
		break;

	case E_LINK_EPI28_8BIT_8PAIR_4KLVDS:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_LVDS_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_LVDS_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_8x1_4K_LVDS_450to600MHz_VER004;
		break;
	case E_LINK_EPI28_8BIT_12PAIR_4KLVDS:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K_LVDS_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
			E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K1K144_LVDS_180to360MHz_VER004;
			else
				idx =
				E_PNL_SUPPORTED_LPLL_EPI_28_8bit_12x1_4K_LVDS_300to600MHz_VER004;
		}
		break;
	case E_LINK_EPI24_10BIT_12PAIR_4KLVDS:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K_LVDS_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
			E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K1K144_LVDS_250to360MHz_VER004;
			else
				idx =
				E_PNL_SUPPORTED_LPLL_EPI_24_10bit_12x1_4K_LVDS_300to600MHz_VER004;
		}
		break;
	case E_LINK_CMPI27_8BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 400 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_300to400MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_6x1_400to600MHz_VER004;
		break;
	case E_LINK_CMPI27_8BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_8x1_450to600MHz_VER004;
		break;
	case E_LINK_CMPI27_8BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_4K1K144_180to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_27_8bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_CMPI27_10BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 400 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_300to400MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_8x1_400to600MHz_VER004;
		break;
	case E_LINK_CMPI27_10BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_4K1K144_250to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_CMPI_24_10bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_USIT_8BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 400 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_300to400MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_6x1_400to600MHz_VER004;
		break;
	case E_LINK_USIT_8BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_4K1K144_180to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_8bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_USIT_10BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_6x1_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_6x1_300to600MHz_VER004;
		break;
	case E_LINK_USIT_10BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_4K1K144_300to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_USI_T_10bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_ISP_8BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 400 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_300to400MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x1_400to600MHz_VER004;
		break;
	case E_LINK_ISP_8BIT_6X2PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_6x2_300to600MHz_VER004;
		break;
	case E_LINK_ISP_8BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_8x1_450to600MHz_VER004;
		break;
	case E_LINK_ISP_8BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_4K1K144_180to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_8bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_ISP_10BIT_6X2PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_6x2_300to600MHz_VER004;
		break;
	case E_LINK_ISP_10BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 400 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_300to400MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_8x1_400to600MHz_VER004;
		break;
	case E_LINK_ISP_10BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				 E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_4K1K144_300to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_ISP_10bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_CHPI_8BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x1_300to600MHz_VER004;
		break;
	case E_LINK_CHPI_8BIT_6X2PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_6x2_300to600MHz_VER004;
		break;
	case E_LINK_CHPI_8BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 450 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_8x1_450to600MHz_VER004;
		break;
	case E_LINK_CHPI_8BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_4K1K144_180to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_8bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_CHPI_10BIT_8PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_300to300MHz_VER004;
		else if (ldHz >= 300 * MHZ && ldHz < 350 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_300to350MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_8x1_350to600MHz_VER004;
		break;
	case E_LINK_CHPI_10BIT_12PAIR:
		if (ldHz < 300 * MHZ) {
			idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_300to300MHz_VER004;
		} else {
			if (priv->out_timing == E_4K1K_144HZ)
				idx =
				E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_4K1K144_250to360MHz_VER004;
			else
				idx = E_PNL_SUPPORTED_LPLL_CHPI_10bit_12x1_300to600MHz_VER004;
		}
		break;
	case E_LINK_CHPI_10BIT_6PAIR:
		if (ldHz < 300 * MHZ)
			idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_300to300MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_300to600MHz_VER004;
		break;
	case E_LINK_LVDS:
		if(ldHz < 300*MHZ)
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_300to300MHz_VER004;
		else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_300to450MHz_VER004;
		else
			idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER004;
		break;
	case E_LINK_VB1:
		switch (priv->out_timing) {
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_300to300MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_300to600MHz_VER004;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_300to300MHz_VER004;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_300to450MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_450to600MHz_VER004;
			}
			else{
				debug("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}

			break;
		case E_4K2K_30HZ:
		case E_FHD_120HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_300to300MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_4_lane_300to600MHz_VER004;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_300to300MHz_VER004;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_300to450MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_4_lane_450to600MHz_VER004;
			}
			else{
				debug("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}

			break;
		case E_FHD_60HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_300to300MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_2_lane_300to600MHz_VER004;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 300*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_300to300MHz_VER004;
				else if((ldHz >= 300*MHZ) && (ldHz < 450*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_300to450MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_2_lane_450to600MHz_VER004;
			}
			else{
				debug("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}

			break;
		case E_4K1K_144HZ:
			if(priv->vbo_byte == E_VBO_4BYTE_MODE){
				if(ldHz < 180*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to180MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER004;
			}
			else if(priv->vbo_byte == E_VBO_3BYTE_MODE){
				if(ldHz < 180*MHZ)
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to180MHz_VER004;
				else if((ldHz >= 180*MHZ) && (ldHz < 200*MHZ))
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_180to200MHz_VER004;
				else
					idx = E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_200to360MHz_VER004;
			}
			else{
				debug("[%s][%d] Link type Not Support\n", __func__, __LINE__);
			}

			break;
		case E_8K4K_144HZ:
		case E_8K4K_120HZ:
		case E_8K4K_60HZ:
		case E_8K4K_30HZ:
		case E_4K2K_120HZ:
		case E_4K2K_144HZ:
		case E_4K1K_240HZ:
			printf("[%s][%d] Link type Todo\n", __func__, __LINE__);
			break;
		case E_OUTPUT_MODE_MAX:
		case E_OUTPUT_NONE:
			printf("[%s][%d] Link type error\n", __func__, __LINE__);
			break;
		default:
			printf("[%s][%d] Not Support out timing\n", __func__, __LINE__);
			break;
		}
		break;
	default:
		printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		break;
	}

	return idx;
}

static struct mtk_lpll_type _get_link_idx(struct udevice *dev)
{
	struct mtk_lpll_type lpll_name = { };
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_MAX;
	lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_MAX;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return lpll_name;
	}

	switch (priv->pnl_lib_version) {
	case BOOT_PNL_VERSION0100:
		if (priv->linktype == E_LINK_LVDS) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz;
		} else if (priv->linktype == E_LINK_VB1) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz;
		} else {
			printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		}
		break;
	case BOOT_PNL_VERSION0200:
	{
		struct mtk_lpll_table_info lpll_tbl_info = { };

		_get_table_info(dev,&lpll_tbl_info);
		if (priv->linktype == E_LINK_LVDS) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER2 - lpll_tbl_info.table_start;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER2;
		} else if (priv->linktype == E_LINK_VB1) {
			if(priv->out_timing == E_4K2K_144HZ) {
				lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_3BYTE_340to720MHz_VER2 - lpll_tbl_info.table_start;
				lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_3BYTE_340to720MHz_VER2;
			} else {
				lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER2 - lpll_tbl_info.table_start;
				lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER2;
			}
		} else {
			printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		}
		break;
	}
	case BOOT_PNL_VERSION0203:
	{
		struct mtk_lpll_table_info lpll_tbl_info;

		_get_table_info(dev, &lpll_tbl_info);
		if (priv->linktype == E_LINK_LVDS) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER0203 - lpll_tbl_info.table_start;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER0203;
		} else if (priv->linktype == E_LINK_VB1) {
			if (priv->out_timing == E_4K2K_144HZ) {
				lpll_name.lpll_tbl_idx =
				E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_340to720MHz_VER0203 - lpll_tbl_info.table_start;
				lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_340to720MHz_VER0203;
			} else {
				lpll_name.lpll_tbl_idx =
				E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER0203 - lpll_tbl_info.table_start;
				lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER0203;
			}
		} else {
			printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		}
		break;
	}
	case BOOT_PNL_VERSION0300:
	{
		struct mtk_lpll_table_info lpll_tbl_info = { };

		_get_table_info(dev, &lpll_tbl_info);
		if (priv->linktype == E_LINK_LVDS) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER0300 - lpll_tbl_info.table_start;
			lpll_name.lpll_type =	E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz_VER0300;
		} else if (priv->linktype == E_LINK_VB1) {
			if (priv->out_timing == E_4K2K_144HZ) {
				lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_4K144_180to360MHz_VER0300 - lpll_tbl_info.table_start;
				lpll_name.lpll_type =	E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_4K144_180to360MHz_VER0300;
			} else if (priv->out_timing == E_8K4K_144HZ) {
				lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_8K144_360to720MHz_VER0300 - lpll_tbl_info.table_start;
				lpll_name.lpll_type =	E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_8K144_360to720MHz_VER0300;
			} else {
				lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER0300 - lpll_tbl_info.table_start;
				lpll_name.lpll_type =	E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz_VER0300;
			}
		} else {
			printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		}
		break;
	}
	case BOOT_PNL_VERSION0400:
	{
		struct mtk_lpll_table_info lpll_tbl_info = { };
		E_PNL_SUPPORTED_LPLL_TYPE idx = __get_ver4_link_idx(priv);

		_get_table_info(dev, &lpll_tbl_info);
		lpll_name.lpll_tbl_idx = idx - lpll_tbl_info.table_start;
		lpll_name.lpll_type = idx;
		break;
	}
	case BOOT_PNL_VERSION0500:
	{
		struct mtk_lpll_table_info lpll_tbl_info = { };
		E_PNL_SUPPORTED_LPLL_TYPE idx = __get_ver5_link_idx(priv);

		_get_table_info(dev, &lpll_tbl_info);
		lpll_name.lpll_tbl_idx = idx - lpll_tbl_info.table_start;
		lpll_name.lpll_type = idx;
		break;
	}
	case BOOT_PNL_VERSION0600:
	{
		struct mtk_lpll_table_info lpll_tbl_info = { };
		E_PNL_SUPPORTED_LPLL_TYPE idx = __get_ver4_link_idx(priv);

		_get_table_info(dev, &lpll_tbl_info);
		lpll_name.lpll_tbl_idx = idx - lpll_tbl_info.table_start;
		lpll_name.lpll_type = idx;
		break;
	}

	default:
		if (priv->linktype == E_LINK_LVDS) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_LVDS_2ch_450to600MHz;
		} else if (priv->linktype == E_LINK_VB1) {
			lpll_name.lpll_tbl_idx = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz;
			lpll_name.lpll_type = E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_5BYTE_300to600MHz;
		} else {
			printf("[%s][%d] Link type Not Support\n", __func__, __LINE__);
		}
		break;
	}

	return lpll_name;
}

int mtk_pll_dump_clk_tbl(struct udevice *dev)
{
	uint16_t idx = 0;
	uint16_t idxmax = 0;
	E_PNL_SUPPORTED_LPLL_TYPE tblidx = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	struct mtk_lpll_table_info lpll_tbl_info = { };
	struct mtk_lpll_type lpll_name = { };

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	lpll_name = _get_link_idx(dev);
	tblidx = lpll_name.lpll_tbl_idx;

	_get_table_info(dev, &lpll_tbl_info);
	idxmax = lpll_tbl_info.lpll_reg_num;

	if (tblidx >= lpll_tbl_info.panel_num) {
		printf("%s: invaild tblidx\n", __func__);
		return -EINVAL;
	}

	UBOOT_DEBUG("pnl_ver = 0x%x\n",  (int)priv->pnl_lib_version);
	UBOOT_DEBUG("dump tbl index = %d\n", tblidx);
	UBOOT_DEBUG("lpll reg num = %d\n", idxmax);

	for (idx = 0; idx < idxmax; idx++) {
		switch (priv->pnl_lib_version) {
		case BOOT_PNL_VERSION0100:
			if ((tblidx >= PNL_NUM_VER1) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER1\n", __func__);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM) {
				printf("[%s] link index is over LPLL_REG_NUM\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL[tblidx][idx].address,
						LPLLSettingTBL[tblidx][idx].value,
						LPLLSettingTBL[tblidx][idx].mask);
			break;

		case BOOT_PNL_VERSION0200:
			if ((tblidx >= PNL_NUM_VER0200) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0200\n", __func__);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER0200) {
				printf("[%s] link index is over LPLL_REG_NUM_VER0200\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER0200[tblidx][idx].address,
					LPLLSettingTBL_VER0200[tblidx][idx].value,
					LPLLSettingTBL_VER0200[tblidx][idx].mask);
			break;

		case BOOT_PNL_VERSION0300:
			if ((tblidx >= PNL_NUM_VER0300) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0300\n", __func__);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER0300) {
				printf("[%s] link index is over LPLL_REG_NUM_VER0300\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER0300[tblidx][idx].address,
						LPLLSettingTBL_VER0300[tblidx][idx].value,
						LPLLSettingTBL_VER0300[tblidx][idx].mask);
			break;

		case BOOT_PNL_VERSION0203:
			if ((tblidx >= PNL_NUM_VER0203) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0203\n", __func__);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER0203) {
				printf("[%s] link index is over LPLL_REG_NUM_VER0203\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER0203[tblidx][idx].address,
						LPLLSettingTBL_VER0203[tblidx][idx].value,
						LPLLSettingTBL_VER0203[tblidx][idx].mask);
			break;

		case BOOT_PNL_VERSION0400:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index (%d )is over PNL_NUM_VER4(%d)\n", __func__,
					tblidx, PNL_NUM_VER004);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER004) {
				printf("[%s] link index is over LPLL_REG_NUM\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER004[tblidx][idx].address,
					LPLLSettingTBL_VER004[tblidx][idx].value,
					LPLLSettingTBL_VER004[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0500:
			if ((tblidx >= PNL_NUM_VER005) || (tblidx < 0)) {
				printf("[%s] link index (%d )is over PNL_NUM_VER4(%d)\n", __func__,
					tblidx, PNL_NUM_VER005);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER005) {
				printf("[%s] link index is over LPLL_REG_NUM\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER005[tblidx][idx].address,
					LPLLSettingTBL_VER005[tblidx][idx].value,
					LPLLSettingTBL_VER005[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0600:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index (%d )is over PNL_NUM_VER4(%d)\n", __func__,
						tblidx, PNL_NUM_VER004);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM_VER004) {
				printf("[%s] link index is over LPLL_REG_NUM\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL_VER004[tblidx][idx].address,
					LPLLSettingTBL_VER004[tblidx][idx].value,
					LPLLSettingTBL_VER004[tblidx][idx].mask);
			break;
		default:
			if ((tblidx >= PNL_NUM_VER1) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER1\n", __func__);
				return -ENXIO;
			}
			if (idx >= LPLL_REG_NUM) {
				printf("[%s] link index is over LPLL_REG_NUM\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(LPLLSettingTBL[tblidx][idx].address,
					LPLLSettingTBL[tblidx][idx].value,
					LPLLSettingTBL[tblidx][idx].mask);
			break;
		}
	}
	efuse_set_lpllsetting(priv->pnl_lib_version);

	return 0;
}

int mtk_mpll_dump_clk_tbl(struct udevice *dev)
{
	uint16_t idx = 0;
	uint16_t idxmax = 0;
	E_PNL_SUPPORTED_LPLL_TYPE tblidx = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	struct mtk_lpll_table_info lpll_tbl_info;
	struct mtk_lpll_type lpll_name;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	lpll_name = _get_link_idx(dev);
	tblidx = lpll_name.lpll_tbl_idx;

	_get_table_info(dev,&lpll_tbl_info);
	idxmax = lpll_tbl_info.mpll_reg_num;

	if (tblidx >= lpll_tbl_info.panel_num) {
		printf("%s: invalid tblidx\n", __func__);
		return -EINVAL;
	}

	debug("[%s] pnl_ver = 0x%x\n", __func__, (int)priv->pnl_lib_version);
	debug("[%s] dump tbl index = %d\n",__func__, tblidx);
	debug("[%s] mpll reg num = %d\n",__func__, idxmax);

	for (idx = 0; idx < idxmax; idx++) {
		switch (priv->pnl_lib_version) {
		case BOOT_PNL_VERSION0100:
			debug("[%s] no mpll table\n", __func__);
			break;
		case BOOT_PNL_VERSION0200:
			if ((tblidx >= PNL_NUM_VER0200) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0200\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER0200[tblidx][idx].address,
					MPLLSettingTBL_VER0200[tblidx][idx].value,
					MPLLSettingTBL_VER0200[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0203:
			if ((tblidx >= PNL_NUM_VER0203) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0203\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER0203[tblidx][idx].address,
					MPLLSettingTBL_VER0203[tblidx][idx].value,
					MPLLSettingTBL_VER0203[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0300:
			if ((tblidx >= PNL_NUM_VER0300) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0300\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER0300[tblidx][idx].address,
						MPLLSettingTBL_VER0300[tblidx][idx].value,
						MPLLSettingTBL_VER0300[tblidx][idx].mask);

			break;
		case BOOT_PNL_VERSION0400:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER4\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER004[tblidx][idx].address,
					MPLLSettingTBL_VER004[tblidx][idx].value,
					MPLLSettingTBL_VER004[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0500:
			if ((tblidx >= PNL_NUM_VER005) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER5\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER005[tblidx][idx].address,
					MPLLSettingTBL_VER005[tblidx][idx].value,
					MPLLSettingTBL_VER005[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0600:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER4\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MPLLSettingTBL_VER004[tblidx][idx].address,
					MPLLSettingTBL_VER004[tblidx][idx].value,
					MPLLSettingTBL_VER004[tblidx][idx].mask);
			break;
		default:
			printf("%s(%d): ERROR! no panle lib version\n", __func__, __LINE__);
			break;
		}
	}

	return 0;
}

void mtk_set_moda_pn_swap(struct mtk_panel_priv *priv)
{
	u16 u16cal_tmp = 0;

	// pn swap
	if ((priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
		(priv->pnl_lib_version == BOOT_PNL_VERSION0500)) {
		if (!priv->lane_info.pn_swap) {
			if (priv->linktype == E_LINK_LVDS ||
				priv->linktype == E_LINK_HSLVDS_2CH) {
				/*Keep and set in LVDS pn swap*/
				W2BYTEMSK(REG_0008_MODA1, 0x0000,
				REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
			} else {
				if (priv->linktype == E_LINK_VB1 &&
					priv->tcon_info.bIsProjector) {
					//Projector customized pn swap setting
					W2BYTEMSK(REG_0008_MODA1, 0x0000,
					REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
				} else {
					if (priv->lanes <= VBO_USE_LANE_NUM_12)
						W2BYTEMSK(REG_0008_MODA1, REG_0FFF,
						REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
					else if (priv->lanes <= VBO_USE_LANE_NUM_16)
						W2BYTEMSK(REG_0008_MODA1, REG_ALL,
						REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
				}
			}
		} else {
			// customized PN_SWAP[31:0] for CH0~CH31
			if (priv->linktype == E_LINK_LVDS ||
				priv->linktype == E_LINK_HSLVDS_2CH) {
				/*Keep and set in LVDS pn swap*/
				W2BYTEMSK(REG_0008_MODA1, 0x0000,
				REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
			} else {
				if (priv->linktype == E_LINK_VB1 &&
					priv->tcon_info.bIsProjector) {
					//Projector VBY1 customized pn swap setting
					W2BYTEMSK(REG_0008_MODA1, 0x0000,
					REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
				} else {
					if (priv->lanes <= VBO_USE_LANE_NUM_12) {
						u16cal_tmp = (REG_0FFF ^ (REG_0FFF & priv->lane_info.pn_swap));
						W2BYTEMSK(REG_0008_MODA1, u16cal_tmp,
							REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
					} else if (priv->lanes <= VBO_USE_LANE_NUM_16) {
						u16cal_tmp = (REG_ALL ^ (REG_ALL & priv->lane_info.pn_swap));
						W2BYTEMSK(REG_0008_MODA1, u16cal_tmp,
							REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
					}
				}
			}
		}
	} else {
	    W2BYTEMSK(REG_0008_MODA1, REG_ALL,
			REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
		W2BYTEMSK(REG_000C_MODA1, REG_ALL,
			REG_000C_MODA1_REG_GCR_DS_POL_CH_19_00_1);
		W2BYTEMSK(REG_0008_MODA2, REG_ALL,
			REG_0008_MODA2_REG_GCR_DS_POL_CH_39_20_0);
		W2BYTEMSK(REG_000C_MODA2, REG_ALL,
			REG_000C_MODA2_REG_GCR_DS_POL_CH_39_20_1);
	}
}

int mtk_moda_dump_clk_tbl(struct udevice *dev)
{
	uint16_t idx = 0;
	uint16_t idxmax = 0;
	E_PNL_SUPPORTED_LPLL_TYPE tblidx = 0;
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	struct mtk_lpll_table_info lpll_tbl_info;
	struct mtk_lpll_type lpll_name;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	lpll_name = _get_link_idx(dev);
	tblidx = lpll_name.lpll_tbl_idx;

	_get_table_info(dev, &lpll_tbl_info);
	idxmax = lpll_tbl_info.moda_reg_num;

	debug("[%s] pnl_ver = 0x%x\n", __func__, (int)priv->pnl_lib_version);
	debug("[%s] dump tbl index = %d\n", __func__, tblidx);
	debug("[%s] moda reg num = %d\n", __func__, idxmax);

	// moda reg. control
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
		if (priv->linktype == E_LINK_VB1) {
			if ((priv->vbo_byte == E_VBO_5BYTE_MODE) &&
				(priv->ext_grpah_combo_info.graph_vbo_byte_mode == E_VBO_5BYTE_MODE))
				W2BYTEMSK(REG_0048_MODA1, 0x0, REG_0048_MODA1_REG_GCR_CKEN);
			else
				W2BYTEMSK(REG_0048_MODA1, 0x1, REG_0048_MODA1_REG_GCR_CKEN);

			if ((priv->vbo_byte == E_VBO_5BYTE_MODE) ||
				(priv->ext_grpah_combo_info.graph_vbo_byte_mode == E_VBO_5BYTE_MODE))
				W2BYTEMSK(REG_0048_MODA1, 0x1, REG_0048_MODA1_REG_GCR_CKEN_PATH2);
			else
				W2BYTEMSK(REG_0048_MODA1, 0x0, REG_0048_MODA1_REG_GCR_CKEN_PATH2);
		} else {
			W2BYTEMSK(REG_0048_MODA1, 0x1, REG_0048_MODA1_REG_GCR_CKEN);
			W2BYTEMSK(REG_0048_MODA1, 0x0, REG_0048_MODA1_REG_GCR_CKEN_PATH2);
		}
		// pn swap
		W2BYTEMSK(REG_0008_MODA1, REG_ALL, REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
		W2BYTEMSK(REG_000C_MODA1, REG_ALL, REG_000C_MODA1_REG_GCR_DS_POL_CH_19_00_1);
		W2BYTEMSK(REG_0008_MODA2, REG_ALL, REG_0008_MODA2_REG_GCR_DS_POL_CH_39_20_0);
		W2BYTEMSK(REG_000C_MODA2, REG_ALL, REG_000C_MODA2_REG_GCR_DS_POL_CH_39_20_1);
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0600)) {
			if (priv->lane_info.pn_swap == 0)
			{
				if(priv->lanes <= 12)
					W2BYTEMSK(REG_0008_MODA1, 0x0FFF, REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
				else if(priv->lanes <= 16)
					W2BYTEMSK(REG_0008_MODA1, 0xFFFF, REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
			}
			else	// customized PN_SWAP[31:0] for CH0~CH31
			{
				if(priv->lanes <= 12)
					W2BYTEMSK(REG_0008_MODA1, (0x0FFF ^ (0x0FFF & priv->lane_info.pn_swap)), REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
				else if(priv->lanes <= 16)
					W2BYTEMSK(REG_0008_MODA1, (0xFFFF ^ (0xFFFF & priv->lane_info.pn_swap)), REG_0008_MODA1_REG_GCR_DS_POL_CH_19_00_0);
			}
		}
		mtk_set_moda_pn_swap(priv);
	}

	//dump moda tbl
	if (tblidx >= lpll_tbl_info.panel_num) {
		printf("%s: invaild tblidx\n", __func__);
		return -EINVAL;
	}

	for (idx = 0; idx < idxmax; idx++) {
		switch (priv->pnl_lib_version) {
		case BOOT_PNL_VERSION0100:
			debug("[%s] no moda table\n", __func__);
			break;
		case BOOT_PNL_VERSION0200:
			if ((tblidx >= PNL_NUM_VER0200) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0200\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER0200[tblidx][idx].address,
					MODASettingTBL_VER0200[tblidx][idx].value,
					MODASettingTBL_VER0200[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0203:
			if ((tblidx >= PNL_NUM_VER0203) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0203\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER0203[tblidx][idx].address,
					MODASettingTBL_VER0203[tblidx][idx].value,
					MODASettingTBL_VER0203[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0300:
			if ((tblidx >= PNL_NUM_VER0300) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER0300\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER0300[tblidx][idx].address,
					MODASettingTBL_VER0300[tblidx][idx].value,
					MODASettingTBL_VER0300[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0400:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER2\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER004[tblidx][idx].address,
					MODASettingTBL_VER004[tblidx][idx].value,
					MODASettingTBL_VER004[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0500:
			if ((tblidx >= PNL_NUM_VER005) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER2\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER005[tblidx][idx].address,
					MODASettingTBL_VER005[tblidx][idx].value,
					MODASettingTBL_VER005[tblidx][idx].mask);
			break;
		case BOOT_PNL_VERSION0600:
			if ((tblidx >= PNL_NUM_VER004) || (tblidx < 0)) {
				printf("[%s] link index is over PNL_NUM_VER2\n", __func__);
				return -ENXIO;
			}
			W2BYTEMSK(MODASettingTBL_VER004[tblidx][idx].address,
					MODASettingTBL_VER004[tblidx][idx].value,
					MODASettingTBL_VER004[tblidx][idx].mask);
			break;
		default:
			printf("[%s] no moda table\n", __func__);
			break;
		}
	}

	return 0;
}

// TODO:
int mtk_out_clk_init(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	struct mtk_lpll_type lpll_name;
	uint64_t lpllset = 0;
	uint16_t tblidx = 0;
	uint8_t loop_gain = 0;
	uint8_t loop_div = 0;
	uint32_t odclk = 0;
	uint64_t odclk_v4 = 0;
	uint32_t p = P_1000;
	uint64_t refresh = 0;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	lpll_name = _get_link_idx(dev);
	tblidx = lpll_name.lpll_tbl_idx;

	if (tblidx >= E_PNL_SUPPORTED_LPLL_MAX) {
		printf("[%s] get link index fail\n", __func__);
		return -ENXIO;
	}

	UBOOT_DEBUG("init output clk\n");

	switch (priv->pnl_lib_version) {
	case BOOT_PNL_VERSION0100:
		if (tblidx >= PNL_NUM_VER1) {
			printf("[%s] link index is over PNL_NUM_VER1\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain[tblidx];
		loop_div = lpll_LoopDiv[tblidx];
		break;
	case BOOT_PNL_VERSION0200:
		if (tblidx >= PNL_NUM_VER0200) {
			printf("[%s] link index is over PNL_NUM_VER0200\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER0200[tblidx];
		loop_div = lpll_LoopDiv_VER0200[tblidx];
		break;
	case BOOT_PNL_VERSION0203:
		if (tblidx >= PNL_NUM_VER0203) {
			printf("[%s] link index is over PNL_NUM_VER0203\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER0203[tblidx];
		loop_div = lpll_LoopDiv_VER0203[tblidx];
		break;
	case BOOT_PNL_VERSION0300:
		if (tblidx >= PNL_NUM_VER0300) {
			printf("[%s] link index is over PNL_NUM_VER0300\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER0300[tblidx];
		loop_div = lpll_LoopDiv_VER0300[tblidx];
		break;
	case BOOT_PNL_VERSION0400:
		if (tblidx >= PNL_NUM_VER004) {
			printf("[%s] link index is over PNL_NUM_VER400\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER004[tblidx];
		loop_div = lpll_LoopDiv_VER004[tblidx];
		break;
	case BOOT_PNL_VERSION0500:
		if (tblidx >= PNL_NUM_VER005) {
			printf("[%s] link index is over PNL_NUM_VER500\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER005[tblidx];
		loop_div = lpll_LoopDiv_VER005[tblidx];
		break;
	case BOOT_PNL_VERSION0600:
		if (tblidx >= PNL_NUM_VER004) {
			printf("[%s] link index is over PNL_NUM_VER400\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain_VER004[tblidx];
		loop_div = lpll_LoopDiv_VER004[tblidx];
		break;
	default:
		if (tblidx >= PNL_NUM_VER1) {
			printf("[%s] link index is over PNL_NUM_VER1\n", __func__);
			return -ENXIO;
		}
		loop_gain = lpll_LoopGain[tblidx];
		loop_div = lpll_LoopDiv[tblidx];
		break;
	}

	if (!(priv->typ_htt && priv->typ_vtt)) {
		printf("[%s] get panel private error\n", __func__);
		return -ENXIO;
	}

	refresh = (priv->typ_dclk / priv->typ_htt / priv->typ_vtt);

	if (priv->lanes == VBO_USE_LANE_NUM_2)
		p = P_250; //2k60
	else if (priv->lanes == VBO_USE_LANE_NUM_4)
		p = P_500; //2k120
	else if (priv->lanes == VBO_USE_LANE_NUM_8)
		p = P_1000; //4k60
	else if (priv->lanes == VBO_USE_LANE_NUM_16) {
		if(priv->out_format == E_OUTPUT_YUV422)
			p = P_4000; //8k60_YUV422
		else
			p = P_2000; //4k120 & 4k144
	} else if (priv->lanes == VBO_USE_LANE_NUM_32)
		p = P_4000; //8k60
	else if (priv->lanes == VBO_USE_LANE_NUM_64)
		p = P_8000; //8k120 & 8k144

	switch (priv->pnl_lib_version) {
	case BOOT_PNL_VERSION0300:
		if (priv->lanes == VBO_USE_LANE_NUM_16) {
			if ((refresh == REFRESH_144) || (priv->out_format == E_OUTPUT_YUV422))
				p = P_4000; //4k144 & 8k60_YUV422
			else
				p = P_2000; //4k120
		}
		break;
	case BOOT_PNL_VERSION0400:
	case BOOT_PNL_VERSION0500:	// FIXME
	case BOOT_PNL_VERSION0600:
		if (priv->lanes == VBO_USE_LANE_NUM_8) {
			if (refresh == REFRESH_144)
				p = P_2000; //4k1k144
			else
				p = P_1000; //4k60
		}
		break;
	}

	//determine odclk
	odclk = priv->typ_dclk / p; //(KHz)
	odclk_v4 = priv->typ_dclk; //(MHZ)
/*
	// determine odclk
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0203) {
		if ((lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_3BYTE_OSD_3BYTE_340to720MHz_VER2) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_3BYTE_OSD_4BYTE_340to720MHz_VER2) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_3BYTE_340to720MHz_VER2) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_340to720MHz_VER2) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_8K144_360to720MHz_VER0300) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_3BYTE_OSD_3BYTE_340to720MHz_VER0203) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_3BYTE_OSD_4BYTE_340to720MHz_VER0203) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_3BYTE_340to720MHz_VER0203) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_Video_4BYTE_OSD_4BYTE_340to720MHz_VER0203))
			odclk = MOD_TOP_CLK_720MHZ;
	}

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0400) {
		if ((lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_10bit_8_lane_4K1K144_180to360MHz_VER004) ||
			(lpll_enum == E_PNL_SUPPORTED_LPLL_Vx1_8bit_8_lane_4K1K144_200to360MHz_VER004))
			odclk = MOD_TOP_CLK_360MHZ;
	}
*/

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0600)
		lpllset = ((uint64_t)MPLL_CLK_1296MHZ * 524288 * LPLL_CAL_PARAM * loop_gain);
	else
		lpllset = ((uint64_t)MPLL_CLK_864MHZ * 524288 * LPLL_CAL_PARAM * loop_gain);

	switch (priv->pnl_lib_version) {
	case BOOT_PNL_VERSION0400:
	case BOOT_PNL_VERSION0500:
	case BOOT_PNL_VERSION0600:
		odclk_v4 = _get_top_clk(priv);
		do_div(lpllset, loop_div);
		lpllset  *= MHZ;    //Unit: MHz
		if(odclk_v4 <= 0xFFFFFFFF)
			do_div(lpllset, (uint32_t)odclk_v4);  // TOP_CLOCK use accurate value
		else
			lpllset = (lpllset + (odclk_v4/2)) / odclk_v4;  // TOP_CLOCK use accurate value

		break;
	default:
		lpllset *= P_1000;
		do_div(lpllset, loop_div);
		do_div(lpllset, odclk);
		break;
	}
	UBOOT_DEBUG("odclk_p = %d\n", p);
	UBOOT_DEBUG("dclk = %lluHz\n", priv->typ_dclk);
	UBOOT_DEBUG("htt = %d\n", priv->typ_htt);
	UBOOT_DEBUG("vtt = %d\n", priv->typ_vtt);
	UBOOT_DEBUG("refresh = %llu\n", refresh);
	UBOOT_DEBUG("lpllset = %llu\n", lpllset);
	UBOOT_DEBUG("odclk = %dKHz\n", odclk);

	// LPLL SET setting
	W2BYTEMSK(REG_003C_LPLL, (lpllset & 0x0000FFFF), REG_003C_LPLL_REG_PLL_SET_0);
	W2BYTEMSK(REG_0040_LPLL, (lpllset & 0xFFFF0000) >> 16, REG_0040_LPLL_REG_PLL_SET_1);

	//RG_PD_LPLL_SCALAR bit1
	//RG_PD_LPLL_FIFO bit2
	W2BYTEMSK(REG_00BC_LPLL, 0, Fld(2,1,AC_MSKB0));//bit2&bit1

	return 0;
}

// function: check when bring up  end
static void _ckg_sw_en_video(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	//general part
	//clock sw_en
	W2BYTEMSK(REG_1814_CKGEN01, 0x1, REG_1814_CKGEN01_REG_SW_EN_MOD_D_ODCLK2MOD_D);
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0400 || priv->pnl_lib_version == BOOT_PNL_VERSION0600)
		W2BYTEMSK(REG_1814_CKGEN01, 0x1, REG_1814_CKGEN01_V004_REG_SW_EN_MOD_D_ODCLK2TTL_1814);

	switch (priv->out_timing) {
	case E_FHD_60HZ:
	{
		if (priv->linktype == E_LINK_LVDS) {
			W2BYTEMSK(REG_1814_CKGEN01, 0x1, REG_1814_CKGEN01_REG_SW_EN_MOD_D_ODCLK2LVDS);
			W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
			W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
			W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
			W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
			W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
			W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
			W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
			W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
			W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
			W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
			W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
			W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
		} else if (priv->linktype == E_LINK_VB1) {
			W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
			W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
			W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
			W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
			W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
			W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
			W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
			W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
			W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
			W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
			W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
			W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
			W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
			W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
		} else {

		}
	}
	break;

	case E_FHD_120HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0600)
			W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		else
			W2BYTEMSK(REG_1868_CKGEN01, 0x0, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);

		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
	}
	break;

	case E_4K2K_60HZ:
	case E_4K1K_120HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
	}
	break;

	case E_4K2K_120HZ:
	case E_4K1K_240HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
	}
	break;

	case E_4K2K_144HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
	}
	break;

	case E_8K4K_60HZ:
	{
		if (priv->out_format == E_OUTPUT_YUV422) {
			W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
			W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
			W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
			W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
			W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
			W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
			W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
			W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
			W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
			W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
			W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
			W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
			W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
			W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
		} else {
			W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
			W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
			W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
			W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
			W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
			W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
			W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
			W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
			W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
			W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
			//sram share
			W2BYTEMSK(REG_187C_CKGEN01, 0x1, REG_187C_CKGEN01_REG_SW_EN_V2_ODCLK_STG32V2);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
			W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
			W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
			W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
			W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
			W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
			W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2V2);
			W2BYTEMSK(REG_1870_CKGEN01, 0x1, REG_1870_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK2V2);
			W2BYTEMSK(REG_18B8_CKGEN01, 0x1, REG_18B8_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE02V2);
			W2BYTEMSK(REG_18BC_CKGEN01, 0x1, REG_18BC_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE12V2);
			W2BYTEMSK(REG_18B4_CKGEN01, 0x1, REG_18B4_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_FINIAL2V2);
		}
	}
	break;

	case E_8K4K_120HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		//sram share
		W2BYTEMSK(REG_187C_CKGEN01, 0x1, REG_187C_CKGEN01_REG_SW_EN_V2_ODCLK_STG32V2);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
		W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2O);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2V2);
		W2BYTEMSK(REG_1870_CKGEN01, 0x1, REG_1870_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK2V2);
		W2BYTEMSK(REG_18B8_CKGEN01, 0x1, REG_18B8_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE02V2);
		W2BYTEMSK(REG_18BC_CKGEN01, 0x1, REG_18BC_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE12V2);
		W2BYTEMSK(REG_18B4_CKGEN01, 0x1, REG_18B4_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_FINIAL2V2);
		W2BYTEMSK(REG_1824_CKGEN01, 0x1, REG_1824_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK2O);
		W2BYTEMSK(REG_18A0_CKGEN01, 0x1, REG_18A0_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE02O);
		W2BYTEMSK(REG_18A4_CKGEN01, 0x1, REG_18A4_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE12O);
		W2BYTEMSK(REG_189C_CKGEN01, 0x1, REG_189C_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_FINIAL2O);
	}
	break;

	case E_8K4K_144HZ:
	{
		W2BYTEMSK(REG_186C_CKGEN01, 0x1, REG_186C_CKGEN01_REG_SW_EN_V1_ODCLK2V1);
		W2BYTEMSK(REG_184C_CKGEN01, 0x1, REG_184C_CKGEN01_REG_SW_EN_V1_ODCLK_STG12V1);
		W2BYTEMSK(REG_1850_CKGEN01, 0x1, REG_1850_CKGEN01_REG_SW_EN_V1_ODCLK_STG22V1);
		W2BYTEMSK(REG_1854_CKGEN01, 0x1, REG_1854_CKGEN01_REG_SW_EN_V1_ODCLK_STG32V1);
		W2BYTEMSK(REG_1858_CKGEN01, 0x1, REG_1858_CKGEN01_REG_SW_EN_V1_ODCLK_STG42V1);
		W2BYTEMSK(REG_185C_CKGEN01, 0x1, REG_185C_CKGEN01_REG_SW_EN_V1_ODCLK_STG52V1);
		W2BYTEMSK(REG_1860_CKGEN01, 0x1, REG_1860_CKGEN01_REG_SW_EN_V1_ODCLK_STG62V1);
		W2BYTEMSK(REG_1864_CKGEN01, 0x1, REG_1864_CKGEN01_REG_SW_EN_V1_ODCLK_STG72V1);
		W2BYTEMSK(REG_1868_CKGEN01, 0x1, REG_1868_CKGEN01_REG_SW_EN_V1_ODCLK_STG82V1);
		W2BYTEMSK(REG_18C0_CKGEN01, 0x1, REG_18C0_CKGEN01_REG_SW_EN_V1_ODCLK_STG92V1);
		//sram share
		W2BYTEMSK(REG_187C_CKGEN01, 0x1, REG_187C_CKGEN01_REG_SW_EN_V2_ODCLK_STG32V2);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2VBY1_V1);
		W2BYTEMSK(REG_181C_CKGEN01, 0x1, REG_181C_CKGEN01_REG_SW_EN_MOD_V1_SR_WCLK2V1);
		W2BYTEMSK(REG_1848_CKGEN01, 0x1, REG_1848_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK2V1);
		W2BYTEMSK(REG_18AC_CKGEN01, 0x1, REG_18AC_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE02V1);
		W2BYTEMSK(REG_18B0_CKGEN01, 0x1, REG_18B0_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_PRE12V1);
		W2BYTEMSK(REG_18A8_CKGEN01, 0x1, REG_18A8_CKGEN01_REG_SW_EN_V1_MOD_SR_RCLK_FINIAL2V1);
		W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2O);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2V2);
		W2BYTEMSK(REG_1870_CKGEN01, 0x1, REG_1870_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK2V2);
		W2BYTEMSK(REG_18B8_CKGEN01, 0x1, REG_18B8_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE02V2);
		W2BYTEMSK(REG_18BC_CKGEN01, 0x1, REG_18BC_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE12V2);
		W2BYTEMSK(REG_18B4_CKGEN01, 0x1, REG_18B4_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_FINIAL2V2);
		W2BYTEMSK(REG_1824_CKGEN01, 0x1, REG_1824_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK2O);
		W2BYTEMSK(REG_18A0_CKGEN01, 0x1, REG_18A0_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE02O);
		W2BYTEMSK(REG_18A4_CKGEN01, 0x1, REG_18A4_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE12O);
		W2BYTEMSK(REG_189C_CKGEN01, 0x1, REG_189C_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_FINIAL2O);
	}
	break;

	default:
	break;

	}
}

static void _ckg_sw_en_ext_video(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->out_timing == E_4K2K_60HZ ||
	    priv->out_timing == E_4K1K_120HZ) {
		W2BYTEMSK(REG_1890_CKGEN01, 0x1, REG_1890_CKGEN01_REG_SW_EN_V2_ODCLK2V2);
		W2BYTEMSK(REG_1874_CKGEN01, 0x1, REG_1874_CKGEN01_REG_SW_EN_V2_ODCLK_STG12V2);
		W2BYTEMSK(REG_1878_CKGEN01, 0x1, REG_1878_CKGEN01_REG_SW_EN_V2_ODCLK_STG22V2);
		W2BYTEMSK(REG_187C_CKGEN01, 0x1, REG_187C_CKGEN01_REG_SW_EN_V2_ODCLK_STG32V2);
		W2BYTEMSK(REG_1880_CKGEN01, 0x1, REG_1880_CKGEN01_REG_SW_EN_V2_ODCLK_STG42V2);
		W2BYTEMSK(REG_1884_CKGEN01, 0x1, REG_1884_CKGEN01_REG_SW_EN_V2_ODCLK_STG52V2);
		W2BYTEMSK(REG_1888_CKGEN01, 0x1, REG_1888_CKGEN01_REG_SW_EN_V2_ODCLK_STG62V2);
		W2BYTEMSK(REG_188C_CKGEN01, 0x1, REG_188C_CKGEN01_REG_SW_EN_V2_ODCLK_STG72V2);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2VBY1_V2);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2V2);
		W2BYTEMSK(REG_1870_CKGEN01, 0x1, REG_1870_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK2V2);
		W2BYTEMSK(REG_18B8_CKGEN01, 0x1, REG_18B8_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE02V2);
		W2BYTEMSK(REG_18BC_CKGEN01, 0x1, REG_18BC_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE12V2);
		W2BYTEMSK(REG_18B4_CKGEN01, 0x1, REG_18B4_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_FINIAL2V2);

	} else if ((priv->out_timing == E_4K2K_120HZ) ||
	(priv->out_timing == E_4K1K_240HZ)) {
		W2BYTEMSK(REG_1890_CKGEN01, 0x1, REG_1890_CKGEN01_REG_SW_EN_V2_ODCLK2V2);
		W2BYTEMSK(REG_1874_CKGEN01, 0x1, REG_1874_CKGEN01_REG_SW_EN_V2_ODCLK_STG12V2);
		W2BYTEMSK(REG_1878_CKGEN01, 0x1, REG_1878_CKGEN01_REG_SW_EN_V2_ODCLK_STG22V2);
		W2BYTEMSK(REG_187C_CKGEN01, 0x1, REG_187C_CKGEN01_REG_SW_EN_V2_ODCLK_STG32V2);
		W2BYTEMSK(REG_1880_CKGEN01, 0x1, REG_1880_CKGEN01_REG_SW_EN_V2_ODCLK_STG42V2);
		W2BYTEMSK(REG_1884_CKGEN01, 0x1, REG_1884_CKGEN01_REG_SW_EN_V2_ODCLK_STG52V2);
		W2BYTEMSK(REG_1888_CKGEN01, 0x1, REG_1888_CKGEN01_REG_SW_EN_V2_ODCLK_STG62V2);
		W2BYTEMSK(REG_188C_CKGEN01, 0x1, REG_188C_CKGEN01_REG_SW_EN_V2_ODCLK_STG72V2);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2VBY1_V2);
		W2BYTEMSK(REG_1820_CKGEN01, 0x1, REG_1820_CKGEN01_REG_SW_EN_MOD_V2_SR_WCLK2V2);
		W2BYTEMSK(REG_1870_CKGEN01, 0x1, REG_1870_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK2V2);
		W2BYTEMSK(REG_18B8_CKGEN01, 0x1, REG_18B8_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE02V2);
		W2BYTEMSK(REG_18BC_CKGEN01, 0x1, REG_18BC_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_PRE12V2);
		W2BYTEMSK(REG_18B4_CKGEN01, 0x1, REG_18B4_CKGEN01_REG_SW_EN_V2_MOD_SR_RCLK_FINIAL2V2);

	} else {
		printf("[%s] Not Support Output Timing for ext videoL\n",__func__);
	}
}

static void _ckg_sw_en_gfx(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		debug("[%s] get device private fail\n", __func__);
		return;
	}

	switch(priv->out_timing) {

	case E_4K2K_60HZ:
	case E_4K1K_120HZ:
		if (priv->vbo_byte == E_VBO_4BYTE_MODE) {
			W2BYTEMSK(REG_1840_CKGEN01, 0x1, REG_1840_CKGEN01_REG_SW_EN_O_ODCLK2O);
			W2BYTEMSK(REG_1828_CKGEN01, 0x1, REG_1828_CKGEN01_REG_SW_EN_O_ODCLK_STG12O);
			W2BYTEMSK(REG_182C_CKGEN01, 0x1, REG_182C_CKGEN01_REG_SW_EN_O_ODCLK_STG22O);
			W2BYTEMSK(REG_1830_CKGEN01, 0x1, REG_1830_CKGEN01_REG_SW_EN_O_ODCLK_STG32O);
			W2BYTEMSK(REG_1834_CKGEN01, 0x1, REG_1834_CKGEN01_REG_SW_EN_O_ODCLK_STG42O);
			W2BYTEMSK(REG_1838_CKGEN01, 0x1, REG_1838_CKGEN01_REG_SW_EN_O_ODCLK_STG52O);
			W2BYTEMSK(REG_183C_CKGEN01, 0x1, REG_183C_CKGEN01_REG_SW_EN_O_ODCLK_STG62O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2VBY1_O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2O);
			W2BYTEMSK(REG_1824_CKGEN01, 0x1, REG_1824_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK2O);
			W2BYTEMSK(REG_18A0_CKGEN01, 0x1, REG_18A0_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE02O);
			W2BYTEMSK(REG_18A4_CKGEN01, 0x1, REG_18A4_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE12O);
			W2BYTEMSK(REG_189C_CKGEN01, 0x1, REG_189C_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_FINIAL2O);

		} else if (priv->vbo_byte == E_VBO_5BYTE_MODE) {
			W2BYTEMSK(REG_1840_CKGEN01, 0x1, REG_1840_CKGEN01_REG_SW_EN_O_ODCLK2O);
			W2BYTEMSK(REG_1828_CKGEN01, 0x1, REG_1828_CKGEN01_REG_SW_EN_O_ODCLK_STG12O);
			W2BYTEMSK(REG_182C_CKGEN01, 0x1, REG_182C_CKGEN01_REG_SW_EN_O_ODCLK_STG22O);
			W2BYTEMSK(REG_1830_CKGEN01, 0x1, REG_1830_CKGEN01_REG_SW_EN_O_ODCLK_STG32O);
			W2BYTEMSK(REG_1834_CKGEN01, 0x1, REG_1834_CKGEN01_REG_SW_EN_O_ODCLK_STG42O);
			W2BYTEMSK(REG_1838_CKGEN01, 0x1, REG_1838_CKGEN01_REG_SW_EN_O_ODCLK_STG52O);
			W2BYTEMSK(REG_183C_CKGEN01, 0x1, REG_183C_CKGEN01_REG_SW_EN_O_ODCLK_STG62O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2VBY1_O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2O);
			W2BYTEMSK(REG_1824_CKGEN01, 0x1, REG_1824_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK2O);
			W2BYTEMSK(REG_18A0_CKGEN01, 0x1, REG_18A0_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE02O);
			W2BYTEMSK(REG_18A4_CKGEN01, 0x1, REG_18A4_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE12O);
			W2BYTEMSK(REG_189C_CKGEN01, 0x1, REG_189C_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_FINIAL2O);

		} else {
			printf("[%s]Output format not supported\n", __func__);
		}
	break;

	case E_4K2K_30HZ:
		if (priv->vbo_byte == E_VBO_4BYTE_MODE) {
			W2BYTEMSK(REG_1840_CKGEN01, 0x1, REG_1840_CKGEN01_REG_SW_EN_O_ODCLK2O);
			W2BYTEMSK(REG_1828_CKGEN01, 0x1, REG_1828_CKGEN01_REG_SW_EN_O_ODCLK_STG12O);
			W2BYTEMSK(REG_182C_CKGEN01, 0x1, REG_182C_CKGEN01_REG_SW_EN_O_ODCLK_STG22O);
			W2BYTEMSK(REG_1830_CKGEN01, 0x1, REG_1830_CKGEN01_REG_SW_EN_O_ODCLK_STG32O);
			W2BYTEMSK(REG_1834_CKGEN01, 0x1, REG_1834_CKGEN01_REG_SW_EN_O_ODCLK_STG42O);
			W2BYTEMSK(REG_1838_CKGEN01, 0x1, REG_1838_CKGEN01_REG_SW_EN_O_ODCLK_STG52O);
			W2BYTEMSK(REG_183C_CKGEN01, 0x1, REG_183C_CKGEN01_REG_SW_EN_O_ODCLK_STG62O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2VBY1_O);
			W2BYTEMSK(REG_1818_CKGEN01, 0x1, REG_1818_CKGEN01_REG_SW_EN_MOD_O_SR_WCLK2O);
			W2BYTEMSK(REG_1824_CKGEN01, 0x1, REG_1824_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK2O);
			W2BYTEMSK(REG_18A0_CKGEN01, 0x1, REG_18A0_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE02O);
			W2BYTEMSK(REG_18A4_CKGEN01, 0x1, REG_18A4_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_PRE12O);
			W2BYTEMSK(REG_189C_CKGEN01, 0x1, REG_189C_CKGEN01_REG_SW_EN_O_MOD_SR_RCLK_FINIAL2O);
		} else {
			printf("[%s]Output format not supported\n", __func__);
		}
		break;

	default:
		printf("[%s] output timing not supported\n", __func__);
		break;
	}

}

void _analog_drv_mode_sel(struct mtk_panel_priv *priv, uint32_t *drv_mode)
{
	#define DRV_MODE_VBY1 1
	#define DRV_MODE_LVDS 3

	if (!priv || !drv_mode) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	if (priv->linktype != E_LINK_NONE) {
		switch (priv->linktype) {
		case E_LINK_VB1:
			*drv_mode = DRV_MODE_VBY1;
		break;
		case E_LINK_LVDS:
			*drv_mode = DRV_MODE_LVDS;
		break;
		default:
			*drv_mode = 0;
			printf("[%s] Not support link type = %d\n",__func__, priv->linktype);
		break;
		}
	} else {
		*drv_mode = 0;
		printf("[%s] Not support link type = %d\n",__func__, priv->linktype);
	}
}

void analog_set_drv_mode(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	u8 u8laneCount;
	u8 u8DrvMode[E_BOOT_PNL_OUTPUT_LANE_MAX] = {false};
	u32 drv_mode = 0;
	en_boot_pnl_output_lane_config eoutput_lane_en_number = E_BOOT_PNL_OUTPUT_LANE0;

	if (!priv) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	//drv mode for each link type.
	_analog_drv_mode_sel(priv, &drv_mode);

	switch (priv->out_timing) {
		case E_FHD_60HZ:
		{
			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
				if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE2) || (drv_mode==0))
					u8DrvMode[u8laneCount] = 0;
				else
					u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		case E_FHD_120HZ:
		{
			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
				if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE4) || (drv_mode==0))
					u8DrvMode[u8laneCount] = 0;
				else
					u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
		{
			if (priv->lane_duplicate_en)
				eoutput_lane_en_number = E_BOOT_PNL_OUTPUT_LANE16;
			else
				eoutput_lane_en_number = E_BOOT_PNL_OUTPUT_LANE8;

			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
				if((priv->lane_info.def_layout[u8laneCount] >= eoutput_lane_en_number) || (drv_mode == 0))
					u8DrvMode[u8laneCount] = 0;
				else
					u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		case E_4K2K_120HZ:
		case E_4K2K_144HZ:
		case E_4K1K_240HZ:
		{
			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
				if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE16) || (drv_mode==0))
					u8DrvMode[u8laneCount] = 0;
				else
					u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		case E_8K4K_60HZ:
		{
			if (priv->out_format == E_OUTPUT_YUV422) {
				for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE16; u8laneCount++) {
					if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE16) || (drv_mode==0))
						u8DrvMode[u8laneCount] = 0;
					else
						u8DrvMode[u8laneCount] = drv_mode;
				}
			}

			if (priv->out_format == E_OUTPUT_YUV444 || priv->out_format == E_OUTPUT_RGB) {
				for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<E_BOOT_PNL_OUTPUT_LANE32; u8laneCount++) {
					if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE32) || (drv_mode==0))
						u8DrvMode[u8laneCount] = 0;
					else
						u8DrvMode[u8laneCount] = drv_mode;
				}
			}
		}
		break;
		case E_8K4K_144HZ:
		case E_8K4K_120HZ:
		{
			if (priv->out_format == E_OUTPUT_YUV444 || priv->out_format == E_OUTPUT_RGB) {
				for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE0; u8laneCount<=E_BOOT_PNL_OUTPUT_LANE63; u8laneCount++) {
						u8DrvMode[u8laneCount] = drv_mode;
				}
			}
		}
		break;
		default:
			printf("[%s][%d] OUTPUT TIMING Not Support\n", __func__, __LINE__);
		break;
	}

	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE0], REG_0154_MODA1_REG_DRV_MODE_CH00);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE1], REG_0154_MODA1_REG_DRV_MODE_CH01);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE2], REG_0154_MODA1_REG_DRV_MODE_CH02);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE3], REG_0154_MODA1_REG_DRV_MODE_CH03);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE4], REG_0154_MODA1_REG_DRV_MODE_CH04);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE5], REG_0154_MODA1_REG_DRV_MODE_CH05);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE6], REG_0154_MODA1_REG_DRV_MODE_CH06);
	W2BYTEMSK(REG_0154_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE7], REG_0154_MODA1_REG_DRV_MODE_CH07);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE8], REG_0158_MODA1_REG_DRV_MODE_CH08);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE9], REG_0158_MODA1_REG_DRV_MODE_CH09);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE10], REG_0158_MODA1_REG_DRV_MODE_CH10);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE11], REG_0158_MODA1_REG_DRV_MODE_CH11);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE12], REG_0158_MODA1_REG_DRV_MODE_CH12);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE13], REG_0158_MODA1_REG_DRV_MODE_CH13);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE14], REG_0158_MODA1_REG_DRV_MODE_CH14);
	W2BYTEMSK(REG_0158_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE15], REG_0158_MODA1_REG_DRV_MODE_CH15);
	W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE16], REG_015C_MODA1_REG_DRV_MODE_CH16);
	W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE17], REG_015C_MODA1_REG_DRV_MODE_CH17);
	W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE18], REG_015C_MODA1_REG_DRV_MODE_CH18);
	W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE19], REG_015C_MODA1_REG_DRV_MODE_CH19);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE20], REG_0154_MODA2_REG_DRV_MODE_CH20);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE21], REG_0154_MODA2_REG_DRV_MODE_CH21);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE22], REG_0154_MODA2_REG_DRV_MODE_CH22);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE23], REG_0154_MODA2_REG_DRV_MODE_CH23);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE24], REG_0154_MODA2_REG_DRV_MODE_CH24);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE25], REG_0154_MODA2_REG_DRV_MODE_CH25);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE26], REG_0154_MODA2_REG_DRV_MODE_CH26);
	W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE27], REG_0154_MODA2_REG_DRV_MODE_CH27);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE28], REG_0158_MODA2_REG_DRV_MODE_CH28);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE29], REG_0158_MODA2_REG_DRV_MODE_CH29);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE30], REG_0158_MODA2_REG_DRV_MODE_CH30);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE31], REG_0158_MODA2_REG_DRV_MODE_CH31);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE32], REG_0158_MODA2_REG_DRV_MODE_CH32);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE33], REG_0158_MODA2_REG_DRV_MODE_CH33);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE34], REG_0158_MODA2_REG_DRV_MODE_CH34);
	W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE35], REG_0158_MODA2_REG_DRV_MODE_CH35);
	W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE36], REG_015C_MODA2_REG_DRV_MODE_CH36);
	W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE37], REG_015C_MODA2_REG_DRV_MODE_CH37);
	W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE38], REG_015C_MODA2_REG_DRV_MODE_CH38);
	W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE39], REG_015C_MODA2_REG_DRV_MODE_CH39);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE40], REG_0154_MODA3_REG_DRV_MODE_CH40);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE41], REG_0154_MODA3_REG_DRV_MODE_CH41);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE42], REG_0154_MODA3_REG_DRV_MODE_CH42);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE43], REG_0154_MODA3_REG_DRV_MODE_CH43);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE44], REG_0154_MODA3_REG_DRV_MODE_CH44);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE45], REG_0154_MODA3_REG_DRV_MODE_CH45);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE46], REG_0154_MODA3_REG_DRV_MODE_CH46);
	W2BYTEMSK(REG_0154_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE47], REG_0154_MODA3_REG_DRV_MODE_CH47);

	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE48], REG_0158_MODA3_REG_DRV_MODE_CH48);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE49], REG_0158_MODA3_REG_DRV_MODE_CH49);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE50], REG_0158_MODA3_REG_DRV_MODE_CH50);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE51], REG_0158_MODA3_REG_DRV_MODE_CH51);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE52], REG_0158_MODA3_REG_DRV_MODE_CH52);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE53], REG_0158_MODA3_REG_DRV_MODE_CH53);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE54], REG_0158_MODA3_REG_DRV_MODE_CH54);
	W2BYTEMSK(REG_0158_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE55], REG_0158_MODA3_REG_DRV_MODE_CH55);

	W2BYTEMSK(REG_015C_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE56], REG_015C_MODA3_REG_DRV_MODE_CH56);
	W2BYTEMSK(REG_015C_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE57], REG_015C_MODA3_REG_DRV_MODE_CH57);
	W2BYTEMSK(REG_015C_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE58], REG_015C_MODA3_REG_DRV_MODE_CH58);
	W2BYTEMSK(REG_015C_MODA3, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE59], REG_015C_MODA3_REG_DRV_MODE_CH59);

	W2BYTEMSK(REG_0154_MODA4, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE60], REG_0154_MODA4_REG_DRV_MODE_CH60);
	W2BYTEMSK(REG_0154_MODA4, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE61], REG_0154_MODA4_REG_DRV_MODE_CH61);
	W2BYTEMSK(REG_0154_MODA4, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE62], REG_0154_MODA4_REG_DRV_MODE_CH62);
	W2BYTEMSK(REG_0154_MODA4, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE63], REG_0154_MODA4_REG_DRV_MODE_CH63);
}

void analog_ext_video_set_drv_mode(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint8_t u8laneCount;
	uint8_t u8DrvMode[E_BOOT_PNL_OUTPUT_LANE_MAX] = {false};
	uint32_t drv_mode = 0;
	bool b_is_ext_video_update = false;
	en_boot_pnl_output_lane_config e_output_lane_en_number =
		E_BOOT_PNL_OUTPUT_LANE0;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	//drv mode for each link type.
	_analog_drv_mode_sel(priv, &drv_mode);

	switch (priv->out_timing) {
	case E_4K2K_60HZ:
	case E_4K1K_120HZ:
	{
		b_is_ext_video_update = true;
		if (priv->lane_duplicate_en)
			e_output_lane_en_number = E_BOOT_PNL_OUTPUT_LANE32;
		else
			e_output_lane_en_number = E_BOOT_PNL_OUTPUT_LANE24;
		for (u8laneCount = E_BOOT_PNL_OUTPUT_LANE16;
		u8laneCount < E_BOOT_PNL_OUTPUT_LANE32; u8laneCount++) {
			if (priv->lane_info.def_layout[u8laneCount] >=
			e_output_lane_en_number || drv_mode == 0)
				u8DrvMode[u8laneCount] = 0;
			else
				u8DrvMode[u8laneCount] = drv_mode;
		}
	}
	break;
	case E_4K2K_120HZ:
	case E_4K1K_240HZ:
	{
		b_is_ext_video_update = true;
		for (u8laneCount = E_BOOT_PNL_OUTPUT_LANE16;
		u8laneCount < E_BOOT_PNL_OUTPUT_LANE32; u8laneCount++)
			u8DrvMode[u8laneCount] = drv_mode;
	}
	break;
	default:
		b_is_ext_video_update = false;
		printf("[%s] Output timing NOT supported\n", __func__);
	break;
	}

	if (b_is_ext_video_update) {
		W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE16], REG_015C_MODA1_REG_DRV_MODE_CH16);
		W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE17], REG_015C_MODA1_REG_DRV_MODE_CH17);
		W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE18], REG_015C_MODA1_REG_DRV_MODE_CH18);
		W2BYTEMSK(REG_015C_MODA1, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE19], REG_015C_MODA1_REG_DRV_MODE_CH19);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE20], REG_0154_MODA2_REG_DRV_MODE_CH20);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE21], REG_0154_MODA2_REG_DRV_MODE_CH21);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE22], REG_0154_MODA2_REG_DRV_MODE_CH22);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE23], REG_0154_MODA2_REG_DRV_MODE_CH23);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE24], REG_0154_MODA2_REG_DRV_MODE_CH24);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE25], REG_0154_MODA2_REG_DRV_MODE_CH25);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE26], REG_0154_MODA2_REG_DRV_MODE_CH26);
		W2BYTEMSK(REG_0154_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE27], REG_0154_MODA2_REG_DRV_MODE_CH27);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE28], REG_0158_MODA2_REG_DRV_MODE_CH28);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE29], REG_0158_MODA2_REG_DRV_MODE_CH29);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE30], REG_0158_MODA2_REG_DRV_MODE_CH30);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE31], REG_0158_MODA2_REG_DRV_MODE_CH31);
	}
}

void analog_gfx_set_drv_mode(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint8_t u8laneCount;
	uint8_t u8DrvMode[E_BOOT_PNL_OUTPUT_LANE_MAX] = {false};
	uint32_t drv_mode = 0;
	bool bIsGfxVideoUpdate = false;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return;
	}

	//drv mode for each link type.
	_analog_drv_mode_sel(priv, &drv_mode);

	switch (priv->out_timing) {
		case E_4K2K_30HZ:
		{
			bIsGfxVideoUpdate = true;
			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE32; u8laneCount<E_BOOT_PNL_OUTPUT_LANE40; u8laneCount++) {
				if((priv->lane_info.def_layout[u8laneCount]>=E_BOOT_PNL_OUTPUT_LANE36) || (drv_mode==0))
					u8DrvMode[u8laneCount] = 0;
				else
					u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
		{
			bIsGfxVideoUpdate = true;
			for(u8laneCount=E_BOOT_PNL_OUTPUT_LANE32; u8laneCount<E_BOOT_PNL_OUTPUT_LANE40; u8laneCount++) {
				u8DrvMode[u8laneCount] = drv_mode;
			}
		}
		break;
		default:
			bIsGfxVideoUpdate = false;
			printf("[%s] Output timing NOT supported\n", __func__);
		break;
	}

	if (bIsGfxVideoUpdate == true) {
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE32], REG_0158_MODA2_REG_DRV_MODE_CH32);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE33], REG_0158_MODA2_REG_DRV_MODE_CH33);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE34], REG_0158_MODA2_REG_DRV_MODE_CH34);
		W2BYTEMSK(REG_0158_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE35], REG_0158_MODA2_REG_DRV_MODE_CH35);
		W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE36], REG_015C_MODA2_REG_DRV_MODE_CH36);
		W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE37], REG_015C_MODA2_REG_DRV_MODE_CH37);
		W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE38], REG_015C_MODA2_REG_DRV_MODE_CH38);
		W2BYTEMSK(REG_015C_MODA2, u8DrvMode[E_BOOT_PNL_OUTPUT_LANE39], REG_015C_MODA2_REG_DRV_MODE_CH39);
	}
}

int mtk_analog_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	//TOP PAD setting
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0400) {
		//W2BYTEMSK(0x60041C, 0x01, Fld(8,0,AC_FULLB0)); //switc to nonpm dmd bank
		//W2BYTEMSK(0xE04004, 0x0, Fld(1,13,AC_MSKB1)); //script value, ver4 release dmd ana misc reset
		//W2BYTEMSK(0xE05CF4, 0x1, Fld(1,0,AC_MSKB0)); //script value, ver4 dmd atop Tx CLK en [0] reg_tx_enable

		W2BYTEMSK(0x202C0, 0x0, Fld(1,0,AC_MSKB0)); //script value, ver4 dmd atop Tx CLK en [0] reg_tx_enable

		//W2BYTEMSK(0x4847DC, 0xC291, Fld(16,0,AC_FULLW10)); scriipt value
		W2BYTEMSK(REG_01DC_MODA1_V004,0x1 ,REG_01DC_MODA1_V004_REG_SW_RSTZ_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x0 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SWAP_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x0 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SWAP_V2_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x0 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SWAP_OSD_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x1 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SEL_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x1 ,REG_01DC_MODA1_V004_REG_VBY1_PAD_SEL_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x2 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SEL_V2_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x4 ,REG_01DC_MODA1_V004_REG_VBY1_LOCKN_HTPDN_SEL_OSD_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V004,0x1 ,REG_01DC_MODA1_V004_REG_VBY1_PAD_SEL_OSD_01DC);
	} else if (priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
#if 0
		// LPLL ref clock from DEMOD_ATOP, the register should control by sboot scripts
		// W2BYTEMSK(0x60041C, 0x01, Fld(8,0,AC_FULLB0)); //switc to nonpm dmd bank
		W2BYTEMSK(0xE04004, 0x0, Fld(1,13,AC_MSKB1)); //script value, ver4 release dmd ana misc reset
		W2BYTEMSK(0xE05CF4, 0x1, Fld(1,0,AC_MSKB0)); //script value, ver4 dmd atop Tx CLK en [0] reg_tx_enable
		//W2BYTEMSK(0x202C0, 0x0, Fld(1,0,AC_MSKB0)); // prdt, eth
		//W2BYTEMSK(0x4847DC, 0xC291, Fld(16,0,AC_FULLW10)); scriipt value
#endif
		// TODO: MT5879: should use pinctrl API,  PAD_LOCK for LOCK_V, TCON5 for HTPD_V, TCON6 for LOCK_O, TCON7 for HTPD_O
		W2BYTEMSK(0x2E40D4, 0x4444, Fld(16,0,AC_FULLW10));  // bank 0x1720_35: [3:0]lockV_psel, [7:4]HtpdV_psel, [11:8]LockO_psel, [15:12]HtpdO_psel
		W2BYTEMSK(REG_01A0_MODD1_V005, 0x1, REG_01A0_MODD1_V005_REG_EXT_PAD_OEN_01A0); // LOCK_PAD OEN = input
		W2BYTEMSK(REG_0150_TCON_GPO_COM_15_00, 0x1 ,Fld(1,5,AC_MSKB0)); // TCON5 oen = input
		W2BYTEMSK(REG_0150_TCON_GPO_COM_15_00, 0x1 ,Fld(1,6,AC_MSKB0)); // TCON6 oen = input
		W2BYTEMSK(REG_0150_TCON_GPO_COM_15_00, 0x1 ,Fld(1,7,AC_MSKB0)); // TCON7 oen = input
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x1, REG_01DC_MODA1_V005_REG_SW_RSTZ_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x0, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SWAP_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x0, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SWAP_V2_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x0, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SWAP_OSD_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x1, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SEL_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x0, REG_01DC_MODA1_V005_REG_VBY1_PAD_SEL_V1_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x2, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SEL_V2_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x4, REG_01DC_MODA1_V005_REG_VBY1_LOCKN_HTPDN_SEL_OSD_01DC);
		W2BYTEMSK(REG_01DC_MODA1_V005, 0x0, REG_01DC_MODA1_V005_REG_VBY1_PAD_SEL_OSD_01DC);
	} else {
		W2BYTEMSK(0x2E4030, 0x4220, Fld(16,0,AC_FULLW10));
		W2BYTEMSK(0x2E4034, 0x0111, Fld(16,0,AC_FULLW10));
	}

	// pn swap
	// will implement in mtk_swing_level_setting
	// which need to after rcon setting
	if (priv->linktype == E_LINK_VB1)
		vby1_set_pn_swap(priv->pnl_lib_version);

	//after 5897, lpll table control
	//after 5897, REG_0048_MODA1 be controlled in function mtk_moda_dump_clk_tbl
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0300 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
		W2BYTEMSK(REG_0160_MODA1, 0x200, REG_0160_MODA1_REG_HW_RINT_TH); //HW RINT TH
		W2BYTEMSK(REG_0000_MODA1, 0xFF, REG_0000_MODA1_REG_EN_CLK_BANK); //ENABLE CLK
		W2BYTEMSK(REG_0048_MODA1, 0x1, REG_0048_MODA1_REG_GCR_CKEN);
		W2BYTEMSK(REG_0048_MODA1, 0x1, REG_0048_MODA1_REG_GCR_CKEN_PATH2);
		W2BYTEMSK(REG_0004_MODA1, 0x0, REG_0004_MODA1_REG_PD_MOD);
	}

	//for report hw rint value
	W2BYTEMSK(REG_1810_CKGEN01,0x1,REG_1810_CKGEN01_REG_SW_EN_MOD_A_ODCLK2MOD_A);
	W2BYTEMSK(REG_1894_CKGEN01,0x1,REG_1894_CKGEN01_REG_SW_EN_XTAL_12M2MOD_A);

	return 0;
}

int mtk_ext_video_analog_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	//after 5897, lpll table control
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0300)
		analog_ext_video_set_drv_mode(dev);

	return 0;
}

int mtk_gfx_analog_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	//after 5897, lpll table control
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0300)
		analog_gfx_set_drv_mode(dev);

	return 0;
}


int mtk_pll_powerdown(struct udevice *dev,bool enable)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	UBOOT_DEBUG("power down enable = %d\n", enable);
	if (enable) {
		W2BYTEMSK(REG_0004_MODA1, 0x0, REG_0004_MODA1_REG_GCR_EN_REG);	//MOD power regulator
		W2BYTEMSK(REG_01B0_LPLL, 0x1, REG_01B0_LPLL_REG_MPLL_PD);	//MPLL power down
		udelay(500);//delay 500us.
		W2BYTEMSK(REG_01B0_LPLL, 0x1, REG_01B0_LPLL_REG_MPLL_CLK_ADC_VCO_DIV2_PD);	//864M clk to LPLL
		W2BYTEMSK(REG_000C_LPLL, 0x1, REG_000C_LPLL_REG_LPLL1_PD);	//LPLL-VCO1 power down
		//W2BYTEMSK(reg_lpll2_pd, 0x20, 0x20); //LPLL-VCO2 power down
        } else {
        	// after version 2, lpll table control MODA1 reg, except MODA1 reg.01[15:8]
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0300)
			W2BYTEMSK(REG_0000_MODA1, 0xFF, REG_0000_MODA1_REG_EN_CLK_BANK); //ENABLE CLK, turn on all channels.

        	W2BYTEMSK(REG_0004_MODA1, 0xFF, REG_0004_MODA1_REG_GCR_EN_REG); //MOD power regulator, [9] MODA power on LPLL
		mtk_mpll_dump_clk_tbl(dev);
		W2BYTEMSK(REG_01B0_LPLL, 0x0, REG_01B0_LPLL_REG_MPLL_PD);	//MPLL power down
		udelay(500);//delay 500us.
		W2BYTEMSK(REG_01B0_LPLL, 0x0, REG_01B0_LPLL_REG_MPLL_CLK_ADC_VCO_DIV2_PD);	//864M clk to LPLL
		W2BYTEMSK(REG_000C_LPLL, 0x0, REG_000C_LPLL_REG_LPLL1_PD);	//LPLL-VCO1 power down
		W2BYTEMSK(REG_00CC_LPLL, 0x0, REG_00CC_LPLL_REG_LPLL2_PD); //LPLL-VCO2 power down
	}

    return 0;
}

int mtk_pnl_xtal_lpll_ckg_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	if (priv->linktype == E_LINK_NONE)
		return 0;

	//RG_PD_LPLL_SYN_432M setting
	W2BYTEMSK(REG_00BC_LPLL, 0, REG_00BC_LPLL_REG_PD_LPLL_SYN432);//bit3

	//clock setting
	W2BYTEMSK(REG_0900_CKGEN01, 0x0, REG_0900_CKGEN01_REG_CKG_LPLL_SYN_432);

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0600)
		W2BYTEMSK(REG_0908_CKGEN01, 0x0, REG_0908_CKGEN01_REG_CKG_LPLL_SYN_864);

	W2BYTEMSK(REG_0A68_CKGEN01, 0x0, REG_0A68_CKGEN01_REG_CKG_XTAL_24M_MOD);
	W2BYTEMSK(REG_1808_CKGEN01, 0x1, REG_1808_CKGEN01_REG_SW_EN_LPLL_SYN_4322FPLL);

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0600)
		W2BYTEMSK(REG_180C_CKGEN01, 0x1, REG_180C_CKGEN01_REG_SW_EN_LPLL_SYN_8642FPLL);

	W2BYTEMSK(REG_1898_CKGEN01, 0x1, REG_1898_CKGEN01_REG_SW_EN_XTAL_24M2FPLL);

	if (priv->pnl_lib_version != BOOT_PNL_VERSION0400 &&
		priv->pnl_lib_version != BOOT_PNL_VERSION0500)
		W2BYTEMSK(REG_17F0_CKGEN01, 0x1, REG_17F0_CKGEN01_REG_SW_EN_LPLL_SYN_864_PIPE2FPLL);

	//REG_SYNC_864_SW_RST
	W2BYTEMSK(0x486168, 0x1, Fld(1,15,AC_MSKB1));//bit15

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
		// sw_en for fpll
		W2BYTEMSK(REG_17FC_CKGEN01, 0x1, REG_17FC_CKGEN01_REG_SW_EN_IDCLK2FPLL);
		W2BYTEMSK(REG_1800_CKGEN01, 0x1, REG_1800_CKGEN01_REG_SW_EN_LPLL_ODCLK_NODIV2FPLL);
		W2BYTEMSK(REG_1804_CKGEN01, 0x1, REG_1804_CKGEN01_REG_SW_EN_LPLL_ODCLK2FPLL);
	}

	return 0;
}

static int _mtk_pnl_tcon_ckg_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv) {
		printf("%s(%d): private data is NULL\n", __func__, __LINE__);
		return -ENXIO;
	}

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
		priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
		switch (priv->out_timing) {
		case E_4K2K_60HZ: // 2p_odclk /2
		case E_4K1K_120HZ:
		default:
			//mod_d ckgen, [0]:gate [1]:inv [4:2]:div sel
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			//mod v1 ckgen, [0]:gate [1]:inv [2]: 0:xtal 1:odclk [5:3]:div sel
			W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x0C, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x0C, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x0C, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x0C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x0C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			W2BYTEMSK(REG_09B8_CKGEN01, 0x0C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x0C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			// GPO clk
			W2BYTEMSK(REG_0198_CKGEN01_V004, 0x01, REG_0198_CKGEN01_V004_REG_CKG_S_GPO_0198);
			//serial top wclk, [0]:gate [1]:inv [4:2]:div sel [5] 0: odclk 1: fifo clk
			if (IS_TCON_FIFOCLK_SUPPORT(priv->linktype)) {
				// ISP/USIT/CHPI case; sr_wclk is from fifo clock
				W2BYTEMSK(REG_0920_CKGEN01, 0x20, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
				W2BYTEMSK(REG_091C_CKGEN01, 0x20, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
			} else {
				// EPI/CMPI/VBy1 case; sr_wclk is from odclk
				W2BYTEMSK(REG_0920_CKGEN01, 0x04, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
				W2BYTEMSK(REG_091C_CKGEN01, 0x04, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
			}
			break;
		case E_4K2K_120HZ: // 2p_odclk /1
		case E_4K2K_144HZ:
		case E_4K1K_144HZ:
		case E_4K1K_240HZ:
			//mod_d ckgen, [0]:gate [1]:inv [4:2]:div sel
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			//mod v1 ckgen, [0]:gate [1]:inv [2]: 0:xtal 1:odclk [5:3]:div sel
			W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			W2BYTEMSK(REG_0988_CKGEN01, 0x04, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x04, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x04, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x04, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x04, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x04, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			W2BYTEMSK(REG_09B8_CKGEN01, 0x04, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x04, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			// GPO clk
			W2BYTEMSK(REG_0198_CKGEN01_V004, 0x00, REG_0198_CKGEN01_V004_REG_CKG_S_GPO_0198);
			//serial top wclk, [0]:gate [1]:inv [4:2]:div sel [5] 0: odclk 1: fifo clk
			if (IS_TCON_FIFOCLK_SUPPORT(priv->linktype)) {
				// ISP/USIT/CHPI case; sr_wclk is from fifo clock
				if (priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
					W2BYTEMSK(REG_0920_CKGEN01_V005, 0x08, REG_0920_CKGEN01_V005_REG_CKG_S_MOD_V1_SR_WCLK_0920);
					W2BYTEMSK(REG_091C_CKGEN01_V005, 0x08, REG_091C_CKGEN01_V005_REG_CKG_S_MOD_O_SR_WCLK_091C);
				} else {
					W2BYTEMSK(REG_0920_CKGEN01, 0x20, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
					W2BYTEMSK(REG_091C_CKGEN01, 0x20, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
				}
			} else {
				// EPI/CMPI/VBy1 case; sr_wclk is from odclk
				if (priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
					W2BYTEMSK(REG_0920_CKGEN01_V005, 0x00, REG_0920_CKGEN01_V005_REG_CKG_S_MOD_V1_SR_WCLK_0920);
					W2BYTEMSK(REG_091C_CKGEN01_V005, 0x00, REG_091C_CKGEN01_V005_REG_CKG_S_MOD_O_SR_WCLK_091C);
				} else {
					W2BYTEMSK(REG_0920_CKGEN01, 0x00, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
					W2BYTEMSK(REG_091C_CKGEN01, 0x00, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
				}
			}
			break;
		}
	}
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_002GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_012GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_022GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_032GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_042GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_052GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_062GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_072GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_082GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_092GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_102GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_112GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_122GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_132GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_142GPO_1498);
	W2BYTEMSK(REG_1498_CKGEN01_V004, 0x01, REG_1498_CKGEN01_V004_REG_SW_EN_GPO_152GPO_1498);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_162GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_172GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_182GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_192GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_202GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_212GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_222GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_232GPO_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x00, REG_14A0_CKGEN01_V004_REG_SW_EN_GPO_ZONE2GPO_14A0);
	if(IS_TCON_FIFOCLK_SUPPORT(priv->linktype))
		W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_MOD_V1_SR_WCLK2TCON_14A0);
	else
		W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x00, REG_14A0_CKGEN01_V004_REG_SW_EN_MOD_V1_SR_WCLK2TCON_14A0);
	// W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x00, REG_14A0_CKGEN01_V004_REG_SW_EN_PWM_DAC2PWM_DAC_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_V1_ODCLK_STG72TCON_14A0);
	W2BYTEMSK(REG_14A0_CKGEN01_V004, 0x01, REG_14A0_CKGEN01_V004_REG_SW_EN_XTAL_12M2TCON_14A0);

	return 0;
}

int mtk_pnl_video_ckg_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	en_vbo_bytemode vbo_byte = E_VBO_NO_LINK;

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	if (priv->linktype == E_LINK_NONE)
		return 0;

	debug("[%s] ckgen setting IN\n",__func__);

	vbo_byte = priv->vbo_byte;

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
		if (priv->linktype == E_LINK_VB1) {
			switch (priv->out_timing) {
			case E_FHD_60HZ:
				W2BYTEMSK(REG_0918_CKGEN01, 0xC, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				break;
			case E_4K2K_120HZ:
			case E_4K2K_144HZ:
			case E_4K1K_240HZ:
				W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				break;
			default:
				W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				break;
			}
		} else if (priv->linktype == E_LINK_LVDS) {
			W2BYTEMSK(REG_0918_CKGEN01, 0xC, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
		} else {
			W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
		}
	} else {
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0300) && (priv->out_timing == E_8K4K_60HZ))
			W2BYTEMSK(REG_0918_CKGEN01, 0xC, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
		else
			W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
	}
	switch (priv->out_timing) {
	case E_8K4K_144HZ:
	case E_8K4K_120HZ:
		W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
		W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
		W2BYTEMSK(REG_0A84_CKGEN01, 0x04, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
		W2BYTEMSK(REG_0A84_CKGEN01, 0x04, REG_0A84_CKGEN01_REG_CKG_V2_ODCLK_SCSCL);
		W2BYTEMSK(REG_0A8C_CKGEN01, 0x04, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
		W2BYTEMSK(REG_0A8C_CKGEN01, 0x04, REG_0A8C_CKGEN01_REG_CKG_V2_ODCLK_SCIP);
		W2BYTEMSK(REG_0A90_CKGEN01, 0x04, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
		W2BYTEMSK(REG_0A90_CKGEN01, 0x04, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
		W2BYTEMSK(REG_0A94_CKGEN01, 0x04, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
		W2BYTEMSK(REG_0A94_CKGEN01, 0x04, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
		W2BYTEMSK(REG_0A98_CKGEN01, 0x04, REG_0A98_CKGEN01_REG_CKG_V2_ODCLK_SCTCON);
		W2BYTEMSK(REG_0ACC_CKGEN01, 0x04, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
		W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		W2BYTEMSK(REG_0988_CKGEN01, 0x04, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x04, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x04, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0x04, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x04, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x04, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		W2BYTEMSK(REG_09B8_CKGEN01, 0x0C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
		W2BYTEMSK(REG_0A58_CKGEN01, 0x14, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
		//SRAM share
		W2BYTEMSK(REG_09E0_CKGEN01, 0x04, REG_09E0_CKGEN01_REG_CKG_V2_ODCLK_STG3);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0x0C, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
		W2BYTEMSK(REG_0924_CKGEN01, 0x0C, REG_0924_CKGEN01_REG_CKG_MOD_V2_SR_WCLK);
		W2BYTEMSK(REG_091C_CKGEN01, 0x0C, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
		W2BYTEMSK(REG_0928_CKGEN01, 0x00, REG_0928_CKGEN01_REG_CKG_O_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A10_CKGEN01, 0x00, REG_0A10_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A18_CKGEN01, 0x00, REG_0A18_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A08_CKGEN01, 0x00, REG_0A08_CKGEN01_REG_CKG_O_MOD_SR_RCLK_FINIAL);
		//serial top rclk
		W2BYTEMSK(REG_0970_CKGEN01, 0x00, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A28_CKGEN01, 0x00, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A30_CKGEN01, 0x00, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A20_CKGEN01, 0x00, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		W2BYTEMSK(REG_09C0_CKGEN01, 0x00, REG_09C0_CKGEN01_REG_CKG_V2_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A40_CKGEN01, 0x00, REG_0A40_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A48_CKGEN01, 0x00, REG_0A48_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A38_CKGEN01, 0x00, REG_0A38_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_FINIAL);
		break;
	case E_8K4K_60HZ:
		if ((priv->out_format == E_OUTPUT_YUV444)||
				(priv->out_format == E_OUTPUT_RGB)) {//32
				//clock setting
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
				W2BYTEMSK(REG_0918_CKGEN01, 0xC, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				W2BYTEMSK(REG_0978_CKGEN01, 0xC, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0xC, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A84_CKGEN01, 0xC, REG_0A84_CKGEN01_REG_CKG_V2_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0xC, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0xC, REG_0A8C_CKGEN01_REG_CKG_V2_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0xC, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0xC, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0xC, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0xC, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0A98_CKGEN01, 0xC, REG_0A98_CKGEN01_REG_CKG_V2_ODCLK_SCTCON);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0xC, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
				W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			} else {
				W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				W2BYTEMSK(REG_0978_CKGEN01, 0x4, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x4, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x4, REG_0A84_CKGEN01_REG_CKG_V2_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x4, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x4, REG_0A8C_CKGEN01_REG_CKG_V2_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0A98_CKGEN01, 0x4, REG_0A98_CKGEN01_REG_CKG_V2_ODCLK_SCTCON);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x4, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x4, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
				W2BYTEMSK(REG_0988_CKGEN01, 0x4, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			}
			W2BYTEMSK(REG_0990_CKGEN01, 0xC, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0xC, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0xC, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0xC, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0xC, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			W2BYTEMSK(REG_09B8_CKGEN01, 0x14, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			//SRAM share
			W2BYTEMSK(REG_09E0_CKGEN01, 0xC, REG_09E0_CKGEN01_REG_CKG_V2_ODCLK_STG3);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
			W2BYTEMSK(REG_0924_CKGEN01, 0xC, REG_0924_CKGEN01_REG_CKG_MOD_V2_SR_WCLK);
			//serial top rclk
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
			W2BYTEMSK(REG_09C0_CKGEN01, 0x0, REG_09C0_CKGEN01_REG_CKG_V2_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A40_CKGEN01, 0x0, REG_0A40_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A48_CKGEN01, 0x0, REG_0A48_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A38_CKGEN01, 0x0, REG_0A38_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_FINIAL);
			debug("[%s] ckgen setting for timing:8K4K@60 ,format: YUV444\n",__func__);
		} else if (priv->out_format == E_OUTPUT_YUV422) {//16
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
				W2BYTEMSK(REG_0918_CKGEN01, 0x0C, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x0C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x0C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x0C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			} else {
				W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
				W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x04, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x04, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x04, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x04, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x04, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x04, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x04, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			}
			W2BYTEMSK(REG_0988_CKGEN01, 0xC, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x14, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x14, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x14, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x14, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x14, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
			//serial top rclk
			W2BYTEMSK(REG_0970_CKGEN01, 0x4, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x4, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x4, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x4, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
			debug("[%s] ckgen setting for timing:8K4K@60 ,format: YUV422\n",__func__);
		} else {
			//error handling, or not support color format
		}
		break;
	case E_4K2K_120HZ: // 16
	case E_4K1K_240HZ:
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500)) {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x4, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x4, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x4, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
		} else if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
			W2BYTEMSK(REG_0918_CKGEN01, 0x4, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x14, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0980_CKGEN01, 0x14, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x14, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x14, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x14, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
		} else {
			W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x0C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x0C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x0C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
		}
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0300)
			W2BYTEMSK(REG_0988_CKGEN01, 0x14, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		else
			W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x14, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x14, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0x14, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x14, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x14, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
		W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);

		//serial top rclk
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500) && (vbo_byte == E_VBO_5BYTE_MODE)) {
			W2BYTEMSK(REG_0970_CKGEN01, 0x4, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x4, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x4, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x4, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		} else {
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		}
		debug("[%s] ckgen setting for timing:4K2K@120 ,format: YUV444\n",__func__);
		break;
	case E_4K2K_144HZ:
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500)) {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x4, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x4, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x4, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x4, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x4, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		} else {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x0C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x0C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x0C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		}

		if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
			W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x0C, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x0C, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x0C, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x0C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x0C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		} else {
			W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x14, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x14, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x14, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x14, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x14, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		}

		if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
			W2BYTEMSK(REG_09B8_CKGEN01, 0x14, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x14, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0x08, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
		} else {
			W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
		}

		//serial top rclk
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500) && (vbo_byte == E_VBO_5BYTE_MODE)) {
			W2BYTEMSK(REG_0970_CKGEN01, 0x4, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x4, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x4, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x4, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		} else {
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		}
		debug("[%s] ckgen setting for timing:4K2K@120 ,format: YUV444\n",__func__);
		break;
	case E_4K1K_144HZ:
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500))
			W2BYTEMSK(REG_0978_CKGEN01, 0x04, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
		else
			W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);

		W2BYTEMSK(REG_0ACC_CKGEN01, 0x0C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);

		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500))
			W2BYTEMSK(REG_0980_CKGEN01, 0x04, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		else
			W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);

		W2BYTEMSK(REG_0988_CKGEN01, 0x0C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x14, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x14, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0x14, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x14, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x14, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
		W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0x8, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);

				//serial top rclk
		if (((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500)) && (vbo_byte == E_VBO_5BYTE_MODE)) {
			W2BYTEMSK(REG_0970_CKGEN01, 0x4, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x4, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x4, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x4, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		} else {
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		}
		debug("[%s] ckgen setting for timing:4K1K@144 ,format: YUV444\n",__func__);
		break;

	case E_4K2K_60HZ: //8
	case E_4K1K_120HZ:
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0203) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0400) ||
			(priv->pnl_lib_version == BOOT_PNL_VERSION0500)) {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x0C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x0C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x0C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x0C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x0C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x0C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x0C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		} else if (priv->pnl_lib_version == BOOT_PNL_VERSION0300) {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x1C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x1C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x1C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x1C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
		} else {
			W2BYTEMSK(REG_0918_CKGEN01, 0x04, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
			W2BYTEMSK(REG_0978_CKGEN01, 0x14, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x14, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x14, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x14, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x14, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		}

		W2BYTEMSK(REG_0988_CKGEN01, 0x14, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x1C, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x1C, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0x1C, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x1C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x1C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
		W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);

		//serial top rclk
		if ((priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500) && (vbo_byte == E_VBO_5BYTE_MODE)) {
			W2BYTEMSK(REG_0970_CKGEN01, 0x4, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x4, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x4, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x4, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		} else {
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		}
		debug("[%s] ckgen setting for timing:4K2K@60 ,format: YUV444\n",__func__);
		break;
	case E_FHD_120HZ: // 4
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
			W2BYTEMSK(REG_0978_CKGEN01, 0x14, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x14, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x14, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x14, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x14, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x14, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x14, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		} else {
			W2BYTEMSK(REG_0978_CKGEN01, 0x1C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
			W2BYTEMSK(REG_0A84_CKGEN01, 0x1C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
			W2BYTEMSK(REG_0A8C_CKGEN01, 0x1C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
			W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
			W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
			W2BYTEMSK(REG_0ACC_CKGEN01, 0x1C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
			W2BYTEMSK(REG_0980_CKGEN01, 0x1C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		}
		W2BYTEMSK(REG_0988_CKGEN01, 0x1C, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x24, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x24, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0x1C, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x1C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x1C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
		W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
		//serial top rclk
		W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
		debug("[%s] ckgen setting for timing:FHD@120 ,format: YUV444\n",__func__);
		break;
	case E_FHD_60HZ:
		if (priv->linktype == E_LINK_LVDS) {
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
				W2BYTEMSK(REG_0978_CKGEN01, 0x1C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x1C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x1C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x1C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x1C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			} else {
				W2BYTEMSK(REG_0978_CKGEN01, 0x24, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x24, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x24, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x24, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x24, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x24, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x24, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x24, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x24, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			}
			W2BYTEMSK(REG_0988_CKGEN01, 0x24, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x2C, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x2C, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x24, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x1C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x1C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
			//serial top rclk
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
			debug("[%s] ckgen setting for LVDS case timing:FHD@60\n",__func__);
		}
		if (priv->linktype == E_LINK_VB1) {
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0400 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0500 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
				W2BYTEMSK(REG_0978_CKGEN01, 0x1C, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x1C, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x1C, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x1C, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x1C, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x1C, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x1C, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			} else {
				W2BYTEMSK(REG_0978_CKGEN01, 0x24, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
				W2BYTEMSK(REG_0A84_CKGEN01, 0x24, REG_0A84_CKGEN01_REG_CKG_V1_ODCLK_SCSCL);
				W2BYTEMSK(REG_0A8C_CKGEN01, 0x24, REG_0A8C_CKGEN01_REG_CKG_V1_ODCLK_SCIP);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x24, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCTCON);
				W2BYTEMSK(REG_0A90_CKGEN01, 0x24, REG_0A90_CKGEN01_REG_CKG_V1_ODCLK_SCDISP_0A90);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x24, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCMW);
				W2BYTEMSK(REG_0A94_CKGEN01, 0x24, REG_0A94_CKGEN01_REG_CKG_V1_ODCLK_SCPQ_0A94);
				W2BYTEMSK(REG_0ACC_CKGEN01, 0x24, REG_0ACC_CKGEN01_REG_CKG_V1_META_ODCLK_SCTCON);
				W2BYTEMSK(REG_0980_CKGEN01, 0x24, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
			}
			W2BYTEMSK(REG_0988_CKGEN01, 0x24, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
			W2BYTEMSK(REG_0990_CKGEN01, 0x2C, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
			W2BYTEMSK(REG_0998_CKGEN01, 0x2C, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
			W2BYTEMSK(REG_09A0_CKGEN01, 0x24, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
			W2BYTEMSK(REG_09A8_CKGEN01, 0x1C, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
			W2BYTEMSK(REG_09B0_CKGEN01, 0x1C, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
			W2BYTEMSK(REG_09B8_CKGEN01, 0x1C, REG_09B8_CKGEN01_REG_CKG_V1_ODCLK_STG8);
			W2BYTEMSK(REG_0A58_CKGEN01, 0x1C, REG_0A58_CKGEN01_REG_CKG_V1_ODCLK_STG9);
			//serial top wclk
			W2BYTEMSK(REG_0920_CKGEN01, 0xC, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
			//serial top rclk
			W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
			debug("[%s] ckgen setting for VB1 timing:FHD@60\n",__func__);
		}
		break;
	default:
		printf("%s: Unknow ckgen\n", __func__);
		break;
	}

#ifdef CONFIG_HAPS
	//if (priv->pnl_lib_version == BOOT_PNL_VERSION0500)
	{
		// HAPS case, MOD clock from xtal, not 600MHz TOP clock
		W2BYTEMSK(REG_0918_CKGEN01, 0x0, REG_0918_CKGEN01_REG_CKG_MOD_D_ODCLK);
		W2BYTEMSK(REG_0978_CKGEN01, 0x4, REG_0978_CKGEN01_REG_CKG_V1_ODCLK);
		W2BYTEMSK(REG_0980_CKGEN01, 0x4, REG_0980_CKGEN01_REG_CKG_V1_ODCLK_STG1);
		W2BYTEMSK(REG_0988_CKGEN01, 0xC, REG_0988_CKGEN01_REG_CKG_V1_ODCLK_STG2);
		W2BYTEMSK(REG_0990_CKGEN01, 0x14, REG_0990_CKGEN01_REG_CKG_V1_ODCLK_STG3);
		W2BYTEMSK(REG_0998_CKGEN01, 0x14, REG_0998_CKGEN01_REG_CKG_V1_ODCLK_STG4);
		W2BYTEMSK(REG_09A0_CKGEN01, 0xC, REG_09A0_CKGEN01_REG_CKG_V1_ODCLK_STG5);
		W2BYTEMSK(REG_09A8_CKGEN01, 0x4, REG_09A8_CKGEN01_REG_CKG_V1_ODCLK_STG6);
		W2BYTEMSK(REG_09B0_CKGEN01, 0x4, REG_09B0_CKGEN01_REG_CKG_V1_ODCLK_STG7);
		//serial top wclk
		W2BYTEMSK(REG_0920_CKGEN01, 0x0, REG_0920_CKGEN01_REG_CKG_MOD_V1_SR_WCLK);
		//serial top rclk
		W2BYTEMSK(REG_0970_CKGEN01, 0x0, REG_0970_CKGEN01_REG_CKG_V1_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A28_CKGEN01, 0x0, REG_0A28_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A30_CKGEN01, 0x0, REG_0A30_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A20_CKGEN01, 0x0, REG_0A20_CKGEN01_REG_CKG_V1_MOD_SR_RCLK_FINIAL);
	}
#endif //#ifdef CONFIG_HAPS

	_ckg_sw_en_video(dev);
	// Tcon init
	if (priv->tcon_info.bUsingTCON)
		_mtk_pnl_tcon_ckg_setting(dev);

	debug("[%s] ckgen setting done\n",__func__);
	return 0;
}

int mtk_disp_odclk_init(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0400 || priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
		W2BYTEMSK(REG_0A84_CKGEN01_V004, 0x03, REG_0A84_CKGEN01_V004_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84);
		W2BYTEMSK(REG_0A8C_CKGEN01_V004, 0x01, REG_0A8C_CKGEN01_V004_REG_CKG_S_V1_ODCLK_SCIP_TMP_0A8C);
		W2BYTEMSK(REG_0A94_CKGEN01_V004, 0x01, REG_0A94_CKGEN01_V004_REG_CKG_S_V1_ODCLK_SCBE_TMP_0A94);
		W2BYTEMSK(REG_0A98_CKGEN01_V004, 0x03, REG_0A98_CKGEN01_V004_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98);
		W2BYTEMSK(REG_0A9C_CKGEN01_V004, 0x03, REG_0A9C_CKGEN01_V004_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C);
		W2BYTEMSK(REG_0A9C_CKGEN01_V004, 0x01, REG_0A9C_CKGEN01_V004_REG_CKG_S_1P_ODCLK_SCIP_TMP_0A9C);
		W2BYTEMSK(REG_1144_CKGEN01_V004, 0x03, REG_1144_CKGEN01_V004_REG_CKG_S_F2P_ODCLK_SCTC_TMP_1144);
		W2BYTEMSK(REG_1148_CKGEN01_V004, 0x03, REG_1148_CKGEN01_V004_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148);
		W2BYTEMSK(REG_114C_CKGEN01_V004, 0x01, REG_114C_CKGEN01_V004_REG_CKG_S_1P_ODCLK_SCTC_TMP_114C);
		W2BYTEMSK(REG_1150_CKGEN01_V004, 0x03, REG_1150_CKGEN01_V004_REG_CKG_S_VAC_ODCLK_SCTC_TMP_1150);
		W2BYTEMSK(REG_1154_CKGEN01_V004, 0x03, REG_1154_CKGEN01_V004_REG_CKG_S_PAFRC_ODCLK_SCTC_TMP_1154);
		W2BYTEMSK(REG_1158_CKGEN01_V004, 0x03, REG_1158_CKGEN01_V004_REG_CKG_S_LINEOD_ODCLK_SCTC_TMP_1158);
	} else if (priv->pnl_lib_version == BOOT_PNL_VERSION0500) {
#ifdef CONFIG_HAPS	// Scaler Clock fix 4K2K60 case
		W2BYTEMSK(REG_0A84_CKGEN01_V005, 0x03, REG_0A84_CKGEN01_V005_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84); // h2A1[13:10], [3:1]=odclk_2p_sel, [0]=1
		W2BYTEMSK(REG_0A98_CKGEN01_V005, 0x03, REG_0A98_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98); // h2A6[13:10], [3:1]=odclk_2p_sel, [0]=1
		W2BYTEMSK(REG_1148_CKGEN01_V005, 0x03, REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148); // h452[5:2], [3:1]=odclk_2p_sel, [0]=1
		W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_0204); // h81[7:4], [3:1]=odclk_2p_sel, [0]=1
		W2BYTEMSK(REG_0B04_CKGEN01_V005, 0x01, REG_0B04_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_SCBE_TMP_0B04); // h2C1[5:2], [3:1]=odclk_osd_sel, [0]=1
		W2BYTEMSK(REG_0A9C_CKGEN01_V005, 0x01, REG_0A9C_CKGEN01_V005_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C); // h2A7[5:2], [3:1]=odclk_osd_sel, [0]=1
		W2BYTEMSK(REG_0204_CKGEN01_V005, 0x01, REG_0204_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_0204); // h81[15:12], [3:1]=odclk_osd_sel, [0]=1
#else   // pnl_lib_version = 5
        // follow Scaler_Clock_APN,
        // odclk_2p_sel[3:1] should depends on use case
        // odclk_osd_sel[3:1] fix 1, for graphic 4K2K30 case.
		switch(priv->out_timing){
		case E_4K2K_120HZ:
		case E_4K2K_144HZ:
		case E_4K1K_144HZ:
		default:
			W2BYTEMSK(REG_0A84_CKGEN01_V005, 0x01, REG_0A84_CKGEN01_V005_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84);
			// h2A1[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0A98_CKGEN01_V005, 0x01, REG_0A98_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98);
			// h2A6[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_1148_CKGEN01_V005, 0x01, REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148);
			// h452[5:2], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x01, REG_0204_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_0204);
			// h81[7:4], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0B04_CKGEN01_V005, 0x03, REG_0B04_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_SCBE_TMP_0B04);
			// h2C1[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0A9C_CKGEN01_V005, 0x03, REG_0A9C_CKGEN01_V005_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C);
			// h2A7[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_0204);
			// h81[15:12], [3:1]=odclk_osd_sel, [0]=1
		break;
		case E_4K2K_60HZ:
		case E_4K1K_120HZ:
			W2BYTEMSK(REG_0A84_CKGEN01_V005, 0x03, REG_0A84_CKGEN01_V005_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84);
			// h2A1[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0A98_CKGEN01_V005, 0x03, REG_0A98_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98);
			// h2A6[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_1148_CKGEN01_V005, 0x03, REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148);
			// h452[5:2], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_0204);
			// h81[7:4], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0B04_CKGEN01_V005, 0x03, REG_0B04_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_SCBE_TMP_0B04);
			// h2C1[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0A9C_CKGEN01_V005, 0x03, REG_0A9C_CKGEN01_V005_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C);
			// h2A7[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_0204);
			// h81[15:12], [3:1]=odclk_osd_sel, [0]=1
		break;
		case E_FHD_60HZ:
			W2BYTEMSK(REG_0A84_CKGEN01_V005, 0x07, REG_0A84_CKGEN01_V005_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84);
			// h2A1[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0A98_CKGEN01_V005, 0x07, REG_0A98_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98);
			// h2A6[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_1148_CKGEN01_V005, 0x07, REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148);
			// h452[5:2], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x07, REG_0204_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_0204);
			// h81[7:4], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0B04_CKGEN01_V005, 0x03, REG_0B04_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_SCBE_TMP_0B04);
			// h2C1[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0A9C_CKGEN01_V005, 0x03, REG_0A9C_CKGEN01_V005_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C);
			// h2A7[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_0204);
			// h81[15:12], [3:1]=odclk_osd_sel, [0]=1
		break;
		case E_FHD_120HZ:
			W2BYTEMSK(REG_0A84_CKGEN01_V005, 0x05, REG_0A84_CKGEN01_V005_REG_CKG_S_V2_ODCLK_SCBE_TMP_0A84);
			// h2A1[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0A98_CKGEN01_V005, 0x05, REG_0A98_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCIP_TMP_0A98);
			// h2A6[13:10], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_1148_CKGEN01_V005, 0x05, REG_1148_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_SCTC_TMP_1148);
			// h452[5:2], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x05, REG_0204_CKGEN01_V005_REG_CKG_S_B2P_ODCLK_0204);
			// h81[7:4], [3:1]=odclk_2p_sel, [0]=1
			W2BYTEMSK(REG_0B04_CKGEN01_V005, 0x03, REG_0B04_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_SCBE_TMP_0B04);
			// h2C1[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0A9C_CKGEN01_V005, 0x03, REG_0A9C_CKGEN01_V005_REG_CKG_S_O_ODCLK_SCTC_TMP_0A9C);
			// h2A7[5:2], [3:1]=odclk_osd_sel, [0]=1
			W2BYTEMSK(REG_0204_CKGEN01_V005, 0x03, REG_0204_CKGEN01_V005_REG_CKG_S_OSD_ODCLK_0204);
			// h81[15:12], [3:1]=odclk_osd_sel, [0]=1
		break;
		}
#endif
	} else {
		W2BYTEMSK(REG_0A88_CKGEN01, 0x0, REG_0A88_CKGEN01_REG_V1_V2_O_ODCLK_FROM_MOD);
		W2BYTEMSK(REG_0AE0_CKGEN01, 0x0, REG_0AE0_CKGEN01_REG_CKG_XC_OD_SCIP);
		W2BYTEMSK(REG_0A7C_CKGEN01, 0x0, REG_0A7C_CKGEN01_REG_CKG_XC_OD_DELTA_SCIP);
		W2BYTEMSK(REG_0A80_CKGEN01, 0x0, REG_0A80_CKGEN01_REG_CKG_XC_OD_SCSCL);
		W2BYTEMSK(REG_0A80_CKGEN01, 0x0, REG_0A80_CKGEN01_REG_CKG_XC_OD_DELTA_SCSCL);
		W2BYTEMSK(REG_0B78_CKGEN01, 0x0, REG_0B78_CKGEN01_REG_CKG_XC_OD_SCMW);

		W2BYTEMSK(REG_0AB4_CKGEN01, 0x0, REG_0AB4_CKGEN01_REG_CKG_XC_OD_META_SCTCON);
		W2BYTEMSK(REG_0ABC_CKGEN01, 0x0, REG_0ABC_CKGEN01_REG_CKG_XC_OD_DELTA_META_SCTCON);
		W2BYTEMSK(REG_0AC4_CKGEN01, 0x0, REG_0AC4_CKGEN01_REG_CKG_XC_OD_OSD_META_SCTCON);
		W2BYTEMSK(REG_0C10_CKGEN01, 0x0, REG_0C10_CKGEN01_REG_CKG_XC_OD_SCTCON);
		W2BYTEMSK(REG_0E4C_CKGEN01, 0x0, REG_0E4C_CKGEN01_REG_CKG_XC_OD_DIV2_SCTCON);
		W2BYTEMSK(REG_0E50_CKGEN01, 0x0, REG_0E50_CKGEN01_REG_CKG_XC_OD_DIV4_SCTCON);
		W2BYTEMSK(REG_0E48_CKGEN01, 0x0, REG_0E48_CKGEN01_REG_CKG_XC_OD_DELTA_SCTCON);
		W2BYTEMSK(REG_0E68_CKGEN01, 0x0, REG_0E68_CKGEN01_REG_CKG_XC_OD_OSD_SCTCON);

		// XC_FD_CLK init
		W2BYTEMSK(REG_0AC8_CKGEN01, 0x4, REG_0AC8_CKGEN01_REG_CKG_XC_FD);
	}
	return 0;
}

int mtk_ext_video_clk_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	debug("[%s]extdev out_timing = %d\n", __func__, priv->out_timing);

	if (priv->out_timing == E_4K2K_60HZ ||
	    priv->out_timing == E_4K1K_120HZ) {
		W2BYTEMSK(REG_09C8_CKGEN01, 0x4, REG_09C8_CKGEN01_REG_CKG_V2_ODCLK);
		W2BYTEMSK(REG_0A84_CKGEN01, 0x4, REG_0A84_CKGEN01_REG_CKG_V2_ODCLK_SCSCL);
		W2BYTEMSK(REG_0A8C_CKGEN01, 0x4, REG_0A8C_CKGEN01_REG_CKG_V2_ODCLK_SCIP);
		W2BYTEMSK(REG_0A98_CKGEN01, 0x4, REG_0A98_CKGEN01_REG_CKG_V2_ODCLK_SCTCON);
		W2BYTEMSK(REG_09D0_CKGEN01, 0xC, REG_09D0_CKGEN01_REG_CKG_V2_ODCLK_STG1);
		W2BYTEMSK(REG_09D8_CKGEN01, 0x14, REG_09D8_CKGEN01_REG_CKG_V2_ODCLK_STG2);
		W2BYTEMSK(REG_09E0_CKGEN01, 0x1C, REG_09E0_CKGEN01_REG_CKG_V2_ODCLK_STG3);
		W2BYTEMSK(REG_09E8_CKGEN01, 0x1C, REG_09E8_CKGEN01_REG_CKG_V2_ODCLK_STG4);
		W2BYTEMSK(REG_09F0_CKGEN01, 0x1C, REG_09F0_CKGEN01_REG_CKG_V2_ODCLK_STG5);
		W2BYTEMSK(REG_09F8_CKGEN01, 0x1C, REG_09F8_CKGEN01_REG_CKG_V2_ODCLK_STG6);
		W2BYTEMSK(REG_0A00_CKGEN01, 0x1C, REG_0A00_CKGEN01_REG_CKG_V2_ODCLK_STG7);
		W2BYTEMSK(REG_0924_CKGEN01, 0xC, REG_0924_CKGEN01_REG_CKG_MOD_V2_SR_WCLK);
		W2BYTEMSK(REG_09C0_CKGEN01, 0x0, REG_09C0_CKGEN01_REG_CKG_V2_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A40_CKGEN01, 0x0, REG_0A40_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A48_CKGEN01, 0x0, REG_0A48_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A38_CKGEN01, 0x0, REG_0A38_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_FINIAL);
	} else if ((priv->out_timing == E_4K2K_120HZ) ||
		(priv->out_timing == E_4K1K_240HZ)) {
		W2BYTEMSK(REG_09C8_CKGEN01, 0xC, REG_09C8_CKGEN01_REG_CKG_V2_ODCLK);
		W2BYTEMSK(REG_0A84_CKGEN01, 0xC, REG_0A84_CKGEN01_REG_CKG_V2_ODCLK_SCSCL);
		W2BYTEMSK(REG_0A8C_CKGEN01, 0xC, REG_0A8C_CKGEN01_REG_CKG_V2_ODCLK_SCIP);
		W2BYTEMSK(REG_0A98_CKGEN01, 0xC, REG_0A98_CKGEN01_REG_CKG_V2_ODCLK_SCTCON);
		W2BYTEMSK(REG_09D0_CKGEN01, 0xC, REG_09D0_CKGEN01_REG_CKG_V2_ODCLK_STG1);
		W2BYTEMSK(REG_09D8_CKGEN01, 0xC, REG_09D8_CKGEN01_REG_CKG_V2_ODCLK_STG2);
		W2BYTEMSK(REG_09E0_CKGEN01, 0x14, REG_09E0_CKGEN01_REG_CKG_V2_ODCLK_STG3);
		W2BYTEMSK(REG_09E8_CKGEN01, 0x14, REG_09E8_CKGEN01_REG_CKG_V2_ODCLK_STG4);
		W2BYTEMSK(REG_09F0_CKGEN01, 0x14, REG_09F0_CKGEN01_REG_CKG_V2_ODCLK_STG5);
		W2BYTEMSK(REG_09F8_CKGEN01, 0x14, REG_09F8_CKGEN01_REG_CKG_V2_ODCLK_STG6);
		W2BYTEMSK(REG_0A00_CKGEN01, 0x14, REG_0A00_CKGEN01_REG_CKG_V2_ODCLK_STG7);
		W2BYTEMSK(REG_0924_CKGEN01, 0xC, REG_0924_CKGEN01_REG_CKG_MOD_V2_SR_WCLK);
		W2BYTEMSK(REG_09C0_CKGEN01, 0x0, REG_09C0_CKGEN01_REG_CKG_V2_MOD_SR_RCLK);
		W2BYTEMSK(REG_0A40_CKGEN01, 0x0, REG_0A40_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE0);
		W2BYTEMSK(REG_0A48_CKGEN01, 0x0, REG_0A48_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_PRE1);
		W2BYTEMSK(REG_0A38_CKGEN01, 0x0, REG_0A38_CKGEN01_REG_CKG_V2_MOD_SR_RCLK_FINIAL);
	} else {
		printf("[%s]output timing not supported\n",__func__);
	}

	_ckg_sw_en_ext_video(dev); //sw_en

	return 0;
}

int mtk_gfx_clk_setting(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		printf("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	switch (priv->out_timing) {
	case E_4K2K_60HZ:
	case E_4K1K_120HZ:
		if (priv->vbo_byte == E_VBO_4BYTE_MODE) {
			W2BYTEMSK(REG_0930_CKGEN01, 0x4, REG_0930_CKGEN01_REG_CKG_O_ODCLK);
			W2BYTEMSK(REG_0A9C_CKGEN01, 0x4, REG_0A9C_CKGEN01_REG_CKG_O_ODCLK_SCTCON);
			W2BYTEMSK(REG_0938_CKGEN01, 0xC, REG_0938_CKGEN01_REG_CKG_O_ODCLK_STG1);
			W2BYTEMSK(REG_0940_CKGEN01, 0x14, REG_0940_CKGEN01_REG_CKG_O_ODCLK_STG2);
			W2BYTEMSK(REG_0948_CKGEN01, 0x1C, REG_0948_CKGEN01_REG_CKG_O_ODCLK_STG3);
			W2BYTEMSK(REG_0950_CKGEN01, 0x1C, REG_0950_CKGEN01_REG_CKG_O_ODCLK_STG4);
			W2BYTEMSK(REG_0958_CKGEN01, 0x1C, REG_0958_CKGEN01_REG_CKG_O_ODCLK_STG5);
			W2BYTEMSK(REG_0960_CKGEN01, 0x1C, REG_0960_CKGEN01_REG_CKG_O_ODCLK_STG6);
			W2BYTEMSK(REG_091C_CKGEN01, 0xC, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
			W2BYTEMSK(REG_0928_CKGEN01, 0x0, REG_0928_CKGEN01_REG_CKG_O_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A10_CKGEN01, 0x0, REG_0A10_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A18_CKGEN01, 0x0, REG_0A18_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A08_CKGEN01, 0x0, REG_0A08_CKGEN01_REG_CKG_O_MOD_SR_RCLK_FINIAL);
		} else if (priv->vbo_byte == E_VBO_5BYTE_MODE) {
			W2BYTEMSK(REG_0930_CKGEN01, 0x4, REG_0930_CKGEN01_REG_CKG_O_ODCLK);
			W2BYTEMSK(REG_0A9C_CKGEN01, 0x4, REG_0A9C_CKGEN01_REG_CKG_O_ODCLK_SCTCON);
			W2BYTEMSK(REG_0938_CKGEN01, 0xC, REG_0938_CKGEN01_REG_CKG_O_ODCLK_STG1);
			W2BYTEMSK(REG_0940_CKGEN01, 0x14, REG_0940_CKGEN01_REG_CKG_O_ODCLK_STG2);
			W2BYTEMSK(REG_0948_CKGEN01, 0x1C, REG_0948_CKGEN01_REG_CKG_O_ODCLK_STG3);
			W2BYTEMSK(REG_0950_CKGEN01, 0x1C, REG_0950_CKGEN01_REG_CKG_O_ODCLK_STG4);
			W2BYTEMSK(REG_0958_CKGEN01, 0x1C, REG_0958_CKGEN01_REG_CKG_O_ODCLK_STG5);
			W2BYTEMSK(REG_0960_CKGEN01, 0x1C, REG_0960_CKGEN01_REG_CKG_O_ODCLK_STG6);
			W2BYTEMSK(REG_091C_CKGEN01, 0xC, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
			W2BYTEMSK(REG_0928_CKGEN01, 0x4, REG_0928_CKGEN01_REG_CKG_O_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A10_CKGEN01, 0x4, REG_0A10_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A18_CKGEN01, 0x4, REG_0A18_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A08_CKGEN01, 0x4, REG_0A08_CKGEN01_REG_CKG_O_MOD_SR_RCLK_FINIAL);
		} else {
			debug("[%s]Output format not supported\n", __func__);
		}
	break;

	case E_4K2K_30HZ:
		if (priv->vbo_byte == E_VBO_4BYTE_MODE) {
			W2BYTEMSK(REG_0930_CKGEN01, 0xC, REG_0930_CKGEN01_REG_CKG_O_ODCLK);
			W2BYTEMSK(REG_0A9C_CKGEN01, 0xC, REG_0A9C_CKGEN01_REG_CKG_O_ODCLK_SCTCON);
			W2BYTEMSK(REG_0938_CKGEN01, 0x14, REG_0938_CKGEN01_REG_CKG_O_ODCLK_STG1);
			W2BYTEMSK(REG_0940_CKGEN01, 0x1C, REG_0940_CKGEN01_REG_CKG_O_ODCLK_STG2);
			W2BYTEMSK(REG_0948_CKGEN01, 0x24, REG_0948_CKGEN01_REG_CKG_O_ODCLK_STG3);
			W2BYTEMSK(REG_0950_CKGEN01, 0x24, REG_0950_CKGEN01_REG_CKG_O_ODCLK_STG4);
			W2BYTEMSK(REG_0958_CKGEN01, 0x1C, REG_0958_CKGEN01_REG_CKG_O_ODCLK_STG5);
			W2BYTEMSK(REG_0960_CKGEN01, 0x1C, REG_0960_CKGEN01_REG_CKG_O_ODCLK_STG6);
			W2BYTEMSK(REG_091C_CKGEN01, 0xC, REG_091C_CKGEN01_REG_CKG_MOD_O_SR_WCLK);
			W2BYTEMSK(REG_0928_CKGEN01, 0x0, REG_0928_CKGEN01_REG_CKG_O_MOD_SR_RCLK);
			W2BYTEMSK(REG_0A10_CKGEN01, 0x0, REG_0A10_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE0);
			W2BYTEMSK(REG_0A18_CKGEN01, 0x0, REG_0A18_CKGEN01_REG_CKG_O_MOD_SR_RCLK_PRE1);
			W2BYTEMSK(REG_0A08_CKGEN01, 0x0, REG_0A08_CKGEN01_REG_CKG_O_MOD_SR_RCLK_FINIAL);
		} else {
			debug("[%s]Output format not supported\n", __func__);
		}
		break;
	default:
		printf("[%s] output timing not supported\n", __func__);
		break;

	}
	_ckg_sw_en_gfx(dev); //sw_en

	return 0;
}
