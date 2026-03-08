/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _AMP_AD82089_HEADER
#define _AMP_AD82089_HEADER

struct amp_ad82089_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_powerdown;
};

#endif /* _AMP_AD82089_HEADER */
