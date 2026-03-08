// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _AD82088_HEADER
#define _AD82088_HEADER

struct amp_ad82088_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_reset;
	struct gpio_desc amp_powerdown;
};

#endif /* _AD82088_HEADER */