/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */
/*
 * Copyright (c) 2022 MediaTek Inc.
 */

#ifndef _TAS5805_HEADER
#define _TAS5805_HEADER

struct amp_tas5805_priv {
	struct device	*dev;
	uint i2c_addr;
	struct gpio_desc amp_powerdown;
};

#endif /* _TAS5805_HEADER */
