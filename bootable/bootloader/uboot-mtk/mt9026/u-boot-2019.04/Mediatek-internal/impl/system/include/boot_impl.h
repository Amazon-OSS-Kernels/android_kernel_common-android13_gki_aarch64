/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _BOOT_IMPL_
#define _BOOT_IMPL_

#define BOOT_MODE_RECOVERY_STR "boot-recovery"
#define BOOT_MODE_RECOVERY_QUIESCENT_STR "boot-recovery,quiescent"
#define BOOT_MODE_RECOVERY_WIPE_DATA "recovery\n--wipe_data\n"
#define BOOT_MODE_QUIESCENT_STR "boot-quiescent"
#if defined(CONFIG_FASTBOOT)
#define BOOT_MODE_FASTBOOTD_STR "boot-fastboot"
#define BOOT_MODE_BOOTLOADER_STR "bootonce-bootloader"
#endif

#define ERASE_FIRST_4K_TO_SPEEDUP
#define USERDATA_PARTITON_NAME   "userdata"
#define METADATA_PARTITON_NAME   "metadata"
#define CLEAN_BUFFER_ADDR   0x27000000UL
#ifndef ERASE_FIRST_4K_TO_SPEEDUP
#define CLEAN_BUFFER_SIZE   0x10000000UL
#else
#define CLEAN_BUFFER_SIZE   0x1000UL
#endif

typedef enum
{
    EN_BOOT_MODE_UNKNOWN,
    EN_BOOT_MODE_NORMAL,
    EN_BOOT_MODE_RECOVERY,
    EN_BOOT_MODE_FORCEUGRADE,
    EN_BOOT_MODE_USB_UPGRADE,
    EN_BOOT_MODE_NET_UPGRADE,
    EN_BOOT_MODE_OAD_UPGRADE,
    EN_BOOT_MODE_UPDATELOGIC_UPGRADE,
    EN_BOOT_MODE_ENV_UPGRADE,
    EN_BOOT_MODE_USB_RECOVRY_UPGRADE,
    EN_BOOT_MODE_OTA_UPGRADE,
    EN_BOOT_MODE_RECOVRY_WIPE_DATA,
    EN_BOOT_MODE_RECOVRY_WIPE_CACHE,
    EN_BOOT_MODE_UART_DEBUG,
    EN_BOOT_MODE_SYSTEM_RESTORE,
    EN_BOOT_MODE_FASTBOOT,
    EN_BOOT_MODE_FORCEPOWERON,
    EN_BOOT_MODE_POWERKEYLONGPRESS,
}EN_BOOT_MODE;

#if defined(CONFIG_MT58XX_SARADC)
void long_press_sar_key_detect(void);
void long_press_sar_reset_key_detect(void);
#endif
int get_boot_mode_flag(void);
int boot_mode_check(void);
int upgrade_mode_check(void);
int get_kernel_start(unsigned long long *);
#if defined(CONFIG_BOOT_IR)
int bootir_mode_check(void);
#endif
int set_recovery(void);
int wipe_user_data(void);
int wipe_user_data_in_uboot(void);
bool is_fastboot_bootloader_mode(void);
int is_ldm_support(void);
#endif
