/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _AMP_NTP8928_HEADER
#define _AMP_NTP8928_HEADER

struct amp_ntp8928_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_powerdown;
	struct gpio_desc amp_reset;
};

#endif /* _AMP_NTP8928_HEADER */
