/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _STANDBY_MODE_HEADER_
#define _STANDBY_MODE_HEADER_

#ifdef CONFIG_DATA_SEPARATION
#define STANDBY_SECTION     "Standby"
#define STANDBY_KEY         "m_pStandbyMode_File"
#endif

#if defined(CONFIG_ANDROID_CN_PLATFORM)
#define STANDBY_PARTITION   "persist"
#define STANDBY_FILE_PATH   "standby_mode.ini"
#elif defined(CONFIG_LINUX_REF_PLATFORM)
#define STANDBY_PARTITION   "persist"
#define STANDBY_FILE_PATH   "standby_mode.ini"
#else
#define STANDBY_PARTITION   "persist"
#define STANDBY_FILE_PATH   "standby/standby_mode.ini"
#endif

typedef enum
{
    STANDBY_DIRECT_MODE       = 0,
    STANDBY_MEMORY_MODE       = 1,
    STANDBY_SECOND_MODE       = 2,
    STANDBY_UNKNOW            = 0xFF,

} SECOND_STANDBY_MODE;

int standby_init(void);
int standby_mode_get_mode(void);
int standby_mode_is_enter_standby(void);
int standby_mode_get_second_standby_mode(void);
int standby_mode_enter_standby(void);

#endif
