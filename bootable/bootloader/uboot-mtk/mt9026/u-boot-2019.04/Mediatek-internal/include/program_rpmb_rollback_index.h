/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <../lib/libavb/libavb.h>
#include <malloc.h>
#include <tee.h>


#define RPMB_H1L_VERSION 1
#define RPMB_RL_VERSION 2
#define RPMB_TL_VERSION 3
#define RPMB_ATF_VERSION 4
#define RPMB_OPTEE_VERSION 5
#define RPMB_UBOOT_VERSION 6
#define RPMB_OPTEECUST_VERSION 7

#define VBMETA_ROLLBACK_INDEX_LOCATION 0
#define RECOVERY_ROLLBACK_INDEX_LOCATION 1
#define BOOT_ROLLBACK_INDEX_LOCATION 2
#define PMU_ROLLBACK_INDEX_LOCATION 10

AvbIOResult get_rpmb_rollback_enabling_bit(unsigned int* val);
AvbIOResult read_avb_rollback_index_by_rpmb(size_t rollback_index_slot, u64 *out_rollback_index);
AvbIOResult read_sboot_rollback_index_by_rpmb(size_t rollback_index_slot, u64 *out_rollback_index);
AvbIOResult read_pmu_rollback_index_by_rpmb(u64 *out_rollback_index);
