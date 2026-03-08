// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */

#ifndef _MTK_TV_PNL_H
#define _MTK_TV_PNL_H

#include <asm/gpio.h>

#define MAX_LANES	(64)
#define MPLL_CLK_864MHZ	(864)
#define MPLL_CLK_1296MHZ	(1296)
#define LPLL_SPAN_FACTOR	(131072)
#define LPLL_CAL_PARAM	(16)
#define SSC_SPAN_CAL_PARAM	(4)
#define LPLL_SPAN_FACTOR_V6	(524288)

#define MHZ (1000000ULL)
#define TGEN_VTTV_BUF_4320 (24) //should sync with boot
#define TGEN_VTTV_BUF_2160 (12) //should sync with boot
#define TGEN_VTTV_BUF_1080 (6) //should sync with boot
#define TGEN_PIXELSHIFT_BUF (10) //should sync with kernel
#define TGEN_VDE_ST_1080 (39) //should sync with kernel
#define TGEN_VDE_ST_2160 (78) //should sync with kernel
#define TGEN_VDE_ST_4320 (156) //should sync with kernel
#define TGEN_VDE_4320 4320
#define TGEN_VDE_2160 2160
#define TGEN_VDE_1080 1080

#define IS_OUT_8K4K(width,height) (((width <= 8000)&&(width >= 7500))&&((height <= 4500)&&(height >= 4000)))
#define IS_OUT_4K2K(width,height) (((width <= 4000)&&(width >= 3750))&&((height <= 2250)&&(height >= 2000)))
#define IS_OUT_2K1K(width,height) (((width <= 2000)&&(width >= 1875))&&((height <= 1125)&&(height >= 1000)))
#define IS_OUT_HD(width, height) (((width <= 1400) && (width >= 1200)) && ((height <= 800) && (height >= 700)))
#define IS_OUT_4K1K(width,height) (((width <= 4000)&&(width >= 3750))&&((height <= 1125)&&(height >= 1000)))
#define IS_VFREQ_60HZ_GROUP(vfreq) ((vfreq < 65) && (vfreq > 45))
#define IS_VFREQ_120HZ_GROUP(vfreq) ((vfreq < 125) && (vfreq > 90))
#define IS_VFREQ_144HZ_GROUP(vfreq) ((vfreq < 149) && (vfreq > 139))
#define IS_VFREQ_240HZ_GROUP(vfreq) ((vfreq < 260) && (vfreq > 180))
#define IS_DCLK_75MHZ_GROUP(dclk) ((dclk < 100) && (dclk > 25))
#define IS_DCLK_150MHZ_GROUP(dclk) ((dclk < 200) && (dclk > 100))
#define IS_DCLK_300MHZ_GROUP(dclk) ((dclk < 400) && (dclk > 200))
#define IS_DCLK_600MHZ_GROUP(dclk) ((dclk < 800) && (dclk > 400))
#define IS_DCLK_1200MHZ_GROUP(dclk) ((dclk < 1500) && (dclk > 800))
#define IS_VB1_OUT(x) ((x == E_LINK_VB1) || (x == E_LINK_VB1_TO_HDMITX))
#define IS_TCON_FIFOCLK_SUPPORT(x)    (((x >= E_LINK_USIT_8BIT_6PAIR) && (x <= E_LINK_CHPI_10BIT_6PAIR)) || \
                                       ((x >= E_LINK_USIT_8BIT_8PAIR) && (x <= E_LINK_USIT_10BIT_16PAIR)) || \
                                       ((x >= E_LINK_MINILVDS_1BLK_3PAIR_6BIT) && (x <= E_LINK_MINILVDS_2BLK_6PAIR_8BIT)))

#define OVERDRIVE_444_EN_VALUE                 (0x01) // 000 1
#define OVERDRIVE_565_EN_VALUE                 (0x03) // 001 1
#define OVERDRIVE_666_EN_VALUE                 (0x09) // 100 1
#define OVERDRIVE_COMPRESS_EN_VALUE            (0x0B) // 101 1
#define OVERDRIVE_555_EN_VALUE                 (0x0D) // 110 1
#define OVERDRIVE_888_EN_VALUE                 (0x0F) // 111 1

typedef enum {
	E_OUT_MODEL_VG_BLENDED,
	E_OUT_MODEL_VG_SEPARATED,
	E_OUT_MODEL_VG_SEPARATED_W_EXTDEV,
	E_OUT_MODEL_MAX,
}en_out_model;

typedef enum {
	E_OUTPUT_NONE,
	E_8K4K_144HZ,
	E_8K4K_120HZ,
	E_8K4K_60HZ,
	E_8K4K_30HZ,
	E_4K2K_120HZ,
	E_4K2K_60HZ,
	E_4K2K_30HZ,
	E_FHD_120HZ,
	E_FHD_60HZ,
	E_HD_120HZ,
	E_4K2K_144HZ,
	E_4K1K_144HZ,
	E_4K1K_240HZ,
	E_4K1K_120HZ,
	E_HD_60HZ,
	E_OUTPUT_MODE_MAX,
} en_output_timing;

typedef enum {
	E_OUTPUT_RGB,
	E_OUTPUT_YUV444,
	E_OUTPUT_YUV422,
	E_OUTPUT_YUV420,
	E_OUTPUT_ARGB8101010,
	E_OUTPUT_ARGB8888_W_DITHER,
	E_OUTPUT_ARGB8888_W_ROUND,
	E_OUTPUT_ARGB8888_MODE0,
	E_OUTPUT_ARGB8888_MODE1,
	E__OUTPUT_FORMAT_MAX,
} en_output_format;

typedef enum {
	E_LINK_NONE,							// 0
	E_LINK_VB1,							// 1
	E_LINK_LVDS,								// 2
	E_LINK_VB1_TO_HDMITX,					// 3
	E_LINK_MINILVDS,                      // 4
	E_LINK_TTL,                      // 5
	E_LINK_RESERVED6,                      // 6
	E_LINK_RESERVED7,					// 7
	E_LINK_RESERVED8,							//8
	E_LINK_RESERVED9,                      // 9
	E_LINK_EXT,                      	// 10
    E_LINK_HSLVDS_1CH = E_LINK_EXT,
    E_LINK_HSLVDS_2CH,
    E_LINK_MINILVDS_1BLK_3PAIR_6BIT,
    E_LINK_MINILVDS_1BLK_6PAIR_6BIT,
    E_LINK_MINILVDS_2BLK_3PAIR_6BIT,
    E_LINK_MINILVDS_2BLK_6PAIR_6BIT,
    E_LINK_MINILVDS_1BLK_3PAIR_8BIT,
    E_LINK_MINILVDS_1BLK_6PAIR_8BIT,
    E_LINK_MINILVDS_2BLK_3PAIR_8BIT,
    E_LINK_MINILVDS_2BLK_6PAIR_8BIT,
    E_LINK_EPI28_8BIT_2PAIR_2KCML,
    E_LINK_EPI28_8BIT_4PAIR_2KCML,
    E_LINK_EPI28_8BIT_6PAIR_2KCML,
    E_LINK_EPI28_8BIT_8PAIR_2KCML,
    E_LINK_EPI28_8BIT_6PAIR_4KCML,
    E_LINK_EPI28_8BIT_8PAIR_4KCML,
    E_LINK_EPI28_8BIT_12PAIR_4KCML,
    E_LINK_EPI24_10BIT_12PAIR_4KCML,
    E_LINK_EPI28_8BIT_2PAIR_2KLVDS,
    E_LINK_EPI28_8BIT_4PAIR_2KLVDS,
    E_LINK_EPI28_8BIT_6PAIR_2KLVDS,
    E_LINK_EPI28_8BIT_8PAIR_2KLVDS,
    E_LINK_EPI28_8BIT_6PAIR_4KLVDS,
    E_LINK_EPI28_8BIT_8PAIR_4KLVDS,
    E_LINK_EPI28_8BIT_12PAIR_4KLVDS,
    E_LINK_EPI24_10BIT_12PAIR_4KLVDS,
    E_LINK_CMPI27_8BIT_6PAIR,
    E_LINK_CMPI27_8BIT_8PAIR,
    E_LINK_CMPI27_8BIT_12PAIR,
    E_LINK_CMPI27_10BIT_8PAIR,
    E_LINK_CMPI27_10BIT_12PAIR,
    E_LINK_USIT_8BIT_6PAIR,
    E_LINK_USIT_8BIT_12PAIR,
    E_LINK_USIT_10BIT_6PAIR,
    E_LINK_USIT_10BIT_12PAIR,
    E_LINK_ISP_8BIT_6PAIR,
    E_LINK_ISP_8BIT_6X2PAIR,
    E_LINK_ISP_8BIT_8PAIR,
    E_LINK_ISP_8BIT_12PAIR,
    E_LINK_ISP_10BIT_6X2PAIR,
    E_LINK_ISP_10BIT_8PAIR,
    E_LINK_ISP_10BIT_12PAIR,
    E_LINK_CHPI_8BIT_6PAIR,
    E_LINK_CHPI_8BIT_6X2PAIR,
    E_LINK_CHPI_8BIT_8PAIR,
    E_LINK_CHPI_8BIT_12PAIR,
    E_LINK_CHPI_10BIT_8PAIR,
    E_LINK_CHPI_10BIT_12PAIR,
    E_LINK_CHPI_10BIT_6PAIR,
    E_LINK_EPI28_8BIT_16PAIR_4KCML,
    E_LINK_EPI24_10BIT_16PAIR_4KCML,    //60
    E_LINK_EPI28_8BIT_16PAIR_4KLVDS,
    E_LINK_EPI24_10BIT_16PAIR_4KLVDS,
    E_LINK_USIT_8BIT_8PAIR,
    E_LINK_USIT_8BIT_16PAIR,
    E_LINK_USIT_10BIT_16PAIR,
    E_LINK_MAX,
} en_link_if;

typedef enum {
	E_VBO_NO_LINK = 0,
	E_VBO_3BYTE_MODE = 3,
	E_VBO_4BYTE_MODE = 4,
	E_VBO_5BYTE_MODE = 5,
	E_VBO_MODE_MAX,
} en_vbo_bytemode;

typedef enum {
	E_DITHER_BYPASS,
	E_DITHER_TRUNCATION,
	E_DITHER_ROUND,
	E_DITHER_PATTERN_TABLE_DYNAMIC,//Regular
	E_DITHER_PATTERN_TABLE_STATIC,
	E_DITHER_RANDOM_DYNAMIC,//LFSR
	E_DITHER_RANDOM_STATIC,
	E_DITHER_MAX,
} en_dither_pattern;

typedef enum {
	E_DITHER_DEPTH_6,
	E_DITHER_DEPTH_8,
	E_DITHER_DEPTH_10,
	E_DITHER_DEPTH_12,
	E_DITHER_DEPTH_MAX,
} en_dither_depth;

typedef enum {
	E_PNL_MIRROR_NONE = 0,
	E_PNL_MIRROR_V,
	E_PNL_MIRROR_H,
	E_PNL_MIRROR_V_H,
	E_PNL_MIRROR_TYPE_MAX
} drm_en_pnl_mirror_type;

typedef enum {
	E_PNL_LPLL_DRV_VERSION_0 = 0,
	E_PNL_LPLL_DRV_VERSION_1,
	E_PNL_LPLL_DRV_VERSION_2,
	E_PNL_LPLL_DRV_VERSION_3,

	E_PNL_LPLL_DRV_VERSION_MAX
} drm_en_pnl_lpll_drv_type;

typedef enum {
	BOOT_PNL_VERSION0100 = 0x01,
	BOOT_PNL_VERSION0200 = 0x02,
	BOOT_PNL_VERSION0203 = 0x0203,
	BOOT_PNL_VERSION0300 = 0x03,
	BOOT_PNL_VERSION0400 = 0x04,
	BOOT_PNL_VERSION0500 = 0x05,
	BOOT_PNL_VERSION0600 = 0x06,
	BOOT_PNL_VERSION0700 = 0x07,
	BOOT_PNL_VERSIONMAX,
} en_boot_pnl_version;

typedef enum {
	E_BOOT_PNL_OUTPUT_LANE0,
	E_BOOT_PNL_OUTPUT_LANE1,
	E_BOOT_PNL_OUTPUT_LANE2,
	E_BOOT_PNL_OUTPUT_LANE3,
	E_BOOT_PNL_OUTPUT_LANE4,
	E_BOOT_PNL_OUTPUT_LANE5,
	E_BOOT_PNL_OUTPUT_LANE6,
	E_BOOT_PNL_OUTPUT_LANE7,
	E_BOOT_PNL_OUTPUT_LANE8,
	E_BOOT_PNL_OUTPUT_LANE9,
	E_BOOT_PNL_OUTPUT_LANE10,
	E_BOOT_PNL_OUTPUT_LANE11,
	E_BOOT_PNL_OUTPUT_LANE12,
	E_BOOT_PNL_OUTPUT_LANE13,
	E_BOOT_PNL_OUTPUT_LANE14,
	E_BOOT_PNL_OUTPUT_LANE15,
	E_BOOT_PNL_OUTPUT_LANE16,
	E_BOOT_PNL_OUTPUT_LANE17,
	E_BOOT_PNL_OUTPUT_LANE18,
	E_BOOT_PNL_OUTPUT_LANE19,
	E_BOOT_PNL_OUTPUT_LANE20,
	E_BOOT_PNL_OUTPUT_LANE21,
	E_BOOT_PNL_OUTPUT_LANE22,
	E_BOOT_PNL_OUTPUT_LANE23,
	E_BOOT_PNL_OUTPUT_LANE24,
	E_BOOT_PNL_OUTPUT_LANE25,
	E_BOOT_PNL_OUTPUT_LANE26,
	E_BOOT_PNL_OUTPUT_LANE27,
	E_BOOT_PNL_OUTPUT_LANE28,
	E_BOOT_PNL_OUTPUT_LANE29,
	E_BOOT_PNL_OUTPUT_LANE30,
	E_BOOT_PNL_OUTPUT_LANE31,
	E_BOOT_PNL_OUTPUT_LANE32,
	E_BOOT_PNL_OUTPUT_LANE33,
	E_BOOT_PNL_OUTPUT_LANE34,
	E_BOOT_PNL_OUTPUT_LANE35,
	E_BOOT_PNL_OUTPUT_LANE36,
	E_BOOT_PNL_OUTPUT_LANE37,
	E_BOOT_PNL_OUTPUT_LANE38,
	E_BOOT_PNL_OUTPUT_LANE39,
	E_BOOT_PNL_OUTPUT_LANE40,
	E_BOOT_PNL_OUTPUT_LANE41,
	E_BOOT_PNL_OUTPUT_LANE42,
	E_BOOT_PNL_OUTPUT_LANE43,
	E_BOOT_PNL_OUTPUT_LANE44,
	E_BOOT_PNL_OUTPUT_LANE45,
	E_BOOT_PNL_OUTPUT_LANE46,
	E_BOOT_PNL_OUTPUT_LANE47,
	E_BOOT_PNL_OUTPUT_LANE48,
	E_BOOT_PNL_OUTPUT_LANE49,
	E_BOOT_PNL_OUTPUT_LANE50,
	E_BOOT_PNL_OUTPUT_LANE51,
	E_BOOT_PNL_OUTPUT_LANE52,
	E_BOOT_PNL_OUTPUT_LANE53,
	E_BOOT_PNL_OUTPUT_LANE54,
	E_BOOT_PNL_OUTPUT_LANE55,
	E_BOOT_PNL_OUTPUT_LANE56,
	E_BOOT_PNL_OUTPUT_LANE57,
	E_BOOT_PNL_OUTPUT_LANE58,
	E_BOOT_PNL_OUTPUT_LANE59,
	E_BOOT_PNL_OUTPUT_LANE60,
	E_BOOT_PNL_OUTPUT_LANE61,
	E_BOOT_PNL_OUTPUT_LANE62,
	E_BOOT_PNL_OUTPUT_LANE63,
	E_BOOT_PNL_OUTPUT_LANE_MAX,
} en_boot_pnl_output_lane_config;

typedef enum
{
	E_PNL_INTERFACE_MINILVDS_TYPE = 0,
	E_PNL_INTERFACE_EPI_TYPE,
	E_PNL_INTERFACE_ISP_TYPE,
	E_PNL_INTERFACE_CEDS_TYPE,
	E_PNL_INTERFACE_USIT_TYPE,
	E_PNL_INTERFACE_CMPI_TYPE,
	E_PNL_INTERFACE_VBY1_TYPE,
	E_PNL_INTERFACE_CHPI_TYPE,
	E_PNL_INTERFACE_CSPI_TYPE,
	E_PNL_INTERFACE_MAX_TYPE,
} EN_PNL_INTERFACE_TYPE;

enum en_boot_scdisp_path_sel {
	E_BOOT_SCDISP_PATH_PQGA_OSDB_LD,
	E_BOOT_SCDISP_PATH_PQGA_LD_OSDB,
	E_BOOT_SCDISP_PATH_OSDB_PQGA_LD,
	E_BOOT_SCDISP_PATH_OSDB_LD_PQGA,
	E_BOOT_SCDISP_PATH_LD_PQGA_OSDB,
	E_BOOT_SCDISP_PATH_LD_OSDB_PQGA,
	E_BOOT_SCDISP_PATH_MAX,
};

enum PNL_HAL_OVERDRIVE_TYPE {
	E_HALPNL_OD_MODE_NONE                       = 0,

	E_HALPNL_OD_MODE_RGB_444,
	E_HALPNL_OD_MODE_RGB_565,
	E_HALPNL_OD_MODE_RGB_666,
	E_HALPNL_OD_MODE_RGB_555,
	E_HALPNL_OD_MODE_RGB_888,                   // 5

	E_HALPNL_OD_MODE_RGB_444_HSCALING,
	E_HALPNL_OD_MODE_RGB_565_HSCALING,
	E_HALPNL_OD_MODE_RGB_666_HSCALING,
	E_HALPNL_OD_MODE_RGB_555_HSCALING,
	E_HALPNL_OD_MODE_RGB_888_HSCALING,          // 10

	E_HALPNL_OD_MODE_RGB_444_HVSCALING,
	E_HALPNL_OD_MODE_RGB_565_HVSCALING,
	E_HALPNL_OD_MODE_RGB_666_HVSCALING,
	E_HALPNL_OD_MODE_RGB_555_HVSCALING,
	E_HALPNL_OD_MODE_RGB_888_HVSCALING,         // 15

	E_HALPNL_OD_MODE_COMPRESS_VLC,
	E_HALPNL_OD_MODE_COMPRESS_VLC_HSCALING,
	E_HALPNL_OD_MODE_COMPRESS_VLC_HVSCALING,    // 18

	E_HALPNL_OD_MODE_COMPRESS_CE,
	E_HALPNL_OD_MODE_COMPRESS_CE_HSCALING,
	E_HALPNL_OD_MODE_COMPRESS_ONE3RD,
	E_HALPNL_OD_MODE_COMPRESS_ONE3RD_HCALING,   // 22

	E_HALPNL_OD_MODE_RGB_444_VSD_HSD_4X,
	E_HALPNL_OD_MODE_RGB_565_VSD_HSD_4X,
	E_HALPNL_OD_MODE_RGB_666_VSD_HSD_4X,
	E_HALPNL_OD_MODE_RGB_555_VSD_HSD_4X,
	E_HALPNL_OD_MODE_RGB_888_VSD_HSD_4X,            // 27

	E_HALPNL_OD_MODE_MAX,
};

typedef struct  {
	uint32_t graph_vbo_byte_mode;
}st_ext_graph_combo_info;

typedef struct  {
	uint32_t mdelta;
	uint32_t mirror_mode;
	bool hmirror_en;
	uint32_t vcc_bl_cusctrl;
	enum en_boot_scdisp_path_sel scdisp_path_sel;
	uint32_t panel_type;
	uint32_t pwm_current_min;
	uint32_t pwm_current_max;
	uint32_t spi_reference_current;
	uint32_t adim_frequency;
	uint32_t pwm_dimming_rate;
	bool ctrl_bit_en;
}st_cus_info;

typedef struct {
	bool rcon_enable;
	uint32_t rcon_max;
	uint32_t rcon_min;
	uint32_t rcon_value;
	uint32_t biascon_single_max;
	uint32_t biascon_single_min;
	uint32_t biascon_single_value;
	uint32_t biascon_double_max;
	uint32_t biascon_double_min;
	uint32_t biascon_double_value;
	bool rint_enable;
	uint32_t rint_max;
	uint32_t rint_min;
	uint32_t rint_value;
}st_hw_info;

typedef struct  {
	uint32_t sup_lanes;
	uint32_t def_layout[MAX_LANES];
	bool usr_defined;
	uint32_t ctrl_lanes;
	uint32_t lane_order[MAX_LANES];
	uint32_t pn_swap;

}st_out_lane_order;

typedef struct {
	volatile bool usr_swing_level; //work around?
	bool common_swing;
	uint32_t ctrl_lanes;
	uint32_t swing_level[MAX_LANES];
}st_out_swing_level;

typedef struct {
	bool pe_usr;
	bool common_pe;
	uint32_t ctrl_lanes;
	uint32_t pe_level[MAX_LANES];
}st_out_pe_level;

typedef struct {
	uint32_t ssc_en; //work around? //20220311 remove volatile
	uint32_t ssc_modulation;
	uint32_t ssc_deviation;
}st_out_ssc_ctrl;

typedef struct {
	en_dither_pattern dither_pattern;
	en_dither_depth dither_depth;
	uint32_t dither_capability;
}st_pnl_dither;

typedef struct
{
    uint32_t u32PanelLinkType;      //use for STI and uboot
    uint32_t u32PanelLinkExtType;   //use for mixed mode utopia
    uint32_t u32FixedVBackPorch;
    uint32_t u32FixedHBackPorch;
    uint32_t u32VsyncStart;
    uint32_t u32VsyncEnd;
    uint32_t u32VDEStart;
    uint32_t u32VDEEnd;
    uint32_t u32PanelMaxVTotal;
    uint32_t u32PanelVTotal;
    uint32_t u32PanelMinVTotal;
    uint32_t u32VDEHeadDummy;
    uint32_t u32VDETailDummy;
    uint32_t u32HsyncStart;
    uint32_t u32HsyncEnd;
    uint32_t u32HDEStart;
    uint32_t u32HDEEnd;
    uint32_t u32PanelMaxHTotal;
    uint32_t u32PanelHTotal;
    uint32_t u32PanelMinHTotal;
    uint32_t u32HDEHeadDummy;
    uint32_t u32HDETailDummy;
    uint32_t u32PanelMaxDCLK;
    uint32_t u32PanelDCLK;
    uint32_t u32PanelMinDCLK;
    uint32_t u32PanelMaxSET;
    uint32_t u32PanelMinSET;
    uint32_t u32OutTimingMode;
    uint32_t u32HTotalPair;
    uint32_t u32VTotalPair_Even;
    uint32_t u32VTotalPair_Odd;
    uint32_t u32DaulPort;
    uint32_t u32PanelHsyncWidth;
    uint32_t u32PanelHsyncBackPorch;
    uint32_t u32PanelVsyncWidth;
    uint32_t u32PanelVsyncBackPorch;
    uint32_t u32SSCEnable;
    uint32_t u32SSC_Fmodulation;
    uint32_t u32SSC_Percentage;
    uint32_t u32SSC_TconBinCtrl;
    uint32_t u32Demura_sel;
    uint32_t u32DataPath_sel;
    uint32_t u32Vcom_sel;
    uint32_t u32DemuraEnable;
	u32 u32OdEnable;
	u32 u32OdModeType;
}ST_PNL_TCON_PANELINFO;

struct st_lvds_info {
	bool panel_swap_lvds_pol;
	bool panel_swap_lvds_ch;
	bool panel_pdp_10bit;
	bool panel_lvds_ti_mode;
	u32 ti_bit_mode;	//TI_10BIT_MODE = 0,    //TI_8BIT_MODE  = 2,    //TI_6BIT_MODE  = 3,
};

typedef struct {
	bool bUsingTCON;
	const char *tcon_bin_path;
	const char *tcon_dlg_bin_path;
	ST_PNL_TCON_PANELINFO stPanelInfo;
	EN_PNL_INTERFACE_TYPE enPanelInterface;
	bool bPanelGammaEn;
	const char *panelgamma_bin_path;
	const char *panelgamma_dlg_bin_path;
	bool bOverDriveEn;
    const char *chPanelIDString;
    uint32_t u32PanelIndex;
    uint32_t u32InchSize;
    uint32_t u32ChassisIndex;
    uint32_t u32VcomPattern;
    uint32_t u32VcomType;
    uint32_t u32SpreadPermillage;
    uint32_t u32SpreadFreq;
    uint32_t u32OcellDemuraIdx;
    uint32_t u32TconIndex;
    uint32_t u32CurrentMax;
    uint32_t u32BackLightCurrFreq;
    uint32_t u32BackLightPwmFreq;
    uint32_t u32BackLightWaitLogo;
    uint32_t u32BackLightControlIf;
    const char *higt_frame_rate_bin_path;
    const char *higt_pixel_clock_bin_path;
    const char *game_mode_bin_path;
    const char *higt_frame_rate_gamma_bin_path;
    const char *higt_pixel_clock_gamma_bin_path;
    const char *game_mode_gamma_bin_path;
    const char *power_seq_on_bin_path;
	bool bvrr_od_en;
	u32 u32AutoPga;
	u32 u32P2pCmdUpdate;
	bool bIsProjector;
	uint32_t vcc_to_custic_delay;
}st_tcon_info;

struct mtk_panel_priv {
	en_link_if linktype;
	en_vbo_bytemode vbo_byte;
	uint32_t lanes;
	uint32_t div_sec;
	uint32_t ontiming_1;
	uint32_t ontiming_2;
	uint32_t offtiming_1;
	uint32_t offtiming_2;
	uint32_t hsync_st;
	uint32_t hsync_w;
	uint32_t hsync_pol;
	uint32_t vsync_st;
	uint32_t vsync_w;
	uint32_t vsync_pol;
	uint32_t de_hstart;
	uint32_t de_vstart;
	uint32_t de_width;
	uint32_t de_height;
	uint32_t max_htt;
	uint32_t typ_htt;
	uint32_t min_htt;
	uint32_t max_vtt;
	uint32_t typ_vtt;
	uint32_t min_vtt;
	uint32_t max_vtt_panelprotect;
	uint32_t min_vtt_panelprotect;
	uint32_t max_dclk;
	uint64_t typ_dclk;
	uint32_t min_dclk;
	uint32_t pnl_lib_version;
	en_output_timing out_timing;
	en_output_format out_format;
	en_output_format cout_format;
	st_out_lane_order lane_info;
	st_out_swing_level swing_info;
	st_out_pe_level pe_info;
	st_out_ssc_ctrl ssc_info;
	struct st_lvds_info lvds_info;
	struct gpio_desc gpio_vcc;
	struct gpio_desc gpio_backlight;
	st_hw_info hw_info;
	st_cus_info cus_info;
	st_pnl_dither dither_info;
	st_ext_graph_combo_info ext_grpah_combo_info;
	bool lane_duplicate_en;
	bool hpc_mode_en;
	uint32_t game_direct_fr_group;
	bool dlg_on;
	st_tcon_info tcon_info;
	uint32_t video_oled_pixel_shift;
	uint32_t video_oled_pixel_hmax;
	uint32_t video_oled_pixel_hmin;
	uint32_t video_oled_pixel_vmax;
	uint32_t video_oled_pixel_vmin;
};

struct mtk_ext_device_priv {
	en_link_if linktype;
	int lanes;
	int div_sec;
	en_output_timing out_timing;
	en_output_format out_format;
};

struct mtk_qms_tfr_mapping_info {
	u16 tfr;
	u16 input_vfrq;
	u16 ratio;
	u16 output_vfrq;
};

typedef void (*cb_mtk_pnl_set_vby1_mft_hmirror)(struct udevice *dev);
typedef int (*cb_mtk_analog_setting)(struct udevice *dev);
typedef int (*cb_mtk_pll_powerdown)(struct udevice *dev,bool enable);
typedef int (*cb_mtk_pnl_xtal_lpll_ckg_setting)(struct udevice *dev);
typedef int (*cb_mtk_pll_dump_clk_tbl)(struct udevice *dev);
typedef int (*cb_mtk_moda_dump_clk_tbl)(struct udevice *dev);
typedef int (*cb_mtk_out_clk_init)(struct udevice *dev);
typedef int (*cb_mtk_tgen_init)(struct udevice *dev);
typedef int (*cb_mtk_pnl_video_ckg_setting)(struct udevice *dev);
typedef int (*cb_mtk_out_if_init)(struct udevice *dev);
typedef int (*cb_mtk_disp_odclk_init)(struct udevice *dev);
typedef void (*cb_mtk_dump_mod_efuse)(struct udevice *dev);
typedef void (*cb_mtk_swing_level_setting)(struct udevice *dev);
typedef void (*cb_mtk_pre_emphasis_setting)(struct udevice *dev);
typedef void (*cb_mtk_SSC_control)(struct udevice *dev);
typedef void (*cb_mtk_dither_setting)(struct udevice *dev);
typedef void (*cb_mtk_pnl_controlbit_init)(struct udevice *dev);
typedef void (*cb_mtk_out_if_hbkproch_protect_init)(struct udevice *dev);
typedef void (*cb_mtk_pnl_set_panel_SCDISP_Path_Sel)(struct udevice *dev);

typedef struct _pubif_hwreg_render_video_pnl {
	cb_mtk_pnl_set_vby1_mft_hmirror
		fp_mtk_pnl_set_vby1_mft_hmirror;
	cb_mtk_analog_setting
		fp_mtk_analog_setting;
	cb_mtk_pll_powerdown
		fp_mtk_pll_powerdown;
	cb_mtk_pnl_xtal_lpll_ckg_setting
		fp_mtk_pnl_xtal_lpll_ckg_setting;
	cb_mtk_pll_dump_clk_tbl
		fp_mtk_pll_dump_clk_tbl;
	cb_mtk_moda_dump_clk_tbl
		fp_mtk_moda_dump_clk_tbl;
	cb_mtk_out_clk_init
		fp_mtk_out_clk_init;
	cb_mtk_tgen_init
		fp_mtk_tgen_init;
	cb_mtk_pnl_video_ckg_setting
		fp_mtk_pnl_video_ckg_setting;
	cb_mtk_out_if_init
		fp_mtk_out_if_init;
	cb_mtk_disp_odclk_init
		fp_mtk_disp_odclk_init;
	cb_mtk_dump_mod_efuse
		fp_mtk_dump_mod_efuse;
	cb_mtk_swing_level_setting
		fp_mtk_swing_level_setting;
	cb_mtk_pre_emphasis_setting
		fp_mtk_pre_emphasis_setting;
	cb_mtk_SSC_control
		fp_mtk_SSC_control;
	cb_mtk_dither_setting
		fp_mtk_dither_setting;
	cb_mtk_pnl_controlbit_init
		fp_mtk_pnl_controlbit_init;
	cb_mtk_out_if_hbkproch_protect_init
		fp_mtk_out_if_hbkproch_protect_init;
	cb_mtk_pnl_set_panel_SCDISP_Path_Sel
		fp_mtk_pnl_set_panel_SCDISP_Path_Sel;
} PUBIF_HWREG_RENDER_VIDEO_PNL;

#endif

