// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */
#ifndef _MTK_PNL_CLK_CTRL_V006_H_
#define _MTK_PNL_CLK_CTRL_V006_H_

int mtk_pll_powerdown_v006(struct udevice *dev, bool enable);
int mtk_pnl_xtal_lpll_ckg_setting_v006(struct udevice *dev);
int mtk_disp_odclk_init_v006(struct udevice *dev);
int mtk_analog_setting_v006(struct udevice *dev);
int mtk_pll_dump_clk_tbl_v006(struct udevice *dev);
int mtk_moda_dump_clk_tbl_v006(struct udevice *dev);
int mtk_out_clk_init_v006(struct udevice *dev);
int mtk_pnl_video_ckg_setting_v006(struct udevice *dev);

#endif
