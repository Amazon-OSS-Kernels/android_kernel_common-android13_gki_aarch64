/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_STANDBY_H_
#define _MTK_STANDBY_H_

/**
 * main chip powner down control API
 */
int mtk_main_chip_power_standby(void);
#if defined(CONFIG_MULTICORES_PLATFORM)
void *standby_mode_thread_entry(void *args);
#endif
#endif
