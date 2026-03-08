/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MTK_BOOTMUSIC_PUBLIC_H_
#define _MTK_BOOTMUSIC_PUBLIC_H_

#if defined(CONFIG_MULTICORES_PLATFORM)
void *boot_music_init_thread_entry(void *args);
void *boot_music_play_thread_entry(void *args);
#endif
#endif
