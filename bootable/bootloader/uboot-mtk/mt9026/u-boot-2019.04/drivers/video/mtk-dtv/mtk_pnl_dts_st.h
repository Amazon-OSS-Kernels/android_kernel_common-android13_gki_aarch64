// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */

//Tags for mapping device tree elements.

#ifndef _MTK_PNL_DTS_ST_H
#define _MTK_PNL_DTS_ST_H

#define INFO	"panel_info"
#define DLG_INFO	"panel_info_DLG"

#define SYS_INFO	"system_info"
#define PNL_LIB_VER	"pnl_lib_version"

#define PNL_NAME 	"panel_name"
#define LINK_TYPE 	"link_type"
#define VBO_BYTE	"vbo_byte_mode"
#define DIV_SEC 	"div_section"
#define ON_T1 		"on_timing1"
#define ON_T2 		"on_timing2"
#define OFF_T1		"off_timing1"
#define OFF_T2 		"off_timing2"
#define HS_ST		"hsync_start"
#define HS_WIDTH	"hsync_width"
#define HS_POL		"hsync_polarity"
#define VS_ST		"vsync_start"
#define VS_WIDTH	"vsync_width"
#define VS_POL		"vsync_polarity"
#define DE_H_ST		"de_hstart"
#define DE_V_ST		"de_vstart"
#define DE_WIDTH	"resolution_width"
#define DE_HEIGHT	"resolution_height"
#define MAX_HTT		"max_h_total"
#define TYP_HTT		"typ_h_total"
#define MIN_HTT		"min_h_total"
#define MAX_VTT		"max_v_total"
#define TYP_VTT		"typ_v_total"
#define MIN_VTT		"min_v_total"
#define MAX_VTT_PANEL_PROTECT		"max_v_total_panelprotect"
#define MIN_VTT_PANEL_PROTECT		"min_v_total_panelprotect"
#define MAX_CLK		"max_clk"
#define TYP_CLK_H		"typ_clk_high"
#define TYP_CLK_L		"typ_clk_low"
#define MIN_CLK		"min_clk"

#define OUT_FORMAT	"output_format"
#define COUT_FORMAT    "content_output_format"

#define HPC_MODE_TAG		"hpc_mode"
#define GAME_DIRECT_FR_GROUP_TAG	"GameDirectFrameRateGroup"
#define DLG_ON_TAG	"dlg_on"

#define EXT_GRAPH_COMBO_INFO	"ext_graph_combo_info"
#define GRAPH_VBO_BYTE_MODE		"graph_vbo_byte_mode"

#define CUS_INFO	"panel-cus-setting"
#define M_DELTA		"M_delta"
#define PANEL_MIRROR_MODE		"PanelMirrorMode"
#define CTRL_BIT_EN_TAG		"CTRL_BIT_Enable"
#define TCON_ENABLE             "TCON_Enable"
#define TCON_BIN_PATH           "TCON_BinPath"
#define PANELGAMMA_ENABLE       "PanelGamma_Enable"
#define PANELGAMMA_BIN_PATH     "PanelGamme_BinPath"
#define OVERDIRVE_ENABLE        "OverDrive_Enable"
#define PANEL_INDEX             "PanelIndex"
#define PANEL_ID_STRING         "PanelIDString"
#define INCH_SIZE               "InchSize"
#define CHASSIS_INDEX           "ChassisIndex"
#define VCOM_PATTERN            "VCOM_Pattern"
#define VCOM_TYPE               "VCOM_Type"
#define SPREAD_PERMILLAGE       "SpreadPermillage"
#define SPREAD_FREQ             "SpreadFreq"
#define OCELL_DEMURA_IDX        "Ocell_Demura_IDX"
#define TCON_INDEX              "Tcon_Index"
#define CURRENT_MAX             "CURRENT_MAX"
#define BL_CUR_FREQ             "BL_CUR_FREQ"
#define BL_PWM_FREQ             "BL_PWM_FREQ"
#define BL_WAIT_LOGO            "BL_WAIT_LOGO"
#define BL_CONTROL_IF           "BL_CONTROL_IF"
#define AUTO_PANELGAMMA         "Auto_PanelGamma"
#define P2P_CMD_UPDATE          "p2p_cmd_update"
#define PANEL_TYPE              "Panel_Type"
#define PWM_CURRENT_MIN         "pwm_current_min"
#define PWM_CURRENT_MAX         "pwm_current_max"
#define SPI_REFERENCE_CURRENT   "spi_reference_current"
#define ADIM_FREQUENCY          "adim_frequency"
#define PWM_DIMMING_RATE        "pwm_dimming_rate"

#define VCC_BL_CUSCTRL_TAG      "VCC_BL_CusCtrl"
#define SCDISP_PATH_SEL         "SCDISP_PATH_SEL"

#define VCC_TO_CUSTIC_DLY       "vcc_to_custic_delay"

/*
 *#################################
 *#    tconless model setting     #
 *#################################
 */
#define TCONLESS_INFO                   "tconless_model_info"
#define HIGT_FRAME_RATE_BIN_PATH        "higt_frame_rate_bin_path"
#define HIGT_PIXEL_CLOCK_BIN_PATH       "higt_pixel_clock_bin_path"
#define GAME_MODE_BIN_PATH              "game_mode_bin_path"
#define HIGT_FRAME_RATE_GAMMA_BIN_PATH  "higt_frame_rate_gamma_bin_path"
#define HIGT_PIXEL_CLOCK_GAMMA_BIN_PATH "higt_pixel_clock_gamma_bin_path"
#define GAME_MODE_GAMMA_BIN_PATH        "game_mode_gamma_bin_path"
#define POWER_SEQ_ON_BIN_PATH           "power_seq_on_bin_path"

#define PNL_TCON_VRR_OD_INFO_NODE       "panel_tcon_vrr_od_info"
#define VRR_OD_EN_TAG                   "vrr_od_enable"

#define HW_INFO		"hw-info"
#define RCON_EN		"rcon_enable"
#define RCON_MAX	"rcon_max"
#define RCON_MIN	"rcon_min"
#define RCON_VAL	"rcon_value"
#define BIAS_S_MAX	"biascon_single_max"
#define BIAS_S_MIN	"biascon_single_min"
#define BIAS_S_VAL	"biascon_single_value"
#define BIAS_D_MAX	"biascon_double_max"
#define BIAS_D_MIN	"biascon_double_min"
#define BIAS_D_VAL	"biascon_double_value"
#define RINT_EN		"rint_enable"
#define RINT_MAX	"rint_max"
#define RINT_MIN	"rint_min"
#define RINT_VAL	"rint_value"

#define PKG_LN_ORDER	"pkg_lane_order"
#define SUP_LANES	"support_lanes"
#define LAYOUT_ORDER	"layout_order"

#define USR_LN_ORDER	"usr_lane_order"
#define USR_DEF_MODE	"usr_define"
#define CTRL_LANES	"ctrl_lanes"
#define LANE_ORDER	"laneorder"
#define PN_SWAP		"pn_swap"

#define SWING_INFO	"output_swing_info"
#define USR_SWING	"swing_usr_define"
#define COMMON_SWING	"common_swing"
#define SWING_LANES	"swing_ctrl_lane"
#define SWING_LEVEL	"swing_level"

#define PE_INFO		"output_pre_emphasis"
#define USR_PE		"pe_usr_define"
#define COMMON_PE	"common_pe"
#define PE_LANES	"pe_ctrl_lane"
#define PE_LEVEL	"pe_level"

#define LANE_DUPLICATE "lane_duplicate"

#define SSC_INFO	"spread_spectrum_info"
#define SSC_EN		"ssc_ctrl_en"
#define SSC_MODULATION	"ssc_modulation"
#define SSC_DEVIATION	"ssc_deviation"

#define DITHER_INFO	"panel-dither-info"
#define DITHER_DEPTH	"dither_out_depth"
#define DITHER_PATTERN	"dither_pattern_type"
#define DITHER_CAPABILITY	"dither_capability"

#define PROJECTOR_UI_EN		"Projector_UI_en"
#define PANEL_SWAP_LVDS_POL	"Panel_Swap_Lvds_Pol"
#define PANEL_SWAP_LVDS_CH	"Panel_Swap_Lvds_Ch"
#define PANEL_PDP_10BIT	"Panel_Pdp_10bit"
#define PANEL_LVDS_TI_MODE	"Panel_Lvds_TI_Mode"
#define PANEL_TI_BIT_MODE	"Panel_TI_Bit_Mode"
#define PROJECTOR_UI_EN		"Projector_UI_en"

#endif
