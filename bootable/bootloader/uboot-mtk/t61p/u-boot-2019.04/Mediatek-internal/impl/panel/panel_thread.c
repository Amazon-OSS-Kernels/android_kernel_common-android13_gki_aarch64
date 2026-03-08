// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <command.h>
#include <debug_impl.h>
#include <smp/thread_info.h>
#include <smp/thread.h>

extern thread_t *panel_pre_init_thread_id;
extern thread_t *panel_mute_true_thread_id;
extern thread_t *panel_panel_enable_thread_id;
extern thread_t *panel_mute_false_thread_id;
extern thread_t *panel_post_init_thread_id;
extern thread_t *show_logo_thread_id;

void *panel_pre_init_thread_entry(void *args)
{
    int ret = -1;

    ret = run_command("panel_pre_init", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("panel_pre_init command task execute failure.\n");
    }

    return NULL;
}

void *panel_mute_enable_thread_entry(void *args)
{
    int ret = -1;

    if (panel_pre_init_thread_id)
    {
        thread_join(panel_pre_init_thread_id, NULL);
    }

    ret = run_command("panel_mute 1", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("panel_mute command task execute failure.\n");
    }

    return NULL;
}

void *panel_enable_thread_entry(void *args)
{
    int ret = -1;

    if (panel_mute_true_thread_id)
    {
        thread_join(panel_mute_true_thread_id, NULL);
    }

    ret = run_command("panel_enable 1", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("panel_enable command task execute failure.\n");
    }

    return NULL;
}

void *panel_mute_disable_thread_entry(void *args)
{
    int ret = -1;

    if (show_logo_thread_id)
    {
        thread_join(show_logo_thread_id, NULL);
    }

    ret = run_command("panel_mute 0", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("panel_mute command task execute failure.\n");
    }

    return NULL;
}

void *panel_post_init_thread_entry(void *args)
{
    int ret = -1;

    if (panel_mute_false_thread_id)
    {
        thread_join(panel_mute_false_thread_id, NULL);
    }

    ret = run_command("panel_post_init", 0);
    if (ret < 0)
    {
        UBOOT_ERROR("panel_post_init command task execute failure.\n");
    }

    return NULL;
}
