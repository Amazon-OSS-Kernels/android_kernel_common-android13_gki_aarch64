// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _AMP_RT9114_HEADER
#define _AMP_RT9114_HEADER

struct amp_rt9114_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_mute;
	struct gpio_desc amp_reset;
};

#endif /* _AMP_RT9114_HEADER */
