// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_PNL_PUBLIC_HEADER_
#define _MTK_PNL_PUBLIC_HEADER_

#define ENV_SETTING_ON  "yes"
#define ENV_SETTING_ON_SIZE  3

typedef enum {
    E_PNL_PROP_TCON_EN = 0,
    E_PNL_PROP_DEMURA_SEL,
    E_PNL_PROP_VCOM_SEL,
    E_PNL_PROP_DEMURA_EN,
    E_PNL_PROP_AUTO_PGA_UPDATE,
    E_PNL_PROP_AUTO_P2P_CMD_UPDATE,
    E_PNL_PROP_MAX
} EN_PNL_PROPERTY_TYPE;

typedef enum
{
    E_SET_PNL_PROP_NONE = 0,
    E_SET_PNL_PROP_AUTO_PGA_60Hz,
    E_SET_PNL_PROP_AUTO_PGA_120Hz,
    E_SET_PNL_PROP_CSPI_CMD_UPDATE,
    E_SET_PNL_PROP_CEDS_CMD_UPDATE,
    E_SET_PNL_PROP_USIT_CMD_UPDATE,
    E_SET_PNL_PROP_MAX
} EN_SET_PNL_PROPERTY_TYPE;

bool mtk_is_panel_enable(void);
int mtk_is_panel_ready(void);
void mtk_panel_backlight_on(void);
int mtk_panel_init(void);
void mtk_panel_backlight_off(void);
bool MApi_check_is_trunk_flow(void);
bool MApi_check_is_oled(void);
bool MApi_check_is_internal_module(void);
#ifdef CONFIG_MTK_PANEL
int mtk_panel_init_device(void);
int mtk_panel_enable(bool en);
int mtk_panel_enable_backlight(bool en);
int mtk_panel_enable_vcc(bool en);
int mtk_panel_mute(bool en);
int mtk_get_panel_gpio_cusctrl(void);
#endif

#if defined(CONFIG_MULTICORES_PLATFORM)
void *panel_pre_init_thread_entry(void *);
void *panel_mute_enable_thread_entry(void *);
void *panel_mute_disable_thread_entry(void *);
void *panel_enable_thread_entry(void *);
void *panel_post_init_thread_entry(void *);
#endif
#endif
