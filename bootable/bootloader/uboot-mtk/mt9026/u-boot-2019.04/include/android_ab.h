/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2017 The Android Open Source Project
 */

#ifndef __ANDROID_AB_H
#define __ANDROID_AB_H

#include <common.h>

/* Android standard boot slot names are 'a', 'b', 'c', ... */
#define ANDROID_BOOT_SLOT_NAME(slot_num) ('a' + (slot_num))

/* Number of slots */
#define ANDROID_NUM_SLOTS 2

#if (CONFIG_TARGET_MT5896 == 1)
#define AB_RETRY_COUNT_MAXIMUM 3
/*TODO: confirm base address (0x1C040400)(0x1C040480) is fixed in M6 family in the future*/
#define ROMCODE_AB_RETRY_COUNT	0x1C040400
#define ROMCODE_AB_SLOT_NUM	0x1C040480
#define ROMCODE_AB_SLOT_A_SHIFT	4
#define ROMCODE_AB_SLOT_B_SHIFT	4
#else
#define AB_RETRY_COUNT_MAXIMUM 7
#endif

/**
 * Select the slot where to boot from.
 *
 * On Android devices with more than one boot slot (multiple copies of the
 * kernel and system images) selects which slot should be used to boot from and
 * registers the boot attempt. This is used in by the new A/B update model where
 * one slot is updated in the background while running from the other slot. If
 * the selected slot did not successfully boot in the past, a boot attempt is
 * registered before returning from this function so it isn't selected
 * indefinitely.
 *
 * @param[in] dev_desc Place to store the device description pointer
 * @param[in] part_info Place to store the partition information
 * @return The slot number (>= 0) on success, or a negative on error
 */
int ab_select_slot(struct blk_desc *dev_desc, disk_partition_t *part_info);

#if (CONFIG_AB_SIDELOAD == 1)
int ab_pre_select_slot(void);
/*1 if this slot has booted successfully, 0 otherwise */
unsigned char ab_get_select_successful_boot(void);
#endif
#endif /* __ANDROID_AB_H */
