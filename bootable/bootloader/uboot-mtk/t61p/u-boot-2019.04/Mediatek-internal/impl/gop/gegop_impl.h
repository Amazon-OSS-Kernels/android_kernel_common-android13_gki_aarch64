/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _GEGOP_IMPL_HEADER_
#define _GEGOP_IMPL_HEADER_

#include <apiGFX.h>
#define PANEL_DTS_NODE_NAME         "/video_out"
#define GOP_DTS_NODE_NAME           "/graphic_out"
#define RESOLUTION_DTS_SUBNAME_GFX      "panel_info"
#define RESOLUTION_DTS_SUBNAME_MAIN      "panel_info"
#define RESOLUTION_DTS_SUBNAME_MAIN_DLG      "panel_info_DLG"
#define GOP_VG_SEPARATE_PROP        "status"
#define GOP_VG_SEPARATE_ENABLE      "okay"
#define GOP_VG_SEPARATE_DISABLE     "disable"
#define DISPLAY_RESOLUTION_WIDTH    "resolution_width"
#define DISPLAY_RESOLUTION_HEIGHT   "resolution_height"
#define DISPLAY_HSYNC_START         "hsync_start"
#define DISPLAY_TYP_CLK             "typ_clk"
#define DISPLAY_TYP_CLK_H           "typ_clk_high"
#define DISPLAY_TYP_CLK_L           "typ_clk_low"
#define DISPLAY_TYP_H_TOTAL         "typ_h_total"
#define DISPLAY_TYP_V_TOTAL         "typ_v_total"
#define OUTPUT_FRAME_RATE_120       120
#define OUTPUT_FRAME_RATE_144       144
#define RESOLUTION_WIDTH_4K         3840
#define RESOLUTION_HEIGHT_4K        2160
#define RESOLUTION_WIDTH_8K         7680
#define RESOLUTION_HEIGHT_8K        4320
#define FB_AND_WINDOW_ID_MAX        0xFF
#define SEPERATE_NUM                2
#define BUFFER_LENGTH               32
#define BYTESPP                     2

typedef struct
{
    unsigned char   gop_setting_type;//0:mux, 1:layer
    unsigned short  gop_layer_num;
    unsigned short  *gop_layer_index;
}st_gopmux_para;

typedef enum
{
    FONT32X32 = 0,
    FONT16X16,
    FONT_MAX = 7
}font_size;

typedef enum
{
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_MIDDLE,
    TEXT_ALIGN_RIGHT

} text_attrib;

typedef struct
{
    unsigned char font_width;
    unsigned char font_height;
    unsigned char space_width;
    unsigned char interval_width;
    unsigned char *p_char_table;
    unsigned char *p_font_width_table;
}osd_font_info;


unsigned char mtk_get_logo_miu_idx(void);
void mtk_osd_create(u8 u8logo_gop_index, GFX_Block* p_blk, size_t gop_buffer);
void mtk_draw_rect(GFX_Block* p_blk, GFX_RgbColor color);
void mtk_select_font(u8 font_choose);
void mtk_draw_string_text(const char* p_str_text, u16 x, u16 y, GFX_RgbColor color, text_attrib text_attrib);
void mtk_draw_progress(u16 x, u16 y, GFX_RgbColor fcolor, u8 per);
void mtk_flush_canvas_to_screen(void);
void mtk_draw_pixel(GFX_Point p, GFX_RgbColor color);

#endif