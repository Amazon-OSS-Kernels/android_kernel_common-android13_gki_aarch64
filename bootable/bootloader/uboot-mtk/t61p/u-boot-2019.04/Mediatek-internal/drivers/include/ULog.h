/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef __ULOG_H__
#define __ULOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "utopia_dapi.h"

#if (defined CONFIG_UTOPIA_TEE || defined MBOOT || defined MSOS_TYPE_NOS || defined MSOS_TYPE_OPTEE )
#define ULOGI(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)

#elif (defined MSOS_TYPE_ECOS)

#ifdef MS_DEBUG
#define ULOGI(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#else
#define ULOGI(tag,fmt,...)      {}
#define ULOGW(tag,fmt,...)      {}
#define ULOGD(tag,fmt,...)      {}
#define ULOGE(tag,fmt,...)      {}
#define ULOGF(tag,fmt,...)      {}
#endif

#elif (defined MSOS_TYPE_LINUX_KERNEL)

#define ULOGD(tag,fmt,...)      ({if(UtopiaLogSystem(tag)==TRUE){printk(KERN_DEBUG "<UTPA_DEBUG>[Utopia][" tag "]: " fmt, ##__VA_ARGS__);}})   // loglevel 7
#define ULOGI(tag,fmt,...)      ({if(UtopiaLogSystem(tag)==TRUE){printk(KERN_INFO "<UTPA_INFO>[Utopia][" tag "]: " fmt, ##__VA_ARGS__);}})     // loglevel 6
#define ULOGW(tag,fmt,...)      ({if(UtopiaLogSystem(tag)==TRUE){printk(KERN_NOTICE "<UTPA_WARN>[Utopia][" tag "]: " fmt, ##__VA_ARGS__);}})   // loglevel 5
#define ULOGE(tag,fmt,...)      ({if(UtopiaLogSystem(tag)==TRUE){printk(KERN_WARNING "<UTPA_ERR>[Utopia][" tag "]: " fmt, ##__VA_ARGS__);}})   // loglevel 4
#define ULOGF(tag,fmt,...)      ({if(UtopiaLogSystem(tag)==TRUE){printk(KERN_ERR "<UTPA_FATAL>[Utopia][" tag "]: " fmt, ##__VA_ARGS__);}})     // loglevel 3

#elif (defined CONFIG_MLOG)
#include "MLog.h"

#define ULOGI(tag,fmt,...)      MLOGI("Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      MLOGW("Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      MLOGD("Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      MLOGE("Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      MLOGF("Utopia", "[" tag "]" fmt, ##__VA_ARGS__)

#elif (defined ANDROID)
#include <cutils/log.h>

#ifdef ALOG
#define ULOGI(tag,fmt,...)      ALOG(LOG_INFO, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      ALOG(LOG_WARN, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      ALOG(LOG_DEBUG, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      ALOG(LOG_ERROR, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      ALOG(LOG_FATAL, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#else
#define ULOGI(tag,fmt,...)      LOG(LOG_INFO, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      LOG(LOG_WARN, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      LOG(LOG_DEBUG, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      LOG(LOG_ERROR, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      LOG(LOG_FATAL, "Utopia", "[" tag "]" fmt, ##__VA_ARGS__)
#endif

#else

#define ULOGI(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGW(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGD(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGE(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)
#define ULOGF(tag,fmt,...)      printf("[Utopia][" tag "]: " fmt, ##__VA_ARGS__)

#endif

#ifdef __cplusplus
}
#endif

#endif
