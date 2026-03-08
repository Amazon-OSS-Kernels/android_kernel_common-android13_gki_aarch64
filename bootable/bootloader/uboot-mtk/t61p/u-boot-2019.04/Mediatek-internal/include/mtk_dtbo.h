/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
*/

#ifndef _MTK_DTBO_H_
#define _MTK_DTBO_H_
#include <linux/list.h>

typedef struct{
	unsigned long dtb_addr;
	unsigned long dtbo_img_addr;
	bool is_uboot_dtb;
}DTB_INFO_T;

/**
* This interface should be used for customer to get u-boot flag.
*/
bool is_uboot_dtb(void);

void get_uboot_dtb_addr(unsigned long *dtb_addr);

/**
 * step1: u-boot should call set_dtbo_image_address() to set dtbo_img_addr
 */
void set_dtbo_image_address(unsigned long dtbo_img_addr);

/**
 * step2: u-boot should call set_dtb_image_address(dtb_addr, is_uboot_dtb)
 *   to set dtb_addr to this module with is_uboot_dtb
 *   set_dtb_image_address(gd->fdt_blob, true) for overlay u-boot dtb with dtbos
 *   set_dtb_image_address(knl_fdt_blob, false) for overlay kernel dtb with dtbos
 */
void set_dtb_image_address(unsigned long dtb_addr, bool is_uboot_dtb);

/**
 * step3: u-boot should call init_dtbo_selection().
 */
void init_dtbo_selection(void);

/**
 * This interface should be used for customer to implement its device
 * auto-detection and apply dt overlay
 */
void uboot_dtbo_selection(void);

/**
 * This interface should be used for customer to load dtbo from Android dtbo
 * format image according to dtbo_id. After calling, u-boot dtbo is overlay
 * to u-boot dtb. Besides, the selected dtbos are stored into this module so
 * that it's no need to select dtbo again when Linux kernel dtb perform
 * overlaying dtbos
 *
 * dtbo_id: dtbo_id in Android dtbo format image according to Android spec
 * return: zero for success
 *         negative value to indicate error
 */
int dtb_overlay(int dtbo_id);

/**
* This interface is used for getting kernel dtb addr.
*/
unsigned long get_dtb_image_address(void);

#endif
