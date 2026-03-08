// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */

#ifndef _MTK_PNL_OUT_IF_V006_H_
#define _MTK_PNL_OUT_IF_V006_H_
void mtk_pnl_out_en_v006(struct udevice *dev, bool en);
void mtk_pnl_mute_en_v006(struct udevice *dev, bool en);
void _hfrc_setting_for_HAPS_v006(void);
void mtk_pnl_set_vby1_mft_hmirror_v006(struct udevice *dev);
void vby1_set_pn_swap_v006(uint32_t pnl_lib_version);
void mtk_swing_level_setting_v006(struct udevice *dev);
void mtk_pre_emphasis_setting_v006(struct udevice *dev);
void mtk_SSC_control_v006(struct udevice *dev);
void mtk_pnl_controlbit_init_v006(struct udevice *dev);
void mtk_out_if_hbkproch_protect_init_v006(struct udevice *dev);
void mtk_pnl_set_panel_SCDISP_Path_Sel_v006(struct udevice *dev);
void efuse_set_lpllsetting_v006(uint32_t hwVersion);
int mtk_tgen_init_v006(struct udevice *dev);
int mtk_out_if_init_v006(struct udevice *dev);
void mtk_dump_mod_efuse_v006(struct udevice *dev);
void mtk_dither_setting_v006(struct udevice *dev);


#endif
