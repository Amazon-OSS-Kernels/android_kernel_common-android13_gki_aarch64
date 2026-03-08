/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-3-Clause) */
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#ifndef _ONE_PACKAGE_IMPL_
#define _ONE_PACKAGE_IMPL_

struct OnePackageInfo
{
    unsigned int magic; /* 0x4c495344 , used to confirm data struct is initialized */
    unsigned int version; /* default 0 */
    unsigned int reserved[2]; /* for align 16-byte */
    unsigned short unique_id[8]; /* used to confirm info is for this SoC*/
    unsigned int tee_model_id;
    unsigned int reserved2[1015];  /* append to 4KB */
};

#define ONE_PACKAGE_INFO_MAGIC (0x4c495344)
#define ONE_PACKAGE_INFO_VERSION (0x0)

int one_package_init(void);

#endif
