/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef AVB_USER_VERITY_H_
#define AVB_USER_VERITY_H_

#include <../lib/libavb/libavb.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Function to enable or disable dm-verity for an entire slot. The
 * passed in |ops| should be obtained via avb_ops_user_new(). The
 * |ab_suffix| parameter should specify the slot to modify including
 * the leading underscore (e.g. "_a" or "_b"). The |enable_verity|
 * parameter should be set to |true| to enable dm-verity and |false|
 * to disable.
 *
 * Returns |true| if the operation succeeded, otherwise |false|.
 */
bool avb_user_verity_set(AvbOps* ops,
                         const char* ab_suffix,
                         bool enable_verity);

/* Gets whether dm-verity is enabled for an entire slot. The passed in
 * |ops| should be obtained via avb_ops_user_new(). The |ab_suffix|
 * parameter should specify the slot to query including the leading
 * underscore (e.g. "_a" or "_b"). The result is returned in the
 * |out_verity_enabled| parameter.
 *
 * Returns |true| if the operation succeeded, otherwise |false|.
 */
bool avb_user_verity_get(AvbOps* ops,
                         const char* ab_suffix,
                         bool* out_verity_enabled);

#ifdef __cplusplus
}
#endif

#endif /* AVB_USER_VERITY_H_ */
