// SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause)
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#include <common.h>
#include <system_impl.h>
#include <debug_impl.h>
#include <smp/thread_info.h>
#include <smp/thread.h>
#include <mtk-pm.h>
#include <mtk_panel.h>
#include <mtk_bootmusic.h>
#include <mtk_standby.h>
#include <pmu_thread.h>
#include <cm4_thread.h>
#if defined(CONFIG_MTK_LED_SETTING)
#include <led_thread.h>
#endif

int panel_priority = THREAD_MAX_PRIORITY;
int pmu_priority = THREAD_MAX_PRIORITY;
#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4)
thread_t *run_cm4_thread_id = NULL;
#endif
#if defined(CONFIG_MTK_PMU)
thread_t *boot_pmu_thread_id = NULL;
#endif
thread_t *panel_pre_init_thread_id = NULL;
thread_t *panel_mute_true_thread_id = NULL;
thread_t *panel_panel_enable_thread_id = NULL;
thread_t *panel_mute_false_thread_id = NULL;
thread_t *panel_post_init_thread_id = NULL;
#if defined(CONFIG_SOUND)
thread_t *music_init_thread_id = NULL;
thread_t *music_play_thread_id = NULL;
#endif
#if defined(CONFIG_SECOND_STANDBY)
thread_t *standby_mode_thread_id = NULL;
#endif
#if defined(CONFIG_MTK_LED_SETTING)
thread_t *led_setting_thread_id = NULL;
#endif


int do_after_uboot_init_task_create(void)
{
    return 0;
}

int do_before_console_input_task_create(void)
{
    panel_pre_init_thread_id = thread_create_ex("p_pre_init", panel_pre_init_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (panel_pre_init_thread_id == NULL )
    {
        UBOOT_DEBUG("panel_pre_init thread create fail...\n");
    }

#if defined(CONFIG_REMOTEPROC_MTK_VAD_CORTEX_M4)
    pmu_priority--;
    run_cm4_thread_id = thread_create_ex("boot_vad", run_cm4_thread_entry, NULL, 0, MUTLI_CORE_CPU_THREE, pmu_priority, 1);
    if (run_cm4_thread_id == NULL )
    {
        UBOOT_DEBUG("boot_vad thread create fail...\n");
    }
#endif

#if defined(CONFIG_SOUND)
    music_init_thread_id = thread_create_ex("music_init", boot_music_init_thread_entry, NULL, 0, MUTLI_CORE_CPU_ONE, THREAD_DEFAULT_PRIORITY, 1);
    if (music_init_thread_id == NULL )
    {
        UBOOT_DEBUG("music_init thread create fail...\n");
    }
#endif

    panel_priority--;
    panel_mute_true_thread_id = thread_create_ex("p_mute_on", panel_mute_enable_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (panel_mute_true_thread_id == NULL )
    {
        UBOOT_DEBUG("panel_mute thread create fail...\n");
    }

    panel_priority--;
    panel_panel_enable_thread_id = thread_create_ex("p_enable", panel_enable_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (panel_panel_enable_thread_id == NULL )
    {
        UBOOT_DEBUG("panel_enable thread create fail...\n");
    }

#if defined(CONFIG_MTK_PMU)
    pmu_priority--;
    boot_pmu_thread_id = thread_create_ex("boot_pmu", boot_pmu_thread_entry, NULL, 0, MUTLI_CORE_CPU_THREE, pmu_priority, 1);
    if (boot_pmu_thread_id == NULL )
    {
        UBOOT_DEBUG("boot_pmu thread create fail...\n");
    }
#endif

#if defined(CONFIG_MTK_LED_SETTING)
    pmu_priority--;
    led_setting_thread_id = thread_create_ex("led_setting", led_setting_thread_entry, NULL, 0, MUTLI_CORE_CPU_THREE, pmu_priority, 1);
    if (led_setting_thread_id == NULL )
    {
        UBOOT_DEBUG("led_setting thread create fail...\n");
    }
#endif

    return 0;
}

int do_after_console_input_task_create(void)
{
#if defined(CONFIG_SOUND)
    music_play_thread_id = thread_create_ex("music_play", boot_music_play_thread_entry, NULL, 0, MUTLI_CORE_CPU_ONE, THREAD_DEFAULT_PRIORITY, 1);
    if (music_play_thread_id == NULL )
    {
        UBOOT_DEBUG("music_play thread create fail...\n");
    }
#endif
#ifdef CONFIG_MTK_PANEL
    panel_priority--;
    panel_mute_false_thread_id = thread_create_ex("p_mute_off", panel_mute_disable_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (panel_mute_false_thread_id == NULL )
    {
        UBOOT_DEBUG("panel_mute thread create fail...\n");
    }
#endif
    panel_priority--;
    panel_post_init_thread_id = thread_create_ex("p_post_init", panel_post_init_thread_entry, NULL, 0, MUTLI_CORE_CPU_TWO, panel_priority, 1);
    if (panel_post_init_thread_id == NULL )
    {
        UBOOT_DEBUG("panel_mute thread create fail...\n");
    }

    return 0;
}

int do_before_boot_kernel_task_create(void)
{
#if 0 // Move secondary standy to before console and out of thread to avoid race condition
#if defined(CONFIG_SECOND_STANDBY)
    pmu_priority--;
    standby_mode_thread_id = thread_create_ex("standby", standby_mode_thread_entry, NULL, 0, MUTLI_CORE_CPU_THREE, pmu_priority, 1);
    if (standby_mode_thread_id == NULL )
    {
        UBOOT_DEBUG("standby thread create fail...\n");
    }
#endif
#endif
    return 0;
}
