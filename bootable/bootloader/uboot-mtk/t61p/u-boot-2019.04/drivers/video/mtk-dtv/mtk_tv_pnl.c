// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */
//#define DEBUG //for open this file debug("xxx") log
#include "mtk_tv_pnl.h"
#include "mtk_pnl_dts_st.h"
#include "mtk_pnl_out_if.h"
#include "mtk_pnl_out_if_v006.h"

#include "mtk_tcon_out_if.h"
#include "mtk_tcon_common.h"
#include "mtk_pnl_clk_ctrl.h"
#include "mtk_pnl_clk_ctrl_v006.h"
#include <common.h>
#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <asm/gpio.h>
#include <power/regulator.h>
#include <display.h>
#include <time.h>
#include <debug_impl.h>
#include <iniutility.h>

#define OFFSET_32 32
#define PANEL_DLG_PARTITION_PATH		"persist"
#define PANEL_DLG_INI_PATH		"panel_mode_cfg.ini"
static bool is_dlg_mode;

static int mtk_panel_enable_output(struct udevice *dev, int panel_bpp,
		     const struct display_timing *timing)
{
	//enable output config
	//bool enable = true;
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!timing || !dev) {
		UBOOT_ERROR("[%s] param FAIL\n", __func__);
		return -EPERM;
	}

        UBOOT_DEBUG("[%s] timing->flags = 0x%x\n", __func__, timing->flags);

	if (priv->pnl_lib_version == BOOT_PNL_VERSION0600) {
		if (timing->flags & DISPLAY_FLAGS_DE_HIGH)
			mtk_pnl_out_en_v006(dev, true);

		if (timing->flags & DISPLAY_FLAGS_DE_LOW)
			mtk_pnl_out_en_v006(dev, false);

		if (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)
			mtk_pnl_mute_en_v006(dev, true);

		if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
			mtk_pnl_mute_en_v006(dev, false);
	} else {
		if (timing->flags & DISPLAY_FLAGS_DE_HIGH) {
			mtk_pnl_out_en(dev, true);
			if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
				priv->pnl_lib_version == BOOT_PNL_VERSION0300)
				analog_set_drv_mode(dev);
		}

		if (timing->flags & DISPLAY_FLAGS_DE_LOW)
			mtk_pnl_out_en(dev, false);

		if (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)
			mtk_pnl_mute_en(dev, true);

		if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
			mtk_pnl_mute_en(dev, false);
	}
	mtk_testpattern_settings(dev);

	return 0;
}

static int mtk_extv_enable_output(struct udevice *dev, int panel_bpp,
		     const struct display_timing *timing)
{
	struct mtk_panel_priv *priv = NULL;

	if (!timing || !dev) {
		UBOOT_ERROR("[%s] param FAIL\n", __func__);
		return -EPERM;
	}

	priv = dev_get_priv(dev);
	UBOOT_DEBUG("[%s] timing->flags = 0x%x\n", __func__, timing->flags);

	if (timing->flags & DISPLAY_FLAGS_DE_HIGH) {
		mtk_extv_out_en(dev, true);
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0300)
			analog_ext_video_set_drv_mode(dev);
	}

	if (timing->flags & DISPLAY_FLAGS_DE_LOW)
		mtk_extv_out_en(dev, false);

	if (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		mtk_extv_mute_en(dev, true);

	if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
		mtk_extv_mute_en(dev, false);

	return 0;

}

static int mtk_gfx_enable_output(struct udevice *dev, int panel_bpp,
		     const struct display_timing *timing)
{
	struct mtk_panel_priv *priv = NULL;

	if (!timing || !dev) {
		UBOOT_ERROR("[%s] param FAIL\n", __func__);
		return -EPERM;
	}

	priv = dev_get_priv(dev);
	UBOOT_DEBUG("[%s] timing->flags = 0x%x\n", __func__, timing->flags);

	if (timing->flags & DISPLAY_FLAGS_DE_HIGH) {
		mtk_gfx_out_en(dev, true);
		if (priv->pnl_lib_version == BOOT_PNL_VERSION0100 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0200 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0203 ||
			priv->pnl_lib_version == BOOT_PNL_VERSION0300)
			analog_gfx_set_drv_mode(dev);
	}

	if (timing->flags & DISPLAY_FLAGS_DE_LOW)
		mtk_gfx_out_en(dev, false);

	if (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		mtk_gfx_mute_en(dev, true);

	if (timing->flags & DISPLAY_FLAGS_VSYNC_LOW)
		mtk_gfx_mute_en(dev, false);

	return 0;

}

static int mtk_panel_get_property(struct udevice *dev, int type, int *value)
{
    struct mtk_panel_priv *priv = dev_get_priv(dev);

    if (priv == NULL) {
        UBOOT_ERROR("[%s] get device private fail\n", __func__);
        return -ENXIO;
    }

    if (!mtk_pnl_get_property(dev, type, value)) {
        UBOOT_DEBUG("[%s] get property type=%d return false\n", __func__, type);
        return -EINVAL;
    }

    return 0;
}

static int mtk_panel_set_property(struct udevice *dev, int type, uint8_t *value)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv) {
		UBOOT_ERROR("[%s] get device private fail\n", __func__);
		return -ENXIO;
	}

	if (!mtk_pnl_set_property(dev, type, value)) {
		UBOOT_DEBUG("[%s] set property type=%d return false\n", __func__, type);
		return -EINVAL;
	}

	return 0;
}

static int _get_lanes_num_by_linktye(en_link_if linktype)
{
    uint32_t lanes = 0;
    if ((linktype < E_LINK_MINILVDS_1BLK_3PAIR_6BIT) || (linktype > E_LINK_USIT_10BIT_16PAIR))
        return 0;

    switch (linktype ) {
        case E_LINK_EPI28_8BIT_2PAIR_2KCML:
        case E_LINK_EPI28_8BIT_2PAIR_2KLVDS:
        {
            lanes = 2;
        }
        break;
        case E_LINK_MINILVDS_1BLK_3PAIR_6BIT:
        case E_LINK_MINILVDS_2BLK_3PAIR_6BIT:
        case E_LINK_MINILVDS_1BLK_3PAIR_8BIT:
        case E_LINK_MINILVDS_2BLK_3PAIR_8BIT:
        {
            lanes = 3;
        }
        break;
        case E_LINK_EPI28_8BIT_4PAIR_2KCML:
        case E_LINK_EPI28_8BIT_4PAIR_2KLVDS:
        {
            lanes = 4;
        }
        break;
        case E_LINK_MINILVDS_1BLK_6PAIR_6BIT:
        case E_LINK_MINILVDS_2BLK_6PAIR_6BIT:
        case E_LINK_MINILVDS_1BLK_6PAIR_8BIT:
        case E_LINK_MINILVDS_2BLK_6PAIR_8BIT:
        case E_LINK_EPI28_8BIT_6PAIR_2KCML:
        case E_LINK_EPI28_8BIT_6PAIR_4KCML:
        case E_LINK_EPI28_8BIT_6PAIR_2KLVDS:
        case E_LINK_EPI28_8BIT_6PAIR_4KLVDS:
        case E_LINK_CMPI27_8BIT_6PAIR:
        case E_LINK_USIT_8BIT_6PAIR:
        case E_LINK_USIT_10BIT_6PAIR:
        case E_LINK_ISP_8BIT_6PAIR:
        case E_LINK_CHPI_8BIT_6PAIR:
        case E_LINK_CHPI_10BIT_6PAIR:
        {
            lanes = 6;
        }
        break;
        case E_LINK_EPI28_8BIT_8PAIR_2KCML:
        case E_LINK_EPI28_8BIT_8PAIR_4KCML:
        case E_LINK_EPI28_8BIT_8PAIR_2KLVDS:
        case E_LINK_EPI28_8BIT_8PAIR_4KLVDS:
        case E_LINK_CMPI27_8BIT_8PAIR:
        case E_LINK_CMPI27_10BIT_8PAIR:
        case E_LINK_ISP_8BIT_8PAIR:
        case E_LINK_ISP_10BIT_8PAIR:
        case E_LINK_CHPI_8BIT_8PAIR:
        case E_LINK_CHPI_10BIT_8PAIR:
        case E_LINK_USIT_8BIT_8PAIR:
        {
            lanes = 8;
        }
        break;
        case E_LINK_EPI28_8BIT_12PAIR_4KCML:
        case E_LINK_EPI24_10BIT_12PAIR_4KCML:
        case E_LINK_EPI28_8BIT_12PAIR_4KLVDS:
        case E_LINK_EPI24_10BIT_12PAIR_4KLVDS:
        case E_LINK_CMPI27_8BIT_12PAIR:
        case E_LINK_CMPI27_10BIT_12PAIR:
        case E_LINK_USIT_8BIT_12PAIR:
        case E_LINK_USIT_10BIT_12PAIR:
        case E_LINK_ISP_8BIT_12PAIR:
        case E_LINK_ISP_8BIT_6X2PAIR:
        case E_LINK_ISP_10BIT_6X2PAIR:
        case E_LINK_ISP_10BIT_12PAIR:
        case E_LINK_CHPI_8BIT_12PAIR:
        case E_LINK_CHPI_8BIT_6X2PAIR:
        case E_LINK_CHPI_10BIT_12PAIR:
        {
            lanes = 12;
        }
        break;
        case E_LINK_EPI28_8BIT_16PAIR_4KCML:
        case E_LINK_EPI24_10BIT_16PAIR_4KCML:
        case E_LINK_EPI28_8BIT_16PAIR_4KLVDS:
        case E_LINK_EPI24_10BIT_16PAIR_4KLVDS:
        case E_LINK_USIT_8BIT_16PAIR:
        case E_LINK_USIT_10BIT_16PAIR:
        {
            lanes = 16;
        }
        break;
        default:
        {
            lanes = 8;
        }
        break;
    }
    return lanes;
}

static int _parse_dts_info(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	ofnode info_node;
	ofnode mainvideo_info_node;
	ofnode hwinfo_node;
	ofnode tconlessinfo_node;
	uint16_t vfreq = 0;
	uint32_t u32Dclk = 0;
	uint32_t u32Typ_clk_h = 0, u32Typ_clk_l = 0;
	struct udevice *mainvideodev;
	uint32_t tcon_disable = 0;
	int ret = 0;
	int ret_dlg = 0;
	uint32_t tmp = 0;
	struct panel_dlg_info panel_dlg_info;

	memset(&panel_dlg_info, 0, sizeof(panel_dlg_info));
	ret = uclass_get_device_by_name(UCLASS_DISPLAY, "video_out", &mainvideodev);
	mainvideo_info_node = dev_read_subnode(mainvideodev, PKG_LN_ORDER);
	if (!ofnode_valid(mainvideo_info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", PKG_LN_ORDER);
		return -ENOANO;
	}

	if (!priv) {
		UBOOT_ERROR("Get device priv FAIL\n");
		return -EINVAL;
	}
	ofnode_read_u32(mainvideo_info_node, SUP_LANES, &priv->lane_info.sup_lanes);
	if (priv->lane_info.sup_lanes > MAX_LANES) {
		UBOOT_ERROR("Support Lanes(%d) is larger than MAX_LANES(%d)\n"
			,priv->lane_info.sup_lanes, MAX_LANES);
		return -ENODATA;
	}
	ofnode_read_u32_array(mainvideo_info_node, LAYOUT_ORDER, priv->lane_info.def_layout, priv->lane_info.sup_lanes);
	// panle version
	hwinfo_node = dev_read_subnode(mainvideodev, HW_INFO);
	if (!ofnode_valid(hwinfo_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", HW_INFO);
		return -ENOANO;
	}
	ofnode_read_u32(hwinfo_node, PNL_LIB_VER, &priv->pnl_lib_version);

	//lane duplicate
	info_node = dev_ofnode(dev);
	ofnode_read_u32(info_node, LANE_DUPLICATE, &tmp);
	if (tmp == 1)
		priv->lane_duplicate_en = true;
	else
		priv->lane_duplicate_en = false;

	priv->out_format = E_OUTPUT_RGB;


	ret_dlg = get_panel_dlg_info(PANEL_DLG_PARTITION_PATH, PANEL_DLG_INI_PATH, &panel_dlg_info);

	if((ret_dlg >= 0) && (panel_dlg_info.panel_dlg == 1)) {
		UBOOT_INFO("Panel DLG ON\n");
		info_node = dev_read_subnode(dev, DLG_INFO);

		if (!ofnode_valid(info_node)) {
			UBOOT_ERROR("Read subnode (%s) FAIL\n", DLG_INFO);
			return -ENOANO;
		}
		is_dlg_mode = true;
	} else {
	info_node = dev_read_subnode(dev, INFO);

	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", INFO);
		return -ENOANO;
		}
	}

	ofnode_read_u32(info_node, LINK_TYPE, &priv->linktype);
	ofnode_read_u32(info_node, VBO_BYTE, &priv->vbo_byte);
	ofnode_read_u32(info_node, DIV_SEC, &priv->div_sec);

	ofnode_read_u32(info_node, ON_T1, &priv->ontiming_1);
	ofnode_read_u32(info_node, ON_T2, &priv->ontiming_2);
	ofnode_read_u32(info_node, OFF_T1, &priv->offtiming_1);
	ofnode_read_u32(info_node, OFF_T2, &priv->offtiming_2);
	ofnode_read_u32(info_node, HS_ST, &priv->hsync_st);
	ofnode_read_u32(info_node, HS_WIDTH, &priv->hsync_w);
	ofnode_read_u32(info_node, HS_POL, &priv->hsync_pol);
	ofnode_read_u32(info_node, VS_ST, &priv->vsync_st);
	ofnode_read_u32(info_node, VS_WIDTH, &priv->vsync_w);
	ofnode_read_u32(info_node, VS_POL, &priv->vsync_pol);
	ofnode_read_u32(info_node, DE_H_ST, &priv->de_hstart);
	ofnode_read_u32(info_node, DE_V_ST, &priv->de_vstart);
	ofnode_read_u32(info_node, DE_WIDTH, &priv->de_width);
	ofnode_read_u32(info_node, DE_HEIGHT, &priv->de_height);
	ofnode_read_u32(info_node, MAX_HTT, &priv->max_htt);
	ofnode_read_u32(info_node, TYP_HTT, &priv->typ_htt);
	ofnode_read_u32(info_node, MIN_HTT, &priv->min_htt);
	ofnode_read_u32(info_node, MAX_VTT, &priv->max_vtt);
	ofnode_read_u32(info_node, TYP_VTT, &priv->typ_vtt);
	ofnode_read_u32(info_node, MIN_VTT, &priv->min_vtt);
	ofnode_read_u32(info_node, MAX_VTT_PANEL_PROTECT, &priv->max_vtt_panelprotect);
	ofnode_read_u32(info_node, MIN_VTT_PANEL_PROTECT, &priv->min_vtt_panelprotect);
	ofnode_read_u32(info_node, MAX_CLK, &priv->max_dclk);
	ofnode_read_u32(info_node, TYP_CLK_H, &u32Typ_clk_h);
	ofnode_read_u32(info_node, TYP_CLK_L, &u32Typ_clk_l);
	priv->typ_dclk = ((uint64_t)u32Typ_clk_h << OFFSET_32) |
		u32Typ_clk_l;
	ofnode_read_u32(info_node, MIN_CLK, &priv->min_dclk);
	ofnode_read_u32(info_node, OUT_FORMAT, &priv->out_format);
	ofnode_read_u32(info_node, COUT_FORMAT, &priv->cout_format);
	ofnode_read_u32(info_node, HPC_MODE_TAG, &tmp);
	priv->hpc_mode_en = (bool)tmp;
	ofnode_read_u32(info_node, GAME_DIRECT_FR_GROUP_TAG, &priv->game_direct_fr_group);
	ofnode_read_u32(info_node, DLG_ON_TAG, &tmp);
	priv->dlg_on = (bool)tmp;

    //panel cus setting start
    info_node = dev_read_subnode(dev, CUS_INFO);
    if (!ofnode_valid(info_node)) {
        UBOOT_ERROR("Read subnode (%s) FAIL\n", CUS_INFO);
    } else {
        ofnode_read_u32(info_node, TCON_ENABLE, &tmp);
        priv->tcon_info.bUsingTCON = (bool)tmp;

        if (priv->tcon_info.bUsingTCON && is_tcon_force_disable())
            priv->tcon_info.bUsingTCON = false;

		//Check efuse bit
		if(priv->tcon_info.bUsingTCON){
			tcon_disable = mtk_dump_tcon_efuse(priv->pnl_lib_version);
			if(tcon_disable==true){
				priv->tcon_info.bUsingTCON = false;
			}
		}
        priv->tcon_info.tcon_bin_path = ofnode_read_string(info_node, TCON_BIN_PATH);
        UBOOT_DEBUG("[%d] tcon_en=%u\n", __LINE__, priv->tcon_info.bUsingTCON);
        UBOOT_DEBUG("[%d] tcon_bin_path=%s\n", __LINE__, priv->tcon_info.tcon_bin_path);

        ofnode_read_u32(info_node, PANELGAMMA_ENABLE, &tmp);
        priv->tcon_info.bPanelGammaEn = (bool)tmp;
        priv->tcon_info.panelgamma_bin_path = ofnode_read_string(info_node, PANELGAMMA_BIN_PATH);
        UBOOT_DEBUG("[%d] bPanelGammaEn=%u\n", __LINE__, priv->tcon_info.bPanelGammaEn);
        UBOOT_DEBUG("[%d] panelgamma_bin_path=%s\n\033[m", __LINE__, priv->tcon_info.panelgamma_bin_path);

		ofnode_read_u32(info_node, OVERDIRVE_ENABLE, &tmp);
		priv->tcon_info.bOverDriveEn = (bool)tmp;
		UBOOT_DEBUG("[%d] bOverDriveEn=%u\n\033[m", __LINE__, priv->tcon_info.bOverDriveEn);

		ofnode_read_u32(info_node, AUTO_PANELGAMMA, &priv->tcon_info.u32AutoPga);
		ofnode_read_u32(info_node, P2P_CMD_UPDATE, &priv->tcon_info.u32P2pCmdUpdate);
		ofnode_read_u32(info_node, PROJECTOR_UI_EN, &tmp);
		priv->tcon_info.bIsProjector = tmp & 0x1;
    }

    if(priv->tcon_info.bUsingTCON)
    {
        //parsing tcon-related field
        priv->tcon_info.chPanelIDString = ofnode_read_string(info_node, PANEL_ID_STRING);

        ofnode_read_u32(info_node, PANEL_INDEX, &priv->tcon_info.u32PanelIndex);
        ofnode_read_u32(info_node, INCH_SIZE, &priv->tcon_info.u32InchSize);
        ofnode_read_u32(info_node, CHASSIS_INDEX, &priv->tcon_info.u32ChassisIndex);
        ofnode_read_u32(info_node, VCOM_PATTERN, &priv->tcon_info.u32VcomPattern);
        ofnode_read_u32(info_node, VCOM_TYPE, &priv->tcon_info.u32VcomType);
        ofnode_read_u32(info_node, SPREAD_PERMILLAGE, &priv->tcon_info.u32SpreadPermillage);
        ofnode_read_u32(info_node, SPREAD_FREQ, &priv->tcon_info.u32SpreadFreq);
        ofnode_read_u32(info_node, OCELL_DEMURA_IDX, &priv->tcon_info.u32OcellDemuraIdx);
        ofnode_read_u32(info_node, TCON_INDEX, &priv->tcon_info.u32TconIndex);
        ofnode_read_u32(info_node, CURRENT_MAX, &priv->tcon_info.u32CurrentMax);
        ofnode_read_u32(info_node, BL_CUR_FREQ, &priv->tcon_info.u32BackLightCurrFreq);
        ofnode_read_u32(info_node, BL_PWM_FREQ, &priv->tcon_info.u32BackLightPwmFreq);
        ofnode_read_u32(info_node, BL_WAIT_LOGO, &priv->tcon_info.u32BackLightWaitLogo);
        ofnode_read_u32(info_node, BL_CONTROL_IF, &priv->tcon_info.u32BackLightControlIf);

        tconlessinfo_node = dev_read_subnode(dev, TCONLESS_INFO);
        if (!ofnode_valid(tconlessinfo_node)) {
            UBOOT_ERROR("Read subnode (%s) FAIL\n", TCONLESS_INFO);
        } else {
            priv->tcon_info.higt_frame_rate_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            HIGT_FRAME_RATE_BIN_PATH);
            priv->tcon_info.higt_pixel_clock_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            HIGT_PIXEL_CLOCK_BIN_PATH);
            priv->tcon_info.game_mode_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            GAME_MODE_BIN_PATH);
            priv->tcon_info.higt_frame_rate_gamma_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            HIGT_FRAME_RATE_GAMMA_BIN_PATH);
            priv->tcon_info.higt_pixel_clock_gamma_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            HIGT_PIXEL_CLOCK_GAMMA_BIN_PATH);
            priv->tcon_info.game_mode_gamma_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            GAME_MODE_GAMMA_BIN_PATH);
            priv->tcon_info.power_seq_on_bin_path = ofnode_read_string(
                                                            tconlessinfo_node,
                                                            POWER_SEQ_ON_BIN_PATH);

            UBOOT_DEBUG("\nhigt_frame_rate_bin_path=%s\n"
                        "higt_pixel_clock_bin_path=%s\n"
                        "game_mode_bin_path=%s\n"
                        "power_seq_on_bin_path=%s\n",
                        priv->tcon_info.higt_frame_rate_bin_path,
                        priv->tcon_info.higt_pixel_clock_bin_path,
                        priv->tcon_info.game_mode_bin_path,
                        priv->tcon_info.power_seq_on_bin_path);
        }

        if (is_tcon_sti_flow())
        {
            //1) Load tcon data into memory
            //2) Overwrite existing panel info with panel info stored in the tcon bin.
            priv->tcon_info.bUsingTCON = mtk_tcon_preinit(dev);

            TCON_DEBUG("UsingTCON=%u TconFilePath=%s\nPanelGammaEn=%u PanelgammaFilePath=%s\n" \
                    "OverDriveEn=%u PanelIndex=%u PanelIDString=%s\nInchSize=%u ChassisIndex=%u\n" \
                    "VcomPattern=%u VcomType=%u\nSpreadPermillage=%u SpreadFreq=%u\n"\
                    "OcellDemuraIdx=%u\nTconIndex=%u CurrentMax=%u\n" \
                    "BLCurrFreq=%u BLPwmFreq=%u\nBLWaitLogo=%u BLControlIf=%u\n", \
                    priv->tcon_info.bUsingTCON, priv->tcon_info.tcon_bin_path, \
                    priv->tcon_info.bPanelGammaEn, priv->tcon_info.panelgamma_bin_path, \
                    priv->tcon_info.bOverDriveEn, priv->tcon_info.u32PanelIndex, \
                    priv->tcon_info.chPanelIDString, priv->tcon_info.u32InchSize, \
                    priv->tcon_info.u32ChassisIndex, priv->tcon_info.u32VcomPattern, \
                    priv->tcon_info.u32VcomType, priv->tcon_info.u32SpreadPermillage, \
                    priv->tcon_info.u32SpreadFreq, priv->tcon_info.u32OcellDemuraIdx, \
                    priv->tcon_info.u32TconIndex, priv->tcon_info.u32CurrentMax, \
                    priv->tcon_info.u32BackLightCurrFreq, priv->tcon_info.u32BackLightPwmFreq, \
                    priv->tcon_info.u32BackLightWaitLogo, priv->tcon_info.u32BackLightControlIf);
        }
        else
        {
			Init_TCON_Path(dev, is_dlg_mode);
            UBOOT_DEBUG("[%d] init panel info sheet done \n", __LINE__);
        }
    }
	//Cal output vfreq  by clk and htt/vtt.
	priv->out_timing = E_OUTPUT_NONE;

	if (priv->typ_htt != 0 && priv->typ_vtt != 0 ) {

		vfreq = priv->typ_dclk / priv->typ_htt / priv->typ_vtt;
		UBOOT_DEBUG("[%d] vfreq = %dHz\n",__LINE__, vfreq);

		//parse output timing by DCLK/(HTT*VTT)
		if (IS_OUT_8K4K(priv->de_width, priv->de_height)) {
			if (IS_VFREQ_60HZ_GROUP(vfreq))
				priv->out_timing = E_8K4K_60HZ;

			if (IS_VFREQ_120HZ_GROUP(vfreq))
				priv->out_timing = E_8K4K_120HZ;

			if (IS_VFREQ_144HZ_GROUP(vfreq))
				priv->out_timing = E_8K4K_144HZ;

			if (priv->out_timing == E_8K4K_60HZ) {
			//set lane number
			if (priv->out_format == E_OUTPUT_RGB ||
				priv->out_format == E_OUTPUT_YUV444)
				priv->lanes = 32;

			if (priv->out_format == E_OUTPUT_YUV422)
				priv->lanes = 16;

			} else {
				priv->lanes = 64;
			}

		}

		if (IS_OUT_4K2K(priv->de_width, priv->de_height)) {
			if (IS_VFREQ_60HZ_GROUP(vfreq)) {
				priv->out_timing = E_4K2K_60HZ;
				priv->lanes = 8;
			}
			if (IS_VFREQ_120HZ_GROUP(vfreq)) {
				priv->out_timing = E_4K2K_120HZ;
				priv->lanes = 16;
			}
			if (IS_VFREQ_144HZ_GROUP(vfreq)) {
				priv->out_timing = E_4K2K_144HZ;
				priv->lanes = 16;
			}
		}

		if (IS_OUT_2K1K(priv->de_width, priv->de_height)) {
			if (IS_VFREQ_60HZ_GROUP(vfreq)) {
				priv->out_timing = E_FHD_60HZ;
				priv->lanes = 2;
			}
			if (IS_VFREQ_120HZ_GROUP(vfreq)) {
				priv->out_timing = E_FHD_120HZ;
				priv->lanes = 4;
			}
		}

		// special case start
		u32Dclk = priv->typ_dclk / MHZ;
		// 4K1K_120
		if (IS_OUT_4K1K(priv->de_width, priv->de_height)) {
			if ((IS_VFREQ_120HZ_GROUP(vfreq)) &&
				(IS_DCLK_600MHZ_GROUP(u32Dclk))) {
				// MOD/LPLL should be the same as E_4K2K_60HZ,
				// but TGEN should use 4K1K
				priv->out_timing = E_4K1K_120HZ;
				priv->lanes = 8;
			}
		}
		// 4K1K_240
		if (IS_OUT_4K1K(priv->de_width, priv->de_height)) {
			if ((IS_VFREQ_240HZ_GROUP(vfreq)) &&
				(IS_DCLK_1200MHZ_GROUP(u32Dclk))) {
				priv->out_timing = E_4K2K_120HZ;  // MOD/LPLL should be the same as E_4K2K_120HZ,  but TGEN should use 4K1K
				priv->lanes = 16;
			}
		}
		// FHD_120 projector 8lane case
		if (IS_OUT_2K1K(priv->de_width, priv->de_height)) {
			if ((IS_VFREQ_120HZ_GROUP(vfreq)) &&
				(IS_DCLK_300MHZ_GROUP(u32Dclk))) {
				priv->out_timing = E_4K2K_60HZ;  // MOD/LPLL should be the same as E_4K2K_60HZ,  but TGEN should use FHD
				priv->lanes = 8;
			}
		}
		// FHD_240 projector 8lane case
		if (IS_OUT_2K1K(priv->de_width, priv->de_height)) {
			if ((IS_VFREQ_240HZ_GROUP(vfreq)) &&
				(IS_DCLK_600MHZ_GROUP(u32Dclk))) {
				priv->out_timing = E_4K2K_60HZ;  // MOD/LPLL should be the same as E_4K2K_60HZ,  but TGEN should use FHD
				priv->lanes = 8;
			}
		}
		// FHD_240
		if (IS_OUT_2K1K(priv->de_width, priv->de_height)) {
			if ((IS_VFREQ_240HZ_GROUP(vfreq)) &&
				(IS_DCLK_1200MHZ_GROUP(u32Dclk))) {
				priv->out_timing = E_4K2K_120HZ;  // MOD/LPLL should be the same as E_4K2K_120HZ,  but TGEN should use FHD
				priv->lanes = 16;
			}
		}
		// special case end
		if (priv->tcon_info.bUsingTCON) {
            priv->lanes = _get_lanes_num_by_linktye(priv->linktype);
        }
	}else {
		UBOOT_ERROR("Output HTT/VTT Error, SHOULD NOT BE ZERO!\n");
		return -EINVAL;
	}
	return ret;
}


static void _dump_info(struct udevice *dev)
{
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (!priv)
		return;

	UBOOT_DEBUG("-----Dump PNL output info-----\n");
	UBOOT_DEBUG("link type = %d\n",priv->linktype);
	UBOOT_DEBUG("vbo_byte = %d\n",priv->vbo_byte);
	UBOOT_DEBUG("lanes = %d\n",priv->lanes);
	UBOOT_DEBUG("div_sec = %d\n",priv->div_sec);
	UBOOT_DEBUG("out_timing = %d\n",priv->out_timing);
	UBOOT_DEBUG("out_format = %d\n",priv->out_format);
	UBOOT_DEBUG("content_out_format = %d\n",priv->cout_format);
	UBOOT_DEBUG("ontiming_1 = %d\n",priv->ontiming_1);
	UBOOT_DEBUG("ontiming_2 = %d\n",priv->ontiming_2);
	UBOOT_DEBUG("offtiming_1 = %d\n",priv->offtiming_1);
	UBOOT_DEBUG("offtiming_2 = %d\n",priv->offtiming_2);
	UBOOT_DEBUG("hsync_st = %d\n",priv->hsync_st);
	UBOOT_DEBUG("hsync_w = %d\n",priv->hsync_w);
	UBOOT_DEBUG("hsync_pol = %d\n",priv->hsync_pol);
	UBOOT_DEBUG("vsync_st = %d\n",priv->vsync_st);
	UBOOT_DEBUG("vsync_w = %d\n",priv->vsync_w);
	UBOOT_DEBUG("vsync_pol = %d\n",priv->vsync_pol);
	UBOOT_DEBUG("de_hstart = %d\n",priv->de_hstart);
	UBOOT_DEBUG("de_vstart = %d\n",priv->de_vstart);
	UBOOT_DEBUG("de_width = %d\n",priv->de_width);
	UBOOT_DEBUG("de_height = %d\n",priv->de_height);
	UBOOT_DEBUG("max_htt = %d\n",priv->max_htt);
	UBOOT_DEBUG("typ_htt = %d\n",priv->typ_htt);
	UBOOT_DEBUG("min_htt = %d\n",priv->min_htt);
	UBOOT_DEBUG("max_vtt = %d\n",priv->max_vtt);
	UBOOT_DEBUG("typ_vtt = %d\n",priv->typ_vtt);
	UBOOT_DEBUG("min_vtt = %d\n",priv->min_vtt);
	UBOOT_DEBUG("max_vtt_panelprotect = %d\n",priv->max_vtt_panelprotect);
	UBOOT_DEBUG("min_vtt_panelprotect = %d\n",priv->min_vtt_panelprotect);
	UBOOT_DEBUG("max_dclk = %d\n",priv->max_dclk);
	UBOOT_DEBUG("typ_dclk = %lld\n",priv->typ_dclk);
	UBOOT_DEBUG("min_dclk = %d\n",priv->min_dclk);
	UBOOT_DEBUG("pnl_lib_version = %d\n",priv->pnl_lib_version);
    if(priv->tcon_info.bUsingTCON) {
        UBOOT_DEBUG("bUsingTCON = %d\n",priv->tcon_info.bUsingTCON);
        UBOOT_DEBUG("TCON path = %s\n",priv->tcon_info.tcon_bin_path);
        UBOOT_DEBUG("panel gamma = %d\n",priv->tcon_info.bPanelGammaEn);
        UBOOT_DEBUG("panel gamma path = %s\n",priv->tcon_info.panelgamma_bin_path);
        UBOOT_DEBUG("bOverDriveEn = %d\n",priv->tcon_info.bOverDriveEn);
    }

	UBOOT_DEBUG("-----Dump PNL output info DONE-----\n");
}

static int mtk_panel_ofdata_to_platdata(struct udevice *dev)
{
	//dump device tree.
	struct mtk_panel_priv *priv = dev_get_priv(dev);
	uint32_t tmp = 0;
	int ret = 0;
	ofnode info_node;

	if (!priv) {
		UBOOT_ERROR("Get device priv FAIL\n");
		return -EINVAL;
	}

	ret = _parse_dts_info(dev);

	//panel combo
	info_node = dev_read_subnode(dev, EXT_GRAPH_COMBO_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", EXT_GRAPH_COMBO_INFO);
		priv->ext_grpah_combo_info.graph_vbo_byte_mode = 0;
	} else {
		ofnode_read_u32(info_node, GRAPH_VBO_BYTE_MODE, &priv->ext_grpah_combo_info.graph_vbo_byte_mode);
	}

	//panel cus setting start
	info_node = dev_read_subnode(dev, CUS_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", CUS_INFO);
		priv->cus_info.mdelta = 0;
		priv->cus_info.mirror_mode = 0;
		priv->cus_info.hmirror_en = 0;
	} else {
		ofnode_read_u32(info_node, M_DELTA, &priv->cus_info.mdelta);
		ofnode_read_u32(info_node, PANEL_MIRROR_MODE, &priv->cus_info.mirror_mode);
		ofnode_read_u32(info_node, VCC_BL_CUSCTRL_TAG, &priv->cus_info.vcc_bl_cusctrl);
		ofnode_read_u32(info_node, SCDISP_PATH_SEL, &priv->cus_info.scdisp_path_sel);
		ofnode_read_u32(info_node, PANEL_TYPE, &priv->cus_info.panel_type);
		ofnode_read_u32(info_node, PWM_CURRENT_MIN, &priv->cus_info.pwm_current_min);
		ofnode_read_u32(info_node, PWM_CURRENT_MAX, &priv->cus_info.pwm_current_max);
		ofnode_read_u32(info_node, SPI_REFERENCE_CURRENT, &priv->cus_info.spi_reference_current);
		ofnode_read_u32(info_node, ADIM_FREQUENCY, &priv->cus_info.adim_frequency);
		ofnode_read_u32(info_node, PWM_DIMMING_RATE, &priv->cus_info.pwm_dimming_rate);
		ofnode_read_u32(info_node, CTRL_BIT_EN_TAG, &tmp);//Parser CTRL_BIT_ENABLE

		if (tmp == 1) {
			priv->cus_info.ctrl_bit_en = true;
			UBOOT_DEBUG("[%d] ctrl_bit_en=%u\n", __LINE__, priv->cus_info.ctrl_bit_en);
		} else {
			priv->cus_info.ctrl_bit_en = false;
			UBOOT_DEBUG("[%d] ctrl_bit_en=%u\n", __LINE__, priv->cus_info.ctrl_bit_en);
		}

		if (priv->cus_info.mirror_mode == E_PNL_MIRROR_H ||
			priv->cus_info.mirror_mode == E_PNL_MIRROR_V_H)
			priv->cus_info.hmirror_en = 1;
	}
	//panel cus setting end

	//parse hw version start
	info_node = dev_read_subnode(dev, HW_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", HW_INFO);
		memset(&priv->hw_info,0,sizeof(st_hw_info));
	} else {
		ofnode_read_u32(info_node, RCON_EN, &tmp);
		if (tmp == 1)
			priv->hw_info.rcon_enable = true;
		else
			priv->hw_info.rcon_enable = false;
		ofnode_read_u32(info_node,RCON_MAX,&priv->hw_info.rcon_max);
		ofnode_read_u32(info_node,RCON_MIN,&priv->hw_info.rcon_min);
		ofnode_read_u32(info_node,RCON_VAL,&priv->hw_info.rcon_value);
		ofnode_read_u32(info_node,BIAS_S_MAX,&priv->hw_info.biascon_single_max);
		ofnode_read_u32(info_node,BIAS_S_MIN,&priv->hw_info.biascon_single_min);
		ofnode_read_u32(info_node,BIAS_S_VAL,&priv->hw_info.biascon_single_value);
		ofnode_read_u32(info_node,BIAS_D_MAX,&priv->hw_info.biascon_double_max);
		ofnode_read_u32(info_node,BIAS_D_MIN,&priv->hw_info.biascon_double_min);
		ofnode_read_u32(info_node,BIAS_D_VAL,&priv->hw_info.biascon_double_value);
		ofnode_read_u32(info_node, RINT_EN, &tmp);
		if (tmp == 1)
			priv->hw_info.rint_enable = true;
		else
			priv->hw_info.rint_enable = false;
		ofnode_read_u32(info_node,RINT_MAX,&priv->hw_info.rint_max);
		ofnode_read_u32(info_node,RINT_MIN,&priv->hw_info.rint_min);
		ofnode_read_u32(info_node,RINT_VAL,&priv->hw_info.rint_value);
		ofnode_read_u32(info_node, PNL_LIB_VER, &priv->pnl_lib_version);
	}
	//parse hw version end

	//parse lane order start
	info_node = dev_read_subnode(dev, PKG_LN_ORDER);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", PKG_LN_ORDER);
		return -ENODATA;
	}
	ofnode_read_u32(info_node, SUP_LANES, &priv->lane_info.sup_lanes);

	if (priv->lane_info.sup_lanes > MAX_LANES) {
		UBOOT_ERROR("Support Lanes(%d) is larger than MAX_LANES(%d)\n"
			,priv->lane_info.sup_lanes, MAX_LANES);
		return -ENODATA;
	}

	//ofnode_read_u32_array(info_node, LAYOUT_ORDER, &priv->lane_info.def_layout, priv->lane_info.sup_lanes);

	info_node = dev_read_subnode(dev, USR_LN_ORDER);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", USR_LN_ORDER);
		return -ENOANO;
	}

	ofnode_read_u32(info_node, USR_DEF_MODE, &tmp);
	if (tmp == 1)
		priv->lane_info.usr_defined = true;
	else
		priv->lane_info.usr_defined = false;
	ofnode_read_u32(info_node, CTRL_LANES, &priv->lane_info.ctrl_lanes);
	if (priv->lane_info.ctrl_lanes > priv->lane_info.sup_lanes) {
		UBOOT_ERROR("Ctrl Lanes(%d) is larger than Sup Lanes(%d)\n"
			, priv->lane_info.ctrl_lanes, priv->lane_info.sup_lanes);
		return -ENODATA;
	}

	ofnode_read_u32_array(info_node, LANE_ORDER, priv->lane_info.lane_order, priv->lane_info.ctrl_lanes);
	ofnode_read_u32(info_node, PN_SWAP, &priv->lane_info.pn_swap);
	//parse lane order end

	//parse swing start
	info_node = dev_read_subnode(dev, SWING_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", SWING_INFO);
		return -ENOANO;
	}
	ofnode_read_u32(info_node, USR_SWING, &tmp);
	if (tmp == 1)
		priv->swing_info.usr_swing_level = true;
	else
		priv->swing_info.usr_swing_level = false;
	ofnode_read_u32(info_node, COMMON_SWING, &tmp);
	if (tmp == 1)
		priv->swing_info.common_swing = true;
	else
		priv->swing_info.common_swing = false;
	ofnode_read_u32(info_node, SWING_LANES, &priv->swing_info.ctrl_lanes);
	if (priv->swing_info.ctrl_lanes > MAX_LANES) {
		UBOOT_ERROR("Swing control Lanes(%d) is larger than MAX_LANES(%d)\n"
			, priv->swing_info.ctrl_lanes, MAX_LANES);
		return -ENODATA;
	}

	if (priv->swing_info.common_swing != 0) //common swing = 0 is False, >= 1 is True
		ofnode_read_u32_array(info_node, SWING_LEVEL, priv->swing_info.swing_level, 1);
	else
		ofnode_read_u32_array(info_node, SWING_LEVEL, priv->swing_info.swing_level, priv->swing_info.ctrl_lanes);

	//parse swing end

	//parse PE start
	info_node = dev_read_subnode(dev, PE_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", PE_INFO);
		return -ENOANO;
	}
	ofnode_read_u32(info_node, USR_PE, &tmp);
	if (tmp == 1)
		priv->pe_info.pe_usr = true;
	else
		priv->pe_info.pe_usr = false;
	ofnode_read_u32(info_node, COMMON_PE, &tmp);
	if (tmp == 1)
		priv->pe_info.common_pe = true;
	else
		priv->pe_info.common_pe = false;
	ofnode_read_u32(info_node, PE_LANES, &priv->pe_info.ctrl_lanes);
	if (priv->pe_info.ctrl_lanes > MAX_LANES) {
		UBOOT_ERROR("PE control Lanes(%d) is larger than MAX_LANES(%d)\n"
			, priv->pe_info.ctrl_lanes, MAX_LANES);
		return -ENODATA;
	}
	if (priv->pe_info.common_pe != 0) //common_pe = 0 is flase , >= 1 is True
		ofnode_read_u32_array(info_node, PE_LEVEL, priv->pe_info.pe_level, 1);
	else
		ofnode_read_u32_array(info_node, PE_LEVEL, priv->pe_info.pe_level, priv->pe_info.ctrl_lanes);

	//parse PE end

	//parse SSC control start
	info_node = dev_read_subnode(dev, SSC_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", SSC_INFO);
		return -ENOANO;
	}
	//ssc_info.ssc_en
	ofnode_read_u32(info_node, SSC_EN, &priv->ssc_info.ssc_en);
	if (priv->ssc_info.ssc_en != 0)
		priv->ssc_info.ssc_en = 1;
	ofnode_read_u32(info_node, SSC_DEVIATION, &priv->ssc_info.ssc_deviation);
	ofnode_read_u32(info_node, SSC_MODULATION, &priv->ssc_info.ssc_modulation);


	//parse SSC control end

#ifdef CONFIG_HAPS
	priv->linktype = E_LINK_LVDS;
	priv->out_timing = E_FHD_60HZ;
	priv->out_format = E_OUTPUT_RGB;
	priv->lanes = 2;
#endif

	//get VCC/BL gpio
	if( priv->cus_info.vcc_bl_cusctrl != 1){
		gpio_request_by_name(dev, "vcc-gpios", 0, &priv->gpio_vcc, GPIOD_IS_OUT);
		gpio_request_by_name(dev, "backlight-gpios", 0, &priv->gpio_backlight, GPIOD_IS_OUT);
	}


	//parse panel dither start
	info_node = dev_read_subnode(dev, DITHER_INFO);
	if (!ofnode_valid(info_node)) {
		UBOOT_ERROR("Read subnode (%s) FAIL\n", DITHER_INFO);
		return -ENOANO;
	}

	ofnode_read_u32(info_node, DITHER_DEPTH, &priv->dither_info.dither_depth);
	ofnode_read_u32(info_node, DITHER_PATTERN, &priv->dither_info.dither_pattern);
	ofnode_read_u32(info_node, DITHER_CAPABILITY, &priv->dither_info.dither_capability);

	//parse panel dither end

	//test dump info
	_dump_info(dev);

	return ret;
}

static int mtk_ext_video_ofdata_to_platdata(struct udevice *dev)
{
	UBOOT_TRACE("Delta video\n");
	int ret = 0;
	ret = _parse_dts_info(dev);
	_dump_info(dev);
	return ret;
}

static int mtk_gfx_ofdata_to_platdata(struct udevice *dev)
{
	UBOOT_TRACE("GFX path\n");
	int ret = 0;
	ret = _parse_dts_info(dev);
	_dump_info(dev);
	return ret;
}


static int mtk_panel_probe(struct udevice *dev)
{
	//read dts table and init clk
	struct mtk_panel_priv *priv = dev_get_priv(dev);

	if (priv == NULL) {
		UBOOT_ERROR("Get device private fail\n");
		return -ENXIO;
	}
    	UBOOT_TRACE("IN\n");
	PUBIF_HWREG_RENDER_VIDEO_PNL hwreg_render_video_pnl;
	printf("[wei] %s %d, PANEL LIB VER %d\n",__FUNCTION__,__LINE__, priv->pnl_lib_version);
	memset(&hwreg_render_video_pnl, 0, sizeof(PUBIF_HWREG_RENDER_VIDEO_PNL));
#if (CONFIG_HAPS == 0)
	if (priv->pnl_lib_version == BOOT_PNL_VERSION0600)
	{
		printf("[wei] BOOT_PNL_VERSION0600 \n");
		hwreg_render_video_pnl.fp_mtk_analog_setting = mtk_analog_setting_v006;
		hwreg_render_video_pnl.fp_mtk_pll_powerdown = mtk_pll_powerdown_v006;
		hwreg_render_video_pnl.fp_mtk_pnl_xtal_lpll_ckg_setting = mtk_pnl_xtal_lpll_ckg_setting_v006;
		hwreg_render_video_pnl.fp_mtk_pll_dump_clk_tbl = mtk_pll_dump_clk_tbl_v006;
		hwreg_render_video_pnl.fp_mtk_moda_dump_clk_tbl = mtk_moda_dump_clk_tbl_v006;
		hwreg_render_video_pnl.fp_mtk_out_clk_init = mtk_out_clk_init_v006;
		hwreg_render_video_pnl.fp_mtk_tgen_init = mtk_tgen_init_v006;
		hwreg_render_video_pnl.fp_mtk_pnl_video_ckg_setting = mtk_pnl_video_ckg_setting_v006;
		hwreg_render_video_pnl.fp_mtk_out_if_init = mtk_out_if_init_v006;
		hwreg_render_video_pnl.fp_mtk_disp_odclk_init = mtk_disp_odclk_init_v006;
		hwreg_render_video_pnl.fp_mtk_dump_mod_efuse = mtk_dump_mod_efuse_v006;
		hwreg_render_video_pnl.fp_mtk_swing_level_setting = mtk_swing_level_setting_v006;
		hwreg_render_video_pnl.fp_mtk_pre_emphasis_setting = mtk_pre_emphasis_setting_v006;
		hwreg_render_video_pnl.fp_mtk_SSC_control = mtk_SSC_control_v006;
		hwreg_render_video_pnl.fp_mtk_dither_setting = mtk_dither_setting_v006;
		hwreg_render_video_pnl.fp_mtk_pnl_controlbit_init = mtk_pnl_controlbit_init_v006;
		hwreg_render_video_pnl.fp_mtk_out_if_hbkproch_protect_init = mtk_out_if_hbkproch_protect_init_v006;
		hwreg_render_video_pnl.fp_mtk_pnl_set_vby1_mft_hmirror = mtk_pnl_set_vby1_mft_hmirror_v006;
		hwreg_render_video_pnl.fp_mtk_pnl_set_panel_SCDISP_Path_Sel = mtk_pnl_set_panel_SCDISP_Path_Sel_v006;
	}
	else
#endif
	{
		printf("[wei] BOOT_PNL_VERSION OLD \n");
		hwreg_render_video_pnl.fp_mtk_analog_setting = mtk_analog_setting;
		hwreg_render_video_pnl.fp_mtk_pll_powerdown = mtk_pll_powerdown;
		hwreg_render_video_pnl.fp_mtk_pnl_xtal_lpll_ckg_setting = mtk_pnl_xtal_lpll_ckg_setting;
		hwreg_render_video_pnl.fp_mtk_pll_dump_clk_tbl = mtk_pll_dump_clk_tbl;
		hwreg_render_video_pnl.fp_mtk_moda_dump_clk_tbl = mtk_moda_dump_clk_tbl;
		hwreg_render_video_pnl.fp_mtk_out_clk_init = mtk_out_clk_init;
		hwreg_render_video_pnl.fp_mtk_tgen_init = mtk_tgen_init;
		hwreg_render_video_pnl.fp_mtk_pnl_video_ckg_setting = mtk_pnl_video_ckg_setting;
		hwreg_render_video_pnl.fp_mtk_out_if_init = mtk_out_if_init;
		hwreg_render_video_pnl.fp_mtk_disp_odclk_init = mtk_disp_odclk_init;
		hwreg_render_video_pnl.fp_mtk_dump_mod_efuse = mtk_dump_mod_efuse;
		hwreg_render_video_pnl.fp_mtk_swing_level_setting = mtk_swing_level_setting;
		hwreg_render_video_pnl.fp_mtk_pre_emphasis_setting = mtk_pre_emphasis_setting;
		hwreg_render_video_pnl.fp_mtk_SSC_control = mtk_SSC_control;
		hwreg_render_video_pnl.fp_mtk_dither_setting = mtk_dither_setting;
		hwreg_render_video_pnl.fp_mtk_pnl_controlbit_init = mtk_pnl_controlbit_init;
		hwreg_render_video_pnl.fp_mtk_out_if_hbkproch_protect_init = mtk_out_if_hbkproch_protect_init;
		hwreg_render_video_pnl.fp_mtk_pnl_set_vby1_mft_hmirror = mtk_pnl_set_vby1_mft_hmirror;
		hwreg_render_video_pnl.fp_mtk_pnl_set_panel_SCDISP_Path_Sel = mtk_pnl_set_panel_SCDISP_Path_Sel;
	}
		hwreg_render_video_pnl.fp_mtk_analog_setting(dev);
		hwreg_render_video_pnl.fp_mtk_pll_powerdown(dev,false);
		hwreg_render_video_pnl.fp_mtk_pnl_xtal_lpll_ckg_setting(dev);
		hwreg_render_video_pnl.fp_mtk_pll_dump_clk_tbl(dev);
		hwreg_render_video_pnl.fp_mtk_moda_dump_clk_tbl(dev);
		hwreg_render_video_pnl.fp_mtk_out_clk_init(dev);
		hwreg_render_video_pnl.fp_mtk_tgen_init(dev);
		hwreg_render_video_pnl.fp_mtk_pnl_video_ckg_setting(dev);
		hwreg_render_video_pnl.fp_mtk_out_if_init(dev);
		hwreg_render_video_pnl.fp_mtk_disp_odclk_init(dev);
		hwreg_render_video_pnl.fp_mtk_dump_mod_efuse(dev);
		hwreg_render_video_pnl.fp_mtk_swing_level_setting(dev);
		hwreg_render_video_pnl.fp_mtk_pre_emphasis_setting(dev);
		hwreg_render_video_pnl.fp_mtk_SSC_control(dev);
		hwreg_render_video_pnl.fp_mtk_dither_setting(dev);
		hwreg_render_video_pnl.fp_mtk_pnl_controlbit_init(dev);
		hwreg_render_video_pnl.fp_mtk_out_if_hbkproch_protect_init(dev);
		hwreg_render_video_pnl.fp_mtk_pnl_set_vby1_mft_hmirror(dev);
		hwreg_render_video_pnl.fp_mtk_pnl_set_panel_SCDISP_Path_Sel(dev);
/*
	mtk_analog_setting(dev);

	//pnl init, only video part.
	mtk_pll_powerdown(dev,false); // including mpll tbl

	mtk_pnl_xtal_lpll_ckg_setting(dev);

	mtk_pll_dump_clk_tbl(dev); //LPLL tbl

	mtk_moda_dump_clk_tbl(dev); // moda tbl

	mtk_out_clk_init(dev); //odclk

	mtk_tgen_init(dev);

	mtk_pnl_video_ckg_setting(dev); //clk gated

	mtk_out_if_init(dev); //vby1 setting.

	mtk_disp_odclk_init(dev); //enable default clk for display module.

	mtk_dump_mod_efuse(dev);

	mtk_swing_level_setting(dev);
	mtk_pre_emphasis_setting(dev);
	mtk_SSC_control(dev);

	mtk_dither_setting(dev);

	mtk_pnl_controlbit_init(dev);
	mtk_out_if_hbkproch_protect_init(dev);
	mtk_pnl_set_vby1_mft_hmirror(dev);
	mtk_pnl_set_panel_SCDISP_Path_Sel(dev);
*/
	if (!is_tcon_sti_flow())
	{
		if(priv->tcon_info.bUsingTCON)
		{
		    Init_TCON_Panel(dev);
		    UBOOT_TRACE("init tcon done %s %d\n",__FUNCTION__,__LINE__);
		}
		mtk_overdrive_setting(dev);
		mtk_panelgamma_setting(dev, is_dlg_mode);
	}
	else
	{
		if (priv->tcon_info.bUsingTCON)
			mtk_tcon_init(dev); //tcon initialize
	}

    UBOOT_TRACE("OUT\n");
	return 0;
}

static int mtk_extvideo_probe(struct udevice *dev)
{
	//mtk_ext_video_analog_setting(dev); /* change to output enable */
	mtk_ext_video_clk_setting(dev);
	mtk_ext_video_out_if_init(dev); //vby1 setting
	mtk_delta_tgen_setting(dev);
	mtk_extv_controlbit_init(dev);
	mtk_ext_video_out_if_delta_hbkproch_protect_init(dev);
	return 0;
}

static int mtk_graphic_probe(struct udevice *dev)
{
	//mtk_gfx_analog_setting(dev); /* change to output enable */
	mtk_gfx_clk_setting(dev);
	mtk_gfx_out_if_init(dev); //vby1 setting
	mtk_gfx_controlbit_init(dev);
	mtk_gfx_out_if_hbkproch_protect_init(dev);
	return 0;
}

/*
 *static const struct panel_ops mtk_panel_ops = {
 *	.enable_backlight	= mtk_panel_enable_backlight,
 *	.set_backlight		= mtk_panel_set_backlight,
 *};
 */

static const struct dm_display_ops mtk_panel_ops = {
	.enable			= mtk_panel_enable_output,
	.get_property   = mtk_panel_get_property,
	.set_property   = mtk_panel_set_property,
};

static const struct dm_display_ops mtk_extvideo_ops = {
	.enable			= mtk_extv_enable_output,
};

static const struct dm_display_ops mtk_graphic_ops = {
	.enable			= mtk_gfx_enable_output,
};

static const struct udevice_id panel_ids[] = {
	{ .compatible = "mtk,mt5896-panel" },
	{ },
};

static const struct udevice_id ext_be_video_ids[] = {
	{ .compatible = "mtk,mt5896-extdev-panel" },
	{ },
};

static const struct udevice_id ext_be_gfx_ids[] = {
	{ .compatible = "mtk,mt5896-gfx-panel" },
	{ },
};


U_BOOT_DRIVER(mediatek_panel) = {
	.name = "mtk_video_out",
	.id = UCLASS_DISPLAY,
	.of_match = panel_ids,
	.ops = &mtk_panel_ops,
	.ofdata_to_platdata = mtk_panel_ofdata_to_platdata,
	.probe = mtk_panel_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_panel_priv),
};

U_BOOT_DRIVER(mediatek_ext_video) = {
	.name = "mtk_ext_video_out",
	.id = UCLASS_DISPLAY,
	.of_match = ext_be_video_ids,
	.ops = &mtk_extvideo_ops,
	.ofdata_to_platdata = mtk_ext_video_ofdata_to_platdata,
	.probe = mtk_extvideo_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_panel_priv),
};

U_BOOT_DRIVER(mediatek_gfx) = {
	.name = "mtk_graphic_out",
	.id = UCLASS_DISPLAY,
	.of_match = ext_be_gfx_ids,
	.ops = &mtk_graphic_ops,
	.ofdata_to_platdata = mtk_gfx_ofdata_to_platdata,
	.probe = mtk_graphic_probe,
	.priv_auto_alloc_size = sizeof(struct mtk_panel_priv),
};


