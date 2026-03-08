// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <dts_parser.h>
#include <mtk_mmu.h>
#include <iniutility.h>
#include <mtk_gegop.h>
#include <mtk_jpd.h>
#include <lvgl.h>
#include <picture_bar.h>
#include <system_impl.h>

#include <common.h>
#include <malloc.h>
#include <MsDebug.h>
#include <MsUboot.h>

#include <mtk_panel.h>
#include <apiGFX.h>

#define LVGL_WIDTH     1920
#define LVGL_HEIGHT    1080

typedef struct
{
     unsigned int read_buffer;
     unsigned int read_size;
     unsigned int inter_buffer;
     unsigned int inter_size;
     unsigned int output_buffer;
     unsigned int output_size;
}jpd_setting_para;

lv_disp_drv_t disp_drv;
lv_disp_t *disp;

int lvgl_init_flag = 0;
int loader_disp_pic_len = 0;

unsigned char *loader_pic_rgb_buff = NULL;

static size_t out_buf_pa = 0;
static unsigned int out_buf_size = 0;

extern int mtk_gop_show_LvglPicture(logo_disp_config_params config, struct bootlogo_info *logo_info, unsigned int logo_output_buf, unsigned int logo_output_buf_size);

void loader_flush_panel()
{
    logo_disp_config_params logo_config;
    int ret = -1;
    struct bootlogo_info logo_info;

    UBOOT_TRACE("IN\n");

    logo_config.disp_x = 0;
    logo_config.disp_y = 0;
    logo_config.horizontal_stretch = 1;
    logo_config.vertical_stretch = 1;

    logo_info.width = LVGL_WIDTH;
    logo_info.height = LVGL_HEIGHT;
    logo_info.color_format = GFX_FMT_RGB565;

    ret = mtk_gop_show_LvglPicture(logo_config, &logo_info, out_buf_pa, out_buf_size);

    if(ret != 0)
    {
        UBOOT_ERROR("Show lvgl failure\n");
    }

    UBOOT_TRACE("OK\n");
}

void loader_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int x,y;
    lv_color_int_t color_buff;
    lv_color_t color_t_buff;

    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
            color_t_buff = *color_p;
            color_buff = *(lv_color_int_t *)&color_t_buff;
          //  loader_pic_rgb_buff[(y*LV_HOR_RES_MAX + x) * LV_COLOR_DEPTH/8 + 3] = (unsigned char)(color_buff >> 24);
          //  loader_pic_rgb_buff[(y*LV_HOR_RES_MAX + x) * LV_COLOR_DEPTH/8 + 2] = (unsigned char)(color_buff >> 16);
            loader_pic_rgb_buff[(y*LV_HOR_RES_MAX + x) * LV_COLOR_DEPTH/8 + 1] = (unsigned char)(color_buff >> 8);
            loader_pic_rgb_buff[(y*LV_HOR_RES_MAX + x) * LV_COLOR_DEPTH/8] = (unsigned char)(color_buff);
            color_p++;
        }
    }
    lv_disp_flush_ready(disp_drv);
}

static lv_color_t *buf_1 = NULL;
//static lv_color_t *buf_2 = NULL;

#define LV_N_LINE      100

char loader_lvgl_init()
{
    static lv_disp_buf_t disp_buf;

    lv_init();
    if (buf_1 == NULL)
        buf_1 = malloc(LV_HOR_RES_MAX * LV_N_LINE * 4);

    if (buf_1 == NULL /*|| buf_2 == NULL*/)
        printf("malloc disp_buff fail.\n");

    lv_disp_buf_init(&disp_buf, buf_1,/* buf_2*/ NULL, LV_HOR_RES_MAX * LV_N_LINE);

    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = loader_flush_cb;
    disp = lv_disp_drv_register(&disp_drv);

    return true;
}

char loader_picture_init()
{
    struct dts_mmap mmap;
    int ret;
    jpd_setting_para jpd_para;
    UBOOT_TRACE("IN\n");

    ret = parse_dt("/mtk_njpd", jpd_dt_parser, (void *)&jpd_para, NULL);
    if(ret != 0)
    {
        UBOOT_ERROR("Get njpd setting from dts failure\n");
        return false;
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_JPD_OUT", mmap_dt_parser, (void *)&mmap, "reg");
    if(ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: Get JPD out buffer from DTS mmap failure, use default buffer\n");
        out_buf_pa = jpd_para.output_buffer;
        out_buf_size = jpd_para.output_size;
    }
    else
    {
        out_buf_pa = mmap.address;
        out_buf_size = mmap.size;
    }

    if(out_buf_pa < MIU0_BUS_BASE)
    {
        UBOOT_ERROR("out_buf_pa:0x%lX is small than 0x%lX\n", out_buf_pa, MIU0_BUS_BASE);
        return false;
    }
    out_buf_pa = out_buf_pa - MIU0_BUS_BASE;
    loader_pic_rgb_buff = (unsigned char *)phyical_addr_to_virtual_addr(out_buf_pa);

    UBOOT_DEBUG("out_buf_pa=0x%lX, out_buf_size=0x%X, loader_pic_rgb_buff=0x%llX\n",
                out_buf_pa, out_buf_size, (unsigned long long)loader_pic_rgb_buff);
    UBOOT_TRACE("OK\n");
    return true;
}


lv_style_t par_shadow;
lv_style_t style_preload;
lv_style_t label_style;
lv_style_t label_style2;

lv_obj_t * par;
lv_obj_t * obj1;
lv_obj_t * bar1;
lv_obj_t * label1;
lv_obj_t * label2;
lv_obj_t * label3;
lv_obj_t * label4;
lv_obj_t * preload;
void lv_upgrade_init()
{
    static int flag = 0;

    UBOOT_TRACE("IN\n");

    if (flag == 0)
        flag = 1;
    else
        return;

    run_command("panel_pre_init",0);
    run_command("panel_post_init",0);
    run_command("panel_mute 0",0);
    loader_picture_init();  //- get rgb buff
    loader_lvgl_init();

    par = lv_obj_create(lv_disp_get_scr_act(NULL), NULL);
    lv_obj_set_size(par, 1920, 1080);


    obj1 = lv_obj_create(par, NULL);
    lv_obj_set_size(obj1, 1500, 600);
    lv_obj_align(obj1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style(obj1, &lv_style_pretty_color);

    lv_style_copy(&label_style, &lv_style_plain);
    label_style.text.opa = LV_OPA_70;
    lv_style_copy(&label_style2, &lv_style_plain);
    label_style2.text.font = &lv_font_roboto_28;
    label_style2.text.opa = LV_OPA_70;

    label1 = lv_label_create(obj1, NULL);
    lv_label_set_style(label1, LV_LABEL_STYLE_MAIN, &label_style);
    lv_label_set_recolor(label1, true);
    lv_label_set_text(label1, "#ffffff UPGRADING SOFTWARE#");
    lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);

    label2 = lv_label_create(obj1, NULL);
    lv_label_set_style(label2, LV_LABEL_STYLE_MAIN, &label_style);
    lv_label_set_recolor(label2, true);
    lv_label_set_text(label2, "#ffffff PLEASE DO NOT TURN OFF#");
    lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_MID, 0, 150);

    bar1 = lv_bar_create(obj1, NULL);
    lv_obj_set_size(bar1, 1000, 40);
    lv_obj_align(bar1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_anim_time(bar1, 1000);
    lv_bar_set_value(bar1, 0, LV_ANIM_ON);

    label3 = lv_label_create(obj1, NULL);
    lv_label_set_style(label3, LV_LABEL_STYLE_MAIN, &label_style);
    lv_label_set_recolor(label3, true);
    lv_label_set_text(label3, "#ffffff 0%#");
    lv_obj_align(label3, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);

    label4 = lv_label_create(obj1, NULL);
    lv_label_set_style(label4, LV_LABEL_STYLE_MAIN, &label_style2);
    lv_label_set_recolor(label4, true);
    lv_label_set_text(label4, "#ffffff #");
    lv_obj_align(label4, NULL, LV_ALIGN_CENTER, 0, 70);

    lv_style_copy(&style_preload, &lv_style_plain);
    style_preload.line.width = 12;
    style_preload.line.color = lv_color_hex3(0x258);

    style_preload.body.border.color = lv_color_hex3(0xbbb);
    style_preload.body.border.width = 12;
    style_preload.body.padding.left = 1;

    preload = lv_preload_create(obj1, NULL);
    lv_obj_set_size(preload, 100, 100);
    lv_obj_align(preload, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, -160);
//    lv_obj_set_pos(preload, 550, 270);
    lv_preload_set_style(preload, LV_PRELOAD_STYLE_MAIN, &style_preload);

    lvgl_init_flag = 1;
    lv_task_handler();
    loader_flush_panel();
    UBOOT_TRACE("OK\n");
}

void lv_animation_headle()
{
    if (lvgl_init_flag == 0)
        return ;

    lv_tick_inc(40);
    lv_task_handler();
    loader_flush_panel();
}

void lv_check_progress(int progress)
{
    char buff[32];

    if (progress > 100)
        progress = 100;

    lv_label_set_text(label4, "#ffffff Integrity checking#");
    lv_obj_align(label4, NULL, LV_ALIGN_CENTER, 0, 70);

    memset(buff, 0, sizeof(buff));
    if(snprintf(buff, sizeof(buff), "#ffffff %d%%#", progress) < 0) return;
    lv_obj_align(bar1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(bar1, progress, LV_ANIM_OFF);

    lv_label_set_text(label3, buff);
    lv_obj_align(label3, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);

   //lv_animation_headle();
}

void lv_upgrade_progress(int progress)
{
    char buff[32];

    if (progress > 100)
        progress = 100;

    lv_label_set_text(label4, "#ffffff Upgrade system#");
    lv_obj_align(label4, NULL, LV_ALIGN_CENTER, 0, 70);

    memset(buff, 0, sizeof(buff));
    if(snprintf(buff, sizeof(buff), "#ffffff %d%%#", progress) < 0) return;
    lv_obj_align(bar1, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_value(bar1, progress, LV_ANIM_OFF);

    lv_label_set_text(label3, buff);
    lv_obj_align(label3, NULL, LV_ALIGN_IN_RIGHT_MID, -100, 0);

    //lv_animation_headle();
}

void lv_upgrade_error()
{
    lv_label_set_text(label4, "#ff0000 Upgrade error, please restart system#");
    lv_obj_align(label4, NULL, LV_ALIGN_CENTER, 0, 70);
    //lv_animation_headle();
}

void lv_upgrade_complete()
{
    lv_label_set_text(label4, "#00ff00 Upgrade complete#");
    lv_obj_align(label4, NULL, LV_ALIGN_CENTER, 0, 70);
    //lv_animation_headle();
}
