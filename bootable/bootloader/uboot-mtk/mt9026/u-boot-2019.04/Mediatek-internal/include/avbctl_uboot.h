/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _AVBCTL_UBOOT_H_
#define _AVBCTL_UBOOT_H_

#include <../lib/libavb/libavb.h>

int avb_set_verification(AvbOps* ops, const char* ab_suffix, bool enable_verification);
int avb_get_verification(AvbOps* ops, const char* ab_suffix);
int avb_set_verity(AvbOps* ops, const char* ab_suffix, bool enable_verity);
int avb_get_verity(AvbOps* ops, const char* ab_suffix);

#endif
