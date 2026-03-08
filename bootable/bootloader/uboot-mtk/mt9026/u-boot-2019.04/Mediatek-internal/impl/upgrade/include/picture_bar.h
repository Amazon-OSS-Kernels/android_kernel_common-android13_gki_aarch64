/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _API_USB_MS_PICTURE_BAR_H_
#define _API_USB_MS_PICTURE_BAR_H_

#include <linux/types.h>

void loader_flush_panel(void);
char loader_lvgl_init(void);
char loader_picture_init(void);
void lv_upgrade_init(void);
void lv_animation_headle(void);
void lv_check_progress(int progress);
void lv_upgrade_progress(int progress);
void lv_upgrade_error(void);
void lv_upgrade_complete(void);

#endif