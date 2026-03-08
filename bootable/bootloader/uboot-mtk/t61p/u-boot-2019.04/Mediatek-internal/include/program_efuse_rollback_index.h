/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef	_AVB_PROGRAM_EFUSE_VERSION_H
#define _AVB_PROGRAM_EFUSE_VERSION_H

#include <../drivers/driver/efuse/drv/drv_efuse.h>
#include <../lib/libavb/libavb.h>
#include <malloc.h>
#include <tee.h>
#include "efuse_rbi.h"

AvbIOResult read_rollback_index_by_efuse(size_t rollback_index_slot, u64 *out_rollback_index);
AvbIOResult read_pmu_rollback_index_by_efuse(u64 *out_rollback_index);
bool is_slot_in_efuse(size_t rollback_index_slot);
int rollbackindexes_to_efuse(AvbSlotVerifyData* AvbData);
AvbIOResult get_efuse_rollback_enabling_bit(unsigned int* val);

#endif /* _AVB_PROGRAM_EFUSE_VERSION_H */
