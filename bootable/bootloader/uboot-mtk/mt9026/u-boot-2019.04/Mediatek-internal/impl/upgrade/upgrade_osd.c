// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <stdio.h>
#include <common.h>
#include <command.h>
#include <exports.h>
#include <environment.h>
#include <MsTypes.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <usb_upgrade.h>
#include <upgrade_osd.h>
#include <upgrade_utility.h>
#include <apiGFX.h>
#include <mtk_panel.h>
#include <panel_impl.h>
#include <gegop_impl.h>
#include <dts_parser.h>
#include"osd_string_emnu.h"


//config update led Enable
#define CONFIG_GPIO_LED_ENABLE 0
#define CONFIG_PWM_LED_ENABLE  0
#define DEFAULT_STRING_LANGUAGE     "English"   //  English , Chinese , Russian

#define GWIN_WIDTH              720
#define GWIN_HEIGHT             576
#define GRAPHIC_WIDTH           600
#define GRAPHIC_HEIGHT          400
#define GRAPHIC_X               60
#define GRAPHIC_Y               88
#define LINE_HEIGHT             50
#define RECT_LEFT_INTERVAL      50
#if ENABLE_DISPLAY_PICTURE_UPDATE
#define GWIN_PIC_WIDTH              1280
#define GWIN_PIC_HEIGHT             720
#define GRAPHIC_PIC_WIDTH           1280
#define GRAPHIC_PIC_HEIGHT          720
#define GRAPHIC_PIC_X               0
#define GRAPHIC_PIC_Y               0
#define EN_PIC_BASE_PATH            "updating_en_base.jpg"
#define EN_PIC_ERROR_PATH           "updating_en_unsuccessful.jpg"
#define EN_PIC_PROGRESS_X           560
#define EN_PIC_PROGRESS_Y           220
#define EN_PIC_PROGRESS_WIDTH       80
#define EN_PIC_PROGRESS_HEIGHT      60
#define EN_PIC_ERROR_X              570
#define EN_PIC_ERROR_Y              200
#define EN_PIC_ERROR_WIDTH          200
#define EN_PIC_ERROR_HEIGHT         112
#endif


#if (CONFIG_UPGRADE_OSD == 1)
static show_error_cb            fp_show_error = NULL;
static show_finish_cb           fp_show_finish = NULL;
static show_load_data_cb        fp_show_load_data = NULL;
static show_start_upgrading_cb  fp_show_start_upgrading = NULL;
static show_upgrading_cb        fp_show_upgrading = NULL;

int display_osd = 0;
static unsigned int progress_cnt = 0;
static unsigned int progress_start_load_value = 10;
static unsigned int progress_finish_load_value = 200;
static const unsigned int progress_max_value = 1000;
#endif

#if (ENABLE_DISPLAY_PICTURE_UPDATE)
static unsigned int progress_cnt_last = 0;
#endif


#if (CONFIG_UPGRADE_OSD == 1)
void register_cb_show_error(show_error_cb cb)
{
    UBOOT_TRACE("IN\n");
    fp_show_error=cb;
    UBOOT_TRACE("OK\n");
}

void register_cb_show_finish(show_finish_cb cb)
{
    UBOOT_TRACE("IN\n");
    fp_show_finish=cb;
    UBOOT_TRACE("OK\n");
}

void register_cb_show_load_data(show_load_data_cb cb)
{
    UBOOT_TRACE("IN\n");
    fp_show_load_data=cb;
    UBOOT_TRACE("OK\n");
}

void register_cb_show_start_upgrading(show_start_upgrading_cb cb)
{
    UBOOT_TRACE("IN\n");
    fp_show_start_upgrading=cb;
    UBOOT_TRACE("OK\n");
}
void register_cb_show_upgrading(show_upgrading_cb cb)
{
    UBOOT_TRACE("IN\n");
    fp_show_upgrading=cb;
    UBOOT_TRACE("OK\n");
}

void un_register_show_cb(void)
{
    UBOOT_TRACE("IN\n");
    fp_show_error=NULL;
    fp_show_finish=NULL;
    fp_show_load_data=NULL;
    fp_show_start_upgrading=NULL;
    fp_show_upgrading=NULL;
    UBOOT_TRACE("OK\n");
}

#if (CONFIG_LVGL_UPGRADE_BAR == 1)
#include <MsOS.h>

void drvTime_Handler(void)
{
    lv_animation_headle();
}

void lvgl_CreateTimer(void)
{
    UBOOT_TRACE("IN\n");

    extern void timer_irq_register(void *ptCb);
    extern void cpu_interrupt_setting(void);
    extern void timer_period_register(unsigned int timeout_ms);

    timer_irq_register(drvTime_Handler);
    timer_period_register(100);
    cpu_interrupt_setting();

    UBOOT_TRACE("OK\n");
}


static int do_lvgl_demo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    lv_upgrade_init();
    lvgl_CreateTimer();
    return 0;
}

U_BOOT_CMD(
          lvgl_demo, CONFIG_SYS_MAXARGS, 1,    do_lvgl_demo,
          "lvgl_demo   - lvgl function test.\n",
          NULL
          );


int show_lvgl_LoadData(int var)
{
    static int register_timer_flag = 0;

    UBOOT_TRACE("IN\n");
    if (register_timer_flag == 0)
    {
        lv_upgrade_init();
        lvgl_CreateTimer();
        register_timer_flag = 1;
    }
    lv_check_progress(var);
    UBOOT_TRACE("OK\n");

    return 0;
}

int show_lvgl_StartUpgrading(int var)
{
    UBOOT_TRACE("IN\n");
    lv_upgrade_init();
    UBOOT_TRACE("OK\n");
    return 0;
}

int show_lvgl_Upgrading(int current_cnt, int total_cnt)
{
    UBOOT_TRACE("IN\n");
    progress_cnt = (((progress_max_value - progress_finish_load_value)*1000)/total_cnt); //(1000-200)/cmd_cnt
    progress_cnt = (progress_cnt *current_cnt+progress_finish_load_value*1000)/(10*1000);
    printf("--%d %%\n", progress_cnt);

    lv_upgrade_progress(progress_cnt);
    UBOOT_TRACE("OK\n");
    return 0;
}

int show_lvgl_Error(int var)
{
    lv_upgrade_error();
    return 0;
}

int show_lvgl_Finish(int var)
{
    lv_upgrade_complete();
    return 0;
}

#endif
int show_load_data(int var)
{
    if (-1 == display_osd)
        return -1;
    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    int ret = 0;
    UBOOT_TRACE("IN\n");

    if (1 == CONFIG_GPIO_LED_ENABLE)
    {
        char *p_str = NULL;
        if ((p_str = env_get("usb_upgradeing_flicker")) != NULL)
        {
            UBOOT_DEBUG("usb_upgradeing_flicker=%s\n", p_str);
            memset(buffer, 0, CMD_BUF);
            snprintf_len = snprintf(buffer, CMD_BUF, "led time %s", p_str);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            run_command(buffer, 0); //GPIO led Frequency
        }
        else
        {
            memset(buffer, 0, CMD_BUF);
            snprintf_len = snprintf(buffer, CMD_BUF, "led time 0xFA");
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            run_command(buffer, 0); //GPIO led Frequency
        }
    }
    else if (1 == CONFIG_PWM_LED_ENABLE)
    {
        memset(buffer, 0, CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "pwm 1 0 600000000 300000000 normal");  // PWM period=600000000 ns, PWM duty=300000000
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
    }
    progress_cnt = 0;
    progress_start_load_value = 10;
    progress_finish_load_value = 200;
    memset(buffer, 0, CMD_BUF);
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_create %d %d", GWIN_PIC_WIDTH, GWIN_PIC_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
#else
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_create %d %d", GWIN_WIDTH, GWIN_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
#endif
    UBOOT_DEBUG("cmd=%s\n", buffer);
    ret = run_command(buffer, 0);
    if (ret != 0)
    {
        UBOOT_DEBUG("osd_create fail, disable OSD\n");
        display_osd = -1;
        return -1;
    }
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_jpg -fs %d %d %d %d %s",GRAPHIC_PIC_X, GRAPHIC_PIC_Y, GRAPHIC_PIC_WIDTH, GRAPHIC_PIC_HEIGHT, EN_PIC_BASE_PATH);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#else
        memset(buffer, 0 , CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
        memset(buffer, 0 , CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 %x", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2,en_load_data);
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
        memset(buffer, 0 , CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL, GRAPHIC_Y + LINE_HEIGHT * 5, progress_start_load_value/10);
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
#endif
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_flush");
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    UBOOT_TRACE("OK\n");
    return 0;
}

int show_start_upgrading(int var)
{
    if (-1 == display_osd)
        return -1;
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    Spinner_Timer_ISR_Register();
#else
    char buffer[CMD_BUF] = "\0";
    int snprintf_len;

    UBOOT_TRACE("IN\n");

    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 %x", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2,en_upgrading_software);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 %x", GRAPHIC_X,	GRAPHIC_Y + LINE_HEIGHT * 3,en_not_turn_off);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL, GRAPHIC_Y + LINE_HEIGHT * 5, progress_finish_load_value/10);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_flush");
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#endif
    UBOOT_TRACE("OK\n");
    return 0;
}

int show_upgrading(int current_cnt, int total_cnt)
{
    if (-1 == display_osd)
        return -1;

    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    UBOOT_TRACE("IN\n");
    progress_cnt = (((progress_max_value - progress_finish_load_value)*1000)/total_cnt); //(1000-200)/cmd_cnt
    progress_cnt = (progress_cnt *current_cnt+progress_finish_load_value*1000)/(10*1000);
    printf("--%u %%\n", progress_cnt);
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    if (progress_cnt_last == progress_cnt)
    {
        return 0;
    }
    progress_cnt_last = progress_cnt;
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_jpg -fs %d %d %d %d updating_%d.jpg", EN_PIC_PROGRESS_X, EN_PIC_PROGRESS_Y, EN_PIC_PROGRESS_WIDTH, EN_PIC_PROGRESS_HEIGHT, progress_cnt);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#else
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 5, GRAPHIC_WIDTH, LINE_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL,GRAPHIC_Y + LINE_HEIGHT * 5, progress_cnt);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#endif
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_flush");
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    UBOOT_TRACE("OK\n");

    return 0;
 }

int show_error(int var)
{
    if (-1 == display_osd)
        return -1;
    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    UBOOT_TRACE("IN\n");
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    stop_Spinner_Timer_ISR();
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_jpg -fs %d %d %d %d %s", EN_PIC_ERROR_X, EN_PIC_ERROR_Y, EN_PIC_ERROR_WIDTH, EN_PIC_ERROR_HEIGHT, EN_PIC_ERROR_PATH);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#else
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 %x", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 3,en_upgrade_error);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#endif
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_flush");
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    if (1 == CONFIG_GPIO_LED_ENABLE)
    {
        memset(buffer, 0 , CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "led stop");
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
    }
    UBOOT_TRACE("OK\n");

    char *p_str = env_get("usb_upgrade_auto_reset");
    int usb_upgrade_auto_reset = 1;
    if (p_str != NULL)
    {
        usb_upgrade_auto_reset = (int)simple_strtol(p_str, NULL, 10);
        UBOOT_DEBUG("usb_upgrade_auto_reset=%d\n", usb_upgrade_auto_reset);
    }
    if (usb_upgrade_auto_reset == 0)
    {
        if (1 == CONFIG_GPIO_LED_ENABLE)
        {
            if ((p_str = env_get("usb_upgradeerror_flicker")) != NULL)
            {
                UBOOT_DEBUG("usb_upgradeerror_flicker=%s\n", p_str);
                memset(buffer, 0 , CMD_BUF);
                snprintf_len = snprintf(buffer, CMD_BUF, "led time %s", p_str);
                if (snprintf_len >= CMD_BUF)
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                    return -1;
                }
                UBOOT_DEBUG("cmd=%s\n", buffer);
                run_command(buffer, 0); //GPIO led Frequency
            }
            else
            {
                memset(buffer, 0, CMD_BUF);
                snprintf_len = snprintf(buffer, CMD_BUF, "led time 0xFA");
                if (snprintf_len >= CMD_BUF)
                {
                    UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                    return -1;
                }
                UBOOT_DEBUG("cmd=%s\n", buffer);
                run_command(buffer, 0); //GPIO led Frequency
            }
        }
        jump_to_console();
    }
    return 0;
}

int show_finish(int var)
{
    if (-1 == display_osd)
        return -1;

    char buffer[CMD_BUF] = "\0";
    int snprintf_len;
    UBOOT_TRACE("IN\n");
#if (ENABLE_DISPLAY_PICTURE_UPDATE)
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_jpg -fs %d %d %d %d updating_%d.jpg", EN_PIC_PROGRESS_X, EN_PIC_PROGRESS_Y, EN_PIC_PROGRESS_WIDTH, EN_PIC_PROGRESS_HEIGHT, progress_cnt);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#else
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_rect %d %d %d %d 0x800000ff", GRAPHIC_X, GRAPHIC_Y, GRAPHIC_WIDTH, GRAPHIC_HEIGHT);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_string %d %d 0x3fffffff 1 %x", GRAPHIC_X, GRAPHIC_Y + LINE_HEIGHT * 2,en_upgrade_complete);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "draw_progress %d %d 0x3fffffff %d", GRAPHIC_X + RECT_LEFT_INTERVAL, GRAPHIC_Y + LINE_HEIGHT * 5, 100);
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
#endif
    memset(buffer, 0 , CMD_BUF);
    snprintf_len = snprintf(buffer, CMD_BUF, "osd_flush");
    if (snprintf_len >= CMD_BUF)
    {
        UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
        return -1;
    }
    UBOOT_DEBUG("cmd=%s\n", buffer);
    run_command(buffer, 0);
    udelay(2000000);

    if (1 == CONFIG_GPIO_LED_ENABLE)
    {
        memset(buffer, 0 , CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "led stop");
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);

        char *p_str = env_get ("usb_upgradefinish_flicker");
        if (p_str != NULL)
        {
            UBOOT_DEBUG("usb_upgradefinish_flicker=%s\n", p_str);
            memset(buffer, 0 , CMD_BUF);
            snprintf_len = snprintf(buffer, CMD_BUF, "led time %s", p_str);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            run_command(buffer, 0); //GPIO led Frequency
        }
        else
        {
            memset(buffer, 0, CMD_BUF);
            snprintf_len = snprintf(buffer, CMD_BUF, "led time 0xFA");
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
                return -1;
            }
            UBOOT_DEBUG("cmd=%s\n", buffer);
            run_command(buffer, 0); //GPIO led Frequency
        }
    }
    else if (1 == CONFIG_PWM_LED_ENABLE)
    {
        memset(buffer, 0, CMD_BUF);
        snprintf_len = snprintf(buffer, CMD_BUF, "pwm 1 0 0 0 normal");
        if (snprintf_len >= CMD_BUF)
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, buffer);
            return -1;
        }
        UBOOT_DEBUG("cmd=%s\n", buffer);
        run_command(buffer, 0);
    }

    UBOOT_TRACE("OK\n");
    return 0;
}

unsigned char check_osd_active(void)
{
    #define OSD_ACTIVE "osd_active"
    if ((env_get(OSD_ACTIVE)!=NULL) && (strcmp(env_get(OSD_ACTIVE),"off")==0))
    {
        env_set(OSD_ACTIVE,NULL);
        env_save();
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int do_create_osdlayer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    size_t gop_buffer = 0;
    u8 u8logo_gop_index = 0;
    int ret;
    struct dts_mmap mmap;

    if (argc < 3)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }
    u16 w = simple_strtoul(argv[1], NULL, 10);
    u16 h = simple_strtoul(argv[2], NULL, 10);

    GFX_Block dst_blk = {0, 0, w, h};
    u32 size = w * h;

    if (0 == size)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    if (mtk_is_panel_ready() != 1)
    {
        if (run_command("panel_pre_init",0) != 0)
        {
            UBOOT_ERROR("Error : panel_pre_init fail!!\n");
            return -1;
        }
#ifdef CONFIG_MTK_PANEL
        if (run_command("panel_enable 1",0) != 0)
        {
            UBOOT_ERROR("Error : panel_enable 1 fail!!\n");
            return -1;
        }
#endif
    }

#if (ENABLE_HDMI_TX == 1)
    if (run_command("hdmi init",0) != 0)
    {
        UBOOT_ERROR("Error : hdmi init fail!!\n");
        return -1;
    }

#endif

#if (ENABLE_OSDC_Init==1)
    run_command("xc_init" , 0);
#endif

#if (ENABLE_CVBS_OUT == 1)
    run_command("cvbs",0);
#endif

    // set panel backlight ON
    if (mtk_is_panel_ready() == 1 && check_osd_active() == TRUE)
    {
        if (run_command("panel_post_init",0) != 0)
        {
            UBOOT_ERROR("Error : panel_post_init fail!!\n");
            return -1;
        }

#ifdef CONFIG_MTK_PANEL
        if (run_command("panel_mute 0", 0) != 0)
        {
            UBOOT_ERROR("Error : panel_mute fail!!\n");
            return -1;
        }
#endif

        #if (CONFIG_LOCAL_DIMMING)
        run_command("local_dimming", 0);
        #endif
    }

    memset(&mmap, 0, sizeof(mmap));
    ret = parse_dt("/mmap_info/MI_BOOTLOGO_BUF", mmap_dt_parser, (void*)&mmap, "reg");
    if (ret < 0 || mmap.address == 0)
    {
        UBOOT_ERROR("Error: GOP information parse error in DTS\n");
        return -1;
    }
    if (mmap.address > 0xFFFFFFFF)
    {
        UBOOT_ERROR("Error: GE/GOP driver not support > 4G mmap address\n");
        return -1;
    }
    gop_buffer = mmap.address;
    UBOOT_DEBUG("dst_info.u32Addr=graphic_bootlogo_buf_addr=0x%lx\n", gop_buffer);

    UBOOT_DEBUG("BOOTLOGO SET GOP MIU[%u]\n", (unsigned int)mtk_get_logo_miu_idx());

    // initilize the GOP of Subtitle instead of UI;
    // otherwise, then bootlogo will be closed unexpectedly,
    // because UI is initilalized during booting up
    char* p_str = env_get("bootlogo_gopidx");
    if (p_str == NULL)
    {
        UBOOT_DEBUG("Can not find bootlogo_gopidx in enviroment\n");
        ret = parse_dt("/mediatek-drm-tv-kms", integer_dt_parser, (void*)&u8logo_gop_index, "GRAPHIC_BOOTLOGO_GOPIDX");
        if (ret < 0)
        {
            UBOOT_ERROR("Error: GOP information parse error in DTS\n");
            return -1;
        }
    }
    else
    {
        u8logo_gop_index = simple_strtoul(p_str, NULL, 10);
        UBOOT_DEBUG("bootlogo_gopidx in enviroment is %d\n", (int)u8logo_gop_index);
    }
    UBOOT_DEBUG("GOP Boot Logo Index %d \n", (int)u8logo_gop_index);

    mtk_osd_create(u8logo_gop_index, &dst_blk, gop_buffer);
    return 0;
}

int do_draw_rect(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 6)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }
    u16 x = simple_strtoul(argv[1], NULL, 10);
    u16 y = simple_strtoul(argv[2], NULL, 10);
    u16 w = simple_strtoul(argv[3], NULL, 10);
    u16 h = simple_strtoul(argv[4], NULL, 10);
    u32 t_color = simple_strtoul(argv[5], NULL, 16);

    GFX_RgbColor color;
    color.a = t_color >> 24;
    color.r = (t_color >> 16) & 0xFF;
    color.g = (t_color >> 8) & 0xFF;
    color.b = t_color & 0xFF;
    GFX_Block dst_blk = {x, y, w, h};

    mtk_draw_rect(&dst_blk, color);
    return 0;
}

int do_draw_string(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u8 font;
    u16 x, y, attrib;
    u32 t_color;
    GFX_RgbColor color;
    int snprintf_len;

    if (argc < 6)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    /* Set default language is English */
    if (env_get("osd_language") == NULL)
    {
        UBOOT_DEBUG("Can not get osd_language in environment!\n");
        UBOOT_DEBUG("Set default osd_language %s\n", DEFAULT_STRING_LANGUAGE);
        env_set("osd_language", DEFAULT_STRING_LANGUAGE);
        env_save();
    }

    x = simple_strtoul(argv[1], NULL, 10);
    y = simple_strtoul(argv[2], NULL, 10);
    t_color = simple_strtoul(argv[3], NULL, 16);
    attrib = simple_strtoul(argv[4], NULL, 10)%10;
    font = simple_strtoul(argv[4], NULL, 10)/10;

    color.a = t_color >> 24;
    color.r = (t_color >> 16) & 0xFF;
    color.g = (t_color >> 8) & 0xFF;
    color.b = t_color & 0xFF;


    mtk_select_font(font);
    char c_str[202] = {0};
    u16 u16_total = 0;
    MS_S16 i = 5;

    for (; i < argc; ++i)
    {
        u16 u16Len = strlen(argv[i]);
        if (u16_total + u16Len > 200)
        {
            break;
        }

        memset(c_str + u16_total, 0 , (sizeof(c_str)-u16_total));
        snprintf_len = snprintf(c_str + u16_total, (sizeof(c_str)-u16_total), "%s", argv[i]);
        if (snprintf_len >= (sizeof(c_str)-u16_total))
        {
            UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", (int)(sizeof(c_str)-u16_total), c_str + u16_total);
            return -1;
        }
        u16_total += u16Len;
        c_str[u16_total++] = ' ';
    }

    c_str[u16_total] = 0;

    mtk_draw_string_text(c_str, x, y, color, (text_attrib)attrib);
    return 0;
}

int do_draw_progress(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 5)
    {
        printf("Usage:\n%s\n", cmdtp->usage);
        return 1;
    }

    u16 x = simple_strtoul(argv[1], NULL, 10);
    u16 y = simple_strtoul(argv[2], NULL, 10);
    u8 per = simple_strtoul(argv[4], NULL, 10);
    u32 t_color = simple_strtoul(argv[3], NULL, 16);

    GFX_RgbColor color;
    color.a = t_color >> 24;
    color.r = (t_color >> 16) & 0xFF;
    color.g = (t_color >> 8) & 0xFF;
    color.b = t_color & 0xFF;
    mtk_draw_progress(x, y, color, per);
    return 0;
}

int do_flush2screen_osdlayer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    mtk_flush_canvas_to_screen();
    return 0;
}

#endif

int show_error_osd(unsigned int var)
{
#if (CONFIG_UPGRADE_OSD == 1)
    UBOOT_TRACE("IN\n");
    if (fp_show_error != NULL)
    {
        UBOOT_TRACE("OK\n");
        return fp_show_error(var);
    }
    else
    {
        UBOOT_DEBUG("No call back function for 'error' phase\n");
        UBOOT_TRACE("OK\n");
        return 0;
    }
#else
    return 0;
#endif
}

int show_finish_osd(unsigned int var)
{
#if (CONFIG_UPGRADE_OSD == 1)
    UBOOT_TRACE("IN\n");
    if (fp_show_finish != NULL)
    {
        UBOOT_TRACE("OK\n");
        return fp_show_finish(var);
    }
    else
    {
        UBOOT_DEBUG("No call back function for 'finish' phase\n");
        UBOOT_TRACE("OK\n");
        return 0;
    }
#else
    return 0;
#endif
}

int show_load_data_osd(unsigned int var)
{
#if (CONFIG_UPGRADE_OSD == 1)
    UBOOT_TRACE("IN\n");
    if (fp_show_load_data != NULL)
    {
        UBOOT_TRACE("OK\n");
        return fp_show_load_data(var);
    }
    else
    {
        UBOOT_DEBUG("No call back function for 'LoadData' phase\n");
        UBOOT_TRACE("OK\n");
        return 0;
    }
#else
    return 0;
#endif
}
int show_start_upgrading_osd(unsigned int var)
{
#if (CONFIG_UPGRADE_OSD == 1)
    UBOOT_TRACE("IN\n");
    if (fp_show_start_upgrading != NULL)
    {
        UBOOT_TRACE("OK\n");
        return fp_show_start_upgrading(var);
    }
    else
    {
        UBOOT_DEBUG("No call back function for 'StartUpgrading' phase\n");
        UBOOT_TRACE("OK\n");
        return 0;
    }
#else
    return 0;
#endif
}

int show_upgrading_osd(unsigned int var1, unsigned int var2)
{
    int ret = 0;

#if (CONFIG_UPGRADE_OSD == 1)
    UBOOT_TRACE("IN\n");
    if (fp_show_upgrading != NULL)
    {
        char filesize[CMD_BUF] = "\0";
        char *env = NULL;
        int snprintf_len;
        memset(filesize, 0 , CMD_BUF);
        env = env_get("filesize");
        if (env)
        {
            snprintf_len = snprintf(filesize, CMD_BUF, "%s", env);
            if (snprintf_len >= CMD_BUF)
            {
                UBOOT_ERROR("The array size is too small(%d), snprintf fail '%s'\n", CMD_BUF, filesize);
                return -1;
            }
        }
        ret = fp_show_upgrading(var1, var2);
        env_set("filesize", filesize);
        UBOOT_TRACE("OK\n");
    }
    else
    {
        UBOOT_DEBUG("No call back function for 'Upgrading' phase\n");
        UBOOT_TRACE("OK\n");
    }
#endif
    return ret;
}

