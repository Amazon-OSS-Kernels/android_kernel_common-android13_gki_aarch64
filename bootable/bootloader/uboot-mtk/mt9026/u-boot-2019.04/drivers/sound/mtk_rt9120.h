// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _AMP_RT9120_HEADER
#define _AMP_RT9120_HEADER

struct amp_rt9120_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_powerdown;
};

#endif /* _AMP_RT9120_HEADER */
