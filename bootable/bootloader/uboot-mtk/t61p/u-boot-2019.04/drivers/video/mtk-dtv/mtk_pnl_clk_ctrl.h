// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * MediaTek Panel driver
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author Yu-Jen.Huang <yu-jen.huang@mediatek.com>
 */
#ifndef _MTK_PNL_CLK_CTRL_H_
#define _MTK_PNL_CLK_CTRL_H_

int mtk_analog_setting(struct udevice *dev);
int mtk_ext_video_analog_setting(struct udevice *dev);
int mtk_gfx_analog_setting(struct udevice *dev);
int mtk_pll_powerdown(struct udevice *dev,bool enable);
int mtk_mpll_dump_clk_tbl(struct udevice *dev);
int mtk_moda_dump_clk_tbl(struct udevice *dev);
int mtk_pll_dump_clk_tbl(struct udevice *dev);
int mtk_pnl_xtal_lpll_ckg_setting(struct udevice *dev);
int mtk_pnl_video_ckg_setting(struct udevice *dev);
int mtk_out_clk_init(struct udevice *dev);
int mtk_disp_odclk_init(struct udevice *dev);
int mtk_ext_video_clk_setting(struct udevice *dev);
int mtk_gfx_clk_setting(struct udevice *dev);
void analog_set_drv_mode(struct udevice *dev);
void analog_ext_video_set_drv_mode(struct udevice *dev);
void analog_gfx_set_drv_mode(struct udevice *dev);

#endif
