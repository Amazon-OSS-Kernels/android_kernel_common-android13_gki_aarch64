/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _MTK_PM_H_
#define _MTK_PM_H_

/* Boot reason. */
#define PM_BR_AC			(0x00)
#define PM_BR_STR_STANDBY	(0x01)
#define PM_BR_STR_SLEEP		(0x02)
#define PM_BR_STR_DSLEEP	(0x03)
#define PM_BR_MAX_CNT		(0x04)
#define PM_BR_REBOOT		(0xD1)
#define PM_BR_REBOOT_SHELL	(0xD2)
#define PM_BR_REBOOT_FORCE	(0xDF)
#define PM_BR_WATCHDOG		(0xE0)
#define PM_BR_WATCHDOG_FORCE (0xE1)
#define PM_BR_PANIC			(0xEE)
#define PM_BR_PANIC_FORCE	(0xEF)
#define PM_BR_SECONDARY		(0xF1)
#define PM_BR_DC			(0xFF)

/* Customized boot reason start from 0XA0 ~ 0XAF */
#define PM_BR_LONG_PRESS_PWR_KEY	(0xA0)
#define PM_BR_REBOOT_OVER_TEMP	(0xA1)
#define PM_BR_SW_WATCHDOG   (0xA2)
#define PM_BR_RECOVERY_QUIESCENT  (0xA3)

/* Trigger DM probe. */
int32_t pm_init(void);
/* Boot reason record in RIU. */
int32_t pm_get_boot_reason(void);
int32_t pm_set_boot_reason(uint8_t reason);
/* Wakeup reason record in RIU. */
char *pm_get_wakeup_reason_str(void);
int32_t pm_get_wakeup_reason(void);
int32_t pm_set_wakeup_reason(const char *name);
/* Wakeup key record in RIU. */
int32_t pm_get_wakeup_key(void);
int32_t pm_set_wakeup_key(uint16_t key);
/* Wakeup config recode in RIU. */
int32_t pm_get_wakeup_config(const char *name, bool *enable);
int32_t pm_set_wakeup_config(const char *name, bool enable);
#if defined(CONFIG_MULTICORES_PLATFORM)
void *pm_init_thread_entry(void *args);
#endif
#endif
