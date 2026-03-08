/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <common.h>
#include "avbctl_uboot.h"
#include "avb_uboot_verification.h"
#include "avb_uboot_verity.h"
#include <debug_impl.h>

#define EX_SOFTWARE     (-1)
#define EX_OK           (0)

/* Function to enable and disable verification. The |ops| parameter
 * should be an |AvbOps| from libavb_user.
 */
int avb_set_verification(AvbOps* ops, const char* ab_suffix, bool enable_verification)
{
    bool verification_enabled;

    if (!avb_user_verification_get(ops, ab_suffix, &verification_enabled))
    {
        UBOOT_ERROR("Error getting whether verification is enabled.\n");
        return EX_SOFTWARE;
    }

    if ((verification_enabled && enable_verification) ||
        (!verification_enabled && !enable_verification))
    {
        UBOOT_INFO("verification is already %s", verification_enabled ? "enabled" : "disabled");
        if (strcmp(ab_suffix,"") != 0)
        {
            UBOOT_INFO(" on slot with suffix %s", ab_suffix);
        }
        UBOOT_INFO(".\n");
        return EX_OK;
    }

    if (!avb_user_verification_set(ops, ab_suffix, enable_verification))
    {
        UBOOT_ERROR("Error setting verification.\n");
        return EX_SOFTWARE;
    }

    UBOOT_INFO("Successfully %s verification", enable_verification ? "enabled" : "disabled");
    if (strcmp(ab_suffix,"") != 0)
    {
        UBOOT_INFO(" on slot with suffix %s", ab_suffix);
    }
    UBOOT_INFO(".\n");

    return EX_OK;
}

/* Function to query if verification. The |ops| parameter should be an
 * |AvbOps| from libavb_user.
 */
int avb_get_verification(AvbOps* ops, const char* ab_suffix)
{
    bool verification_enabled;

    if (!avb_user_verification_get(ops, ab_suffix, &verification_enabled))
    {
        UBOOT_ERROR("Error getting whether verification is enabled.\n");
        return EX_SOFTWARE;
    }

    UBOOT_INFO("verification is %s", verification_enabled ? "enabled" : "disabled");
    if (strcmp(ab_suffix,"") != 0)
    {
        UBOOT_INFO(" on slot with suffix %s", ab_suffix);
    }
    UBOOT_INFO(".\n");

    return EX_OK;
}

/* Function to enable and disable dm-verity. The |ops| parameter
 * should be an |AvbOps| from libavb_user.
 */
int avb_set_verity(AvbOps* ops, const char* ab_suffix, bool enable_verity)
{
    bool verity_enabled;

    if (!avb_user_verity_get(ops, ab_suffix, &verity_enabled))
    {
        UBOOT_ERROR("Error getting whether verity is enabled.\n");
        return EX_SOFTWARE;
    }

    if ((verity_enabled && enable_verity) ||
        (!verity_enabled && !enable_verity))
    {
        UBOOT_INFO("verity is already %s", verity_enabled ? "enabled" : "disabled");
        if (strcmp(ab_suffix,"") != 0)
        {
            printf(" on slot with suffix %s", ab_suffix);
        }
        UBOOT_INFO(".\n");
        return EX_OK;
    }

    if (!avb_user_verity_set(ops, ab_suffix, enable_verity))
    {
        UBOOT_ERROR("Error setting verity.\n");
        return EX_SOFTWARE;
    }

    UBOOT_INFO("Successfully %s verity", enable_verity ? "enabled" : "disabled");
    if (strcmp(ab_suffix,"") != 0)
    {
        UBOOT_INFO(" on slot with suffix %s", ab_suffix);
    }
    UBOOT_INFO(".\n");

    return EX_OK;
}

/* Function to query if dm-verity is enabled. The |ops| parameter
 * should be an |AvbOps| from libavb_user.
 */
int avb_get_verity(AvbOps* ops, const char* ab_suffix)
{
    bool verity_enabled;

    if (!avb_user_verity_get(ops, ab_suffix, &verity_enabled))
    {
        UBOOT_ERROR("Error getting whether verity is enabled.\n");
        return EX_SOFTWARE;
    }

    UBOOT_INFO("verity is %s", verity_enabled ? "enabled" : "disabled");
    if (strcmp(ab_suffix,"") != 0)
    {
    UBOOT_INFO(" on slot with suffix %s", ab_suffix);
    }
    UBOOT_INFO(".\n");

    return EX_OK;
}
