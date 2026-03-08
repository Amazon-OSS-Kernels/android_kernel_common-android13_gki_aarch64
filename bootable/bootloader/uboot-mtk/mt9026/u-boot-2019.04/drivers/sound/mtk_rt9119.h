/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _AMP_RT9119_HEADER
#define _AMP_RT9119_HEADER

struct amp_rt9119_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_powerdown;
	struct gpio_desc amp_reset;
};

#endif /* _AMP_RT9119_HEADER */
