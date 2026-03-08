/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _DTBO_CFG_SEL_H_
#define _DTBO_CFG_SEL_H_

#include <common.h>

#if defined(CONFIG_ANDROID_CN_PLATFORM)
#define DTBOCFG_ROOT_PATH		"config/dtb"
#else
#define DTBOCFG_ROOT_PATH		"dtb"
#endif

#define DEFAULT_DTB_CFG_LABEL   "default"

bool dtbo_cfg_sel_get_pcb(char *cfg_path, int size, u32 chipid, u32 revision);
#endif
