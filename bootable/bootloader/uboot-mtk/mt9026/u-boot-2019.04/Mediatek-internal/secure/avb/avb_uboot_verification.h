/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef AVB_USER_VERIFICATION_H_
#define AVB_USER_VERIFICATION_H_

#include <../lib/libavb/libavb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function to enable or disable verification for an entire slot. The
 * passed in |ops| should be obtained via avb_ops_user_new(). The
 * |ab_suffix| parameter should specify the slot to modify including
 * the leading underscore (e.g. "_a" or "_b"). The
 * |enable_verification| parameter should be set to |true| to enable
 * dm-verification and |false| to disable.
 *
 * Returns |true| if the operation succeeded, otherwise |false|.
 */
bool avb_user_verification_set(AvbOps* ops,
                               const char* ab_suffix,
                               bool enable_verification);

/* Gets whether verification is enabled for an entire slot. The passed
 * in |ops| should be obtained via avb_ops_user_new(). The |ab_suffix|
 * parameter should specify the slot to query including the leading
 * underscore (e.g. "_a" or "_b"). The result is returned in the
 * |out_verification_enabled| parameter.
 *
 * Returns |true| if the operation succeeded, otherwise |false|.
 */
bool avb_user_verification_get(AvbOps* ops,
                               const char* ab_suffix,
                               bool* out_verification_enabled);

#ifdef __cplusplus
}
#endif

#endif /* AVB_USER_VERIFICATION_H_ */
