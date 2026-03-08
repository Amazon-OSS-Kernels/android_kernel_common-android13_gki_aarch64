// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */

#ifndef _MTK_PNL_OUT_IF_H_
#define _MTK_PNL_OUT_IF_H_


#define SUPPORT_OVERDRIVE                   1
#define SUPPORT_PCID_LINEOD                 1
#define SUPPORT_VAC_256                     1
#define ENABLE_PNL_GAMMA_AUTODOWNLOAD      (TRUE)//FIXME
#define ENABLE_OD_AUTODOWNLOAD             (TRUE)
#define ENABLE_PCID_AUTODOWNLOAD           (TRUE)
#define ENABLE_VAC_AUTODOWNLOAD            (TRUE)


int mtk_tgen_init(struct udevice *dev);
int mtk_delta_tgen_setting(struct udevice *dev);
int mtk_out_if_init(struct udevice *dev); //vby1 setting.
int mtk_ext_video_out_if_init(struct udevice *dev); //vby1 setting for ext_video
int mtk_gfx_out_if_init(struct udevice *dev); //vby1 setting for gfx path
void mtk_out_if_hbkproch_protect_init(struct udevice *dev);
void mtk_ext_video_out_if_delta_hbkproch_protect_init(struct udevice *dev);
void mtk_gfx_out_if_hbkproch_protect_init(struct udevice *dev);
void mtk_pnl_out_en(struct udevice *dev, bool en);
void mtk_extv_out_en(struct udevice *dev, bool en);
void mtk_gfx_out_en(struct udevice *dev, bool en);
void mtk_pnl_mute_en(struct udevice *dev, bool en);
void mtk_extv_mute_en(struct udevice *dev, bool en);
void mtk_gfx_mute_en(struct udevice *dev, bool en);
void mtk_pnl_set_vby1_mft_hmirror(struct udevice *dev);
bool mtk_dump_tcon_efuse(uint32_t hwVersion);
void vby1_set_pn_swap(uint32_t pnl_lib_version);
void mtk_pre_emphasis_setting(struct udevice *dev);
void mtk_swing_level_setting(struct udevice *dev);
void mtk_SSC_control(struct udevice *dev);
void mtk_dump_mod_efuse(struct udevice *dev);
void mtk_pnl_controlbit_init(struct udevice *dev);
void mtk_extv_controlbit_init(struct udevice *dev);
void mtk_gfx_controlbit_init(struct udevice *dev);
void mtk_dither_setting(struct udevice *dev);
void mtk_pnl_set_panel_SCDISP_Path_Sel(struct udevice *dev);
void efuse_set_lpllsetting(uint32_t hwVersion);
void Init_TCON_Panel(struct udevice *dev);
void Init_TCON_Path(struct udevice *dev, bool dlg_mode);
bool mtk_pnl_autodownload_init(struct udevice *dev);
void mtk_panelgamma_setting(struct udevice *dev, bool dlg_mode);
void mtk_overdrive_setting(struct udevice *dev);
bool mtk_pnl_get_property(struct udevice *dev, int enType, int *pVal);
void mtk_testpattern_settings(struct udevice *dev);
bool mtk_pnl_set_property(struct udevice *dev, int enType, uint8_t *pVal);
bool MApi_PNL_TCONMAP_DumpTable(struct udevice *dev, uint8_t *pTCONTable, uint8_t *pEVATable, uint8_t u8Tcontype);
void MHal_PNL_EnableTcon(struct udevice *dev);

#endif
